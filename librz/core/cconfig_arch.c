// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-FileCopyrightText: 2009-2021 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

/**
 * \file: contains all the variables linked to RzArch
 */

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
	if (CONFIG_IS_HELP(*value)) {
		rz_core_types_calling_conventions_print(core, RZ_OUTPUT_MODE_STANDARD);
		return true;
	} else if (*value && !rz_analysis_cc_exist(core->analysis, *value)) {
		RZ_LOG_ERROR("config: invalid analysis.cc value: %s\n", *value);
		return false;
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
	if (CONFIG_IS_HELP(*value)) {
		rz_core_types_calling_conventions_print(core, RZ_OUTPUT_MODE_STANDARD);
		return true;
	} else if (*value && !rz_analysis_cc_exist(core->analysis, *value)) {
		RZ_LOG_ERROR("config: invalid analysis.syscc value: %s\n", *value);
		return false;
	}

	rz_analysis_set_syscc_default(core->analysis, *value);
	return true;
}

/* analysis.roregs GET */
static bool core_config_get_list_analysis_roregs(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const RzList **value = (const RzList **)data;

	if (!(core && core->analysis && core->analysis->reg)) {
		return false;
	}

	RzList *roregs = core_config_string_list_dup(*value);
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

	// TODO: create get function in RzAnalysis
	*value = core->analysis->gp;
	return true;
}

/* analysis.gp SET */
static bool core_config_set_integer_analysis_gp(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->gp = *value;
	return true;
}

/* analysis.limits GET */
static bool core_config_get_bool_analysis_limits(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	*value = rz_analysis_are_limits_enabled(core->analysis);
	return true;
}

/* analysis.limits SET */
static bool core_config_set_bool_analysis_limits(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	rz_analysis_enable_limits(core->analysis, *value);
	return true;
}

/* analysis.limits.from GET */
static bool core_config_get_integer_analysis_limits_from(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *from = (ut64 *)data;

	rz_analysis_get_limits(core->analysis, from, NULL);
	return true;
}

/* analysis.limits.from SET */
static bool core_config_set_integer_analysis_limits_from(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *from = (const ut64 *)data;
	ut64 to = UT64_MAX;
	rz_analysis_get_limits(core->analysis, NULL, &to);
	rz_analysis_set_limits(core->analysis, *from, to);
	return true;
}

/* analysis.limits.to GET */
static bool core_config_get_integer_analysis_limits_to(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *to = (ut64 *)data;

	rz_analysis_get_limits(core->analysis, NULL, to);
	return true;
}

/* analysis.limits.to SET */
static bool core_config_set_integer_analysis_limits_to(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *to = (const ut64 *)data;
	ut64 from = UT64_MAX;
	rz_analysis_get_limits(core->analysis, &from, NULL);
	rz_analysis_set_limits(core->analysis, from, *to);
	return true;
}

/* analysis.recursive.noreturn GET */
static bool core_config_get_bool_analysis_recursive_noreturn(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->recursive_noreturn;
	return true;
}

/* analysis.recursive.noreturn SET */
static bool core_config_set_bool_analysis_recursive_noreturn(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->recursive_noreturn = *value;
	return true;
}

/* analysis.in GET */
static bool core_config_get_string_analysis_in(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	*value = rz_core_get_current_search_map(core);
	return true;
}

/* analysis.in SET */
static bool core_config_set_string_analysis_in(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	if (CONFIG_IS_HELP(*value)) {
		rz_core_print_supported_search_maps();
		return true;
	} else if (!rz_core_set_current_search_map(core, *value)) {
		RZ_LOG_ERROR("config: invalid analysis.in value: %s\n", *value);
		return false;
	}

	return true;
}

/* analysis.jmp.retpoline GET */
static bool core_config_get_bool_analysis_jmp_retpoline(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.retpoline;
	return true;
}

/* analysis.jmp.retpoline SET */
static bool core_config_set_bool_analysis_jmp_retpoline(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.retpoline = *value;
	return true;
}

/* analysis.armthumb GET */
static bool core_config_get_bool_analysis_armthumb(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.armthumb;
	return true;
}

/* analysis.armthumb SET */
static bool core_config_set_bool_analysis_armthumb(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.armthumb = *value;
	return true;
}

/* analysis.jmp.after GET */
static bool core_config_get_bool_analysis_jmp_after(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.afterjmp;
	return true;
}

/* analysis.jmp.after SET */
static bool core_config_set_bool_analysis_jmp_after(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.afterjmp = *value;
	return true;
}

/* analysis.trap.after GET */
static bool core_config_get_bool_analysis_trap_after(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.aftertrap;
	return true;
}

/* analysis.trap.after SET */
static bool core_config_set_bool_analysis_trap_after(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.aftertrap = *value;
	return true;
}

/* analysis.delay GET */
static bool core_config_get_bool_analysis_delay(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.delay;
	return true;
}

/* analysis.delay SET */
static bool core_config_set_bool_analysis_delay(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.delay = *value;
	return true;
}

/* analysis.depth GET */
static bool core_config_get_integer_analysis_depth(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.depth;
	return true;
}

/* analysis.depth SET */
static bool core_config_set_integer_analysis_depth(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.depth = *value;
	return true;
}

/* analysis.graph_depth GET */
static bool core_config_get_integer_analysis_graph_depth(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.graph_depth;
	return true;
}

/* analysis.graph_depth SET */
static bool core_config_set_integer_analysis_graph_depth(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.graph_depth = *value;
	return true;
}

