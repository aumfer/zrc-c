#include <guid.h>
#include <Windows.h>

const guid_t GUID_EMPTY = { 0 };

guid_t guid_create(void) {
	guid_t id;
	guid_init(&id);
	return id;
}
void guid_init(guid_t *id) {
	GUID *pguid = (GUID *)id;
	CoCreateGuid(pguid);
}
