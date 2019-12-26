#include "brain.h"
#include "reindeer.h"
#include "game_state.h"

brain_t*
create_brain()
{
    brain_t* brain = malloc(sizeof(brain_t));
    memset(brain, 0, sizeof(brain_t));
    return brain;
}

void
update_brain(
    brain_t* brain,
    size_t reindeer_index,
    const struct game_state_t* state)
{
    if (brain && brain->update) {
        brain->update(brain->state, reindeer_index, state);
    }
}

bool
brain_input(
    brain_t* brain,
    const ALLEGRO_EVENT* e)
{
    if (brain && brain->input) {
        return brain->input(brain->state, e);
    }
    else {
        return false;
    }
}

void
destroy_brain(brain_t* brain)
{
    if (!brain) return;
    if (brain->destroy) {
        brain->destroy(brain->state);
    }
    free(brain);
}
