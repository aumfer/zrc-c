#pragma once

#include <zrc.h>
#include <khash.h>
#include <guid.h>

KHASH_INIT(zhash, guid_t, char, 0, guid_hash_func, guid_eq_func)

typedef struct zrc_host {
	khash_t(zhash) entities;
} zrc_host_t;

void zrc_host_startup(zrc_host_t *, zrc_t *);
void zrc_host_shutdown(zrc_host_t *);

void zrc_host_tick(zrc_host_t *, zrc_t *);

id_t zrc_host_put(zrc_host_t *, guid_t);
id_t zrc_host_del(zrc_host_t *, guid_t);
id_t zrc_host_get(zrc_host_t *, guid_t);