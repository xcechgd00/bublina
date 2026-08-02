#include "bubbles.h"
#include "hittable_list.h"

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

struct CIE_entry { double x, y, z; };

const CIE_entry cie_table[31] = {
    {0.1344, 0.0040, 0.6456}, // 400
    {0.2839, 0.0116, 1.3856}, // 410
    {0.3483, 0.0230, 1.7471}, // 420
    {0.3362, 0.0380, 1.7721}, // 430
    {0.2908, 0.0600, 1.6692}, // 440
    {0.1954, 0.0910, 1.2876}, // 450
    {0.0956, 0.1390, 0.8130}, // 460
    {0.0320, 0.2080, 0.4652}, // 470
    {0.0147, 0.3230, 0.2720}, // 480
    {0.0049, 0.5030, 0.1582}, // 490
    {0.0093, 0.7100, 0.0782}, // 500
    {0.0633, 0.8620, 0.0422}, // 510
    {0.1655, 0.9540, 0.0203}, // 520
    {0.2904, 0.9950, 0.0087}, // 530
    {0.4334, 0.9950, 0.0039}, // 540
    {0.5945, 0.9520, 0.0021}, // 550
    {0.7621, 0.8700, 0.0017}, // 560
    {0.9163, 0.7570, 0.0011}, // 570
    {1.0263, 0.6310, 0.0008}, // 580
    {1.0622, 0.5030, 0.0003}, // 590
    {1.0026, 0.3810, 0.0002}, // 600
    {0.8544, 0.2650, 0.0000}, // 610
    {0.6424, 0.1750, 0.0000}, // 620
    {0.4479, 0.1070, 0.0000}, // 630
    {0.2835, 0.0610, 0.0000}, // 640
    {0.1649, 0.0320, 0.0000}, // 650
    {0.0874, 0.0170, 0.0000}, // 660
    {0.0468, 0.0082, 0.0000}, // 670
    {0.0227, 0.0041, 0.0000}, // 680
    {0.0114, 0.0021, 0.0000}, // 690
    {0.0058, 0.0010, 0.0000}  // 700
};

CIE_entry cie_lookup(int wl){
    int idx = (wl - 400) / 10;
    return cie_table[idx]; 
}

int main(void){
    double aspect_ratio = 16.0/9.0;
    int image_width = 800;
    int image_height = int(image_width/aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;

    auto focal_length = 1000000000.0;
    auto viewport_height = 2000000000.0;
    auto viewport_width = viewport_height * (double(image_width) / image_height);

    auto camera_center = point3(0, 0, 0);

    auto viewport_u = vec3(viewport_width, 0, 0);
    auto viewport_v = vec3(0, -viewport_height, 0);

    auto pixel_delta_u = viewport_u / image_width;
    auto pixel_delta_v = viewport_v / image_height;

    auto viewport_upper_left = vec3(-viewport_width / 2, viewport_height / 2, -focal_length);
    auto pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

    std::cout << "P3\n" << image_width << " " << image_height << "\n255\n";

    hittable_list world;

    auto thickness = 500;

    auto material = make_shared<dielectric>(1.33);
    world.add(make_shared<sphere>(point3(0, 0, -1000000000), 500000000, material));

    auto material_inside = make_shared<dielectric>(1.0);
    world.add(make_shared<sphere>(point3(0, 0, -1000000000), 500000000 - thickness, material_inside));

    for(int j = 0; j < image_height; j++){
        for(int i = 0; i < image_width; i++){
            auto pixel_center = pixel00_loc + ((i * pixel_delta_u) + vec3(random_double(-0.5, 0.5), 0, 0)) + ((j * pixel_delta_v) + vec3(0, random_double(-0.5, 0.5), 0));
            auto wave_direction = unit_vector(pixel_center - camera_center);

            double X = 0, Y = 0, Z = 0;

            for(int wl = 400; wl <= 700; wl += 10){
                std::vector<wave> waves;
                auto c = 1.0 + 0.0i;
                wave w(camera_center, wave_direction, wl, 1.0 / sqrt(2.0), 1.0 / sqrt(2.0), 1.0);

                polarized_amplitude total = wave_send(w, 10, world);
                double intensity = std::norm(total.s) + std::norm(total.p);

                CIE_entry cie = cie_lookup(wl);
                X += intensity * cie.x;
                Y += intensity * cie.y;
                Z += intensity * cie.z;
            }   

            double N = 0;

            for(int wl = 400; wl <= 700; wl += 10){
                N += cie_lookup(wl).y;
            }
            X /= N;
            Y /= N;
            Z /= N;

            double r = 3.2406 * X - 1.5372 * Y - 0.4986 * Z;
            double g = -0.9689 * X + 1.8758 * Y + 0.0415 * Z;
            double b = 0.0557 * X - 0.2040 * Y + 1.0570 * Z;

            // std::cout << r << " " << g << " " << b << "\n";

            // write_color(std::cout, color(r, g, b));
        }
    }
}

polarized_amplitude wave_send(const wave&w, int depth, const hittable&world){
    if(depth <= 0) return polarized_amplitude(0.0, 0.0);
    hit_record rec;

    if(world.hit(w, interval(0.0000000000001, infinity), rec)){
        std::vector<wave> outgoing;
        rec.mat->scatter(w, rec, outgoing);

        polarized_amplitude sum(0, 0);
        for(const auto& wave_out : outgoing){
            sum += wave_send(wave_out, depth - 1, world);
        }
        // std::cout << sum;

        return sum;
    }
    return polarized_amplitude(w.amplitude_s(), w.amplitude_p());
    // sky hit
}