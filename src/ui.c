#include "ui.h"
#include "img.h"
#include "asset_ids.h"
#include "menu.h"
#include "game.h"
#include "game_state.h"
#include "util.h"
#include "tilemap.h"

#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <math.h>
#include <stdio.h>

#define ACTION_QUIT 0
#define ACTION_BACK 1
#define ACTION_TRACK_SELECT 3
#define ACTION_MODE_SELECT_TIME_TRIAL 101
#define ACTION_MODE_SELECT_SINGLE_RACE 102
#define ACTION_MODE_SELECT_TOURNAMENT 103
#define ACTION_MODE_SELECT_AI_DEBUG 104

typedef struct ui_state_t {
    bool finished;

    menu_t* main_menu;
    menu_t* track_select_menu;
    menu_t* current_menu;
    menu_t** menu_stack;
    size_t menu_stack_cap;
    size_t menu_stack_len;

    char* track_filename;
    int game_mode;

    double flash_anim_phase;

    app_t* game;
} ui_state_t;

int
list_tracks_callback(
    ALLEGRO_FS_ENTRY* entry,
    void* extra)
{
    menu_t* menu = extra;
    const char* filename = strdup(al_get_fs_entry_name(entry)); 
    printf("%s\n", filename);
    tilemap_meta_t* meta = load_tilemap_meta(filename);
    add_menu_item(menu,
        make_action_menu_item_ex(
            meta->name,
            ACTION_TRACK_SELECT,
            (void*)filename));
    destroy_tilemap_meta(meta);
        
    return ALLEGRO_FOR_EACH_FS_ENTRY_OK;
}


void
list_tracks(menu_t* menu, const char* dirname)
{
    if (!dirname) {
        dirname = "data/maps";
    }
    ALLEGRO_FS_ENTRY* dir = al_create_fs_entry(dirname);
    al_for_each_fs_entry(dir, list_tracks_callback, menu);
    al_destroy_fs_entry(dir);
}

ui_state_t*
create_ui_state()
{
    ui_state_t *state = malloc(sizeof(ui_state_t));

    // Game mode menu
    menu_t* game_mode_menu = make_menu();
    add_menu_item(game_mode_menu,
        make_action_menu_item("TIME TRIAL", ACTION_MODE_SELECT_TIME_TRIAL));
    add_menu_item(game_mode_menu,
        make_action_menu_item("SINGLE RACE", ACTION_MODE_SELECT_SINGLE_RACE));
    add_menu_item(game_mode_menu,
        make_action_menu_item("AI DEBUG", ACTION_MODE_SELECT_AI_DEBUG));
    add_menu_item(game_mode_menu,
        make_action_menu_item("<< BACK", ACTION_BACK));

    menu_t* help_menu = make_menu();
    add_menu_item(help_menu, make_static_menu_item("** CONTROLS **"));
    add_menu_item(help_menu,
        set_menu_item_font(FONT_ASSET_ROBOTO_REGULAR,
        set_menu_item_size(FONT_SIZE_S,
        make_static_menu_item("Cursor L/R: steer left / right"))));
    add_menu_item(help_menu,
        set_menu_item_font(FONT_ASSET_ROBOTO_REGULAR,
        set_menu_item_size(FONT_SIZE_S,
        make_static_menu_item("Cursor Up/Dn: pitch down / up"))));
    add_menu_item(help_menu,
        set_menu_item_font(FONT_ASSET_ROBOTO_REGULAR,
        set_menu_item_size(FONT_SIZE_S,
        make_static_menu_item("Left Shift: accelerate"))));
    add_menu_item(help_menu,
        set_menu_item_font(FONT_ASSET_ROBOTO_REGULAR,
        set_menu_item_size(FONT_SIZE_S,
        make_static_menu_item("Left Ctrl: decelerate"))));
    add_menu_item(help_menu,
        set_menu_item_font(FONT_ASSET_ROBOTO_REGULAR,
        set_menu_item_size(FONT_SIZE_S,
        make_static_menu_item("P / Esc: pause"))));
    add_menu_item(help_menu, make_action_menu_item("<< BACK", ACTION_BACK));

    // Make main menu
    state->main_menu = make_menu();
    state->main_menu->background_image_id = IMG_ASSET_UI_TITLE_SCREEN;
    add_menu_item(state->main_menu,
        make_submenu_menu_item("PLAY", game_mode_menu));
    add_menu_item(state->main_menu,
        make_submenu_menu_item("HELP", help_menu));

    state->track_select_menu = make_menu();
    list_tracks(state->track_select_menu, NULL);
    add_menu_item(state->track_select_menu,
        make_action_menu_item("<< BACK", ACTION_BACK));

    menu_t* quit_menu = make_menu();
    add_menu_item(quit_menu,
        make_static_menu_item("Are you sure?"));
    add_menu_item(quit_menu,
        make_action_menu_item("YES", ACTION_QUIT));
    add_menu_item(quit_menu,
        make_action_menu_item("NO", ACTION_BACK));

    add_menu_item(state->main_menu,
        make_submenu_menu_item("QUIT", quit_menu));
            

    // Preallocate some space for the menu stack
    state->menu_stack_cap = 8;
    state->menu_stack = malloc(sizeof(menu_t*) * state->menu_stack_cap);

    // Activate main menu
    state->current_menu = state->main_menu;

    state->track_filename = NULL;

    return state;
}

