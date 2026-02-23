// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-FileCopyrightText: 2009-2021 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

/**
 * \file: contains all the variables linked to zoom.*
 */

static bool core_config_add_zoom_variables(RzCore *core) {
	RzConfig *cfg = core->config;
	return
		/* zoom variables */
		rz_config_add_i(cfg, "zoom.from", "Zoom start address", 0) &&
		rz_config_add_i(cfg, "zoom.maxsz", "Zoom max size of block", 512) &&
		rz_config_add_i(cfg, "zoom.to", "Zoom end address", 0) &&
		rz_config_add_s(cfg, "zoom.in", "Specify  boundaries for zoom", "io.map") &&

		/* zoom options */
		rz_config_set_options(cfg, "zoom.in", rz_core_get_supported_search_maps());
}
