#include "tile.h"

/**
 * The height, above ground, of the obstacle on the given tile.
 */
double
tile_obstacle_height(tile_t t) {
    switch (t) {
        case TREE_TILE: return 64;
        case HOUSE_TILE: return 48;
        case CANDYSTICK_TILE: return 64;
        default: return 0;
    }
}
