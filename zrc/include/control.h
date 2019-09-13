#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <camera.h>
#include <zrc.h>
#include <ui.h>

typedef struct control {
	id_t hover;
	id_t select;
} control_t;

void control_update(control_t *, const ui_t *ui, camera_t *, zrc_t *);

#endif