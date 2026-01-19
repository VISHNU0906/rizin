// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-License-Identifier: LGPL-3.0-only

#include "config_internal.h"

static void config_node_free(RzConfigNode *node) {
	if (!node) {
		return;
	}

	free(node->name);
	free(node->desc);
	if (node->flags & CN_STR) {
		free(node->value.string);
	}
	rz_list_free(node->options);
	free(node);
}

static inline bool config_node_has_string(const RzConfigNode *node) {
	return node->flags & CN_STR && node->value.string;
}

static RzConfigNode *config_node_clone(const RzConfigNode *original) {
	RzConfigNode *copy = RZ_NEW0(RzConfigNode);
	if (!copy) {
		return NULL;
	}
	bool has_string = config_node_has_string(original);

	copy->name = rz_str_dup(original->name);
	copy->desc = rz_str_dup(original->desc);
	copy->flags = original->flags;
	if (has_string) {
		copy->value.string = rz_str_dup(original->value.string);
	}
	copy->setter = original->setter;
	copy->getter = original->getter;
	copy->options = rz_list_clone(original->options);

	if ((original->options && !copy->options) ||
		(original->name && !copy->name) ||
		(original->desc && !copy->desc) ||
		(has_string && !copy->value.string)) {
		config_node_free(copy);
		return NULL;
	}

	return copy;
}

/**
 * \brief      Frees RzConfig structure
 *
 * \param      cfg   The RzConfig to be freed
 */
RZ_API void rz_config_free(RZ_NULLABLE RzConfig *cfg) {
	if (!cfg) {
		return;
	}

	ht_sp_free(cfg->ht);
	rz_list_free(cfg->nodes);
	free(cfg);
}

/**
 * \brief      Allocates and initialize a new RzConfig
 *
 * \param      user  Pointer to the user data (not owned)
 *
 * \return     On success returns a valid pointer, otherwise NULL
 */
RZ_API RZ_OWN RzConfig *rz_config_new(RZ_NULLABLE void *user) {
	RzConfig *cfg = RZ_NEW0(RzConfig);
	if (!cfg) {
		return NULL;
	}
	cfg->ht = ht_sp_new(HT_STR_DUP, NULL, NULL);
	cfg->nodes = rz_list_newf((RzListFree)config_node_free);
	if (!cfg->nodes || !cfg->ht) {
		rz_config_free(cfg);
		return NULL;
	}
	cfg->user = user;
	cfg->lock = false;
	return cfg;
}

/**
 * \brief      Duplicates a given RzConfig
 *
 * \param      cfg   The RzConfig to clone
 *
 * \return     On success returns a valid pointer, otherwise NULL
 */
RZ_API RZ_OWN RzConfig *rz_config_clone(RZ_NONNULL RzConfig *cfg) {
	rz_return_val_if_fail(cfg, NULL);
	RzListIter *iter;
	RzConfigNode *node;
	RzConfig *c = rz_config_new(cfg->user);
	if (!c) {
		return NULL;
	}
	rz_list_foreach (cfg->nodes, iter, node) {
		RzConfigNode *nn = config_node_clone(node);
		if (!nn) {
			rz_config_free(c);
			return NULL;
		}
		ht_sp_insert(c->ht, node->name, nn);
		rz_list_append(c->nodes, nn);
	}
	c->lock = cfg->lock;
	return c;
}

/**
 * \brief      Enable or disable the lock of a given RzConfig which allows or disallows to add new variables
 *
 * \param      cfg          The RzConfig to lock/unlock
 * \param[in]  enable_lock  When true locks the RzConfig structure.
 */
RZ_API void rz_config_lock(RZ_NONNULL RzConfig *cfg, bool enable_lock) {
	rz_return_if_fail(cfg);
	cfg->lock = enable_lock;
}

/**
 * \brief      Setsa given variable to read only or writable mode.
 *
 * \param      cfg        The RzConfig to lock/unlock
 * \param      name       The name of the variable to set as RO/RW
 * \param[in]  read_only  When true sets the variable to read only mode otherwise allows writing.
 */
