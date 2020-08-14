/* include standard libraries */
#include <stdio.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <string.h>

/* include custom libraries */
#include "util.h"
#include "recognize-update.h"
#include "recognize-lib.h"

/* include capture libraries */
#include "display-kms.h"
#include "v4l2.h"
#include "vpe-common.h"

void update_recog_result(recog_arg* arg, recog_result* result) {
    // update sample
    if (result->sample.enabled) {
        // update example. I used memcpy function because return value is pointer.
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

int capture_recognize(recog_result* result, recog_arg* arg) {
    static struct v4l2* v4l2 = NULL;
    static struct vpe* vpe = NULL;
    static struct buffer** input_bufs;
    static bool init = false;
    
    static bool is_first = true;
    static int buf_idx;
    static struct buffer* buf_capt; // vpe output buffer of converted capture image.
    static unsigned char* cam_pbuf[4];

    if (!init) {
        /* Get VPE(input & output information) */
        if (!(vpe = vpe_open())) {
            ERROR("VPE open error");
            return -1;
        }
        vpe->src.width = CAPTURE_IMG_W;
        vpe->src.height = CAPTURE_IMG_H;
        describeFormat(CAPTURE_IMG_FORMAT, &vpe->src);
        vpe->dst.width = VPE_OUTPUT_W;
        vpe->dst.height = VPE_OUTPUT_H;
        describeFormat(VPE_OUTPUT_FORMAT, &vpe->dst);

        /* Open Display */
        int disp_argc = 3;
        char* disp_argv[] = { "dummy", "-s", "4:480x272", "\0" };
        if (!(vpe->disp = disp_open(disp_argc, disp_argv))) {
            ERROR("Display open error");
            vpe_close(vpe);
            return -1;
        }

        /* Get overlay display */
        set_z_order(vpe->disp, vpe->disp->overlay_p.id);
        set_global_alpha(vpe->disp, vpe->disp->overlay_p.id);
        set_pre_multiplied_alpha(vpe->disp, vpe->disp->overlay_p.id);
        alloc_overlay_plane(vpe->disp, OVERLAY_DISP_FORCC, 0, 0, OVERLAY_DISP_W, OVERLAY_DISP_H);

        /* Get V4L2 */
        if (!(v4l2 = v4l2_open(vpe->src.fourcc, vpe->src.width, vpe->src.height))) {
            ERROR("V4L2 open error");
            disp_close(vpe->disp);
            vpe_close(vpe);
            return -1;
        }

        /* Get V4L2 */
        vpe->translen = 1;
        vpe->field = V4L2_FIELD_ANY;

        /* Get V4L2 buffers via DMABUF */
        v4l2_reqbufs(v4l2, NUMBUF);

        /* Initialize VPE input */
        vpe_input_init(vpe);

        /* Allocate input buffers(Allocating shared buffer error) */
        input_bufs = calloc(NUMBUF, sizeof(struct buffer*));
        for (int i = 0; i < NUMBUF; i++) {
            input_bufs[i] = alloc_buffer(vpe->disp, vpe->src.fourcc,
                                         vpe->src.width, vpe->src.height, false);
        }
        if (!input_bufs) {
            ERROR("Allocating shared buffer error");
            return -1;
        }
        for (int i = 0; i < NUMBUF; i++) {
            // Get DMABUF fd for corresponding buffer object
            vpe->input_buf_dmafd[i] = omap_bo_dmabuf(input_bufs[i]->bo[0]);
            input_bufs[i]->fd[0] = vpe->input_buf_dmafd[i];
        }

        if (vpe->dst.coplanar) {
            vpe->disp->multiplanar = true;
        }
        else {
            vpe->disp->multiplanar = false;
        }

        /* Initialize allocate VPE output */
        vpe_output_init(vpe);
        vpe_output_fullscreen(vpe, true);   // true: display full screen
        
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
        
        buf_idx = vpe_output_dqbuf(vpe);
        buf_capt = vpe->disp_bufs[buf_idx];
    
        if (get_framebuf(buf_capt, cam_pbuf) == 0) {
            // copy camera output
            memcpy(arg->camera_output, cam_pbuf[0], VPE_OUTPUT_IMG_SIZE);
            // set display input
            arg->display_input = cam_pbuf[0];
            // udpate recognize result
            update_recog_result(arg, result);
        }

        if (disp_post_vid_buffer(vpe->disp, buf_capt, 0, 0, vpe->dst.width, vpe->dst.height)) {
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

int main(int argc, char** argv) {
    key_t shm_key, msgq_key_ctrlboard;
    int shm_id, msgq_id_ctrlboard;
    recog_result* shm_rr;

    /* Get the arguments from user(shell) */
    if (argc != 3) {
        printf("usage %s [shared memory key of this] [message queue key of ctrlboard] \n", argv[0]);
        return 1;
    }
    shm_key = (key_t)atoi(argv[1]);             // 1st argv: shared memory key of this process
    msgq_key_ctrlboard = (key_t)atoi(argv[2]);  // 2nd argv: message queue key of the ctrlboard process

    /* Initialize a shared memory of recognition results */
    if (get_shm_recog_result(shm_key, &shm_id, &shm_rr, 1) != 0) {
        ERROR("An error occurred while getting shared memory.");
        return -1;
    }
    
    /* Get message queue id of ctrlboard process */
    if ((msgq_id_ctrlboard = msgget(msgq_key_ctrlboard, 0)) == -1) {
        ERROR("An error occurred while getting the message queue id with the key %d"
              "Please check the ctrlboard process is running.", msgq_key_ctrlboard);
    }

    /* Do capture and recognize */
    recog_arg arg;
    arg.msgq_id_ctrlboard = msgq_id_ctrlboard;
    for (;;) {
        capture_recognize(shm_rr, &arg);
    }

    return 0;
}