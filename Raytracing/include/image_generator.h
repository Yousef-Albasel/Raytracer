#pragma once
#ifndef IMAGE_GENERATOR_H
#define IMAGE_GENERATOR_H

#include "color.h"
#include <vector>

class ImageGenerator {
public:
    ImageGenerator(int width, int height) : image_width(width), image_height(height) {
        image_data.resize(image_width * image_height * 3);
    }

    void generate_image() {
        for (int j = 0; j < image_height; ++j) {
            for (int i = 0; i < image_width; ++i) {
                auto r = double(i) / (image_width - 1);
                auto g = double(j) / (image_height - 1);
                auto b = 0.0;

                color pixel_color(r, g, b);
                set_pixel(i, j, pixel_color);
            }
        }
    }

    void set_pixel(int x, int y,  color& pixel_color) {
        int ir = int(255.999 * pixel_color.x());
        int ig = int(255.999 * pixel_color.y());
        int ib = int(255.999 * pixel_color.z());

        image_data[(y * image_width + x) * 3 + 0] = ir; // Red
        image_data[(y * image_width + x) * 3 + 1] = ig; // Green
        image_data[(y * image_width + x) * 3 + 2] = ib; // Blue
    }

    const std::vector<unsigned char>& get_image_data() const {
        return image_data;
    }

    int get_width() const { return image_width; }
    int get_height() const { return image_height; }

private:
    int image_width;
    int image_height;
    std::vector<unsigned char> image_data;
};

#endif