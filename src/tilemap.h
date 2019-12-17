#pragma once

#include "tile.h"

typedef struct tilemap_t tilemap_t;

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

tilemap_t*
load_tilemap(const char* filename);

double
get_tilemap_start_x(const tilemap_t*);

double
get_tilemap_start_y(const tilemap_t*);

double
get_tilemap_checkpoint_x(const tilemap_t*, int);

double
get_tilemap_checkpoint_y(const tilemap_t*, int);
