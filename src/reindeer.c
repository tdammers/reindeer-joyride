#include "reindeer.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

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
    reindeer->grip = 500.0;
    reindeer->best_lap = -1;
}

void
update_reindeer(reindeer_t *reindeer, const tilemap_t *map, double dt)
{
    tile_t t;
    double ground_elev;
    int in_water;
    int airborne;

    // update angle
    reindeer->angle += reindeer->vangle * dt;
    double sa = sin(reindeer->angle);
    double ca = cos(reindeer->angle);

    // determine ground status
    t = tilemap_get(map, (int)floor(reindeer->x) >> 5, (int)floor(reindeer->y) >> 5);
    ground_elev = tile_ground_elev(t);
    airborne = reindeer->alt > ground_elev + 4;
    in_water = (t == WATER_TILE) && !airborne;

    // process grip
    double vp = -ca * reindeer->vy + sa * reindeer->vx;
    double vn = ca * reindeer->vx + sa * reindeer->vy;
    double frictionn = reindeer->grip;
    if (airborne) {
        frictionn = fabs(vn) * 2.5;
    }
    if (fabs(vn) > 0.00000001) {
        double vnnew =
                (vn > 0.0)
                    ? fmax(vn - frictionn * dt, 0.0)
                    : fmin(vn + frictionn * dt, 0.0);
        reindeer->vx =  vp * sa + vnnew * ca;
        reindeer->vy = -vp * ca + vnnew * sa;
        (void)vnnew;
    }

    // process accel/brake controls
    double v = hypotf(reindeer->vx, reindeer->vy);
    if (reindeer->accel_control && v < reindeer->max_speed) {
        reindeer->vx += reindeer->acceleration * dt * sa;
        reindeer->vy -= reindeer->acceleration * dt * ca;
    }

    double friction = reindeer->rolling_friction;
    if (airborne) {
        if (reindeer->brake_control) {
            friction = v * 1.0;
        }
        else {
            friction = v * 0.1;
        }
    }
    else if (in_water) {
        friction = v * 5.0;
    }
    else if (reindeer->brake_control) {
        friction = reindeer->grip;
    }
    if (v > 0.00000001) {
        double vnew = fmax(v - friction * dt, 0.0);
        double vfac = vnew / v;
        reindeer->vx *= vfac;
        reindeer->vy *= vfac;
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
                v / reindeer->max_speed
                - fmax(0.0, 20.0 - v) * 20.0;
    if (reindeer->valt > target_valt) {
        reindeer->valt = fmax(target_valt, reindeer->valt - reindeer->climb_force * dt);
    }
    if (reindeer->valt < target_valt) {
        reindeer->valt = fmin(target_valt, reindeer->valt + reindeer->climb_force * dt);
    }

    // update position
    double dx = reindeer->vx * dt;
    double dy = reindeer->vy * dt;
    int txc, tyc; // current position
    int txn, tyn; // new position

    // lateral collision detection
    txc = (int)floor(reindeer->x) >> 5;
    tyc = (int)floor(reindeer->y) >> 5;
    txn = (int)floor(reindeer->x + dx) >> 5;
    tyn = (int)floor(reindeer->y + dy) >> 5;

    t = tilemap_get(map, txn, tyn);
    ground_elev = tile_ground_elev(t);
    int obstacle_top = tile_obstacle_top(t);
    int obstacle_bottom = tile_obstacle_bottom(t);
    if ((txn != txc || tyn != tyc) &&
        ((reindeer->alt < ground_elev - 0.1) ||
         (reindeer->alt > obstacle_bottom + 0.1 &&
          reindeer->alt < obstacle_top - 0.1))) {
        // collision, oh no!

        if (txn != txc) {
            // passed an X boundary
            reindeer->x -= dx;
            reindeer->vx = -reindeer->vx * 0.5;
        }

        if (tyn != tyc) {
            // passed a Y boundary
            reindeer->y -= dy;
            reindeer->vy = -reindeer->vy * 0.5;
        }
    }
    else {
        reindeer->x += dx;
        reindeer->y += dy;
    }
    // recalculate current tile, we'll need this for checkpoints and altitude
    // logic
    t = tilemap_get(map, (int)floor(reindeer->x) >> 5, (int)floor(reindeer->y) >> 5);

    // update altitude
    ground_elev = 0.0;
    reindeer->alt += reindeer->valt * dt;
    if (reindeer->alt < ground_elev) {
        reindeer->alt = ground_elev;
        reindeer->valt = 0.0;
        if (ground_elev > 0.0) {
            reindeer->vx = 0.0;
            reindeer->vy = 0.0;
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

    double target_bob_strength = fmin(v / 64.0, 1.0);
    if (reindeer->bob_strength < target_bob_strength) {
        reindeer->bob_strength =
            fmin(reindeer->bob_strength + dt * 2.0, target_bob_strength);
    }
    else if (reindeer->bob_strength > target_bob_strength) {
        reindeer->bob_strength =
            fmax(reindeer->bob_strength - dt * 2.0, target_bob_strength);
    }
}

double
get_next_checkpoint_heading(const reindeer_t* reindeer, const tilemap_t* tilemap)
{
    double cx, cy;
    if (reindeer->next_checkpoint < 0) {
        cx = get_tilemap_start_x(tilemap);
        cy = get_tilemap_start_y(tilemap);
    }
    else {
        cx = get_tilemap_checkpoint_x(tilemap, reindeer->next_checkpoint);
        cy = get_tilemap_checkpoint_y(tilemap, reindeer->next_checkpoint);
    }
    double dx = (cx * 32.0) - reindeer->x;
    double dy = (cy * 32.0) - reindeer->y;
    return atan2(-dx, dy);
}

