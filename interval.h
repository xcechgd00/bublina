#ifndef INTERVAL_H
#define INTERVAL_H

#include "bubbles.h"

class interval {
    public:
        double min, max;
        // default inicializacia?
        interval() : min(+infinity), max(-infinity) {}

        interval(double min, double max) : min(min), max(max) {}

        double size() const {
            return max - min;
        }

        double contains(double x) const {
            return x <= max && x >= min;
        }

        double surrounds(double x) const {
            return x < max && x > min;
        }
        
        double clamp(double x) const{
            if (x < min) return min;
            if (x > max) return max;
            return x;
        }
        // tomuto nechapem
        static const interval empty, universe;
};

const interval interval::empty    = interval(+infinity, -infinity);
const interval interval::universe = interval(-infinity, +infinity);


#endif