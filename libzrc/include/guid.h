#pragma once

typedef struct guid {
	int a, b, c, d;
} guid_t;

extern const guid_t GUID_EMPTY;

#define guid_hash_func(id) ((id).a ^ (id).b ^ (id).c ^ (id).d)
#define guid_eq_func(l,r) ((l).a == (r).a && (l).b == (r).b && (l).c == (r).c && (l).d == (r).d)

guid_t guid_create(void);
void guid_init(guid_t *);