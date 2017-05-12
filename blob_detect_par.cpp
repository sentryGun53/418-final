/* Blob Detection */

// http://www.labbookpages.co.uk/software/imgProc/blobDetection.html

#include <omp.h>

#define IDX(R, C)   ((((R) * width) + (C)))

#include <stdlib.h>

typedef struct {
    unsigned short *output;
    unsigned char *input;
    struct blob *blobs;
    short width;
    short height;
    short start_row;
    short end_row;
    short start_label;
} label_args;


typedef struct {
    unsigned short *output;
    unsigned char *input;
    struct blob *blobs;
    short width;
    short height;
    short stitch_row;
} stitch_args;


/*
 * input is a greyscale threshold image (1 byte per pixel) with
 * nonzero pixel values if motion was detected in a pixel
 *
 * output is a greyscale image (1 byte per pixel) that will show each blob shaded
 * with a uniform & unique grey value
 *
 * biggest_blob is a struct blob that contains the biggest blob
 *
 * the return int value is the number of blobs total that were detected
 */


#define MAX_BLOBS_PER_THREAD 200

void label(unsigned short *output, unsigned char *input, struct blob *blobs, short width, short height, short start_row, short end_row, short start_label) {

    // begin labelling blobs at this number
    short label_counter = start_label;

    // iterate through image
    for (short row = start_row; row <= end_row; row++) {
        for (short col = 1; col < width-1; col++) {

            /* we care about the pixel above, to the left, above-left, and above-right.
             * pixels are labelled as follows (X = current pixel):
             *      A B C
             *      D X
             */

            // output[IDX(row, col)] = 0;

            // label that will be assigned to the current pixel. calculated below
            short lbl_x = 0;

            // if our pixel is active, check the neighboring pixels to determine it's label
            if (input[IDX(row, col)] > 0) {

                // get the labels for pixels a, b, c, d  - these have already been calculated.
                short lbl_a = output[IDX(row-1, col-1)];
                short lbl_b = output[IDX(row-1, col  )];
                short lbl_c = output[IDX(row-1, col+1)];
                short lbl_d = output[IDX(row,   col-1)];

                // find the minimum label out of all the neighbors, by checking
                // each label and updating min_label if we find a new minimum
                short min_label = 9999;
                if (lbl_a != 0 && lbl_a < min_label)  min_label = lbl_a;
                if (lbl_b != 0 && lbl_b < min_label)  min_label = lbl_b;
                if (lbl_c != 0 && lbl_c < min_label)  min_label = lbl_c;
                if (lbl_d != 0 && lbl_d < min_label)  min_label = lbl_d;

                // no neighbors found, so we start a new blob on this pixel.
                if (min_label == 9999) {
                    lbl_x = label_counter;         // get a new label and use it for this pixel
                    label_counter += 1;            // increment for use later
                    if (label_counter >= start_label + MAX_BLOBS_PER_THREAD) assert(false);  // error: too many blobs
                    blobs[lbl_x].label = lbl_x;    // update the new label's entry in the label table
                    blobs[lbl_x].x_min = col;
                    blobs[lbl_x].x_max = col;
                    blobs[lbl_x].y_min = row;
                    blobs[lbl_x].y_max = row;
                    blobs[lbl_x].mass = 1;
                    blobs[lbl_x].sum_x = col;
                    blobs[lbl_x].sum_y = row;
                }
                else {
                    // adding to an existing blob: use the minimum label from all of them
                    lbl_x = min_label;

                    // update min/max tables & mass table
                    if (blobs[lbl_x].x_min > col)  blobs[lbl_x].x_min = col;
                    if (blobs[lbl_x].x_max < col)  blobs[lbl_x].x_max = col;
                    if (blobs[lbl_x].y_min > row)  blobs[lbl_x].y_min = row;
                    if (blobs[lbl_x].y_max < row)  blobs[lbl_x].y_max = row;
                    blobs[lbl_x].mass++;
                    blobs[lbl_x].sum_x += col;
                    blobs[lbl_x].sum_y += row;

                    // update the table in case we just joined two blobs
                    if (lbl_a != min_label)  blobs[lbl_a].label = min_label;
                    if (lbl_b != min_label)  blobs[lbl_b].label = min_label;
                    if (lbl_c != min_label)  blobs[lbl_c].label = min_label;
                    if (lbl_d != min_label)  blobs[lbl_d].label = min_label;
                }
            }

            // write results to buf
            output[IDX(row, col)] = lbl_x;
        }
    }

    // redirect all labels within a blob to equal the lowest-index label used by that blob
    for (short i = start_label; i < start_label + MAX_BLOBS_PER_THREAD; i++) {
        if (blobs[i].label != i && blobs[i].label != 0) {
            blobs[i].label = blobs[blobs[i].label].label;
        }
    }
}

