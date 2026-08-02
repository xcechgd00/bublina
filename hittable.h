#ifndef HITTABLE_H
#define HITTABLE_H

#include "bubbles.h"

class material;

class hit_record{
    public:
        point3 p;
        vec3 normal;
        shared_ptr<material> mat;
        double t;
        bool front_face;
        double distance;

        void set_face_normal(const wave&w, const vec3& outward_normal){
            front_face = dot(w.direction(), outward_normal) < 0;
            normal = front_face ? outward_normal : -outward_normal;
        }
};

class hittable{
    public:
        virtual ~hittable() = default;

        virtual bool hit(const wave&w, interval wave_t, hit_record& rec) const = 0;
};

#endif