void
select_prev_item(menu_t* menu)
{
    size_t buffered_item = menu->selected_item;

    do {
        menu->selected_item++;
        menu->selected_item %= menu->num_items;
    } while (menu->selected_item != buffered_item &&
             menu->items[menu->selected_item]->type == MENU_ITEM_TYPE_STATIC);
}

void
select_next_item(menu_t* menu)
{
    size_t buffered_item = menu->selected_item;

    do {
        menu->selected_item += menu->num_items - 1;
        menu->selected_item %= menu->num_items;
    } while (menu->selected_item != buffered_item &&
             menu->items[menu->selected_item]->type == MENU_ITEM_TYPE_STATIC);
}

void
select_first_item(menu_t* menu)
{
    menu->selected_item = 0;
    select_next_item(menu);
    select_prev_item(menu);
}

void
push_menu(ui_state_t* state, menu_t* menu)
{
    if (state->menu_stack_len == state->menu_stack_cap) {
        state->menu_stack_cap <<= 1;
        state->menu_stack = realloc(state->menu_stack, sizeof(menu_t*) * state->menu_stack_cap);
    }
    state->menu_stack[state->menu_stack_len++] = state->current_menu;
    state->current_menu = menu;
    select_first_item(state->current_menu);
}

void
pop_menu(ui_state_t* state)
{
    if (state->menu_stack_len == 0) return;
    state->current_menu = state->menu_stack[--(state->menu_stack_len)];
}

void
destroy_ui_state(ui_state_t* state)
{
    if (state->game) {
        destroy_app(state->game);
        state->game = NULL;
    }
    destroy_menu(state->main_menu);
    destroy_menu(state->track_select_menu);
    free(state->menu_stack);
    free(state);
}

void ui_destroy(app_t* app)
{
    destroy_ui_state(app->state);
}

void ui_tick(struct app_t* app, double dt)
{
    ui_state_t* state = app->state;
    if (state->game) {
        state->game->tick(state->game, dt);
    }
    else {
        state->flash_anim_phase += dt;
        state->flash_anim_phase = fmod(state->flash_anim_phase, 1.0);
    }
}

