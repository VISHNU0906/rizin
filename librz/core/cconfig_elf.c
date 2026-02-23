// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-FileCopyrightText: 2009-2021 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

/**
 * \file: contains all the variables linked to ELF bin format
 */

static bool core_config_add_elf_variables(RzCore *core) {
	RzConfig *cfg = core->config;
	return
		/* elf variables */
		rz_config_add_b(cfg, "elf.checks.sections", "Check elf sections during loading", true) &&
		rz_config_add_b(cfg, "elf.checks.segments", "Check elf segments during loading", true) &&
		rz_config_add_b(cfg, "elf.load.sections", "Automatically load elf sections", true);
}
