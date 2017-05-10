/* Background subtraction */
#include <math.h>

#define THRESHOLD 30

/*
 *  bg (background) and fg (foreground) are RGB color images (3 bytes per pixel).
 *  res is a greyscale image (1 byte per pixel)
 */
void bg_sub(unsigned char *bg, unsigned char *fr, unsigned char *res, int width, int height) {
    #pragma omp parallel for
    for (int row = 0; row < height; row++) {
        #pragma omp parallel for
        for (int col = 0; col < width; col++) {
            int pixel_index_rgb = ((row * width) + col) * 3;
            int pixel_index_grey = ((row * width) + col);
            int total_diff = 0;
            for (int i = 0; i < 3; i++) {
                int diff = bg[pixel_index_rgb + i] - fr[pixel_index_rgb + i];
                total_diff += (diff * diff);
            }
            total_diff = sqrt(total_diff);
            if (total_diff > THRESHOLD) {
                res[pixel_index_grey] = 255;
            } else {
                res[pixel_index_grey] = 0;
            }
        }
    }
}
