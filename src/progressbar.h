#ifndef PROGRESSBAR_H_
#define PROGRESSBAR_H_

#include <iostream>
#include <iomanip>
#include <sys/ioctl.h>
#include <cstdio>

static inline void progressbar(unsigned int progress, unsigned int nitems)
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    unsigned short int bar_width = w.ws_col - 10;

    if ( (progress != nitems) && (progress % (nitems/100+1) != 0) ) return;
 
    float ratio  =  progress/(float)nitems;
    int   c      =  ratio * bar_width;
 
    std::cout << std::setw(3) << (int)(ratio*100) << "% [";
    for (int x=0; x<c; x++) std::cout << "=";
    for (int x=c; x<bar_width; x++) std::cout << " ";
    std::cout << "]\r" << std::flush;
    if (progress >= nitems) std::cout << std::endl;
}

#endif /* PROGRESSBAR_H_ */
