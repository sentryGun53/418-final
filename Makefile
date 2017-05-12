main:
	g++ -c -Wall -Wextra -Ofast -fopenmp -mfpu=neon -o bg_sub_simd.o bg_sub_simd.cpp
	g++ test.cpp bg_sub_simd.o -o run -I/usr/local/include -lraspicam -lmmal -lmmal_core -lmmal_util -L/opt/vc/lib -fopenmp

clean:
	$(RM) blur_simd.o bg_sub_simd.o run *.ppm
