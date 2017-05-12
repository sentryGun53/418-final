/* Blur */

#include"blur_simd.h"

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

void *blur_thread_start(void *thread_arg) {
    blur_args *args = (blur_args*)thread_arg;
    blur_helper(args->res, args->frame, args->blur_vector, args->frame_width, args->frame_height, args->blur_width, args->blur_height, args->start_row, args->end_row, args->start_col, args->end_col);
}

void blur_parallel (unsigned char *res, unsigned char *frame,
                    unsigned char *blur_vector,
                    int frame_width, int frame_height,
                    int blur_width, int blur_height) {

    int num_threads = 16;
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

