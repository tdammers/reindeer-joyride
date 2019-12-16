#include "reindeer.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

void
init_reindeer(reindeer_t *reindeer)
{
    memset(reindeer, 0, sizeof(reindeer_t));
    reindeer->acceleration = 100.0;
    reindeer->max_speed = 300.0;
    reindeer->rolling_friction = 10.0;
    reindeer->turn_torque = 2.0 * M_PI;
    reindeer->turn_rate = M_PI * 0.5;
}

void
update_reindeer(reindeer_t *reindeer, double dt)
{
    // process accel/brake controls
    if (reindeer->accel_control) {
        reindeer->v += reindeer->acceleration * dt;
        if (reindeer->v > reindeer->max_speed) {
            reindeer->v = reindeer->max_speed;
        }
    }
    else {
        reindeer->v -= reindeer->rolling_friction * dt;
        if (reindeer->v < 0.0) {
            reindeer->v = 0.0;
        }
    }

    if (reindeer->brake_control) {
        reindeer->v -= reindeer->acceleration * dt;
        if (reindeer->v < 0.0) {
            reindeer->v = 0.0;
        }
    }

    // process steering controls
    double target_vangle = (double)reindeer->turn_control * reindeer->turn_rate;
    if (reindeer->vangle > target_vangle) {
        reindeer->vangle = fmax(target_vangle, reindeer->vangle - reindeer->turn_torque * dt);
    }
    else if (reindeer->vangle < target_vangle) {
        reindeer->vangle = fmin(target_vangle, reindeer->vangle + reindeer->turn_torque * dt);
    }

    // update position
    reindeer->angle += reindeer->vangle * dt;
    double sa = sin(reindeer->angle);
    double ca = cos(reindeer->angle);
    reindeer->x += reindeer->v * dt * sa;
    reindeer->y -= reindeer->v * dt * ca;

    // update head bobbing animation
    reindeer->bob_phase += dt * 3 * M_PI;
    if (reindeer->bob_phase > 2 * M_PI) {
        reindeer->bob_phase -= 2 * M_PI;
    }

    double target_bob_strength = fmin(reindeer->v / 64.0, 1.0);
    if (reindeer->bob_strength < target_bob_strength) {
        reindeer->bob_strength =
            fmin(reindeer->bob_strength + dt * 2.0, target_bob_strength);
    }
    else if (reindeer->bob_strength > target_bob_strength) {
        reindeer->bob_strength =
            fmax(reindeer->bob_strength - dt * 2.0, target_bob_strength);
    }
}
