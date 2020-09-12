/* include standard libraries */
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* include custom libraries */
#include "ctrlboard-lib.h"
#include "recognize-lib.h"
#include "recognize-update.h"
#include "util.h"

/* include capture libraries */
#include "display-kms.h"
#include "v4l2.h"
#include "vpe-common.h"

void update_recog_result(recog_arg *arg, recog_result *result) {
    // update sample
    if (result->sample.enabled) {
        // update example. I used memcpy function because return value is
        // pointer.
        memcpy(&(result->sample.value), get_sample(arg), SAMPLE_COUNT);
    }

    // update is_on_stop_line
    if (result->is_on_stop_line.enabled) {
        // TODO: write your update function
    }

    // update is_on_end_point
    if (result->is_on_end_point.enabled) {
        // TODO: write your update function
    }

    // update traffic_light
    if (result->traffic_light.enabled) {
        // TODO: write your update function
    }

    // update lane
    if (result->lane.enabled) {
        // TODO: write your update function
    }

    // update is_on_lane
    if (result->is_on_lane.enabled) {
        // TODO: write your update function
    }

    // update is_on_slope
    if (result->is_on_slope.enabled) {
        // TODO: write your update function
    }

    // update is_on_overpass
    if (result->is_on_overpass.enabled) {
        // TODO: write your update function
    }

    // update is_in_tunnel
    if (result->is_in_tunnel.enabled) {
        // TODO: write your update function
    }

    // update curr_velocity
    if (result->curr_velocity.enabled) {
        // TODO: write your update function
    }

    // update stop_obstacle
    if (result->stop_obstacle.enabled) {
        // TODO: write your update function
    }

    // update is_there_car
    if (result->is_there_car.enabled) {
        // TODO: write your update function
    }
}

