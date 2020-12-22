#include "car-header.h"
#include "process.h"

#define SPEED_OVERTAKING 70
#define SLEEP_OVERTAKING 400000
#define STEER_GAIN       50

void init_overtaking(fnCheck_t *fnCheck);
bool check_overtaking(fnRun_t *fnRun);
void do_overtaking(fnClean_t *fnClean);
void clean_overtaking(void);

void init_overtaking(fnCheck_t *fnCheck) {

    set_steering(1500);
    while (!recog->psd.valid) {}
    MSG("UPCOMING MISSION => overtaking");

    *fnCheck            = check_overtaking;
    recog->lane.enabled = true;
    set_steering(1500);
    set_desire_speed(SPEED_OVERTAKING + 30);
}

bool check_overtaking(fnRun_t *fnRun) {
    recog->ext_data.call_init_lane_info = true;
    while (1) {
        set_steering(1500 + (short)recog->lane.value.pos_yawl * STEER_GAIN);
        usleep(1000);
        // MSG("psd: %3.1f ", recog->psd.value[PSD_FRONT]);
        if (recog->psd.value[PSD_FRONT] < 27.f) {
            MSG("1: %3.1f ", recog->psd.value[PSD_FRONT]);
            set_desire_speed(0);
            break;
        }
    }
    *fnRun = do_overtaking;
    MSG("START MISSION => overtaking");
    return true;
}

void do_overtaking(fnClean_t *fnClean) {
    int         overtaking_direction;
    const float turn_rad       = PI * 50.f / 180.f;
    const float straight_tick  = 15.f * TICK_PER_CM;
    int         target_encoder = 0;
    // stop
    set_desire_speed(0);
    set_steering(1500);
    usleep(SLEEP_OVERTAKING * 3);

    MSG("2: %3.1f ", recog->psd.value[PSD_FRONT]);

    set_desire_speed(-SPEED_OVERTAKING / 2);
    while (recog->psd.value[PSD_FRONT] < 27.0f) {}
    set_desire_speed(0);
    usleep(SLEEP_OVERTAKING);
    MSG("3: %3.1f ", recog->psd.value[PSD_FRONT]);

    move(-SPEED_OVERTAKING, -10.f * TICK_PER_CM);
    MSG("4: %3.1f ", recog->psd.value[PSD_FRONT]);

    // search empty road
    recog->other_cars.enable = true;
    set_camera_Yservo(1600);
    usleep(SLEEP_OVERTAKING * 2);
    sleep(3);
    overtaking_direction = 0;
    for (int i = 0; i < 100; i++) {
        overtaking_direction += recog->other_cars.value;
        usleep(2000);
    }

    int steer_direc = (overtaking_direction < 0) ? -500 : 500; // left:right
    // if (overtaking_direction < 0) { // left
    //     steer_direc = -500;
    //     MSG("LEFT!!!\n");
    // } else if (overtaking_direction >= 0) { // right
    //     steer_direc = 500;
    //     MSG("RIGHT!!!\n");
    // }

    set_camera_Yservo(1700);
    recog->other_cars.enable = false;

    // RIGHT | LEFT
    // turn right | left
    set_steering(1500 - steer_direc);
    usleep(SLEEP_OVERTAKING);
    move(SPEED_OVERTAKING, turn_rad * RADIUS * TICK_PER_CM);
    usleep(SLEEP_OVERTAKING);

    set_steering(1500);
    usleep(SLEEP_OVERTAKING);
    move(SPEED_OVERTAKING, straight_tick);
    usleep(SLEEP_OVERTAKING);

    // turn left | right
    set_steering(1500 + steer_direc);
    usleep(SLEEP_OVERTAKING);
    move(SPEED_OVERTAKING, turn_rad * RADIUS * TICK_PER_CM);
    usleep(SLEEP_OVERTAKING);

    // go straight
    /*
    set_steering(1500);
    usleep(SLEEP_OVERTAKING);
    move(SPEED_OVERTAKING, 40.0f * TICK_PER_CM);
    usleep(SLEEP_OVERTAKING);
    */
    recog->lane.enabled = 1;

    // regress
    recog->ext_data.call_init_lane_info = true;
    set_steering(1500);
    set_desire_speed(-SPEED_OVERTAKING);
    target_encoder = read_encoder_counter() - 30.f * TICK_PER_CM;
    usleep(SLEEP_OVERTAKING);
    while (target_encoder < read_encoder_counter()) {
        set_steering(1500 - (short)recog->lane.value.pos_yawl * STEER_GAIN);
        usleep(1000);
    }
    set_desire_speed(0);

    // progress
    recog->ext_data.call_init_lane_info = true;
    set_steering(1500);
    set_desire_speed(SPEED_OVERTAKING);
    target_encoder = read_encoder_counter() + 60.f * TICK_PER_CM;
    usleep(SLEEP_OVERTAKING);
    while (read_encoder_counter() < target_encoder) {
        set_steering(1500 + (short)recog->lane.value.pos_yawl * STEER_GAIN);
        usleep(1000);
    }
    set_desire_speed(0);

    // turn left | right
    set_steering(1500 + steer_direc);
    usleep(SLEEP_OVERTAKING);
    move(SPEED_OVERTAKING, turn_rad * RADIUS * TICK_PER_CM);
    usleep(SLEEP_OVERTAKING);

    set_steering(1500);
    usleep(SLEEP_OVERTAKING);
    move(SPEED_OVERTAKING, straight_tick);
    usleep(SLEEP_OVERTAKING);

    // turn right | left
    set_steering(1500 - steer_direc);
    usleep(SLEEP_OVERTAKING * 2);
    move(SPEED_OVERTAKING, turn_rad * RADIUS * TICK_PER_CM);

    // set_steering(1500);

    // regress
    recog->ext_data.call_init_lane_info = true;
    set_steering(1500);
    set_desire_speed(-SPEED_OVERTAKING);
    usleep(SLEEP_OVERTAKING);
    while (recog->psd.value[PSD_BACK] > 25.f) {
        // MSG("back: %3.1f", recog->psd.value[PSD_BACK]);
        // set_steering(1500 - (short)recog->lane.value.pos_yl * STEER_GAIN);
        // usleep(1000);
    }
    MSG("back: %3.1f ", recog->psd.value[PSD_BACK]);
    set_desire_speed(0);

    // progress
    recog->ext_data.call_init_lane_info = true;
    set_steering(1500);
    set_desire_speed(SPEED_OVERTAKING);
    usleep(SLEEP_OVERTAKING);
    while (!get_is_on_stop_line()) {
        set_steering(1500 + (short)recog->lane.value.pos_yl * STEER_GAIN);
        usleep(1000);
    }
    set_desire_speed(0);
    while (1) {}
}

void clean_overtaking() { recog->other_cars.enable = false; }
