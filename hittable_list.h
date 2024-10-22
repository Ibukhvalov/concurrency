#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h"
#include "triangle_mesh.h"
#include "material.h"
#include "aabb.h"

#include <vector>

class hittable_list : public hittable {
public:
    std::vector<shared_ptr<hittable>> objects;

    hittable_list() {}
    hittable_list(shared_ptr<hittable> object) { add(object); }

    void clear() { objects.clear(); }

    void add(shared_ptr<hittable> object) {
        objects.push_back(object);
        bbox = aabb(bbox, object->bounding_box());
    }

    void addPrimitive(const std::vector<Vertex>& vb, const std::vector<uint32_t>& ib, const glm::float4x4& transform)
    {
        assert(ib.size() % 3 == 0);
        for (int i = 0; i < ib.size(); i+=3) {
            //if (ib.size() - i < 3) return;

            glm::vec4 expanded1(vb[ib[i]].pos[0], vb[ib[i]].pos[1], vb[ib[i]].pos[2], 1.0f);
            glm::vec4 expanded2(vb[ib[i+1]].pos[0], vb[ib[i+1]].pos[1], vb[ib[i+1]].pos[2], 1.0f);
            glm::vec4 expanded3(vb[ib[i+2]].pos[0], vb[ib[i+2]].pos[1], vb[ib[i+2]].pos[2], 1.0f);

            glm::vec4 final1 = transform * expanded1;
            glm::vec4 final2 = transform * expanded2;
            glm::vec4 final3 = transform * expanded3;

            vec3 p1(final1[0]/final1[3], final1[1]/final1[3], final1[2]/final1[3]);
            vec3 p2(final2[0]/final2[3], final2[1]/final2[3], final2[2]/final2[3]);
            vec3 p3(final3[0]/final3[3], final3[1]/final3[3], final3[2]/final3[3]);

            add(make_shared<triangleMesh>(p1,p2,p3,
                                          make_shared<lambertian>(color(0.3,0.3,0.3))));
            //color::random() * color::random() - for random color
            //color(0.3,0.3,0.3) - for gray color
        }
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        if (!bbox.hit(r, ray_t))
            return false;
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

    aabb bounding_box() const override { return bbox; }

private:
    aabb bbox;
};

#endif