void ui_activate_menu(ui_state_t* state)
{
    menu_item_t* item = state->current_menu->items[state->current_menu->selected_item];
    switch (item->type) {
        case MENU_ITEM_TYPE_MENU:
            push_menu(state, item->data);
            break;
        case MENU_ITEM_TYPE_ACTION:
            {
                menu_action_t* action = item->data;
                switch (action->action) {
                    case ACTION_QUIT:
                        state->finished = true;
                        break;
                    case ACTION_BACK:
                        pop_menu(state);
                        break;

                    case ACTION_MODE_SELECT_TIME_TRIAL:
                        state->game_mode = GAME_MODE_TIME_TRIAL;
                        push_menu(state, state->track_select_menu);
                        break;
                    case ACTION_MODE_SELECT_SINGLE_RACE:
                        state->game_mode = GAME_MODE_SINGLE_RACE;
                        push_menu(state, state->track_select_menu);
                        break;
                    case ACTION_MODE_SELECT_AI_DEBUG:
                        state->game_mode = GAME_MODE_AI_DEBUG;
                        push_menu(state, state->track_select_menu);
                        break;

                    case ACTION_TRACK_SELECT:
                        state->track_filename = (char*)action->param;
                        state->game = create_game(state->game_mode, state->track_filename);
                        break;
                }
            }
            break;
    }
}

void ui_event(struct app_t* app, const ALLEGRO_EVENT* ev)
{
    ui_state_t* state = app->state;
    if (state->game) {
        state->game->event(state->game, ev);
        if (state->game->finished(state->game)) {
            destroy_app(state->game);
            state->game = NULL;
        }
    }
    else {
        switch (ev->type) {
            case ALLEGRO_EVENT_KEY_CHAR:
                switch (ev->keyboard.keycode) {
                    case ALLEGRO_KEY_Q:
                        state->finished = true;
                        break;
                    case ALLEGRO_KEY_UP:
                        select_next_item(state->current_menu);
                        break;
                    case ALLEGRO_KEY_DOWN:
                        select_prev_item(state->current_menu);
                        break;
                    case ALLEGRO_KEY_LSHIFT:
                    case ALLEGRO_KEY_SPACE:
                    case ALLEGRO_KEY_ENTER:
                        ui_activate_menu(state);
                        break;
                    case ALLEGRO_KEY_ESCAPE:
                        pop_menu(state);
                        break;
                }
                break;
        }
    }
}

void
ui_draw(const app_t* app, const render_context_t* g)
{
    ui_state_t* state = app->state;
    if (state->game) {
        state->game->draw(state->game, g);
    }
    else {
        double line_height = (double)font_sizes[FONT_SIZE_M] * 1.5;
        al_set_target_bitmap(g->target);
        double x = 160.0;
        double y = (double)(240 - state->current_menu->num_items * line_height) * 0.5;
        al_draw_bitmap(
            get_image(g->images, state->current_menu->background_image_id),
            0, 0, 0);
        for (size_t i = 0; i < state->current_menu->num_items; ++i) {
            menu_item_t* item = state->current_menu->items[i];
            bool selected = i == state->current_menu->selected_item;
            int fontid = (item->font >= 0) ? item->font : FONT_ASSET_UNCIALANTIQUA_REGULAR;
            int fontsize = (item->size >= 0) ? item->size : FONT_SIZE_M;
            al_draw_outlined_text(
                get_font(g->fonts, fontid, fontsize),
                selected ?
                    al_map_rgb(255, (int)floor(192. + sin(state->flash_anim_phase * 2.0 * M_PI) * 63.), 0) :
                    al_map_rgb(240, 32, 0),
                al_map_rgba(0, 0, 0, 200),
                x, y, ALLEGRO_ALIGN_CENTER,
                item->label);
            y += line_height;
        }
    }
}

bool
ui_finished(const app_t* app)
{
    ui_state_t* state = app->state;
    return state->finished;
}

app_t*
create_ui()
{
    app_t *app = create_app();
    app->state = create_ui_state();
    app->destroy = ui_destroy;
    app->draw = ui_draw;
    app->tick = ui_tick;
    app->event = ui_event;
    app->finished = ui_finished;
    return app;
}
