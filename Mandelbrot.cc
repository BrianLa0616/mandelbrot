#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <time.h>

int mandelbrot(double real, double imag)
{
    int limit = 100;
    double zReal = real;
    double zImag = imag;

    for (int i = 0; i < limit; ++i) {
        double r2 = zReal * zReal;
        double i2 = zImag * zImag;

        if (r2 + i2 > 4.0)
            return i;

        zImag = 2.0 * zReal * zImag + imag;
        zReal = r2 - i2 + real;
    }
    return limit;
}

int main()
{
    clock_t init_start = clock();

    int width = 10000; // Increased resolution for better PNG quality
    int height = 8000;

    double x_start = -2.0;
    double x_fin = 1.0;
    double y_start = -1.0;
    double y_fin = 1.0;

    double dx = (x_fin - x_start) / (width - 1);
    double dy = (y_fin - y_start) / (height - 1);

    // Allocate memory for the image
    std::vector<unsigned char> image(width * height * 3);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            double x = x_start + j * dx;
            double y = y_fin - i * dy;

            int value = mandelbrot(x, y);

            // Define color mapping
            unsigned char r, g, b;
            if (value == 100) {
                r = g = b = 0; // Black
            } else if (value > 90) {
                r = 139;
                g = 0;
                b = 0; // Dark Red
            } else if (value > 70) {
                r = 255;
                g = 0;
                b = 0; // Bright Red
            } else if (value > 50) {
                r = 255;
                g = 165;
                b = 0; // Orange
            } else if (value > 30) {
                r = 255;
                g = 255;
                b = 0; // Yellow
            } else if (value > 20) {
                r = 0;
                g = 255;
                b = 0; // Green
            } else if (value > 10) {
                r = 0;
                g = 255;
                b = 255; // Cyan
            } else if (value > 5) {
                r = 0;
                g = 0;
                b = 255; // Blue
            } else if (value > 3) {
                r = 128;
                g = 0;
                b = 128; // Purple
            } else {
                r = 255;
                g = 105;
                b = 180; // Hot Pink
            }

            // Set pixel color in the image
            image[(i * width + j) * 3] = r;
            image[(i * width + j) * 3 + 1] = g;
            image[(i * width + j) * 3 + 2] = b;
        }
    }

    // Write the image to a PNG file
    stbi_write_png("mandelbrot.png", width, height, 3, image.data(), width * 3);

    std::cout << "Mandelbrot set image saved as mandelbrot.png" << std::endl;

    clock_t init_end = clock();
    
    printf("Mandelbrot compute took: %f seconds\n", (double)(init_end - init_start) / CLOCKS_PER_SEC);

    return 0;
}