RZ_API void rz_config_set_readonly(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, bool read_only) {
	rz_return_if_fail(cfg && RZ_STR_ISNOTEMPTY(name));

	RzConfigNode *node = rz_config_node_get(cfg, name);
	if (!node) {
		RZ_LOG_ERROR("config: variable '%s' does not exists.\n", name);
		return;
	}

	if (!read_only) {
		node->flags |= CN_WRITABLE;
		return;
	}

	ut32 flags = ~CN_WRITABLE;
	node->flags &= flags;
}

static RzConfigNode *config_node_new_string(const char *name, const char *desc, const char *value) {
	RzConfigNode *node = RZ_NEW0(RzConfigNode);
	if (!node) {
		return NULL;
	}

	node->name = rz_str_dup(name);
	node->desc = rz_str_dup(desc);
	node->flags = CN_WRITABLE | CN_STR;
	// always duplicate a valid string.
	node->value.string = rz_str_dup(value ? value : "");

	if (!node->name || !node->value.string) {
		config_node_free(node);
		return NULL;
	}
	return node;
}

static RzConfigNode *config_node_new_integer(const char *name, const char *desc, ut64 value) {
	RzConfigNode *node = RZ_NEW0(RzConfigNode);
	if (!node) {
		return NULL;
	}

	node->name = rz_str_dup(name);
	node->desc = rz_str_dup(desc);
	node->flags = CN_WRITABLE | CN_INT;
	node->value.integer = value;
	if (!node->name) {
		config_node_free(node);
		return NULL;
	}
	return node;
}

static RzConfigNode *config_node_new_boolean(const char *name, const char *desc, bool value) {
	RzConfigNode *node = RZ_NEW0(RzConfigNode);
	if (!node) {
		return NULL;
	}

	node->name = rz_str_dup(name);
	node->desc = rz_str_dup(desc);
	node->flags = CN_WRITABLE | CN_BOOL;
	node->value.boolean = value;
	if (!node->name) {
		config_node_free(node);
		return NULL;
	}
	return node;
}

static RzConfigNode *config_node_new_bind(const char *name, const char *desc, ut32 flags, RzConfigCallback getter, RzConfigCallback setter) {
	RzConfigNode *node = RZ_NEW0(RzConfigNode);
	if (!node) {
		return NULL;
	}

	node->name = rz_str_dup(name);
	node->desc = rz_str_dup(desc);
	if (!node->name) {
		config_node_free(node);
		return NULL;
	}

	node->flags = flags;
	node->getter = getter;
	node->setter = setter;
	return node;
}

/**
 * \brief      Adds a new varibale of type string
 *
 * \param      cfg    The RzConfig to add the new variable to
 * \param[in]  name   The name of the variable
 * \param[in]  desc   The description of the variable
 * \param[in]  value  The value to set
 *
 * \return     On success returns true, otherwise false
 */
RZ_API bool rz_config_add_string(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, RZ_NULLABLE const char *value) {
	rz_return_val_if_fail(cfg && RZ_STR_ISNOTEMPTY(name), NULL);
	if (cfg->lock) {
		RZ_LOG_ERROR("config: cannot create variable '%s' when the config is locked.\n", name);
		return false;
	}

	if (rz_config_node_get(cfg, name)) {
		RZ_LOG_ERROR("config: variable '%s' already exists.\n", name);
		return false;
	}

	RzConfigNode *node = config_node_new_string(name, desc, value);
	if (!node) {
		RZ_LOG_ERROR("config: failed to allocate memory for variable '%s'.\n", name);
		return false;
	}

	ht_sp_insert(cfg->ht, node->name, node);
	rz_list_append(cfg->nodes, node);

	return true;
}

/**
 * \brief      Adds a new varibale of type integer
 *
 * \param      cfg    The RzConfig to add the new variable to
 * \param[in]  name   The name of the variable
 * \param[in]  desc   The description of the variable
 * \param[in]  value  The value to set
 *
 * \return     On success returns true, otherwise false
 */
