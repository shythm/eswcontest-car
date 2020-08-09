#ifndef _UPDATE_IS_ON_STOP_LINE_H
#define _UPDATE_IS_ON_STOP_LINE_H

/* Write here "#define UNIT_TEST_IS_ON_STOP_LINE" to run the unit test. */

/*
 * Update is_on_stop_line field in the recognition_result structure.
 * This function is for thread.
 */
void* update_is_on_stop_line(void* arg);

#ifdef UNIT_TEST_IS_ON_STOP_LINE
/*
 * Run unit test for this method. You can enable this function by writing
 * "#define UNIT_TEST_IS_ON_STOP_LINE" at the top of the header file.
 */
int unit_test_is_on_stop_line(int argc, char** argv);
#endif

#endif /* _UPDATE_IS_ON_STOP_LINE_H */