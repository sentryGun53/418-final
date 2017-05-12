/* Blur */

void blur (unsigned char *res, unsigned char *frame, unsigned char *blur_vector,
           int frame_width, int frame_height, int blur_width, int blur_height) {

    int blur_height_half = blur_height/2;
    int blur_width_half = blur_width/2;

    for (int row = 0; row < frame_height; row++) {
        for (int col = 0; col < frame_width; col++) {
            int center_pixel = (row * frame_width) + col;
            int center_pixel_index = center_pixel * 3;
            int new_r = 0;
            int new_g = 0;
            int new_b = 0;
            int blur_value_total = 0;

            for (int i = 0; i < blur_height; i++) {
                for (int j = 0; j < blur_width; j++) {
                    int blur_row = row + (i - blur_height_half);
                    int blur_col = col + (j - blur_width_half);
                    if ((blur_row >= 0) && (blur_row < frame_height) &&
                        (blur_col >= 0) && (blur_col < frame_width)) {

                        int blur_value = blur_vector[(i * blur_width) + j];
                        blur_value_total += blur_value;

                        int blur_pixel = (blur_row * frame_width) + blur_col;
                        int blur_pixel_index = blur_pixel * 3;
                        new_r += blur_value * frame[blur_pixel_index + 0];
                        new_g += blur_value * frame[blur_pixel_index + 1];
                        new_b += blur_value * frame[blur_pixel_index + 2];

                    }
                }
            }

            new_r = new_r / blur_value_total;
            new_g = new_g / blur_value_total;
            new_b = new_b / blur_value_total;
            res[center_pixel_index + 0] = (char) new_r;
            res[center_pixel_index + 1] = (char) new_g;
            res[center_pixel_index + 2] = (char) new_b;
        }
    }
}
