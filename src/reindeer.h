#pragma once

#include "tilemap.h"

typedef struct reindeer_t {
    // physics state
    double x;
    double y;
    double alt;

    double vx;
    double vy;
    double valt;

    double angle;
    double vangle;

    double pitch;

    // head bobbing/animation
    double bob_phase;
    double bob_strength;

    // controls
    int accel_control;
    int brake_control;
    int turn_control;
    int elevator_control;

    // lap stats
    int next_checkpoint;
    int laps_finished;
    int best_lap;
    double lap_times[32];
    double race_time;
    double current_lap_time;

    // stats
    double acceleration;
    double max_speed;
    double rolling_friction;
    double grip;
    double turn_torque;
    double turn_rate;
    double pitch_rate;
    double climb_force;
    double max_climb_rate;
} reindeer_t;

void
init_reindeer(reindeer_t *reindeer);

void
update_reindeer(reindeer_t *reindeer, const tilemap_t* map, double dt);

double
get_next_checkpoint_heading(const reindeer_t* reindeer, const tilemap_t* tilemap);
