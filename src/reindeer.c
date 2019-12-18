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
    reindeer->best_lap = -1;
}

void
update_reindeer(reindeer_t *reindeer, const tilemap_t *map, double dt)
{
    tile_t t;
    double ground_elev;
    int in_water;
    int airborne;

    // calculate friction coefficient
    t = tilemap_get(map, (int)floor(reindeer->x) >> 5, (int)floor(reindeer->y) >> 5);
    ground_elev = tile_obstacle_height(t);
    airborne = reindeer->alt > ground_elev + 4;
    in_water = (t == WATER_TILE) && !airborne;

    // process accel/brake controls
    if (reindeer->accel_control) {
        reindeer->v += reindeer->acceleration * dt;
        if (reindeer->v > reindeer->max_speed) {
            reindeer->v = reindeer->max_speed;
        }
    }

    double friction = reindeer->rolling_friction;
    if (airborne) {
        friction = fabs(reindeer->v) * 0.1;
    }
    else if (in_water) {
        friction = fabs(reindeer->v) * 5.0;
    }
    if (reindeer->v > 0.0) {
        reindeer->v -= friction * dt;
        if (reindeer->v < 0.0) {
            reindeer->v = 0.0;
        }
    }
    else {
        reindeer->v += friction * dt;
        if (reindeer->v > 0.0) {
            reindeer->v = 0.0;
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
    t = tilemap_get(map, (int)floor(reindeer->x + dx) >> 5, (int)floor(reindeer->y + dy) >> 5);
    ground_elev = tile_obstacle_height(t);
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
    // recalculate current tile, we'll need this for checkpoints and altitude
    // logic
    t = tilemap_get(map, (int)floor(reindeer->x) >> 5, (int)floor(reindeer->y) >> 5);

    // update altitude
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

    // handle checkpoint logic
    reindeer->current_lap_time += dt;
    reindeer->race_time += dt;
    if (reindeer->alt <= 32) {
        if (reindeer->next_checkpoint >= 0 &&
                t == CHECKPOINT0_TILE + reindeer->next_checkpoint) {
            reindeer->next_checkpoint++;
            if (reindeer->next_checkpoint > get_tilemap_max_checkpoint(map)) {
                reindeer->next_checkpoint = -1;
            }
        }
        else if (reindeer->next_checkpoint == -1 &&
                    t == START_FINISH_TILE) {
            if (reindeer->laps_finished < 32) {
                reindeer->lap_times[reindeer->laps_finished] =
                    reindeer->current_lap_time;
            }
            if (reindeer->best_lap < 0 ||
                reindeer->current_lap_time < reindeer->lap_times[reindeer->best_lap]) {
                reindeer->best_lap = reindeer->laps_finished;
            }
            reindeer->current_lap_time = 0.0;
            reindeer->laps_finished++;
            reindeer->next_checkpoint = 0;
        }
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
