/*

MAKE:
  g++ main.cpp -o main -I/usr/local/include -lraspicam -lmmal -lmmal_core -lmmal_util -L/opt/vc/lib

TODO:
  - write output back into input array in background subtration to save some space
  - frame_thresh (results from bg subtraction) only needs to be binary (24x smaller)

*/

#include <ctime>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <raspicam/raspicam.h>
using namespace std;

#include "bg_sub.cpp"
#include "blur.cpp"
#include "blob_detect.cpp"

#define FRAME_WIDTH   640
#define FRAME_HEIGHT  480


void write_ppm(unsigned char *image, const char *filename, int width, int height, int size) {
    std::ofstream outFile (filename,  std::ios::binary);
    outFile << "P6\n" << width << " " << height << " 255\n";
    outFile.write (( char* )image, size);
    cout << "PPM image saved at " << filename << "  - " << width << " x " << height << endl;
}

int main ( int argc,char **argv ) {

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
    unsigned char *frame_thresh  = new unsigned char[img_size_bytes]();
    unsigned char *frame_blobs  = new unsigned char[img_size_bytes]();

    // open connection to camera
    cout << "Opening Camera..." << endl;
    if (!Camera.open()) {
        cerr << "Error opening camera" << endl;
        return -1;
    }

    // sleep(1);
    Camera.grab();
    Camera.retrieve(background);

    // wait a while until camera stabilizes
    cout << "Sleeping for 3 secs" << endl;
    sleep(3);

    // capture
    Camera.grab();

    // extract the image in rgb format
    Camera.retrieve(frame_raw);


    /********************************************************/
    /*               BEGIN IMAGE PROCESSING                 */
    /********************************************************/

    cout << "Beginning image processing" << endl;

    // iterate through all pixels
    // for (int row = 0; row < FRAME_HEIGHT; row++) {
    //     for (int col = 0; col < FRAME_WIDTH; col++) {

    //         // basic pixel indexing (in order)
    //         int pixel_index = ((row * FRAME_WIDTH) + col) * 3;

    //         // read each color
    //         char r = data[pixel_index + 0];
    //         char g = data[pixel_index + 1];
    //         char b = data[pixel_index + 2];

    //         // set all red pixels to full brightness (255)
    //         g = 0;

    //         // write back
    //         data[pixel_index + 0] = r;
    //         data[pixel_index + 1] = g;
    //         data[pixel_index + 2] = b;
    //     }
    // }


    int blur_width = 5;
    int blur_height = 5;
    unsigned char blur_vector[25] = {3, 3, 3, 3, 3,
                                    3, 3, 3, 3, 3,
                                    3, 3, 9, 3, 3,
                                    3, 3, 3, 3, 3,
                                    3, 3, 3, 3, 3};

    blur(frame_blur, frame_raw, blur_vector, FRAME_WIDTH, FRAME_HEIGHT, blur_width, blur_height);
    cout << "  1) Blurring done" << endl;

    bg_sub(background, frame_blur, frame_thresh, FRAME_WIDTH, FRAME_HEIGHT);
    cout << "  2) Background subtraction done" << endl;

    unsigned char num_blobs = blob_detect(frame_blobs, frame_thresh, FRAME_WIDTH, FRAME_HEIGHT);
    cout << "  3) Blob detection done" << endl;
    cout << "num_blobs = " << (int)num_blobs << endl;

    /********************************************************/
    /*                 END IMAGE PROCESSING                 */
    /********************************************************/

    // save
    write_ppm(background, "background.ppm", FRAME_WIDTH, FRAME_HEIGHT, img_size_bytes);
    write_ppm(frame_raw, "frame_raw.ppm", FRAME_WIDTH, FRAME_HEIGHT, img_size_bytes);
    write_ppm(frame_blur, "frame_blur.ppm", FRAME_WIDTH, FRAME_HEIGHT, img_size_bytes);
    write_ppm(frame_thresh, "frame_thresh.ppm", FRAME_WIDTH, FRAME_HEIGHT, img_size_bytes);
    write_ppm(frame_blobs, "frame_blobs.ppm", FRAME_WIDTH, FRAME_HEIGHT, img_size_bytes);


    // free resrources
    delete background;
    delete frame_raw;
    delete frame_blur;
    delete frame_thresh;
    delete frame_blobs;
    return 0;
}
