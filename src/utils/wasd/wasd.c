#include "ctrlboard-direct.h"
#include "recognize-shm.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>

#define CONSTRAIN(val, min, max)                                               \
    (((val) < (min)) ? (min) : (((max) < (val)) ? (max) : (val)))

#define DESIRE_SPEED           100
#define SPEED_PER_KEYINPUT     50
#define STEERING_PER_KEYINPUT  100
#define CAM_SERVO_PER_KEYINPUT 100
#define TICK_PER_CM            19.7628f

static int  getkey(int);
static void printh();

int main(int argc, char **argv) {
    // Initialize ctrlboard directly
    ctrld_init();
    ctrld_write(CMD_POSITION_CONTROL_ON_OFF, 0);
    ctrld_write(CMD_SPEED_CONTROL_ON_OFF, 1);
    ctrld_write(CMD_SPEED_PID_PROPORTIONAL, 4);
    ctrld_write(CMD_SPEED_PID_INTEGRAL, 20);
    ctrld_write(CMD_SPEED_PID_DIFFERENTAL, 20);
    ctrld_write(CMD_STEERING_SERVO_CONTROL, 1500);
    ctrld_write(CMD_CAMERA_X_SERVO_CONTROL, 1500);
    ctrld_write(CMD_CAMERA_Y_SERVO_CONTROL, 1500);
    printf("Initializing wasd process is success! \n");

    int  key_code;
    int  speed = 0, steering = 1500;
    int  cam_servo_x = 1500, cam_servo_y = 1500;
    bool gof = false, gob = false; // 주행 결정 요소

    unsigned int prev_encoder = 0, curr_encoder = 0;

    for (;;) {
        key_code = getkey(0); // block

        switch (key_code) {
        case 'w': // w (go forward)
            if (gob) {
                gob = false;
                break;
            }
            if (gof) {
                speed += SPEED_PER_KEYINPUT;
            } else {
                gof   = true;
                gob   = false;
                speed = DESIRE_SPEED;
            }
            break;
        case 's': // s (go back)
            if (gof) {
                gof = false;
                break;
            }
            if (gob) {
                speed -= SPEED_PER_KEYINPUT;
            } else {
                gob   = true;
                gof   = false;
                speed = -DESIRE_SPEED;
            }
            break;
        case 'a': // a (left handling)
            steering += STEERING_PER_KEYINPUT;
            break;
        case 'd': // d (right handling)
            steering -= STEERING_PER_KEYINPUT;
            break;
        case ';': // camera servo down
            cam_servo_y += CAM_SERVO_PER_KEYINPUT;
            break;
        case 'p': // camera servo up
            cam_servo_y -= CAM_SERVO_PER_KEYINPUT;
            break;
        case 'l': // camera servo left
            cam_servo_x += CAM_SERVO_PER_KEYINPUT;
            break;
        case '\'': // camera servo right
            cam_servo_x -= CAM_SERVO_PER_KEYINPUT;
            break;
        case '[': // start to record encoder counter
            ctrld_read(CMD_ENCODER_COUNTER, &prev_encoder);
            printf("Base point has been recorded! \n");
            printf("Press ']' key to show difference of encoder counter. \n");
            break;
        case ']': // show difference of encoder counter
            ctrld_read(CMD_ENCODER_COUNTER, &curr_encoder);
            int de = curr_encoder - prev_encoder;
            printf("Encoder => Tick: %d, Distance: %f (cm) \n", de,
                   de / TICK_PER_CM);
            break;
        case '\\': // show current state
            printf("Current State => SP: %+4d, ST: %4d, CX: %4d, CY: %4d \n",
                   speed, steering, cam_servo_x, cam_servo_y);
            break;
        case 'h':
            printh();
            break;
        }

        if (gob || gof) {
            speed = CONSTRAIN(speed, -500, 500);
        } else {
            speed = 0;
        }
        ctrld_write(CMD_DESIRE_SPEED, speed);

        steering = CONSTRAIN(steering, 1000, 2000);
        ctrld_write(CMD_STEERING_SERVO_CONTROL, steering);

        cam_servo_x = CONSTRAIN(cam_servo_x, 600, 2400);
        ctrld_write(CMD_CAMERA_X_SERVO_CONTROL, cam_servo_x);

        cam_servo_y = CONSTRAIN(cam_servo_y, 1200, 1800);
        ctrld_write(CMD_CAMERA_Y_SERVO_CONTROL, cam_servo_y);
    }

    printf("exit!");
    return 0;
}

static void printh() {
    printf("Usage: hit the keyboard! \n");
    printf("Key: \n");
    printf("  'w'   Go Forward.\n"
           "        Stops if previous key is 'S'.\n"
           "        Speed up if previous key is 'W'.\n");
    printf("  'a'   Steer the car to the left.\n");
    printf("  's'   Go Back.\n"
           "        Stops if previous key is 'W'.\n"
           "        Speed up if previous key is 'S'.\n");
    printf("  'd'   Steer the car to the right.\n");
    printf("  'p'   Turn up the camera servo.\n");
    printf("  'l'   Turn the camera servo to the left.\n");
    printf("  ';'   Turn down the camera servo.\n");
    printf("  '''   Turn the camera servo to the right.\n");
    printf("  '\\'   Show current states.\n");
    printf("  '['   Record current encoder counter.\n");
    printf("  ']'   Shows the difference of encoder counter between previous "
           "value.\n");
    printf("\n");
}

static int getkey(int is_echo) {
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
