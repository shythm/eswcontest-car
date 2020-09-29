#include "ctrlboard-lib.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>

typedef struct _thread_data {
    mqid_ctrl ctrl;
} thread_data;

int getkey(int is_echo) {
    int            ch;
    struct termios old;
    struct termios current;

    /* 현재 설정된 terminal i/o 값을 backup함 */
    tcgetattr(0, &old);

    /* 현재의 설정된 terminal i/o에 일부 속성만 변경하기 위해 복사함 */
    current = old;

    /* buffer i/o를 중단함 */
    current.c_lflag &= ~ICANON;
    if (is_echo) {
        // 입력값을 화면에 표시할 경우
        current.c_lflag |= ECHO;
    } else {
        // 입력값을 화면에 표시하지 않을 경우
        current.c_lflag &= ~ECHO;
    }

    /* 변경된 설정값으로 설정합니다.*/
    tcsetattr(0, TCSANOW, &current);
    ch = getchar();
    tcsetattr(0, TCSANOW, &old);
    return ch;
}

void *control_thread(void *argv) {
    int                      msg_id = 'w' + 'a' + 's' + 'd';
    int                      ret;
    thread_data *            thr_data = (thread_data *)argv;
    ctrlboard_byte_container container;
    short                    speed, steering, cam_servo_x, cam_servo_y;
    unsigned char            gain;
    int                      desire_encoder, key_code;

    // SpeedControlOnOff_Write(CONTROL);
    container.c_uint8 = 1;
    if ((ret = comm_ctrlboard(thr_data->ctrl, msg_id, CMD_SPEED_CONTROL_ON_OFF,
                              CMD_TYPE_WRITE, 1, &container)) ==
        MSG_STATE_SUCCESS) {
        printf("Speed Control ON \n");
    } else {
        printf("Failed to set speed control to ON (Errno: %d) \n", ret);
        return NULL;
    }

    // DesireSpeed_Write(speed);
    speed             = 500;
    container.c_int16 = speed;
    if ((ret = comm_ctrlboard(thr_data->ctrl, msg_id, CMD_DESIRE_SPEED,
                              CMD_TYPE_WRITE, 2,
                              &container) == MSG_STATE_SUCCESS)) {
        printf("Desire Speed: %d \n", speed);
    } else {
        printf("Failed to set desire speed (Errno: %d) \n", ret);
        return NULL;
    }

    // PositionControlOnOff_Write(CONTROL);
    container.c_uint8 = 1;
    if ((ret = comm_ctrlboard(thr_data->ctrl, msg_id,
                              CMD_POSITION_CONTROL_ON_OFF, CMD_TYPE_WRITE, 1,
                              &container) == MSG_STATE_SUCCESS)) {
        printf("Position Control ON \n");
    } else {
        printf("Failed to set position control to ON (Errno: %d) \n", ret);
        return NULL;
    }

    // PositionProportionPoint_Write(gain);
    gain              = 7;
    container.c_uint8 = gain;
    if ((ret = comm_ctrlboard(thr_data->ctrl, msg_id,
                              CMD_POSITION_PROPORTION_POINT, CMD_TYPE_WRITE, 1,
                              &container) == MSG_STATE_SUCCESS)) {
        printf("Position Proportion Point: %d \n", gain);
    } else {
        printf("Failed to set position proportion point (Errno: %d) \n", ret);
        return NULL;
    }

    // Initialization to servo motor
    steering          = 1500;
    container.c_int16 = steering;
    if ((ret = comm_ctrlboard(thr_data->ctrl, msg_id,
                              CMD_STEERING_SERVO_CONTROL, CMD_TYPE_WRITE, 2,
                              &container) == MSG_STATE_SUCCESS)) {
        printf("Steering Servo Control: %d \n", steering);
    } else {
        printf("Failed to set steering servo control (Errno: %d) \n", ret);
        return NULL;
    }

    // set camera servo x
    cam_servo_x       = 1500;
    container.c_int16 = cam_servo_x;
    if ((ret = comm_ctrlboard(thr_data->ctrl, msg_id,
                              CMD_CAMERA_X_SERVO_CONTROL, CMD_TYPE_WRITE, 2,
                              &container) == MSG_STATE_SUCCESS)) {
        printf("Set camera servo x: %d \n", cam_servo_x);
    } else {
        printf("Failed to set camera servo x (Errno: %d) \n", ret);
        return NULL;
    }

    // set camera servo y
    cam_servo_y       = 1500;
    container.c_int16 = cam_servo_y;
    if ((ret = comm_ctrlboard(thr_data->ctrl, msg_id,
                              CMD_CAMERA_Y_SERVO_CONTROL, CMD_TYPE_WRITE, 2,
                              &container) == MSG_STATE_SUCCESS)) {
        printf("Set camera servo y: %d \n", cam_servo_y);
    } else {
        printf("Failed to set camera servo y (Errno: %d) \n", ret);
        return NULL;
    }

    printf("Initializing wasd process is success! \n");

    static long count = 0;

    for (;;) {
        desire_encoder = 0;
        /* Set the encoder count to zero(0) */
        container.c_int32 = 0;
        send_ctrlboard(thr_data->ctrl, CMD_ENCODER_COUNTER, 4, &container);
        key_code = getkey(0);
        count++;
        printf("cnt: %d \n", count);

        switch (key_code) {
        case 'w': // w (go forward)
            desire_encoder += 200;
            break;
        case 's': // s (go back)
            desire_encoder -= 200;
            break;
        case 'a': // a (left handling)
            steering += 100;
            if (steering > 2000) steering = 2000;
            break;
        case 'd': // d (right handling)
            steering -= 100;
            if (steering < 1000) steering = 1000;
            break;
        case ';': // camera servo down
            cam_servo_y += 50;
            if (cam_servo_y > 1800) cam_servo_y = 1800;
            break;
        case 'p': // camera servo up
            cam_servo_y -= 50;
            if (cam_servo_y < 1200) cam_servo_y = 1200;
            break;
        case 'l': // camera servo left
            cam_servo_x += 50;
            if (cam_servo_x > 2400) cam_servo_x = 2400;
            break;
        case '\'': // camera servo right
            cam_servo_x -= 50;
            if (cam_servo_x < 600) cam_servo_x = 600;
            break;
        }
        // Desire Encoder Count
        container.c_int32 = desire_encoder;
        send_ctrlboard(thr_data->ctrl, CMD_DESIRE_ENCODER_COUNT, 4, &container);

        // Steering Servo
        container.c_int16 = steering;
        send_ctrlboard(thr_data->ctrl, CMD_STEERING_SERVO_CONTROL, 2,
                       &container);

        // Set camera sevro x
        container.c_int16 = cam_servo_x;
        send_ctrlboard(thr_data->ctrl, CMD_CAMERA_X_SERVO_CONTROL, 2,
                       &container);

        // Set camera servo y
        container.c_int16 = cam_servo_y;
        send_ctrlboard(thr_data->ctrl, CMD_CAMERA_Y_SERVO_CONTROL, 2,
                       &container);
    }
}

int main(int argc, char **argv) {
    int         ret;
    pthread_t   thr_id[1];
    thread_data thr_data;
    key_t       msgq_key;

    if (get_mqid_ctrl(&thr_data.ctrl) == -1) {
        printf("Failed get message queue id!");
        return 1;
    }

    ret = pthread_create(&thr_id[0], NULL, control_thread, (void *)&thr_data);
    if (ret) {
        printf("Failed creating control thread");
        return 1;
    }
    pthread_join(thr_id[0], NULL);

    return 0;
}