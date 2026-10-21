#ifndef BUBBLES_H
#define BUBBLES_H

#include <cmath>
#include <iostream>
#include <cstdlib>
#include <limits>
#include <memory>

using std::make_shared;
using std::shared_ptr;

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

inline double degrees_to_radians(double degrees){
    return degrees * pi / 180.0;
}

inline double random_double(){
    return std::rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max){
    return min + (max - min) * random_double();
}

#include "vec3.h"
#include "wave.h"
#include "interval.h"
#include "color.h"
#include "hittable.h"
#include <vector>
#include "hittable_list.h"
#include "material.h"
#include <complex>
#include "sphere.h"
#include <cmath>
#include <limits>
#include <cstdlib>

#endif