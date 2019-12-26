#pragma once

#include <stdlib.h>
#include <math.h>
#include <allegro5/allegro.h>

struct game_state_t;

typedef struct brain_t {
    void* state;
    void (*update)(
        void* state,
        size_t reindeer_index,
        const struct game_state_t* game_state);
    bool (*input)(
        void* state,
        const ALLEGRO_EVENT* ev);
    void (*destroy)(void* brain);
} brain_t;

brain_t*
create_brain();

void
update_brain(
    brain_t* brain,
    size_t reindeer_index,
    const struct game_state_t* state);

bool
brain_input(
    brain_t* brain,
    const ALLEGRO_EVENT* e);

void
destroy_brain(brain_t* brain);
