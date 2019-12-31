#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>
#include <math.h>

#include "app.h"
#include "game.h"
#include "ui.h"
#include "img.h"
#include "fonts.h"
#include "util.h"
#include "asset_ids.h"

void
run_app(app_t *app, int fullscreen);

typedef struct options_t {
    const char* track_filename;
    int fullscreen;
} options_t;

void
init_options(options_t* options) {
    memset(options, 0, sizeof(options_t));
    options->track_filename = "data/maps/test.tilemap";
}

void
parse_options(options_t* options, int argc, char **argv)
{
    int i = 1;
    for (i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == '-') {
                // long arg
                const char* arg = argv[i] + 2;
                if (strcmp(arg, "fullscreen") == 0) {
                    options->fullscreen = 1;
                }
                else {
                    char msg[512];
                    snprintf(msg, 512, "Invalid option: %s", argv[i]);
                    die(msg);
                }
            }
        }
        else {
            options->track_filename = argv[i];
        }
    }
}

int
main(int argc, char **argv)
{
    options_t options;
    init_options(&options);
    parse_options(&options, argc, argv);
    app_t *app = create_ui(options.track_filename);
    run_app(app, options.fullscreen);
    destroy_app(app);
}

void
run_app(app_t *app, int fullscreen)
{
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_BITMAP *drawbuf = NULL;
    ALLEGRO_DISPLAY *display = NULL;
    double t, tl, tprev;
    double frame_rate = 256.0;
    double frame_time = 0.0;
    double frame_count = 0.0;
    double fps = 0.0;
    images_t *images = NULL;
    fonts_t *fonts = NULL;

    al_init();

    if (!al_install_keyboard()) {
        die("Installing keyboard failed");
    }

    if (!al_init_primitives_addon()) {
        die("Initializing primitives addon failed");
    }

    if (!al_init_font_addon()) {
        die("Initializing font addon failed");
    }

    if (!al_init_ttf_addon()) {
        die("Initializing ttf addon failed");
    }

    if (!al_init_image_addon()) {
        die("Initializing image addon failed");
    }

    if (fullscreen) {
        al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
    }
    else {
        al_set_new_display_flags(ALLEGRO_WINDOWED);
    }
    display = al_create_display(640, 480);
    if (!display) {
        die("Could not create display");
    }
    fprintf(stderr, "Display size: %ix%i\n",
        al_get_display_width(display),
        al_get_display_height(display));

    event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    drawbuf = al_create_bitmap(320, 240);
    al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
    images = load_images("data/");
    fonts = load_fonts("data/");

    t = tl = tprev = al_get_time();

    while (!(app->finished(app))) {
        tprev = t;
        t = al_get_time();
        while (tl < t) {
            tl += 1.0 / frame_rate;
            if (app->tick) app->tick(app, 1.0 / frame_rate);
        }
        frame_time += t - tprev;
        frame_count += 1.0;
        if (frame_time >= 1.0) {
            fps = frame_count / frame_time;
            frame_time = t - tprev;
            frame_count = 1.0;
        }

        if (app->draw) {
            render_context_t g = { drawbuf, images, fonts };
            app->draw(app, &g);
        }
        al_set_target_backbuffer(display);
        int l, t, w, h;
        int display_w = al_get_display_width(display);
        int display_h = al_get_display_height(display);
        if (display_w * 240 >= display_h * 320) {
            h = display_h;
            w = display_h * 320 / 240;
        }
        else {
            w = display_w;
            h = display_w * 240 / 320;
        }
        l = (display_w - w) / 2;
        t = (display_h - h) / 2;
        al_clear_to_color(al_map_rgb(0,0,0));
        al_draw_scaled_bitmap(drawbuf,
            0, 0, 320, 240,
            l, t, w, h,
            0);
        {
            char str[256];
            snprintf(str, 255, "%3.0f FPS", round(fps));
            al_draw_text(
                get_font(fonts, FONT_ASSET_ROBOTO_MEDIUM, FONT_SIZE_S),
                al_map_rgb(255, 128, 0),
                0, al_get_bitmap_height(al_get_backbuffer(display)) - font_sizes[FONT_SIZE_S],
                0,
                str);
        }
        al_flip_display();
        {
            ALLEGRO_EVENT ev;
            while (al_get_next_event(event_queue, &ev)) {
                if (app->event) app->event(app, &ev);
            }
        }
    }
    al_unregister_event_source(event_queue, al_get_keyboard_event_source());
    al_destroy_event_queue(event_queue);
    unload_images(images);
    unload_fonts(fonts);
    al_destroy_bitmap(drawbuf);
    al_destroy_display(display);
    al_uninstall_keyboard();
}

