#ifndef PROGRESSBAR_H_
#define PROGRESSBAR_H_

#include <iostream>
#include <iomanip>
#include <sys/ioctl.h>

static inline void progressbar(unsigned int x, unsigned int n)
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    if ( (x != n) && (x % (n/100+1) != 0) ) return;
 
    float ratio  =  x/(float)n;
    int   c      =  ratio * w.ws_col;
 
    std::cout << std::setw(3) << (int)(ratio*100) << "% [";
    for (int x=0; x<c; x++) std::cout << "=";
    for (int x=c; x<w.ws_col; x++) std::cout << " ";
    std::cout << "]\n\033[F\033[J" << std::flush;
}

#endif /* PROGRESSBAR_H_ */
