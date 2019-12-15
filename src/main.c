#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>

#include "app.h"
#include "game.h"

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
    double t, tl;
    double frame_rate = 30.0;

    al_init();

    if (!al_install_keyboard()) {
        die("Installing keyboard failed");
    }

    if (!al_init_primitives_addon()) {
        die("Initializing primitives addon failed");
    }

    display = al_create_display(800, 600);
    if (!display) {
        die("Could not create display");
    }

    event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    drawbuf = al_create_bitmap(320, 240);

    app->init(app);

    t = tl = al_get_time();

    while (running) {
        tl = t;
        t = al_get_time();
        while (tl < t) {
            tl += 1.0 / frame_rate;
            app->tick(app);
        }

        app->draw(app, drawbuf);
        ALLEGRO_BITMAP* backbuf = al_get_backbuffer(display);
        al_set_target_backbuffer(display);
        al_draw_scaled_bitmap(drawbuf,
            0, 0, 320, 240,
            0, 0, al_get_bitmap_width(backbuf), al_get_bitmap_height(backbuf),
            0);
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
                app->event(app, &ev);
            }
        }
    }
}

void
die(const char* msg)
{
    fprintf(stdout, "Error: %s\n", msg);
    exit(-1);
}

