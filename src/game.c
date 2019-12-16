#include "game.h"
#include "tilemap.h"
#include "img.h"
#include "asset_ids.h"
#include "mode7.h"
#include "reindeer.h"

#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <math.h>
#include <stdio.h>

typedef struct game_state_t {
    tilemap_t *map;
    reindeer_t reindeer;
    int steering_keys;
    int elevator_keys;
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
    if (state->map) destroy_tilemap(state->map);
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

static void
update_elevator(game_state_t* state)
{
    switch (state->elevator_keys) {
        case 1:
            state->reindeer.elevator_control = -1;
            break;
        case 2:
            state->reindeer.elevator_control = 1;
            break;
        default:
            state->reindeer.elevator_control = 0;
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
                    state->elevator_keys |= 1;
                    update_elevator(state);
                    break;
                case ALLEGRO_KEY_DOWN:
                    state->elevator_keys |= 2;
                    update_elevator(state);
                    break;
                case ALLEGRO_KEY_LSHIFT:
                    state->reindeer.accel_control = 1;
                    break;
                case ALLEGRO_KEY_LCTRL:
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
                    state->elevator_keys &= ~1;
                    update_elevator(state);
                    break;
                case ALLEGRO_KEY_DOWN:
                    state->elevator_keys &= ~2;
                    update_elevator(state);
                    break;
                case ALLEGRO_KEY_LSHIFT:
                    state->reindeer.accel_control = 0;
                    break;
                case ALLEGRO_KEY_LCTRL:
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

ALLEGRO_BITMAP* billboard_tile_image_for(tile_t t, const images_t* images)
{
    switch (t) {
        case TREE_TILE:
            return get_image(images, IMG_ASSET_SPRITE_TREE);
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

void
draw_mode7_billboard_sprite(
    const game_state_t* state,
    const mode7_view* view,
    const render_context_t* g,
    int tx,
    int ty)
{
    double sprite_x, sprite_y, sprite_size;
    tile_t t;
    ALLEGRO_BITMAP* billboard_bmp;

    t = tilemap_get(state->map, tx, ty);
    billboard_bmp = billboard_tile_image_for(t, g->images);
    if (billboard_bmp) {
        if (!mode7_unproject(view,
                &sprite_x, &sprite_y, &sprite_size,
                (tx << 5) + 16, (ty << 5) + 16, 0)) {
            return;
        }
        double sw = (double)al_get_bitmap_width(billboard_bmp);
        double sh = (double)al_get_bitmap_height(billboard_bmp);
        double dw = sprite_size * sw * 0.5;
        double dh = sprite_size * sh * 0.5;
        al_draw_scaled_bitmap(
            billboard_bmp,
            0.0, 0.0,
            sw, sh,
            sprite_x - dw * 0.5,
            sprite_y - dh,
            dw, dh,
            0);
    }
}

void game_draw_mode7(const game_state_t* state, const render_context_t* g)
{
    tile_t t;
    int x, y;
    int tx, ty;
    int tx0, tx1, ty0, ty1;
    int tdx, tdy;
    int sx, sy;
    double screen_w;
    double screen_h;
    double fground_x;
    double fground_y;
    int ground_x;
    int ground_y;
    ALLEGRO_COLOR color;
    ALLEGRO_BITMAP *tile_bmp;
    mode7_view view;

    al_set_target_bitmap(g->target);
    screen_w = al_get_bitmap_width(g->target);
    screen_h = al_get_bitmap_height(g->target);

    view.cam_x = state->reindeer.x;
    view.cam_y = state->reindeer.y;
    view.cam_angle = state->reindeer.angle;
    view.cam_alt = state->reindeer.alt + 16;
    view.screen_w = screen_w;
    view.screen_h = screen_h;
    view.screen_dist = screen_h * 2;
    view.horizon_screen_y = screen_h / 4 + state->reindeer.pitch * screen_h / 8;

    al_draw_filled_rectangle(
        0, 0,
        screen_w, screen_h,
        al_map_rgb(0,0,64));


    al_lock_bitmap(g->target, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READWRITE);

    for (y = view.horizon_screen_y; y < screen_h; ++y) {
        for (x = 0; x < screen_w; ++x) {
            mode7_project(&view, &fground_x, &fground_y, x, y);
            ground_x = (int)floor(fground_x);
            ground_y = (int)floor(fground_y);
            tx = ground_x >> 5;
            ty = ground_y >> 5;
            sx = ground_x & 31;
            sy = ground_y & 31;
            t = tilemap_get(state->map, tx, ty);
            tile_bmp = ground_tile_image_for(t, g->images);
            if (tile_bmp) {
                color = al_get_pixel(tile_bmp, sx, sy);
            }
            else {
                color = tile_color_for(t);
            }
            al_put_pixel(x, y, color);
        }
    }
    
    al_unlock_bitmap(g->target);

    double sa = sin(view.cam_angle);
    double ca = cos(view.cam_angle);

    if (ca > 0.0) {
        ty0 = ((int)floor(state->reindeer.y) >> 5) - 128;
        ty1 = ty0 + 256;
        tdy = 1;
    }
    else {
        ty1 = ((int)floor(state->reindeer.y) >> 5) - 128;
        ty0 = ty1 + 256;
        tdy = -1;
    }

    if (sa < 0.0) {
        tx0 = ((int)floor(state->reindeer.x) >> 5) - 128;
        tx1 = tx0 + 256;
        tdx = 1;
    }
    else {
        tx1 = ((int)floor(state->reindeer.x) >> 5) - 128;
        tx0 = tx1 + 256;
        tdx = -1;
    }

    if (fabs(ca) > fabs(sa)) {
        for (ty = ty0; ty != ty1; ty += tdy) {
            for (tx = tx0; tx != tx1; tx += tdx) {
                draw_mode7_billboard_sprite(state, &view, g, tx, ty);
            }
        }
    }
    else {
        for (tx = tx0; tx != tx1; tx += tdx) {
            for (ty = ty0; ty != ty1; ty += tdy) {
                draw_mode7_billboard_sprite(state, &view, g, tx, ty);
            }
        }
    }

    al_draw_bitmap(
        get_image(g->images, IMG_ASSET_SPRITE_REINDEER_FPV),
        0, 80.0 + sin(state->reindeer.bob_phase) * state->reindeer.bob_strength * 20.0,
        0);
}

void game_draw_top_down(const game_state_t* state, const render_context_t* g)
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

    al_set_target_bitmap(g->target);
    screen_w = al_get_bitmap_width(g->target);
    screen_h = al_get_bitmap_height(g->target);

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
            tile_bmp = ground_tile_image_for(t, g->images);
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
draw_stats(const game_state_t* state, const render_context_t* g)
{
    char str[512];
    int x = 2;
    int y = 2;

    snprintf(str, 512,
        "SPD: %3.0f",
        state->reindeer.v);
    al_draw_text(
        g->font,
        al_map_rgb(255, 128, 0),
        x, y, 0,
        str);
    y += 10;

    snprintf(str, 512,
        "PITCH: %+5.3f",
        state->reindeer.pitch);
    al_draw_text(
        g->font,
        al_map_rgb(255, 128, 0),
        x, y, 0,
        str);
    y += 10;

    snprintf(str, 512,
        "ALT: %3.0f (%+3.0f)",
        state->reindeer.alt,
        state->reindeer.valt);
    al_draw_text(
        g->font,
        al_map_rgb(255, 128, 0),
        x, y, 0,
        str);
    y += 10;
}

void
game_draw(const app_t* app, const render_context_t* g)
{
    game_state_t* state = app->state;
    switch (state->view_mode) {
        case 1:
            game_draw_mode7(state, g);
            break;
        default:
            game_draw_top_down(state, g);
            break;
    }
    draw_stats(state, g);
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
