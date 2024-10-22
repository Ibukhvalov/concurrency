#include "rtweekend.h"

#include "gltfLoader.h"
#include "hittable.h"
#include "camera.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "triangle_mesh.h"
#include "bvh.h"


int main() {
    hittable_list world;

    //triangle.gltf //2triangles_rotatedZ //2triangles_transformed
    //C:/Users/Anton/Downloads/Telegram Desktop/vespa/vespa/vespa
    if (!GltfLoader::loadGltf("aboba.gltf", world)) std::cerr << "MODEL LOADING ERROR\n";

    /*auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());
            point3 t1(a + 0.9*random_double(), 0.4*random_double(), b + 0.9*random_double());
            point3 t2(a + 0.9*random_double(), 0.4*random_double(), b + 0.9*random_double());
            point3 t3(a + 0.9*random_double(), 0.4*random_double(), b + 0.9*random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<triangleMesh>(t1, t2, t3, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<triangleMesh>(t1, t2, t3, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<triangleMesh>(t1, t2, t3, sphere_material));
                }
            }
        }
    }*/

    //auto material1 = make_shared<dielectric>(1.5);
    //auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    //auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    /*point3 p1{-1,1,-1};
    point3 p2{1,-1,0};
    point3 p3{0,0,1};

    world.add(make_shared<triangleMesh>(point3(4, 1, 0)+p1,
                                        point3(4, 1, 0)+p2,
                                        point3(4, 1, 0)+p3,
                                        make_shared<lambertian>(color(0.1, 0.2, 0.7))));

    world.add(make_shared<triangleMesh>(point3(-4, 1, 0)+p1,
                                        point3(-4, 1, 0)+p2,
                                        point3(-4, 1, 0)+p3,
                                        make_shared<lambertian>(color(0.7, 0.2, 0.1))));

    world.add(make_shared<triangleMesh>(point3(0, 1, 0)+p1,
                                        point3(0, 1, 0)+p2,
                                        point3(0, 1, 0)+p3,
                                        make_shared<lambertian>(color(0.2, 0.7, 0.1))));

    world.add(make_shared<sphere>(point3(4, 1, 0), 1,
                                        make_shared<lambertian>(color(0.1, 0.2, 0.7))));

    world.add(make_shared<sphere>(point3(-4, 1, 0), 1,
                                        make_shared<lambertian>(color(0.7, 0.2, 0.1))));

    world.add(make_shared<sphere>(point3(0, 1, 0), 1,
                                        make_shared<lambertian>(color(0.2, 0.7, 0.1))));*/

    world = hittable_list(make_shared<bvh_node>(world));

    camera cam;

    cam.aspect_ratio      = 9.0 / 9.0;
    cam.image_width       = 500;
    cam.samples_per_pixel = 80;
    cam.max_depth         = 16;

    cam.vfov     = 30;
    cam.lookfrom = point3(4.5,2,4.5);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.0;
    cam.focus_dist    = 10.0;

    cam.render(world);
}