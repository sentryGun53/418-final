/*

http://www.labbookpages.co.uk/software/imgProc/blobDetection.html

*/

#define IDX(R, C)   ((((R) * width) + (C)) * 3)


unsigned char blob_detect(unsigned char *output, unsigned char *input, int width, int height) {

    unsigned char num_blobs = 0;
    unsigned char label_counter = 10;

    for (int row = 1; row < height-1; row++) {
        for (int col = 1; col < width-1; col++) {

            // if our pixel is active, check the neighboring pixels to see if they've been labelled already
            if (input[IDX(row, col)] > 0) {

                unsigned char new_label = 0;

                unsigned char label_up_left  = output[IDX( row-1, col-1 )];
                unsigned char label_up       = output[IDX( row-1, col   )];
                unsigned char label_up_right = output[IDX( row-1, col+1 )];
                unsigned char label_left     = output[IDX( row,   col-1 )];

                // if all neighbors are 0, we give this pixel a new label
                if (label_up_left==0 && label_up==0 && label_up_right==0 && label_left==0) {
                    new_label = label_counter;
                    label_counter += 30;
                    num_blobs++;
                }
                else {  // one or more neighbors are already labelled: use the minimum label

                    // find the minimum label out of this group
                    unsigned char minimum_label = 255;
                    if (label_up_left  != 0)  minimum_label = min(minimum_label, label_up_left);
                    if (label_up       != 0)  minimum_label = min(minimum_label, label_up);
                    if (label_up_right != 0)  minimum_label = min(minimum_label, label_up_right);
                    if (label_left     != 0)  minimum_label = min(minimum_label, label_left);

                    // label all of the active neighbor pixels with the new label
                    if (label_up_left  != 0)  output[IDX( row-1, col-1 )] = minimum_label;
                    if (label_up       != 0)  output[IDX( row-1, col   )] = minimum_label;
                    if (label_up_right != 0)  output[IDX( row-1, col+1 )] = minimum_label;
                    if (label_left     != 0)  output[IDX( row,   col-1 )] = minimum_label;

                    // label the current pixel with the new label
                    new_label = minimum_label;
                }

                // write results to output
                output[IDX(row, col)] = new_label;

            }
        }
    }
    return num_blobs;
}
