// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-FileCopyrightText: 2009-2021 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

/**
 * \file: contains all the single variables that does not fit any other groups.
 */

static bool core_config_add_various_variables(RzCore *core) {
	RzConfig *cfg = core->config;
	return
		/* hud variables */
		rz_config_add_s(cfg, "hud.path", "Set a custom path for the HUD file", "") &&
		/* magic variables */
		rz_config_add_i(cfg, "magic.depth", "Recursivity depth in magic description strings", 100) &&
		/* rap variables */
		rz_config_add_b(cfg, "rap.loop", "Run rap as a forever-listening daemon (=:9090)", true) &&
		/* tcp */
		rz_config_add_b(cfg, "tcp.islocal", "Bind a loopback for tcp command server", false);
}
