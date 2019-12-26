#include "player_brain.h"
#include "reindeer.h"
#include "game_state.h"

typedef struct player_brain_state_t {
    int steering_keys;
    int elevator_keys;
    int accel_control;
    int brake_control;
} player_brain_state_t;

player_brain_state_t*
create_player_brain_state()
{
    player_brain_state_t* pbs = malloc(sizeof(player_brain_state_t));
    memset(pbs, 0, sizeof(player_brain_state_t));
    return pbs;
}

void
destroy_player_brain_state(void* vstate)
{
    free(vstate);
}

void
update_player_brain(
    void* vstate,
    size_t reindeer_index,
    const struct game_state_t* game_state)
{
    player_brain_state_t* state = (player_brain_state_t*)vstate;
    reindeer_t* reindeer = game_state->reindeer + reindeer_index;

    reindeer->accel_control = state->accel_control;
    reindeer->brake_control = state->brake_control;

    switch (state->steering_keys) {
        case 1:
            reindeer->turn_control = -1;
            break;
        case 2:
            reindeer->turn_control = 1;
            break;
        default:
            reindeer->turn_control = 0;
            break;
    }
    switch (state->elevator_keys) {
        case 1:
            reindeer->elevator_control = -1;
            break;
        case 2:
            reindeer->elevator_control = 1;
            break;
        default:
            reindeer->elevator_control = 0;
            break;
    }
}

bool
player_brain_input(void* vstate, const ALLEGRO_EVENT* ev)
{
    player_brain_state_t* state = vstate;
    switch (ev->type) {
        case ALLEGRO_EVENT_KEY_DOWN:
            switch (ev->keyboard.keycode) {
                case ALLEGRO_KEY_LEFT:
                    state->steering_keys |= 1;
                    return true;
                case ALLEGRO_KEY_RIGHT:
                    state->steering_keys |= 2;
                    return true;
                case ALLEGRO_KEY_UP:
                    state->elevator_keys |= 1;
                    return true;
                case ALLEGRO_KEY_DOWN:
                    state->elevator_keys |= 2;
                    return true;
                case ALLEGRO_KEY_LSHIFT:
                    state->accel_control = 1;
                    return true;
                case ALLEGRO_KEY_LCTRL:
                    state->brake_control = 1;
                    return true;
            }
            break;
        case ALLEGRO_EVENT_KEY_UP:
            switch (ev->keyboard.keycode) {
                case ALLEGRO_KEY_LEFT:
                    state->steering_keys &= ~1;
                    return true;
                case ALLEGRO_KEY_RIGHT:
                    state->steering_keys &= ~2;
                    return true;
                case ALLEGRO_KEY_UP:
                    state->elevator_keys &= ~1;
                    return true;
                case ALLEGRO_KEY_DOWN:
                    state->elevator_keys &= ~2;
                    return true;
                case ALLEGRO_KEY_LSHIFT:
                    state->accel_control = 0;
                    return true;
                case ALLEGRO_KEY_LCTRL:
                    state->brake_control = 0;
                    return true;
            }
            break;
    }
    return false;
}

brain_t*
create_player_brain()
{
    brain_t* brain = create_brain();
    brain->state = create_player_brain_state();
    brain->update = update_player_brain;
    brain->input = player_brain_input;
    brain->destroy = destroy_player_brain_state;
    return brain;
}
