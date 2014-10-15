
#include <iostream>
#include <iomanip>
#include <iterator>
#include <assert.h>

#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/simplelayout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/basicconfigurator.h>
using namespace log4cxx;
using namespace log4cxx::xml;

#include <boost/program_options.hpp>
namespace po = boost::program_options;
#include <boost/any.hpp>
using boost::any_cast;

#include "swmr-reader.h"
#include "swmr-writer.h"

using namespace std;

class SwmrDemoCli {
public:
    SwmrDemoCli();
    ~SwmrDemoCli();
    void main_args(int argc, char* argv[]);
    void parse_options();
    int run();
    void log_options();

private:
    int run_read();
    int run_write();

    enum {help, read, write} m_subcmd;
    LoggerPtr m_log;
    int m_argc;
    char **m_argv;
    po::options_description m_options_description;
    po::variables_map m_options;
};

SwmrDemoCli::SwmrDemoCli() :
        m_argc(0), m_argv(NULL), m_subcmd(help)
{
    m_log = Logger::getLogger("SwmrDemoCli");
}

SwmrDemoCli::~SwmrDemoCli()
{
    if (m_argv != NULL) {
        delete [] m_argv;
    }
}

void SwmrDemoCli::main_args(int argc, char* argv[])
{
    m_argc = argc - 1;
    m_argv = new char *[m_argc];

    if (argc < 2) {
        LOG4CXX_ERROR(m_log, "ERROR: No subcommand found" );
        throw runtime_error("No subcommand found. What do you want?");
    }

    m_argv[0] = argv[0]; // The program name

    string subcmd = argv[1];  // The program subcommand
    if (subcmd == "help" or subcmd == "h") {
        m_subcmd = help;
    } else if (subcmd == "read" or subcmd == "r") {
        m_subcmd = read;
    } else if (subcmd == "write" or subcmd == "w") {
        m_subcmd = write;
    } else {
        LOG4CXX_ERROR(m_log, "ERROR: Unknown subcommand: " << subcmd );
    }

    // Shift all the remaining args forward to remove the subcommand
    for (int i = 2; i < argc; i++)
    {
        m_argv[i-1] = argv[i];
    }
}

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

void SwmrDemoCli::parse_options()
{
    string desc_string;

    po::options_description common_opts("Common options");
    common_opts.add_options()
        ("help,h", "Produce help message and quit")
        ("dataset,s", po::value<string>()->default_value("data"),
                "Name of HDF5 SWMR dataset to use")
        ("testdatafile,f", po::value<string>(),
                "HDF5 reference test data file name")
        ("testdataset,d", po::value<string>()->default_value("data"),
                "HDF5 reference dataset name")
        ("logconfig,l", po::value<string>(),
                "Log4CXX XML configuration file");

    po::options_description cmd_options_description("Command options");
    switch(m_subcmd) {
    case help:
        // ignore any other options set
        desc_string =  "Usage:\n  swmr SUBCMD [options] [DATAFILE]\n\n"
                       "    SUBCMD:   The subcommand to run (help|read|write)\n"
                       "    DATAFILE: The HDF5 SWMR datafile to operate on.\n\n"
                       "Option Groups";
        //cmd_options_description.add(po::options_description(desc_string)).add(common_opts);
        break;
    case read:
        desc_string =  "Usage:\n  swmr read [options] [DATAFILE]\n\n"
                       "    DATAFILE: The HDF5 SWMR datafile to operate on.\n\n"
                       "Option Groups";
        //cmd_options_description.add(po::options_description(desc_string)).add(common_opts);
        cmd_options_description.add_options()
            ("nframes,n", po::value<int>()->default_value(-1),
                    "Number of frames to expect in input dataset (-1: unknown)")
            ("timeout,t", po::value<double>()->default_value(2.0),
                    "Timeout [sec] waiting for new data")
            ("polltime,p", po::value<double>()->default_value(1.0),
                    "Monitor polling time [sec]");
        break;
    case write:
        desc_string =  "Usage:\n  swmr write [options] [DATAFILE]\n\n"
                       "    DATAFILE: The HDF5 SWMR datafile to operate on.\n\n"
                       "Option Groups";
        //cmd_options_description.add(po::options_description(desc_string)).add(common_opts);
        cmd_options_description.add_options()
            ("niter,n", po::value<int>()->default_value(2),
                    "Number of write iterations")
            ("chunk,c", po::value<int>()->default_value(1),
                    "Number of chunked frames")
            ("period,p", po::value<double>()->default_value(1.0),
                    "Write loop period [sec]");
        break;
    }

    po::options_description options_description(desc_string);
    options_description.add(common_opts).add(cmd_options_description);
    m_options_description.add(options_description); // store the public options in the class

    // Hidden options, will be allowed both on command line and
    // in config file, but will not be shown to the user.
    po::options_description hidden("Hidden options");
    hidden.add_options()
        ("datafile", po::value<string>()->default_value("swmr.h5"),
                "HDF5 SWMR input filename");
    po::positional_options_description p;
    p.add("datafile", -1);
    options_description.add(hidden);

    try {
        store(po::command_line_parser(m_argc, m_argv).
              options(options_description).positional(p).run(), m_options);
        notify(m_options);

        option_dependency(m_options, "testdataset", "testdata");
    }
    catch(exception& e) {
        LOG4CXX_ERROR(m_log, "Exception (rethrowing): " << e.what() );
        throw;
    }

    if (m_options.count("logconfig")) {
        DOMConfigurator::configure(m_options["logconfig"].as<string>());
    }
}

