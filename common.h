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

#ifndef BOOTMENU_COMMON_H
#define BOOTMENU_COMMON_H

#include <stdio.h>
#include <cutils/klog.h>

#define ERROR(x...)   KLOG_ERROR("Bootmenu", x)
#define NOTICE(x...)  KLOG_NOTICE("Bootmenu", x)
#define INFO(x...)    KLOG_INFO("Bootmenu", x)
// Initialize the graphics system.
void ui_init();
void ui_exit();

// Use KEY_* codes from <linux/input.h> or KEY_DREAM_* from "minui/minui.h".
int ui_wait_key();            // waits for a key/button press, returns the code
int ui_key_pressed(int key);  // returns >0 if the code is currently pressed
int ui_text_visible();        // returns >0 if text log is currently visible
int ui_text_ever_visible();   // returns >0 if text log was ever visible
void ui_show_text(int visible);
void ui_clear_key_queue();

// Write a message to the on-screen log shown with Alt-L (also to stderr).
// The screen is small, and users may need to report these messages to support,
// so keep the output short and not too cryptic.
void ui_print(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

// Display some header text followed by a menu of items, which appears
// at the top of the screen (in place of any scrolling ui_print()
// output, if necessary).
void ui_start_menu(char** headers, char** items, int initial_selection);
// Set the menu highlight to the given index, and return it (capped to
// the range [0..numitems).
int ui_menu_select(int sel);
// End menu mode, resetting the text overlay so that ui_print()
// statements will be displayed.
void ui_end_menu();

// Set the icon (normally the only thing visible besides the progress bar).
enum {
  BACKGROUND_ICON_NONE,
  BACKGROUND_ICON_INSTALLING,
  NUM_BACKGROUND_ICONS
};
void ui_set_background(int icon);

// Show a progress bar and define the scope of the next operation:
//   portion - fraction of the progress bar the next operation will use
//   seconds - expected time interval (progress bar moves at this minimum rate)
void ui_show_progress(float portion, int seconds);
void ui_set_progress(float fraction);  // 0.0 - 1.0 within the defined scope

// Default allocation of progress bar segments to operations
static const int VERIFICATION_PROGRESS_TIME = 60;
static const float VERIFICATION_PROGRESS_FRACTION = 0.25;
static const float DEFAULT_FILES_PROGRESS_FRACTION = 0.4;
static const float DEFAULT_IMAGE_PROGRESS_FRACTION = 0.1;

// Hide and reset the progress bar.
void ui_reset_progress();

#define LOGE(...) ui_print("E:" __VA_ARGS__)
#define LOGW(...) fprintf(stdout, "W:" __VA_ARGS__)
#define LOGI(...) fprintf(stdout, "I:" __VA_ARGS__)

#if 0
#define LOGV(...) fprintf(stdout, "V:" __VA_ARGS__)
#define LOGD(...) fprintf(stdout, "D:" __VA_ARGS__)
#else
#define LOGV(...) do {} while (0)
#define LOGD(...) do {} while (0)
#endif

#define STRINGIFY(x) #x
#define EXPAND(x) STRINGIFY(x)

typedef struct {
	// number of frames per second to try to maintain when animating
    int update_fps;

    // number of frames in installing animation.  may be zero for a
    // static installation icon.
    int installing_frames;

    // the install icon is animated by drawing images containing the
    // changing part over the base icon.  These specify the
    // coordinates of the upper-left corner.
    int install_overlay_offset_x;
    int install_overlay_offset_y;

} UIParameters;

int device_toggle_display(volatile char* key_pressed, int key_code);
int device_reboot_now(volatile char* key_pressed, int key_code);
static void unlock();


//UI related
#define MAX_COLS 48
#define MAX_ROWS 16

#define CHAR_WIDTH 15
#define CHAR_HEIGHT 24
#define CHAR_SPACE2 48
#define CHAR_SPACE 130

int selected;
int wait_timeout;
int boot_default;
int stock_init, second_init;
int stock_adbd, second_adbd;
int recovery_mode;

enum {
  DISABLE,
  ENABLE
};

#define NO_ACTION           -1

#define HIGHLIGHT_UP        -2
#define HIGHLIGHT_DOWN      -3
#define SELECT_ITEM         -4

#define ITEM_STOCK           0
#define ITEM_SECOND          1
#define ITEM_RECOVERY        2

///var


///action
int exec_and_wait(char** argp);
void led(const char* color, int value);
void amoled(int value);
void vibrate(int value);
void boot_stock();
void boot_second();
void boot_recovery();
void boot(const char* script, int adbd, int init);

#define RECOVERY_MODE_FILE "/preinstall/.recovery_mode"
#define RECOVERY_MODE_TYPE "/preinstall/.recovery_second"
#define STOCK_MODE_FILE "/preinstall/.stock_mode"
#define SECOND_MODE_FILE "/preinstall/.second_mode"

#endif  // BOOTMENU_COMMON_H
