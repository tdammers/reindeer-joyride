#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include "img.h"

typedef struct render_context_t {
    ALLEGRO_BITMAP* target;
    images_t* images;
    ALLEGRO_FONT* font;
} render_context_t;

typedef struct app_t {
    void (*init)(struct app_t*);
    void (*destroy)(struct app_t*);
    void (*draw)(const struct app_t*, const render_context_t*);
    void (*tick)(struct app_t*, double);
    void (*event)(struct app_t*, const ALLEGRO_EVENT*);
    void *state;
} app_t;

app_t*
create_app();

void
destroy_app(app_t*);