void SwmrDemoCli::log_options()
{
    // Debug - print out the options and values
    LOG4CXX_DEBUG(m_log, "Subcommand: " << m_subcmd);
    po::variables_map::iterator it;
    for (it = m_options.begin(); it != m_options.end(); ++it)
    {
        if (not it->second.empty()) {
            if (it->second.value().type() == typeid(int)) {
                LOG4CXX_DEBUG(m_log, setw(14) << it->first << ": " << it->second.as<int>());
            } else if (it->second.value().type() == typeid(double)) {
                LOG4CXX_DEBUG(m_log, setw(14) << it->first << ": " << it->second.as<double>());
            } else if (it->second.value().type() == typeid(string)) {
                LOG4CXX_DEBUG(m_log, setw(14) << it->first << ": " << it->second.as<string>());
            } else {
                LOG4CXX_DEBUG(m_log, setw(14) << it->first << ": <nknown datatype>");
            }
        } else {
            LOG4CXX_DEBUG(m_log, setw(14) << it->first << ": <empty>");
        }
    } // End-of-Debug
}

int SwmrDemoCli::run()
{
    int ret = 0;
    if (m_options.count("help")) {
        m_subcmd = help;
    }

    switch(m_subcmd) {
    case help:
        cout << "Available subcommands: [help|read|write] " << endl;
        cout << m_options_description << endl;
        ret = 0;
        break;
    case read:
        LOG4CXX_DEBUG(m_log, "Reading...");
        this->run_read();
        break;
    case write:
        LOG4CXX_DEBUG(m_log, "Writing...");
        this->run_write();
        break;
    }
    return ret;
}

int SwmrDemoCli::run_read()
{
    string datafile(m_options["datafile"].as<string>());
    string dataset(m_options["dataset"].as<string>());

    LOG4CXX_DEBUG(m_log, "Creating a SWMR Reader object");
    SWMRReader srd;

    LOG4CXX_INFO(m_log, "Opening file (" << datafile << ")");
    srd.open_file(datafile, dataset);

    LOG4CXX_DEBUG(m_log, "Getting test data");
    if (m_options.count("testdatafile")) {
        string testdatafile(m_options["testdatafile"].as<string>());
        string testdataset(m_options["testdataset"].as<string>());
        srd.get_test_data(testdatafile, testdataset);
    } else {
        srd.get_test_data();
    }

    LOG4CXX_INFO(m_log, "Starting monitor");
    double polltime = m_options["polltime"].as<double>();
    double timeout = m_options["timeout"].as<double>();
    srd.monitor_dataset(timeout, polltime);
    int fail_count = srd.report();
    return fail_count;
}

int SwmrDemoCli::run_write()
{
    string datafile(m_options["datafile"].as<string>());
    int niter = m_options["niter"].as<int>();
    int nchunked_frames = m_options["chunk"].as<int>();
    double period = m_options["period"].as<double>();

    LOG4CXX_DEBUG(m_log, "Creating a SWMR Writer object (" << datafile << ")");
    SWMRWriter swr = SWMRWriter(datafile);

    LOG4CXX_DEBUG(m_log, "Creating file: "<< datafile);
    swr.create_file();

    LOG4CXX_INFO(m_log, "Getting test data");
    if (m_options.count("testdatafile")) {
        string testdatafile(m_options["testdatafile"].as<string>());
        string testdataset(m_options["testdataset"].as<string>());
        swr.get_test_data(testdatafile, testdataset);
    } else {
        swr.get_test_data();
    }

    LOG4CXX_INFO(m_log, "Writing 40 iterations");
    swr.write_test_data(niter, nchunked_frames, period);

    return 0;
}


int main(int ac, char* av[])
{
    // Create a default simple console appender for log4cxx.
    // This can be overridden by the user on the CLI with the --logconfig option
    ConsoleAppender * consoleAppender = new ConsoleAppender(LayoutPtr(new SimpleLayout()));
    BasicConfigurator::configure(AppenderPtr(consoleAppender));
    Logger::getRootLogger()->setLevel(Level::getWarn());

    SwmrDemoCli cli;
    cli.main_args(ac, av);
    cli.parse_options();
    cli.log_options();
    return cli.run();
}

