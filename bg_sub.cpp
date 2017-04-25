/* Background subtraction */
#include <math.h>

#define THRESHOLD 30

void bg_sub(unsigned char *bg, unsigned char *fr, unsigned char *res,
                      int width, int height) {
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int pixel_index = ((row * width) + col) * 3;
            int total_diff = 0;
            for (int i = 0; i < 3; i++) {
                int diff = bg[pixel_index + i] - fr[pixel_index + i];
                total_diff += (diff * diff);
            }
            total_diff = sqrt(total_diff);
            if (total_diff > THRESHOLD) {
                res[pixel_index + 0] = 255;
                res[pixel_index + 1] = 255;
                res[pixel_index + 2] = 255;
            } else {
                res[pixel_index + 0] = 0;
                res[pixel_index + 1] = 0;
                res[pixel_index + 2] = 0;
            }
        }
    }
}
