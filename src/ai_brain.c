#include "ai_brain.h"
#include "reindeer.h"
#include "game_state.h"

#include <stdio.h>
#include <math.h>

typedef struct ai_brain_state_t {
    size_t next_waypoint;
    double randx;
    double randy;
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

    const waypoint_list_t* wpl = get_tilemap_ai_waypoints(game_state->map);
    double tx, ty;

    if (rand() % 1024 == 0) {
        state->randx = (double)rand() / (double)RAND_MAX * 2.0 - 1.0;
        state->randy = (double)rand() / (double)RAND_MAX * 2.0 - 1.0;
    }

    if (wpl && wpl->num) {
        const waypoint_t* wp;
        if (state->next_waypoint >= wpl->num) {
            state->next_waypoint = 0;
        }
        wp = wpl->points + state->next_waypoint;
        double d = get_distance_to(reindeer, wp->x, wp->y);
        if (wp->flyover) {
            if (wp->checkpoint != reindeer->next_checkpoint) {
                state->next_waypoint++;
                state->next_waypoint %= wpl->num;
                wp = wpl->points + state->next_waypoint;
                // printf("Next waypoint: #%i\n", (int)state->next_waypoint);
            }
        }
        else if (d < 64.0) {
            state->next_waypoint++;
            state->next_waypoint %= wpl->num;
            wp = wpl->points + state->next_waypoint;
            // printf("Next waypoint: #%i\n", (int)state->next_waypoint);
        }

        tx = wp->x;
        ty = wp->y;
    }
    else {
        get_next_checkpoint(&tx, &ty, reindeer, game_state->map);
    }

    tx += 0.5 + state->randx;
    ty += 0.5 + state->randy;

    double distance = get_distance_to(reindeer, tx, ty);

    double target_angle = get_heading_to(reindeer, tx, ty);

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

    double target_alt = 0;
    double dx = sin(target_angle);
    double dy = -cos(target_angle);
    if (fabs(dx) > fabs(dy)) {
        dy /= fabs(dx);
        dx /= fabs(dx);
    }
    else {
        dx /= fabs(dy);
        dy /= fabs(dy);
    }
    dx *= 2;
    dy *= 2;
    size_t steps = (size_t)ceil(distance / hypotf(dx, dy)) + 5;
    // printf("D: %03.0f | DX,DY: %03.1f,%03.1f | STEPS: %i\n",
    //     distance,
    //     dx, dy,
    //     (int)steps);
    double x = reindeer->x;
    double y = reindeer->y;
    for (size_t i = 0; i < steps; ++i) {
        tile_t t = tilemap_get(
                    game_state->map,
                    (int)floor(x / 32.0),
                    (int)floor(y / 32.0));
        double alt = tile_ai_elev(t);
        double alt_limit = tile_obstacle_bottom(t);

        // printf("%3i|%3i -> %03.0f\n",
        //     (int)floor(x / 32.0),
        //     (int)floor(y / 32.0),
        //     alt);

        if (alt_limit > 0.0 && alt_limit < target_alt) {
            // We have to pass below an obstacle, so don't consider subsequent
            // tiles.
            break;
        }
        target_alt = fmax(alt, target_alt);
        x += dx;
        y += dy;
    }

    double target_valt = (target_alt - reindeer->alt);
    if (target_valt > reindeer->valt + 0.1) {
        reindeer->elevator_control = 1;
    }
    else if (target_valt < reindeer->valt - 0.1) {
        reindeer->elevator_control = -1;
    }
    else {
        reindeer->elevator_control = 0;
    }
    // printf("T: %+03.0f | A: %+03.0f | E: %i\n",
    //     target_alt,
    //     reindeer->alt,
    //     reindeer->elevator_control);
    
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
