#ifndef TriangleMesh_H
#define TriangleMesh_H

#include "hittable.h"

class triangleMesh : public hittable {
public:
    triangleMesh(const point3 &a, const point3 &b, const point3 &c, shared_ptr<material> mat)
            : a(a), b(b), c(c), mat(mat) {}

    bool hit(const ray &r, interval ray_t, hit_record &rec) const override {
        constexpr float epsilon = std::numeric_limits<float>::epsilon();

        vec3 edge1 = b - a;
        vec3 edge2 = c - a;
        vec3 ray_cross_e2 = cross(r.direction(), edge2);
        float det = dot(edge1, ray_cross_e2);

        if (det > -epsilon && det < epsilon)
            return false;    // This ray is parallel to this triangle.

        float inv_det = 1.0 / det;
        vec3 s = r.origin() - a;
        float u = inv_det * dot(s, ray_cross_e2);

        if (u < 0 || u > 1)
            return false;

        vec3 s_cross_e1 = cross(s, edge1);
        float v = inv_det * dot(r.direction(), s_cross_e1);

        if (v < 0 || u + v > 1)
            return false;

        // At this stage we can compute t to find out where the intersection point is on the line.
        float t = inv_det * dot(edge2, s_cross_e1);

        if (t > epsilon) // ray intersection
        {
            rec.t = t;
            rec.p = vec3(r.origin() + r.direction() * t);
            vec3 normal = unit_vector(cross(a - b, b - c));
            rec.set_face_normal(r, normal);;
            rec.mat = mat;

            return true;
        } else // This means that there is a line intersection but not a ray intersection.
            return false;
    }

private:
    point3 a, b, c; // 3 vertices
    shared_ptr<material> mat;

};

#endif