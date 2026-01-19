// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-License-Identifier: LGPL-3.0-only

#include "config_internal.h"

static const char *config_node_get_type(const RzConfigNode *node) {
	if (node->flags & RZ_CONFIG_VAR_STR) {
		return "string";
	} else if (node->flags & RZ_CONFIG_VAR_INT) {
		return "integer";
	} else if (node->flags & RZ_CONFIG_VAR_BOOL) {
		return "boolean";
	}
	return "list";
}

static void config_node_free(RzConfigNode *node) {
	if (!node) {
		return;
	}

	free(node->name);
	free(node->desc);
	if (node->flags & RZ_CONFIG_VAR_STR) {
		free(node->value.string);
	} else if (node->flags & RZ_CONFIG_VAR_LIST) {
		rz_list_free(node->value.list);
	}
	rz_list_free(node->options);
	free(node);
}

static inline bool config_node_has_string(const RzConfigNode *node) {
	return node->flags & RZ_CONFIG_VAR_STR && node->value.string;
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
	copy->setopt = original->setopt;
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

RZ_IPI RZ_OWN RzList /*<char *>*/ *rz_config_string_list_dup(RZ_NULLABLE const RzList /*<char *>*/ *list) {
	const char *elem;
	const RzListIter *it;
	RzList *dup = rz_list_newf(free);
	if (!dup) {
		return NULL;
	}

	rz_list_foreach (list, it, elem) {
		if (RZ_STR_ISEMPTY(elem)) {
			continue;
		}

		char *copy = rz_str_dup(elem);
		if (!copy || !rz_list_append(dup, copy)) {
			free(copy);
		}
	}
	return dup;
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
 * \brief      Sets a given variable to read only or writable mode.
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
		node->flags |= RZ_CONFIG_VAR_WRITABLE;
		return;
	}

	ut32 flags = ~RZ_CONFIG_VAR_WRITABLE;
	node->flags &= flags;
}

/**
 * \brief      Iterates over all the variables.
 *
 * \param      cfg        The RzConfig to iterate
 * \param      iterator   The iterator to call
 * \param[in]  user       Additional data to pass to iterator.
 */
RZ_API void rz_config_iterate_vars(RZ_NONNULL const RzConfig *cfg, RZ_NONNULL RzConfigIterator iterator, RZ_NULLABLE void *user) {
	rz_return_if_fail(cfg && iterator);

	const RzListIter *iter;
	const RzConfigNode *node;
	rz_list_foreach (cfg->nodes, iter, node) {
		if (!iterator(node->name, node->desc, node->flags, user)) {
			return;
		}
	}
}

static RzConfigNode *config_node_new_string(const char *name, const char *desc, const char *value) {
	RzConfigNode *node = RZ_NEW0(RzConfigNode);
	if (!node) {
		return NULL;
	}

	node->name = rz_str_dup(name);
	node->desc = rz_str_dup(desc);
	node->flags = RZ_CONFIG_VAR_WRITABLE | RZ_CONFIG_VAR_STR;
	// always duplicate a valid string.
	node->value.string = rz_str_dup(value ? value : "");
	node->options = rz_list_newf(free);

	if (!node->name ||
		!node->value.string ||
		!node->options) {
		config_node_free(node);
		return NULL;
	}
	return node;
}

