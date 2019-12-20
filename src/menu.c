#include "menu.h"

#include <stdlib.h>
#include <string.h>

#include "asset_ids.h"

menu_item_t*
make_menu_item(const char* label)
{
    menu_item_t* item = malloc(sizeof(menu_item_t));
    memset(item, 0, sizeof(menu_item_t));
    item->label = strdup(label);
    return item;
}

menu_item_t*
make_static_menu_item(const char* label)
{
    menu_item_t* item = make_menu_item(label);
    item->type = MENU_ITEM_TYPE_STATIC;
    return item;
}

menu_item_t*
make_submenu_menu_item(const char* label, menu_t* menu)
{
    menu_item_t* item = make_menu_item(label);
    item->type = MENU_ITEM_TYPE_MENU;
    item->data = menu;
    return item;
}

menu_item_t*
make_action_menu_item(const char* label, int action)
{
    return make_action_menu_item_ex(label, action, NULL);
}

menu_item_t*
make_action_menu_item_ex(const char* label, int action, void* param)
{
    menu_action_t* action_data = malloc(sizeof(menu_action_t));
    action_data->action = action;
    action_data->param = param;

    menu_item_t* item = make_menu_item(label);
    item->type = MENU_ITEM_TYPE_ACTION;
    item->data = action_data;
    return item;
}

void
destroy_action(menu_action_t* action)
{
    if (!action) return;
    free(action->param);
    free(action);
}

void
destroy_menu_item(menu_item_t* item)
{
    switch (item->type) {
        case MENU_ITEM_TYPE_MENU:
            destroy_menu(item->data);
            break;
        case MENU_ITEM_TYPE_ACTION:
            destroy_action(item->data);
            break;
        default:
            free(item->data);
            break;
    }
    free(item);
}

menu_t*
make_menu()
{
    menu_t* menu = malloc(sizeof(menu_t));
    memset(menu, 0, sizeof(menu_t));
    menu->cap_items = 2;
    menu->items = malloc(sizeof(menu_item_t*) * menu->cap_items);
    menu->background_image_id = IMG_ASSET_UI_MENU_SCREEN;
    return menu;
}

void
add_menu_item(menu_t* menu, menu_item_t* item)
{
    if (menu->cap_items == menu->num_items) {
        menu->cap_items <<= 1;
        menu->items = realloc(menu->items, sizeof(menu_item_t*) * menu->cap_items);
    }
    menu->items[menu->num_items++] = item;
}

void
destroy_menu(menu_t* menu)
{
    for (size_t i = 0; i < menu->num_items; ++i) {
        destroy_menu_item(menu->items[i]);
    }
    free(menu->items);
    free(menu);
}
