#include "tilemap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "util.h"

void
destroy_tilemap_meta(tilemap_meta_t* m)
{
    if (!m) return;
    free(m->name);
    free(m->description);
    free(m);
}

struct tilemap_t {
    int w;
    int h;
    tile_t *data;
    int max_checkpoint;
    double start_x;
    double start_y;
    double checkpoint_x[10];
    double checkpoint_y[10];
    waypoint_list_t* ai_waypoints;
    tilemap_meta_t* meta;
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

waypoint_list_t*
create_waypoint_list()
{
    waypoint_list_t* l = malloc(sizeof(waypoint_list_t));
    memset(l, 0, sizeof(waypoint_list_t));
    return l;
}

void
destroy_waypoint_list(waypoint_list_t* l)
{
    if (!l) return;
    free(l->points);
    free(l);
}

void
push_waypoint(waypoint_list_t* l, waypoint_t w)
{
    if (l->num >= l->cap) {
        if (l->cap == 0) {
            l->cap = 4;
        }
        else {
            l->cap <<= 1;
        }
        l->points = realloc(l->points, sizeof(waypoint_t) * l->cap);
    }
    memcpy(l->points + l->num, &w, sizeof(waypoint_t));
    l->num++;
}

void
destroy_tilemap(tilemap_t *tilemap)
{
    free(tilemap->data);
    destroy_waypoint_list(tilemap->ai_waypoints);
    free(tilemap);
    destroy_tilemap_meta(tilemap->meta);
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

tilemap_meta_t*
load_tilemap_meta_f(FILE* f)
{
    char buf[1024];
    size_t cur_len = 0;
    size_t total_len = 0;

    tilemap_meta_t* m = malloc(sizeof(tilemap_meta_t));
    memset(m, 0, sizeof(tilemap_meta_t));
    if (!fgets(buf, 1024, f)) return m;
    m->name = strdup(buf);

    do {
        if (feof(f)) break;
        if (!fgets(buf, 1024, f)) break;
        if (strcmp(buf, "-----\n") == 0) break;
        cur_len = strlen(buf);
        m->description = realloc(m->description, total_len + cur_len + 1);
        memcpy(m->description + total_len, buf, cur_len + 1);
        total_len += cur_len;
    } while (1);

    return m;
}

const waypoint_list_t*
get_tilemap_ai_waypoints(const tilemap_t* t)
{
    return t->ai_waypoints;
}

const tilemap_meta_t*
get_tilemap_meta(const tilemap_t* t)
{
    return t->meta;
}


tilemap_meta_t*
load_tilemap_meta(const char* filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) {
        return NULL;
    }
    tilemap_meta_t* m = load_tilemap_meta_f(f);
    fclose(f);
    return m;
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
    tilemap_meta_t* meta = load_tilemap_meta_f(f);
    fscanf(f, "%i %i\n", &w, &h);
    m = create_tilemap(w, h);
    m->meta = meta;
    waypoint_t letter_wps[26];
    memset(letter_wps, 0, sizeof(waypoint_t) * 26);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            c = fgetc(f);
            if (c >= 'a' && c <= 'z') {
                int wpi = (int)c - (int)'a';
                letter_wps[wpi].x = x;
                letter_wps[wpi].y = y;
                letter_wps[wpi].max_alt = 256;
                letter_wps[wpi].flyover = false;
                c = EMPTY_TILE;
            }
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
    m->start_x /= start_count;
    m->start_y /= start_count;
    for (int i = 0; i < 10; ++i) {
        m->checkpoint_x[i] /= checkpoint_count[i];
        m->checkpoint_y[i] /= checkpoint_count[i];
    }

    while (!feof(f)) {
        char buf[1024];
        if (!fgets(buf, 1024, f)) break;
        if (strcmp(buf, "ai-waypoints\n") == 0) {
            printf("Load AI waypoints\n");
            if (!m->ai_waypoints) {
                m->ai_waypoints = create_waypoint_list();
            }
            fgets(buf, 1024, f);
            printf("Waypoint list: %s\n", buf);
            for (char* cp = buf; *cp; ++cp) {
                char c = *cp;
                if (c >= 'a' && c <= 'z') {
                    printf("Add AI waypoint (%c): %3.0f, %3.0f\n",
                        c,
                        letter_wps[c - 'a'].x,
                        letter_wps[c - 'a'].y);
                    push_waypoint(m->ai_waypoints, letter_wps[c - 'a']);
                }
                else if (c >= '0' && c <= '9') {
                    waypoint_t wp;
                    int i = c - '0';
                    wp.x = m->checkpoint_x[i];
                    wp.y = m->checkpoint_y[i];
                    wp.max_alt = 8;
                    wp.flyover = true;
                    wp.checkpoint = i;
                    printf("Add AI waypoint (%c): %3.0f, %3.0f\n", c, wp.x, wp.y);
                    push_waypoint(m->ai_waypoints, wp);
                }
                else if (c == '#') {
                    waypoint_t wp;
                    wp.x = m->start_x;
                    wp.y = m->start_y;
                    wp.max_alt = 8;
                    wp.flyover = true;
                    wp.checkpoint = -1;
                    printf("Add AI waypoint (%c): %3.0f, %3.0f\n", c, wp.x, wp.y);
                    push_waypoint(m->ai_waypoints, wp);
                }
                else if (c == '\n') {
                    continue;
                }
                else {
                    fprintf(stderr, "Warning: invalid waypoint '%c'\n", c);
                }
            }

        }
        else {
            fprintf(stderr, "Unknown entry: %s\n", buf);
            fgets(buf, 1024, f);
        }
    }
    fclose(f);
    return m;
}
