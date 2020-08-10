#include <pthread.h>
#include "recognize.h"
#include "util.h"
#include "carprop.h"
#include "carshm.h"

/* include update functions */
#include "update_sample.h"
#include "update_is_on_stop_line.h"

int init(void) {
    if (init_mmap != 0)
        return 1;
    if (init_uart() != 0)
        return 1;
    if (init_camera() != 0)
        return 1;
    if (init_dispaly() != 0)
        return 1;
}

int init_mmap(void) {
    // TODO: Initialize memory map for recognition_result
    return 0;
}

int init_uart(void) {
    // TODO: Initialize uart to communicate with control board
    return 0;
}

unsigned char* init_camera(void) {
    // TODO: Initialize camera and set camera_data
    return 0;
}

unsigned char* init_display(void) {
    // TODO: Initialize camera display
    // TODO: Initialize overlay display and set display_overlay_data
    return 0;
}

void unit_test(void) {
    // TODO: Review arguments. There are fatal errors.
    #ifdef UNIT_TEST_SAMPLE
    unit_test_sample(argc, argv);
    #endif

    #ifdef UNIT_TEST_IS_ON_STOP_LINE
    unit_test_is_on_stop_line(argc, argv);
    #endif
}

int main(void) {
    // TODO: Get an answer that how to share memories.

    /* Initialization Part */
    if (init()) {
        return -1;
    }

    /* Thread Part */
    pthread_t threads[METHOD_COUNT];
    void* (*update_methods[METHOD_COUNT])(void*) = { 
        update_sample,
        update_is_on_stop_line
    };
    struct recognize_thread_data thr_data;
    // TODO: Fill the fields of the thr_data

    for (int i = 0; i < METHOD_COUNT; i++) {
        if (pthread_create(&threads[0], NULL, update_sample, &thr_data)) {
            ERROR("Failed creating input thread");
            return 1;
        }
        pthread_detach(threads[0]);
    }

    /* Unit Test Part */
    unit_test();

    /* Pause main thread */
    pause();
    // TODO: Ready to recive exit message!

    return 0;
}