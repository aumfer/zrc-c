#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <camera.h>
#include <zrc.h>
#include <ui.h>

#define CONTROL_BUTTON_FORWARD SAPP_KEYCODE_W
#define CONTROL_BUTTON_BACKWARD SAPP_KEYCODE_S
#define CONTROL_BUTTON_LEFT SAPP_KEYCODE_A
#define CONTROL_BUTTON_RIGHT SAPP_KEYCODE_D
#define CONTROL_BUTTON_STRAFE_LEFT SAPP_KEYCODE_Q
#define CONTROL_BUTTON_STRAFE_RIGHT SAPP_KEYCODE_E

#define CONTROL_BUTTON_CAST0 SAPP_MOUSEBUTTON_LEFT
#define CONTROL_BUTTON_CAST1 SAPP_MOUSEBUTTON_RIGHT
#define CONTROL_BUTTON_CAST2 SAPP_KEYCODE_1
#define CONTROL_BUTTON_CAST3 SAPP_KEYCODE_2
#define CONTROL_BUTTON_CAST4 SAPP_KEYCODE_3
#define CONTROL_BUTTON_CAST5 SAPP_KEYCODE_4
#define CONTROL_BUTTON_CAST6 SAPP_KEYCODE_R
#define CONTROL_BUTTON_CAST7 SAPP_KEYCODE_F

typedef struct control {
	id_t target;
	id_t unit;

	float ground[2];
} control_t;

void control_create(control_t *);
void control_delete(control_t *);

void control_frame(control_t *, const ui_t *ui, camera_t *, zrc_t *);

#endif