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
