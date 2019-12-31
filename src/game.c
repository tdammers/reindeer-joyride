#include "game.h"
#include "tilemap.h"
#include "img.h"
#include "asset_ids.h"
#include "mode7.h"
#include "util.h"
#include "game_state.h"

#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <math.h>
#include <stdio.h>

void game_destroy(app_t* app)
{
    assert(app->state);
    destroy_game_state(app->state);
}

void game_tick(struct app_t* app, double dt)
{
    game_state_t* state = app->state;

    if (state->paused) {
        // TODO: pause mode animations
    }
    else if (state->pre_race_countdown > 0.0) {
        state->pre_race_countdown -= dt;
    }
    else {
        state->menu_anim_phase += dt;
        state->menu_anim_phase = fmod(state->menu_anim_phase, 1.0);
        for (size_t i = 0; i < state->num_reindeer; ++i) {
            update_brain(state->brains[i], i, state);
        }
        for (size_t i = 0; i < state->num_reindeer; ++i) {
            update_reindeer(state->reindeer + i, state->map, state->num_laps, dt);
        }
        recalc_ranking(state);
    }
}

void game_event(struct app_t* app, const ALLEGRO_EVENT* ev)
{
    game_state_t* state = app->state;
    for (size_t i = 0; i < state->num_reindeer; ++i) {
        if (brain_input(state->brains[i], ev)) break;
    }
    if (state->paused) {
        switch (ev->type) {
            case ALLEGRO_EVENT_KEY_CHAR:
                switch (ev->keyboard.keycode) {
                    case ALLEGRO_KEY_P:
                    case ALLEGRO_KEY_ESCAPE:
                        state->paused = false;
                        break;
                    case ALLEGRO_KEY_Q:
                        state->finished = true;
                        break;
                }
                break;
        }
    }
    else {
        switch (ev->type) {
            case ALLEGRO_EVENT_KEY_CHAR:
                switch (ev->keyboard.keycode) {
                    case ALLEGRO_KEY_P:
                    case ALLEGRO_KEY_ESCAPE:
                        state->paused = true;
                        break;
                    case ALLEGRO_KEY_V:
                        state->view_mode++;
                        state->view_mode %= 3;
                        break;
                }
                break;
        }
    }

}

ALLEGRO_BITMAP* ground_tile_image_for(tile_t t, const images_t* images)
{
    switch (t) {
        case EMPTY_TILE:
        case HOUSE_TILE:
            return get_image(images, IMG_ASSET_TILE_SNOW);
        case TREE_TILE:
        case CANDYSTICK_TILE:
            return get_image(images, IMG_ASSET_TILE_SNOW_SHADOW);
        case ARROW_N_TILE:
            return get_image(images, IMG_ASSET_TILE_SNOW_N);
        case ARROW_S_TILE:
            return get_image(images, IMG_ASSET_TILE_SNOW_S);
        case ARROW_W_TILE:
            return get_image(images, IMG_ASSET_TILE_SNOW_W);
        case ARROW_E_TILE:
            return get_image(images, IMG_ASSET_TILE_SNOW_E);
        case WATER_TILE:
            return get_image(images, IMG_ASSET_TILE_WATER);
        case START_FINISH_TILE:
            return get_image(images, IMG_ASSET_TILE_START_FINISH);
        case CHECKPOINT0_TILE:
        case CHECKPOINT1_TILE:
        case CHECKPOINT2_TILE:
        case CHECKPOINT3_TILE:
        case CHECKPOINT4_TILE:
        case CHECKPOINT5_TILE:
        case CHECKPOINT6_TILE:
        case CHECKPOINT7_TILE:
        case CHECKPOINT8_TILE:
        case CHECKPOINT9_TILE:
            return get_image(images, IMG_ASSET_TILE_SNOW_DARK);
        default:
            return NULL;
    }
}

