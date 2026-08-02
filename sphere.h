#ifndef SPHERE_H
#define SPHERE_H

#include "bubbles.h"

class sphere : public hittable{
    public:
        // sphere definition
        sphere(const point3& center, double radius, shared_ptr<material> mat) : center(center), radius(std::fmax(0, radius)), mat(mat) {}

        bool hit(const wave& w, interval wave_t, hit_record& rec) const override {
            // camera to center
            // rest is mathematically upravene so that it is easy to calculate
            vec3 oc = center - w.origin();
            auto a = w.direction().length_squared();
            auto h = dot(w.direction(), oc);
            auto c = dot(oc, oc) - radius * radius;

            auto discriminant = h * h - a * c;
            if(discriminant < 0) return false;

            auto sqrtd = std::sqrt(discriminant);

            auto root = ((h - sqrtd) / a);
            if(!wave_t.surrounds(root)){
                root = ((h + sqrtd) / a);
                if(!wave_t.surrounds(root)){
                    return false;
                }
            }
            
            rec.t = root;
            rec.p = w.at(root);
            rec.distance = (rec.p - w.origin()).length();
            // delenie polomerom normalizuje vektor
            vec3 outward_normal = (rec.p - center) / radius;
            rec.set_face_normal(w, outward_normal);
            rec.mat = mat;

            return true;
        }
    
    private:
        point3 center;
        double radius;
        shared_ptr<material> mat;
};

#endif