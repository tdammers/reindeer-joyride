#include "game_state.h"

#include "player_brain.h"
#include "ai_brain.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>

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

    state->ranking = malloc(sizeof(size_t) * state->num_reindeer);
    memset(state->ranking, 0, sizeof(size_t) * state->num_reindeer);

    state->brains = malloc(sizeof(brain_t*) * state->num_reindeer);
    memset(state->brains, 0, sizeof(brain_t*) * state->num_reindeer);

    for (size_t i = 0; i < state->num_reindeer; ++i) {
        if (i == 0 && game_mode != GAME_MODE_AI_DEBUG) {
            state->brains[i] = create_player_brain();
        }
        else {
            state->brains[i] = create_ai_brain();
        }
        state->ranking[i] = (size_t)i;
    }
    state->view_mode = 0;
    state->num_laps = 4;

    begin_race(state);

    return state;
}

void
begin_race(game_state_t* state)
{
    if (state->map) destroy_tilemap(state->map);
    state->map = load_tilemap(state->race_filenames[state->current_race]);
    for (size_t i = 0; i < state->num_reindeer; ++i) {
        init_reindeer(state->reindeer + i, NULL);
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
    if (state->ranking) free(state->ranking);
    if (state->brains) {
        for (size_t i = 0; i < state->num_reindeer; ++i) {
            destroy_brain(state->brains[i]);
        }
        free(state->brains);
    }
    free(state);
}

static game_state_t* ranking_game_state = NULL;

int
reindeer_ranking_cb(const void* v1, const void* v2)
{
    const size_t *s1p = v1;
    const size_t *s2p = v2;
    const game_state_t* state = ranking_game_state;
    size_t s1 = *s1p;
    size_t s2 = *s2p;

    const reindeer_t* r1 = state->reindeer + s1;
    const reindeer_t* r2 = state->reindeer + s2;
    
    // First order: lower total race time wins.
    if (r1->race_time != r2->race_time) return fsign(r1->race_time - r2->race_time);

    // Second order: higher lap count wins.
    if (r1->laps_finished != r2->laps_finished) return r2->laps_finished - r1->laps_finished;

    // Third order: higher checkpoint count wins.
    if (r1->next_checkpoint != r2->next_checkpoint) {
        if (r1->next_checkpoint == -1) return -1;
        if (r2->next_checkpoint == -1) return 1;
        return r2->next_checkpoint - r1->next_checkpoint;
    }
    // Fourth order: closest to next checkpoint wins.
    return fsign(get_next_checkpoint_distance(r1, state->map) - get_next_checkpoint_distance(r2, state->map));
}

void
recalc_ranking(game_state_t* state) {
    ranking_game_state = state;
    qsort(
        (void*)state->ranking,
        state->num_reindeer,
        sizeof(size_t),
        reindeer_ranking_cb);
    // printf("-- RANKING: --\n");
    // printf("%1s %8s %1s %3s %8s %5s\n",
    //     "#", "name", "l", "chk", "race", "dist");
    // for (size_t i = 0; i < state->num_reindeer; ++i) {
    //     const reindeer_t* r = state->reindeer + state->ranking[i];
    //     printf("%i %8s %i %3i %8.3fs %5.1f\n",
    //         (int)i,
    //         r->name,
    //         r->laps_finished,
    //         r->next_checkpoint,
    //         r->race_time,
    //         get_next_checkpoint_distance(r, state->map));
    // }
}

