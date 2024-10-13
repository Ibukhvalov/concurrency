#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h"
#include "triangle_mesh.h"
#include "material.h"

#include <vector>

class hittable_list : public hittable {
public:
    std::vector<shared_ptr<hittable>> objects;

    hittable_list() {}
    hittable_list(shared_ptr<hittable> object) { add(object); }

    void clear() { objects.clear(); }

    void add(shared_ptr<hittable> object) {
        objects.push_back(object);
    }

    void addPrimitive(const std::vector<Vertex>& vb, const std::vector<uint32_t>& ib)
    {
        //assert(vb.size() % 3 == 0);
        for (int i = 0; i < vb.size(); i+=3) {
            if (vb.size() - i < 3) return;
            add(make_shared<triangleMesh>(vb[i].pos, vb[i+1].pos, vb[i+2].pos, make_shared<lambertian>(color::random() * color::random())));
        }
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        hit_record temp_rec;
        bool hit_anything = false;
        auto closest_so_far = ray_t.max;

        for (const auto& object : objects) {
            if (object->hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hit_anything;
    }
};

#endif