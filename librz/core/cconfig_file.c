// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-FileCopyrightText: 2009-2021 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

/**
 * \file: contains all the variables linked to file.*
 */

/* file.info GET */
static bool core_config_get_bool_file_info(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	const RzBinInfo *info = core_config_get_bin_info(core);

	*value = info != NULL;
	return true;
}

/* file.type GET */
static bool core_config_get_string_file_type(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	const RzBinInfo *info = core_config_get_bin_info(core);
	if (!info) {
		*value = NULL;
		return true;
	}

	// shouldn't this evar be renamed?
	*value = info->rclass;
	return true;
}

/* file.path GET */
static bool core_config_get_string_file_path(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	const RzBinFile *bf = rz_bin_cur(core->bin);
	if (!bf) {
		*value = NULL;
		return true;
	}

	*value = bf->file;
	return true;
}

/* file.offset GET */
static bool core_config_get_integer_file_offset(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	const RzBinObject *obj = rz_bin_cur_object(core->bin);
	if (!obj) {
		*value = 0;
		return NULL;
	}

	*value = obj->opts.baseaddr;
	return true;
}

static bool core_config_add_file_variables(RzCore *core) {
	RzConfig *cfg = core->config;
	return
		/* file binds */
		rz_config_add_b_bind(cfg, "file.info", "Returns true if the RzBin info are available", core_config_get_bool_file_info, core_config_disabled_set) &&
		rz_config_add_i_bind(cfg, "file.offset", "Offset where the file is mapped at", core_config_get_integer_file_offset, core_config_disabled_set) &&
		rz_config_add_s_bind(cfg, "file.path", "Path of current file", core_config_get_string_file_path, core_config_disabled_set) &&
		rz_config_add_s_bind(cfg, "file.type", "Type of current file", core_config_get_string_file_type, core_config_disabled_set) &&

		/* file variables */
		rz_config_add_i(cfg, "file.loadalign", "Alignment of load addresses", 1024) &&
		rz_config_add_b(cfg, "file.openmany", "When true, allows to open all the inner files of a complex structure", true) &&

		/* the following data is read only */
		rz_config_set_readonly(cfg, "file.info", true) &&
		rz_config_set_readonly(cfg, "file.type", true) &&
		rz_config_set_readonly(cfg, "file.path", true) &&
		rz_config_set_readonly(cfg, "file.offset", true);
}
