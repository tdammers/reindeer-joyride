#include "game.h"
#include "tilemap.h"
#include "img.h"
#include "asset_ids.h"

#include <allegro5/allegro_primitives.h>
#include <math.h>

typedef struct game_state_t {
    tilemap_t *map;
    double cam_x, cam_y;
    double cam_vx, cam_vy;
} game_state_t;

game_state_t*
create_game_state(const char* map_filename)
{
    game_state_t *state = malloc(sizeof(game_state_t));
    state->map = load_tilemap(map_filename);
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
    state->cam_x += state->cam_vx * dt;
    state->cam_y += state->cam_vy * dt;
}

void game_event(struct app_t* app, const ALLEGRO_EVENT* ev)
{
    game_state_t* state = app->state;
    switch (ev->type) {
        case ALLEGRO_EVENT_KEY_DOWN:
            switch (ev->keyboard.keycode) {
                case ALLEGRO_KEY_LEFT:
                    state->cam_vx -= 32;
                    break;
                case ALLEGRO_KEY_RIGHT:
                    state->cam_vx += 32;
                    break;
                case ALLEGRO_KEY_UP:
                    state->cam_vy -= 32;
                    break;
                case ALLEGRO_KEY_DOWN:
                    state->cam_vy += 32;
                    break;
            }
            break;
        case ALLEGRO_EVENT_KEY_UP:
            switch (ev->keyboard.keycode) {
                case ALLEGRO_KEY_LEFT:
                    state->cam_vx += 32;
                    break;
                case ALLEGRO_KEY_RIGHT:
                    state->cam_vx -= 32;
                    break;
                case ALLEGRO_KEY_UP:
                    state->cam_vy += 32;
                    break;
                case ALLEGRO_KEY_DOWN:
                    state->cam_vy -= 32;
                    break;
            }
            break;
    }
}

void
game_draw(const app_t* app, ALLEGRO_BITMAP* target, const images_t* images)
{
    const game_state_t* state = app->state;
    int x, y;
    int tx, ty;
    int tx0, ty0, tx1, ty1;
    int x0, y0;
    tile_t t;
    ALLEGRO_COLOR color;
    ALLEGRO_BITMAP *tile_bmp;
    al_set_target_bitmap(target);

    tx0 = (int)floor(state->cam_x) >> 5;
    tx1 = tx0 + 11;
    ty0 = (int)floor(state->cam_y) >> 5;
    ty1 = ty0 + 9;
    x0 = -((int)floor(state->cam_x) & 31);
    y0 = -((int)floor(state->cam_y) & 31);

    y = y0;
    for (ty = ty0; ty < ty1; ++ty) {
        x = x0;
        for (tx = tx0; tx < tx1; ++tx) {
            t = tilemap_get(state->map, tx, ty);
            switch (t) {
                case '.':
                    tile_bmp = get_image(images, IMG_ASSET_TILE_SNOW);
                    color = al_map_rgb(240, 248, 255);
                    break;
                case '~':
                    tile_bmp = get_image(images, IMG_ASSET_TILE_WATER);
                    color = al_map_rgb(0, 32, 128);
                    break;
                default:
                    tile_bmp = NULL;
                    color = al_map_rgb(255, 128, 0);
                    break;
            }
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
