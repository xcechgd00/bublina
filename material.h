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

            vec3 unit_wave_direction = unit_vector(w_in.direction());

            if(!rec.front_face){
                if(refraction_index > 1.32 && refraction_index < 1.34){
                    n2 = 1.0;
                }
                else {
                    n2 = 1.33;
                }
            }

            double ri = n1 / n2;

            // calculating the angle of refraction

            double cos_theta = std::fmin(dot(-unit_wave_direction, rec.normal), 1.0);
            double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

            double sin_t = (ri) * sin_theta;

            double optical_path_length = n1 * rec.distance;

            double delta_phi = (2 * pi / w_in.wavelength()) * optical_path_length;
            std::complex<double> propagation = std::exp(std::complex<double>(0, delta_phi));

            if (sin_t >= 1.0)
            {
                wave reflected(
                    rec.p,
                    unit_vector(reflect(unit_wave_direction, rec.normal)),
                    w_in.wavelength(),
                    w_in.amplitude_s() * propagation,
                    w_in.amplitude_p() * propagation,
                    n1
                );

                outgoing.push_back(reflected);

                return true;
            }

            // std::cout << "IN: " << n1 << "OUT: " << n2 << "\n";

            double cos_t = std::sqrt(1.0 - sin_t * sin_t);

            auto n1_cos_theta = n1 * cos_theta;
            auto n2_cos_t = n2 * cos_t;

            auto n2_cos_theta = n2 * cos_theta;
            auto n1_cos_t = n1 * cos_t;

            // fresnel equations, s for perpendicular and p for parallel

            double r_s = (n1_cos_theta - n2_cos_t) / (n1_cos_theta + n2_cos_t);
            double r_p = (n2_cos_theta - n1_cos_t) / (n1_cos_t + n2_cos_theta);

            // double t_s = (2 * n1 * cos_theta) / (n1_cos_theta + n2_cos_t);
            // double t_p = (2 * n1 * cos_theta) / (n2_cos_theta + n1_cos_t);

            double t_s = r_s + 1;
            double t_p = ri * (r_p + 1);

            std::complex<double> s_propagated = w_in.amplitude_s() * propagation;
            std::complex<double> p_propagated = w_in.amplitude_p() * propagation;

            std::complex<double> r_amplitude_s = s_propagated * r_s;
            std::complex<double> r_amplitude_p = p_propagated * r_p;
            std::complex<double> t_amplitude_s = s_propagated * t_s;
            std::complex<double> t_amplitude_p = p_propagated * t_p;

            wave reflected(rec.p, unit_vector(reflect(unit_wave_direction, rec.normal)), w_in.wavelength(), r_amplitude_s, r_amplitude_p, n1);
            wave transmitted(rec.p, unit_vector(refract(unit_wave_direction, rec.normal, ri)), w_in.wavelength(), t_amplitude_s, t_amplitude_p, n2);

            outgoing.push_back(reflected);
            outgoing.push_back(transmitted);

            return true;
        }

    private:
        double refraction_index = 0.0;
};

#endif