/* analysis.ignbithints GET */
static bool core_config_get_bool_analysis_ignbithints(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.ignbithints;
	return true;
}

/* analysis.ignbithints SET */
static bool core_config_set_bool_analysis_ignbithints(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.ignbithints = *value;
	return true;
}

static bool core_config_add_arch_variables(RzCore *core) {
	RzConfig *cfg = core->config;

	return
		/* analysis variables */
		rz_config_add_b(cfg, "analysis.detectwrites", "Automatically reanalyze function after a write", false) &&
		rz_config_add_s(cfg, "analysis.fcnprefix", "Prefix new function names with this", "fcn") &&
		rz_config_add_b(cfg, "analysis.gpfixed", "Set gp register to analysis.gp before emulating each instruction in aae", true) &&
		rz_config_add_i(cfg, "analysis.timeout", "Stop analyzing after a couple of seconds", 0) &&
		rz_config_add_b(cfg, "analysis.calls", "Make basic af analysis walk into calls", false) &&
		rz_config_add_b(cfg, "analysis.autoname", "Speculatively set a name for the functions, may result in some false positives", false) &&
		rz_config_add_b(cfg, "analysis.hasnext", "Continue analysis after each function", false) &&
		rz_config_add_b(cfg, "analysis.esil", "Use the new ESIL code analysis", false) &&
		rz_config_add_b(cfg, "analysis.types.verbose", "Verbose output from type analysis", false) &&
		rz_config_add_b(cfg, "analysis.types.constraint", "Enable constraint types analysis for variables", false) &&
		rz_config_add_b(cfg, "analysis.vinfun", "Search values in functions (aav) (false by default to only find on non-code)", true) &&
		rz_config_add_b(cfg, "analysis.vinfunrange", "Search values outside function ranges (requires analysis.vinfun=false)", false) &&

		/* analysis binds */
		rz_config_add_s_bind(cfg, "analysis.cc", "Specify default calling convention", core_config_get_string_analysis_cc, core_config_set_string_analysis_cc) &&
		rz_config_add_s_bind(cfg, "analysis.syscc", "Specify default syscall calling convention", core_config_get_string_analysis_syscc, core_config_set_string_analysis_syscc) &&
		rz_config_add_l_bind(cfg, "analysis.roregs", "Comma separated list of register names to be readonly", core_config_get_list_analysis_roregs, core_config_set_list_analysis_roregs) &&
		rz_config_add_i_bind(cfg, "analysis.gp", "Set the value of the GP register (MIPS)", core_config_get_integer_analysis_gp, core_config_set_integer_analysis_gp) &&
		rz_config_add_b_bind(cfg, "analysis.limits", "When enabled, restricts the analysis to the range [analysis.limits.from : analysis.limits.to]", core_config_get_bool_analysis_limits, core_config_set_bool_analysis_limits) &&
		rz_config_add_i_bind(cfg, "analysis.limits.from", "Sets the lower limit of the analysis range [analysis.limits.from : analysis.limits.to]", core_config_get_integer_analysis_limits_from, core_config_set_integer_analysis_limits_from) &&
		rz_config_add_i_bind(cfg, "analysis.limits.to", "Sets the upper limit of the analysis range [analysis.limits.from : analysis.limits.to]", core_config_get_integer_analysis_limits_to, core_config_set_integer_analysis_limits_to) &&
		rz_config_add_b_bind(cfg, "analysis.recursive.noreturn", "When enabled, performs recursive no return check analysis (experimental)", core_config_get_bool_analysis_recursive_noreturn, core_config_set_bool_analysis_recursive_noreturn) &&
		rz_config_add_s_bind(cfg, "analysis.in", "Defines the search boundaries maps for analysis", core_config_get_string_analysis_in, core_config_set_string_analysis_in) &&
		rz_config_set_options(cfg, "analysis.in", rz_core_get_supported_search_maps()) &&
		rz_config_add_b_bind(cfg, "analysis.jmp.retpoline", "Analyze retpolines, may be slower if not needed", core_config_get_bool_analysis_jmp_retpoline, core_config_set_bool_analysis_jmp_retpoline) &&
		rz_config_add_b_bind(cfg, "analysis.armthumb", "aae computes arm/thumb changes (lot of false positives ahead)", core_config_get_bool_analysis_armthumb, core_config_set_bool_analysis_armthumb) &&
		rz_config_add_b_bind(cfg, "analysis.jmp.after", "Continue analysis after jmp/ujmp", core_config_get_bool_analysis_jmp_after, core_config_set_bool_analysis_jmp_after) &&
		rz_config_add_b_bind(cfg, "analysis.trap.after", "Continue analysis after trap instructions.", core_config_get_bool_analysis_trap_after, core_config_set_bool_analysis_trap_after) &&
		rz_config_add_b_bind(cfg, "analysis.delay", "Enable delay slot analysis if supported by the architecture", core_config_get_bool_analysis_delay, core_config_set_bool_analysis_delay) &&
		rz_config_add_i_bind(cfg, "analysis.depth", "Max depth at code analysis", core_config_get_integer_analysis_depth, core_config_set_integer_analysis_depth) &&
		rz_config_add_i_bind(cfg, "analysis.graph_depth", "Max depth for path search", core_config_get_integer_analysis_graph_depth, core_config_set_integer_analysis_graph_depth) &&
		rz_config_add_b_bind(cfg, "analysis.ignbithints", "Ignore the ahb hints (only obey asm.bits)", core_config_get_bool_analysis_ignbithints, core_config_set_bool_analysis_ignbithints) &&

		/* end */
		true;
}
