// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-License-Identifier: LGPL-3.0-only

#ifndef CONFIG_INTERNAL_H
#define CONFIG_INTERNAL_H

#include <rz_config.h>
#include <rz_util.h>
#include <rz_vector.h>

typedef struct rz_config_node_t {
	char *name; ///< Variable name
	char *desc; ///< Description of the variable
	RzList /*<char *>*/ *options; ///< Holds the string options
	ut32 flags; ///< define the type of the data via RzConfigNodeFlags

	union {
		char *string;
		ut64 integer;
		bool boolean;
		RzList /*<char *>*/ *list;
	} value;
	RzConfigCallbackGet getter; ///< Gets the value via the callback (`data` can be `const char **` or `ut64 *` or `bool *`)
	RzConfigCallbackSet setter; ///< Sets the value via the callback (`data` can be `const char **` or `ut64 *` or `bool *`)
	RzConfigCallbackOpt getopt; ///< Sets the options via the callback (`data` is always RzList <char *> *` which can be modified)
} RzConfigNode;

struct rz_config_t {
	bool lock; ///< When true forbids the add/remove of variables
	void *user; ///< Pointer passed to the callbacks (getter, setter, getopt)
	RzList /*<RzConfigNode *>*/ *nodes; ///< List of stored variables.
	HtSP *ht; ///< HashMap for fast access to the nodes.
};

typedef struct config_value_t {
	ut32 flags; ///< Copy of the variable flags (type RzConfigNodeFlags)
	char *name; ///< Copy of the variable name
	union {
		char *string;
		ut64 integer;
		bool boolean;
		RzList /*<char *>*/ *list;
	} value; ///< Copy of the variable value
} ConfigValue;

struct rz_config_hold_t {
	RzConfig *cfg;
	RzVector /* ConfigValue */ variables;
};

static inline RzConfigNode *rz_config_node_get(const RzConfig *cfg, const char *name) {
	return ht_sp_find(cfg->ht, name, NULL);
}

RZ_IPI RZ_OWN RzList /*<char *>*/ *rz_config_string_list_dup(RZ_NULLABLE const RzList /*<char *>*/ *list);

RZ_IPI bool rz_config_node_set_bool(RzConfig *cfg, RzConfigNode *node, bool value);
RZ_IPI bool rz_config_node_set_integer(RzConfig *cfg, RzConfigNode *node, ut64 value);
RZ_IPI bool rz_config_node_set_string(RzConfig *cfg, RzConfigNode *node, const char *value);
RZ_IPI bool rz_config_node_set_list(RzConfig *cfg, RzConfigNode *node, RZ_NULLABLE const RzList /*<char *>*/ *value);

RZ_IPI const char *rz_config_node_get_string(const RzConfig *cfg, const RzConfigNode *node);
RZ_IPI ut64 rz_config_node_get_integer(const RzConfig *cfg, const RzConfigNode *node);
RZ_IPI bool rz_config_node_get_bool(const RzConfig *cfg, const RzConfigNode *node);
RZ_IPI const RzList /*<char *>*/ *rz_config_node_get_list(const RzConfig *cfg, const RzConfigNode *node);

#endif /* CONFIG_INTERNAL_H */
