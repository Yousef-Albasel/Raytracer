#ifndef CAMERA_H
#define CAMERA_H
#include "hittable.h"
#include "color.h"
#include "material.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <thread>
#include <vector>
#include <algorithm>
class camera {
public:
    double aspect_ratio = 16.0 / 9.0; // Default aspect ratio
    int image_width = 400;            // Default image width
    int samples_per_pixel = 10;
    int max_depth = 10;
    double vfov = 90;

    point3 lookfrom = point3(0,0,0);   // Point camera is looking from
    point3 lookat = point3(0, 0, -1);  // Point camera is looking at
    vec3   vup = vec3(0, 1, 0);

    // Constructor
    camera() {
        initialize();
    }
    ray get_ray(int i, int j) const {

        auto offset = sample_square();
        auto pixel_sample = pixel00_loc
            + ((i + offset.x()) * pixel_delta_u)
            + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = camera_center;
        auto ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }

    vec3 sample_square() const {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }
    // Render the scene
    void render(const hittable& world) {
        int num_threads = std::thread::hardware_concurrency(); 
        std::vector<std::thread> threads;
        int rows_per_thread = image_height / num_threads;

        auto render_chunk = [&](int start_row, int end_row) {
            for (int j = start_row; j < end_row; j++) {
                std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;

                for (int i = 0; i < image_width; i++) {
                    color pixel_color(0, 0, 0);
                    for (int sample = 0; sample < samples_per_pixel; sample++) {
                        ray r = get_ray(i, j);
                        pixel_color += ray_color(r, max_depth, world);
                    }
                    pixel_color *= (1.0 / samples_per_pixel);
                    write_color(i, j, pixel_color);
                }
            }
            };

        for (int t = 0; t < num_threads; t++) {
            int start_row = t * rows_per_thread;
            int end_row = (t == num_threads - 1) ? image_height : start_row + rows_per_thread;
            threads.emplace_back(render_chunk, start_row, end_row);
        }

        for (auto& thread : threads) {
            thread.join();
        }

        stbi_write_png("output.png", image_width, image_height, 3, image_buffer.data(), image_width * 3);
    }    const unsigned char* getImageData() const {
        return image_buffer.data();
    }

    int getWidth() const { return image_width; }
    int getHeight() const { return image_height; }
    void set_camera_position(const point3& new_lookfrom) {
        lookfrom = new_lookfrom;
        initialize();  // Recalculate everything
    }


private:
    int image_height;               // Calculated based on aspect ratio
    vec3 pixel_delta_u;             // Offset to pixel to the right
    vec3 pixel_delta_v;             // Offset to pixel below
    point3 pixel00_loc;             // Location of pixel 0, 0
    point3 camera_center; // Initialize camera center
    std::vector<unsigned char> image_buffer; // Image buffer
    vec3   u, v, w;
    // Initialize camera settings
    void initialize() {
        // Calculate image height based on aspect ratio
        image_height = static_cast<int>(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        // Resize image buffer
        image_buffer.resize(image_width * image_height * 3);

        camera_center = lookfrom;
        // Camera setup
        auto focal_length = (lookfrom - lookat).length();
        // Determine viewport dimensions.
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta / 2);
        auto viewport_height = 2 * h * focal_length;
        auto viewport_width = viewport_height * (double(image_width) / image_height);
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);
        // Define viewport vectors
        vec3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
        vec3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge


        // Define delta vectors (distance between pixels)
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Define upper-left point of the viewport
        auto viewport_upper_left = camera_center - (focal_length * w) - viewport_u / 2 - viewport_v / 2;

        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v); // Center of pixel 00
    }

    // Compute the color of a ray
    color ray_color(const ray& r, int depth, const hittable& world) const {
        if (depth <= 0)
            return color(0, 0, 0);
        hit_record rec;

        if (world.hit(r, interval(0.001, infinity), rec)) {
            ray scattered;
            color attenuation;
            if (rec.mat->scatter(r, rec, attenuation, scattered))
                return attenuation * ray_color(scattered, depth - 1, world);
            return color(0, 0, 0);
        }

        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
    }
    // Write color to the image buffer
    inline double linear_to_gamma(double linear_component)
    {
        if (linear_component > 0)
            return std::sqrt(linear_component);

        return 0;
    }

    void write_color(int x, int y, color& pixel_color) {
        auto r = pixel_color.x();
        auto g = pixel_color.y();
        auto b = pixel_color.z();
        r = linear_to_gamma(r);
        g = linear_to_gamma(g);
        b = linear_to_gamma(b);
        static const interval intensity(0.000, 0.999);
        int rbyte = int(256 * intensity.clamp(r));
        int gbyte = int(256 * intensity.clamp(g));
        int bbyte = int(256 * intensity.clamp(b));
        //std::cout << rbyte << " : " << gbyte << " : " << bbyte << std::endl;
        int index = (y * image_width + x) * 3;
        image_buffer[index] = rbyte;
        image_buffer[index + 1] = gbyte;
        image_buffer[index + 2] = bbyte;
    }
};

#endif