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
    reindeer->pitch_rate = 2.0;
    reindeer->climb_force = 100.0;
    reindeer->max_climb_rate = 100.0;
}

void
update_reindeer(reindeer_t *reindeer, const tilemap_t *map, double dt)
{
    // process accel/brake controls
    if (reindeer->accel_control) {
        reindeer->v += reindeer->acceleration * dt;
        if (reindeer->v > reindeer->max_speed) {
            reindeer->v = reindeer->max_speed;
        }
    }
    else {
        if (reindeer->v > 0.0) {
            reindeer->v -= reindeer->rolling_friction * dt;
            if (reindeer->v < 0.0) {
                reindeer->v = 0.0;
            }
        }
        else {
            reindeer->v += reindeer->rolling_friction * dt;
            if (reindeer->v > 0.0) {
                reindeer->v = 0.0;
            }
        }
    }

    if (reindeer->brake_control) {
        if (reindeer->v > 0.0) {
            reindeer->v -= reindeer->acceleration * dt;
            if (reindeer->v < 0.0) {
                reindeer->v = 0.0;
            }
        }
        else {
            reindeer->v += reindeer->acceleration * dt;
            if (reindeer->v > 0.0) {
                reindeer->v = 0.0;
            }
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

    // process elevator
    double target_pitch = (double)reindeer->elevator_control;
    if (reindeer->pitch > target_pitch) {
        reindeer->pitch = fmax(target_pitch, reindeer->pitch - reindeer->pitch_rate * dt);
    }
    if (reindeer->pitch < target_pitch) {
        reindeer->pitch = fmin(target_pitch, reindeer->pitch + reindeer->pitch_rate * dt);
    }

    // elevator -> vertical speed
    double target_valt =
                reindeer->max_climb_rate * reindeer->pitch *
                reindeer->v / reindeer->max_speed
                - fmax(0.0, 20.0 - reindeer->v);
    if (reindeer->valt > target_valt) {
        reindeer->valt = fmax(target_valt, reindeer->valt - reindeer->climb_force * dt);
    }
    if (reindeer->valt < target_valt) {
        reindeer->valt = fmin(target_valt, reindeer->valt + reindeer->climb_force * dt);
    }

    // update position
    reindeer->angle += reindeer->vangle * dt;
    double sa = sin(reindeer->angle);
    double ca = cos(reindeer->angle);
    double dx = reindeer->v * dt * sa;
    double dy = -reindeer->v * dt * ca;

    // lateral collision detection
    tile_t t = tilemap_get(map, (int)floor(reindeer->x + dx) >> 5, (int)floor(reindeer->y + dy) >> 5);
    double ground_elev = tile_obstacle_height(t);
    if (((((int)floor(reindeer->x + dx) >> 5) != (int)floor(reindeer->x) >> 5) ||
        (((int)floor(reindeer->y + dy) >> 5) != (int)floor(reindeer->y) >> 5))
        &&
        (ground_elev > reindeer->alt + 0.1)) {
        // collision, oh no!
        reindeer->v = -reindeer->v * 0.5;
        reindeer->x -= dx;
        reindeer->y -= dy;
    }
    else {
        reindeer->x += dx;
        reindeer->y += dy;
    }

    // update altitude
    t = tilemap_get(map, (int)floor(reindeer->x) >> 5, (int)floor(reindeer->y) >> 5);
    ground_elev = 0.0; // tile_obstacle_height(t);
    reindeer->alt += reindeer->valt * dt;
    if (reindeer->alt < ground_elev) {
        reindeer->alt = ground_elev;
        reindeer->valt = 0.0;
        if (ground_elev > 0.0) {
            reindeer->v = 0.0;
        }
    }
    if (reindeer->alt > 256.0) {
        reindeer->alt = 256.0;
        reindeer->valt = 0.0;
    }

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
