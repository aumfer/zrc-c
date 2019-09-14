#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <camera.h>
#include <zrc.h>
#include <ui.h>

#define CONTROL_BUTTON_FORWARD SAPP_KEYCODE_W
#define CONTROL_BUTTON_BACKWARD SAPP_KEYCODE_S
#define CONTROL_BUTTON_LEFT SAPP_KEYCODE_A
#define CONTROL_BUTTON_RIGHT SAPP_KEYCODE_D

#define CONTROL_BUTTON_CAST0 SAPP_MOUSEBUTTON_LEFT
#define CONTROL_BUTTON_CAST1 SAPP_MOUSEBUTTON_RIGHT

typedef struct control {
	id_t target;
	id_t unit;

	float ground[2];
} control_t;

void control_frame(control_t *, const ui_t *ui, camera_t *, zrc_t *);

#endif