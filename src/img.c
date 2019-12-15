#include "img.h"
#include "asset_ids.h"
#include <allegro5/allegro.h>

struct images_t {
    ALLEGRO_BITMAP** images;
};

images_t*
load_images(const char* datadir)
{
    ALLEGRO_PATH *dir;
    ALLEGRO_PATH *file;
    ALLEGRO_PATH *path;
    int i;
    dir = al_create_path(datadir);
    images_t *images = malloc(sizeof(images_t));
    images->images = malloc(sizeof(ALLEGRO_BITMAP*) * NUM_IMG_ASSETS);
    for (i = 0; i < NUM_IMG_ASSETS; ++i) {
        path = al_clone_path(dir);
        file = al_create_path(img_asset_filenames[i]);
        al_join_paths(path, file);
        al_destroy_path(file);
        images->images[i] =
            al_load_bitmap(al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP));
        al_destroy_path(path);
    }
    al_destroy_path(dir);
    return images;
}

void
unload_images(images_t* images)
{
    int i;
    if (!images) return;
    for (i = 0; i < NUM_IMG_ASSETS; ++i) {
        al_destroy_bitmap(images->images[i]);
    }
    free(images->images);
    free(images);
}

ALLEGRO_BITMAP*
get_image(images_t* images, int i)
{
    if (!images) return NULL;
    if (!images->images) return NULL;
    if (i >= NUM_IMG_ASSETS) return NULL;
    if (i < 0) return NULL;
    return images->images[i];
}
