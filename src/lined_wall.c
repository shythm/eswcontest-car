#include <stdio.h>
#include <termios.h>
#include <pthread.h>
#include <unistd.h>
//#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "ctrlboard-lib.h"
#include "recognize-lib.h"

#define MAX_SPEED 100
#define MAX_POS_X 320
#define SLEEP_TIME_STEERING_CONTROL 10000 //[us]
#define SLEEP_TIME_LINING_WALL 100000 //[us]



key_t msgq_key;
key_t msgq_key;
key_t shm_key =0; // not hello-kitty
int shm_id;
int msgq_id;
int steering=1500;
int steer_flag =0;

recog_result * prr;

typedef struct _thread_data {
    int msgq_id;
} thread_data;

void get_recog_result(void){   
    if(get_shm_recog_result(&prr, 0)!=0){
        printf("Failed get_shm_recof_result\n");
    }
}

void* steering_control(void *argv){

    int msg_id = 'w'+'a'+'l'+'l';
    int ret;
    thread_data* thr_data = (thread_data*)argv;
    ctrlboard_byte_container container;
    short steering;
    float psd_distance;

    while(!steer_flag)
    usleep(1000);
    while(1){
       
        psd_distance = prr->psd.value[PSD_RIGHT_1];
        // printf("psd_data: %f\n", psd_distance);

        steering = -(1000.f/20.f)*(psd_distance-30.f)+1000.f;

        if(steering >2000) steering = 2000;
        else if (steering<1000) steering = 1000;

        // Steering Servo-
        ret = message_ctrlboard(thr_data->msgq_id, msg_id,
                                CMD_STEERING_SERVO_CONTROL, CMD_TYPE_WRITE,
                                2, &container);
        if (ret != MSG_STATE_SUCCESS) {
            printf("Err(%d): Failed to steering servo motor \n", ret);
        }

        usleep(SLEEP_TIME_STEERING_CONTROL);
    }
}
void* lining_wall(void *argv){

    int msg_id = 'w'+'a'+'l'+'l';
    int ret;
    thread_data* thr_data = (thread_data*)argv;
    ctrlboard_byte_container container;
    short speed, steering;
    unsigned char gain;
    int desire_encoder, key_code;
    float psd_distance;

    container.c_uint8 = 1;
    if ((ret = message_ctrlboard(thr_data->msgq_id, msg_id,
                                 CMD_SPEED_CONTROL_ON_OFF, CMD_TYPE_WRITE,
                                 1, &container)) == MSG_STATE_SUCCESS) {
        printf("Speed Control ON \n");
    } else {
        printf("Failed to set speed control to ON (Errno: %d) \n", ret);
        return NULL;
    }

    // DesireSpeed_Write(speed);
    speed = 100;
    container.c_int16 = speed;
    if ((ret = message_ctrlboard(thr_data->msgq_id, msg_id,
                          CMD_DESIRE_SPEED, CMD_TYPE_WRITE,
                          2, &container) == MSG_STATE_SUCCESS)) {
        printf("Desire Speed: %d \n", speed);
    } else {
        printf("Failed to set desire speed (Errno: %d) \n", ret);
    }

    // PositionControlOnOff_Write(CONTROL);
    container.c_uint8 = 1;
    if ((ret = message_ctrlboard(thr_data->msgq_id, msg_id,
                          CMD_POSITION_CONTROL_ON_OFF, CMD_TYPE_WRITE,
                          1, &container) == MSG_STATE_SUCCESS)) {
        printf("Position Control ON \n");
    } else {
        printf("Failed to set position control to ON (Errno: %d) \n", ret);
    }

    // PositionProportionPoint_Write(gain);
    gain = 10;
    container.c_uint8 = gain;
    if ((ret = message_ctrlboard(thr_data->msgq_id, msg_id,
                          CMD_POSITION_PROPORTION_POINT, CMD_TYPE_WRITE,
                          1, &container) == MSG_STATE_SUCCESS)) {
        printf("Position Proportion Point: %d \n", gain);
    } else {
        printf("Failed to set position proportion point (Errno: %d) \n", ret);
    }

    // Initialization to servo motor
    steering = 1500;
    container.c_int16 = steering;
    if ((ret = message_ctrlboard(thr_data->msgq_id, msg_id,
                          CMD_STEERING_SERVO_CONTROL, CMD_TYPE_WRITE,
                          2, &container) == MSG_STATE_SUCCESS)) {
        printf("Steering Servo Control: %d \n", steering);
    } else {
        printf("Failed to set steering servo control (Errno: %d) \n", ret);
    }
    steer_flag=1;

    while(1){
        desire_encoder = 0;
        /* Set the encoder count to zero(0) */
        container.c_int32 = 0;
        ret = message_ctrlboard(thr_data->msgq_id, msg_id, CMD_ENCODER_COUNTER, CMD_TYPE_WRITE, 4, &container);
        if (ret != MSG_STATE_SUCCESS) {printf("Err(%d): Failed to set encoder count to zero \n", ret); }
        
        psd_distance = prr->psd.value[PSD_RIGHT_1];
        //printf("psd_data: %f\n", psd_distance);

        // if(psd_distance > 29.f ){
        //     desire_encoder = 0;
        // }
        // else {
            desire_encoder += 300;
            container.c_int32 = desire_encoder;
            ret = message_ctrlboard(thr_data->msgq_id, msg_id, 
                                    CMD_DESIRE_ENCODER_COUNT, CMD_TYPE_WRITE,
                                    4, &container);
            if (ret != MSG_STATE_SUCCESS) {
                printf("Err(%d): Failed to set desire encoder count to %d \n", ret, desire_encoder);
            }
        // }
        
        // Desire Encoder Count
        
        usleep(SLEEP_TIME_LINING_WALL);
    }
}



int main(int argc, char** argv) {
    int ret;
   // pthread_t thr_data[2];
   pthread_t thr_id[2];
   thread_data thr_data;


    if (get_msgq_id_ctrlboard(&thr_data.msgq_id, 0) == -1) {
        printf("Failed get message queue id!");
        return 1;
    }

    get_recog_result();

    ret = pthread_create(&thr_id[0], NULL, lining_wall, (void*)&thr_data);
    if(ret){
        printf("Failed creating streer control thread");
        return 1;
    }
    pthread_detach(thr_id[0]);

    ret = pthread_create(&thr_id[1], NULL, steering_control, (void*)&thr_data);
    if(ret){
        printf("Failed creating streer control thread");
        return 1;
    }
    pthread_detach(thr_id[2]);

    int speed = 0;
    ctrlboard_byte_container container;
    container.c_int16 = steering;

    // SteeringServoControl_Write(steering);
    if (message_ctrlboard(msgq_id, 101, CMD_STEERING_SERVO_CONTROL,CMD_TYPE_WRITE,2,&container) == MSG_STATE_SUCCESS) {} 
    else { printf("Fail to set steering servo control");  }

    pause();
    return 0;
}