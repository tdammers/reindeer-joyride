#include "ai_brain.h"
#include "reindeer.h"
#include "game_state.h"

#include <stdio.h>

typedef struct ai_brain_state_t {
    int dummy;
} ai_brain_state_t;

ai_brain_state_t*
create_ai_brain_state()
{
    ai_brain_state_t* pbs = malloc(sizeof(ai_brain_state_t));
    memset(pbs, 0, sizeof(ai_brain_state_t));
    return pbs;
}

void
destroy_ai_brain_state(void* vstate)
{
    free(vstate);
}

void
update_ai_brain(
    void* vstate,
    size_t reindeer_index,
    const struct game_state_t* game_state)
{
    ai_brain_state_t* state = (ai_brain_state_t*)vstate;
    reindeer_t* reindeer = game_state->reindeer + reindeer_index;

    (void)state;

    double target_angle = get_next_checkpoint_heading(reindeer, game_state->map);
    double actual_angle = fmod(reindeer->angle, 2.0 * M_PI);
    double angle_error = target_angle - actual_angle;
    while (angle_error > M_PI) {
        angle_error -= 2.0 * M_PI;
    }
    while (angle_error < -M_PI) {
        angle_error += 2.0 * M_PI;
    }

    // printf("T: %+04.0f° | R: %+04.0f° | ERR: %+04.0f°\n",
    //     target_angle * 180.0 / M_PI,
    //     actual_angle * 180.0 / M_PI,
    //     angle_error * 180.0 / M_PI);

    double v = hypot(reindeer->vx, reindeer->vy);
    double distance = get_next_checkpoint_distance(reindeer, game_state->map);
    double turn_radius = v / reindeer->turn_rate / M_PI;
    if (distance >= fabs(angle_error) * turn_radius) {
        reindeer->accel_control = 1.0;
        reindeer->brake_control = 0.0;
    }
    else {
        reindeer->accel_control = 0.0;
        reindeer->brake_control = 1.0;
    }
    reindeer->turn_control = fmin(1.0, fmax(-1.0, (angle_error * 10.0 - reindeer->vangle) * 10.0));
    // printf("T: %+2i | A: %i | B: %i\n",
    //     reindeer->turn_control,
    //     reindeer->accel_control,
    //     reindeer->brake_control);
}

brain_t*
create_ai_brain()
{
    brain_t* brain = create_brain();
    brain->state = create_ai_brain_state();
    brain->update = update_ai_brain;
    brain->destroy = destroy_ai_brain_state;
    return brain;
}
