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
	} value; ///< Variable held value unless setter/getter are defined.

	RzConfigCallback getter; ///< Gets the value via the callback (`data` can be `const char **` or `ut64 *` or `bool *`)
	RzConfigCallback setter; ///< Sets the value via the callback (`data` can be `const char **` or `ut64 *` or `bool *`)
	RzConfigCallback setopt; ///< Sets the options via the callback (`data` is always RzList <char *> *` which can be modified)
} RzConfigNode;

struct rz_config_t {
	bool lock; ///< When true forbids the add/remove of variables
	void *user; ///< Pointer passed to the callbacks (getter, setter, setopt)
	RzList /*<RzConfigNode *>*/ *nodes; ///< List of stored variables.
	HtSP *ht; ///< HashMap for fast access to the nodes.
};

typedef struct config_value_t {
	ut32 flags;
	char *name;
	union {
		char *string;
		ut64 integer;
		bool boolean;
	} value;
} ConfigValue;

struct rz_config_hold_t {
	RzConfig *cfg;
	RzVector /* ConfigValue */ variables;
};

static inline RzConfigNode *rz_config_node_get(RzConfig *cfg, const char *name) {
	return ht_sp_find(cfg->ht, name, NULL);
}

RZ_IPI bool rz_config_node_set_boolean(RzConfig* cfg, RzConfigNode *node, bool value);
RZ_IPI bool rz_config_node_set_integer(RzConfig* cfg, RzConfigNode *node, ut64 value);
RZ_IPI bool rz_config_node_set_string(RzConfig* cfg, RzConfigNode *node, const char *value);

RZ_IPI RZ_BORROW const char *rz_config_node_get_string(RzConfig *cfg, const RzConfigNode *node);
RZ_IPI ut64 rz_config_node_get_integer(RzConfig *cfg, const RzConfigNode *node);
RZ_IPI bool rz_config_node_get_boolean(RzConfig *cfg, const RzConfigNode *node);

#endif /* CONFIG_INTERNAL_H */
