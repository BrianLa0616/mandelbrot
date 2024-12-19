#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

#ifdef MPI
#include <mpi.h>
#endif

#include "mpi.hpp"

int main(int argc, char** argv) {
    // Initialize MPI
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int width = 10000;
    int height = 8000;
    std::string output_file = "mandelbrot_mpi.png";
    
    for (int i = 1; i < argc; i += 2) {
        std::string arg = argv[i];
        if (i + 1 >= argc) {
            if (rank == 0) {
                std::cerr << "Missing value for argument " << arg << std::endl;
            }
            MPI_Finalize();
            return 1;
        }
        
        if (arg == "--width") {
            width = std::stoi(argv[i + 1]);
        } else if (arg == "--height") {
            height = std::stoi(argv[i + 1]);
        } else if (arg == "--output") {
            output_file = argv[i + 1];
        } else {
            if (rank == 0) {
                std::cerr << "Unknown argument: " << arg << std::endl;
            }
            MPI_Finalize();
            return 1;
        }
    }
    
    double start_time = MPI_Wtime();
    
    init(width, height, rank, size);
    
    compute();
    
    gather_results();
    
    save_image(output_file.c_str());
    
    double end_time = MPI_Wtime();
    
    if (rank == 0) {
        std::cout << "Mandelbrot set image saved as " << output_file << std::endl;
        std::cout << "Computation took " << end_time - start_time 
                  << " seconds with " << size << " processes" << std::endl;
    }
    
    MPI_Finalize();
    return 0;
}