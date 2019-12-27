#include "game_state.h"

#include "player_brain.h"
#include "ai_brain.h"

game_state_t*
create_game_state(const char* map_filename)
{
    game_state_t *state = malloc(sizeof(game_state_t));
    memset(state, 0, sizeof(game_state_t));
    state->map = load_tilemap(map_filename);

    state->num_reindeer = 4;

    state->reindeer = malloc(sizeof(reindeer_t) * state->num_reindeer);
    memset(state->reindeer, 0, sizeof(reindeer_t) * state->num_reindeer);

    state->brains = malloc(sizeof(brain_t*) * state->num_reindeer);
    memset(state->brains, 0, sizeof(brain_t*) * state->num_reindeer);

    for (size_t i = 0; i < state->num_reindeer; ++i) {
        init_reindeer(state->reindeer + i);
        state->reindeer[i].x = (get_tilemap_start_x(state->map) * 32.0) + 16 + i * 32 - 32 * (state->num_reindeer - 1) * 0.5;
        state->reindeer[i].y = ((get_tilemap_start_y(state->map) + 1) * 32.0) + 16;
        state->brains[i] = i ? create_ai_brain() : create_player_brain();
        // state->brains[i] = create_ai_brain();
    }
    state->view_mode = 0;

    return state;
}

void
destroy_game_state(game_state_t* state)
{
    if (state->map) destroy_tilemap(state->map);
    if (state->reindeer) free(state->reindeer);
    if (state->brains) {
        for (size_t i = 0; i < state->num_reindeer; ++i) {
            destroy_brain(state->brains[i]);
        }
        free(state->brains);
    }
    free(state);
}

