#pragma once

#include <stdint.h>

#define GUID_BYTES 16

typedef union guid {
	struct {
		uint32_t a, b, c, d;
	};
	struct {
		uint8_t bytes[GUID_BYTES];
	};
} guid_t;

static_assert(sizeof(guid_t) == GUID_BYTES, "invalid guid");

extern const guid_t GUID_EMPTY;

#define guid_hash_func(id) ((id).a ^ (id).b ^ (id).c ^ (id).d)
#define guid_eq_func(l,r) ((l).a == (r).a && (l).b == (r).b && (l).c == (r).c && (l).d == (r).d)
#define guid_cmp_func(l,r) memcmp(&(l), &(r), sizeof(guid_t))

guid_t guid_create(void);
void guid_init(guid_t *);