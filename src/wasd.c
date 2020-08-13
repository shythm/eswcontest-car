#include <stdio.h>
#include <termios.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include "ctrlboard.h"

key_t msgq_key;
int msgq_id;

int getkey(int is_echo) { 
    int ch; 
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

void position_control(unsigned char speed, unsigned char gain) {
    int keyCode, position, steering, distance;
    ctrlboard_msg msg;
    size_t size = sizeof(ctrlboard_msg) - sizeof(long);

    // Initialization to control position
    msg.msgid = 100;

    // SpeedControlOnOff_Write(CONTROL);
    msg.cmd.code = CMD_SPEED_CONTROL_ON_OFF;
    msg.cmd.rw = CMD_TYPE_WRITE;
    msg.cmd.bytec = 1;
    msg.bytes[0] = 1;
    if (msgsnd(msgq_id, &msg, size, 0) == 0) {
        if (msgrcv(msgq_id, &msg, size, msg.msgid, 0) >= 0) {
            if (msg.state == MSG_STATE_SUCCESS) {
                printf("Speed Control ON! \n");
            } else {
                printf("Failed to speed control \n");
                return -1;
            }
        }
    }

    // DesireSpeed_Write(speed);
    msg.cmd.code = CMD_DESIRE_SPEED;
    msg.cmd.rw = CMD_TYPE_WRITE;
    msg.cmd.bytec = 2;
    msg.bytes[0] = speed & 0xff;
    msg.bytes[1] = (speed >> 8) & 0xff;
    if (msgsnd(msgq_id, &msg, size, 0) == 0) {
        if (msgrcv(msgq_id, &msg, size, msg.msgid, 0) >= 0) {
            if (msg.state == MSG_STATE_SUCCESS) {
                printf("Set speed to %d \n", speed);
            } else {
                printf("Failed to set speed \n");
                return -1;
            }
        }
    }

    // PositionControlOnOff_Write(CONTROL);
    msg.cmd.code = CMD_POSITION_CONTROL_ON_OFF;
    msg.cmd.rw = CMD_TYPE_WRITE;
    msg.cmd.bytec = 1;
    msg.bytes[0] = 1;
    if (msgsnd(msgq_id, &msg, size, 0) == 0) {
        if (msgrcv(msgq_id, &msg, size, msg.msgid, 0) >= 0) {
            if (msg.state == MSG_STATE_SUCCESS) {
                printf("Set position control to ON \n");
            } else {
                printf("Failed to set position control \n");
                return -1;
            }
        }
    }

    // PositionProportionPoint_Write(gain);
    msg.cmd.code = CMD_POSITION_PROPORTION_POINT;
    msg.cmd.rw = CMD_TYPE_WRITE;
    msg.cmd.bytec = 1;
    msg.bytes[0] = gain;
    if (msgsnd(msgq_id, &msg, size, 0) == 0) {
        if (msgrcv(msgq_id, &msg, size, msg.msgid, 0) >= 0) {
            if (msg.state == MSG_STATE_SUCCESS) {
                printf("Set position proportion to %d \n", gain);
            } else {
                printf("Failed to set position proportion \n");
                return -1;
            }
        }
    }

    char bytes[4];

    // Initialization to servo motor
    steering = 1500;
    bytes[0] = steering & 0xff;
    bytes[1] = (steering >> 8) & 0xff;
    // SteeringServoControl_Write(steering);
    if (message_ctrlboard(msgq_id, 100,
                          CMD_STEERING_SERVO_CONTROL,
                          CMD_TYPE_WRITE,
                          2,
                          bytes) == MSG_STATE_SUCCESS) {
        printf("Set steering servo control to %d \n", steering);
    } else {
        printf("Fail to set steering servo control \n");
        return -1;
    };

    distance = 200;
    while (1) {
        position = 0;
        // EncoderCounter_Write(position);
        bytes[0] = position & 0xff;
        bytes[1] = (position >> 8) & 0xff;
        bytes[2] = (position >> 16) & 0xff;
        bytes[3] = (position >> 24) & 0xff;
        message_ctrlboard(msgq_id, 100, CMD_ENCODER_COUNTER, CMD_TYPE_WRITE, 4, bytes);
        keyCode = getkey(0);
        
        switch (keyCode) {
        case 'w': // w (go forward)
            position += distance;
            break;
        case 's': // s (go back)
            position -= distance;
            break;
        case 'a': // a (left handling)
            steering += 100;
            if (steering > 2000)
                steering = 2000;
            break;
        case 'd': // d (right handling)
            steering -= 100;
            if (steering < 1000)
                steering = 1000;
            break;
        }

        // DesireEncoderCount_Write(position);
        bytes[0] = position & 0xff;
        bytes[1] = (position >> 8) & 0xff;
        bytes[2] = (position >> 16) & 0xff;
        bytes[3] = (position >> 24) & 0xff;
        message_ctrlboard(msgq_id, 100, CMD_DESIRE_ENCODER_COUNT, CMD_TYPE_WRITE, 4, bytes);

        bytes[0] = steering & 0xff;
        bytes[1] = (steering >> 8) & 0xff;
        // SteeringServoControl_Write(steering);
        if (message_ctrlboard(msgq_id, 100,
                            CMD_STEERING_SERVO_CONTROL,
                            CMD_TYPE_WRITE,
                            2,
                            bytes) == MSG_STATE_SUCCESS) {
            printf("Set steering servo control to %d \n", steering);
        } else {
            printf("Fail to set steering servo control \n");
            return -1;
        };
    }
}

void* control_thread(void* argv) {
    position_control(255, 10);
}

int main(int argc, char** argv) {
    int ret;
    pthread_t thr_data[2];

    if (argc != 2) {
        printf("Usage %s: [Message queue key of ctrlboard process] \n", argv[0]);
    }
    msgq_key = (key_t)atoi(argv[1]);

    ret = pthread_create(&thr_data[0], NULL, control_thread, NULL);
    if (ret) {
        printf("Failed creating control thread");
        return 1;
    }

    pthread_join(thr_data[0], NULL);

    return 0;
}