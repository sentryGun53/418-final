/* Write ppm files */

void write_ppm(unsigned char *image, const char *filename, int width, int height, int size) {
    std::ofstream outFile (filename,  std::ios::binary);
    outFile << "P6\n" << width << " " << height << " 255\n";
    outFile.write (( char* )image, size);
    cout << "PPM image saved at " << filename << "  - " << width << " x " << height << endl;
}

void write_ppm_greyscale(unsigned char *image, const char *filename, int width, int height, int size) {
    std::ofstream outFile (filename,  std::ios::binary);
    outFile << "P6\n" << width << " " << height << " 255\n";
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
        	char val = image[row * width + col];
    		outFile.write (&val, 1);
    		outFile.write (&val, 1);
    		outFile.write (&val, 1);
    	}
    }
    cout << "PPM image saved at " << filename << "  - " << width << " x " << height << endl;
}


void write_ppm_greyscale_short(unsigned short *image, const char *filename, int width, int height, int size) {
    std::ofstream outFile (filename,  std::ios::binary);
    outFile << "P6\n" << width << " " << height << " 255\n";
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            short v = image[row * width + col];
            char val = v % 256;
            outFile.write (&val, 1);
            outFile.write (&val, 1);
            outFile.write (&val, 1);
        }
    }
    cout << "PPM image saved at " << filename << "  - " << width << " x " << height << endl;
}
