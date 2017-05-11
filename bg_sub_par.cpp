/* Background subtraction */
#include <math.h>

#define THRESHOLD 30

typedef struct {
    unsigned char *bg;
    unsigned char *fr;
    unsigned char *res;
    int width;
    int height;
    int start_row;
    int end_row;
} bg_sub_args;

void bg_sub_helper(unsigned char *bg, unsigned char *fr, unsigned char *res, int width, int height, int start_row, int end_row) {
    for (int row = start_row; row < end_row; row++) {
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

void *bg_sub_thread_start(void *thread_arg) {
    bg_sub_args *args = (bg_sub_args*)thread_arg;
    bg_sub_helper(args->bg, args->fr, args->res, args->width, args->height, args->start_row, args->end_row);
}

/*
 *  bg (background) and fg (foreground) are RGB color images (3 bytes per pixel).
 *  res is a greyscale image (1 byte per pixel)
 */
void bg_sub_parallel(unsigned char *bg, unsigned char *fr, unsigned char *res, int width, int height) {
    pthread_t thread_id1;
    pthread_t thread_id2;
    pthread_t thread_id3;

    bg_sub_args args1;
    bg_sub_args args2;
    bg_sub_args args3;
    args1.bg = bg;
    args1.fr = fr;
    args1.res = res;
    args1.width = width;
    args1.height = height;
    args1.start_row = 0;
    args1.end_row = height / 4;
    args2.bg = bg;
    args2.fr = fr;
    args2.res = res;
    args2.width = width;
    args2.height = height;
    args2.start_row = height / 4;
    args2.end_row = height / 2;
    args3.bg = bg;
    args3.fr = fr;
    args3.res = res;
    args3.width = width;
    args3.height = height;
    args3.start_row = height / 2;
    args3.end_row = (height / 4) * 3;

    pthread_create(&thread_id1, NULL, bg_sub_thread_start, &args1);
    pthread_create(&thread_id2, NULL, bg_sub_thread_start, &args2);
    pthread_create(&thread_id3, NULL, bg_sub_thread_start, &args3);

    bg_sub_helper(bg, fr, res, width, height, (height / 4) * 3, height);

    pthread_join(thread_id1, NULL);
    pthread_join(thread_id2, NULL);
    pthread_join(thread_id3, NULL);

}

/* OpenMP version for testing */
void bg_sub_omp(unsigned char *bg, unsigned char *fr, unsigned char *res, int width, int height) {
    //#pragma omp parallel for
    for (int row = 0; row < height; row++) {
        //#pragma omp parallel for
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
