#pragma once

#include <allegro5/allegro.h>

typedef struct images_t images_t;

images_t*
load_images(const char* datadir);

void
unload_images(images_t*);

ALLEGRO_BITMAP*
get_image(const images_t*, int);
