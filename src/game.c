#include "game.h"
#include "tilemap.h"
#include "img.h"
#include "asset_ids.h"
#include "mode7.h"
#include "reindeer.h"

#include <allegro5/allegro_primitives.h>
#include <math.h>

typedef struct game_state_t {
    tilemap_t *map;
    reindeer_t reindeer;
    int steering_keys;
    int view_mode;
} game_state_t;

#define VIEW_MODE_TOP_DOWN 0
#define VIEW_MODE_FIRST_PERSON 1

game_state_t*
create_game_state(const char* map_filename)
{
    game_state_t *state = malloc(sizeof(game_state_t));
    state->map = load_tilemap(map_filename);
    init_reindeer(&(state->reindeer));

    return state;
}

void
destroy_game_state(game_state_t* state)
{
    destroy_tilemap(state->map);
    free(state);
}

void game_destroy(app_t* app)
{
    assert(app->state);
    destroy_game_state(app->state);
}

void game_tick(struct app_t* app, double dt)
{
    game_state_t* state = app->state;
    update_reindeer(&(state->reindeer), dt);
}

static void
update_steering(game_state_t* state)
{
    switch (state->steering_keys) {
        case 1:
            state->reindeer.turn_control = -1;
            break;
        case 2:
            state->reindeer.turn_control = 1;
            break;
        default:
            state->reindeer.turn_control = 0;
            break;
    }
}

void game_event(struct app_t* app, const ALLEGRO_EVENT* ev)
{
    game_state_t* state = app->state;
    switch (ev->type) {
        case ALLEGRO_EVENT_KEY_CHAR:
            switch (ev->keyboard.keycode) {
                case ALLEGRO_KEY_V:
                    state->view_mode ^= 1;
                    break;
            }
            break;
        case ALLEGRO_EVENT_KEY_DOWN:
            switch (ev->keyboard.keycode) {
                case ALLEGRO_KEY_LEFT:
                    state->steering_keys |= 1;
                    update_steering(state);
                    break;
                case ALLEGRO_KEY_RIGHT:
                    state->steering_keys |= 2;
                    update_steering(state);
                    break;
                case ALLEGRO_KEY_UP:
                    state->reindeer.accel_control = 1;
                    break;
                case ALLEGRO_KEY_DOWN:
                    state->reindeer.brake_control = 1;
                    break;
            }
            break;
        case ALLEGRO_EVENT_KEY_UP:
            switch (ev->keyboard.keycode) {
                case ALLEGRO_KEY_LEFT:
                    state->steering_keys &= ~1;
                    update_steering(state);
                    break;
                case ALLEGRO_KEY_RIGHT:
                    state->steering_keys &= ~2;
                    update_steering(state);
                    break;
                case ALLEGRO_KEY_UP:
                    state->reindeer.accel_control = 0;
                    break;
                case ALLEGRO_KEY_DOWN:
                    state->reindeer.brake_control = 0;
                    break;
            }
            break;
    }
}

ALLEGRO_BITMAP* ground_tile_image_for(tile_t t, const images_t* images)
{
    switch (t) {
        case EMPTY_TILE:
        case TREE_TILE:
        case HOUSE_TILE:
            return get_image(images, IMG_ASSET_TILE_SNOW);
        case WATER_TILE:
            return get_image(images, IMG_ASSET_TILE_WATER);
        default:
            return NULL;
    }
}

ALLEGRO_COLOR tile_color_for(tile_t t)
{
    switch (t) {
        case EMPTY_TILE:
        case TREE_TILE:
        case HOUSE_TILE:
            return al_map_rgb(240, 248, 255);
        case WATER_TILE:
            return al_map_rgb(0, 32, 128);
        default:
            return al_map_rgb(255, 128, 0);
    }
}


