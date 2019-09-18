#include <ui.h>
#include <assert.h>
#include <string.h>

void ui_event_cb(const sapp_event*e, void*userdata) {
	ui_t *ui = userdata;

	switch (e->type) {
	case SAPP_EVENTTYPE_KEY_DOWN:
		ui->buttons[e->frame_count&UI_MASK][e->key_code] = 1;
		break;
	case SAPP_EVENTTYPE_KEY_UP:
		ui->buttons[e->frame_count&UI_MASK][e->key_code] = 0;
		break;
	case SAPP_EVENTTYPE_MOUSE_DOWN:
		ui->buttons[e->frame_count&UI_MASK][e->mouse_button] = 1;
		break;
	case SAPP_EVENTTYPE_MOUSE_UP:
		ui->buttons[e->frame_count&UI_MASK][e->mouse_button] = 0;
		break;
	case SAPP_EVENTTYPE_MOUSE_MOVE:
		ui->touches[e->frame_count&UI_MASK][UI_TOUCH_POINTER][0] = e->mouse_x;
		ui->touches[e->frame_count&UI_MASK][UI_TOUCH_POINTER][1] = sapp_height() - e->mouse_y;
		break;
	case SAPP_EVENTTYPE_MOUSE_SCROLL:
		ui->touches[e->frame_count&UI_MASK][UI_TOUCH_SCROLL][0] += e->scroll_x;
		ui->touches[e->frame_count&UI_MASK][UI_TOUCH_SCROLL][1] += e->scroll_y;
		break;
	}
}

void ui_frame(ui_t *ui) {
	uint64_t prev = (sapp_frame_count() + 0)&UI_MASK;
	uint64_t next = (sapp_frame_count() + 1)&UI_MASK;
	memcpy(ui->buttons[next], ui->buttons[prev], sizeof(ui->buttons[next]));
	memcpy(ui->touches[next], ui->touches[prev], sizeof(ui->touches[next]));
}

ui_buttonstate_t ui_button(const ui_t *ui, ui_button_t button) {
	ui_buttonstate_t state = BUTTON_INVALID;

	if (ui->buttons[sapp_frame_count()&UI_MASK][button]) {
		if (ui->buttons[(sapp_frame_count()-1)&UI_MASK][button]) {
			state = BUTTON_DOWN;
		} else {
			state = BUTTON_PRESSED;
		}
	} else {
		if (ui->buttons[(sapp_frame_count() - 1)&UI_MASK][button]) {
			state = BUTTON_RELEASED;
		} else {
			state = BUTTON_UP;
		}
	}
	assert(state != BUTTON_INVALID);
	return state;
}

ui_touchpoint_t ui_touch(const ui_t *ui, ui_touch_t touch) {
	ui_touchpoint_t state = {
		.x = ui->touches[(sapp_frame_count())&UI_MASK][touch][0],
		.y = ui->touches[(sapp_frame_count())&UI_MASK][touch][1]
	};
	return state;
}