#pragma once

typedef unsigned short tile_t;

#define EMPTY_TILE '.'
#define ARROW_N_TILE '^'
#define ARROW_S_TILE 'v'
#define ARROW_W_TILE '<'
#define ARROW_E_TILE '>'
#define WATER_TILE '~'
#define TREE_TILE 'T'
#define HOUSE_TILE 'H'
#define CANDYSTICK_TILE 'J'
#define CHECKPOINT0_TILE '0'
#define CHECKPOINT1_TILE '1'
#define CHECKPOINT2_TILE '2'
#define CHECKPOINT3_TILE '3'
#define CHECKPOINT4_TILE '4'
#define CHECKPOINT5_TILE '5'
#define CHECKPOINT6_TILE '6'
#define CHECKPOINT7_TILE '7'
#define CHECKPOINT8_TILE '8'
#define CHECKPOINT9_TILE '9'
#define START_FINISH_TILE '#'

double
tile_obstacle_top(tile_t);

double
tile_obstacle_bottom(tile_t);

double
tile_ground_elev(tile_t);

double
tile_ai_elev(tile_t);
