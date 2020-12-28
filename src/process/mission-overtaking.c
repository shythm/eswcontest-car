#include "car-header.h"
#include "process.h"

#define SPEED_OVERTAKING 100
#define SLEEP_OVERTAKING 400000
#define STEER_GAIN1      15
#define STEER_GAIN2      50

void init_overtaking(fnCheck_t *fnCheck);
bool check_overtaking(fnRun_t *fnRun);
void do_overtaking(fnClean_t *fnClean);
void clean_overtaking(void);

void init_overtaking(fnCheck_t *fnCheck) {
    while (!recog->psd.valid) {}
    MSG("UPCOMING MISSION => overtaking");
    set_desire_speed(SPEED_OVERTAKING - 30);

    *fnCheck            = check_overtaking;
    recog->lane.enabled = true;
}

bool check_overtaking(fnRun_t *fnRun) {
    int steering;
    while (1) {
        set_steering(1500 + recog->lane.value.pos_yawl * STEER_GAIN1);
        usleep(1000);
        if (recog->psd.value[PSD_FRONT] < 27.f) {
            MSG("1: %3.1f ", recog->psd.value[PSD_FRONT]);
            set_desire_speed(0);
            break;
        }
        if (get_is_on_stop_line()) {
            *fnRun = NULL;
            return true;
        }
    }
    *fnRun = do_overtaking;
    MSG("START MISSION => overtaking");
    return true;
}

void do_overtaking(fnClean_t *fnClean) {
    int         overtaking_direction;
    const float turn_rad       = PI * 50.f / 180.f;
    const int   straight_tick  = 15.f * TICK_PER_CM;
    int         target_encoder = 0;
    int         steering       = 0;
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
    for (int i = 0; i < 50; i++) {
        overtaking_direction += recog->other_cars.value;
        usleep(2000);
    }

    int steer_direc = (overtaking_direction < 0) ? -500 : 500; // left:right

    set_camera_Yservo(1700);
    recog->other_cars.enable = false;

    // RIGHT | LEFT
    // turn right | left   => first overtaking
    set_steering(1500 - steer_direc);
    usleep(SLEEP_OVERTAKING);
    move(SPEED_OVERTAKING, turn_rad * RADIUS * TICK_PER_CM);
    usleep(SLEEP_OVERTAKING);

    // progress
    set_steering(1500);
    usleep(SLEEP_OVERTAKING);
    move(SPEED_OVERTAKING, straight_tick);
    usleep(SLEEP_OVERTAKING);

    // turn left | right
    set_steering(1500 + steer_direc);
    usleep(SLEEP_OVERTAKING);
    move(SPEED_OVERTAKING, turn_rad * RADIUS * TICK_PER_CM);
    usleep(SLEEP_OVERTAKING); //

    // regress(Yellow & White lane)     => move car to center of road
    recog->ext_data.call_init_lane_info = true;
    set_steering(1500);
    set_desire_speed(-SPEED_OVERTAKING);
    target_encoder = read_encoder_counter() - 50.f * TICK_PER_CM;
    usleep(SLEEP_OVERTAKING);
    while (target_encoder < read_encoder_counter()) {
        set_steering(1500 - (short)recog->lane.value.pos_yawl * STEER_GAIN2);
        usleep(1000);
    }
    set_desire_speed(0);

    // progress (Yellow & White lane)
    set_steering(1500);
    target_encoder = read_encoder_counter() + 60.f * TICK_PER_CM;
    usleep(SLEEP_OVERTAKING);
    set_desire_speed(SPEED_OVERTAKING);
    while (read_encoder_counter() < target_encoder) {
        set_steering(1500 + (short)recog->lane.value.pos_yawl * STEER_GAIN2);
        usleep(1000);
    }
    set_desire_speed(0);

    // turn left | right   => second overtaking
    set_steering(1500 + steer_direc);
    usleep(SLEEP_OVERTAKING);
    move(SPEED_OVERTAKING, turn_rad * RADIUS * TICK_PER_CM);
    usleep(SLEEP_OVERTAKING);

    // progress
    set_steering(1500);
    usleep(SLEEP_OVERTAKING);
    move(SPEED_OVERTAKING, straight_tick);
    usleep(SLEEP_OVERTAKING);

    // turn right | left
    set_steering(1500 - steer_direc);
    usleep(SLEEP_OVERTAKING * 2);
    move(SPEED_OVERTAKING, turn_rad * RADIUS * TICK_PER_CM);

    // regress (psd) => move car to center of road
    recog->ext_data.call_init_lane_info = true;
    set_steering(1500);
    usleep(SLEEP_OVERTAKING);
    set_desire_speed(-SPEED_OVERTAKING);
    while (recog->psd.value[PSD_BACK] > 25.f) {}
    MSG("back: %3.1f ", recog->psd.value[PSD_BACK]);
    set_desire_speed(0);

    // progress (Yellow lane)
    set_steering(1500);
    usleep(SLEEP_OVERTAKING);
    set_desire_speed(SPEED_OVERTAKING);
    while (!get_is_on_stop_line()) {
        set_steering(1500 + (short)recog->lane.value.pos_yl * STEER_GAIN2);
    }
    set_desire_speed(0);
}

void clean_overtaking() {}
