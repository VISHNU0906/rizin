// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-FileCopyrightText: 2009-2021 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

/**
 * \file: contains all the variables linked to RzDebug
 */

#if __x86_64__
#define CORE_DBG_GLIBC_MA_OFFSET 0x000000
#define CORE_DBG_GLIBC_FC_OFFSET 0x00280
#else
#define CORE_DBG_GLIBC_MA_OFFSET 0x1bb000
#define CORE_DBG_GLIBC_FC_OFFSET 0x148
#endif

#if __GLIBC_MINOR__ > 25
#define CORE_DBG_GLIBC_TCACHE true
#else
#define CORE_DBG_GLIBC_TCACHE false
#endif

/* dbg.libs GET */
static bool core_config_get_string_dbg_libs(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	*value = core->dbg->glob_libs;
	return true;
}

/* dbg.libs SET */
static bool core_config_set_string_dbg_libs(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	free(core->dbg->glob_libs);
	core->dbg->glob_libs = rz_str_dup(*value);
	return true;
}

/* dbg.profile GET */
static bool core_config_get_string_dbg_profile(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	*value = core->io->runprofile;
	return true;
}

/* dbg.profile SET */
static bool core_config_set_string_dbg_profile(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	RZ_FREE(core->io->runprofile);
	if (RZ_STR_ISNOTEMPTY(*value)) {
		core->io->runprofile = rz_str_dup(*value);
	}
	return true;
}

static bool core_config_add_dbg_variables(RzCore *core) {
	RzConfig *cfg = core->config;
	return
		/* dbg variables */
		rz_config_add_s(cfg, "dbg.exe.path", "Path to binary being debugged", "") &&
		rz_config_add_o(cfg, "dbg.glibc.version", "Set glibc version for heap parsing (auto-detected if 'auto')", "auto", "2.23", "2.26", "2.27", "2.29", "2.30", "2.31", "2.32", "2.33", "2.34", "2.35", "2.36", "2.37", "2.38", "2.39", NULL) &&
		rz_config_add_o(cfg, "dbg.jemalloc.version", "Select jemalloc version for heap parsing (auto-detected if 'auto')", "auto", "4.5.0", "5.3.0", NULL) &&
		rz_config_add_o(cfg, "dbg.jemalloc.page_size", "Select page size for jemalloc heap parsing (auto-detected if 'auto')", "auto", "4k", "16k", "64k", NULL) &&
		rz_config_add_i(cfg, "dbg.glibc.ma_offset", "Main_arena offset from his symbol", CORE_DBG_GLIBC_MA_OFFSET) &&
		rz_config_add_i(cfg, "dbg.glibc.fc_offset", "First chunk offset from brk_start", CORE_DBG_GLIBC_FC_OFFSET) &&
		rz_config_add_i(cfg, "dbg.glibc.fastbinmax", "Upper bound on the number of fastbins printed", 10) &&
		rz_config_add_b(cfg, "dbg.glibc.tcache", "Set glib tcache parsing", CORE_DBG_GLIBC_TCACHE) &&
		rz_config_add_b(cfg, "dbg.hwbp", "Use hardware instead of software breakpoints by default", false) &&
		rz_config_add_b(cfg, "dbg.skipover", "Make dso perform a dss (same goes for esil and visual/graph", false) &&
		rz_config_add_b(cfg, "dbg.slow", "Show stack and regs in visual mode in a slow but verbose mode", false) &&
		rz_config_add_b(cfg, "dbg.funcarg", "Display arguments to function call in visual mode", false) &&
		rz_config_add_b(cfg, "dbg.status", "Set cmd.prompt to '.dr*' or '.dr*;drd;sr PC;pi 1;shu'", false) &&
		rz_config_add_b(cfg, "dbg.trace.inrange", "While tracing, avoid following calls outside specified range", false) &&
		rz_config_add_b(cfg, "dbg.bpsysign", "Ignore system breakpoints", false) &&
		rz_config_add_b(cfg, "dbg.rebase", "Rebase analysis/meta/comments/flags when reopening file in debugger", true) &&
		rz_config_add_b(cfg, "dbg.trace.libs", "Trace library code too", true) &&
		rz_config_add_b(cfg, "dbg.exitkills", "Kill process on exit", true) &&

		/* dbg binds */
		rz_config_add_s_bind(cfg, "dbg.libs", "If set stop when loading matching libname", core_config_get_string_dbg_libs, core_config_set_string_dbg_libs) &&
		rz_config_add_s_bind(cfg, "dbg.profile", "Path to RzRunProfile file", core_config_get_string_dbg_profile, core_config_set_string_dbg_profile) &&

		true;
}

