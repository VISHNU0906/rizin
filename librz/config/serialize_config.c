// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-FileCopyrightText: 2020 Florian Märkl <info@florianmaerkl.de>
// SPDX-License-Identifier: LGPL-3.0-only

#include <rz_util/rz_serialize.h>
#include "config_internal.h"

/*
 *
 * RzConfig isn't completely serialized, only the values.
 *
 * SDB Format:
 *
 * /
 *   <name>=<value>
 *   ...
 *
 */

typedef struct load_config_ctx_t {
	RzConfig *config;
	HtSP *exclude;
} LoadConfigCtx;

static bool apply_config_from_sdb(void *user, const SdbKv *kv) {
	LoadConfigCtx *ctx = user;
	RzConfig *config = ctx->config;
	HtSP *exclude = ctx->exclude;

	const char *name = sdbkv_key(kv);
	if (exclude && ht_sp_find_kv(exclude, name, NULL)) {
		return true;
	}

	RzConfigNode *node = rz_config_node_get(config, name);
	if (!node) {
		RZ_LOG_WARN("config-sdb: failed to find variable named '%s'\n", name);
		return true;
	}

	const char *value = sdbkv_value(kv);
	if (node->flags & RZ_CONFIG_VAR_BOOL) {
		rz_config_node_set_bool(config, node, rz_str_is_true(value));
	} else if (node->flags & RZ_CONFIG_VAR_STR) {
		rz_config_node_set_string(config, node, value);
	} else if (node->flags & RZ_CONFIG_VAR_INT) {
		ut64 uvalue = rz_num_get(NULL, value);
		rz_config_node_set_integer(config, node, uvalue);
	} else {
		RzList *list = rz_str_split_duplist(value, ",", false);
		rz_config_node_set_list(config, node, list);
		rz_list_free(list);
	}
	return true;
}

/**
 * \param
 */
/**
 * \brief      Unserializes an sdb file into a given config.
 *
 * \param      db      The database to unserialize
 * \param      config  The RzConfig to write to
 * \param      exclude Null-terminated array of keys to not load from the sdb.
 * \param      res     The results to write to
 */
RZ_API bool rz_serialize_config_load(RZ_NONNULL Sdb *db, RZ_NONNULL RzConfig *cfg,
	RZ_NULLABLE const char *const *exclude, RZ_NULLABLE RzSerializeResultInfo *res) {
	rz_return_val_if_fail(cfg && db, false);

	LoadConfigCtx ctx = { cfg, NULL };
	if (exclude) {
		ctx.exclude = ht_sp_new(HT_STR_DUP, NULL, NULL);
		if (!ctx.exclude) {
			return false;
		}
		for (; *exclude; exclude++) {
			ht_sp_insert(ctx.exclude, *exclude, NULL);
		}
	}

	sdb_foreach(db, apply_config_from_sdb, &ctx);

	ht_sp_free(ctx.exclude);
	return true;
}

/**
 * \brief      Serializes a given config into an sdb file.
 *
 * \param      db      The database to write to
 * \param      config  The RzConfig to serialize
 */
RZ_API void rz_serialize_config_save(RZ_NONNULL Sdb *db, RZ_NONNULL RzConfig *cfg) {
	rz_return_if_fail(cfg && db);

	char temp[256] = { 0 };
	RzListIter *iter;
	RzConfigNode *node;
	rz_list_foreach (cfg->nodes, iter, node) {
		if (node->flags & RZ_CONFIG_VAR_BOOL) {
			bool boolean = rz_config_node_get_bool(cfg, node);
			sdb_set(db, node->name, rz_str_bool(boolean));
		} else if (node->flags & RZ_CONFIG_VAR_STR) {
			const char *str = rz_config_node_get_string(cfg, node);
			sdb_set(db, node->name, str);
		} else if (node->flags & RZ_CONFIG_VAR_INT) {
			ut64 uvalue = rz_config_node_get_integer(cfg, node);
			rz_strf(temp, "0x%" PFMT64x, uvalue);
			sdb_set(db, node->name, temp);
		} else {
			const RzList *list = rz_config_node_get_list(cfg, node);
			char *str_list = rz_str_list_join(list, ",");
			sdb_set(db, node->name, str_list);
			free(str_list);
		}
	}
}
