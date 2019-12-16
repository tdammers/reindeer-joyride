#include "mode7.h"
#include <math.h>

// typedef struct mode7_view {
//     double cam_x, // camera position X in world space
//     double cam_y, // camera position Y in world space
//     double cam_angle, // camera view angle about world Z axis (yaw)
//     double cam_alt, // camera altitude above ground
//     int screen_w, // screen width in pixels
//     int screen_h, // screen height in pixels
//     double screen_dist, // screen dist, in pixels
//     int horizon_screen_y // Y coordinate of the horizon, in screen coords
// } mode7_view;

int
mode7_project(
    const mode7_view* view,
    double* ground_x,
    double* ground_y,
    int screen_x,
    int screen_y
)
{
    // Make screen coordinates relative to (center_x, horizon_y)
    double rel_screen_x = (double)screen_x - (0.5 * (double)view->screen_w);
    double rel_screen_y = (double)screen_y - (double)view->horizon_screen_y;

    // Only draw on the ground
    if (rel_screen_y < 1) return 0;

    // Calculate ground coordinates in camera space
    double rel_ground_y = view->screen_dist * view->cam_alt / rel_screen_y;
    double rel_ground_x = rel_screen_x * rel_ground_y / view->screen_dist;

    // Transform from camera space to world space
    double sa = sin(view->cam_angle);
    double ca = cos(view->cam_angle);

    *ground_x = view->cam_x + rel_ground_x * ca + rel_ground_y * sa;
    *ground_y = view->cam_y - rel_ground_y * ca + rel_ground_x * sa;

    return 1;
}

int
mode7_unproject(
    const mode7_view* view,
    double *screen_x,
    double *screen_y,
    double *screen_size_factor,
    double world_x,
    double world_y,
    double world_alt
)
{
    // relative position in world space, centered at camera
    double rel_x = world_x - view->cam_x;
    double rel_y = world_y - view->cam_y;

    // camera-space positions
    double sa = sin(view->cam_angle);
    double ca = cos(view->cam_angle);
    double eye_x = ca * rel_x + sa * rel_y;
    double eye_y = ca * rel_y - sa * rel_x;
    double eye_alt = world_alt - view->cam_alt;

    if (eye_y > -8.0) return 0;

    *screen_x = (double)view->screen_w * 0.5 - eye_x * view->screen_dist / eye_y;
    *screen_y = (double)view->horizon_screen_y + eye_alt * view->screen_dist / eye_y;
    *screen_size_factor = view->screen_dist / -eye_y;

    return 1;
}
