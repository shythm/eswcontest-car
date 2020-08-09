#ifndef _RECOGNIZE_H
#define _RECOGNIZE_H

#define METHOD_COUNT 2

struct recognize_thread_data {
    int uart_fd;
    unsigned char* camera_output;
    unsigned char* display_overlay;
    struct recognition_result* recognition;
};

#endif /* _RECOGNIZE_H */