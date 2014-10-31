#ifndef SWMR_WRITER_H_
#define SWMR_WRITER_H_

#include <string>
#include <vector>
#include <log4cxx/logger.h>
#include <hdf5.h>

#include "frame.h"

class SWMRWriter {
public:
    SWMRWriter(const std::string& fname);
    ~SWMRWriter();
    void create_file();
    void get_test_data();
    void get_test_data(const std::string& fname, const std::string& dsetname);
    void write_test_data(unsigned int niter, unsigned int nframes_cache, bool direct);
    void report();

private:
    LoggerPtr log;
    hid_t fid;
    std::string filename;
    Frame img;
    std::vector<double> write_times;
    double dt_start;
    unsigned int nframes;
};




#endif /* SWMR_WRITER_H_ */
