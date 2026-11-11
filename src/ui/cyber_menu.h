#ifndef __CYBER_MENU_H__
#define __CYBER_MENU_H__

#include <lvgl.h>

/**
 * Setup the complete Cyber Menu on the given parent object.
 * This includes the top notification bar and the 12 modern icons.
 */
void setup_cyber_menu(lv_obj_t *menu);

extern volatile const char* pending_cyber_menu_name;

#endif