#if 0


	SETCB("dbg.profile", "", &cb_runprofile, "Path to RzRunProfile file");
	SETCB("dbg.unlibs", "", &cb_dbg_unlibs, "If set stop when unloading matching libname");
	SETCB("dbg.verbose", "false", &cb_dbg_verbose, "Verbose debug output");

	SETCB("dbg.bpinmaps", "true", &cb_dbg_bpinmaps, "Activate breakpoints only if they are inside a valid map");
	SETCB("dbg.forks", "false", &cb_dbg_forks, "Stop execution if fork() is done (see dbg.threads)");
#if __WINDOWS__
	n = NODECB("dbg.btalgo", "default", &cb_dbg_btalgo);
#else
	n = NODECB("dbg.btalgo", "fuzzy", &cb_dbg_btalgo);
#endif
	SETDESC(n, "Select backtrace algorithm");
	SETOPTIONS(n, "default", "fuzzy", "analysis", "trace", NULL);
	SETCB("dbg.threads", "false", &cb_stopthreads, "Stop all threads when debugger breaks (see dbg.forks)");
	SETCB("dbg.clone", "false", &cb_dbg_clone, "Stop execution if new thread is created");
	SETCB("dbg.aftersyscall", "true", &cb_dbg_aftersc, "Stop execution before the syscall is executed (see dcs)");
	SETCB("dbg.args", "", &cb_dbg_args, "Set the args of the program to debug");
	SETCB("dbg.follow.child", "false", &cb_dbg_follow_child, "Continue tracing the child process on fork. By default the parent process is traced");
	SETCB("dbg.trace_continue", "true", &cb_dbg_trace_continue, "Trace every instruction between the initial PC position and the PC position at the end of continue's execution");
	SETCB("dbg.create_new_console", "true", &cb_dbg_create_new_console, "Create a new console window for the debugee on debug start");
	/* debug */
#if DEBUGGER
	SETCB("dbg.backend", "native", &cb_dbgbackend, "Select the debugger backend");
#else
	SETCB("dbg.backend", "esil", &cb_dbgbackend, "Select the debugger backend");
#endif
	n = NODECB("dbg.bep", "loader", &cb_dbgbep);
	SETDESC(n, "Break on entrypoint");
	SETOPTIONS(n, "loader", "entry", "constructor", "main", NULL);

	if (core->cons->rows > 30) { // HACKY
		rz_config_set_i(cfg, "dbg.follow", 64);
	} else {
		rz_config_set_i(cfg, "dbg.follow", 32);
	}
	rz_config_desc(cfg, "dbg.follow", "Follow program counter when pc >= core->offset + dbg.follow");

	SETCB("dbg.execs", "false", &cb_dbg_execs, "Stop execution if new thread is created");
	SETICB("dbg.gdb.page_size", 4096, &cb_dbg_gdb_page_size, "Page size on gdb target (useful for QEMU)");
	SETICB("dbg.gdb.retries", 10, &cb_dbg_gdb_retries, "Number of retries before gdb packet read times out");
	SETCB("dbg.consbreak", "false", &cb_consbreak, "SIGINT handle for attached processes");

	SETCB("dbg.swstep", "false", &cb_swstep, "Force use of software steps (code analysis+breakpoint)");
	rz_config_set_getter(cfg, "dbg.swstep", (RzConfigCallback)__dbg_swstep_getter);

	SETICB("dbg.btdepth", 128, &cb_dbgbtdepth, "Depth of backtrace");
	SETCB("dbg.trace", "false", &cb_trace, "Trace program execution (see asm.trace)");
	SETICB("dbg.trace.tag", 0, &cb_tracetag, "Trace tag");

#endif