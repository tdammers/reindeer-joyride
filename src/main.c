#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>
#include <math.h>

#include "app.h"
#include "game.h"
#include "img.h"

void
run_app(app_t *app);

void
die(const char* msg);

int
main(int argc, char **argv)
{
    app_t *game = create_game("data/maps/test.tilemap");
    run_app(game);
    destroy_app(game);
}

void
run_app(app_t *app)
{
    int running = 1;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_BITMAP *drawbuf = NULL;
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_FONT *default_font = NULL;
    double t, tl, tprev;
    double frame_rate = 256.0;
    double frame_time = 0.0;
    images_t *images = NULL;

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

    if (!al_init_image_addon()) {
        die("Initializing image addon failed");
    }

    display = al_create_display(640, 480);
    if (!display) {
        die("Could not create display");
    }

    default_font = al_create_builtin_font();

    event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    images = load_images("data/");

    drawbuf = al_create_bitmap(320, 240);

    t = tl = tprev = al_get_time();

    while (running) {
        tprev = t;
        t = al_get_time();
        while (tl < t) {
            tl += 1.0 / frame_rate;
            if (app->tick) app->tick(app, 1.0 / frame_rate);
        }
        frame_time = t - tprev;

        if (app->draw) app->draw(app, drawbuf, images);
        ALLEGRO_BITMAP* backbuf = al_get_backbuffer(display);
        al_set_target_backbuffer(display);
        al_draw_scaled_bitmap(drawbuf,
            0, 0, 320, 240,
            0, 0, al_get_bitmap_width(backbuf), al_get_bitmap_height(backbuf),
            0);
        {
            char str[256];
            if (frame_time < 0.00000000000001) {
                snprintf(str, 255, "%3.0s FPS", "+++");
            }
            else {
                snprintf(str, 255, "%3.0f FPS", round(1.0 / frame_time));
            }
            al_draw_text(default_font, al_map_rgb(255, 128, 0), 10, 10, 0, str);
        }
        al_flip_display();
        {
            ALLEGRO_EVENT ev;
            while (al_get_next_event(event_queue, &ev)) {
                switch (ev.type) {
                    case ALLEGRO_EVENT_KEY_CHAR:
                        switch (ev.keyboard.keycode) {
                            case ALLEGRO_KEY_ESCAPE:
                                running = false;
                                break;
                        }
                        break;
                }
                if (app->event) app->event(app, &ev);
            }
        }
    }
    al_destroy_font(default_font);
    al_destroy_bitmap(drawbuf);
}

void
die(const char* msg)
{
    fprintf(stdout, "Error: %s\n", msg);
    exit(-1);
}

