#include <arm_neon.h>
#include <math.h>

#define THRESHOLD 30

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
        }
    }
}

void bg_sub_parallel(unsigned char *bg, unsigned char *fr, unsigned char *res, int width, int height) {
    bg_sub_helper(bg, fr, res, width, height, 0, height);
}

