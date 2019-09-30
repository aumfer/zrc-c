#include <guid.h>

const guid_t GUID_EMPTY = { 0 };

guid_t guid_create(void) {
	guid_t id;
	guid_init(&id);
	return id;
}

#if WIN32
#include <Windows.h>

void guid_init(guid_t *id) {
	GUID *pguid = (GUID *)id;
	CoCreateGuid(pguid);
}
#else
#include <stdlib.h>

// todo https://www.ietf.org/rfc/rfc4122.txt
void guid_init(guid_t *id) {
	for (int i = 0; i < GUID_BYTES; ++i) {
		id->bytes[i] = rand() & 255;
	}
}
#endif