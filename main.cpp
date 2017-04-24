/*

MAKE:
g++ main.cpp -o main -I/usr/local/include -lraspicam -lmmal -lmmal_core -lmmal_util -L/opt/vc/lib

*/

#include <ctime>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <raspicam/raspicam.h>
using namespace std;

int main ( int argc,char **argv ) {

    raspicam::RaspiCam Camera; //Cmaera object

    //Open camera
    cout<<"Opening Camera..."<<endl;
    if (!Camera.open()) {
        cerr<<"Error opening camera"<<endl;
        return -1;
    }

    //wait a while until camera stabilizes
    cout<<"Sleeping for 3 secs"<<endl;
    sleep(3);

    //capture
    Camera.grab();

    //allocate memory
    unsigned char *data=new unsigned char[  Camera.getImageTypeSize ( raspicam::RASPICAM_FORMAT_RGB )];

    //extract the image in rgb format
    Camera.retrieve ( data,raspicam::RASPICAM_FORMAT_RGB );//get camera image

    //save
    std::ofstream outFile ( "image.ppm",std::ios::binary );
    outFile<<"P6\n"<<Camera.getWidth() <<" "<<Camera.getHeight() <<" 255\n";
    outFile.write ( ( char* ) data, Camera.getImageTypeSize ( raspicam::RASPICAM_FORMAT_RGB ) );
    cout<<"Image saved at image.ppm"<<endl;

    //free resrources
    delete data;
    return 0;
}