void game_draw_mode7(const game_state_t* state, ALLEGRO_BITMAP* target, const images_t* images)
{
    tile_t t;
    int x, y;
    int tx, ty;
    double screen_w;
    double screen_h;
    double fground_x;
    double fground_y;
    int ground_x;
    int ground_y;
    ALLEGRO_COLOR color;
    ALLEGRO_BITMAP *tile_bmp;
    mode7_view view;

    al_set_target_bitmap(target);
    screen_w = al_get_bitmap_width(target);
    screen_h = al_get_bitmap_height(target);

    view.cam_x = state->reindeer.x;
    view.cam_y = state->reindeer.y;
    view.cam_angle = state->reindeer.angle;
    view.cam_alt = 16;
    view.screen_w = screen_w;
    view.screen_h = screen_h;
    view.screen_dist = screen_h * 2;
    view.horizon_screen_y = screen_h / 4;

    al_lock_bitmap(target, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READWRITE);

    for (y = screen_h / 4; y < screen_h; ++y) {
        for (x = 0; x < screen_w; ++x) {
            mode7_project(&view, &fground_x, &fground_y, x, y);
            ground_x = (int)floor(fground_x);
            ground_y = (int)floor(fground_y);
            tx = ground_x >> 5;
            ty = ground_y >> 5;
            t = tilemap_get(state->map, tx, ty);
            tile_bmp = ground_tile_image_for(t, images);
            if (tile_bmp) {
                color = al_get_pixel(tile_bmp, ground_x & 31, ground_y & 31);
            }
            else {
                color = tile_color_for(t);
            }
            al_put_pixel(x, y, color);
        }
    }
    
    al_unlock_bitmap(target);

    al_draw_filled_rectangle(
        0, 0, screen_w, screen_h / 4,
        al_map_rgb(0,0,64));

    al_draw_bitmap(
        get_image(images, IMG_ASSET_SPRITE_REINDEER_FPV),
        0, 80.0 + sin(state->reindeer.bob_phase) * state->reindeer.bob_strength * 20.0,
        0);
}

void game_draw_top_down(const game_state_t* state, ALLEGRO_BITMAP* target, const images_t* images)
{
    int x, y;
    int tx, ty;
    int tx0, ty0, tx1, ty1;
    int x0, y0;
    tile_t t;
    double screen_w;
    double screen_h;
    double sa = sin(state->reindeer.angle);
    double ca = cos(state->reindeer.angle);
    ALLEGRO_COLOR color;
    ALLEGRO_BITMAP *tile_bmp;

    al_set_target_bitmap(target);
    screen_w = al_get_bitmap_width(target);
    screen_h = al_get_bitmap_height(target);

    tx0 = (int)floor(state->reindeer.x - screen_w / 2) >> 5;
    tx1 = tx0 + 11;
    ty0 = (int)floor(state->reindeer.y - screen_h / 2) >> 5;
    ty1 = ty0 + 9;
    x0 = -((int)floor(state->reindeer.x - screen_w / 2) & 31);
    y0 = -((int)floor(state->reindeer.y - screen_h / 2) & 31);

    y = y0;
    for (ty = ty0; ty < ty1; ++ty) {
        x = x0;
        for (tx = tx0; tx < tx1; ++tx) {
            t = tilemap_get(state->map, tx, ty);
            tile_bmp = ground_tile_image_for(t, images);
            color = tile_color_for(t);
            if (tile_bmp) {
                al_draw_bitmap(tile_bmp, x, y, 0);
            }
            else {
                al_draw_filled_rectangle(
                    x, y, x + 32, y + 32,
                    color);
            }
            x += 32;
        }
        y += 32;
    }

    al_draw_line(
        screen_w / 2.0,
        screen_h / 2.0,
        screen_w / 2.0 + sa * 16,
        screen_h / 2.0 - ca * 16,
        al_map_rgb(255, 0, 0),
        1);
}

void
game_draw(const app_t* app, ALLEGRO_BITMAP* target, const images_t* images)
{
    game_state_t* state = app->state;
    switch (state->view_mode) {
        case 1:
            game_draw_mode7(state, target, images);
            break;
        default:
            game_draw_top_down(state, target, images);
            break;
    }
}

app_t*
create_game(const char *map_filename)
{
    app_t *app = create_app();
    app->state = create_game_state(map_filename);
    app->destroy = game_destroy;
    app->draw = game_draw;
    app->tick = game_tick;
    app->event = game_event;
    return app;
}
