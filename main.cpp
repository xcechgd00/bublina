#include "bubbles.h"
#include "hittable_list.h"
#include "cie_table.h"

using namespace std::complex_literals;

struct polarized_amplitude {
    std::complex<double> s;
    std::complex<double> p;

    polarized_amplitude() : s(0), p(0) {}

    polarized_amplitude(std::complex<double> s, std::complex<double> p)
        : s(s), p(p) {}

    polarized_amplitude operator+(const polarized_amplitude& other) const {
        return {
            s + other.s,
            p + other.p
        };
    }

    polarized_amplitude& operator+=(const polarized_amplitude& other) {
        s += other.s;
        p += other.p;
        return *this;
    }
};

polarized_amplitude wave_send(const wave&w, int depth, const hittable&world);
double background_intensity(const vec3& direction);

CIE_entry cie_lookup(int wl){
    int idx = (wl - 400);
    return cie_table[idx]; 
}

long double nm = 1e-9;

int main(void){
    
    double vfov = 90;
    double aspect_ratio = 16.0/9.0;
    int image_width = 1200;
    int image_height = int(image_width/aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;

    point3 lookfrom = point3(0, 0, 0);
    point3 lookat = point3(0, 0, -1);
    vec3 vup = vec3(0, 1, 0);

    auto focal_length = 1.0;
    auto theta = degrees_to_radians(vfov);
    auto h = std::tan(theta/2) * focal_length;
    auto viewport_height = 2 * h;
    auto viewport_width = viewport_height * (double(image_width)/image_height);

    auto camera_center = lookfrom;

    vec3 u, v, w;

    w = unit_vector(lookfrom - lookat);
    u = unit_vector(cross(vup, w));
    v = cross(u, w);

    auto viewport_u = viewport_width * u;
    auto viewport_v = viewport_height * v;

    auto pixel_delta_u = viewport_u / image_width;
    auto pixel_delta_v = viewport_v / image_height;

    auto viewport_upper_left = camera_center - (focal_length * w) - viewport_u / 2 - viewport_v / 2;
    auto pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

    std::cout << "P3\n" << image_width << " " << image_height << "\n255\n";

    hittable_list world;

    long double thickness = 500 * nm;

    auto material = make_shared<dielectric>(1.33);
    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5, material));

    // auto material_ground = make_shared<dielectric>(2.42);
    // world.add(make_shared<sphere>(point3(0, -499999999, -120000000), 500000000, material_ground));

    auto material_inside = make_shared<dielectric>(1.0);
    world.add(make_shared<sphere>(point3(0, 50 * nm, -1), 0.5 - thickness, material_inside));

    double exposure = 1 / 5.0;
    int samples_per_pixel = 10;
    double samples_inverted = 1.0 / samples_per_pixel;
    int wave_depth = 7;
    double exp_samples = exposure * samples_inverted;


    /*
    Iterate through every pixel, sending wavelengths through each one, checking for object hits
    When finished, calculate intensity and color by using CIE lookup table
    */
    for(int j = 0; j < image_height; j++){
        std::clog << "\rScanlines remaining: " << (image_height -j) << ' ' << std::flush; 
        for(int i = 0; i < image_width; i++){

        double X = 0, Y = 0, Z = 0;
            for(int s = 0; s < samples_per_pixel; s++){
                auto pixel_center = pixel00_loc + (((i + random_double(-0.5, 0.5)) * pixel_delta_u)) + (((j + random_double(-0.5, 0.5)) * pixel_delta_v));
                auto wave_direction = pixel_center - camera_center;


                for(int wl = 400; wl <= 700; wl += 10){
                    wave w(camera_center, wave_direction, wl * nm, 1.0 + 0.0i, 0.0 + 1.0i, 1.0);

                    polarized_amplitude total = wave_send(w, wave_depth, world);
                    long double intensity = std::norm(total.s) + std::norm(total.p);

                    CIE_entry cie = cie_lookup(wl);
                    X += intensity * cie.x;
                    Y += intensity * cie.y;
                    Z += intensity * cie.z;
                }   
            }

            X *= exp_samples;
            Y *= exp_samples;
            Z *= exp_samples;

            // std::cout << X << " " << Y << " " << Z << "\n";

            double r = 3.2406 * X - 1.5372 * Y - 0.4986 * Z;
            double g = -0.9689 * X + 1.8758 * Y + 0.0415 * Z;
            double b = 0.0557 * X - 0.2040 * Y + 1.0570 * Z;

            // std::cout << r << " " << g << " " << b << "\n";

            write_color(std::cout, color(r, g, b));
        }
    }
}


/*
Return when energy of wave is very low, does not contribute to calculation
Scatter waves upon hitting an object, one reflected one transmitted with their relative attributes
Return total sum of wave amplitudes
*/
polarized_amplitude wave_send(const wave&w, int depth, const hittable&world){
    if(depth <= 0) return polarized_amplitude(w.amplitude_s(), w.amplitude_p());
    hit_record rec;

    if(std::norm(w.amplitude_s()) + std::norm(w.amplitude_p()) < 0.00001) return polarized_amplitude(w.amplitude_s(), w.amplitude_p());

    if(world.hit(w, interval(1e-8, infinity), rec)){
        std::vector<wave> outgoing;
        rec.mat->scatter(w, rec, outgoing);

        polarized_amplitude sum(0, 0);
        for(const auto& wave_out : outgoing){
            sum += wave_send(wave_out, depth - 1, world);
        }
        return sum;
    }
    double bg_intensity = background_intensity(w.direction());
    double amp_scale = std::sqrt(bg_intensity);
    return polarized_amplitude(w.amplitude_s() * amp_scale, w.amplitude_p() * amp_scale);
}

double background_intensity(const vec3& dir){
    // vector aiming for the sun
    vec3 sun_dir = unit_vector(vec3(5, 10, 5));
    // how similar the vectors are
    double cos_angle = dot(sun_dir, dir);
    
    double sun_radius = 0.1;
    double sun_edge = cos(sun_radius);

    if(cos_angle > sun_edge){
        return 1.0;
    }

    // return std::fmax(((cos_angle + 1) / 2) / 4, 0.000000001);
    return 0.02;
}