RZ_API bool rz_config_add_integer(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, ut64 value) {
	rz_return_val_if_fail(cfg && RZ_STR_ISNOTEMPTY(name), NULL);
	if (cfg->lock) {
		RZ_LOG_ERROR("config: cannot create variable '%s' when the config is locked.\n", name);
		return false;
	}

	if (rz_config_node_get(cfg, name)) {
		RZ_LOG_ERROR("config: variable '%s' already exists.\n", name);
		return false;
	}

	RzConfigNode *node = config_node_new_integer(name, desc, value);
	if (!node) {
		RZ_LOG_ERROR("config: failed to allocate memory for variable '%s'.\n", name);
		return false;
	}

	ht_sp_insert(cfg->ht, node->name, node);
	rz_list_append(cfg->nodes, node);

	return true;
}

/**
 * \brief      Adds a new varibale of type boolean
 *
 * \param      cfg    The RzConfig to add the new variable to
 * \param[in]  name   The name of the variable
 * \param[in]  desc   The description of the variable
 * \param[in]  value  The value to set
 *
 * \return     On success returns true, otherwise false
 */
RZ_API bool rz_config_add_boolean(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, bool value) {
	rz_return_val_if_fail(cfg && RZ_STR_ISNOTEMPTY(name), NULL);
	if (cfg->lock) {
		RZ_LOG_ERROR("config: cannot create variable '%s' when the config is locked.\n", name);
		return false;
	}

	if (rz_config_node_get(cfg, name)) {
		RZ_LOG_ERROR("config: variable '%s' already exists.\n", name);
		return false;
	}

	RzConfigNode *node = config_node_new_boolean(name, desc, value);
	if (!node) {
		RZ_LOG_ERROR("config: failed to allocate memory for variable '%s'.\n", name);
		return false;
	}

	ht_sp_insert(cfg->ht, node->name, node);
	rz_list_append(cfg->nodes, node);

	return true;
}

/**
 * \brief      Adds a new varibale of type string, but with options
 *
 * \param      cfg     The RzConfig to add the new variable to
 * \param[in]  name    The name of the variable
 * \param[in]  desc    The description of the variable
 * \param[in]  vararg  The options to set
 *
 * \return     On success returns true, otherwise false
 */
RZ_API bool rz_config_add_options(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, ...) {
	rz_return_val_if_fail(cfg && RZ_STR_ISNOTEMPTY(name), NULL);
	va_list argp;

	if (cfg->lock) {
		RZ_LOG_ERROR("config: cannot create variable '%s' when the config is locked.\n", name);
		return false;
	}

	if (rz_config_node_get(cfg, name)) {
		RZ_LOG_ERROR("config: variable '%s' already exists.\n", name);
		return false;
	}

	RzConfigNode *node = config_node_new_string(name, desc, NULL);
	if (!node) {
		RZ_LOG_ERROR("config: failed to allocate memory for variable '%s'.\n", name);
		return false;
	}

	node->options = rz_list_newf(free);
	if (!node->options) {
		RZ_LOG_ERROR("config: failed to allocate options for variable '%s'.\n", name);
		config_node_free(node);
		return false;
	}

	va_start(argp, desc);
	const char *option = va_arg(argp, const char *);
	while (option) {
		char *copy = rz_str_dup(option);
		if (!copy || !rz_list_append(node->options, copy)) {
			config_node_free(node);
			return false;
		}
		option = va_arg(argp, const char *);
	}
	va_end(argp);

	if (rz_list_empty(node->options)) {
		RZ_LOG_ERROR("config: failed to set options for variable '%s'.\n", name);
		config_node_free(node);
		return false;
	}

	free(node->value.string);
	node->value.string = rz_str_dup(rz_list_first_val(node->options));
	if (!node->value.string) {
		config_node_free(node);
		return NULL;
	}

	ht_sp_insert(cfg->ht, node->name, node);
	rz_list_append(cfg->nodes, node);

	return true;
}

