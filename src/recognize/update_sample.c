#include <stdlib.h>
#include <stdbool.h>
#include "update_sample.h"

unsigned char* get_sample(recog_arg* arg) {
    static unsigned char sample[SAMPLE_COUNT];
    static bool init = false;
    
    if (!init) {
        sample[0] = 100;
        sample[1] = 200;
        init = true;
    }

    sample[0] += 1;
    sample[1] += 2;

    return sample;
}