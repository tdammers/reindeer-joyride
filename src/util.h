#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

int min(int, int);
int max(int, int);
int mid(int, int, int);

void
al_draw_outlined_text(
    ALLEGRO_FONT* font,
    ALLEGRO_COLOR fg,
    ALLEGRO_COLOR bg,
    int x, int y,
    int mode,
    const char* str);

void
die(const char* msg);
