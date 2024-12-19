#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#ifdef MPI
#include <mpi.h>
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "mpi.hpp"

int width, height;
double x_start, x_fin, y_start, y_fin;
double dx, dy;
std::vector<unsigned char> full_image;
std::vector<unsigned char> local_image;
std::vector<int> sendcounts;
std::vector<int> displs;
int rank, size;

int mandelbrot(double real, double imag) {
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

void init(int width_, int height_, int rank_, int size_) {
    width = width_;
    height = height_;
    rank = rank_;
    size = size_;
    
    x_start = -2.0;
    x_fin = 1.0;
    y_start = -1.0;
    y_fin = 1.0;
    
    dx = (x_fin - x_start) / (width - 1);
    dy = (y_fin - y_start) / (height - 1);
    
    // Calculate distribution of work
    int rows_per_proc = height / size;
    int remainder = height % size;
    
    sendcounts.resize(size);
    displs.resize(size);
    
    // Send counts & displacements
    for (int i = 0; i < size; i++) {
        sendcounts[i] = (rows_per_proc + (i < remainder ? 1 : 0)) * width * 3;
        displs[i] = (i > 0 ? displs[i-1] + sendcounts[i-1] : 0);
    }
    
    // Allocate memory for local portion
    int local_height = rows_per_proc + (rank < remainder ? 1 : 0);
    local_image.resize(local_height * width * 3);
    
    if (rank == 0) {
        full_image.resize(width * height * 3);
    }
}

void compute() {
    int rows_per_proc = height / size;
    int remainder = height % size;
    int local_height = rows_per_proc + (rank < remainder ? 1 : 0);
    int start_row = rank * rows_per_proc + std::min(rank, remainder);
    
    for (int i = 0; i < local_height; i++) {
        for (int j = 0; j < width; j++) {
            double x = x_start + j * dx;
            double y = y_fin - (start_row + i) * dy;
            
            int value = mandelbrot(x, y);
            
            // Define color mapping
            unsigned char r, g, b;
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
            
            local_image[(i * width + j) * 3] = r;
            local_image[(i * width + j) * 3 + 1] = g;
            local_image[(i * width + j) * 3 + 2] = b;
        }
    }
}

void gather_results() {
    MPI_Gatherv(local_image.data(), local_image.size(), MPI_UNSIGNED_CHAR,
                full_image.data(), sendcounts.data(), displs.data(),
                MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
}

void save_image(const char* filename) {
    if (rank == 0) {
        stbi_write_png(filename, width, height, 3, full_image.data(), width * 3);
    }
}