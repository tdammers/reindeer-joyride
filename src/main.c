#include <allegro5/allegro.h>
#include <stdio.h>

void die(const char* msg)
{
    fprintf(stdout, "Error: %s\n", msg);
    exit(-1);
}

int main(int argc, char **argv)
{
    int running = 1;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;

    al_init();
    if (!al_install_keyboard()) {
        die("Installing keyboard failed");
    }
    if (!al_create_display(800, 600)) {
        die("Could not create display");
    }
    event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    while (running) {
        al_flip_display();
        {
            ALLEGRO_EVENT ev;
            if (al_get_next_event(event_queue, &ev)) {
                switch (ev.type) {
                    case ALLEGRO_EVENT_KEY_CHAR:
                        {
                            char buf[32];
                            memset(buf, 0, 32);
                            al_utf8_encode(buf, ev.keyboard.unichar);
                            printf("KEY: %s\n", buf);
                        }
                        switch (ev.keyboard.keycode) {
                            case ALLEGRO_KEY_ESCAPE:
                                running = false;
                                break;
                        }
                        break;
                }
            }
        }

    }
}