static bool config_add_bind(RzConfig *cfg, const char *name, const char *desc, RzConfigCallback getter, RzConfigCallback setter, ut32 flags) {
	rz_return_val_if_fail(cfg && RZ_STR_ISNOTEMPTY(name) && getter && setter, NULL);
	if (cfg->lock) {
		RZ_LOG_ERROR("config: cannot create variable '%s' when the config is locked.\n", name);
		return false;
	}

	if (rz_config_node_get(cfg, name)) {
		RZ_LOG_ERROR("config: variable '%s' already exists.\n", name);
		return false;
	}

	RzConfigNode *node = config_node_new_bind(name, desc, flags, getter, setter);
	if (!node) {
		RZ_LOG_ERROR("config: failed to allocate memory for variable '%s'.\n", name);
		return false;
	}

	ht_sp_insert(cfg->ht, node->name, node);
	rz_list_append(cfg->nodes, node);
	return true;
}

/**
 * \brief      Adds a new varibale of type string, but the data is accessed via getter & setter
 *
 * \param      cfg     The RzConfig to add the new variable to
 * \param[in]  name    The name of the variable
 * \param[in]  desc    The description of the variable
 * \param[in]  getter  The getter callback to get the value
 * \param[in]  setter  The setter callback to set the value
 *
 * \return     On success returns true, otherwise false
 */
RZ_API bool rz_config_add_string_bind(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, RZ_NONNULL RzConfigCallback getter, RZ_NONNULL RzConfigCallback setter) {
	return config_add_bind(cfg, name, desc, getter, setter, CN_WRITABLE | CN_STR);
}

/**
 * \brief      Adds a new varibale of type integer, but the data is accessed via getter & setter
 *
 * \param      cfg     The RzConfig to add the new variable to
 * \param[in]  name    The name of the variable
 * \param[in]  desc    The description of the variable
 * \param[in]  getter  The getter callback to get the value
 * \param[in]  setter  The setter callback to set the value
 *
 * \return     On success returns true, otherwise false
 */
RZ_API bool rz_config_add_integer_bind(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, RZ_NONNULL RzConfigCallback getter, RZ_NONNULL RzConfigCallback setter) {
	return config_add_bind(cfg, name, desc, getter, setter, CN_WRITABLE | CN_INT);
}

/**
 * \brief      Adds a new varibale of type boolean, but the data is accessed via getter & setter
 *
 * \param      cfg     The RzConfig to add the new variable to
 * \param[in]  name    The name of the variable
 * \param[in]  desc    The description of the variable
 * \param[in]  getter  The getter callback to get the value
 * \param[in]  setter  The setter callback to set the value
 *
 * \return     On success returns true, otherwise false
 */
RZ_API bool rz_config_add_boolean_bind(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, RZ_NONNULL RzConfigCallback getter, RZ_NONNULL RzConfigCallback setter) {
	return config_add_bind(cfg, name, desc, getter, setter, CN_WRITABLE | CN_BOOL);
}

static int config_compare_options(const void *_value, const void *_entry, void *user) {
	const char *value = (const char *)_value;
	const char *entry = (const char *)_entry;
	return rz_str_casecmp(value, rz_str_get(entry));
}

RZ_IPI bool rz_config_node_set_string(RzConfig *cfg, RzConfigNode *node, const char *value) {
	const char *safe_val = value ? value : "";

	if (node->setter) {
		return node->setter(cfg->user, (void *)&safe_val);
	} else if (!rz_list_empty(node->options)) {
		RzListIter *found = rz_list_find(node->options, safe_val, config_compare_options, NULL);
		if (!found) {
			RZ_LOG_ERROR("config: value '%s' is a valid option for variable '%s'.\n", safe_val, node->name);
			return false;
		}

		safe_val = rz_list_iter_get_data(found);
		if (!safe_val) {
			safe_val = "";
		}
	}

	free(node->value.string);
	node->value.string = rz_str_dup(safe_val);
	return node->value.string != NULL;
}

RZ_IPI bool rz_config_node_set_integer(RzConfig *cfg, RzConfigNode *node, ut64 value) {
	if (node->setter) {
		return node->setter(cfg->user, &value);
	}

	node->value.integer = value;
	return true;
}

RZ_IPI bool rz_config_node_set_boolean(RzConfig *cfg, RzConfigNode *node, bool value) {
	if (node->setter) {
		return node->setter(cfg->user, &value);
	}

	node->value.boolean = value;
	return true;
}

