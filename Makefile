CPP = CC
CFLAGS=-lm
NVCC = nvcc
MPIFLAGS=-DMPI

CXXFLAGS = -std=c++0x

OPTFLAGS = -O3 -ffast-math -funroll-loops -march=native
CUDAFLAGS = -O3

baseline: build/baseline

build/baseline: baseline_mandelbrot.cc
	$(CPP) $(CXXFLAGS) $^ -o $@

serial: build/serial

build/serial: Mandelbrot.cc
	$(CPP) $(CXXFLAGS) $(OPTFLAGS) $^ -o $@

cuda: build/mandelbrot_cuda

build/cuda: mandelbrot_cuda.cu
	$(NVCC) $(CUDAFLAGS) $^ -o $@

run_cuda: build/cuda
	./build/cuda

mpi: build/mpi

build/mpi: main.cc mpi.cc
	$(CPP) $^ -o $@ $(MPIFLAGS) $(CFLAGS) $(OPTFLAGS) 

run: build/baseline
	./build/baseline

.PHONY: clean

clean:
	rm -f build/serial
	rm -f build/cuda
	rm -f build/baseline
	rm -f build/mpi
	rm -f build/*.out
	rm -f build/*.o
	rm -f build/*.gif