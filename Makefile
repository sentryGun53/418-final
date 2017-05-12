main:
	g++ main.cpp -o main -I/usr/local/include -lraspicam -lmmal -lmmal_core -lmmal_util -L/opt/vc/lib -fopenmp

test:
	g++ test.cpp -o test -I/usr/local/include -lraspicam -lmmal -lmmal_core -lmmal_util -L/opt/vc/lib -fopenmp

simd_step:
	g++ -c -Wall -Wextra -Ofast -fopenmp -mfpu=neon -o bg_sub_simd.o bg_sub_simd.cpp

simd:
	g++ test.cpp bg_sub_simd.o -o test -I/usr/local/include -lraspicam -lmmal -lmmal_core -lmmal_util -L/opt/vc/lib -fopenmp
