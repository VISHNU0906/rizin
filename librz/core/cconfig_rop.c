// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-License-Identifier: LGPL-3.0-only

/**
 * \file: contains all the variables linked to ROP
 */

static bool core_config_add_rop_variables(RzCore *core) {
	RzConfig *cfg = core->config;
	return
		/* rop variables */
		rz_config_add_b(cfg, "rop.cache", "Cache rop gadget results(experimental)", false) &&
		rz_config_add_b(cfg, "rop.comments", "Display comments in rop search output", false) &&
		rz_config_add_b(cfg, "rop.conditional", "Include conditional jump, calls and returns in ropsearch", false) &&
		rz_config_add_b(cfg, "rop.subchains", "Display every length gadget from rop.len=X to 2 in /Rl", false) &&
		rz_config_add_i(cfg, "rop.len", "Maximum ROP gadget length", 5);
}
