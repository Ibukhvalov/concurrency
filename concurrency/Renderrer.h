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


class Renderer {
public:
    Renderer(int numConsumers, int width, int height, hittable* world, camera* camera) : doneProducing(false),
                                                                                         width(width), height(height), pixels(new uint8_t[width * height * 3]),
                                                                                         world(world), camera(camera){
        for (int i = 0; i < numConsumers; ++i) {
            consumers.emplace_back(&Renderer::consume, this);
        }
    }



    void produce() {
        for (int j = 0; j < height; ++j) {
            for (int i = 0; i < width; ++i) {
                std::unique_lock<std::mutex> lock(mtx);
                pixelQueue.push({i, j});
                lock.unlock();
                cv.notify_one();
            }
        }

        std::unique_lock<std::mutex> lock(mtx);
        doneProducing = true;
        lock.unlock();
        cv.notify_all();

        for (auto& consumer : consumers) {
            consumer.join();
        }

        stbi_write_jpg("result.jpg", width, height, 3, pixels, 100);
        delete[] pixels;
    }

private:
    int width, height;
    uint8_t* pixels;
    hittable* world;
    camera* camera;
    std::queue<Pixel> pixelQueue;
    std::mutex mtx;
    std::condition_variable cv;
    bool doneProducing;
    std::vector<std::thread> consumers;

    void consume() {
        bool processing = true;
        while (processing) {
            Pixel pixel;
            bool picked = false;
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this] { return !pixelQueue.empty() || doneProducing; });
                if (pixelQueue.empty() && doneProducing) {
                    processing = false;
                }

                if (!pixelQueue.empty()) {
                    pixel = pixelQueue.front();
                    picked = true;
                    pixelQueue.pop();
                }
            }

            if (picked) {
                color color = camera->get_pixel_color(pixel.i, pixel.j, world);

                int index = pixel.j*width+pixel.i;
                fillPixel(pixels, index, color);
            }
        }
    }
};


#endif //PPM_IMAGE_RENDERRER_H
