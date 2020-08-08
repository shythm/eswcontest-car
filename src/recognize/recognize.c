#include "util.h"
#include "carprop.h"
#include "carmmap.h"

static int uart_fd;

int init_uart(void) {
    return 0;
}

int init_camera(void) {
    return 0;
}

int init_display(void) {
    return 0;
}

int main(void) {
    // Initialize
    init_uart();
    init_camera();
    init_dispaly();

    // For thread

    // For while
    while (1) {
        
    }

    return 0;
}

