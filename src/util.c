#include "util.h"

int min(int a, int b) { return (a < b) ? a : b; }
int max(int a, int b) { return (a > b) ? a : b; }
int mid(int lo, int val, int hi) { return min(hi, max(lo, val)); }

void
al_draw_outlined_text(
    ALLEGRO_FONT* font,
    ALLEGRO_COLOR fg,
    ALLEGRO_COLOR bg,
    int x, int y,
    int mode,
    const char* str)
{
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            al_draw_text(font, bg, x+dx, y+dy, mode, str);
        }
    }
    al_draw_text(font, fg, x, y, mode, str);
}
