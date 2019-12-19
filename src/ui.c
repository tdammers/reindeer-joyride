#include "ui.h"
#include "img.h"
#include "asset_ids.h"
#include "menu.h"
#include "game.h"

#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <math.h>
#include <stdio.h>

#define ACTION_QUIT 0
#define ACTION_BACK 1
#define ACTION_PLAY 2

typedef struct ui_state_t {
    bool finished;

    menu_t* main_menu;
    menu_t* current_menu;
    menu_t** menu_stack;
    size_t menu_stack_cap;
    size_t menu_stack_len;

    char* track_filename;

    app_t* game;
} ui_state_t;

ui_state_t*
create_ui_state(const char* track_filename)
{
    ui_state_t *state = malloc(sizeof(ui_state_t));

    // Make main menu
    state->main_menu = make_menu();
    state->main_menu->background_image_id = IMG_ASSET_UI_TITLE_SCREEN;
    add_menu_item(state->main_menu,
        make_action_menu_item("PLAY", ACTION_PLAY));

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

    state->track_filename = strdup(track_filename);

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
    menu->selected_item = menu->num_items - 1;
    select_next_item(menu);
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
    }
    destroy_menu(state->main_menu);
    free(state->menu_stack);
    free(state->track_filename);
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
        (void)app;
        (void)dt;
        (void)state;
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
            switch (((menu_action_t*)item->data)->action) {
                case ACTION_QUIT:
                    state->finished = true;
                    break;
                case ACTION_BACK:
                    pop_menu(state);
                    break;
                case ACTION_PLAY:
                    state->game = create_game(state->track_filename);
                    break;
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
                        select_prev_item(state->current_menu);
                        break;
                    case ALLEGRO_KEY_DOWN:
                        select_next_item(state->current_menu);
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
        al_set_target_bitmap(g->target);
        double x = 160.0;
        double y = (double)(240 - state->current_menu->num_items * 10) * 0.5;
        al_draw_bitmap(
            get_image(g->images, state->current_menu->background_image_id),
            0, 0, 0);
        for (size_t i = 0; i < state->current_menu->num_items; ++i) {
            menu_item_t* item = state->current_menu->items[i];
            bool selected = i == state->current_menu->selected_item;
            al_draw_text(
                g->font,
                al_map_rgba(0, 0, 0, 200),
                x+1, y+1, ALLEGRO_ALIGN_CENTER,
                item->label);
                
            al_draw_text(
                g->font,
                selected ? al_map_rgb(255, 128, 0) : al_map_rgb(255, 255, 255),
                x, y, ALLEGRO_ALIGN_CENTER,
                item->label);

            y += 10;
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
create_ui(const char* track_filename)
{
    app_t *app = create_app();
    app->state = create_ui_state(track_filename);
    app->destroy = ui_destroy;
    app->draw = ui_draw;
    app->tick = ui_tick;
    app->event = ui_event;
    app->finished = ui_finished;
    return app;
}
