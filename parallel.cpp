#include <ctime>
#include <unistd.h>
#include <fstream>
#include <iostream>
using namespace std;

//#include "bg_sub.cpp"
//#include "blur.cpp"
//#include "blob_detect.cpp"
// #include "blur_par.cpp"
#include "blur_bob.cpp"
//#include "bg_sub_par.cpp"
#include "bg_sub_simd.h"
#include "blob_detect_par.cpp"

void par(int img_size_bytes, int frame_width, int frame_height,
         unsigned char *background, unsigned char *frame_raw,
         unsigned short *frame_blobs) {

    // setup buffers for image data
    unsigned char *frame_blur = new unsigned char[img_size_bytes]();
    unsigned char *frame_thresh = new unsigned char[frame_width * frame_height]();

    // blur raw frame
    int blur_width = 3;
    int blur_height = 3;
    unsigned char blur_vector[blur_width * blur_height] =
    {3, 3, 3,
     3, 9, 3,
     3, 3, 3};
    std::clock_t start = std::clock();
    blur_parallel(frame_blur, frame_raw, blur_vector, frame_width, frame_height, blur_width, blur_height);
    double duration_blur = (std::clock() - start) / (double) CLOCKS_PER_SEC;
    std::cout<<"    Blur: "<< duration_blur <<'\n';

    // background subtractiom
    start = std::clock();
    bg_sub_parallel(background, frame_blur, frame_thresh, frame_width, frame_height);
    double duration_sub = (std::clock() - start) / (double) CLOCKS_PER_SEC;
    std::cout<<"    Sub:  "<< duration_sub <<'\n';

    // blob detection
    start = std::clock();
    struct blob biggest_blob;
    int num_blobs;
    num_blobs = blob_detect_par(biggest_blob, frame_blobs, frame_thresh, frame_width, frame_height);
    double duration_blob = (std::clock() - start) / (double) CLOCKS_PER_SEC;
    std::cout<<"    Blob: "<< duration_blob <<'\n';


    // free resources
    delete frame_blur;
    delete frame_thresh;

}
