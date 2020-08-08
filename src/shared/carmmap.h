#ifndef _CARMMAP_H
#define _CARMMAP_H

#include <stdbool.h>
#include "carprop.h"

// memory map structure for camera data
struct camera_mmap
{
    unsigned char data[3][CAMERA_OUTPUT_W][CAMERA_OUTPUT_H];
};

/*
// memory map structure for display data
struct display_mmap {

};
*/

/* 
 * This is the memory map structure for results of recognition processing.
 * You can add some fields to share the output of the function which you have been made.
 */
struct recognition_result
{
    // add result(output) of function
    bool is_stop_line;
};

#endif /* _CARMMAP_H */
