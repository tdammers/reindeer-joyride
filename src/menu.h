#pragma once

#include <stdlib.h>

#define MENU_ITEM_TYPE_MENU 0
#define MENU_ITEM_TYPE_STATIC 1
#define MENU_ITEM_TYPE_ACTION 2

typedef struct menu_item_t {
    char* label;
    int type;
    void* data;
} menu_item_t;

typedef struct menu_t {
    int background_image_id;
    size_t selected_item;
    size_t num_items;
    size_t cap_items;
    menu_item_t** items;
} menu_t;

typedef struct menu_action_t {
    int action;
    void* param;
} menu_action_t;

menu_item_t*
make_static_menu_item(const char*);

menu_item_t*
make_submenu_menu_item(const char*, menu_t*);

menu_item_t*
make_action_menu_item(const char*, int);

menu_item_t*
make_action_menu_item_ex(const char*, int, void*);

void
destroy_menu_item(menu_item_t*);

menu_t*
make_menu();

void
add_menu_item(menu_t* menu, menu_item_t* item);

void
destroy_menu(menu_t*);