static RzConfigNode *config_node_new_list(const char *name, const char *desc, const char *value) {
	RzConfigNode *node = RZ_NEW0(RzConfigNode);
	if (!node) {
		return NULL;
	}

	node->name = rz_str_dup(name);
	node->desc = rz_str_dup(desc);
	node->flags = RZ_CONFIG_VAR_WRITABLE | RZ_CONFIG_VAR_LIST;
	// always duplicate a valid string.
	node->value.list = rz_list_newf(free);

	if (!node->name ||
		!node->value.list) {
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
	node->flags = RZ_CONFIG_VAR_WRITABLE | RZ_CONFIG_VAR_INT;
	node->value.integer = value;
	if (!node->name) {
		config_node_free(node);
		return NULL;
	}
	return node;
}

static RzConfigNode *config_node_new_bool(const char *name, const char *desc, bool value) {
	RzConfigNode *node = RZ_NEW0(RzConfigNode);
	if (!node) {
		return NULL;
	}

	node->name = rz_str_dup(name);
	node->desc = rz_str_dup(desc);
	node->flags = RZ_CONFIG_VAR_WRITABLE | RZ_CONFIG_VAR_BOOL;
	node->value.boolean = value;
	if (!node->name) {
		config_node_free(node);
		return NULL;
	}
	return node;
}

static RzConfigNode *config_node_new_bind(const char *name, const char *desc, ut32 flags, RzConfigCallbackGet getter, RzConfigCallbackSet setter, RzConfigCallbackOpt setopt) {
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
	node->setopt = setopt;

	return node;
}

/**
 * \brief      Adds a new variable of type string
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

	if (rz_str_is_bool(value)) {
		RZ_LOG_WARN("config: variable '%s' is boolean.\n", name);
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
 * \brief      Adds a new variable of type integer
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
 * \brief      Adds a new variable of type boolean
 *
 * \param      cfg    The RzConfig to add the new variable to
 * \param[in]  name   The name of the variable
 * \param[in]  desc   The description of the variable
 * \param[in]  value  The value to set
 *
 * \return     On success returns true, otherwise false
 */
RZ_API bool rz_config_add_bool(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, bool value) {
	rz_return_val_if_fail(cfg && RZ_STR_ISNOTEMPTY(name), NULL);
	if (cfg->lock) {
		RZ_LOG_ERROR("config: cannot create variable '%s' when the config is locked.\n", name);
		return false;
	}

	if (rz_config_node_get(cfg, name)) {
		RZ_LOG_ERROR("config: variable '%s' already exists.\n", name);
		return false;
	}

	RzConfigNode *node = config_node_new_bool(name, desc, value);
	if (!node) {
		RZ_LOG_ERROR("config: failed to allocate memory for variable '%s'.\n", name);
		return false;
	}

	ht_sp_insert(cfg->ht, node->name, node);
	rz_list_append(cfg->nodes, node);

	return true;
}

/**
 * \brief      Adds a new variable of type string, but with options
 *
 * \param      cfg     The RzConfig to add the new variable to
 * \param[in]  name    The name of the variable
 * \param[in]  desc    The description of the variable
 * \param[in]  vararg  The options to set
 *
 * \return     On success returns true, otherwise false
 */
RZ_API bool rz_config_add_list(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, ...) {
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

	RzConfigNode *node = config_node_new_list(name, desc, NULL);
	if (!node) {
		RZ_LOG_ERROR("config: failed to allocate memory for variable '%s'.\n", name);
		return false;
	}

	va_start(argp, desc);
	const char *option = va_arg(argp, const char *);
	while (option) {
		char *copy = rz_str_dup(option);
		if (!copy || !rz_list_append(node->value.list, copy)) {
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

/**
 * \brief      Adds a new variable of type string, but with options
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

static bool config_add_bind(RzConfig *cfg, const char *name, const char *desc, RzConfigCallbackGet getter, RzConfigCallbackSet setter, RzConfigCallbackOpt setopt, ut32 flags) {
	rz_return_val_if_fail(cfg && RZ_STR_ISNOTEMPTY(name) && getter && setter, NULL);
	if (cfg->lock) {
		RZ_LOG_ERROR("config: cannot create variable '%s' when the config is locked.\n", name);
		return false;
	}

	if (rz_config_node_get(cfg, name)) {
		RZ_LOG_ERROR("config: variable '%s' already exists.\n", name);
		return false;
	}

	RzConfigNode *node = config_node_new_bind(name, desc, flags, getter, setter, setopt);
	if (!node) {
		RZ_LOG_ERROR("config: failed to allocate memory for variable '%s'.\n", name);
		return false;
	}

	ht_sp_insert(cfg->ht, node->name, node);
	rz_list_append(cfg->nodes, node);
	return true;
}

/**
 * \brief      Adds a new variable of type string, but the data is accessed via getter & setter
 *
 * \param      cfg     The RzConfig to add the new variable to
 * \param[in]  name    The name of the variable
 * \param[in]  desc    The description of the variable
 * \param[in]  getter  The getter callback to get the value from the bind (data is `const char *`)
 * \param[in]  setter  The setter callback to set the value from the bind (data is `const char *`)
 *
 * \return     On success returns true, otherwise false
 */
RZ_API bool rz_config_add_string_bind(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, RZ_NONNULL RzConfigCallbackGet getter, RZ_NONNULL RzConfigCallbackSet setter) {
	return config_add_bind(cfg, name, desc, getter, setter, NULL, RZ_CONFIG_VAR_WRITABLE | RZ_CONFIG_VAR_STR);
}

/**
 * \brief      Adds a new variable of type integer, but the data is accessed via getter & setter
 *
 * \param      cfg     The RzConfig to add the new variable to
 * \param[in]  name    The name of the variable
 * \param[in]  desc    The description of the variable
 * \param[in]  setter  The setter callback to set the value from the bind (data is `ut64 *`)
 * \param[in]  getter  The getter callback to get the value from the bind (data is `ut64 *`)
 *
 * \return     On success returns true, otherwise false
 */
RZ_API bool rz_config_add_integer_bind(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, RZ_NONNULL RzConfigCallbackGet getter, RZ_NONNULL RzConfigCallbackSet setter) {
	return config_add_bind(cfg, name, desc, getter, setter, NULL, RZ_CONFIG_VAR_WRITABLE | RZ_CONFIG_VAR_INT);
}

/**
 * \brief      Adds a new variable of type boolean, but the data is accessed via getter & setter
 *
 * \param      cfg     The RzConfig to add the new variable to
 * \param[in]  name    The name of the variable
 * \param[in]  desc    The description of the variable
 * \param[in]  setter  The setter callback to set the value from the bind (data is `bool *`)
 * \param[in]  getter  The getter callback to get the value from the bind (data is `bool *`)
 *
 * \return     On success returns true, otherwise false
 */
RZ_API bool rz_config_add_bool_bind(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, RZ_NONNULL RzConfigCallbackGet getter, RZ_NONNULL RzConfigCallbackSet setter) {
	return config_add_bind(cfg, name, desc, getter, setter, NULL, RZ_CONFIG_VAR_WRITABLE | RZ_CONFIG_VAR_BOOL);
}

/**
 * \brief      Adds a new variable of type string list, but the data is accessed via getter & setter
 *
 * \param      cfg     The RzConfig to add the new variable to
 * \param[in]  name    The name of the variable
 * \param[in]  desc    The description of the variable
 * \param[in]  getter  The getter callback to get the value from the bind (data is `RzList <char *> *`)
 * \param[in]  setter  The setter callback to set the value from the bind (data is `RzList <char *> *`)
 *
 * \return     On success returns true, otherwise false
 */
RZ_API bool rz_config_add_list_bind(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, RZ_NONNULL RzConfigCallbackGet getter, RZ_NONNULL RzConfigCallbackSet setter) {
	return config_add_bind(cfg, name, desc, getter, setter, NULL, RZ_CONFIG_VAR_WRITABLE | RZ_CONFIG_VAR_LIST);
}

/**
 * \brief      Adds a new variable of type string, but the data is accessed via getter & setter
 *
 * \param      cfg     The RzConfig to add the new variable to
 * \param[in]  name    The name of the variable
 * \param[in]  desc    The description of the variable
 * \param[in]  getter  The getter callback to get the value from the bind (data is `const char *`)
 * \param[in]  setter  The setter callback to set the value from the bind (data is `const char *`)
 * \param[in]  setopt  The setopt callback to set the options from the bind (data is `RzList <char *> *`)
 *
 * \return     On success returns true, otherwise false
 */
RZ_API bool rz_config_add_options_bind(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, RZ_NONNULL RzConfigCallbackGet getter, RZ_NONNULL RzConfigCallbackSet setter, RZ_NONNULL RzConfigCallbackOpt setopt) {
	rz_return_val_if_fail(setopt, false);
	return config_add_bind(cfg, name, desc, getter, setter, setopt, RZ_CONFIG_VAR_WRITABLE | RZ_CONFIG_VAR_STR);
}

static int config_compare_options(const void *_value, const void *_entry, void *user) {
	const char *value = (const char *)_value;
	const char *entry = (const char *)_entry;
	return rz_str_casecmp(value, rz_str_get(entry));
}

RZ_IPI bool rz_config_node_set_string(RzConfig *cfg, RzConfigNode *node, RZ_NULLABLE const char *value) {
	if (!(node->flags & RZ_CONFIG_VAR_STR)) {
		RZ_LOG_ERROR("config: variable %s was accessed as string but is %s\n", node->name, config_node_get_type(node));
		return false;
	}

	const char *safe_val = value ? value : "";

	if (!rz_list_empty(node->options)) {
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

	if (node->setter) {
		if (!node->setter(cfg->user, (void *)&safe_val)) {
			return false;
		}
		if (!node->setopt) {
			return true;
		}
		return node->setopt(cfg->user, (void *)&node->options);
	}

	free(node->value.string);
	node->value.string = rz_str_dup(safe_val);
	return node->value.string != NULL;
}

RZ_IPI bool rz_config_node_set_integer(RzConfig *cfg, RzConfigNode *node, ut64 value) {
	if (!(node->flags & RZ_CONFIG_VAR_INT)) {
		RZ_LOG_ERROR("config: variable %s was accessed as integer but is %s\n", node->name, config_node_get_type(node));
		return false;
	}

	if (node->setter) {
		return node->setter(cfg->user, &value);
	}

	node->value.integer = value;
	return true;
}

RZ_IPI bool rz_config_node_set_bool(RzConfig *cfg, RzConfigNode *node, bool value) {
	if (!(node->flags & RZ_CONFIG_VAR_BOOL)) {
		RZ_LOG_ERROR("config: variable %s was accessed as boolean but is %s\n", node->name, config_node_get_type(node));
		return false;
	}

	if (node->setter) {
		return node->setter(cfg->user, &value);
	}

	node->value.boolean = value;
	return true;
}

RZ_IPI bool rz_config_node_set_list(RzConfig *cfg, RzConfigNode *node, RZ_NULLABLE const RzList /*<char *>*/ *value) {
	if (!(node->flags & RZ_CONFIG_VAR_LIST)) {
		RZ_LOG_ERROR("config: variable %s was accessed as string list but is %s\n", node->name, config_node_get_type(node));
		return false;
	}

	if (node->setter) {
		return node->setter(cfg->user, &value);
	}

	rz_list_free(node->value.list);
	node->value.list = rz_config_string_list_dup(value);
	return node->value.list != NULL;
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
	} else if (!(node->flags & RZ_CONFIG_VAR_WRITABLE)) {
		RZ_LOG_ERROR("config: variable %s is read only\n", node->name);
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
	} else if (!(node->flags & RZ_CONFIG_VAR_WRITABLE)) {
		RZ_LOG_ERROR("config: variable %s is read only\n", node->name);
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
RZ_API bool rz_config_set_bool(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, bool value) {
	RzConfigNode *node = rz_config_node_get(cfg, name);
	if (!node) {
		RZ_LOG_ERROR("config: variable '%s' does not exists.\n", name);
		return false;
	} else if (!(node->flags & RZ_CONFIG_VAR_WRITABLE)) {
		RZ_LOG_ERROR("config: variable %s is read only\n", node->name);
		return false;
	}

	return rz_config_node_set_bool(cfg, node, value);
}

/**
 * \brief      Sets the value (type string list) of a given variable of the config
 *
 * \param      cfg    The RzConfig to modify
 * \param[in]  name   The name of the variable
 * \param[in]  value  The value to update
 *
 * \return     On success returns true, otherwise false
 */
RZ_API bool rz_config_set_list(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const RzList /*<char *>*/ *value) {
	RzConfigNode *node = rz_config_node_get(cfg, name);
	if (!node) {
		RZ_LOG_ERROR("config: variable '%s' does not exists.\n", name);
		return false;
	} else if (!(node->flags & RZ_CONFIG_VAR_WRITABLE)) {
		RZ_LOG_ERROR("config: variable %s is read only\n", node->name);
		return false;
	}

	return rz_config_node_set_list(cfg, node, value);
}

/**
 * \brief      Toggles (i.e. inverts) the value of a boolean
 *
 * \param      cfg    The RzConfig to modify
 * \param[in]  name   The name of the variable to toggle
 *
 * \return     On success returns true, otherwise false
 */
RZ_API bool rz_config_toggle_bool(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name) {
	RzConfigNode *node = rz_config_node_get(cfg, name);
	if (!node) {
		RZ_LOG_ERROR("config: variable '%s' does not exists.\n", name);
		return false;
	} else if (!(node->flags & RZ_CONFIG_VAR_WRITABLE)) {
		RZ_LOG_ERROR("config: variable %s is read only\n", node->name);
		return false;
	} else if (!(node->flags & RZ_CONFIG_VAR_BOOL)) {
		RZ_LOG_ERROR("config: variable %s was accessed as boolean but is %s\n", node->name, config_node_get_type(node));
		return false;
	}

	bool value = rz_config_node_get_bool(cfg, node);
	return rz_config_node_set_bool(cfg, node, !value);
}

RZ_IPI RZ_BORROW const char *rz_config_node_get_string(RzConfig *cfg, const RzConfigNode *node) {
	if (!(node->flags & RZ_CONFIG_VAR_STR)) {
		RZ_LOG_ERROR("config: variable %s was accessed as string but is %s\n", node->name, config_node_get_type(node));
		return "";
	}

	const char *safe_val = NULL;
	if (node->setter) {
		node->getter(cfg->user, (void *)&safe_val);
	} else if (node->value.string) {
		safe_val = node->value.string;
	}

	return safe_val ? safe_val : "";
}

RZ_IPI ut64 rz_config_node_get_integer(RzConfig *cfg, const RzConfigNode *node) {
	if (!(node->flags & RZ_CONFIG_VAR_INT)) {
		RZ_LOG_ERROR("config: variable %s was accessed as integer but is %s\n", node->name, config_node_get_type(node));
		return 0;
	}

	if (node->getter) {
		ut64 value = 0;
		node->getter(cfg->user, &value);
		return value;
	}

	return node->value.integer;
}

RZ_IPI bool rz_config_node_get_bool(RzConfig *cfg, const RzConfigNode *node) {
	if (!(node->flags & RZ_CONFIG_VAR_BOOL)) {
		RZ_LOG_ERROR("config: variable %s was accessed as boolean but is %s\n", node->name, config_node_get_type(node));
		return 0;
	}

	if (node->getter) {
		bool value = 0;
		node->getter(cfg->user, &value);
		return value;
	}

	return node->value.boolean;
}

RZ_IPI RZ_BORROW const RzList /*<char *>*/ *rz_config_node_get_list(RzConfig *cfg, const RzConfigNode *node) {
	if (!(node->flags & RZ_CONFIG_VAR_BOOL)) {
		RZ_LOG_ERROR("config: variable %s was accessed as list but is %s\n", node->name, config_node_get_type(node));
		return NULL;
	}

	if (node->getter) {
		const RzList /*<char *>*/ *value = NULL;
		node->getter(cfg->user, &value);
		return value;
	}

	return node->value.list;
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
RZ_API bool rz_config_get_bool(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name) {
	RzConfigNode *node = rz_config_node_get(cfg, name);
	if (!node) {
		RZ_LOG_ERROR("config: variable '%s' does not exists.\n", name);
		return false;
	}

	return rz_config_node_get_bool(cfg, node);
}

/**
 * \brief      Gets the value (type string list) of a given variable of the config
 *
 * \param      cfg    The RzConfig to use
 * \param[in]  name   The name of the variable
 *
 * \return     On success returns a valid pointer, otherwise NULL
 */
RZ_API RZ_BORROW const RzList /*<char *>*/ *rz_config_get_list(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name) {
	RzConfigNode *node = rz_config_node_get(cfg, name);
	if (!node) {
		RZ_LOG_ERROR("config: variable '%s' does not exists.\n", name);
		return NULL;
	}

	return rz_config_node_get_list(cfg, node);
}

/**
 * \brief      Gets the options of a given variable of the config
 *
 * \param      cfg    The RzConfig to use
 * \param[in]  name   The name of the variable
 *
 * \return     On success returns a valid pointer, otherwise NULL
 */
RZ_API RZ_BORROW const RzList /*<char *>*/ *rz_config_get_options(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name) {
	RzConfigNode *node = rz_config_node_get(cfg, name);
	if (!node) {
		RZ_LOG_ERROR("config: variable '%s' does not exists.\n", name);
		return NULL;
	}

	return node->options;
}

/**
 * \brief      Gets the flags of a given variable of the config
 *
 * \param      cfg    The RzConfig to use
 * \param[in]  name   The name of the variable
 *
 * \return     On success returns a valid pointer, otherwise NULL
 */
RZ_API const ut32 rz_config_get_flags(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name) {
	RzConfigNode *node = rz_config_node_get(cfg, name);
	if (!node) {
		RZ_LOG_ERROR("config: variable '%s' does not exists.\n", name);
		return 0;
	}

	return node->flags;
}
