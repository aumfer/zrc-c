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

ui_state_t ui_button(const ui_t *ui, int button) {
	ui_state_t state = UI_INVALID;

	if (ui->buttons[sapp_frame_count()&UI_MASK][button]) {
		if (ui->buttons[(sapp_frame_count()-1)&UI_MASK][button]) {
			state = UI_DOWN;
		} else {
			state = UI_PRESSED;
		}
	} else {
		if (ui->buttons[(sapp_frame_count() - 1)&UI_MASK][button]) {
			state = UI_RELEASED;
		} else {
			state = UI_UP;
		}
	}
	assert(state != UI_INVALID);
	return state;
}

ui_touchstate_t ui_touch(const ui_t *ui, int touch) {
	ui_touchstate_t state = {
		.point[0] = ui->touches[(sapp_frame_count())&UI_MASK][touch][0],
		.point[1] = ui->touches[(sapp_frame_count())&UI_MASK][touch][1]
	};
	return state;
}