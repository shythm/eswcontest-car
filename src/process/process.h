#ifndef _PROCESS_H_
#define _PROCESS_H_
#include "config-car.h"
#include "ctrlboard-lib.h"
#include "recognize-lib.h"
#include "util.h"
#include <stdbool.h>

#define MODE_PRACTICE

typedef void (*fnClean_t)(void);
typedef void (*fnRun_t)(fnClean_t *);
typedef bool (*fnCheck_t)(fnRun_t *);
typedef void (*fnInit_t)(fnCheck_t *);

void init_drive();
void do_drive();

/* `recog' is global variable that is externed by process.h and initialized by
 * process.c. It stores the reocgnition results. */
extern recog_result *recog;

/* `ctrl' is global variable that is externed by process.h and initialized by
 * process.c. It stores message queue ids of ctrlboard process to communicate
 * with control board hardware. */
extern mqid_ctrl ctrl;

ctrlboard_msg_state_t command(ctrlboard_cmd_code cmd, int data);

#endif