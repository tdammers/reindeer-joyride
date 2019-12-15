#pragma once

#include "tile.h"

typedef struct tilemap_t tilemap_t;

tilemap_t*
create_tilemap(int w, int h);

int
get_tilemap_width(tilemap_t *tilemap);

int
get_tilemap_height(tilemap_t *tilemap);

void
destroy_tilemap(tilemap_t *tilemap);

tile_t
tilemap_get(const tilemap_t*, int, int);

void
tilemap_set(tilemap_t*, int, int, tile_t);

tilemap_t*
load_tilemap(const char* filename);
