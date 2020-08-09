#ifndef _UPDATE_SAMPLE_H
#define _UPDATE_SAMPLE_H

/* Write here "#define UNIT_TEST_SAMPLE" to run the unit test. */
// #define UNIT_TEST_SAMPLE

/*
 * Update sample field in the recognition_result structure.
 * This function is for thread.
 */
void* update_sample(void* arg);

#ifdef UNIT_TEST_SAMPLE
/*
 * Run unit test for this method. You can enable this function by writing
 * "#define UNIT_TEST_SAMPLE" at the top of the header file.
 */
int unit_test_sample(int argc, char** argv);
#endif

#endif /* _UPDATE_SAMPLE_H */