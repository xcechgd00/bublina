#ifndef MATERIAL_H
#define MATERIAL_H

#include "bubbles.h"
#include <complex>

class material{
    public:
        virtual ~material() = default;

        virtual bool scatter(const wave& w_in, const hit_record& rec, std::vector<wave>& outgoing) const {
            return false;
        }
};

class dielectric: public material{
    public:
        dielectric(double refraction_index) : refraction_index(refraction_index) {}

        bool scatter(const wave& w_in, const hit_record& rec, std::vector<wave>& outgoing) const override {

            double n1 = w_in.medium();
            double n2 = refraction_index;

            double ri = n1 / n2;

            double cos_theta = std::fmin(dot(-w_in.direction(), rec.normal), 1.0);
            double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

            double sin_t = (ri) * sin_theta;

            if (sin_t >= 1.0)
            {
                wave reflected(
                    rec.p,
                    reflect(w_in.direction(), rec.normal),
                    w_in.wavelength(),
                    w_in.amplitude_s(),
                    w_in.amplitude_p(),
                    n1
                );

                outgoing.push_back(reflected);

                return true;
            }

            double cos_t = std::sqrt(1.0 - sin_t * sin_t);

            auto med_theta = n1 * cos_theta;
            auto refract_t = n2 * cos_t;

            auto refract_theta = n2 * cos_theta;
            auto med_t = n1 * cos_t;

            double r_s = (med_theta - refract_t) / (med_theta + refract_t);
            double r_p = (refract_theta - med_t) / (med_t + refract_theta);

            double t_s = (2 * n1 * cos_theta) / (med_theta + refract_t);
            double t_p = (2 * n1 * cos_theta) / (refract_theta + med_t);


            double delta_phi = (2 * pi / w_in.wavelength()) * n1 * rec.distance;
            std::complex<double> propagation = std::exp(std::complex<double>(0, delta_phi));

            std::complex<double> r_amplitude_s = w_in.amplitude_s() * propagation;
            std::complex<double> r_amplitude_p = w_in.amplitude_p() * propagation;
            std::complex<double> t_amplitude_s = w_in.amplitude_s() * propagation;
            std::complex<double> t_amplitude_p = w_in.amplitude_p() * propagation;

            r_amplitude_s *= r_s;
            r_amplitude_p *= r_p;
            t_amplitude_s *= t_s;
            t_amplitude_p *= t_p;

            wave reflected(rec.p, reflect(w_in.direction(), rec.normal), w_in.wavelength(), r_amplitude_s, r_amplitude_p, n1);
            wave transmitted(rec.p, refract(w_in.direction(), rec.normal, ri), w_in.wavelength(), t_amplitude_s, t_amplitude_p, n2);

            outgoing.push_back(reflected);
            outgoing.push_back(transmitted);

            return true;
        }

    private:
        double refraction_index = 1.0;
};

#endif