// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-FileCopyrightText: 2009-2021 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

/**
 * \file: contains all the variables linked to key.*
 */

static bool core_config_add_key_variables(RzCore *core) {
	RzConfig *cfg = core->config;
	return
		/* key variables */
		rz_config_add_s(cfg, "key.s", "Override step into action", "") &&
		rz_config_add_s(cfg, "key.S", "Override step over action", "") &&
		rz_config_add_s(cfg, "key.f1", "Run this when F1 key is pressed in visual mode", "") &&
		rz_config_add_s(cfg, "key.f2", "Run this when F2 key is pressed in visual mode", "") &&
		rz_config_add_s(cfg, "key.f3", "Run this when F3 key is pressed in visual mode", "") &&
		rz_config_add_s(cfg, "key.f4", "Run this when F4 key is pressed in visual mode", "") &&
		rz_config_add_s(cfg, "key.f5", "Run this when F5 key is pressed in visual mode", "") &&
		rz_config_add_s(cfg, "key.f6", "Run this when F6 key is pressed in visual mode", "") &&
		rz_config_add_s(cfg, "key.f7", "Run this when F7 key is pressed in visual mode", "") &&
		rz_config_add_s(cfg, "key.f8", "Run this when F8 key is pressed in visual mode", "") &&
		rz_config_add_s(cfg, "key.f9", "Run this when F9 key is pressed in visual mode", "") &&
		rz_config_add_s(cfg, "key.f10", "Run this when F10 key is pressed in visual mode", "") &&
		rz_config_add_s(cfg, "key.f11", "Run this when F11 key is pressed in visual mode", "") &&
		rz_config_add_s(cfg, "key.f12", "Run this when F12 key is pressed in visual mode", "");
}