int capture_recognize(recog_result *result, recog_arg *arg) {
    static struct v4l2 *   v4l2 = NULL;
    static struct vpe *    vpe  = NULL;
    static struct buffer **input_bufs;
    static bool            init = false;

    static bool is_first = true;
    static int  buf_idx;
    static struct buffer
        *buf_capt; // vpe output buffer of converted capture image.
    static unsigned char *cam_pbuf[4];

    if (!init) {
        /* Get VPE(input & output information) */
        if (!(vpe = vpe_open())) {
            ERROR("VPE open error");
            return -1;
        }
        vpe->src.width  = CAPTURE_IMG_W;
        vpe->src.height = CAPTURE_IMG_H;
        describeFormat(CAPTURE_IMG_FORMAT, &vpe->src);
        vpe->dst.width  = VPE_OUTPUT_W;
        vpe->dst.height = VPE_OUTPUT_H;
        describeFormat(VPE_OUTPUT_FORMAT, &vpe->dst);

        /* Open Display */
        int   disp_argc   = 3;
        char *disp_argv[] = {"dummy", "-s", "4:480x272", "\0"};
        if (!(vpe->disp = disp_open(disp_argc, disp_argv))) {
            ERROR("Display open error");
            vpe_close(vpe);
            return -1;
        }

        /* Get overlay display */
        set_z_order(vpe->disp, vpe->disp->overlay_p.id);
        set_global_alpha(vpe->disp, vpe->disp->overlay_p.id);
        set_pre_multiplied_alpha(vpe->disp, vpe->disp->overlay_p.id);
        alloc_overlay_plane(vpe->disp, OVERLAY_DISP_FORCC, 0, 0, OVERLAY_DISP_W,
                            OVERLAY_DISP_H);

        /* Get V4L2 */
        if (!(v4l2 = v4l2_open(vpe->src.fourcc, vpe->src.width,
                               vpe->src.height))) {
            ERROR("V4L2 open error");
            disp_close(vpe->disp);
            vpe_close(vpe);
            return -1;
        }

        /* Get V4L2 */
        vpe->translen = 1;
        vpe->field    = V4L2_FIELD_ANY;

        /* Get V4L2 buffers via DMABUF */
        v4l2_reqbufs(v4l2, NUMBUF);

        /* Initialize VPE input */
        vpe_input_init(vpe);

        /* Allocate input buffers(Allocating shared buffer error) */
        input_bufs = calloc(NUMBUF, sizeof(struct buffer *));
        for (int i = 0; i < NUMBUF; i++) {
            input_bufs[i] =
                alloc_buffer(vpe->disp, vpe->src.fourcc, vpe->src.width,
                             vpe->src.height, false);
        }
        if (!input_bufs) {
            ERROR("Allocating shared buffer error");
            return -1;
        }
        for (int i = 0; i < NUMBUF; i++) {
            // Get DMABUF fd for corresponding buffer object
            vpe->input_buf_dmafd[i] = omap_bo_dmabuf(input_bufs[i]->bo[0]);
            input_bufs[i]->fd[0]    = vpe->input_buf_dmafd[i];
        }

        if (vpe->dst.coplanar) {
            vpe->disp->multiplanar = true;
        } else {
            vpe->disp->multiplanar = false;
        }

        /* Initialize allocate VPE output */
        vpe_output_init(vpe);
        vpe_output_fullscreen(vpe, true); // true: display full screen

        for (int i = 0; i < NUMBUF; i++) {
            v4l2_qbuf(v4l2, vpe->input_buf_dmafd[i], i);
        }
        for (int i = 0; i < NUMBUF; i++) {
            vpe_output_qbuf(vpe, i);
        }

        v4l2_streamon(v4l2);
        vpe_stream_on(vpe->fd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);

        init = true;
    } else {
        buf_idx = v4l2_dqbuf(v4l2, &vpe->field);
        vpe_input_qbuf(vpe, buf_idx);

        if (is_first) {
            vpe_stream_on(vpe->fd, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
            is_first = false;
        }

        buf_idx  = vpe_output_dqbuf(vpe);
        buf_capt = vpe->disp_bufs[buf_idx];

        if (get_framebuf(buf_capt, cam_pbuf) == 0) {
            // copy camera output
            memcpy(arg->camera_output, cam_pbuf[0], VPE_OUTPUT_IMG_SIZE);
            // set display input
            arg->display_input = cam_pbuf[0];
            // udpate recognize result
            update_recog_result(arg, result);
        }

        if (disp_post_vid_buffer(vpe->disp, buf_capt, 0, 0, vpe->dst.width,
                                 vpe->dst.height)) {
            ERROR("Display post video buffer failed");
            return -1;
        }
        update_overlay_disp(vpe->disp);

        vpe_output_qbuf(vpe, buf_idx);
        buf_idx = vpe_input_dqbuf(vpe);
        v4l2_qbuf(v4l2, vpe->input_buf_dmafd[buf_idx], buf_idx);
    }

    return 0;
}

/* define I2C & PSD constants */
#define PSD_I2C_DEVICE   "/dev/i2c-2"
#define PSD_I2C_BUF_SIZE 8
#define PSD_I2C_DELAY_US 2000

#define PSD_CMD_FRONT   0x8C
#define PSD_CMD_RIGHT_1 0xCC
#define PSD_CMD_RIGHT_2 0x9C
#define PSD_CMD_BACK    0xDC
#define PSD_CMD_LEFT_2  0xAC
#define PSD_CMD_LEFT_1  0xEC

#define PSD_DISTANCE_MIN 4.0f
#define PSD_DISTANCE_MAX 30.0f

#define PSD_MEDIAN_SAMPLE_SIZE 11

void bubble_sort(uint16_t *arr, int length) {
    int      i, j;
    uint16_t temp;

    for (i = 0; i < length - 1; i++) {
        for (j = 0; j < length - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                temp       = arr[j];
                arr[j]     = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

void *update_psd_value(void *argv) {
    /* get the shared memory from the pthread argument */
    recog_result *result = (recog_result *)argv;

    /* these are for I2C communication */
    int                 i, j, i2c_fd = -1;
    const unsigned char psd_channel[PSD_COUNT] = {
        PSD_CMD_FRONT, PSD_CMD_RIGHT_1, PSD_CMD_RIGHT_2,
        PSD_CMD_BACK,  PSD_CMD_LEFT_2,  PSD_CMD_LEFT_1};
    const size_t  buf_read_size = 2;
    unsigned char buf_read[PSD_I2C_BUF_SIZE];

    /* these are the data containers of the PSD value */
    uint16_t   psd_samples[PSD_COUNT][PSD_MEDIAN_SAMPLE_SIZE];
    uint16_t   psd_raw[PSD_COUNT];
    psd_data_t psd_dist[PSD_COUNT];

    /* I2C Initialization Part */
    if ((i2c_fd = open(PSD_I2C_DEVICE, O_RDWR)) < 0) {
        ERROR("Failed to open I2C for PSD.");
        return NULL;
    }
    if (ioctl(i2c_fd, I2C_SLAVE, 0x4b) < 0) {
        ERROR("Failed to ioctl I2C for PSD.");
        return NULL;
    }
    MSG("I2C Device for PSD(%s) has been initialized.", PSD_I2C_DEVICE);

    /* Update PSD Value Part */
    for (;;) {
        // Fisrt, Communicate with the I2C Device
        for (i = 0; i < PSD_COUNT; i++) {
            for (j = 0; j < PSD_MEDIAN_SAMPLE_SIZE; j++) {
                // command to i2c
                write(i2c_fd, psd_channel + i, 1);
                usleep(PSD_I2C_DELAY_US);

                // get the psd raw value from i2c
                if (read(i2c_fd, buf_read, buf_read_size) != buf_read_size) {
                    ERROR("Failed to read PSD data from I2C.");
                    return NULL;
                }
                usleep(PSD_I2C_DELAY_US);

                // save psd samples
                psd_samples[i][j] =
                    ((buf_read[0] & 0b00001111) << 8) + buf_read[1];
            }

            // sort the samples for applying median filter.
            bubble_sort(psd_samples[i], PSD_MEDIAN_SAMPLE_SIZE);
            // store the result of median filter of the psd samples
            psd_raw[i] = psd_samples[i][PSD_MEDIAN_SAMPLE_SIZE / 2];
        }

        // Second, apply the function of PSD raw data to distance data(cm)
        psd_dist[PSD_FRONT] = 51.83f * expf(-0.001981f * psd_raw[PSD_FRONT]) +
                              17.8f * expf(-0.0004166f * psd_raw[PSD_FRONT]);
        psd_dist[PSD_RIGHT_1] =
            52.04f * expf(-0.001964f * psd_raw[PSD_RIGHT_1]) +
            18.16f * expf(-0.0003931f * psd_raw[PSD_RIGHT_1]);
        psd_dist[PSD_RIGHT_2] =
            51.58f * expf(-0.001936f * psd_raw[PSD_RIGHT_2]) +
            17.79f * expf(-0.0003686f * psd_raw[PSD_RIGHT_2]);
        psd_dist[PSD_BACK] = 57.7f * expf(-0.002206f * psd_raw[PSD_BACK]) +
                             19.1f * expf(-0.0004304f * psd_raw[PSD_BACK]);
        psd_dist[PSD_LEFT_2] = 490.1f * expf(-0.004111f * psd_raw[PSD_LEFT_2]) +
                               24.61f * expf(-0.0004845f * psd_raw[PSD_LEFT_2]);
        psd_dist[PSD_LEFT_1] = 638.6f * expf(-0.004488f * psd_raw[PSD_LEFT_1]) +
                               26.45f * expf(-0.000508f * psd_raw[PSD_LEFT_1]);

        for (i = 0; i < PSD_COUNT;
             i++) { // limit the psd value (PSD_DISTANCE_MIN <= psd_dist <=
                    // PSD_DISTANCE_MAX)
            if (psd_dist[i] <= PSD_DISTANCE_MIN) psd_dist[i] = PSD_DISTANCE_MIN;
            else if (psd_dist[i] >= PSD_DISTANCE_MAX)
                psd_dist[i] = PSD_DISTANCE_MAX;
        }

        // Third, update the psd value of the shared memory
        memcpy(result->psd.value, psd_dist, sizeof(psd_data_t) * PSD_COUNT);
    }
}

int main(int argc, char **argv) {
    recog_result *shm_rr;
    pthread_t     thread_update_psd_value;

    /* Initialize a shared memory of recognition results */
    if (get_shm_recog_result(&shm_rr, 1) != 0) {
        ERROR("An error occurred while getting shared memory.");
        return -1;
    }

    /* Get PSD data from I2C by thread */
    if (pthread_create(&thread_update_psd_value, NULL, update_psd_value,
                       shm_rr)) {
        ERROR("An error occurred while creating thread for update_psd_value.");
        return -1;
    }
    pthread_detach(thread_update_psd_value);

    /* Do capture and recognize */
    recog_arg arg;

    // Get message queue id of ctrlboard process
    if (get_msgq_id_ctrlboard(&(arg.msgq_id_ctrlboard), 0) == -1) {
        ERROR("An error occurred while getting the message queue id. Check "
              "that the ctrlboard process is running");
        return -1;
    }

    for (;;) {
        capture_recognize(shm_rr, &arg);
    }

    return 0;
}