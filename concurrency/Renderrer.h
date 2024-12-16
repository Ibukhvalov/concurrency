#ifndef PPM_IMAGE_RENDERRER_H
#define PPM_IMAGE_RENDERRER_H


#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include "../ray.h"
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
        stbi_write_jpg(imagePath , ctx.width, ctx.height, 3, pixels, 100);
    }

    RenderContext ctx{};
    std::queue<Pixel*> pixelsToProduce;
    uint8_t* pixels{};
    const char* imagePath;
};


class Renderer {
public:
    Renderer(int numConsumers): isEnding(false) {
        for (int i = 0; i < numConsumers; ++i) {
            consumers.emplace_back(&Renderer::consume, this);
        }
    }

    ~Renderer() {
        isEnding = true;
        cv.notify_all();

        for (auto& cons : consumers) {
            if (cons.joinable()) {
                cons.join();
            }
        }
    }



    void produceImage(const char* path, int height, int width, hittable* world, camera* camera) {
        std::unique_lock<std::mutex> lock(mtx);
        tasks.push(new RenderImageTask(path, width, height, world, camera));
        cv.notify_all();


    }

private:
    std::queue<RenderImageTask*> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    bool isEnding;
    std::vector<std::thread> consumers;

    void consume() {
        while (true) {
            RenderImageTask* pickedTask;
            Pixel* pickedPixel;

            bool picked = false;
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this] { return !tasks.empty() || isEnding; });
                if(tasks.empty() && isEnding) break;
                if (!tasks.empty()) {
                    pickedTask = tasks.front();

                    if(pickedTask->pixelsToProduce.empty()) {
                        tasks.pop();
                        pickedTask->writeImage();
                        delete pickedTask;
                    } else {
                        pickedPixel = (pickedTask->pixelsToProduce.front());
                        pickedTask->pixelsToProduce.pop();
                        picked = true;
                    }
                }
            }

            if (picked) {
                color color = pickedTask->ctx.camera->get_pixel_color(pickedPixel->i, pickedPixel->j, pickedTask->ctx.world);
                int index = pickedPixel->j*pickedTask->ctx.width+pickedPixel->i;
                fillPixel(pickedTask->pixels, index, color);
            }
        }
    }
};


#endif //PPM_IMAGE_RENDERRER_H
