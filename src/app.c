#include "app.h"

app_t*
create_app()
{
    app_t *app = malloc(sizeof(app_t));
    memset(app, 0, sizeof(app_t));
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
