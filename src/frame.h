/*
 * frame.h
 *
 *  Created on: 30 Sep 2014
 *      Author: up45
 */

#ifndef FRAME_H_
#define FRAME_H_

#include <string>
#include <vector>
#include "hdf5.h"


class Frame {
public:
    Frame();
    Frame(const Frame& frame);
    Frame(const std::vector<hsize_t>& dims, unsigned int * pdata);
    Frame(const std::vector<hsize_t>& dims, const unsigned int * pdata);
    Frame(const std::string fname, const std::string dset);
    ~Frame();

    const std::vector<hsize_t>& dimensions();
    unsigned int * create_buffer();
    const unsigned int * pdata();

    // Operators
    Frame& operator=(const Frame& src); // assignment
    bool operator==(const Frame& cmp);  // equal
    bool operator!=(const Frame& cmp);  // not equal
private:
    unsigned int *m_pdata;
    std::vector<hsize_t> m_dims;

    void copy(const Frame& src);
    bool is_equal(const Frame& src);
    unsigned long long num_items();
    unsigned long long num_items(const std::vector<hsize_t>& dims) const;
};



#endif /* FRAME_H_ */