void stitch(unsigned short *output, unsigned char *input, struct blob *blobs, short width, short height, short stitch_row) {
    short row = stitch_row;
    for (short col = 1; col < width-1; col++) {

        /* we care about the pixel above, above-left, and above-right.
         * pixels are labelled as follows (X = current pixel):
         *      A B C
         *        X
         */
        short lbl_x = output[IDX(row, col)];
        short lbl_a = output[IDX(row-1, col-1)];
        short lbl_b = output[IDX(row-1, col  )];
        short lbl_c = output[IDX(row-1, col+1)];

        // find the minimum label out of all the neighbors, by checking
        // each label and updating min_label if we find a new minimum
        short min_label = 9999;
        if (lbl_a != 0 && lbl_a < min_label)  min_label = lbl_a;
        if (lbl_b != 0 && lbl_b < min_label)  min_label = lbl_b;
        if (lbl_c != 0 && lbl_c < min_label)  min_label = lbl_c;

        // if a neighbor is found, stich the entries in the blob list together
        if (min_label != 9999) {
            if (lbl_x != min_label)  blobs[lbl_x].label = min_label;
            if (lbl_a != min_label)  blobs[lbl_a].label = min_label;
            if (lbl_b != min_label)  blobs[lbl_b].label = min_label;
            if (lbl_c != min_label)  blobs[lbl_c].label = min_label;
        }
    }
}

void *label_thread_start(void *thread_arg) {
    label_args *args = (label_args*)thread_arg;
    label(args->output, args->input, args->blobs, args->width, args->height, args->start_row, args->end_row, args->start_label);
}

void *stitch_thread_start(void *thread_arg) {
    stitch_args *args = (stitch_args*)thread_arg;
    stitch(args->output, args->input, args->blobs, args->width, args->height, args->stitch_row);
}

