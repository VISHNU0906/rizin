// SPDX-FileCopyrightText: 2006-2021 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

#include "config_internal.h"

static void config_hold_variable_add(RzConfigHold *hold, const char *name) {
	RzConfigNode *node = rz_config_node_get(hold->cfg, name);
	if (!node) {
		RZ_LOG_WARN("config-hold: failed to get node named '%s'\n", name);
		return;
	}

	ConfigValue cv = { 0 };

	cv.name = rz_str_dup(name);
	cv.flags = node->flags;
	if (node->flags & RZ_CONFIG_VAR_BOOL) {
		cv.value.boolean = rz_config_node_get_bool(hold->cfg, node);
	} else if (node->flags & RZ_CONFIG_VAR_STR) {
		const char *str = rz_config_node_get_string(hold->cfg, node);
		cv.value.string = rz_str_dup(str);
	} else {
		cv.value.integer = rz_config_node_get_integer(hold->cfg, node);
	}

	rz_vector_push(&hold->variables, &cv);
}

/**
 * \brief Save a copy of the current config options.
 *
 * Get the current values of the config and save them in the RzConfigHold object \p h .
 *
 * \param hold  Reference to RzConfigHold instance
 * \param ...   List of config variables to save, terminated by NULL.
 * \return      Returns true if at least one variable is correctly saved, false otherwise
 */
RZ_API bool rz_config_hold_variable(RZ_NONNULL RzConfigHold *hold, ...) {
	rz_return_val_if_fail(hold, false);

	va_list ap;
	const char *name;
	va_start(ap, hold);

	while ((name = va_arg(ap, const char *))) {
		config_hold_variable_add(hold, name);
	}

	va_end(ap);
	return rz_vector_len(&hold->variables) > 0;
}

static void config_value_free(void *e, void *user) {
	ConfigValue *cv = (ConfigValue *)e;
	if (cv->flags & RZ_CONFIG_VAR_STR) {
		free(cv->value.string);
	}
	free(cv->name);
}

/**
 * \brief Create an opaque object to save/restore some configuration options
 *
 * \param cfg RzConfig reference
 * \return RzConfigHold allocated object
 */
RZ_API RzConfigHold *rz_config_hold_new(RZ_NONNULL RzConfig *cfg) {
	rz_return_val_if_fail(cfg, NULL);

	RzConfigHold *hold = RZ_NEW0(RzConfigHold);
	if (!hold) {
		return false;
	}
	hold->cfg = cfg;
	rz_vector_init(&hold->variables, sizeof(ConfigValue), config_value_free, NULL);
	return hold;
}

/**
 * \brief Create an opaque object to save/restore some configuration options and saves the given variables.
 *
 * \param cfg RzConfig reference
 * \param ...   List of config variables to save, terminated by NULL.
 * \return RzConfigHold allocated object
 */
RZ_API RzConfigHold *rz_config_hold_new2(RZ_NONNULL RzConfig *cfg, ...) {
	rz_return_val_if_fail(cfg, NULL);
	RzConfigHold *hold = rz_config_hold_new(cfg);
	if (!hold) {
		return NULL;
	}

	va_list ap;
	const char *name;
	va_start(ap, cfg);

	while ((name = va_arg(ap, const char *))) {
		config_hold_variable_add(hold, name);
	}

	va_end(ap);

	if (rz_vector_len(&hold->variables) < 1) {
		rz_config_hold_free(hold);
		return NULL;
	}

	return hold;
}

/**
 * \brief Restore whatever config options were previously saved in \p h
 *
 * \param h Reference to RzConfigHold
 */
RZ_API void rz_config_hold_restore(RZ_NULLABLE RzConfigHold *hold) {
	if (!hold) {
		return;
	}

	ConfigValue *cv;
	rz_vector_foreach (&hold->variables, cv) {
		if (!cv->name) {
			continue;
		}

		RzConfigNode *node = rz_config_node_get(hold->cfg, cv->name);
		if (!node) {
			RZ_LOG_WARN("config-hold: failed to get node named '%s'\n", cv->name);
			continue;
		}

		if (node->flags & RZ_CONFIG_VAR_BOOL) {
			rz_config_node_set_bool(hold->cfg, node, cv->value.boolean);
		} else if (node->flags & RZ_CONFIG_VAR_STR) {
			rz_config_node_set_string(hold->cfg, node, cv->value.string);
		} else {
			rz_config_node_set_integer(hold->cfg, node, cv->value.integer);
		}
	}
}

/**
 * \brief Free a RzConfigHold object \p h
 *
 * \param h Reference to RzConfigHold
 */
RZ_API void rz_config_hold_free(RZ_NULLABLE RzConfigHold *h) {
	if (!h) {
		return;
	}
	rz_vector_fini(&h->variables);
	free(h);
}

RZ_API void rz_config_hold_restore_and_free(RZ_NULLABLE RzConfigHold *hold) {
	if (!hold) {
		return;
	}
	rz_config_hold_restore(hold);
	rz_config_hold_free(hold);
}