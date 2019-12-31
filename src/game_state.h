#pragma once

#include <stdlib.h>
#include <math.h>
#include <allegro5/allegro.h>
#include "reindeer.h"
#include "tilemap.h"
#include "brain.h"

typedef struct game_state_t {
    tilemap_t *map;
    size_t num_reindeer;
    reindeer_t *reindeer;
    size_t *ranking;
    brain_t **brains;
    int view_mode;
    bool paused;
    bool finished;
    size_t num_races;
    size_t current_race;
    char** race_filenames;
    double pre_race_countdown;
    double menu_anim_phase;
    int num_laps;
} game_state_t;

#define VIEW_MODE_TOP_DOWN 0
#define VIEW_MODE_FIRST_PERSON 1

#define GAME_MODE_TIME_TRIAL 0
#define GAME_MODE_AI_DEBUG 1
#define GAME_MODE_SINGLE_RACE 2
#define GAME_MODE_TOURNAMENT 3

game_state_t*
create_game_state(int game_mode, const char* level_filename);

void
destroy_game_state(game_state_t* state);

void
recalc_ranking(game_state_t* state);
