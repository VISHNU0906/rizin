// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-FileCopyrightText: 2009-2021 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

/**
 * \file: contains all the generic config variables
 */

#if __WINDOWS__
#define CFG_DEFAULT_EDITOR "notepad"
#else
#define CFG_DEFAULT_EDITOR "vi"
#endif

/* cfg.bigendian GET */
static bool core_config_get_bool_cfg_bigendian(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAsm
	*value = core->rasm->big_endian;
	return true;
}

/* cfg.bigendian SET */
static bool core_config_set_bool_cfg_bigendian(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// Try to set endian based on preference, restrict by RzAsmPlugin
	rz_asm_set_big_endian(core->rasm, *value);
	// Set analysis endianness the same as asm
	rz_analysis_set_big_endian(core->analysis, *value);
	// While analysis sets endianess for TypesDB there might
	// be cases when it isn't availble for the chosen analysis
	// plugin but types and printing commands still need the
	// corresponding endianness. Thus we set these explicitly:
	rz_type_db_set_endian(core->analysis->typedb, *value);
	core->print->big_endian = *value;
	// the big endian should also be assigned to dbg->bp->endian
	if (core->dbg && core->dbg->bp) {
		core->dbg->bp->endian = *value;
	}
	return true;
}

/* cfg.debug GET */
static bool core_config_get_bool_cfg_debug(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	*value = core->bin->is_debugger;
	return true;
}

/* cfg.debug SET */
static bool core_config_set_bool_cfg_debug(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// this is a terrible way to enable the debugger.
	if (core->io) {
		rz_config_set_b(core->config, "io.va", !*value);
	}
	if (*value) {
		const char *dbg_backend = rz_config_get_s(core->config, "dbg.backend");
		if (RZ_STR_ISEMPTY(dbg_backend)) {
			RZ_LOG_ERROR("config: cannot set cfg.debug if dbg.backend is invalid\n");
			return false;
		}
		core->bin->is_debugger = true;
		rz_debug_use(core->dbg, dbg_backend);
		if (core->file) {
			rz_debug_select(core->dbg, rz_io_fd_get_pid(core->io, core->file->fd),
				rz_io_fd_get_tid(core->io, core->file->fd));
		}
	} else {
		rz_debug_use(core->dbg, NULL);
		core->bin->is_debugger = false;
	}
	return true;
}

/* cfg.fortunes GET */
static bool core_config_get_string_cfg_fortunes(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: implement get method in RzCore
	*value = core->show_fortunes;
	return true;
}

/* cfg.fortunes SET */
static bool core_config_set_string_cfg_fortunes(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: implement set method in RzCore
	core->show_fortunes = *value;
	return true;
}

/* cfg.fortunes.file GET */
static bool core_config_get_string_cfg_fortunes_file(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	// TODO: implement get method in RzCore
	*value = core->fortunes_file;
	return true;
}

/* cfg.fortunes.file SET */
static bool core_config_set_string_cfg_fortunes_file(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	// TODO: implement set method in RzCore
	free(core->fortunes_file);
	if (RZ_STR_ISEMPTY(*value)) {
		core->fortunes_file = rz_str_dup("tips");
	} else {
		core->fortunes_file = rz_str_dup(*value);
	}

	return true;
}

static bool core_config_add_cfg_variables(RzCore *core) {
	RzConfig *cfg = core->config;
	char *peditor = NULL;
	char *whoami = rz_sys_whoami();
	const char *preferred_editor = NULL;

	preferred_editor = peditor = rz_sys_getenv("EDITOR");
	if (RZ_STR_ISEMPTY(preferred_editor)) {
		preferred_editor = CFG_DEFAULT_EDITOR;
	}

	bool ok =
		/* cfg variables */
		rz_config_add_b(cfg, "cfg.fortunes.clippy", "Use 'clippy' instead of 'echo'", false) &&
		rz_config_add_b(cfg, "cfg.plugins", "Load plugins at startup", true) &&
		rz_config_add_b(cfg, "cfg.seek.silent", "When true, seek movements are not logged in seek history", false) &&
		rz_config_add_b(cfg, "cfg.wseek", "Seek after write", false) &&
		rz_config_add_i(cfg, "cfg.cpuaffinity", "Run on cpuid", 0) &&
		rz_config_add_i(cfg, "cfg.seek.histsize", "Maximum size of the seek history", 63) &&
		rz_config_add_s(cfg, "cfg.editor", "Select default editor program", preferred_editor) &&
		rz_config_add_s(cfg, "cfg.prefixdump", "Filename prefix for automated dumps", "dump") &&
		rz_config_add_s(cfg, "cfg.user", "Set current username/pid", whoami) &&

		/* cfg binds */
		rz_config_add_b_bind(cfg, "cfg.bigendian", "Use little (false) or big (true) endianness.", core_config_get_bool_cfg_bigendian, core_config_set_bool_cfg_bigendian) &&
		rz_config_add_b_bind(cfg, "cfg.debug", "Debugger mode", core_config_get_bool_cfg_debug, core_config_set_bool_cfg_debug) &&
		rz_config_add_b_bind(cfg, "cfg.fortunes", "If enabled show tips at start", core_config_get_string_cfg_fortunes, core_config_set_string_cfg_fortunes) &&
		rz_config_add_s_bind(cfg, "cfg.fortunes.file", "Type of fortunes to show (tips, fun)", core_config_get_string_cfg_fortunes_file, core_config_set_string_cfg_fortunes_file);

	free(whoami);
	free(peditor);
	return ok;
}
