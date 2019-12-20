#include "tilemap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "util.h"

struct tilemap_t {
    int w;
    int h;
    tile_t *data;
    int max_checkpoint;
    double start_x;
    double start_y;
    double checkpoint_x[10];
    double checkpoint_y[10];
};

int
hash_tilemap_coords(int x, int y)
{
    int32_t coords[2] = { x, y };
    char* bytes = (char*)coords;
    static const int p = 31;
    static const int m = 1e9 + 9;
    int ppow = 1;
    int hash = 0;
    for (size_t i = 0; i < 2 * sizeof(int32_t); ++i) {
        hash += bytes[i] * ppow;
        hash %= m;
        ppow *= p;
        ppow %= m;
    }
    return hash;
}

tilemap_t*
create_tilemap(int w, int h)
{
    tilemap_t *tilemap = malloc(sizeof(tilemap_t));
    memset(tilemap, 0, sizeof(tilemap_t));
    tilemap->w = w;
    tilemap->h = h;
    tilemap->data = malloc(sizeof(tile_t) * w * h);
    memset(tilemap->data, 0, sizeof(tile_t) * w * h);
    tilemap->max_checkpoint = -1;
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
    int ofs = mid(0, x, m->w-1) + mid(0, y, m->h-1) * m->w;
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


double
get_tilemap_start_x(const tilemap_t* tilemap)
{
    if (!tilemap) return 0;
    return tilemap->start_x;
}

double
get_tilemap_start_y(const tilemap_t* tilemap)
{
    if (!tilemap) return 0;
    return tilemap->start_y;
}

double
get_tilemap_checkpoint_x(const tilemap_t* tilemap, int i)
{
    if (!tilemap) return 0;
    if (i < 0) return 0;
    if (i > tilemap->max_checkpoint) return 0;
    return tilemap->checkpoint_x[i];
}

double
get_tilemap_checkpoint_y(const tilemap_t* tilemap, int i)
{
    if (!tilemap) return 0;
    if (i < 0) return 0;
    if (i > tilemap->max_checkpoint) return 0;
    return tilemap->checkpoint_y[i];
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
    double checkpoint_count[10] = {0};
    double start_count = 0;
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
                int i = c - CHECKPOINT0_TILE;
                if (i > m->max_checkpoint) {
                    m->max_checkpoint = i;
                }
                checkpoint_count[i] += 1;
                m->checkpoint_x[i] += x;
                m->checkpoint_y[i] += y;
            }
            if (c == START_FINISH_TILE) {
                start_count += 1;
                m->start_x += x;
                m->start_y += y;
            }
        }
        c = fgetc(f);
        assert(c == '\n');
    }
    fclose(f);
    m->start_x /= start_count;
    m->start_y /= start_count;
    for (int i = 0; i < 10; ++i) {
        m->checkpoint_x[i] /= checkpoint_count[i];
        m->checkpoint_y[i] /= checkpoint_count[i];
    }
    return m;
}
