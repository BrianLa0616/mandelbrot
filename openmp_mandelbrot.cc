#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <omp.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void compute_mandelbrot_chunk(const double x_start, const double y_start, 
                            const double dx, const double dy,
                            const int start_i, const int chunk_size,
                            const int width, const int height,
                            std::vector<unsigned char>& image) {
    constexpr int limit = 10000;
    
    #pragma omp simd
    for (int idx = 0; idx < chunk_size; ++idx) {
        int i = (start_i + idx) / width;
        int j = (start_i + idx) % width;
        
        if (i >= height) continue;
        
        double x = x_start + j * dx;
        double y = y_start + i * dy;
        
        double q = (x - 0.25) * (x - 0.25) + y * y;
        if (q * (q + (x - 0.25)) <= 0.25 * y * y || 
            (x + 1.0) * (x + 1.0) + y * y <= 0.0625) {
            const size_t pixel_idx = (i * width + j) * 3;
            image[pixel_idx] = image[pixel_idx + 1] = image[pixel_idx + 2] = 0;
            continue;
        }
        
        double zReal = 0;
        double zImag = 0;
        double zReal2 = 0;
        double zImag2 = 0;
        int iter = 0;
        
        while (iter < limit && zReal2 + zImag2 <= 4.0) {
            zImag = 2.0 * zReal * zImag + y;
            zReal = zReal2 - zImag2 + x;
            zReal2 = zReal * zReal;
            zImag2 = zImag * zImag;
            ++iter;
        }
        
        static const unsigned char colors[][3] = {
            {255, 105, 180}, // Hot Pink
            {128, 0, 128},   // Purple
            {0, 0, 255},     // Blue
            {0, 255, 255},   // Cyan
            {0, 255, 0},     // Green
            {255, 255, 0},   // Yellow
            {255, 165, 0},   // Orange
            {255, 0, 0},     // Bright Red
            {139, 0, 0},     // Dark Red
            {0, 0, 0}        // Black
        };
        
        int color_idx;
        if (iter == limit) color_idx = 9;
        else if (iter > 90) color_idx = 8;
        else if (iter > 70) color_idx = 7;
        else if (iter > 50) color_idx = 6;
        else if (iter > 30) color_idx = 5;
        else if (iter > 20) color_idx = 4;
        else if (iter > 10) color_idx = 3;
        else if (iter > 5)  color_idx = 2;
        else if (iter > 3)  color_idx = 1;
        else color_idx = 0;
        
        const size_t pixel_idx = (i * width + j) * 3;
        image[pixel_idx] = colors[color_idx][0];
        image[pixel_idx + 1] = colors[color_idx][1];
        image[pixel_idx + 2] = colors[color_idx][2];
    }
}

int main() {
    omp_set_nested(1);
    omp_set_max_active_levels(2);
    
    const int width = 24000;
    const int height = 24000;
    const double x_start = -2.0;
    const double x_fin = 1.0;
    const double y_start = -1.0;
    const double y_fin = 1.0;
    
    const double dx = (x_fin - x_start) / (width - 1);
    const double dy = (y_fin - y_start) / (height - 1);
    
    const int total_pixels = width * height;
    const int chunk_size = 256; 
    const int num_chunks = (total_pixels + chunk_size - 1) / chunk_size;
    
    std::vector<unsigned char> image(width * height * 3);
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < image.size(); ++i) {
        image[i] = 0;
    }
    
    double start_time = omp_get_wtime();
    
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            for (int chunk = 0; chunk < num_chunks; ++chunk) {
                const int start_pixel = chunk * chunk_size;
                #pragma omp task
                {
                    compute_mandelbrot_chunk(x_start, y_start, dx, dy,
                                           start_pixel, chunk_size,
                                           width, height, image);
                }
            }
        }
    }
    
    stbi_write_png("mandelbrot.png", width, height, 3, image.data(), width * 3);
    
    double end_time = omp_get_wtime();
    printf("Computation time: %.3f seconds\n", end_time - start_time);
    
    return 0;
}
