/* Blob Detection */

// http://www.labbookpages.co.uk/software/imgProc/blobDetection.html

#include <omp.h>

#define IDX(R, C)   ((((R) * width) + (C)))

#include <stdlib.h>

struct blob {
    int label;
    int x_min;
    int x_max;
    int y_min;
    int y_max;
    int centroid_x;
    int centroid_y;
    unsigned long sum_x;
    unsigned long sum_y;
    int mass;
};

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

int blob_detect(struct blob &biggest_blob, unsigned short *output, unsigned char *input, short width, short height) {

    // begin labels at 1 so all relevant labels are nonzero.
    int label_counter = 1;

    // our list of blobs. TODO: better calculation of size here
    struct blob blobs[width * height / 4] = {0};

    // iterate through image
    for (int row = 1; row < height-1; row++) {
        for (int col = 1; col < width-1; col++) {

            /* we care about the pixel above, to the left, above-left, and above-right.
             * pixels are labelled as follows (X = current pixel):
             *      A B C
             *      D X
             */

            output[IDX(row, col)] = 0;

            // label that will be assigned to the current pixel. calculated below
            int lbl_x = 0;

            // if our pixel is active, check the neighboring pixels to determine it's label
            if (input[IDX(row, col)] > 0) {

                // get the labels for pixels a, b, c, d  - these have already been calculated.
                int lbl_a = output[IDX(row-1, col-1)];
                int lbl_b = output[IDX(row-1, col  )];
                int lbl_c = output[IDX(row-1, col+1)];
                int lbl_d = output[IDX(row,   col-1)];

                // find the minimum label out of all the neighbors, by checking
                // each label and updating min_label if we find a new minimum
                int min_label = 255;
                if (lbl_a != 0 && lbl_a < min_label)  min_label = lbl_a;
                if (lbl_b != 0 && lbl_b < min_label)  min_label = lbl_b;
                if (lbl_c != 0 && lbl_c < min_label)  min_label = lbl_c;
                if (lbl_d != 0 && lbl_d < min_label)  min_label = lbl_d;

                // no neighbors found, so we start a new blob on this pixel.
                if (min_label == 255) {
                    lbl_x = label_counter;         // get a new label and use it for this pixel
                    label_counter += 1;            // increment for use later
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
    for (int i = 1; i < label_counter; i++) {
        if (blobs[i].label != i) {
            blobs[i].label = blobs[blobs[i].label].label;
        }
    }

    // merge blobs that touch, put resulting list of blobs into consolidated_blobs[]
    struct blob consolidated_blobs[label_counter];
    int num_blobs = 0;

    // go thorugh the un-merged list backwards, and merge towards beginning
    for (int i = label_counter - 1; i > 0; i--) {

        // if this blob's label points to a different blob, merge with the lower blob
        if (blobs[i].label != i) {

            // which blob to merge with
            int d = blobs[i].label;

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
    for (int row = 1; row < height-1; row++) {
        for (int col = 1; col < width-1; col++) {
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
    int biggest_blob_index = 0;
    for (int i = 0; i < num_blobs; i++) {
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
    int x_min = biggest_blob.x_min;
    int x_max = biggest_blob.x_max;
    int y_min = biggest_blob.y_min;
    int y_max = biggest_blob.y_max;

    // OPTIONAL: draw centroid
    int centroid_x = biggest_blob.centroid_x;
    int centroid_y = biggest_blob.centroid_y;
    // cout << centroid_x << "," << centroid_y << endl;

    // OPTIONAL: magnify brightness differences and draw bounding boxes
    for (int row = 1; row < height-1; row++) {
        for (int col = 1; col < width-1; col++) {
            output[IDX(row, col)] = output[IDX(row, col)] * 30;

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
