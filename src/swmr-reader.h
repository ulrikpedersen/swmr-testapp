/*
 * swmr-reader.h
 *
 *  Created on: 2 Oct 2014
 *      Author: up45
 */

#ifndef SWMR_READER_H_
#define SWMR_READER_H_

#include <string>
#include <log4cxx/logger.h>
#include <hdf5.h>

#include "frame.h"

class SWMRReader {
public:
    SWMRReader();
    ~SWMRReader();
    void open_file(const std::string& fname, const std::string& dsetname);
    void get_test_data();
    void get_test_data(const std::string& fname, const std::string& dsetname);
    unsigned long long latest_frame_number();
    void read_latest_frame();
    bool check_dataset();
    void monitor_dataset(double timeout = 2.0, double polltime=0.2, int expected=-1);
    int report();

private:
    void print_open_objects();

    LoggerPtr m_log;
    std::string m_filename;
    std::string m_dsetname;
    hid_t m_fid;
    hsize_t m_dims[3];
    hsize_t m_maxdims[3];

    Frame m_testimg;
    uint32_t * m_pdata;
    unsigned long long m_latest_framenumber;
    std::vector<bool> m_checks;
};

#endif /* SWMR_READER_H_ */
