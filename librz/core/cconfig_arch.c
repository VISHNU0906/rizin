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

static bool core_config_add_arch_variables(RzCore *core) {
	RzConfig *cfg = core->config;

	return
		/* analysis binds */
		rz_config_add_b_bind(cfg, "analysis.armthumb", "aae computes arm/thumb changes (lot of false positives ahead)", core_config_get_bool_analysis_armthumb, core_config_set_bool_analysis_armthumb) &&
		rz_config_add_b_bind(cfg, "analysis.delay", "Enable delay slot analysis if supported by the architecture", core_config_get_bool_analysis_delay, core_config_set_bool_analysis_delay) &&
		rz_config_add_b_bind(cfg, "analysis.ignbithints", "Ignore the ahb hints (only obey asm.bits)", core_config_get_bool_analysis_ignbithints, core_config_set_bool_analysis_ignbithints) &&
		rz_config_add_b_bind(cfg, "analysis.jmp.after", "Continue analysis after jmp/ujmp", core_config_get_bool_analysis_jmp_after, core_config_set_bool_analysis_jmp_after) &&
		rz_config_add_b_bind(cfg, "analysis.jmp.retpoline", "Analyze retpolines, may be slower if not needed", core_config_get_bool_analysis_jmp_retpoline, core_config_set_bool_analysis_jmp_retpoline) &&
		rz_config_add_b_bind(cfg, "analysis.limits", "When enabled, restricts the analysis to the range [analysis.limits.from : analysis.limits.to]", core_config_get_bool_analysis_limits, core_config_set_bool_analysis_limits) &&
		rz_config_add_b_bind(cfg, "analysis.recursive.noreturn", "When enabled, performs recursive no return check analysis (experimental)", core_config_get_bool_analysis_recursive_noreturn, core_config_set_bool_analysis_recursive_noreturn) &&
		rz_config_add_b_bind(cfg, "analysis.trap.after", "Continue analysis after trap instructions.", core_config_get_bool_analysis_trap_after, core_config_set_bool_analysis_trap_after) &&
		rz_config_add_i_bind(cfg, "analysis.depth", "Max depth at code analysis", core_config_get_integer_analysis_depth, core_config_set_integer_analysis_depth) &&
		rz_config_add_i_bind(cfg, "analysis.gp", "Set the value of the GP register (MIPS)", core_config_get_integer_analysis_gp, core_config_set_integer_analysis_gp) &&
		rz_config_add_i_bind(cfg, "analysis.graph_depth", "Max depth for path search", core_config_get_integer_analysis_graph_depth, core_config_set_integer_analysis_graph_depth) &&
		rz_config_add_i_bind(cfg, "analysis.limits.from", "Sets the lower limit of the analysis range [analysis.limits.from : analysis.limits.to]", core_config_get_integer_analysis_limits_from, core_config_set_integer_analysis_limits_from) &&
		rz_config_add_i_bind(cfg, "analysis.limits.to", "Sets the upper limit of the analysis range [analysis.limits.from : analysis.limits.to]", core_config_get_integer_analysis_limits_to, core_config_set_integer_analysis_limits_to) &&
		rz_config_add_i_bind(cfg, "analysis.nonull", "Do not analyze regions of N null bytes (0 is disabled)", core_config_get_integer_analysis_nonull, core_config_set_integer_analysis_nonull) &&
		rz_config_add_l_bind(cfg, "analysis.roregs", "Comma separated list of register names to be readonly", core_config_get_list_analysis_roregs, core_config_set_list_analysis_roregs) &&
		rz_config_add_s_bind(cfg, "analysis.cc", "Specify default calling convention", core_config_get_string_analysis_cc, core_config_set_string_analysis_cc) &&
		rz_config_add_s_bind(cfg, "analysis.in", "Defines the search boundaries maps for analysis", core_config_get_string_analysis_in, core_config_set_string_analysis_in) &&
		rz_config_add_s_bind(cfg, "analysis.syscc", "Specify default syscall calling convention", core_config_get_string_analysis_syscc, core_config_set_string_analysis_syscc) &&
		rz_config_add_b_bind(cfg, "analysis.strings", "Identify and register strings during analysis (aar only)", core_config_get_bool_analysis_strings, core_config_set_bool_analysis_strings) &&
		rz_config_add_b_bind(cfg, "analysis.vars", "Analyze local variables and arguments", core_config_get_bool_analysis_vars, core_config_set_bool_analysis_vars) &&
		rz_config_add_b_bind(cfg, "analysis.norevisit", "Do not visit function analysis twice (experimental)", core_config_get_bool_analysis_norevisit, core_config_set_bool_analysis_norevisit) &&
		rz_config_add_b_bind(cfg, "analysis.nopskip", "Skip nops at the beginning of functions", core_config_get_bool_analysis_nopskip, core_config_set_bool_analysis_nopskip) &&

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
		rz_config_add_s(cfg, "asm.cmt.off", "Show offset comment in disasm (true, false, nodup)", "nodup") &&
		rz_config_add_s(cfg, "asm.highlight", "Highlight current line", "") &&
		rz_config_add_s(cfg, "asm.strip", "strip all instructions given comma separated types", "") &&

		/* set special options */
		rz_config_set_options(cfg, "analysis.in", rz_core_get_supported_search_maps()) &&
		/* end */
		true;
}