/**
 * \brief      Sets the value (type string) of a given variable of the config
 *
 * \param      cfg    The RzConfig to modify
 * \param[in]  name   The name of the variable
 * \param[in]  value  The value to update
 *
 * \return     On success returns true, otherwise false
 */
RZ_API bool rz_config_set_string(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *value) {
	RzConfigNode *node = rz_config_node_get(cfg, name);
	if (!node) {
		RZ_LOG_ERROR("config: variable '%s' does not exists.\n", name);
		return false;
	}

	return rz_config_node_set_string(cfg, node, value);
}

/**
 * \brief      Sets the value (type integer) of a given variable of the config
 *
 * \param      cfg    The RzConfig to modify
 * \param[in]  name   The name of the variable
 * \param[in]  value  The value to update
 *
 * \return     On success returns true, otherwise false
 */
RZ_API bool rz_config_set_integer(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, ut64 value) {
	RzConfigNode *node = rz_config_node_get(cfg, name);
	if (!node) {
		RZ_LOG_ERROR("config: variable '%s' does not exists.\n", name);
		return false;
	}

	return rz_config_node_set_integer(cfg, node, value);
}

/**
 * \brief      Sets the value (type boolean) of a given variable of the config
 *
 * \param      cfg    The RzConfig to modify
 * \param[in]  name   The name of the variable
 * \param[in]  value  The value to update
 *
 * \return     On success returns true, otherwise false
 */
RZ_API bool rz_config_set_boolean(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, bool value) {
	RzConfigNode *node = rz_config_node_get(cfg, name);
	if (!node) {
		RZ_LOG_ERROR("config: variable '%s' does not exists.\n", name);
		return false;
	}

	return rz_config_node_set_boolean(cfg, node, value);
}

RZ_IPI RZ_BORROW const char *rz_config_node_get_string(RzConfig *cfg, const RzConfigNode *node) {
	const char *safe_val = NULL;
	if (node->setter) {
		node->getter(cfg->user, (void *)&safe_val);
	} else if (node->value.string) {
		safe_val = node->value.string;
	}

	return safe_val ? safe_val : "";
}

RZ_IPI ut64 rz_config_node_get_integer(RzConfig *cfg, const RzConfigNode *node) {
	if (node->getter) {
		ut64 value = 0;
		node->getter(cfg->user, &value);
		return value;
	}

	return node->value.integer;
}

RZ_IPI bool rz_config_node_get_boolean(RzConfig *cfg, const RzConfigNode *node) {
	if (node->getter) {
		bool value = 0;
		node->getter(cfg->user, &value);
		return value;
	}

	return node->value.boolean;
}

/**
 * \brief      Gets the value (type string) of a given variable of the config
 *
 * \param      cfg    The RzConfig to modify
 * \param[in]  name   The name of the variable
 *
 * \return     On success returns true, otherwise false
 */
RZ_API RZ_BORROW const char *rz_config_get_string(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name) {
	RzConfigNode *node = rz_config_node_get(cfg, name);
	if (!node) {
		RZ_LOG_ERROR("config: variable '%s' does not exists.\n", name);
		return false;
	}

	return rz_config_node_get_string(cfg, node);
}

/**
 * \brief      Gets the value (type integer) of a given variable of the config
 *
 * \param      cfg    The RzConfig to modify
 * \param[in]  name   The name of the variable
 *
 * \return     On success returns true, otherwise false
 */
RZ_API ut64 rz_config_get_integer(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name) {
	RzConfigNode *node = rz_config_node_get(cfg, name);
	if (!node) {
		RZ_LOG_ERROR("config: variable '%s' does not exists.\n", name);
		return false;
	}

	return rz_config_node_get_integer(cfg, node);
}

/**
 * \brief      Gets the value (type boolean) of a given variable of the config
 *
 * \param      cfg    The RzConfig to modify
 * \param[in]  name   The name of the variable
 *
 * \return     On success returns true, otherwise false
 */
RZ_API bool rz_config_get_boolean(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name) {
	RzConfigNode *node = rz_config_node_get(cfg, name);
	if (!node) {
		RZ_LOG_ERROR("config: variable '%s' does not exists.\n", name);
		return false;
	}

	return rz_config_node_get_boolean(cfg, node);
}
