#include "app.h"

void
def_app_init(struct app_t* app) { (void)app; }

void
def_app_destroy(struct app_t* app) { (void)app; }

void
def_app_draw(const struct app_t* app, ALLEGRO_BITMAP* target) { (void)app; (void)target; }

void
def_app_tick(struct app_t* app) { (void)app; }

void
def_app_event(struct app_t* app, const ALLEGRO_EVENT* ev) { (void)app; (void)ev; }

app_t*
create_app()
{
    app_t *app = malloc(sizeof(app_t));
    memset(app, 0, sizeof(app_t));
    app->init = def_app_init;
    app->destroy = def_app_destroy;
    app->draw = def_app_draw;
    app->event = def_app_event;
    app->tick = def_app_tick;
    return app;
}

void
destroy_app(app_t* app)
{
    if (app->destroy) {
        app->destroy(app);
    }
    free(app);
}
