#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <mpi.h>
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

int main(int argc, char* argv[])
{
    // Initialize MPI
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    double start_time = MPI_Wtime();
    
    const int width = 10000;
    const int height = 8000;
    
    double x_start = -2.0;
    double x_fin = 1.0;
    double y_start = -1.0;
    double y_fin = 1.0;
    
    double dx = (x_fin - x_start) / (width - 1);
    double dy = (y_fin - y_start) / (height - 1);
    
    // Calculate local size and offsets
    int rows_per_proc = height / size;
    int remainder = height % size;
    
    std::vector<int> sendcounts(size);
    std::vector<int> displs(size);
    
    // Calculate send counts and displacements for each process
    for (int i = 0; i < size; i++) {
        sendcounts[i] = (rows_per_proc + (i < remainder ? 1 : 0)) * width * 3;
        displs[i] = (i > 0 ? displs[i-1] + sendcounts[i-1] : 0);
    }
    
    // Local height for this process
    int local_height = rows_per_proc + (rank < remainder ? 1 : 0);
    int start_row = rank * rows_per_proc + std::min(rank, remainder);
    
    // Allocate memory only for local portion
    std::vector<unsigned char> local_image(local_height * width * 3);
    
    // Compute Mandelbrot set for local portion
    for (int i = 0; i < local_height; i++) {
        for (int j = 0; j < width; j++) {
            double x = x_start + j * dx;
            double y = y_fin - (start_row + i) * dy;
            
            int value = mandelbrot(x, y);
            
            // Define color mapping
            unsigned char r, g, b;
            if (value == 100) {
                r = g = b = 0; // Black
            } else if (value > 90) {
                r = 139; g = 0; b = 0;    // Dark Red
            } else if (value > 70) {
                r = 255; g = 0; b = 0;    // Bright Red
            } else if (value > 50) {
                r = 255; g = 165; b = 0;  // Orange
            } else if (value > 30) {
                r = 255; g = 255; b = 0;  // Yellow
            } else if (value > 20) {
                r = 0; g = 255; b = 0;    // Green
            } else if (value > 10) {
                r = 0; g = 255; b = 255;  // Cyan
            } else if (value > 5) {
                r = 0; g = 0; b = 255;    // Blue
            } else if (value > 3) {
                r = 128; g = 0; b = 128;  // Purple
            } else {
                r = 255; g = 105; b = 180; // Hot Pink
            }
            
            // Set pixel color in the local image
            local_image[(i * width + j) * 3] = r;
            local_image[(i * width + j) * 3 + 1] = g;
            local_image[(i * width + j) * 3 + 2] = b;
        }
    }
    
    // Allocate memory for the complete image only on rank 0
    std::vector<unsigned char> full_image;
    if (rank == 0) {
        full_image.resize(width * height * 3);
    }
    
    // Gather all parts of the image to rank 0
    MPI_Gatherv(local_image.data(), local_image.size(), MPI_UNSIGNED_CHAR,
                full_image.data(), sendcounts.data(), displs.data(),
                MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    
    // Write the image to a file (only rank 0)
    if (rank == 0) {
        stbi_write_png("mandelbrot_mpi.png", width, height, 3, full_image.data(), width * 3);
        
        double end_time = MPI_Wtime();
        std::cout << "Mandelbrot set image saved as mandelbrot_mpi.png" << std::endl;
        std::cout << "Computation took " << end_time - start_time << " seconds with " 
                  << size << " processes" << std::endl;
    }
    
    MPI_Finalize();
    return 0;
}