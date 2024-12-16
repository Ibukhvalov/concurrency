#ifndef PPM_IMAGE_RENDERIMAGETASK_H
#define PPM_IMAGE_RENDERIMAGETASK_H

#include <iostream>
#include <queue>
#include "../camera.h"
#include "../color.h"
#include "../stb_image_write.h"

struct Pixel {
    int i;
    int j;
};


struct RenderContext {
    hittable* world;
    camera* camera;
    int width;
    int height;
};

struct RenderImageTask {
    RenderImageTask(const char* path, int width, int height, hittable* world, camera* camera) {
        std::clog << "Added image render task to save at: " << path << std::endl;

        ctx = RenderContext{world, camera, width, height};
        imagePath = path;
        pixels = new uint8_t[width * height * 3];

        for (int j = 0; j < height; ++j)
            for (int i = 0; i < width; ++i)
                pixelsToProduce.push(new Pixel{i, j});


    }

    ~RenderImageTask() {
        delete[] pixels;
    }

    void writeImage() const {
        std::clog << "Image saved at: " << imagePath << std::endl;
        stbi_write_jpg(imagePath , ctx.width, ctx.height, 3, pixels, 100);
    }

    RenderContext ctx{};
    std::queue<Pixel*> pixelsToProduce;
    uint8_t* pixels{};
    const char* imagePath;
};

#endif //PPM_IMAGE_RENDERIMAGETASK_H
