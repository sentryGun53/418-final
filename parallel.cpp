#include <ctime>
#include <unistd.h>
#include <fstream>
#include <iostream>
using namespace std;

//#include "bg_sub.cpp"
//#include "blur.cpp"
//#include "blob_detect.cpp"
#include "blur_par.cpp"
#include "bg_sub_par.cpp"
//#include "bg_sub_simd.h"
#include "blob_detect_par.cpp"

void par(int img_size_bytes, int frame_width, int frame_height,
         unsigned char *background, unsigned char *frame_raw,
         unsigned short *frame_blobs) {

    // setup buffers for image data
    unsigned char *frame_blur = new unsigned char[img_size_bytes]();
    unsigned char *frame_thresh = new unsigned char[frame_width * frame_height]();

    // blur raw frame
    int blur_width = 5;
    int blur_height = 5;
    unsigned char blur_vector[blur_width * blur_height] =
    {3, 3, 3, 3, 3,
     3, 3, 3, 3, 3,
     3, 3, 9, 3, 3,
     3, 3, 3, 3, 3,
     3, 3, 3, 3, 3};
    blur_parallel(frame_blur, frame_raw, blur_vector, frame_width, frame_height, blur_width, blur_height);

    // background subtractiom
    bg_sub_parallel(background, frame_blur, frame_thresh, frame_width, frame_height);

    write_ppm(frame_blur, "frame_blur_par.ppm", frame_width, frame_height, img_size_bytes);
    write_ppm_greyscale(frame_thresh, "frame_thresh_par.ppm", frame_width, frame_height, frame_width * frame_height);


    // blob detection
    struct blob biggest_blob;
    int num_blobs;
    num_blobs = blob_detect_par(biggest_blob, frame_blobs, frame_thresh, frame_width, frame_height);

    // save
    write_ppm_greyscale_short(frame_blobs, "frame_blobs_par.ppm", frame_width, frame_height, frame_width * frame_height);

    // free resources
    delete frame_blur;
    delete frame_thresh;
    //delete frame_blobs;

}
