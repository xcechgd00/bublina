#ifndef WAVE_H
#define WAVE_H

#include "bubbles.h"
#include <complex>

class wave{
    public:
        wave () {}

        wave(const point3& origin, const vec3& direction, const double& wavelength, const std::complex<double>& amplitude_s, const std::complex<double>& amplitude_p, double medium) : 
        orig(origin), dir(direction), wl(wavelength),  E_s(amplitude_s), E_p(amplitude_p), med(medium) {}

        const point3 origin() const {return orig;}
        const point3 direction() const {return dir;}
        const double wavelength() const {return wl;}
        const std::complex<double> amplitude_s() const {return E_s;}
        const std::complex<double> amplitude_p() const {return E_p;}
        const double medium() const {return med;}

        point3 at(double t) const {
            return orig + t * dir;
        }

    private:
        point3 orig;
        vec3 dir;
        double wl;
        std::complex<double> E_s;
        std::complex<double> E_p;
        double med;
        double optical_distance;
};

#endif