ALLEGRO_BITMAP* billboard_tile_image_for(tile_t t, const images_t* images, int variation)
{
    switch (t) {
        case TREE_TILE:
            return get_image(images, IMG_ASSET_SPRITE_TREE);
        case HOUSE_TILE:
            return get_image(images, IMG_ASSET_SPRITE_HOUSE1);
        case CANDYSTICK_TILE:
            return get_image(images, IMG_ASSET_SPRITE_CANDYSTICK00 + (variation & 3));
        case START_FINISH_TILE:
            return get_image(images, IMG_ASSET_TILE_START_FINISH);
        case CHECKPOINT0_TILE:
        case CHECKPOINT1_TILE:
        case CHECKPOINT2_TILE:
        case CHECKPOINT3_TILE:
        case CHECKPOINT4_TILE:
        case CHECKPOINT5_TILE:
        case CHECKPOINT6_TILE:
        case CHECKPOINT7_TILE:
        case CHECKPOINT8_TILE:
        case CHECKPOINT9_TILE:
            return get_image(images, IMG_ASSET_TILE_CHECKPOINT);
        default: return NULL;
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

double
billboard_ground_elev_for(tile_t t)
{
    switch (t) {
        case START_FINISH_TILE:
        case CHECKPOINT0_TILE:
        case CHECKPOINT1_TILE:
        case CHECKPOINT2_TILE:
        case CHECKPOINT3_TILE:
        case CHECKPOINT4_TILE:
        case CHECKPOINT5_TILE:
        case CHECKPOINT6_TILE:
        case CHECKPOINT7_TILE:
        case CHECKPOINT8_TILE:
        case CHECKPOINT9_TILE:
            return 48.0;
        case TREE_TILE:
            return -3.0;
        case CANDYSTICK_TILE:
            return -3.0;
        default:
            return 0.0;
    }
}

void
draw_mode7_billboard_sprite(
    const mode7_view* view,
    const render_context_t* g,
    double x, double y, double elev,
    ALLEGRO_BITMAP* bmp)
{
    double sprite_x, sprite_y, sprite_size;

    (void)g;

    if (!mode7_unproject(view,
            &sprite_x, &sprite_y, &sprite_size,
            x, y, 0)) {
        return;
    }
    double sw = (double)al_get_bitmap_width(bmp);
    double sh = (double)al_get_bitmap_height(bmp);
    double dw = sprite_size * sw;
    double dh = sprite_size * sh;
    al_draw_scaled_bitmap(
        bmp,
        0.0, 0.0,
        sw, sh,
        sprite_x - dw * 0.5,
        sprite_y - dh - elev * sprite_size,
        dw, dh,
        0);
}

void
draw_mode7_billboard_tile_sprite(
    const game_state_t* state,
    const mode7_view* view,
    const render_context_t* g,
    int tx,
    int ty)
{
    tile_t t;
    ALLEGRO_BITMAP* billboard_bmp;
    double elev = 0.0;
    int variation = hash_tilemap_coords(tx, ty);
    t = tilemap_get(state->map, tx, ty);
    billboard_bmp = billboard_tile_image_for(t, g->images, variation);
    elev = billboard_ground_elev_for(t);
    if (billboard_bmp) {
        draw_mode7_billboard_sprite(
            view, g, (tx << 5) + 16, (ty << 5) + 16, elev,
            billboard_bmp);
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

    view.cam_x = state->reindeer[0].x;
    view.cam_y = state->reindeer[0].y;
    view.cam_angle = state->reindeer[0].angle;
    view.cam_alt =
        state->reindeer[0].alt +
        cos(state->reindeer[0].bob_phase) * state->reindeer[0].bob_strength * 2.0 +
        16.0;
    view.screen_w = screen_w;
    view.screen_h = screen_h;
    view.screen_dist = screen_h * 2;
    view.horizon_screen_y = screen_h / 4 + state->reindeer[0].pitch * screen_h / 8;

    ALLEGRO_BITMAP* background = get_image(g->images, IMG_ASSET_BACKGROUND_NIGHT_SKY);
    int bg_w = al_get_bitmap_width(background);
    int bg_h = al_get_bitmap_height(background);
    int bg_x = (int)round(-view.cam_angle * (double)bg_w * 0.5 / M_PI) % bg_w;
    int bg_y = view.horizon_screen_y - bg_h + 1;
    for (x = bg_x - bg_w; x < screen_w; x += bg_w) {
        al_draw_bitmap(
            background,
            x, bg_y,
            0);
    }

    al_lock_bitmap(g->target, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READWRITE);

    for (y = view.horizon_screen_y + 1; y < screen_h; ++y) {
        for (x = 0; x < screen_w; ++x) {
            mode7_project(&view, &fground_x, &fground_y, x, y);
            ground_x = (int)floor(fground_x);
            ground_y = (int)floor(fground_y);
            tx = ground_x >> 5;
            ty = ground_y >> 5;
            sx = ground_x & 31;
            sy = ground_y & 31;
            t = tilemap_get(state->map, tx, ty);
            if (state->reindeer[0].next_checkpoint >= 0 &&
                t == CHECKPOINT0_TILE + state->reindeer[0].next_checkpoint) {
                tile_bmp = get_image(g->images, IMG_ASSET_TILE_CHECKPOINT_GROUND);
            }
            else {
                tile_bmp = ground_tile_image_for(t, g->images);
            }
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
    int view_range = 64;

    if (ca > 0.0) {
        ty0 = ((int)floor(state->reindeer[0].y) >> 5) - view_range;
        ty1 = ty0 + view_range * 2;
        tdy = 1;
    }
    else {
        ty1 = ((int)floor(state->reindeer[0].y) >> 5) - view_range;
        ty0 = ty1 + view_range * 2;
        tdy = -1;
    }

    if (sa < 0.0) {
        tx0 = ((int)floor(state->reindeer[0].x) >> 5) - view_range;
        tx1 = tx0 + view_range * 2;
        tdx = 1;
    }
    else {
        tx1 = ((int)floor(state->reindeer[0].x) >> 5) - view_range;
        tx0 = tx1 + view_range * 2;
        tdx = -1;
    }

    if (fabs(ca) > fabs(sa)) {
        for (ty = ty0; ty != ty1; ty += tdy) {
            for (tx = tx0; tx != tx1; tx += tdx) {
                draw_mode7_billboard_tile_sprite(state, &view, g, tx, ty);
            }
        }
    }
    else {
        for (tx = tx0; tx != tx1; tx += tdx) {
            for (ty = ty0; ty != ty1; ty += tdy) {
                draw_mode7_billboard_tile_sprite(state, &view, g, tx, ty);
            }
        }
    }

    al_draw_bitmap(
        get_image(g->images, IMG_ASSET_SPRITE_REINDEER_FPV),
        0, 80.0 + sin(state->reindeer[0].bob_phase) * state->reindeer[0].bob_strength * 20.0,
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
    ALLEGRO_COLOR color;
    ALLEGRO_BITMAP *tile_bmp;

    al_set_target_bitmap(g->target);
    screen_w = al_get_bitmap_width(g->target);
    screen_h = al_get_bitmap_height(g->target);

    tx0 = (int)floor(state->reindeer[0].x - screen_w / 2) >> 5;
    tx1 = tx0 + 11;
    ty0 = (int)floor(state->reindeer[0].y - screen_h / 2) >> 5;
    ty1 = ty0 + 9;
    x0 = -((int)floor(state->reindeer[0].x - screen_w / 2) & 31);
    y0 = -((int)floor(state->reindeer[0].y - screen_h / 2) & 31);

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

    ALLEGRO_BITMAP* reindeer_bmp = get_image(g->images, IMG_ASSET_SPRITE_REINDEER_TOPDOWN);

    for (size_t i = 0; i < state->num_reindeer; ++i) {
        double sa = sin(state->reindeer[i].angle);
        double ca = cos(state->reindeer[i].angle);
        double xx = screen_w / 2.0 + state->reindeer[i].x - state->reindeer[0].x;
        double yy = screen_h / 2.0 + state->reindeer[i].y - state->reindeer[0].y;
        if (reindeer_bmp) {
            al_draw_rotated_bitmap(
                reindeer_bmp,
                16.0, 16.0,
                xx, yy,
                state->reindeer[i].angle,
                0);
        }
        else {
            al_draw_line(
                xx, yy,
                xx + sa * 16, yy - ca * 16,
                al_map_rgb(255, 0, 0),
                1);
        }
    }
}

ALLEGRO_COLOR map_tile_color(tile_t t) {
    switch (t) {
        case TREE_TILE:
            return al_map_rgb(0, 128, 0);
        case CANDYSTICK_TILE:
        case HOUSE_TILE:
            return al_map_rgb(96, 0, 0);

        case CHECKPOINT0_TILE:
        case CHECKPOINT1_TILE:
        case CHECKPOINT2_TILE:
        case CHECKPOINT3_TILE:
        case CHECKPOINT4_TILE:
        case CHECKPOINT5_TILE:
        case CHECKPOINT6_TILE:
        case CHECKPOINT7_TILE:
        case CHECKPOINT8_TILE:
        case CHECKPOINT9_TILE:
        case START_FINISH_TILE:
            return al_map_rgb(0, 255, 0);

        case WATER_TILE:
            return al_map_rgb(128, 192, 255);
        default:
            return al_map_rgb(255, 255, 255);
    }
}

void game_draw_map(const game_state_t* state, const render_context_t* g)
{
    tile_t t;
    double screen_w;
    double screen_h;
    ALLEGRO_COLOR color;

    al_set_target_bitmap(g->target);
    screen_w = al_get_bitmap_width(g->target);
    screen_h = al_get_bitmap_height(g->target);

    int tx0 = (int)round(state->reindeer->x / 32 - screen_w / 2);
    int ty0 = (int)round(state->reindeer->y / 32 - screen_h / 2);

    al_lock_bitmap(g->target, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READWRITE);
    for (int sy = 0; sy < screen_h; ++sy) {
        for (int sx = 0; sx < screen_w; ++sx) {
            t = tilemap_get(state->map, sx + tx0, sy + ty0);
            color = map_tile_color(t);
            al_put_pixel(sx, sy, color);
        }
    }

    color = al_map_rgb(200, 100, 0);
    for (size_t i = 0; i < state->num_reindeer; ++i) {
        al_put_pixel(
            screen_w / 2 + (state->reindeer[i].x - state->reindeer[0].x) / 32,
            screen_h / 2 + (state->reindeer[i].y - state->reindeer[0].y) / 32,
            color);
    }
    al_unlock_bitmap(g->target);
}

char*
stopwatch_fmt(char* buf, size_t bufsize, double t)
{
    int m = (int)floor(t / 60.0);
    double s = t - (double)m * 60.0;
    snprintf(buf, bufsize, "%02i:%06.3f", m, s);
    return buf;
}

void
draw_game_overlay(const game_state_t* state, const render_context_t* g)
{
    if (state->pre_race_countdown > 3.0) {
        double fac = fmin(1.0, (state->pre_race_countdown - 3.0));
        ALLEGRO_COLOR fg = al_map_rgba(255 * fac, 128 * fac, 0, 255 * fac);
        ALLEGRO_COLOR bg = al_map_rgba(0, 0, 0, 128 * fac);
        al_draw_outlined_text(
            get_font(g->fonts, FONT_ASSET_UNCIALANTIQUA_REGULAR, FONT_SIZE_L),
            fg, bg,
            160, 120 - 0.5 * font_sizes[FONT_SIZE_L],
            ALLEGRO_ALIGN_CENTER,
            get_tilemap_meta(state->map)->name);
    }
    else if (state->pre_race_countdown > 0.0) {
        int secs = (int)floor(state->pre_race_countdown);
        double fac = state->pre_race_countdown - (double)secs;
        char buf[16];
        snprintf(buf, 16, "%i", secs + 1);
        ALLEGRO_COLOR fg = al_map_rgba(255 * fac, 128 * fac, 0, 255 * fac);
        ALLEGRO_COLOR bg = al_map_rgba(0, 0, 0, 128 * fac);
        al_draw_outlined_text(
            get_font(g->fonts, FONT_ASSET_UNCIALANTIQUA_REGULAR, FONT_SIZE_L),
            fg, bg,
            160, 120 - 0.5 * font_sizes[FONT_SIZE_L],
            ALLEGRO_ALIGN_CENTER,
            buf);
    }
}

void
draw_alti(const reindeer_t* reindeer, double cx, double cy, const render_context_t* g)
{
    ALLEGRO_BITMAP* clock_bmp = get_image(g->images, IMG_ASSET_UI_ALTI);
    double angle = M_PI * 2.0 * (reindeer->alt + 0.5) / 256.0;
    double sa = sin(angle);
    double ca = cos(angle);
    double dx = sa * 13.0;
    double dy = ca * -13.0;
    al_draw_bitmap(clock_bmp, cx - 15, cy - 15, 0);
    al_draw_line(cx, cy, cx + dx, cy + dy, al_map_rgb(240, 240, 240), 1.0);
}

void
draw_asi(const reindeer_t* reindeer, double cx, double cy, const render_context_t* g)
{
    ALLEGRO_BITMAP* clock_bmp = get_image(g->images, IMG_ASSET_UI_ASI);
    double v = hypotf(reindeer->vx, reindeer->vy);
    double angle = 
            (v < 20.0)
            ?
            (M_PI * 0.25 + M_PI * 0.25 * v / 20.0)
            :
            (M_PI * 0.5 + M_PI * 1.0 * (v - 20.0) / (reindeer->max_speed - 20.0));
    angle = fmax(M_PI * 0.25, fmin(angle, M_PI * 1.75));
    double sa = sin(angle);
    double ca = cos(angle);
    double dx = sa * 13.0;
    double dy = ca * -13.0;
    al_draw_bitmap(clock_bmp, cx - 15, cy - 15, 0);
    al_draw_line(cx, cy, cx + dx, cy + dy, al_map_rgb(240, 240, 240), 1.0);
}

void
draw_nav(const game_state_t* state, double cx, double cy, const render_context_t* g)
{
    ALLEGRO_BITMAP* clock_bmp = get_image(g->images, IMG_ASSET_UI_NAV);
    ALLEGRO_BITMAP* rose_bmp = get_image(g->images, IMG_ASSET_UI_NAV_ROSE);
    ALLEGRO_BITMAP* arrow_bmp = get_image(g->images, IMG_ASSET_UI_NAV_ARROW);
    double arrow_dir = get_next_checkpoint_heading(state->reindeer, state->map) - state->reindeer[0].angle;
    al_draw_bitmap(clock_bmp, cx - 15, cy - 15, 0);
    al_draw_rotated_bitmap(rose_bmp, 16, 16, cx + 1, cy + 1, -state->reindeer[0].angle, 0);
    al_draw_rotated_bitmap(arrow_bmp, 16, 16, cx + 1, cy + 1, arrow_dir, 0);
}

void
draw_stats(const game_state_t* state, const render_context_t* g)
{
    int x = 2;
    int y = 2;
    int mode = ALLEGRO_ALIGN_LEFT;

    if (state->paused) {
        al_draw_filled_rectangle(0, 0, 320, 240, al_map_rgba(0, 0, 0, 64));
    }

    draw_asi(state->reindeer, 128, 220, g);
    draw_alti(state->reindeer, 160, 220, g);
    draw_nav(state, 192, 220, g);

    if (state->paused) {
        int i = 0;
        int num_lines = 0;
        char lines[512][32];

        snprintf(lines[i++], 512,
            "PAUSED");
        snprintf(lines[i++], 512,
            "P - CONTINUE");
        snprintf(lines[i++], 512,
            "Q -   QUIT  ");
        snprintf(lines[i++], 512,
            "------------");

        x = 160;
        y = (240 - 16 * i) * 0.5;
        mode = ALLEGRO_ALIGN_CENTRE;

        num_lines = i;

        for (int i = 0; i < num_lines; ++i) {
            al_draw_outlined_text(
                get_font(g->fonts, FONT_ASSET_UNCIALANTIQUA_REGULAR, FONT_SIZE_M),
                al_map_rgb(255, 128, 0),
                al_map_rgba(0, 0, 0, 200),
                x, y, mode,
                lines[i]);
            y += 16;
        }
    }
    else {
        char buf[256];
        char tbuf[256];

        ALLEGRO_FONT* font_s = get_font(g->fonts, FONT_ASSET_ROBOTO_MEDIUM, FONT_SIZE_S);
        ALLEGRO_FONT* font_m = get_font(g->fonts, FONT_ASSET_ROBOTO_MEDIUM, FONT_SIZE_M);
        ALLEGRO_COLOR white = al_map_rgb(255, 255, 255);
        ALLEGRO_COLOR gray = al_map_rgb(192, 192, 192);
        ALLEGRO_COLOR green = al_map_rgb(192, 255, 192);
        ALLEGRO_COLOR bg = al_map_rgba(0, 0, 0, 128);

        double y = 2;

        snprintf(buf, 256,
            "Race: %s",
            stopwatch_fmt(tbuf, 256, state->reindeer[0].race_time));
        al_draw_outlined_text(
            font_m,
            white, bg,
            2, y, ALLEGRO_ALIGN_LEFT,
            buf);

        y += 2 + font_sizes[FONT_SIZE_M];

        for (int i = 0; i < min(state->reindeer[0].laps_finished, state->num_laps); ++i) {
            snprintf(buf, 256,
                "Lap %i: %s",
                i + 1,
                stopwatch_fmt(tbuf, 256, state->reindeer[0].lap_times[i]));
            al_draw_outlined_text(
                font_s,
                (i == state->reindeer[0].best_lap) ? green : gray, bg,
                2, y, ALLEGRO_ALIGN_LEFT,
                buf);
            y += 2 + font_sizes[FONT_SIZE_S];
        }
        if (state->reindeer[0].laps_finished < state->num_laps) {
            snprintf(buf, 256,
                "Lap %i: %s",
                state->reindeer[0].laps_finished + 1,
                stopwatch_fmt(tbuf, 256, state->reindeer[0].current_lap_time));
            al_draw_outlined_text(
                font_s,
                white, bg,
                2, y, ALLEGRO_ALIGN_LEFT,
                buf);
            y += 2 + font_sizes[FONT_SIZE_S];
        }

        int font = FONT_ASSET_ROBOTO_MEDIUM;
        int size = FONT_SIZE_S;
        double x0 = 320 - font_sizes[size] * 10;
        double y0 = 2;
        if (state->reindeer[0].laps_finished >= state->num_laps) {
            font = FONT_ASSET_UNCIALANTIQUA_REGULAR;
            size = FONT_SIZE_M;
            y0 = 120 - (0.5 * (double)state->num_reindeer) * font_sizes[size];
            x0 = 80;
        }
        for (size_t i = 0; i < state->num_reindeer; ++i) {
            reindeer_t* reindeer = state->reindeer + state->ranking[i];
            double fac = sin(state->menu_anim_phase * 2.0 * M_PI);
            ALLEGRO_COLOR fg =
                (state->ranking[i] == 0) ?
                al_map_rgb(192 + 64 * fac, 192 + 64 * fac, 32) :
                (reindeer->laps_finished >= state->num_laps) ?
                al_map_rgb(255, 255, 255) :
                al_map_rgb(240, 120, 0);
            ALLEGRO_COLOR bg = al_map_rgba(0, 0, 0, 128);
            double y = y0 + (double)i * font_sizes[size];
            snprintf(buf, 256, "%i", (int)(i + 1));
            al_draw_outlined_text(
                get_font(g->fonts, font, size),
                fg, bg,
                x0, y,
                ALLEGRO_ALIGN_LEFT,
                buf);
            al_draw_outlined_text(
                get_font(g->fonts, font, size),
                fg, bg,
                x0 + font_sizes[size], y,
                ALLEGRO_ALIGN_LEFT,
                reindeer->name);
            if (reindeer->laps_finished >= state->num_laps) {
                stopwatch_fmt(buf, 256, reindeer->race_time);
                al_draw_outlined_text(
                    get_font(g->fonts, font, size),
                    fg, bg,
                    x0 + font_sizes[size] * 6, y,
                    ALLEGRO_ALIGN_LEFT,
                    buf);
            }
        }
    }
}

void
game_draw(const app_t* app, const render_context_t* g)
{
    game_state_t* state = app->state;
    switch (state->view_mode % 3) {
        case 0:
            game_draw_mode7(state, g);
            break;
        case 1:
            game_draw_top_down(state, g);
            break;
        case 2:
            game_draw_map(state, g);
            break;
    }
    draw_game_overlay(state, g);
    draw_stats(state, g);
}

bool
game_finished(const app_t* app)
{
    game_state_t* state = app->state;
    return state->finished;
}

app_t*
create_game(int mode, const char *map_filename)
{
    app_t *app = create_app();
    app->state = create_game_state(mode, map_filename);
    app->destroy = game_destroy;
    app->draw = game_draw;
    app->tick = game_tick;
    app->event = game_event;
    app->finished = game_finished;
    return app;
}
