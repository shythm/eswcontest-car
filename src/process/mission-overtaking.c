#include "car-header.h"
#include "process.h"

#define SPEED_OVERTAKING 110
#define STEER_GAIN1      15
#define STEER_GAIN2      30

#define REGRESS_ENABLE1 0
#define REGRESS_ENABLE2 0

void init_overtaking(fnCheck_t *fnCheck);
bool check_overtaking(fnRun_t *fnRun);
void do_overtaking(fnClean_t *fnClean);
void clean_overtaking(void);

void init_overtaking(fnCheck_t *fnCheck) {
    while (!recog->psd.valid) {}
    MSG("UPCOMING MISSION => overtaking");

    *fnCheck            = check_overtaking;
    recog->lane.enabled = true;
}

bool check_overtaking(fnRun_t *fnRun) {

    set_desire_speed(SPEED_OVERTAKING + 10);
    while (1) {
        set_steering(1500 + recog->lane.value.pos_yawl * STEER_GAIN1);
        // usleep(1000);
        if (recog->psd.value[PSD_FRONT] < 27.f) {
            MSG("1: %3.1f ", recog->psd.value[PSD_FRONT]);
            set_desire_speed(0);
            break;
        }
        // if (get_is_on_stop_line()) {
        //     *fnRun = NULL;
        //     return true;
        // }
    }
    *fnRun = do_overtaking;
    MSG("START MISSION => overtaking");
    return true;
}

void do_overtaking(fnClean_t *fnClean) {
    const float turn_rad       = PI * 50.f / 180.f;
    const int   straight_tick  = 18.f * TICK_PER_CM;
    const int   supp_turn_tick = 4.f * TICK_PER_CM;
    int         target_encoder = 0;
    int         steering       = 0;
    int volatile direction;
    // stop
    set_desire_speed(0);
    recog->empty_road.enable = true;
    set_camera_Yservo(1600);
    set_steering(1500);
    usleep(SLEEP_STOP);
    MSG("2: %3.1f ", recog->psd.value[PSD_FRONT]);

    // regress: precisely control distance between obstacle and robot
    set_desire_speed(-SPEED_OVERTAKING / 2);
    while (recog->psd.value[PSD_FRONT] < 27.0f) {}
    MSG("3: %3.1f ", recog->psd.value[PSD_FRONT]);
    // regress
    move(-SPEED_OVERTAKING / 2, -10.f * TICK_PER_CM);
    MSG("4: %3.1f ", recog->psd.value[PSD_FRONT]);

    sleep(1);
    // search empty road
    direction = 0;
    for (int i = 0; i < 40; i++) {
        direction += recog->empty_road.value;
        usleep(3000);
    }

    direction = (direction < 0) ? -500 : 500; // left:right

    set_camera_Yservo(1700);
    recog->empty_road.enable = false;

    // overtaking direction:  RIGHT | LEFT
    // turn right | left   => first overtaking
    set_steering(1500 - direction);
    move(SPEED_OVERTAKING, turn_rad * RADIUS * TICK_PER_CM);

    // progress
    set_steering(1500);
    move(SPEED_OVERTAKING, straight_tick);

    // turn left | right
    set_steering(1500 + direction);
    move(SPEED_OVERTAKING, turn_rad * RADIUS * TICK_PER_CM);

#if REGRESS_ENABLE1
    // regress(Yellow & White lane)  move car to center of road
    recog->ext_data.call_init_lane_info = true;
    set_steering(1500);
    target_encoder = read_encoder_counter() - 20.f * TICK_PER_CM;
    usleep(SLEEP_STEER);
    set_desire_speed(-SPEED_OVERTAKING);
    while (target_encoder < read_encoder_counter()) {
        set_steering(1500 - recog->lane.value.pos_yawl * STEER_GAIN2);
        usleep(1000);
    }
#endif
    set_desire_speed(0);

    // progress (Yellow & White lane)
    recog->ext_data.call_init_lane_info = true;
    set_steering(1500);
    target_encoder = read_encoder_counter() + 30.f * TICK_PER_CM;
    usleep(SLEEP_STEER);
    set_desire_speed(SPEED_OVERTAKING);
    while (read_encoder_counter() < target_encoder) {
        set_steering(1500 + recog->lane.value.pos_yawl * STEER_GAIN2);
        usleep(1000);
    }
    set_desire_speed(0);

    // turn left | right   => second overtaking
    set_steering(1500 + direction);
    move(SPEED_OVERTAKING, turn_rad * RADIUS * TICK_PER_CM);

    // progress
    set_steering(1500);
    move(SPEED_OVERTAKING, straight_tick);

    // turn right | left
    set_steering(1500 - direction);
    move(SPEED_OVERTAKING, turn_rad * RADIUS * TICK_PER_CM + supp_turn_tick);

#if REGRESS_ENABLE2
    // regress (psd) => move car to center of road
    recog->ext_data.call_init_lane_info = true;
    set_steering(1500);
    usleep(SLEEP_STEER);
    set_desire_speed(-SPEED_OVERTAKING);
    while (recog->psd.value[PSD_BACK] > 29.f) {
        // set_steering(1500 - recog->lane.value.pos_yawl * STEER_GAIN2);
    }
    MSG("back: %3.1f ", recog->psd.value[PSD_BACK]);
    set_desire_speed(0);
    set_steering(1500);
#endif

    // progress (Yellow lane)
    recog->stop_line_pos.enable         = true;
    recog->ext_data.call_init_lane_info = true;
    target_encoder = read_encoder_counter() + 30.F * TICK_PER_CM;
    set_steering(1500);
    usleep(SLEEP_STEER);
    set_desire_speed(SPEED_OVERTAKING);
    while (read_encoder_counter() < target_encoder) {
        if (recog->stop_line_pos.value < 0.0f && !get_is_on_stop_line())
            set_steering(1500 + recog->lane.value.pos_yawl * STEER_GAIN2);
        else
            break;
    }

    set_desire_speed(0);
}
