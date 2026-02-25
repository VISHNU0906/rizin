// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-FileCopyrightText: 2009-2021 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

/**
 * \file: contains all the single variables that does not fit any other groups.
 */

/* str.escbslash GET */
static bool core_config_get_bool_str_escbslash(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	*value = core->print->esc_bslash;
	return true;
}

/* str.escbslash SET */
static bool core_config_set_bool_str_escbslash(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	core->print->esc_bslash = *value;
	return true;
}

/* str.search.mode GET */
static bool core_config_get_string_str_search_mode(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	switch (core->bin->str_search_cfg.mode) {
	case RZ_BIN_STRING_SEARCH_MODE_AUTO:
		*value = "auto";
		return true;
	case RZ_BIN_STRING_SEARCH_MODE_READ_ONLY_SECTIONS:
		*value = "rosections";
		return true;
	case RZ_BIN_STRING_SEARCH_MODE_RAW_BINARY:
		*value = "raw";
		return true;
	default:
		return false;
	}
}

/* str.search.mode SET */
static bool core_config_set_string_str_search_mode(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	if (RZ_STR_EQ("auto", *value)) {
		core->bin->str_search_cfg.mode = RZ_BIN_STRING_SEARCH_MODE_AUTO;
	} else if (RZ_STR_EQ("rosections", *value)) {
		core->bin->str_search_cfg.mode = RZ_BIN_STRING_SEARCH_MODE_READ_ONLY_SECTIONS;
	} else if (RZ_STR_EQ("raw", *value)) {
		core->bin->str_search_cfg.mode = RZ_BIN_STRING_SEARCH_MODE_RAW_BINARY;
	} else {
		RZ_LOG_ERROR("Invalid value for str.search.mode (%s).\n", *value);
		return false;
	}

	check_reload_bin_str_search(core);
	return true;
}

/* str.encoding GET */
static bool core_config_get_string_str_encoding(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	*value = rz_str_enc_as_string(core->bin->str_search_cfg.string_encoding);
	return true;
}

/* str.encoding SET */
static bool core_config_set_string_str_encoding(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	core->bin->str_search_cfg.string_encoding = rz_str_enc_string_as_type(*value);
	check_reload_bin_str_search(core);
	return true;
}

static bool core_config_add_various_variables(RzCore *core) {
	RzConfig *cfg = core->config;
	return
		/* hud variables */
		rz_config_add_s(cfg, "hud.path", "Set a custom path for the HUD file", "") &&
		/* magic variables */
		rz_config_add_i(cfg, "magic.depth", "Recursivity depth in magic description strings", 100) &&
		/* rap variables */
		rz_config_add_b(cfg, "rap.loop", "Run rap as a forever-listening daemon (=:9090)", true) &&
		/* str variables */
		rz_config_add_b(cfg, "str.search.reload", "When enabled, any change to any option `str.search.*` will reload the bin strings.", true) &&
		/* str binds */
		rz_config_add_b_bind(cfg, "str.escbslash", "Escape the backslash.", core_config_get_bool_str_escbslash, core_config_set_bool_str_escbslash) &&
		rz_config_add_s_bind(cfg, "str.search.mode", "String search mode which can override how strings are found (auto, rosections or raw)", core_config_get_string_str_search_mode, core_config_set_string_str_search_mode) &&
		rz_config_add_s_bind(cfg, "str.encoding", "The default string encoding type (when set to guess, it is automatically guessed).", core_config_get_string_str_encoding, core_config_set_string_str_encoding) &&
		/* str options */
		rz_config_set_options(cfg, "str.encoding", rz_str_enc_string_as_list()) &&
		rz_config_set_options2(cfg, "str.search.mode", "auto", "rosections", "raw", NULL) &&
		/* tcp */
		rz_config_add_b(cfg, "tcp.islocal", "Bind a loopback for tcp command server", false);
}
