// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-FileCopyrightText: 2009-2021 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

/**
 * \file: contains all the variables linked to project files
 */

static bool core_config_add_prj_variables(RzCore *core) {
	RzConfig *cfg = core->config;
	return
		/* prj variables */
		rz_config_add_s(cfg, "prj.file", "Path of the currently opened project", "") &&
		rz_config_add_b(cfg, "prj.compress", "Compress the project file while saving", false);
}
