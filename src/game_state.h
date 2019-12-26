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
    brain_t **brains;
    int view_mode;
    double checkpoint_anim_phase;
    bool paused;
    bool finished;
} game_state_t;

#define VIEW_MODE_TOP_DOWN 0
#define VIEW_MODE_FIRST_PERSON 1

game_state_t*
create_game_state(const char* map_filename);

void
destroy_game_state(game_state_t* state);
