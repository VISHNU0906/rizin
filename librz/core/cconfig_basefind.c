// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-FileCopyrightText: 2009-2021 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

/**
 * \file: contains all the variables linked to basefind
 */

static bool core_config_add_basefind_variables(RzCore *core) {
	RzConfig *cfg = core->config;
	return
		/* basefind variables */
		rz_config_add_b(cfg, "basefind.progress", "When true, shows basefind threads progress", false) &&
		rz_config_add_i(cfg, "basefind.search.start", "Basefind start search address", RZ_BASEFIND_BASE_MIN_ADDRESS) &&
		rz_config_add_i(cfg, "basefind.search.end", "Basefind end search address", RZ_BASEFIND_BASE_MAX_ADDRESS) &&
		rz_config_add_i(cfg, "basefind.alignment", "Basefind alignment in bytes", RZ_BASEFIND_BASE_ALIGNMENT) &&
		rz_config_add_i(cfg, "basefind.min.score", "Basefind min score value to consider it valid", RZ_BASEFIND_SCORE_MIN_VALUE) &&
		rz_config_add_i(cfg, "basefind.min.string", "Basefind min string size to find to consider it valid", RZ_BASEFIND_STRING_MIN_LENGTH) &&
		rz_config_add_i(cfg, "basefind.max.threads", "Basefind max threads number (when 0 uses all available cores)", RZ_THREAD_N_CORES_ALL_AVAILABLE);
}
