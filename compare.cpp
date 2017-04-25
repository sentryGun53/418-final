/* Compare sequential and parallel for correctness */

bool compare(unsigned char *image1, unsigned char *image2, int width, int height) {
    for (int i = 0; i < width * height; i++) {
        if (image1[i] != image2[i]) return false;
    }
    return true;
}
