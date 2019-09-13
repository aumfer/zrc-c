#include <timer.h>

void timer_create(timer_t *timer) {
	timer->elapsed = 0;
	timer->time = stm_now();
	timer->prev = timer->time;
	timer->dt = 0;
}
void timer_delete(timer_t *timer) {

}
void timer_update(timer_t *timer) {
	timer->prev = timer->time;
	timer->time = stm_now();
	timer->dt = stm_diff(timer->time, timer->prev);
	timer->elapsed += timer->dt;
}