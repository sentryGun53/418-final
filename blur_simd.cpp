#include <arm_neon.h>

void blur_helper(unsigned char *res, unsigned char *frame,
                 unsigned char *blur_vector,
                 int frame_width, int frame_height,
                 int blur_width, int blur_height,
                 int start_row, int end_row, int start_col, int end_col) {

    int32x4_t fheight = vdupq_n_s32(frame_height);
    int32x4_t fwidth = vdupq_n_s32(frame_width);
    int32x4_t zero = vdupq_n_s32(0);
    uint32x4_t one = vdupq_n_u32(1);
    int32x4_t c0123; // {0, 1, 2, 3}
    int32x4_t bwidth = vdupq_n_s32(blur_width);
    int32x4_t blur_height_half = vdupq_n_s32(blur_height/2);
    int32x4_t blur_width_half = vdupq_n_s32(blur_width/2);
    for (int i = 0; i < 4; i++) {
        c0123 = vsetq_lane_s32(i, c0123, i);
    }

    for (int32_t row = start_row; row < end_row; row++) {
        for (int32_t col = start_col; col < end_col; col+=4) {
            int32x4_t mrow = vdupq_n_s32(row);
            int32x4_t mcol = vaddq_s32(vdupq_n_s32(col), c0123);
            int32x4_t new_r = vdupq_n_s32(0);
            int32x4_t new_g = vdupq_n_s32(0);
            int32x4_t new_b = vdupq_n_s32(0);
            int blur_value_total = 0;

            for (int i = 0; i < blur_height; i++) {
                for (int j = 0; j < blur_width; j++) {
                    int32x4_t mi = vdupq_n_s32(i);
                    int32x4_t mj = vdupq_n_s32(j);
                    int32x4_t blur_row =
                        vaddq_s32(mrow, vsubq_s32(mi, blur_height_half));
                    int32x4_t blur_col =
                        vaddq_s32(mcol, vsubq_s32(mj, blur_width_half));
                    uint32x4_t mask1 = vcgeq_s32(blur_row, zero);
                    uint32x4_t mask2 = vcltq_s32(blur_row, fheight);
                    uint32x4_t mask3 = vcgeq_s32(blur_col, zero);
                    uint32x4_t mask4 = vcltq_s32(blur_col, fwidth);
                    // zero means skip
                    uint32x4_t mask = vandq_u32(mask4, vandq_u32(mask3,
                                vandq_u32(mask2, vandq_u32(mask1, one))));
                    int32x4_t blur_vector_ind =
                        vaddq_s32(vmulq_s32(mi, bwidth), mj);
                    // loop through lanes
                    for (int lane = 0; lane < 4; lane++) {
                        if (vgetq_lane_u32(mask, lane) != 0) {
                            int ind = vgetq_lane_s32(blur_vector_ind, lane);
                            int blur_value = blur_vector[ind];
                            blur_value_total += blur_value;
                            int blur_pixel_index =
                                ((vgetq_lane_s32(blur_row, lane) * frame_width) +
                                vgetq_lane_s32(blur_col, lane) * 3);
                            int old_r = vgetq_lane_s32(new_r, lane);
                            int old_b = vgetq_lane_s32(new_b, lane);
                            int old_g = vgetq_lane_s32(new_g, lane);
                            new_r = vsetq_lane_s32(old_r + (blur_value *
                                        frame[blur_pixel_index]), new_r, lane);
                            new_b = vsetq_lane_s32(old_b + (blur_value *
                                        frame[blur_pixel_index+1]), new_b, lane);
                            new_g = vsetq_lane_s32(old_g + (blur_value *
                                        frame[blur_pixel_index+2]), new_g, lane);
                        }
                    }
                }
            }

            // loop through lanes
            for (int lane = 0; lane < 4; lane++) {
                int center_pixel_index = 3 * ((row * frame_width) + (col + lane));
                new_r = new_r / blur_value_total;
                new_g = new_g / blur_value_total;
                new_b = new_b / blur_value_total;
                res[center_pixel_index + 0] = (char) (vgetq_lane_s32(new_r, lane) /
                        blur_value_total);
                res[center_pixel_index + 1] = (char) (vgetq_lane_s32(new_b, lane) /
                        blur_value_total);
                res[center_pixel_index + 2] = (char) (vgetq_lane_s32(new_g, lane) /
                        blur_value_total);
            }
        }
    }
}
