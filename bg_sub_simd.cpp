#include <arm_neon.h>
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

    uint32x4_t c0123; // {0, 1, 2, 3}
    for (int i = 0; i < 4; i++) {
        c0123 = vsetq_lane_u32(i, c0123, i);
    }
    uint32x4_t three = vdupq_n_u32(3);

    for (int row = start_row; row < end_row; row++) {
        for (int col = 0; col < width; col+=4) {
            uint32x4_t mrow = vdupq_n_u32(row);
            uint32x4_t mcol = vaddq_u32(vdupq_n_u32(col), c0123);
            uint32x4_t pixel_index_grey =
                vaddq_u32(vmulq_u32(mrow, vdupq_n_u32(width)), mcol);
            uint32x4_t pixel_index_rgb =
                vmulq_u32(pixel_index_grey, three);
            // loop through lanes
            for (int lane = 0; lane < 4; lane++) {
                uint32_t ind = vgetq_lane_u32(pixel_index_rgb, lane);
                int total_diff = 0;
                // loop through rbg
                for (int i = 0; i < 3; i++) {
                    int diff = bg[ind + i] - fr[ind + i];
                    total_diff += (diff * diff);
                }
                total_diff = sqrt(total_diff);
                uint32_t ind_grey = vgetq_lane_u32(pixel_index_grey, lane);
                if (total_diff > THRESHOLD) {
                    res[ind_grey] = 255;
                } else {
                    res[ind_grey] = 0;
                }
            }

            /*
            int pixel_index_rgb = ((row * width) + col) * 3;
            int pixel_index_grey = ((row * width) + col);
            int total_diff = 0;
            for (int i = 0; i < 3; i++) {
                // make bg[ind] uint32x4_t w/ loads and stores of a single vector lane
                int diff = bg[pixel_index_rgb + i] - fr[pixel_index_rgb + i];
                total_diff += (diff * diff);
            }
            total_diff = sqrt(total_diff);
            if (total_diff > THRESHOLD) {
                res[pixel_index_greyscale] = 255;
            } else {
                res[pixel_index_greyscale] = 0;
            }
            */
        }
    }
}

/*
void *bg_sub_thread_start(void *thread_arg) {
    bg_sub_args *args = (bg_sub_args*)thread_arg;
    bg_sub_helper(args->bg, args->fr, args->res, args->width, args->height, args->start_row, args->end_row);
}
*/

void bg_sub_parallel(unsigned char *bg, unsigned char *fr, unsigned char *res, int width, int height) {
    /*
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
    */

    bg_sub_helper(bg, fr, res, width, height, 0, height);

}

