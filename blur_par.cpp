/* Blur */

typedef struct {
    unsigned char *res;
    unsigned char *frame;
    unsigned char *blur_vector;
    int frame_width;
    int frame_height;
    int blur_width;
    int blur_height;
    int start_row;
    int end_row;
    int start_col;
    int end_col;
} blur_args;

void blur_helper (unsigned char *res, unsigned char *frame, unsigned char *blur_vector,
           int frame_width, int frame_height, int blur_width, int blur_height,
           int start_row, int end_row, int start_col, int end_col) {
    int blur_height_half = blur_height/2;
    int blur_width_half = blur_width/2;
    for (int row = start_row; row < end_row; row++) {
        for (int col = start_col; col < end_col; col++) {
            int center_pixel = (row * frame_width) + col;
            int center_pixel_index = center_pixel * 3;
            int blur_height_half = blur_height/2;
            int blur_width_half = blur_width/2;
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

void *blur_thread_start(void *thread_arg) {
    blur_args *args = (blur_args*)thread_arg;
    blur_helper(args->res, args->frame, args->blur_vector, args->frame_width, args->frame_height, args->blur_width, args->blur_height, args->start_row, args->end_row, args->start_col, args->end_col);
}

void blur_parallel (unsigned char *res, unsigned char *frame, unsigned char *blur_vector,
        int frame_width, int frame_height, int blur_width, int blur_height) {
    pthread_t thread_id1;
    pthread_t thread_id2;
    pthread_t thread_id3;

    blur_args args1;
    args1.res = res;
    args1.frame = frame;
    args1.blur_vector = blur_vector;
    args1.frame_width = frame_width;
    args1.frame_height = frame_height;
    args1.blur_width = blur_width;
    args1.blur_height = blur_height;
    args1.start_row = 0;
    args1.end_row = frame_height / 4;
    args1.start_col = 0;
    args1.end_col = frame_width;
    blur_args args2;
    args2.res = res;
    args2.frame = frame;
    args2.blur_vector = blur_vector;
    args2.frame_width = frame_width;
    args2.frame_height = frame_height;
    args2.blur_width = blur_width;
    args2.blur_height = blur_height;
    args2.start_row = frame_height / 4;
    args2.end_row = frame_height / 2;
    args2.start_col = 0;
    args2.end_col = frame_width;
    blur_args args3;
    args3.res = res;
    args3.frame = frame;
    args3.blur_vector = blur_vector;
    args3.frame_width = frame_width;
    args3.frame_height = frame_height;
    args3.blur_width = blur_width;
    args3.blur_height = blur_height;
    args3.start_row = frame_height / 2;
    args3.end_row = (frame_height / 4) * 3;
    args3.start_col = 0;
    args3.end_col = frame_width;

    pthread_create(&thread_id1, NULL, blur_thread_start, &args1);
    pthread_create(&thread_id2, NULL, blur_thread_start, &args2);
    pthread_create(&thread_id3, NULL, blur_thread_start, &args3);

    blur_helper (res, frame, blur_vector, frame_width, frame_height,
            blur_width, blur_height, (frame_height/4)*3, frame_height,
            0, frame_width);

    pthread_join(thread_id1, NULL);
    pthread_join(thread_id2, NULL);
    pthread_join(thread_id3, NULL);
}

