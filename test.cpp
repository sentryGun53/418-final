#include <ctime>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <ctime>
#include <cassert>
#include <raspicam/raspicam.h>
using namespace std;

#include "write_ppm.cpp"
#include "sequential.cpp"
#include "parallel.cpp"
#include "compare.cpp"

#define FRAME_WIDTH   320
#define FRAME_HEIGHT  240

int main (int argc,  char **argv) {

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
    unsigned short *frame_blobs_seq = new unsigned short[FRAME_WIDTH * FRAME_HEIGHT]();
    unsigned short *frame_blobs_par = new unsigned short[FRAME_WIDTH * FRAME_HEIGHT]();

    // open connection to camera
    cout << "Opening Camera..." << endl;
    if (!Camera.open()) {
        cerr << "Error opening camera" << endl;
        return -1;
    }

    // wait a while until camera stabilizes
    cout << "Sleeping for 1 secs" << endl;
    sleep(1);

    // record background
    Camera.grab();
    Camera.retrieve(background);
    cout << "Recorded background." << endl;
    write_ppm(background, "background.ppm", FRAME_WIDTH, FRAME_HEIGHT, img_size_bytes);

    //for (int i = 0; i < 10; i++) {
        // capture
        cout << "Sleeping for 3 secs" << endl;
        sleep(3);
        Camera.grab();
        // extract the image in rgb format
        Camera.retrieve(frame_raw);
        write_ppm(frame_raw, "raw.ppm", FRAME_WIDTH, FRAME_HEIGHT, img_size_bytes);

        std::cout<<"\n";

        std::clock_t start_seq = std::clock();
        seq(img_size_bytes, FRAME_WIDTH, FRAME_HEIGHT, background, frame_raw, frame_blobs_seq);
        double duration_seq = (std::clock() - start_seq) / (double) CLOCKS_PER_SEC;
        std::cout<<"Sequential time: "<< duration_seq << "\n \n";

        std::clock_t start_par = std::clock();
        par(img_size_bytes, FRAME_WIDTH, FRAME_HEIGHT, background, frame_raw, frame_blobs_par);
        double duration_par = (std::clock() - start_par) / (double) CLOCKS_PER_SEC;
        std::cout<<"Parallel time: "<< duration_par << "\n \n";

        std::cout<<"Sequential time - Parallel time: "<< (duration_seq - duration_par) <<'\n';

        double speedup = duration_seq / duration_par;
        std::cout<<"Speedup: "<< speedup << "x \n";

        // assert(compare(frame_blobs_seq, frame_blobs_par, FRAME_WIDTH, FRAME_HEIGHT));


    //}

    // free resrources
    delete background;
    delete frame_raw;
    delete frame_blobs_seq;
    delete frame_blobs_par;
    return 0;
}
