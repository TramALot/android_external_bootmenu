/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * Changes @ June 2012
 * Project lense (@whirleyes) - Fork Android recovery source code for spyder bootmenu
 *
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>

#include "common.h"
#include "cutils/properties.h"
#include "cutils/android_reboot.h"
#include "minui/minui.h"

//Menu related stuff

char* MENU_HEADERS[] = { " Project Lense Boot Menu v" EXPAND(RECOVERY_API_VERSION), "", " Please choose your boot preference", NULL };

char stock_name[40] = "Stock System";
char second_name[40] = "Second System";
char recovery_name[40] = "Custom System";
char* MENU_ITEMS[] = { stock_name, second_name, recovery_name, NULL };

extern UIParameters ui_parameters;

void configuration(const char* file) {
	//default value
	boot_default = 0;
	touch_y_sen = 70;
	touch_x_sen = 360;
	touch_select = 0;
	int brightness = 100;
	int keypad_light = 1;
	wait_timeout = 10;
	char theme[40] = "default";

	FILE* fp = fopen(file, "r");
	if (fp != NULL) {
		char item[40];
		char value[40];
		static char buffer[2048];
		while(fgets(buffer, sizeof(buffer), fp) != (char *)NULL) {
			if(buffer[0] == '#') continue;
			if(sscanf(buffer, "%39[^=]=%39[^\n]", item, value) == 2) {
				if(!strcmp(item, "default")) {
					if(!strcmp(value, "stock")) boot_default = 0;
					else if(!strcmp(value, "second")) boot_default = 1;
					else if(!strcmp(value, "recovery")) boot_default = 2;
				} else if(!strcmp(item, "stock_name")) {
					strncpy(stock_name, value, 32);
				} else if(!strcmp(item, "second_name")) {
					strncpy(second_name, value, 32);
				} else if(!strcmp(item, "recovery_name")) {
					strncpy(recovery_name, value, 32);
				} else if(!strcmp(item, "brightness")) {
					brightness = atoi(value);
				} else if(!strcmp(item, "keypad_light")) {
					keypad_light = atoi(value);
				} else if(!strcmp(item, "timeout")) {
					wait_timeout = atoi(value);
				} else if(!strcmp(item, "theme")) {
					strncpy(theme, value, 39);
				} else if(!strcmp(item, "touch_select")) {
					touch_select = atoi(value);
				} else if(!strcmp(item, "touch_x")) {
					touch_x_sen = atoi(value);
				} else if(!strcmp(item, "touch_y")) {
					touch_y_sen = atoi(value);
				}
			}
		}
		fclose(fp);
	}
	
	sprintf(RES_LOC,"/preinstall/bootmenu/themes/%s/%%s.png",theme);
	led("button-backlight", keypad_light);
	amoled(brightness);
}

int device_toggle_display(volatile char* key_pressed, int key_code) {
    return key_code == KEY_BACK;
}

int device_reboot_now(volatile char* key_pressed, int key_code) {
    return 0;
}

int device_handle_key(int key_code, int visible) {
    if (visible) {
        switch (key_code) {
            case KEY_MENU:
            case KEY_VOLUMEDOWN:
                return HIGHLIGHT_DOWN;

            case KEY_HOME:
            case KEY_VOLUMEUP:
                return HIGHLIGHT_UP;

			case KEY_SEARCH:
            case KEY_END:
                return SELECT_ITEM;
        }
    }
    return NO_ACTION;
}

static int get_menu_selection(char** headers, char** items, int menu_only, int initial_selection) {
    // throw away keys pressed previously, so user doesn't
    // accidentally trigger menu items.
    ui_clear_key_queue();
    ui_start_menu(headers, items, initial_selection);
    int selected = initial_selection;
    int chosen_item = -1;

	int presstext = 0;

    while (chosen_item < 0) {
        int key = ui_wait_key();
        int visible = ui_text_visible();

        if (key == -1) {
            ui_end_menu();
			ui_show_text(ENABLE);
			ui_print("Boot selection timeout...\n");
            return boot_default;
        }
		
		if (presstext == 0) {
			presstext++;
			if (touch_select == 1) {
				ui_print("       [-->] Slide to select\n\n");
			}
		}

        int action = device_handle_key(key, visible);

        if (action < 0) {
            switch (action) {
                case HIGHLIGHT_UP:
                    --selected;
                    selected = ui_menu_select(selected);
                    break;
                case HIGHLIGHT_DOWN:
                    ++selected;
                    selected = ui_menu_select(selected);
                    break;
                case SELECT_ITEM:
                    chosen_item = selected;
                    break;
                case NO_ACTION:
                    break;
            }
        } else if (!menu_only) {
            chosen_item = action;
        }
    }

    ui_end_menu();
    return chosen_item;
}

void run_action(int action) {
    switch (action) {
        case ITEM_STOCK:
			boot_stock();
			return;
   	    case ITEM_SECOND:
			boot_second();
			return;
 	    case ITEM_RECOVERY:
			boot_recovery();
			return;
   	}
}

int main(int argc, char** argv) {
	if (NULL != strstr(argv[0], "payload")) {

		//skip if booted by wall charger / 2nd-init
		char prop[30];
		property_get("ro.bootmode", prop, "unknown");
		if ( !strcmp(prop, "charger") || !strcmp(prop, "bootmenu")) {
			return EXIT_SUCCESS;
		}

		//read configuration file
		configuration("/preinstall/bootmenu/config/bootmenu.prop");

		struct stat modes;
		if (stat(RECOVERY_MODE_FILE, &modes) == 0) {
			remove(RECOVERY_MODE_FILE);
			run_action(ITEM_RECOVERY);
			led("button-backlight", DISABLE);
			return EXIT_SUCCESS;
		}

		if (stat(SECOND_MODE_FILE, &modes) == 0) {
			remove(SECOND_MODE_FILE);
			run_action(ITEM_SECOND);
			led("button-backlight", DISABLE);
			return EXIT_SUCCESS;
		}

		if (wait_timeout == 0) {
			run_action(boot_default);
			led("button-backlight", DISABLE);
			return EXIT_SUCCESS;
		}

		//Start ui
		ui_init();
		ui_set_background(BACKGROUND_ICON_INSTALLING);
		

		//action
		int action = get_menu_selection(MENU_HEADERS, (char**)MENU_ITEMS, 0, 0);
		ui_print("Booting %s...\n",MENU_ITEMS[action]);
		run_action(action);

		//finish
		led("button-backlight", DISABLE);
		ui_exit();
		return EXIT_SUCCESS;
	}

	printf("Bootmenu!\n");
	printf("Info     : Boot Menu\n");
	printf("Target   : Motorola Spyder (Kernel 3.0.8, ICS 4.0.4)\n");
	printf("Build    : v"EXPAND(RECOVERY_API_VERSION)" by whirleyes@github\n");
	printf("\n");
	return EXIT_SUCCESS;
}
