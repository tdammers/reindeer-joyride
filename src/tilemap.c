#include "tilemap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

struct tilemap_t {
    int w;
    int h;
    tile_t *data;
    int max_checkpoint;
    int start_x;
    int start_y;
};

tilemap_t*
create_tilemap(int w, int h)
{
    tilemap_t *tilemap = malloc(sizeof(tilemap_t));
    tilemap->w = w;
    tilemap->h = h;
    tilemap->data = malloc(sizeof(tile_t) * w * h);
    tilemap->max_checkpoint = -1;
    memset(tilemap->data, 0, sizeof(tile_t) * w * h);
    return tilemap;
}

void
destroy_tilemap(tilemap_t *tilemap)
{
    free(tilemap->data);
    free(tilemap);
}

tile_t
tilemap_get(const tilemap_t* m, int x, int y)
{
    if (!m) return EMPTY_TILE;
    int ofs = x + y * m->w;
    if (x < 0 || x >= m->w || y < 0 || y >= m->h) return EMPTY_TILE;
    return m->data[ofs];
}

int
get_tilemap_width(const tilemap_t *tilemap)
{
    if (!tilemap) return 0;
    return tilemap->w;
}

int
get_tilemap_height(const tilemap_t *tilemap)
{
    if (!tilemap) return 0;
    return tilemap->h;
}


int
get_tilemap_start_x(const tilemap_t* tilemap)
{
    if (!tilemap) return 0;
    return tilemap->start_x;
}

int
get_tilemap_start_y(const tilemap_t* tilemap)
{
    if (!tilemap) return 0;
    return tilemap->start_y;
}

int
get_tilemap_max_checkpoint(const tilemap_t *tilemap)
{
    if (!tilemap) return 0;
    return tilemap->max_checkpoint;
}

void
tilemap_set(tilemap_t* m, int x, int y, tile_t t)
{
    if (!m) return;
    int ofs = x + y * m->w;
    if (x < 0 || x >= m->w || y < 0 || y >= m->h) return;
    m->data[ofs] = t;
}

tilemap_t*
load_tilemap(const char* filename)
{
    int w, h;
    char c;
    tilemap_t *m;
    FILE *f = fopen(filename, "r");
    if (!f) {
        return NULL;
    }
    fscanf(f, "%i %i\n", &w, &h);
    m = create_tilemap(w, h);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            c = fgetc(f);
            tilemap_set(m, x, y, c);
            if (c >= CHECKPOINT0_TILE && c <= CHECKPOINT9_TILE) {
                int checkpoint = c - CHECKPOINT0_TILE;
                if (checkpoint > m->max_checkpoint) {
                    m->max_checkpoint = checkpoint;
                }
            }
            if (c == START_FINISH_TILE) {
                m->start_x = x;
                m->start_y = y;
            }
        }
        c = fgetc(f);
        assert(c == '\n');
    }
    fclose(f);
    return m;
}
