#pragma once

#include <sokol_app.h>

typedef enum ui_state {
	UI_INVALID = -1,
	UI_UP,
	UI_DOWN,
	UI_PRESSED,
	UI_RELEASED
} ui_state_t;

typedef struct ui_touchstate {
	float point[2];
	float move[2];
} ui_touchstate_t;

#define UI_FRAMES 64
#define UI_MASK (UI_FRAMES-1)

#define UI_BUTTON_COUNT SAPP_KEYCODE_MENU

typedef enum ui_touchpoint {
	UI_TOUCH_POINTER = SAPP_MAX_TOUCHPOINTS,
	UI_TOUCH_SCROLL,
	UI_TOUCH_COUNT
} ui_touchpoint_t;

typedef struct ui {
	int buttons[UI_FRAMES][UI_BUTTON_COUNT];
	float touches[UI_FRAMES][UI_TOUCH_COUNT][2];
} ui_t;

void ui_event_cb(const sapp_event*, void*);
void ui_frame(ui_t *);
ui_state_t ui_button(const ui_t *, int button);
ui_touchstate_t ui_touch(const ui_t *, int touch);