int blob_detect_par(struct blob &biggest_blob, unsigned short *output, unsigned char *input, short width, short height) {

    // our list of blobs. TODO: better calculation of size here
    struct blob blobs[(MAX_BLOBS_PER_THREAD+1)*4] = {0};

    /*
     *   ThreadID   start_row   end_row
     *   0          1           59
     *   1          60          119
     *   2          120         179
     *   3          180         239
     */

    // spawn 3 additional labeller threads (4 total)
    pthread_t thread_id1, thread_id2, thread_id3;
    label_args args1, args2, args3;
    args1.output = args2.output = args3.output = output;
    args1.input = args2.input = args3.input = input;
    args1.blobs = args2.blobs = args3.blobs = blobs;
    args1.width = args2.width = args3.width = width;
    args1.height = args2.height = args3.height = height;
    args1.start_row = 60;
    args1.end_row = 119;
    args1.start_label = MAX_BLOBS_PER_THREAD*1 + 1;
    args2.start_row = 120;
    args2.end_row = 179;
    args2.start_label = MAX_BLOBS_PER_THREAD*2 + 1;
    args3.start_row = 180;
    args3.end_row = 239;
    args3.start_label = MAX_BLOBS_PER_THREAD*3 + 1;
    pthread_create(&thread_id1, NULL, label_thread_start, &args1);
    pthread_create(&thread_id2, NULL, label_thread_start, &args2);
    pthread_create(&thread_id3, NULL, label_thread_start, &args3);

    // MAIN THREAD:
    label(output, input, blobs, width, height,    1,   59,    1 );
    // OTHER THREADS:
    //     label(output, input, blobs, width, height,   60,  119,  101 );
    //     label(output, input, blobs, width, height,  120,  179,  201 );
    //     label(output, input, blobs, width, height,  180,  239,  301 );

    // join labeller threads
    pthread_join(thread_id1, NULL);
    pthread_join(thread_id2, NULL);
    pthread_join(thread_id3, NULL);


    // spawn 2 additional stitcher threads (3 total)
    stitch_args sargs1, sargs2;
    sargs1.output = sargs2.output = output;
    sargs1.input = sargs2.input = input;
    sargs1.blobs = sargs2.blobs = blobs;
    sargs1.width = sargs2.width = width;
    sargs1.height = sargs2.height = height;
    sargs1.stitch_row = 120;
    sargs2.stitch_row = 180;
    pthread_create(&thread_id1, NULL, stitch_thread_start, &sargs1);
    pthread_create(&thread_id2, NULL, stitch_thread_start, &sargs2);

    // MAIN THREAD:
    stitch(output, input, blobs, width, height,   60 );
    // OTHER THREADS:
    //     stitch(output, input, blobs, width, height,  120 );
    //     stitch(output, input, blobs, width, height,  180 );

    // join stitcher threads
    pthread_join(thread_id1, NULL);
    pthread_join(thread_id2, NULL);

    // redirect all labels within a blob to equal the lowest-index label used by that blob
    for (short i = 1; i < MAX_BLOBS_PER_THREAD*4; i++) {
        if (blobs[i].label != i) {
            blobs[i].label = blobs[blobs[i].label].label;
        }
    }

    // merge blobs that touch, put resulting list of blobs into consolidated_blobs[]
    struct blob consolidated_blobs[MAX_BLOBS_PER_THREAD];
    short num_blobs = 0;

    // go thorugh the un-merged list backwards, and merge towards beginning
    for (short i = MAX_BLOBS_PER_THREAD*4 - 1; i > 0; i--) {

        if (blobs[i].label == 0) continue;

        // if this blob's label points to a different blob, merge with the lower blob
        if (blobs[i].label != i) {

            // which blob to merge with
            short d = blobs[i].label;

            // merge max/min and mass table into the lower entry (combine blobs)
            if (blobs[d].x_min > blobs[i].x_min)  blobs[d].x_min = blobs[i].x_min;
            if (blobs[d].x_max < blobs[i].x_max)  blobs[d].x_max = blobs[i].x_max;
            if (blobs[d].y_min > blobs[i].y_min)  blobs[d].y_min = blobs[i].y_min;
            if (blobs[d].y_max < blobs[i].y_max)  blobs[d].y_max = blobs[i].y_max;
            blobs[d].mass  += blobs[i].mass;
            blobs[d].sum_x += blobs[i].sum_x;
            blobs[d].sum_y += blobs[i].sum_y;
        }
        else {   // we reached the original entry, record to the final list
            consolidated_blobs[num_blobs].label = num_blobs;
            consolidated_blobs[num_blobs].x_min = blobs[i].x_min;
            consolidated_blobs[num_blobs].x_max = blobs[i].x_max;
            consolidated_blobs[num_blobs].y_min = blobs[i].y_min;
            consolidated_blobs[num_blobs].y_max = blobs[i].y_max;
            consolidated_blobs[num_blobs].mass  = blobs[i].mass;
            consolidated_blobs[num_blobs].sum_x = blobs[i].sum_x;
            consolidated_blobs[num_blobs].sum_y = blobs[i].sum_y;

            // calculate centroid
            consolidated_blobs[num_blobs].centroid_x = consolidated_blobs[num_blobs].sum_x / consolidated_blobs[num_blobs].mass;
            consolidated_blobs[num_blobs].centroid_y = consolidated_blobs[num_blobs].sum_y / consolidated_blobs[num_blobs].mass;

            num_blobs++;
        }
    }

    // OPTIONAL: update to show merged blobs in the output image
    for (short row = 1; row < height-1; row++) {
        for (short col = 1; col < width-1; col++) {
            if (output[IDX(row, col)] != 0) {
                output[IDX(row, col)] = blobs[output[IDX(row, col)]].label;
            }
        }
    }


    // OPTIONAL: print blob statistics
    // for (int i = 0; i < num_blobs; i++) {
    //     cout << "[" << i << "]: label_table=" << (int)consolidated_blobs[i].label <<
    //                          ",\t x_min=" << consolidated_blobs[i].x_min <<
    //                          ",\t x_max=" << consolidated_blobs[i].x_max <<
    //                          ",\t y_min=" << consolidated_blobs[i].y_min <<
    //                          ",\t y_max=" << consolidated_blobs[i].y_max <<
    //                          ",\t mass=" << consolidated_blobs[i].mass <<  endl;
    // }

    // find biggest blob
    short biggest_blob_index = 0;
    for (short i = 0; i < num_blobs; i++) {
        if (consolidated_blobs[i].mass > consolidated_blobs[biggest_blob_index].mass)  biggest_blob_index = i;
    }

    // fill biggest_blob with the info of the biggest blob (to be used by caller upon return)
    biggest_blob.label = consolidated_blobs[biggest_blob_index].label;
    biggest_blob.x_min = consolidated_blobs[biggest_blob_index].x_min;
    biggest_blob.x_max = consolidated_blobs[biggest_blob_index].x_max;
    biggest_blob.y_min = consolidated_blobs[biggest_blob_index].y_min;
    biggest_blob.y_max = consolidated_blobs[biggest_blob_index].y_max;
    biggest_blob.mass  = consolidated_blobs[biggest_blob_index].mass;
    biggest_blob.centroid_x = consolidated_blobs[biggest_blob_index].centroid_x;
    biggest_blob.centroid_y = consolidated_blobs[biggest_blob_index].centroid_y;

    // OPTIONAL: draw bounding box around biggest blob
    short x_min = biggest_blob.x_min;
    short x_max = biggest_blob.x_max;
    short y_min = biggest_blob.y_min;
    short y_max = biggest_blob.y_max;

    // OPTIONAL: draw centroid
    short centroid_x = biggest_blob.centroid_x;
    short centroid_y = biggest_blob.centroid_y;
    // cout << centroid_x << "," << centroid_y << endl;

    // OPTIONAL: magnify brightness differences and draw bounding boxes
    for (short row = 1; row < height-1; row++) {
        for (short col = 1; col < width-1; col++) {
            output[IDX(row, col)] = (output[IDX(row, col)] * 30) % 256;

            // bounding rails
            if ((col == x_min || col == x_max) && (row > y_min && row < y_max)) {
                output[IDX(row, col)] = 255;
            }
            if ((row == y_min || row == y_max) && (col > x_min && col < x_max)) {
                output[IDX(row, col)] = 255;
            }

            // centroid cross
            if (abs(col - centroid_x) < 2 || abs(row - centroid_y) < 2) {
                if (row > y_min+10 && row < y_max-10 && col > x_min+10 && col < x_max-10) {
                    output[IDX(row, col)] = 255;
                }
            }
        }
    }

    return num_blobs;
}
