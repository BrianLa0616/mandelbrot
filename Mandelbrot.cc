#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <immintrin.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <time.h>

// SIMD-optimized Mandelbrot calculation for 4 points at once
void mandelbrot_simd(double* x_vals, double* y_vals, int* results) {
    const int limit = 100;
    __m256d x = _mm256_load_pd(x_vals);
    __m256d y = _mm256_load_pd(y_vals);
    __m256d x0 = x;
    __m256d y0 = y;
    
    __m256d four = _mm256_set1_pd(4.0);
    __m256d two = _mm256_set1_pd(2.0);
    __m256i counts = _mm256_setzero_si256();
    __m256i ones = _mm256_set1_epi64x(1);
    
    for (int iter = 0; iter < limit; iter++) {
        __m256d x2 = _mm256_mul_pd(x, x);
        __m256d y2 = _mm256_mul_pd(y, y);
        __m256d xy = _mm256_mul_pd(x, y);
        
        __m256d mag2 = _mm256_add_pd(x2, y2);
        
        // Check escaped points
        __m256d mask = _mm256_cmp_pd(mag2, four, _CMP_LE_OQ);
        __m256i mask_int = _mm256_castpd_si256(mask);
        
        // Break if all points have escaped
        if (_mm256_movemask_pd(mask) == 0) break;
        
        // Increment counts for points that haven't escaped
        counts = _mm256_sub_epi64(counts, mask_int);
        
        // Update x and y
        y = _mm256_add_pd(_mm256_mul_pd(two, xy), y0);
        x = _mm256_add_pd(_mm256_sub_pd(x2, y2), x0);
    }
    
    // Store results
    alignas(32) int64_t temp[4];
    _mm256_store_si256((__m256i*)temp, counts);
    for (int i = 0; i < 4; i++) {
        results[i] = std::min(static_cast<int>(temp[i]), limit);
    }
}

inline void map_color(int value, unsigned char& r, unsigned char& g, unsigned char& b) {
    if (value == 100) {
        r = g = b = 0;
    } else if (value > 90) {
        r = 139; g = 0; b = 0;
    } else if (value > 70) {
        r = 255; g = 0; b = 0;
    } else if (value > 50) {
        r = 255; g = 165; b = 0;
    } else if (value > 30) {
        r = 255; g = 255; b = 0;
    } else if (value > 20) {
        r = 0; g = 255; b = 0;
    } else if (value > 10) {
        r = 0; g = 255; b = 255;
    } else if (value > 5) {
        r = 0; g = 0; b = 255;
    } else if (value > 3) {
        r = 128; g = 0; b = 128;
    } else {
        r = 255; g = 105; b = 180;
    }
}

int main() {
    clock_t init_start = clock();
    
    const int width = 10000;
    const int height = 8000;
    const double x_start = -2.0;
    const double x_fin = 1.0;
    const double y_start = -1.0;
    const double y_fin = 1.0;
    
    const double dx = (x_fin - x_start) / (width - 1);
    const double dy = (y_fin - y_start) / (height - 1);
    
    // Allocate aligned memory for the image
    std::vector<unsigned char> image(width * height * 3);
    
    // Allocate aligned memory for SIMD operations
    alignas(32) double x_vals[4];
    alignas(32) double y_vals[4];
    alignas(32) int results[4];
    
    for (int i = 0; i < height; i++) {
        double y = y_fin - i * dy;
        
        for (int j = 0; j < width; j += 4) {
            // Prepare 4 x coordinates for SIMD processing
            for (int k = 0; k < 4; k++) {
                x_vals[k] = x_start + (j + k) * dx;
                y_vals[k] = y;
            }
            
            // Process 4 points at once using SIMD
            mandelbrot_simd(x_vals, y_vals, results);
            
            // Map colors and store results
            for (int k = 0; k < 4 && (j + k) < width; k++) {
                unsigned char r, g, b;
                map_color(results[k], r, g, b);
                
                image[(i * width + j + k) * 3] = r;
                image[(i * width + j + k) * 3 + 1] = g;
                image[(i * width + j + k) * 3 + 2] = b;
            }
        }
    }
    
    // Write the image to a PNG file
    stbi_write_png("mandelbrot.png", width, height, 3, image.data(), width * 3);
    
    clock_t init_end = clock();
    printf("Mandelbrot compute took: %f seconds\n", (double)(init_end - init_start) / CLOCKS_PER_SEC);
    
    return 0;
}