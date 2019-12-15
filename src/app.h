#pragma once

#include <allegro5/allegro.h>
#include "img.h"

typedef struct app_t {
    void (*init)(struct app_t*);
    void (*destroy)(struct app_t*);
    void (*draw)(const struct app_t*, ALLEGRO_BITMAP*, const images_t*);
    void (*tick)(struct app_t*, double);
    void (*event)(struct app_t*, const ALLEGRO_EVENT*);
    void *state;
} app_t;

app_t*
create_app();

void
destroy_app(app_t*);
