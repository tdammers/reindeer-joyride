#pragma once

typedef struct reindeer_t {
    // physics state
    double x;
    double y;
    double alt;

    double v;
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

    // stats
    double acceleration;
    double max_speed;
    double rolling_friction;
    double turn_torque;
    double turn_rate;
    double pitch_rate;
    double climb_force;
    double max_climb_rate;
} reindeer_t;

void
init_reindeer(reindeer_t *reindeer);

void
update_reindeer(reindeer_t *reindeer, double dt);
