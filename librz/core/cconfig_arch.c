// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-FileCopyrightText: 2009-2021 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

static RZ_OWN RzList /*<char *>*/ *core_config_string_list_dup(RZ_NULLABLE const RzList /*<char *>*/ *list) {
	const char *elem;
	const RzListIter *it;
	RzList *dup = rz_list_newf(free);
	if (!dup) {
		return NULL;
	}

	rz_list_foreach (list, it, elem) {
		if (RZ_STR_ISEMPTY(elem)) {
			continue;
		}

		char *copy = rz_str_dup(elem);
		if (!copy || !rz_list_append(dup, copy)) {
			free(copy);
		}
	}
	return dup;
}

/* analysis.cc GET */
static bool core_config_get_string_analysis_cc(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	*value = rz_analysis_cc_default(core->analysis);
	return true;
}

/* analysis.cc SET */
static bool core_config_set_string_analysis_cc(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;
	if (!strcmp(*value, "?")) {
		rz_core_types_calling_conventions_print(core, RZ_OUTPUT_MODE_STANDARD);
		return true;
	}

	rz_analysis_set_cc_default(core->analysis, *value);
	return true;
}

/* analysis.syscc GET */
static bool core_config_get_string_analysis_syscc(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	*value = rz_analysis_syscc_default(core->analysis);
	return true;
}

/* analysis.syscc SET */
static bool core_config_set_string_analysis_syscc(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;
	if (!strcmp(*value, "?")) {
		rz_core_types_calling_conventions_print(core, RZ_OUTPUT_MODE_STANDARD);
		return true;
	}

	rz_analysis_set_syscc_default(core->analysis, *value);
	return true;
}

/* analysis.roregs GET */
static bool core_config_get_list_analysis_roregs(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const RzList **value = (const RzList **)data;

	if (!(core && core->analysis && core->analysis->reg)) {
		return false;
	}

	RzList *roregs = core_config_string_list_dup(value);
	rz_reg_set_readonly(core->analysis->reg, roregs);
	return roregs != NULL;
}

/* analysis.roregs SET */
static bool core_config_set_list_analysis_roregs(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const RzList **value = (const RzList **)data;
	if (!(core && core->analysis && core->analysis->reg)) {
		return false;
	}

	*value = rz_reg_get_readonly(core->analysis->reg);
	return true;
}

/* analysis.gp GET */
static bool core_config_get_integer_analysis_gp(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	*value = core->analysis->gp;
	return true;
}

/* analysis.gp SET */
static bool core_config_set_integer_analysis_gp(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64*value = (const ut64*)data;

	core->analysis->gp = *value;
	return true;
}

static bool core_config_add_arch_variables(RzCore *core) {
	RzConfig *cfg = core->config;

	return
		/* analysis */
		rz_config_add_b(cfg, "analysis.detectwrites", "Automatically reanalyze function after a write", false) &&
		rz_config_add_s(cfg, "analysis.fcnprefix", "Prefix new function names with this", "fcn") &&

		rz_config_add_s_bind(cfg, "analysis.cc", "Specify default calling convention", core_config_get_string_analysis_cc, core_config_set_string_analysis_cc) &&
		rz_config_add_s_bind(cfg, "analysis.syscc", "Specify default syscall calling convention", core_config_get_string_analysis_syscc, core_config_set_string_analysis_syscc) &&
		rz_config_add_l_bind(cfg, "analysis.roregs", "Comma separated list of register names to be readonly", core_config_get_list_analysis_roregs, core_config_set_list_analysis_roregs);

		rz_config_add_i_bind(cfg, "analysis.gp", (RzConfigCallback)&cb_analysis_gp, "Set the value of the GP register (MIPS)");


		/* end */
		true;
}
