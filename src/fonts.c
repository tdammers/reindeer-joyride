#include "fonts.h"
#include "asset_ids.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

const int font_sizes[NUM_FONT_SIZES] = {
    11,  /* FONT_SIZE_S */
    14, /* FONT_SIZE_M */
    18, /* FONT_SIZE_L */
};

struct fonts_t {
    ALLEGRO_FONT** fonts;
};

fonts_t*
load_fonts(const char* datadir)
{
    ALLEGRO_PATH *dir;
    ALLEGRO_PATH *file;
    ALLEGRO_PATH *path;
    dir = al_create_path(datadir);
    fonts_t *fonts = malloc(sizeof(fonts_t));
    fonts->fonts = malloc(sizeof(ALLEGRO_FONT*) * NUM_FONT_ASSETS * NUM_FONT_SIZES);
    for (size_t i = 0; i < NUM_FONT_ASSETS; ++i) {
        path = al_clone_path(dir);
        file = al_create_path(font_asset_filenames[i]);
        al_join_paths(path, file);
        al_destroy_path(file);
        for (size_t s = 0; s < NUM_FONT_SIZES; ++s) {
            fonts->fonts[i * NUM_FONT_SIZES + s] =
                al_load_font(
                    al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP),
                    font_sizes[s], ALLEGRO_TTF_MONOCHROME);
        }
        al_destroy_path(path);
    }
    al_destroy_path(dir);
    return fonts;
}

void
unload_fonts(fonts_t* fonts)
{
    if (!fonts) return;
    for (size_t i = 0; i < NUM_FONT_ASSETS * NUM_FONT_SIZES; ++i) {
        al_destroy_font(fonts->fonts[i]);
    }
    free(fonts->fonts);
    free(fonts);
}

ALLEGRO_FONT*
get_font(const fonts_t* fonts, int i, int s)
{
    if (!fonts) return NULL;
    if (!fonts->fonts) return NULL;
    if (i >= NUM_FONT_ASSETS) return NULL;
    if (i < 0) return NULL;
    if (s >= NUM_FONT_SIZES) return NULL;
    if (s < 0) return NULL;
    return fonts->fonts[i * NUM_FONT_SIZES + s];
}
