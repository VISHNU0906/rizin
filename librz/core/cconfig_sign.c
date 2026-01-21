// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-FileCopyrightText: 2009-2021 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

/**
 * \file: contains all the variables linked to RzSign
 */

static bool core_config_add_sign_variables(RzCore *core) {
	RzConfig *cfg = core->config;

	return
		/* flirt variables */
		rz_config_add_s(cfg, "flirt.sig.library", "FLIRT library name for sig format", RZ_FLIRT_LIBRARY_NAME_DFL) &&
		rz_config_add_i(cfg, "flirt.sig.version", "FLIRT version for sig format", 10) &&
		rz_config_add_o(cfg, "flirt.sig.file", "FLIRT file list for sig format", "all", "msdos", "win", "os2", "netware", "unix", "other", "none") &&
		rz_config_add_o(cfg, "flirt.sig.os", "FLIRT operating system list for sig format", "all",
			"aixar", "aout", "ar", "bin", "coff", "dos:com", "dos:com:old", "dos:exe", "dos:exe:old", "dosdrv", "elf", "intelhex", "le", "loader",
			"lx", "moshex", "ne", "nlm", "omf", "omflib", "pe", "pilot", "srec", "w32run", "zip", "none") &&
		rz_config_add_b(cfg, "flirt.sig.deflate", "Enables/disables FLIRT zlib compression when creating a signature file (available only for .sig files)", false) &&
		rz_config_add_b(cfg, "flirt.ignore.unknown", "When enabled, on FLIRT creation it will ignore any function starting with `fcn.`", true) &&
		rz_config_add_b(cfg, "flirt.sigdb.load.system", "Load signatures from the system path", true) &&
		rz_config_add_b(cfg, "flirt.sigdb.load.extra", "Load signatures from the extra path", true) &&
		rz_config_add_b(cfg, "flirt.sigdb.load.home", "Load signatures from the home path", true) &&
		rz_config_add_i(cfg, "flirt.node.optimize", "FLIRT optimization option when creating a signature file (none: 0, normal: 1, smallest: 2)", RZ_FLIRT_NODE_OPTIMIZE_MAX) &&
		rz_config_add_s(cfg, "flirt.sigdb.path", "Additional user defined rizin sigdb location to load on the filesystem.", "");
}
