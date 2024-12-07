CPP = g++

CXXFLAGS = -std=c++0x

OPTFLAGS = -O3 -ffast-math -funroll-loops -march=native

serial: build/serial

build/serial: Mandelbrot.cc
	$(CPP) $(CXXFLAGS) $(OPTFLAGS) $^ -o $@

run: build/serial
	./build/serial

.PHONY: clean

clean:
	rm -f build/serial
	rm -f build/*.out
	rm -f build/*.o
	rm -f build/*.gif