main:
	g++ main.cpp -o main -I/usr/local/include -lraspicam -lmmal -lmmal_core -lmmal_util -L/opt/vc/lib -fopenmp

test:
	g++ test.cpp -o test -I/usr/local/include -lraspicam -lmmal -lmmal_core -lmmal_util -L/opt/vc/lib -fopenmp
