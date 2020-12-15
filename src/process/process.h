#ifndef _PROCESS_H_
#define _PROCESS_H_
#include "config-car.h"
#include "ctrlboard-direct.h"
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
extern volatile recog_result *recog;

#endif