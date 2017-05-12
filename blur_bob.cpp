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

void blur_helper (unsigned char *res, unsigned char *frame,
                  unsigned char *blur_vector,
                  int frame_width, int frame_height,
                  int blur_width, int blur_height,
                  int start_row, int end_row, int start_col, int end_col) {

    int blur_height_half = blur_height/2;
    int blur_width_half = blur_width/2;

    int blur_radius = (blur_width - 1) / 2;

    for (int row = start_row; row < end_row; row++) {
        if (row < blur_radius || row > frame_height-1-blur_radius) continue;
        for (int col = start_col; col < end_col; col++) {
            if (col < blur_radius || col > frame_width-1-blur_radius) continue;
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

void *blur_thread_start(void *thread_arg) {
    blur_args *args = (blur_args*)thread_arg;
    blur_helper(args->res, args->frame, args->blur_vector, args->frame_width, args->frame_height, args->blur_width, args->blur_height, args->start_row, args->end_row, args->start_col, args->end_col);
}

void blur_parallel (unsigned char *res, unsigned char *frame,
                    unsigned char *blur_vector,
                    int frame_width, int frame_height,
                    int blur_width, int blur_height) {

    int num_threads = 32;
    pthread_t *thread_ids = new pthread_t[num_threads];
    blur_args *args = new blur_args[num_threads];
    // int rows_per_thread = frame_height / num_threads;
    int cols_per_thread = frame_width / num_threads;
    for (int i = 0; i < num_threads; i++) {
        args[i].res = res;
        args[i].frame = frame;
        args[i].blur_vector = blur_vector;
        args[i].frame_width = frame_width;
        args[i].frame_height = frame_height;
        args[i].blur_width = blur_width;
        args[i].blur_height = blur_height;
        args[i].start_row = 0;
        args[i].end_row = frame_height;
        args[i].start_col = i * cols_per_thread;
        args[i].end_col = (i+1) * cols_per_thread-1;
        pthread_create(&(thread_ids[i]), NULL, blur_thread_start, &(args[i]));
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }
}

