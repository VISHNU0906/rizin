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
	if (*value && !rz_analysis_cc_exist(core->analysis, *value)) {
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
	if (*value && !rz_analysis_cc_exist(core->analysis, *value)) {
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

	if (!rz_core_set_current_search_map(core, *value)) {
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

/* analysis.nonull GET */
static bool core_config_get_integer_analysis_nonull(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.nonull;
	return true;
}

/* analysis.nonull SET */
static bool core_config_set_integer_analysis_nonull(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.nonull = *value;
	return true;
}

/* analysis.strings GET */
static bool core_config_get_bool_analysis_strings(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: This is a core options and a terrible idea.
	*value = !rz_config_get_b(core->config, "bin.strings");
	return true;
}

/* analysis.strings SET */
static bool core_config_set_bool_analysis_strings(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: This is a core options and a terrible idea.
	return rz_config_set_b(core->config, "bin.strings", !*value);
}

/* analysis.vars GET */
static bool core_config_get_bool_analysis_vars(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.vars;
	return true;
}

/* analysis.vars SET */
static bool core_config_set_bool_analysis_vars(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.vars = *value;
	return true;
}

/* analysis.norevisit GET */
static bool core_config_get_bool_analysis_norevisit(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.norevisit;
	return true;
}

/* analysis.norevisit SET */
static bool core_config_set_bool_analysis_norevisit(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.norevisit = *value;
	return true;
}

/* analysis.nopskip GET */
static bool core_config_get_bool_analysis_nopskip(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.nopskip;
	return true;
}

/* analysis.nopskip SET */
static bool core_config_set_bool_analysis_nopskip(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.nopskip = *value;
	return true;
}

/* analysis.hpskip GET */
static bool core_config_get_bool_analysis_hpskip(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.hpskip;
	return true;
}

/* analysis.hpskip SET */
static bool core_config_set_bool_analysis_hpskip(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.hpskip = *value;
	return true;
}

/* analysis/asm.arch GET */
static bool core_config_get_string_arch_name(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	if (!core->rasm->cur) {
		rz_warn_if_reached();
		return false;
	}

	*value = core->rasm->cur->name;
	return true;
}

/* analysis/asm.arch SET */
static bool core_config_set_string_arch_name(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	if (RZ_STR_ISEMPTY(*value)) {
		RZ_LOG_ERROR("config: invalid analysis.arch value: %s\n", *value);
		return false;
	}

	return rz_core_arch_configure(core, /*arch*/ *value, /*bits*/ 0, /*cpu*/ NULL, /*os*/ NULL, /*platform*/ NULL);
}

/* asm.bits GET */
static bool core_config_get_integer_arch_bits(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	*value = rz_asm_get_bits(core->rasm);
	return true;
}

/* asm.bits SET */
static bool core_config_set_integer_arch_bits(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	return rz_core_arch_configure(core, /*arch*/ NULL, /*bits*/ *value, /*cpu*/ NULL, /*os*/ NULL, /*platform*/ NULL);
}

/* asm.bits OPT */
static bool core_config_opt_string_arch_bits(void *user, const RzList /*<char *>*/ **data) {
	RzCore *core = (RzCore *)user;
	*data = core->rasm->supported_bits;

	return true;
}

/* analysis/asm.cpu GET */
static bool core_config_get_string_arch_cpu(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	*value = rz_asm_get_cpu(core->rasm);
	return true;
}

/* analysis/asm.cpu SET */
static bool core_config_set_string_arch_cpu(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	return rz_core_arch_configure(core, /*arch*/ NULL, /*bits*/ 0, /*cpu*/ *value, /*os*/ NULL, /*platform*/ NULL);
}

/* analysis/asm.cpu OPT */
static bool core_config_opt_string_arch_cpu(void *user, const RzList /*<char *>*/ **data) {
	RzCore *core = (RzCore *)user;
	*data = core->rasm->supported_cpus;

	return true;
}

/* asm.features GET */
static bool core_config_get_string_arch_features(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	*value = core->rasm->features;
	return true;
}

/* asm.features SET */
static bool core_config_set_string_arch_features(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	return rz_asm_set_features(core->rasm, *value);
}

/* asm.features OPT */
static bool core_config_opt_string_arch_features(void *user, const RzList /*<char *>*/ **data) {
	RzCore *core = (RzCore *)user;
	*data = core->rasm->supported_features;

	return true;
}

/* asm.platform GET */
static bool core_config_get_string_arch_platform(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	*value = core->rasm->platforms;
	return true;
}

/* asm.platform SET */
static bool core_config_set_string_arch_platform(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	return rz_core_arch_configure(core, /*arch*/ NULL, /*bits*/ 0, /*cpu*/ NULL, /*os*/ NULL, /*platform*/ *value);
}

/* asm.platform OPT */
static bool core_config_opt_string_arch_platform(void *user, const RzList /*<char *>*/ **data) {
	RzCore *core = (RzCore *)user;
	*data = core->rasm->supported_platforms;

	return true;
}

/* asm.syntax GET */
static bool core_config_get_string_arch_syntax(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	int syntax = rz_asm_get_syntax(core->rasm);
	*value = rz_asm_syntax_to_string(syntax);
	return true;
}

/* asm.syntax SET */
static bool core_config_set_string_arch_syntax(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	int syntax = rz_asm_syntax_from_string(*value);
	return rz_asm_set_syntax(core->rasm, syntax);
}

/* analysis.recont GET */
static bool core_config_get_bool_analysis_recont(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.recont;
	return true;
}

/* analysis.recont SET */
static bool core_config_set_bool_analysis_recont(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.recont = *value;
	return true;
}

/* analysis.jmp.indir GET */
static bool core_config_get_bool_analysis_jmp_indir(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.ijmp;
	return true;
}

/* analysis.jmp.indir SET */
static bool core_config_set_bool_analysis_jmp_indir(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.ijmp = *value;
	return true;
}

/* analysis.jmp.tbl GET */
static bool core_config_get_bool_analysis_jmp_tbl(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.jmptbl;
	return true;
}

/* analysis.jmp.tbl SET */
static bool core_config_set_bool_analysis_jmp_tbl(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.jmptbl = *value;
	return true;
}

/* analysis.jmp.tblmax GET */
static bool core_config_get_integer_analysis_jmp_tblmax(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.jmptbl_maxcount;
	return true;
}

/* analysis.jmp.tblmax SET */
static bool core_config_set_integer_analysis_jmp_tblmax(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.jmptbl_maxcount = *value;
	return true;
}

/* analysis.jmp.tblmaxoffset GET */
static bool core_config_get_integer_analysis_jmp_tblmaxoffset(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.jmptbl_maxoffset;
	return true;
}

/* analysis.jmp.tblmaxoffset SET */
static bool core_config_set_integer_analysis_jmp_tblmaxoffset(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.jmptbl_maxoffset = *value;
	return true;
}

/* analysis.jmp.cref GET */
static bool core_config_get_bool_analysis_jmp_cref(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.cjmpref;
	return true;
}

/* analysis.jmp.cref SET */
static bool core_config_set_bool_analysis_jmp_cref(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.cjmpref = *value;
	return true;
}

/* analysis.jmp.ref GET */
static bool core_config_get_bool_analysis_jmp_ref(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.jmpref;
	return true;
}

/* analysis.jmp.ref SET */
static bool core_config_set_bool_analysis_jmp_ref(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.jmpref = *value;
	return true;
}

/* analysis.jmp.above GET */
static bool core_config_get_bool_analysis_jmp_above(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.jmpabove;
	return true;
}

/* analysis.jmp.above SET */
static bool core_config_set_bool_analysis_jmp_above(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.jmpabove = *value;
	return true;
}

/* analysis.loads GET */
static bool core_config_get_bool_analysis_loads(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.loads;
	return true;
}

/* analysis.loads SET */
static bool core_config_set_bool_analysis_loads(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.loads = *value;
	return true;
}

/* analysis.datarefs GET */
static bool core_config_get_bool_analysis_datarefs(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.followdatarefs;
	return true;
}

/* analysis.datarefs SET */
static bool core_config_set_bool_analysis_datarefs(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.followdatarefs = *value;
	return true;
}

/* analysis.brokenrefs GET */
static bool core_config_get_bool_analysis_brokenrefs(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.followbrokenfcnsrefs;
	return true;
}

/* analysis.brokenrefs SET */
static bool core_config_set_bool_analysis_brokenrefs(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.followbrokenfcnsrefs = *value;
	return true;
}

/* analysis.jmp.mid GET */
static bool core_config_get_bool_analysis_jmp_mid(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.jmpmid;
	return true;
}

/* analysis.jmp.mid SET */
static bool core_config_set_bool_analysis_jmp_mid(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.jmpmid = *value;
	return true;
}

/* analysis.refstr GET */
static bool core_config_get_bool_analysis_refstr(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.searchstringrefs;
	return true;
}

/* analysis.refstr SET */
static bool core_config_set_bool_analysis_refstr(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.searchstringrefs = *value;
	return true;
}

/* analysis.trycatch GET */
static bool core_config_get_bool_analysis_trycatch(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.trycatch;
	return true;
}

/* analysis.trycatch SET */
static bool core_config_set_bool_analysis_trycatch(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.trycatch = *value;
	return true;
}

/* analysis.bb.maxsize GET */
static bool core_config_get_integer_analysis_bb_maxsize(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.bb_max_size;
	return true;
}

/* analysis.bb.maxsize SET */
static bool core_config_set_integer_analysis_bb_maxsize(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.bb_max_size = *value;
	return true;
}

/* analysis.fcn_max_size GET */
static bool core_config_get_integer_analysis_fcn_max_size(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.fcn_max_size;
	return true;
}

/* analysis.fcn_max_size SET */
static bool core_config_set_integer_analysis_fcn_max_size(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.fcn_max_size = *value;
	return true;
}

/* analysis.pushret GET */
static bool core_config_get_bool_analysis_pushret(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->opt.pushret;
	return true;
}

/* analysis.pushret SET */
static bool core_config_set_bool_analysis_pushret(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->opt.pushret = *value;
	return true;
}

/* analysis.cpp.abi GET */
static bool core_config_get_string_analysis_cpp_abi(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	switch (core->analysis->cpp_abi) {
	case RZ_ANALYSIS_CPP_ABI_ITANIUM:
		*value = "itanium";
		return true;
	case RZ_ANALYSIS_CPP_ABI_MSVC:
		*value = "msvc";
		return true;
	default:
		rz_warn_if_reached();
		*value = "";
		return false;
	}
}

/* analysis.cpp.abi SET */
static bool core_config_set_string_analysis_cpp_abi(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	if (RZ_STR_EQ(*value, "itanium")) {
		core->analysis->cpp_abi = RZ_ANALYSIS_CPP_ABI_ITANIUM;
	} else if (RZ_STR_EQ(*value, "msvc")) {
		core->analysis->cpp_abi = RZ_ANALYSIS_CPP_ABI_MSVC;
	} else {
		RZ_LOG_ERROR("config: invalid analysis.cpp.abi value: %s\n", *value);
		return false;
	}
	return true;
}

/* analysis.lines.maxref GET */
static bool core_config_get_integer_analysis_lines_maxref(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->maxreflines;
	return true;
}

/* analysis.lines.maxref SET */
static bool core_config_set_integer_analysis_lines_maxref(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->maxreflines = *value;
	return true;
}

/* asm.os GET */
static bool core_config_get_string_asm_os(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->os;
	return true;
}

/* asm.os SET */
static bool core_config_set_string_asm_os(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	return rz_core_arch_configure(core, /*arch*/ NULL, /*bits*/ 0, /*cpu*/ NULL, /*os*/ *value, /*platform*/ NULL);
}

/* asm.invhex GET */
static bool core_config_get_bool_asm_invhex(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAsm
	*value = core->rasm->invhex;
	return true;
}

/* asm.invhex SET */
static bool core_config_set_bool_asm_invhex(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAsm
	core->rasm->invhex = *value;
	return true;
}

/* asm.pcalign GET */
static bool core_config_get_integer_asm_pcalign(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	// TODO: create get function in RzAnalysis & RzAsm
	*value = core->rasm->pcalign;
	return true;
}

/* asm.pcalign SET */
static bool core_config_set_integer_asm_pcalign(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	if (rz_bits_count_ones_ut64(*value) != 1) {
		RZ_LOG_ERROR("config: alignment is only defined for '>0' and 'n^2'. Is = %" PFMT64d "\n", *value);
		return false;
	}

	// TODO: create set function in RzAnalysis & RzAsm
	core->rasm->pcalign = *value;
	core->analysis->pcalign = *value;
	return true;
}

/* asm.immhash GET */
static bool core_config_get_bool_asm_immhash(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAsm
	*value = core->rasm->immdisp;
	return true;
}

/* asm.immhash SET */
static bool core_config_set_bool_asm_immhash(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAsm
	core->rasm->immdisp = *value;
	return true;
}

/* asm.flags.maxname GET */
static bool core_config_get_integer_asm_flags_maxname(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	// TODO: create get function in RzParser
	*value = core->parser->maxflagnamelen;
	return true;
}

/* asm.flags.maxname SET */
static bool core_config_set_integer_asm_flags_maxname(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	// TODO: create set function in RzParser
	core->parser->maxflagnamelen = *value;
	return true;
}

/* asm.flags.real GET */
static bool core_config_get_bool_asm_flags_real(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzFlags
	*value = core->flags->realnames;
	return true;
}

/* asm.flags.real SET */
static bool core_config_set_bool_asm_flags_real(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzFlags
	core->flags->realnames = *value;
	return true;
}

/* asm.lines.width GET */
static bool core_config_get_integer_asm_lines_width(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	// TODO: create get function in RzAnalysis
	*value = core->analysis->lineswidth;
	return true;
}

/* asm.lines.width SET */
static bool core_config_set_integer_asm_lines_width(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	// TODO: create set function in RzAnalysis
	core->analysis->lineswidth = *value;
	return true;
}

/* asm.sub.varmin GET */
static bool core_config_get_integer_asm_sub_varmin(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	// TODO: create get function in RzParser
	*value = core->parser->minval;
	return true;
}

/* asm.sub.varmin SET */
static bool core_config_set_integer_asm_sub_varmin(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	// TODO: create set function in RzParser
	core->parser->minval = *value;
	return true;
}

/* asm.sub.tail GET */
static bool core_config_get_bool_asm_sub_tail(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzParser
	*value = core->parser->subtail;
	return true;
}

/* asm.sub.tail SET */
static bool core_config_set_bool_asm_sub_tail(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzParser
	core->parser->subtail = *value;
	return true;
}

/* asm.sub.section GET */
static bool core_config_get_bool_asm_sub_section(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	*value = rz_print_has_flags(core->print, RZ_PRINT_FLAGS_SECSUB);
	return true;
}

/* asm.sub.section SET */
static bool core_config_set_bool_asm_sub_section(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	if (*value) {
		rz_print_add_flags(core->print, RZ_PRINT_FLAGS_SECSUB);
	} else {
		rz_print_remove_flags(core->print, RZ_PRINT_FLAGS_SECSUB);
	}
	return true;
}

/* asm.pseudo GET */
static bool core_config_get_bool_asm_pseudo(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzAsm
	*value = core->rasm->pseudo;
	return true;
}

/* asm.pseudo SET */
static bool core_config_set_bool_asm_pseudo(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create set function in RzAsm
	core->rasm->pseudo = *value;
	return true;
}

/* asm.assembler GET */
static bool core_config_get_string_asm_assembler(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	// TODO: create get function in RzAsm
	if (core->rasm->acur) {
		*value = core->rasm->acur->name;
	} else {
		*value = "";
	}
	return true;
}

/* asm.assembler SET */
static bool core_config_set_string_asm_assembler(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	rz_asm_use_assembler(core->rasm, *value);
	return true;
}

/*asm.segoff GET */
static bool core_config_get_bool_asm_segoff(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	*value = rz_print_has_flags(core->print, RZ_PRINT_FLAGS_SEGOFF);
	return true;
}

/*asm.segoff SET */
static bool core_config_set_bool_asm_segoff(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	if (*value) {
		rz_print_add_flags(core->print, RZ_PRINT_FLAGS_SEGOFF);
	} else {
		rz_print_remove_flags(core->print, RZ_PRINT_FLAGS_SEGOFF);
	}
	return true;
}

/*asm.decoff GET */
static bool core_config_get_bool_asm_decoff(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	*value = rz_print_has_flags(core->print, RZ_PRINT_FLAGS_ADDRDEC);
	return true;
}

/*asm.decoff SET */
static bool core_config_set_bool_asm_decoff(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	if (*value) {
		rz_print_add_flags(core->print, RZ_PRINT_FLAGS_ADDRDEC);
	} else {
		rz_print_remove_flags(core->print, RZ_PRINT_FLAGS_ADDRDEC);
	}
	return true;
}

/* asm.seggrn GET */
static bool core_config_get_integer_asm_seggrn(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	// TODO: create get function in RzAsm
	*value = core->rasm->seggrn;
	return true;
}

/* asm.seggrn SET */
static bool core_config_set_integer_asm_seggrn(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	// TODO: create set function in RzAsm, RzAnalysis, RzPrint
	core->rasm->seggrn = *value;
	core->print->seggrn = *value;
	core->analysis->seggrn = *value;
	return true;
}

static bool core_config_add_arch_variables(RzCore *core) {
	RzConfig *cfg = core->config;

	bool ok =
		/* asm binds */
		rz_config_add_b_bind(cfg, "asm.decoff", "Show address in base 10 instead of hexadecimal", core_config_get_bool_asm_decoff, core_config_set_bool_asm_decoff) &&
		rz_config_add_b_bind(cfg, "asm.flags.real", "Show flags' unfiltered realnames instead of names, except realnames from demangling", core_config_get_bool_asm_flags_real, core_config_set_bool_asm_flags_real) &&
		rz_config_add_b_bind(cfg, "asm.imm.hash", "Hides the hashtag (#) char for immediates in various archs", core_config_get_bool_asm_immhash, core_config_set_bool_asm_immhash) &&
		rz_config_add_b_bind(cfg, "asm.invhex", "Show invalid instructions as hexadecimal numbers", core_config_get_bool_asm_invhex, core_config_set_bool_asm_invhex) &&
		rz_config_add_b_bind(cfg, "asm.pseudo", "Enable pseudo syntax", core_config_get_bool_asm_pseudo, core_config_set_bool_asm_pseudo) &&
		rz_config_add_b_bind(cfg, "asm.segoff", "Show segmented address in prompt (x86-16)", core_config_get_bool_asm_segoff, core_config_set_bool_asm_segoff) &&
		rz_config_add_b_bind(cfg, "asm.sub.section", "Show offsets in disasm prefixed with section/map name", core_config_get_bool_asm_sub_section, core_config_set_bool_asm_sub_section) &&
		rz_config_add_b_bind(cfg, "asm.sub.tail", "Replace addresses with prefix .. syntax", core_config_get_bool_asm_sub_tail, core_config_set_bool_asm_sub_tail) &&
		rz_config_add_i_bind(cfg, "asm.flags.maxname", "Maximum length of flag name with smart chopping", core_config_get_integer_asm_flags_maxname, core_config_set_integer_asm_flags_maxname) &&
		rz_config_add_i_bind(cfg, "asm.lines.width", "Number of columns for program flow arrows", core_config_get_integer_asm_lines_width, core_config_set_integer_asm_lines_width) &&
		rz_config_add_i_bind(cfg, "asm.pcalign", "Only recognize as valid instructions aligned to this value", core_config_get_integer_asm_pcalign, core_config_set_integer_asm_pcalign) &&
		rz_config_add_i_bind(cfg, "asm.seggrn", "Segment granularity in bits (x86-16)", core_config_get_integer_asm_seggrn, core_config_set_integer_asm_seggrn) &&
		rz_config_add_i_bind(cfg, "asm.sub.varmin", "Minimum value to substitute in instructions (asm.sub.var)", core_config_get_integer_asm_sub_varmin, core_config_set_integer_asm_sub_varmin) &&
		rz_config_add_l_bind(cfg, "asm.features", "Specify supported features by the target CPU", core_config_get_string_arch_features, core_config_set_string_arch_features) &&
		rz_config_add_o_bind(cfg, "asm.cpu", "Set the kind of asm.arch cpu", core_config_get_string_arch_cpu, core_config_set_string_arch_cpu, core_config_opt_string_arch_cpu) &&
		rz_config_add_o_bind(cfg, "asm.platform", "Specify supported platforms by the target architecture", core_config_get_string_arch_platform, core_config_set_string_arch_platform, core_config_opt_string_arch_platform) &&
		rz_config_add_s_bind(cfg, "asm.arch", "Set the arch to be used by asm", core_config_get_string_arch_name, core_config_set_string_arch_name) &&
		rz_config_add_s_bind(cfg, "asm.assembler", "Set the plugin name to use when assembling", core_config_get_string_asm_assembler, core_config_set_string_asm_assembler) &&
		rz_config_add_s_bind(cfg, "asm.os", "Select operating system (kernel)", core_config_get_string_asm_os, core_config_set_string_asm_os) &&
		rz_config_add_s_bind(cfg, "asm.syntax", "Select assembly syntax", core_config_get_string_arch_syntax, core_config_set_string_arch_syntax) &&
		rz_config_add_i_bind(cfg, "asm.bits", "Word size in bits at assembler", core_config_get_integer_arch_bits, core_config_set_integer_arch_bits) &&

		/* analysis binds */
		rz_config_add_b_bind(cfg, "analysis.armthumb", "aae computes arm/thumb changes (lot of false positives ahead)", core_config_get_bool_analysis_armthumb, core_config_set_bool_analysis_armthumb) &&
		rz_config_add_b_bind(cfg, "analysis.brokenrefs", "Follow function references even if function analysis has failed", core_config_get_bool_analysis_brokenrefs, core_config_set_bool_analysis_brokenrefs) &&
		rz_config_add_b_bind(cfg, "analysis.datarefs", "Follow data references for code coverage", core_config_get_bool_analysis_datarefs, core_config_set_bool_analysis_datarefs) &&
		rz_config_add_b_bind(cfg, "analysis.delay", "Enable delay slot analysis if supported by the architecture", core_config_get_bool_analysis_delay, core_config_set_bool_analysis_delay) &&
		rz_config_add_b_bind(cfg, "analysis.hpskip", "Skip `mov reg, reg` and `lea reg, [reg] at the beginning of functions", core_config_get_bool_analysis_hpskip, core_config_set_bool_analysis_hpskip) &&
		rz_config_add_b_bind(cfg, "analysis.ignbithints", "Ignore the ahb hints (only obey asm.bits)", core_config_get_bool_analysis_ignbithints, core_config_set_bool_analysis_ignbithints) &&
		rz_config_add_b_bind(cfg, "analysis.jmp.above", "Jump above function pointer", core_config_get_bool_analysis_jmp_above, core_config_set_bool_analysis_jmp_above) &&
		rz_config_add_b_bind(cfg, "analysis.jmp.after", "Continue analysis after jmp/ujmp", core_config_get_bool_analysis_jmp_after, core_config_set_bool_analysis_jmp_after) &&
		rz_config_add_b_bind(cfg, "analysis.jmp.cref", "Create references for conditional jumps", core_config_get_bool_analysis_jmp_cref, core_config_set_bool_analysis_jmp_cref) &&
		rz_config_add_b_bind(cfg, "analysis.jmp.indir", "Follow the indirect jumps in function analysis", core_config_get_bool_analysis_jmp_indir, core_config_set_bool_analysis_jmp_indir) &&
		rz_config_add_b_bind(cfg, "analysis.jmp.mid", "Continue analysis after jump to middle of instruction (x86 only)", core_config_get_bool_analysis_jmp_mid, core_config_set_bool_analysis_jmp_mid) &&
		rz_config_add_b_bind(cfg, "analysis.jmp.ref", "Create references for unconditional jumps", core_config_get_bool_analysis_jmp_ref, core_config_set_bool_analysis_jmp_ref) &&
		rz_config_add_b_bind(cfg, "analysis.jmp.retpoline", "Analyze retpolines, may be slower if not needed", core_config_get_bool_analysis_jmp_retpoline, core_config_set_bool_analysis_jmp_retpoline) &&
		rz_config_add_b_bind(cfg, "analysis.jmp.tbl", "Analyze jump tables in switch statements", core_config_get_bool_analysis_jmp_tbl, core_config_set_bool_analysis_jmp_tbl) &&
		rz_config_add_b_bind(cfg, "analysis.limits", "When enabled, restricts the analysis to the range [analysis.limits.from : analysis.limits.to]", core_config_get_bool_analysis_limits, core_config_set_bool_analysis_limits) &&
		rz_config_add_b_bind(cfg, "analysis.loads", "Define as dword/string/qword when analyzing load instructions", core_config_get_bool_analysis_loads, core_config_set_bool_analysis_loads) &&
		rz_config_add_b_bind(cfg, "analysis.nopskip", "Skip nops at the beginning of functions", core_config_get_bool_analysis_nopskip, core_config_set_bool_analysis_nopskip) &&
		rz_config_add_b_bind(cfg, "analysis.norevisit", "Do not visit function analysis twice (experimental)", core_config_get_bool_analysis_norevisit, core_config_set_bool_analysis_norevisit) &&
		rz_config_add_b_bind(cfg, "analysis.pushret", "Analyze push+ret as jmp", core_config_get_bool_analysis_pushret, core_config_set_bool_analysis_pushret) &&
		rz_config_add_b_bind(cfg, "analysis.recont", "End block after splitting a basic block instead of error", core_config_get_bool_analysis_recont, core_config_set_bool_analysis_recont) &&
		rz_config_add_b_bind(cfg, "analysis.recursive.noreturn", "When enabled, performs recursive no return check analysis (experimental)", core_config_get_bool_analysis_recursive_noreturn, core_config_set_bool_analysis_recursive_noreturn) &&
		rz_config_add_b_bind(cfg, "analysis.refstr", "Search string references in data references", core_config_get_bool_analysis_refstr, core_config_set_bool_analysis_refstr) &&
		rz_config_add_b_bind(cfg, "analysis.strings", "Identify and register strings during analysis (aar only)", core_config_get_bool_analysis_strings, core_config_set_bool_analysis_strings) &&
		rz_config_add_b_bind(cfg, "analysis.trap.after", "Continue analysis after trap instructions.", core_config_get_bool_analysis_trap_after, core_config_set_bool_analysis_trap_after) &&
		rz_config_add_b_bind(cfg, "analysis.trycatch", "Honor try.X.Y.{from,to,catch} flags", core_config_get_bool_analysis_trycatch, core_config_set_bool_analysis_trycatch) &&
		rz_config_add_b_bind(cfg, "analysis.vars", "Analyze local variables and arguments", core_config_get_bool_analysis_vars, core_config_set_bool_analysis_vars) &&
		rz_config_add_i_bind(cfg, "analysis.bb.maxsize", "Maximum basic block size", core_config_get_integer_analysis_bb_maxsize, core_config_set_integer_analysis_bb_maxsize) &&
		rz_config_add_i_bind(cfg, "analysis.depth", "Max depth at code analysis", core_config_get_integer_analysis_depth, core_config_set_integer_analysis_depth) &&
		rz_config_add_i_bind(cfg, "analysis.fcn_max_size", "Maximum function size (unspecified units)", core_config_get_integer_analysis_fcn_max_size, core_config_set_integer_analysis_fcn_max_size) &&
		rz_config_add_i_bind(cfg, "analysis.gp", "Set the value of the GP register (MIPS)", core_config_get_integer_analysis_gp, core_config_set_integer_analysis_gp) &&
		rz_config_add_i_bind(cfg, "analysis.graph_depth", "Max depth for path search", core_config_get_integer_analysis_graph_depth, core_config_set_integer_analysis_graph_depth) &&
		rz_config_add_i_bind(cfg, "analysis.jmp.tblmax", "Maximum amount of entries to analyze in jump tables", core_config_get_integer_analysis_jmp_tblmax, core_config_set_integer_analysis_jmp_tblmax) &&
		rz_config_add_i_bind(cfg, "analysis.jmp.tblmaxoffset", "Maximum offset from the jump table jump instruction to consider it valid", core_config_get_integer_analysis_jmp_tblmaxoffset, core_config_set_integer_analysis_jmp_tblmaxoffset) &&
		rz_config_add_i_bind(cfg, "analysis.limits.from", "Sets the lower limit of the analysis range [analysis.limits.from : analysis.limits.to]", core_config_get_integer_analysis_limits_from, core_config_set_integer_analysis_limits_from) &&
		rz_config_add_i_bind(cfg, "analysis.limits.to", "Sets the upper limit of the analysis range [analysis.limits.from : analysis.limits.to]", core_config_get_integer_analysis_limits_to, core_config_set_integer_analysis_limits_to) &&
		rz_config_add_i_bind(cfg, "analysis.lines.maxref", "Maximum number of reflines to be analyzed and displayed in asm.lines with pd", core_config_get_integer_analysis_lines_maxref, core_config_set_integer_analysis_lines_maxref) &&
		rz_config_add_i_bind(cfg, "analysis.nonull", "Do not analyze regions of N null bytes (0 is disabled)", core_config_get_integer_analysis_nonull, core_config_set_integer_analysis_nonull) &&
		rz_config_add_l_bind(cfg, "analysis.roregs", "Comma separated list of register names to be readonly", core_config_get_list_analysis_roregs, core_config_set_list_analysis_roregs) &&
		rz_config_add_s_bind(cfg, "analysis.arch", "Select the architecture to use", core_config_get_string_arch_name, core_config_set_string_arch_name) &&
		rz_config_add_s_bind(cfg, "analysis.cc", "Specify default calling convention", core_config_get_string_analysis_cc, core_config_set_string_analysis_cc) &&
		rz_config_add_s_bind(cfg, "analysis.cpp.abi", "Select C++ ABI (Compiler)", core_config_get_string_analysis_cpp_abi, core_config_set_string_analysis_cpp_abi) &&
		rz_config_add_s_bind(cfg, "analysis.cpu", "Specify the analysis.cpu to use", core_config_get_string_arch_cpu, core_config_set_string_arch_cpu) &&
		rz_config_add_s_bind(cfg, "analysis.in", "Defines the search boundaries maps for analysis", core_config_get_string_analysis_in, core_config_set_string_analysis_in) &&
		rz_config_add_s_bind(cfg, "analysis.syscc", "Specify default syscall calling convention", core_config_get_string_analysis_syscc, core_config_set_string_analysis_syscc) &&

		/* analysis variables */
		rz_config_add_b(cfg, "analysis.apply.signature", "enables/disables auto-applying signatures to the loaded binary (see also flirt.sigdb.path)", true) &&
		rz_config_add_b(cfg, "analysis.autoname", "Speculatively set a name for the functions, may result in some false positives", false) &&
		rz_config_add_b(cfg, "analysis.calls", "Make basic af analysis walk into calls", false) &&
		rz_config_add_b(cfg, "analysis.detectwrites", "Automatically reanalyze function after a write", false) &&
		rz_config_add_b(cfg, "analysis.esil", "Use the new ESIL code analysis", false) &&
		rz_config_add_b(cfg, "analysis.gpfixed", "Set gp register to analysis.gp before emulating each instruction in aae", true) &&
		rz_config_add_b(cfg, "analysis.hasnext", "Continue analysis after each function", false) &&
		rz_config_add_b(cfg, "analysis.resolve.pointers", "enables/disables analysis of pointers to data sections.", true) &&
		rz_config_add_b(cfg, "analysis.types.constraint", "Enable constraint types analysis for variables", false) &&
		rz_config_add_b(cfg, "analysis.types.verbose", "Verbose output from type analysis", false) &&
		rz_config_add_b(cfg, "analysis.vinfun", "Search values in functions (aav) (false by default to only find on non-code)", true) &&
		rz_config_add_b(cfg, "analysis.vinfunrange", "Search values outside function ranges (requires analysis.vinfun=false)", false) &&
		rz_config_add_i(cfg, "analysis.prelude.limit", "Maximum size of the range to scan for preludes", 1024 * 1024 * 20) &&
		rz_config_add_i(cfg, "analysis.ptrdepth", "Maximum number of nested pointers to follow in analysis", 3) &&
		rz_config_add_i(cfg, "analysis.timeout", "Stop analyzing after a couple of seconds", 0) &&
		rz_config_add_s(cfg, "analysis.fcnprefix", "Prefix new function names with this", "fcn") &&
		rz_config_add_s(cfg, "analysis.prelude", "Specify an hexpair to find preludes in code", "") &&
		rz_config_add_s(cfg, "analysis.types.spec", "Set profile for specifying format chars used in type analysis", "gcc") &&

		/* asm variables */
		rz_config_add_b(cfg, "asm.analysis", "Analyze code and refs while disassembling (see analysis.strings)", false) &&
		rz_config_add_b(cfg, "asm.bb.line", "Show empty line after every basic block", false) &&
		rz_config_add_b(cfg, "asm.bb.middle", "Realign disassembly if a basic block starts in the middle of an instruction", true) &&
		rz_config_add_b(cfg, "asm.bytes", "Display the bytes of each instruction", false) &&
		rz_config_add_b(cfg, "asm.bytes.right", "Display the bytes at the right of the disassembly", false) &&
		rz_config_add_b(cfg, "asm.bytes.space", "Separate hexadecimal bytes with a whitespace", false) &&
		rz_config_add_b(cfg, "asm.calls", "Show callee function related info as comments in disasm", true) &&
		rz_config_add_b(cfg, "asm.capitalize", "Use camelcase at disassembly", false) &&
		rz_config_add_b(cfg, "asm.cmt.esil", "Show ESIL expressions as comments", false) &&
		rz_config_add_b(cfg, "asm.cmt.flgrefs", "Show comment flags associated to branch reference", true) &&
		rz_config_add_b(cfg, "asm.cmt.il", "Show RzIL expressions as comments", false) &&
		rz_config_add_b(cfg, "asm.cmt.patch", "Show patch comments in disasm", false) &&
		rz_config_add_b(cfg, "asm.cmt.refs", "Show flag and comments from refs in disasm", false) &&
		rz_config_add_b(cfg, "asm.cmt.right", "Show comments at right of disassembly if they fit in screen", true) &&
		rz_config_add_b(cfg, "asm.comments", "Show comments in disassembly view", true) &&
		rz_config_add_b(cfg, "asm.cycles", "Show CPU-cycles taken by instruction at disassembly", false) &&
		rz_config_add_b(cfg, "asm.cyclespace", "Indent instructions depending on CPU-cycles", false) &&
		rz_config_add_b(cfg, "asm.debuginfo", "Show debug information in disassembly", false) &&
		rz_config_add_b(cfg, "asm.debuginfo.abspath", "Show absolute path from the debug information", false) &&
		rz_config_add_b(cfg, "asm.debuginfo.file", "Show file name from the debug information in disassembly", true) &&
		rz_config_add_b(cfg, "asm.debuginfo.lines", "Show source line information from debug metadata in disassembly", true) &&
		rz_config_add_b(cfg, "asm.decode", "Use code analysis as a disassembler", false) &&
		rz_config_add_b(cfg, "asm.demangle", "Show demangled symbols in disasm", true) &&
		rz_config_add_b(cfg, "asm.describe", "Show opcode description", false) &&
		rz_config_add_b(cfg, "asm.emu", "Run ESIL emulation analysis on disasm", false) &&
		rz_config_add_b(cfg, "asm.esil", "Show ESIL instead of mnemonic", false) &&
		rz_config_add_b(cfg, "asm.family", "Show family name in disasm", false) &&
		rz_config_add_b(cfg, "asm.fcn.signature", "Show function signature in disasm", true) &&
		rz_config_add_b(cfg, "asm.fcn.size", "Show function size in disasm", false) &&
		rz_config_add_b(cfg, "asm.flags", "Show flags", true) &&
		rz_config_add_b(cfg, "asm.flags.inbytes", "Display flags inside the bytes space", false) &&
		rz_config_add_b(cfg, "asm.flags.inline", "Display flags in line separated by commas instead of newlines", false) &&
		rz_config_add_b(cfg, "asm.flags.offset", "Show offset in flags", false) &&
		rz_config_add_b(cfg, "asm.functions", "Show functions in disassembly", true) &&
		rz_config_add_b(cfg, "asm.hint.call", "Show call hints [numbers] in disasm", true) &&
		rz_config_add_b(cfg, "asm.hint.call.indirect", "Hints for indirect call intructions go to the call destination", true) &&
		rz_config_add_b(cfg, "asm.hint.cdiv", "Show CDIV hints optimization hint", false) &&
		rz_config_add_b(cfg, "asm.hint.emu", "Show asm.emu hints [numbers] in disasm", false) &&
		rz_config_add_b(cfg, "asm.hint.jmp", "Show jump hints [numbers] in disasm", false) &&
		rz_config_add_b(cfg, "asm.hint.lea", "Show LEA hints [numbers] in disasm", false) &&
		rz_config_add_b(cfg, "asm.hints", "Disable all asm.hint* if false", true) &&
		rz_config_add_b(cfg, "asm.imm.str", "Show immediates values as strings", true) &&
		rz_config_add_b(cfg, "asm.imm.trim", "Remove all offsets and constants from disassembly", false) &&
		rz_config_add_b(cfg, "asm.indent", "Indent disassembly based on reflines depth", false) &&
		rz_config_add_b(cfg, "asm.instr", "Display the disassembled instruction", true) &&
		rz_config_add_b(cfg, "asm.lbytes", "Align disasm bytes to left", true) &&
		rz_config_add_b(cfg, "asm.lines", "Show ASCII-art lines at disassembly", true) &&
		rz_config_add_b(cfg, "asm.lines.bb", "Show flow lines at jumps", true) &&
		rz_config_add_b(cfg, "asm.lines.call", "Enable call lines", false) &&
		rz_config_add_b(cfg, "asm.lines.fcn", "Show function boundary lines", true) &&
		rz_config_add_b(cfg, "asm.lines.out", "Show out of block lines", true) &&
		rz_config_add_b(cfg, "asm.lines.ret", "Show separator lines after ret", false) &&
		rz_config_add_b(cfg, "asm.lines.right", "Show lines before opcode instead of offset", false) &&
		rz_config_add_b(cfg, "asm.lines.wide", "Put a space between lines", false) &&
		rz_config_add_b(cfg, "asm.marks", "Show marks before the disassembly", true) &&
		rz_config_add_b(cfg, "asm.meta", "Display the code/data/format conversions in disasm", true) &&
		rz_config_add_b(cfg, "asm.midcursor", "Cursor in visual disasm mode breaks the instruction", false) &&
		rz_config_add_b(cfg, "asm.middle", "Allow disassembling jumps in the middle of an instruction", false) &&
		rz_config_add_b(cfg, "asm.minicols", "Only show the instruction in the column disasm", false) &&
		rz_config_add_b(cfg, "asm.nodup", "Do not show dupped instructions (collapse disasm)", false) &&
		rz_config_add_b(cfg, "asm.noisy", "Show comments considered noisy but possibly useful", true) &&
		rz_config_add_b(cfg, "asm.offset", "Show offsets in disassembly", true) &&
		rz_config_add_b(cfg, "asm.optype", "show opcode type next to the instruction bytes", false) &&
		rz_config_add_b(cfg, "asm.payloads", "Show payload bytes in disasm", false) &&
		rz_config_add_b(cfg, "asm.refptr", "Show refpointer information in disasm", true) &&
		rz_config_add_b(cfg, "asm.reloff", "Show relative offsets instead of absolute address in disasm", false) &&
		rz_config_add_b(cfg, "asm.reloff.flags", "Show relative offsets to flags (not only functions)", false) &&
		rz_config_add_b(cfg, "asm.section", "Show section name before offset", false) &&
		rz_config_add_b(cfg, "asm.section.name", "Show section name in the disasm", true) &&
		rz_config_add_b(cfg, "asm.section.perm", "Show section permissions in the disasm", false) &&
		rz_config_add_b(cfg, "asm.size", "Show size of opcodes in disassembly (pd)", false) &&
		rz_config_add_b(cfg, "asm.slow", "Perform slow analysis operations in disasm", true) &&
		rz_config_add_b(cfg, "asm.stackptr", "Show stack pointer at disassembly", false) &&
		rz_config_add_b(cfg, "asm.sub.jmp", "Always substitute jump, call and branch targets in disassembly", true) &&
		rz_config_add_b(cfg, "asm.sub.names", "Replace numeric values by flags (e.g. 0x4003e0 -> sym.imp.printf)", true) &&
		rz_config_add_b(cfg, "asm.sub.reg", "Substitute register names with their associated role name (drp~=)", false) &&
		rz_config_add_b(cfg, "asm.sub.rel", "Substitute pc relative expressions in disasm", true) &&
		rz_config_add_b(cfg, "asm.sub.var", "Substitute variables in disassembly", true) &&
		rz_config_add_b(cfg, "asm.sub.varonly", "Substitute the entire variable expression with the local variable name (e.g. [local10h] instead of [ebp+local10h])", true) &&
		rz_config_add_b(cfg, "asm.symbol", "Show symbol+delta instead of absolute offset", false) &&
		rz_config_add_b(cfg, "asm.tabs.once", "Only tabulate the opcode, not the arguments", true) &&
		rz_config_add_b(cfg, "asm.trace", "Show execution traces for each opcode", false) &&
		rz_config_add_b(cfg, "asm.tracespace", "Indent disassembly with trace.count information", false) &&
		rz_config_add_b(cfg, "asm.ucase", "Use uppercase syntax at disassembly", false) &&
		rz_config_add_b(cfg, "asm.usercomments", "Show user comments even if asm.comments is false", false) &&
		rz_config_add_b(cfg, "asm.var", "Show local function variables in disassembly", true) &&
		rz_config_add_b(cfg, "asm.var.access", "Show accesses of local variables", false) &&
		rz_config_add_b(cfg, "asm.xrefs", "Show xrefs in disassembly", true) &&
		rz_config_add_b(cfg, "asm.xrefs.code", "Show the code xrefs (generated by jumps instead of calls)", true) &&
		rz_config_add_i(cfg, "asm.cmt.col", "Column to align comments", 71) &&
		rz_config_add_i(cfg, "asm.flags.limit", "Maximum number of flags to show in a single offset", 0) &&
		rz_config_add_i(cfg, "asm.flags.middle", "Realign disassembly if there is a flag in the middle of an instruction (0 hide flag, 1 show but no-realign, 2 realign at middle flag, 3 realign at middle if symbol)", 2) &&
		rz_config_add_i(cfg, "asm.hint.pos", "Shortcut hint position (-1, 0, 1)", 1) &&
		rz_config_add_i(cfg, "asm.indentspace", "How many spaces to indent the code", 2) &&
		rz_config_add_i(cfg, "asm.nbytes", "Number of bytes for each opcode at disassembly", 6) &&
		rz_config_add_i(cfg, "asm.section.col", "Columns width to show asm.section", 30) &&
		rz_config_add_i(cfg, "asm.symbol.col", "Columns width to show asm.section", 40) &&
		rz_config_add_i(cfg, "asm.tabs", "Use tabs in disassembly", 6) &&
		rz_config_add_i(cfg, "asm.tabs.off", "tabulate spaces after the offset", 0) &&
		rz_config_add_i(cfg, "asm.types", "Display the fcn types in calls (0=no,1=quiet,2=verbose)", 1) &&
		rz_config_add_i(cfg, "asm.var.summary", "Show variables summary instead of full list in disasm (0, 1, 2)", 0) &&
		rz_config_add_i(cfg, "asm.xrefs.fold", "Maximum number of xrefs to be displayed as list (use columns above)", 5) &&
		rz_config_add_i(cfg, "asm.xrefs.max", "Maximum number of xrefs to be displayed without folding", 20) &&
		rz_config_add_o(cfg, "asm.var.fold", "Fold same-typed variables", "none", "group", "hide", NULL) &&
		rz_config_add_s(cfg, "asm.cmt.off", "Show offset comment in disasm (true, false, nodup)", "nodup") &&
		rz_config_add_s(cfg, "asm.highlight", "Highlight current line", "") &&
		rz_config_add_s(cfg, "asm.strip", "strip all instructions given comma separated types", "") &&

		/* set special options */
		rz_config_set_options(cfg, "analysis.cc", rz_analysis_calling_conventions(core->analysis)) &&
		rz_config_set_options(cfg, "analysis.syscc", rz_analysis_calling_conventions(core->analysis)) &&
		rz_config_set_options(cfg, "analysis.in", rz_core_get_supported_search_maps()) &&
		rz_config_set_options(cfg, "analysis.arch", rz_core_analysis_get_supported_archs(core)) &&
		rz_config_set_options(cfg, "asm.arch", rz_core_analysis_get_supported_archs(core)) &&
		rz_config_set_options2(cfg, "analysis.cpp.abi", "itanium", "msvc", NULL) &&
		rz_config_set_options2(cfg, "asm.syntax", "none", "att", "intel", "masm", "jz", "regnum", NULL) &&
		rz_config_set_options2(cfg, "asm.os", "ios", "dos", "darwin", "linux", "freebsd", "openbsd", "netbsd", "windows", "s110", "none", NULL) &&
		rz_config_set_options3(cfg, "asm.bits", core_config_opt_string_arch_bits) &&
		rz_config_set_options3(cfg, "asm.features", core_config_opt_string_arch_features);

	// we need to call `rz_core_arch_configure` once.
	return ok && rz_core_arch_configure(core, /*arch*/ RZ_SYS_ARCH, /*bits*/ 0, /*cpu*/ NULL, /*os*/ NULL, /*platform*/ NULL);
}
