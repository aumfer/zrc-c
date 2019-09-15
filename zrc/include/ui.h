#pragma once

#include <sokol_app.h>

typedef enum ui_buttonstate {
	BUTTON_INVALID = -1,
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_PRESSED,
	BUTTON_RELEASED
} ui_buttonstate_t;

typedef struct ui_touchpoint {
	float x, y;
} ui_touchpoint_t;

#define UI_FRAMES 64
#define UI_MASK (UI_FRAMES-1)

typedef enum ui_button {
	UI_BUTTON_COUNT = SAPP_KEYCODE_MENU
} ui_button_t;

typedef enum ui_touch {
	UI_TOUCH_POINTER = SAPP_MAX_TOUCHPOINTS,
	UI_TOUCH_SCROLL,
	UI_TOUCH_COUNT
} ui_touch_t;

typedef struct ui {
	int buttons[UI_FRAMES][UI_BUTTON_COUNT];
	float touches[UI_FRAMES][UI_TOUCH_COUNT][2];
} ui_t;

void ui_event_cb(const sapp_event*, void*);
void ui_frame(ui_t *);
ui_buttonstate_t ui_button(const ui_t *, ui_button_t button);
ui_touchpoint_t ui_touch(const ui_t *, ui_touch_t touch);