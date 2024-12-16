#include "rtweekend.h"

#include "gltfLoader.h"
#include "hittable.h"
#include "camera.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "triangle_mesh.h"
#include "bvh.h"
#include "concurrency/Renderrer.h"


int main() {
    hittable_list world;

    if (!GltfLoader::loadGltf("abobaText.gltf", world)) std::cerr << "MODEL LOADING ERROR\n";



    world = hittable_list(make_shared<bvh_node>(world));

    camera cam;

    cam.aspect_ratio      = 9.0 / 9.0;
    cam.image_width       = 200;
    cam.samples_per_pixel = 80;
    cam.max_depth         = 16;

    cam.vfov     = 30;
    cam.lookfrom = point3(4.5,2,4.5);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.0;
    cam.focus_dist    = 10.0;

    cam.build();



    Renderer renderer(5);
    renderer.produceImage("../data/test1.jpg", cam.image_width, cam.image_height, &world, &cam);

    camera cam2(cam);
    cam.vfov = 45;
    cam2.build();

    renderer.produceImage("../data/test2.jpg", cam.image_width, cam.image_height, &world, &cam2);
}