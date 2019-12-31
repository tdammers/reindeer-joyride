#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#define FONT_SIZE_S 0
#define FONT_SIZE_M 1
#define FONT_SIZE_L 2
#define NUM_FONT_SIZES 3

typedef struct fonts_t fonts_t;

fonts_t*
load_fonts(const char* datadir);

void
unload_fonts(fonts_t*);

ALLEGRO_FONT*
get_font(const fonts_t*, int, int);

extern const int font_sizes[NUM_FONT_SIZES];
