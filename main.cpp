/*

MAKE:
  g++ main.cpp -o main -I/usr/local/include -lraspicam -lmmal -lmmal_core -lmmal_util -L/opt/vc/lib -fopenmp

TODO:
  - write output back into input array in background subtration to save some space
  - frame_thresh (results from bg subtraction) only needs to be binary (24x smaller)

TEST HARNESS:
  - grab background frame and a new frame
  - feed same frame & background to both algorithms (sequential & parallel)
  - time execution
  - compare results
  - possibly test at different resolutions?

*/

#include <ctime>
#include <omp.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <raspicam/raspicam.h>
using namespace std;

#include "write_ppm.cpp"
#include "bg_sub.cpp"
#include "blur.cpp"
#include "blob_detect.cpp"


#define FRAME_WIDTH   640
#define FRAME_HEIGHT  480



int main (int argc,  char **argv) {

    std::cout << omp_get_max_threads() << " OMP threads available" << std::endl;
    std::cout << omp_get_num_threads() << " OMP threads currently in use " << std::endl;

    // initialize camera object
    raspicam::RaspiCam Camera;

    // write camera parameters (resolution, format, white balance, etc)
    Camera.setFormat(raspicam::RASPICAM_FORMAT_RGB);
    Camera.setCaptureSize(FRAME_WIDTH, FRAME_HEIGHT);

    // calculate size of image buffers
    int img_size_bytes = Camera.getImageTypeSize(Camera.getFormat());

    // setup buffers for image data
    unsigned char *background = new unsigned char[img_size_bytes]();
    unsigned char *frame_raw  = new unsigned char[img_size_bytes]();
    unsigned char *frame_blur  = new unsigned char[img_size_bytes]();

    // greyscale threshold image
    unsigned char *frame_thresh  = new unsigned char[FRAME_WIDTH * FRAME_HEIGHT]();

    // greyscale output image (blobs each shaded with a uniform & unique grey value)
    unsigned char *frame_blobs  = new unsigned char[FRAME_WIDTH * FRAME_HEIGHT]();


    // open connection to camera
    cout << "Opening Camera..." << endl;
    if (!Camera.open()) {
        cerr << "Error opening camera" << endl;
        return -1;
    }

    // wait a while until camera stabilizes
    cout << "Sleeping for 3 secs" << endl;
    sleep(3);


    // record background
    Camera.grab();
    Camera.retrieve(background);
    cout << "Recorded background." << endl;


    for (int i = 0; i < 10; i++) {
        // capture
        Camera.grab();

        // extract the image in rgb format
        Camera.retrieve(frame_raw);

        // blur raw frame
        int blur_width = 5;
        int blur_height = 5;
        unsigned char blur_vector[25] = {3, 3, 3, 3, 3,
                                        3, 3, 3, 3, 3,
                                        3, 3, 9, 3, 3,
                                        3, 3, 3, 3, 3,
                                        3, 3, 3, 3, 3};
        blur(frame_blur, frame_raw, blur_vector, FRAME_WIDTH, FRAME_HEIGHT, blur_width, blur_height);

        // background subtractiom
        bg_sub(background, frame_blur, frame_thresh, FRAME_WIDTH, FRAME_HEIGHT);

        // blob detection
        struct blob biggest_blob;
        int num_blobs;
        num_blobs = blob_detect(biggest_blob, frame_blobs, frame_thresh, FRAME_WIDTH, FRAME_HEIGHT);

        /********************************************************/
        /*                 END IMAGE PROCESSING                 */
        /********************************************************/

        // save
        write_ppm(background, "background.ppm", FRAME_WIDTH, FRAME_HEIGHT, img_size_bytes);
        write_ppm(frame_raw, "frame_raw.ppm", FRAME_WIDTH, FRAME_HEIGHT, img_size_bytes);
        write_ppm(frame_blur, "frame_blur.ppm", FRAME_WIDTH, FRAME_HEIGHT, img_size_bytes);
        write_ppm_greyscale(frame_thresh, "frame_thresh.ppm", FRAME_WIDTH, FRAME_HEIGHT, FRAME_WIDTH * FRAME_HEIGHT);
        write_ppm_greyscale(frame_blobs, "frame_blobs.ppm", FRAME_WIDTH, FRAME_HEIGHT, FRAME_WIDTH * FRAME_HEIGHT);

    }

    // free resrources
    delete background;
    delete frame_raw;
    delete frame_blur;
    delete frame_thresh;
    // delete frame_blobs;
    return 0;
}
