#include "game_state.h"

#include "player_brain.h"
#include "ai_brain.h"
#include "util.h"

void
begin_race(game_state_t* state);

game_state_t*
create_game_state(int game_mode, const char* level_filename)
{
    game_state_t *state = malloc(sizeof(game_state_t));
    memset(state, 0, sizeof(game_state_t));

    switch (game_mode) {
        case GAME_MODE_TIME_TRIAL:
        case GAME_MODE_AI_DEBUG:
            state->num_reindeer = 1;
            break;
        case GAME_MODE_SINGLE_RACE:
            state->num_reindeer = 4;
            break;
    }

    switch (game_mode) {
        case GAME_MODE_TOURNAMENT:
            // aaaargh!
            die("Tournament mode not implemented yet!");
            break;
        default:
            state->num_races = 1;
            state->race_filenames = malloc(sizeof(char*));
            state->race_filenames[0] = strdup(level_filename);
            break;
    }

    state->current_race = 0;

    state->reindeer = malloc(sizeof(reindeer_t) * state->num_reindeer);
    memset(state->reindeer, 0, sizeof(reindeer_t) * state->num_reindeer);

    state->brains = malloc(sizeof(brain_t*) * state->num_reindeer);
    memset(state->brains, 0, sizeof(brain_t*) * state->num_reindeer);

    for (size_t i = 0; i < state->num_reindeer; ++i) {
        if (i == 0 && game_mode != GAME_MODE_AI_DEBUG) {
            state->brains[i] = create_player_brain();
        }
        else {
            state->brains[i] = create_ai_brain();
        }
    }
    state->view_mode = 0;

    begin_race(state);

    return state;
}

void
begin_race(game_state_t* state)
{
    if (state->map) destroy_tilemap(state->map);
    state->map = load_tilemap(state->race_filenames[state->current_race]);
    for (size_t i = 0; i < state->num_reindeer; ++i) {
        init_reindeer(state->reindeer + i);
        state->reindeer[i].x = (get_tilemap_start_x(state->map) * 32.0) + 16 + i * 32 - 32 * (state->num_reindeer - 1) * 0.5;
        state->reindeer[i].y = ((get_tilemap_start_y(state->map) + 1) * 32.0) + 16;
    }
    state->pre_race_countdown = 5.0;
}

void
destroy_game_state(game_state_t* state)
{
    if (state->race_filenames) {
        for (size_t i = 0; i < state->num_races; ++i) {
            free(state->race_filenames[i]);
        }
        free(state->race_filenames);
    }
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

