#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <stdio.h>
#include "stb_image_write.h"

// Include STB library implementation
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// CUDA kernel for Mandelbrot set calculation
__device__ int mandelbrot(double real, double imag) {
    int limit = 10000;
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

// CUDA kernel for pixel calculation
__global__ void calculatePixels(unsigned char* image, int width, int height, 
                              double x_start, double x_fin, double y_start, double y_fin) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int idy = blockIdx.y * blockDim.y + threadIdx.y;

    if (idx < width && idy < height) {
        double dx = (x_fin - x_start) / (width - 1);
        double dy = (y_fin - y_start) / (height - 1);

        double x = x_start + idx * dx;
        double y = y_fin - idy * dy;

        int value = mandelbrot(x, y);

        // Color mapping
        unsigned char r, g, b;
        if (value == 100) {
            r = g = b = 0; // Black
        } else if (value > 90) {
            r = 139; g = 0; b = 0; // Dark Red
        } else if (value > 70) {
            r = 255; g = 0; b = 0; // Bright Red
        } else if (value > 50) {
            r = 255; g = 165; b = 0; // Orange
        } else if (value > 30) {
            r = 255; g = 255; b = 0; // Yellow
        } else if (value > 20) {
            r = 0; g = 255; b = 0; // Green
        } else if (value > 10) {
            r = 0; g = 255; b = 255; // Cyan
        } else if (value > 5) {
            r = 0; g = 0; b = 255; // Blue
        } else if (value > 3) {
            r = 128; g = 0; b = 128; // Purple
        } else {
            r = 255; g = 105; b = 180; // Hot Pink
        }

        // Set pixel color in the image
        int pixelIndex = (idy * width + idx) * 3;
        image[pixelIndex] = r;
        image[pixelIndex + 1] = g;
        image[pixelIndex + 2] = b;
    }
}

int main() {
    const int width = 1000;
    const int height = 800;
    const size_t imageSize = width * height * 3 * sizeof(unsigned char);

    // Host memory
    unsigned char* h_image = (unsigned char*)malloc(imageSize);

    // Device memory
    unsigned char* d_image;
    cudaMalloc(&d_image, imageSize);

    // Set computation parameters
    double x_start = -2.0;
    double x_fin = 1.0;
    double y_start = -1.0;
    double y_fin = 1.0;

    // Define grid and block dimensions
    dim3 blockSize(16, 16);
    dim3 gridSize((width + blockSize.x - 1) / blockSize.x,
                  (height + blockSize.y - 1) / blockSize.y);

    // Launch kernel
    calculatePixels<<<gridSize, blockSize>>>(d_image, width, height,
                                           x_start, x_fin, y_start, y_fin);

    // Check for kernel launch errors
    cudaError_t cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "Kernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
        return 1;
    }

    // Copy result back to host
    cudaMemcpy(h_image, d_image, imageSize, cudaMemcpyDeviceToHost);

    // Write image to file
    stbi_write_png("mandelbrot_cuda.png", width, height, 3, h_image, width * 3);
    printf("Mandelbrot set image saved as mandelbrot_cuda.png\n");

    // Cleanup
    cudaFree(d_image);
    free(h_image);

    return 0;
}