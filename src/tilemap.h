#pragma once

#include "tile.h"
#include <stdlib.h>

typedef struct tilemap_t tilemap_t;

typedef struct tilemap_meta_t {
    char* name;
    char* description;
} tilemap_meta_t;

typedef struct waypoint_t {
    double x;
    double y;
    double max_alt;
    int flyover;
    int checkpoint;
} waypoint_t;

typedef struct waypoint_list_t {
    size_t num;
    size_t cap;
    waypoint_t* points;
} waypoint_list_t;


int
hash_tilemap_coords(int x, int y);

tilemap_t*
create_tilemap(int w, int h);

int
get_tilemap_width(const tilemap_t *tilemap);

int
get_tilemap_height(const tilemap_t *tilemap);

int
get_tilemap_max_checkpoint(const tilemap_t *tilemap);

void
destroy_tilemap(tilemap_t *tilemap);

tile_t
tilemap_get(const tilemap_t*, int, int);

void
tilemap_set(tilemap_t*, int, int, tile_t);

tilemap_meta_t*
load_tilemap_meta(const char* filename);

void
destroy_tilemap_meta(tilemap_meta_t*);

tilemap_t*
load_tilemap(const char* filename);

const waypoint_list_t*
get_tilemap_ai_waypoints(const tilemap_t*);

double
get_tilemap_start_x(const tilemap_t*);

double
get_tilemap_start_y(const tilemap_t*);

double
get_tilemap_checkpoint_x(const tilemap_t*, int);

double
get_tilemap_checkpoint_y(const tilemap_t*, int);
