#include "control.h"

void climb_slope(State *state)
{
    while (state->input->is_on_slope.value)
    {
        // Go forward with speed 100
    }
}

void check_slope(State *state)
{
    if (state->input->is_on_slope.value)
    {
        state->missions.slope.priority = 1;
        state->missions.slope.function = climb_slope;
    }
}