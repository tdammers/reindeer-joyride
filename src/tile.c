#include "tile.h"

double
tile_obstacle_top(tile_t t) {
    if ((t >= CHECKPOINT0_TILE && t <= CHECKPOINT9_TILE) ||
        (t == START_FINISH_TILE))
        return 48 + 32;
    return 0;
}

double
tile_obstacle_bottom(tile_t t) {
    if ((t >= CHECKPOINT0_TILE && t <= CHECKPOINT9_TILE) ||
        (t == START_FINISH_TILE))
        return 48 - 16;
    return 0;
}

double
tile_ground_elev(tile_t t) {
    switch (t) {
        case TREE_TILE: return 64;
        case HOUSE_TILE: return 48;
        case CANDYSTICK_TILE: return 64;
        default: return 0;
    }
}

double
tile_ai_elev(tile_t t) {
    switch (t) {
        case TREE_TILE: return 76;
        case HOUSE_TILE: return 60;
        case CANDYSTICK_TILE: return 76;
        case WATER_TILE: return 16;
        default: return 0;
    }
}
