
#include <iostream>
#include <iomanip>
#include <iterator>
#include <assert.h>

#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
using namespace log4cxx;
using namespace log4cxx::xml;

#include <boost/program_options.hpp>
namespace po = boost::program_options;
#include <boost/any.hpp>
using boost::any_cast;

#include "swmr-reader.h"

using namespace std;

void option_dependency(const po::variables_map& vm,
                       const char* for_what,
                       const char* required_option)
{
    if (vm.count(for_what) && !vm[for_what].defaulted())
        if (vm.count(required_option) == 0 || vm[required_option].defaulted())
            throw logic_error(string("Option '") + for_what
                              + "' requires option '"
                              + required_option + "'.");
}


int get_options(int ac, char* av[], po::variables_map& vm)
{
    try {
    
        po::options_description global_opts("Options");
        global_opts.add_options()
            ("help,h", "Produce help message and quit")
            ("command", po::value<std::string>(), "Command to run (read|write)")
            ("subargs", po::value<std::vector<std::string> >(), "Arguments for command");
        po::positional_options_description global_pos;
        global_pos.add("command", 1).add("subargs", -1);
        po::parsed_options parsed = po::command_line_parser(ac, av).
            options(global_opts).
            positional(global_pos).
            allow_unregistered().
            run();
        po::store(parsed, vm);
        std::string cmd = vm["command"].as<std::string>();

        string desc_string = "Usage:\n  swmr [options] [DATAFILE]\n\n"
                             "    DATAFILE: The HDF5 SWMR datafile to operate on.\n\n"
                             "Options";
        
        po::options_description common_opts(desc_string);
        common_opts.add_options()
            ("help,h", "Produce help message and quit")
            ("dataset,s", po::value<string>()->default_value("data"),
                    "Name of HDF5 SWMR dataset to use")
            ("testdatafile,f", po::value<string>(),
                    "HDF5 reference test data file name")
            ("testdataset,d", po::value<string>()->default_value("data"),
                    "HDF5 reference dataset name");
        // Hidden options, will be allowed both on command line and
        // in config file, but will not be shown to the user.
        po::options_description hidden("Hidden options");
        hidden.add_options()
            ("datafile", po::value<string>()->default_value("swmr.h5"),
                    "HDF5 SWMR input filename")
        ;
        po::options_description cmdline_options;
        cmdline_options.add(common_opts).add(hidden);

        po::positional_options_description p;
        p.add("datafile", -1);
        store(po::command_line_parser(ac, av).
              options(cmdline_options).positional(p).run(), vm);
        notify(vm);

        if (cmd == "read") {
            po::options_description read_opts("write options");
            read_opts.add_options()
                    ("nframes,n", po::value<int>()->default_value(-1),
                            "Number of frames to expect in input dataset (-1: unknown)")
                    ("timeout,t", po::value<double>()->default_value(2.0),
                            "Timeout [sec] waiting for new data")
                    ("polltime,p", po::value<double>()->default_value(0.20),
                            "Monitor polling time [sec]");

            // Collect all the unrecognized options from the first pass. This will include the
            // (positional) command name, so we need to erase that.
            std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
            opts.erase(opts.begin());

            read_opts.add(cmdline_options);
            // Parse again...
            po::store(po::command_line_parser(opts).options(read_opts).run(), vm);

        } else if (cmd == "write") {
            po::options_description write_opts("write options");
            write_opts.add_options()
                    ("nframes,n", po::value<int>()->default_value(-1),
                            "Number of frames to expect in input dataset (-1: unknown)")
                    ("timeout,t", po::value<double>()->default_value(2.0),
                            "Timeout [sec] waiting for new data")
                    ("polltime,p", po::value<double>()->default_value(0.20),
                            "Monitor polling time [sec]");

            // Collect all the unrecognized options from the first pass. This will include the
            // (positional) command name, so we need to erase that.
            std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
            opts.erase(opts.begin());

            // Parse again...
            po::store(po::command_line_parser(opts).options(write_opts).run(), vm);


        } else {
            cerr << "ERROR: unsupported command: " << cmd << endl;
            return -1;
        }



        po::store(po::parse_command_line(ac, av, common_opts), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << common_opts << endl;
            return 1;
        }

        option_dependency(vm, "testdataset", "testdata");
    }
    catch(exception& e) {
        cerr << "Error: " << e.what() << endl;
        return -1;
    }
    return 0;
}

int main(int ac, char* av[])
{
    DOMConfigurator::configure("Log4cxxConfig.xml");
    LoggerPtr log(Logger::getLogger("swmr-reader"));

    po::variables_map options;
    int parseerr = get_options(ac, av, options);
    if (parseerr < 0) {
        cerr << "Option parser error. Aborting" << endl;
        return 2;
    }
    if (options.count("help")) return 0;

    // Debug - print out the options and values
    po::variables_map::iterator it;
    for (it = options.begin(); it != options.end(); ++it)
    {
        if (not it->second.empty()) {
            if (it->second.value().type() == typeid(int)) {
                LOG4CXX_DEBUG(log, setw(14) << it->first << ": " << it->second.as<int>());
            } else if (it->second.value().type() == typeid(double)) {
                LOG4CXX_DEBUG(log, setw(14) << it->first << ": " << it->second.as<double>());
            } else if (it->second.value().type() == typeid(string)) {
                LOG4CXX_DEBUG(log, setw(14) << it->first << ": " << it->second.as<string>());
            } else {
                LOG4CXX_DEBUG(log, setw(14) << it->first << ": <nknown datatype>");
            }
        } else {
            LOG4CXX_DEBUG(log, setw(14) << it->first << ": <empty>");
        }
    } // End-of-Debug

    LOG4CXX_INFO(log, "Creating a SWMR Reader object");
    SWMRReader srd = SWMRReader();

    LOG4CXX_INFO(log, "Opening file");
    srd.open_file(options["datafile"].as<string>(),
                  options["dataset"].as<string>());

    LOG4CXX_INFO(log, "Getting test data");
    if (options.count("testdatafile")) {
        srd.get_test_data(options["testdatafile"].as<string>(),
                          options["testdataset"].as<string>());
    } else {
        srd.get_test_data();
    }

    LOG4CXX_INFO(log, "Starting monitor. Timeout = " << options["timeout"].as<double>()
                 << " Polltime = " << options["polltime"].as<double>());
    srd.monitor_dataset(options["timeout"].as<double>(),
                        options["polltime"].as<double>());

    return 0;

}
