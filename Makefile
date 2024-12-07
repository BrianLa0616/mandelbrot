CPP=CC

serial: build/serial 

build/serial: Mandelbrot.cc $(CPP)

.PHONY: clean

clean:
	rm -f build/*.out
	rm -f build/*.o
	rm -f build/*.gif