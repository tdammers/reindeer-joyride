#include "game.h"
#include "tilemap.h"

#include <allegro5/allegro_primitives.h>

typedef struct game_state_t {
    tilemap_t *map;
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

void destroy_game(app_t* app) {
    assert(app->state);
    destroy_game_state(app->state);
}

void
draw_game(const app_t* app, ALLEGRO_BITMAP* target)
{
    const game_state_t* state = app->state;
    int x, y;
    tile_t t;
    ALLEGRO_COLOR color;
    al_set_target_bitmap(target);
    for (y = 0; y < get_tilemap_height(state->map); ++y) {
        for (x = 0; x < get_tilemap_width(state->map); ++x) {
            t = tilemap_get(state->map, x, y);
            switch (t) {
                case '.':
                    color = al_map_rgb(240, 248, 255);
                    break;
                case '~':
                    color = al_map_rgb(0, 32, 128);
                    break;
                default:
                    color = al_map_rgb(255, 128, 0);
                    break;
            }
            al_draw_filled_rectangle(
                x * 32, y * 32,
                x * 32 + 32, y * 32 + 32,
                color);
        }
    }
}

app_t*
create_game(const char *map_filename)
{
    app_t *app = create_app();
    app->state = create_game_state(map_filename);
    app->destroy = destroy_game;
    app->draw = draw_game;
    return app;
}
