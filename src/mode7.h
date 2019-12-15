#pragma once

#include <allegro5/allegro.h>

typedef struct mode7_view {
    double cam_x; // camera position X in world space
    double cam_y; // camera position Y in world space
    double cam_angle; // camera view angle about world Z axis (yaw)
    double cam_alt; // camera altitude above ground
    int screen_w; // screen width in pixels
    int screen_h; // screen height in pixels
    double screen_dist; // screen dist, in pixels
    int horizon_screen_y; // Y coordinate of the horizon, in screen coords
} mode7_view;

/**
 * Return true (1) if ground_x and ground_y contain sensible coordinates.
 * For pixels at or above the horizon, this will not be the case.
 */
int
mode7_project(
    const mode7_view*,
    double* ground_x,
    double* ground_y,
    int screen_x,
    int screen_y
);
