// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-FileCopyrightText: 2009-2021 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

/**
 * \file: contains all the variables linked to screen/terminal
 */

/* scr.fgets GET */
static bool core_config_get_bool_scr_fgets(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	*value = core->cons->user_fgets == NULL;
	return true;
}

/* scr.fgets SET */
static bool core_config_set_bool_scr_fgets(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	core->cons->user_fgets = *value ? NULL : (void *)rz_core_fgets;
	core->cons->user_fgets_user = core;
	return true;
}

/* scr.utf8 GET */
static bool core_config_get_bool_scr_utf8(void *user, void *data) {
	bool *value = (bool *)data;

	*value = rz_cons_get_utf8();
	return true;
}

/* scr.utf8 SET */
static bool core_config_set_bool_scr_utf8(void *user, const void *data) {
	const bool *value = (const bool *)data;

	rz_cons_set_utf8(*value);
	return true;
}

/* scr.utf8.curvy GET */
static bool core_config_get_bool_scr_utf8_curvy(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	*value = core->cons->use_utf8_curvy;
	return true;
}

/* scr.utf8.curvy SET */
static bool core_config_set_bool_scr_utf8_curvy(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	core->cons->use_utf8_curvy = *value;
	return true;
}

/* scr.null GET */
static bool core_config_get_bool_scr_null(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	*value = core->cons->null;
	return true;
}

/* scr.null SET */
static bool core_config_set_bool_scr_null(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	core->cons->null = *value;
	return true;
}

/* scr.color.grep GET */
static bool core_config_get_bool_scr_color_grep(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	*value = core->cons->grep_color;
	return true;
}

/* scr.color.grep SET */
static bool core_config_set_bool_scr_color_grep(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	core->cons->grep_color = *value;
	return true;
}

/* scr.color GET */
static bool core_config_get_integer_scr_color(void *user, void *data) {
	ut64 *value = (ut64 *)data;

	*value = rz_cons_singleton()->context->color_mode;
	return true;
}

/* scr.color SET */
static bool core_config_set_integer_scr_color(void *user, const void *data) {
	const ut64 *value = (const ut64 *)data;

	if (*value > COLOR_MODE_16M) {
		RZ_LOG_ERROR("config: invalid color mode %" PFMT64u ". must be 0 (none) or 1 (ansi) or 2 (256 colors) or 3 (truecolor)\n", *value);
		return false;
	}

	rz_cons_singleton()->context->color_mode = *value;
	return true;
}

/* scr.prompt GET */
static bool core_config_get_bool_scr_prompt(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	*value = core->print->scr_prompt;
	return true;
}

/* scr.prompt SET */
static bool core_config_set_bool_scr_prompt(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	core->print->scr_prompt = *value;
	core->cons->line->echo = *value;
	return true;
}

/* scr.interactive GET */
static bool core_config_get_bool_scr_interactive(void *user, void *data) {
	bool *value = (bool *)data;

	*value = rz_cons_singleton()->context->is_interactive;
	return true;
}

/* scr.interactive SET */
static bool core_config_set_bool_scr_interactive(void *user, const void *data) {
	const bool *value = (const bool *)data;

	rz_cons_singleton()->context->is_interactive = *value;
	return true;
}

/* scr.echo GET */
static bool core_config_get_bool_scr_echo(void *user, void *data) {
	bool *value = (bool *)data;

	*value = rz_cons_singleton()->echo;
	return true;
}

/* scr.echo SET */
static bool core_config_set_bool_scr_echo(void *user, const void *data) {
	const bool *value = (const bool *)data;

	rz_cons_singleton()->echo = *value;
	return true;
}

/* scr.prompt.popup GET */
static bool core_config_get_bool_scr_prompt_popup(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	*value = core->cons->show_autocomplete_widget;
	return true;
}

/* scr.prompt.popup SET */
static bool core_config_set_bool_scr_prompt_popup(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	core->cons->show_autocomplete_widget = *value;
	return true;
}

/* scr.maxtab GET */
static bool core_config_get_integer_scr_maxtab(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	*value = core->cons->line->completion.args_limit;
	return true;
}

/* scr.maxtab SET */
static bool core_config_set_integer_scr_maxtab(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	if (*value > (1024 * 1024)) {
		core->cons->line->completion.args_limit = (1024 * 1024);
	} else if (*value < 1024) {
		core->cons->line->completion.args_limit = 1024;
	} else {
		core->cons->line->completion.args_limit = *value;
	}
	return true;
}

static bool core_config_add_scr_variables(RzCore *core) {
	RzConfig *cfg = core->config;

	// this should be initialized elsewhere?
	rz_cons_set_utf8(rz_cons_is_utf8());
	core->cons->use_utf8_curvy = false;
	core->cons->grep_color = false;
	rz_cons_singleton()->context->color_mode = (core->print->flags & RZ_PRINT_FLAGS_COLOR) ? COLOR_MODE_16 : COLOR_MODE_DISABLED;
	rz_cons_singleton()->context->is_interactive = true;
	core->print->scr_prompt = true;
	core->cons->line->echo = true;
	rz_cons_singleton()->echo = false;
	core->cons->line->completion.args_limit = 4096;

	return
		/* scr variables */
		rz_config_add_b(cfg, "scr.slow", "Do slow stuff on visual mode like RzFlag.get_at(true)", true) &&
		rz_config_add_b(cfg, "scr.wheel.nkey", "Use sn/sp and scr.nkey on wheel instead of scroll", false) &&
		rz_config_add_b(cfg, "scr.wheel", "Mouse wheel in Visual; temporaryly disable/reenable by right click/Enter)", true) &&
		rz_config_add_b(cfg, "scr.panelborder", "Specify panels border active area (0 by default)", false) &&
		rz_config_add_b(cfg, "scr.dumpcols", "Prefer pC commands before p ones", false) &&
		rz_config_add_b(cfg, "scr.randpal", "Random color palete or just get the next one from 'eco'", false) &&
		rz_config_add_b(cfg, "scr.prompt.file", "Show user prompt file (used by rizin -q)", false) &&
		rz_config_add_b(cfg, "scr.prompt.flag", "Show flag name in the prompt", false) &&
		rz_config_add_b(cfg, "scr.prompt.flag.only", "Show the flag name only in the prompt", false) &&
		rz_config_add_b(cfg, "scr.prompt.sect", "Show section name in the prompt", false) &&
		rz_config_add_b(cfg, "scr.hist.ruler", "Show histogram ruler", true) &&
		rz_config_add_b(cfg, "scr.color.pipe", "Enable colors when using pipes", false) &&
		rz_config_add_b(cfg, "scr.color.ops", "Colorize numbers and registers in opcodes", true) &&
		rz_config_add_b(cfg, "scr.color.args", "Colorize arguments and variables of functions", true) &&
		rz_config_add_b(cfg, "scr.color.bytes", "Colorize bytes that represent the opcodes of the instruction", true) &&
		rz_config_add_b(cfg, "scr.histsave", "Always save history on exit", true) &&
		rz_config_add_b(cfg, "scr.confirmquit", "Confirm on quit", false) &&
		rz_config_add_b(cfg, "scr.square", "Use square pixels or not", true) &&
		rz_config_add_i(cfg, "scr.feedback", "Set visual feedback level (1=arrow on jump, 2=every key (useful for videos))", 1) &&
		rz_config_add_i(cfg, "scr.scrollbar", "Show flagzone (fz) scrollbar in visual mode (0=no,1=right,2=top,3=bottom)", 0) &&

		/* cfg binds */
		rz_config_add_b_bind(cfg, "scr.color.grep", "Enable colors when using ~grep", core_config_get_bool_scr_color_grep, core_config_set_bool_scr_color_grep) &&
		rz_config_add_b_bind(cfg, "scr.echo", "Show rcons output in realtime to stderr and buffer", core_config_get_bool_scr_echo, core_config_set_bool_scr_echo) &&
		rz_config_add_b_bind(cfg, "scr.fgets", "Use fgets() instead of dietline for prompt input.", core_config_get_bool_scr_fgets, core_config_set_bool_scr_fgets) &&
		rz_config_add_b_bind(cfg, "scr.interactive", "Start in interactive mode", core_config_get_bool_scr_interactive, core_config_set_bool_scr_interactive) &&
		rz_config_add_b_bind(cfg, "scr.null", "Show no output.", core_config_get_bool_scr_null, core_config_set_bool_scr_null) &&
		rz_config_add_b_bind(cfg, "scr.prompt", "Show user prompt (used by rizin -q)", core_config_get_bool_scr_prompt, core_config_set_bool_scr_prompt) &&
		rz_config_add_b_bind(cfg, "scr.prompt.popup", "Show widget dropdown for autocomplete", core_config_get_bool_scr_prompt_popup, core_config_set_bool_scr_prompt_popup) &&
		rz_config_add_b_bind(cfg, "scr.utf8", "Show UTF-8 characters instead of ANSI.", core_config_get_bool_scr_utf8, core_config_set_bool_scr_utf8) &&
		rz_config_add_b_bind(cfg, "scr.utf8.curvy", "Show curved UTF-8 corners (requires scr.utf8).", core_config_get_bool_scr_utf8_curvy, core_config_set_bool_scr_utf8_curvy) &&
		rz_config_add_i_bind(cfg, "scr.color", "Enable colors (0: none, 1: ansi, 2: 256 colors, 3: truecolor)", core_config_get_integer_scr_color, core_config_set_integer_scr_color) &&
		rz_config_add_i_bind(cfg, "scr.maxtab", "Change max number of auto completion suggestions", core_config_get_integer_scr_maxtab, core_config_set_integer_scr_maxtab) &&

#if 0
	SETICB("scr.linesleep", 0, &cb_scrlinesleep, "Flush sleeping some ms in every line");
	SETICB("scr.maxtab", 4096, &cb_completion_maxtab, "Change max number of auto completion suggestions");
	SETICB("scr.pagesize", 1, &cb_scrpagesize, "Flush in pages when scr.linesleep is != 0");
	SETCB("scr.flush", "false", &cb_scrflush, "Force flush to console in realtime (breaks scripting)");
#if __WINDOWS__
	SETICB("scr.vtmode", rz_cons_singleton()->vtmode,
		&scr_vtmode, "Use VT sequences on Windows (0: Disable, 1: Output, 2: Input & Output)");
#endif
#if __ANDROID__
	SETBPREF("scr.responsive", "true", "Auto-adjust Visual depending on screen (e.g. unset asm.bytes)");
	SETI("scr.wheel.speed", 1, "Mouse wheel speed");
#else
	SETBPREF("scr.responsive", "false", "Auto-adjust Visual depending on screen (e.g. unset asm.bytes)");
	SETI("scr.wheel.speed", 4, "Mouse wheel speed");
#endif
	// RENAME TO scr.mouse
	SETPREF("scr.layout", "", "Name of the selected layout");
	// DEPRECATED: USES hex.cols now SETI ("scr.colpos", 80, "Column position of cmd.cprompt in visual");
	SETCB("scr.breakword", "", &cb_scrbreakword, "Emulate console break (^C) when a word is printed (useful for pD)");
	SETCB("scr.breaklines", "false", &cb_breaklines, "Break lines in Visual instead of truncating them");
	SETCB("scr.gadgets", "true", &cb_scr_gadgets, "Run pg in prompt, visual and panels");
	SETICB("scr.columns", 0, &cb_scrcolumns, "Force console column count (width)");
	SETCB("scr.rows", "0", &cb_scrrows, "Force console row count (height) ");
	SETICB("scr.rows", 0, &cb_rows, "Force console row count (height) (duplicate?)");
	SETICB("scr.fix.rows", 0, &cb_fixrows, "Workaround for Linux TTY");
	SETICB("scr.fix.columns", 0, &cb_fixcolumns, "Workaround for Prompt iOS SSH client");
	SETCB("scr.highlight", "", &cb_scrhighlight, "Highlight that word at RzCons level");
	SETCB("scr.bgfill", "false", &cb_scr_bgfill, "Fill background for ascii art when possible");
	SETCB("scr.html", "false", &cb_scrhtml, "Disassembly uses HTML syntax");
	n = NODECB("scr.nkey", "flag", &cb_scrnkey);
	SETDESC(n, "Select visual seek mode (affects n/N visual commands)");
	SETOPTIONS(n, "fun", "hit", "flag", NULL);
	SETCB("scr.pager", "", &cb_pager, "System program (or '..') to use when output exceeds screen boundaries");
	SETCB("scr.highlight.grep", "false", &cb_scr_color_grep_highlight, "Highlight (INVERT) the grepped words");
	SETCB("scr.hist.block", "true", &cb_scr_histblock, "Use blocks for histogram");
	
	SETCB("scr.tee", "", &cb_teefile, "Pipe output to file of this name");
	SETPREF("scr.seek", "", "Seek to the specified address on startup");
	n = NODECB("scr.strconv", "asciiesc", &cb_scrstrconv);
	SETDESC(n, "Convert string before display. asciiesc = Escape non-printable ASCII characters. asciidot = Replace non-printable ASCII characters with '.'.");
	SETOPTIONS(n, "asciiesc", "asciidot", NULL);
	SETICB("scr.visual.mode", RZ_CORE_VISUAL_MODE_PX, &cb_visual_mode, "Visual mode (0: hexdump, 1: disassembly, 2: debug, 3: color blocks, 4: strings)");

	SETCB("scr.prompt.vi", "false", &cb_scr_vi, "Use vi mode for input prompt");
	SETCB("scr.prompt.mode", "false", &cb_scr_prompt_mode, "Set prompt color based on vi mode");
	SETCB("scr.wideoff", "false", &cb_scr_wideoff, "Adjust offsets to match asm.bits");
	SETCB("scr.last", "true", &cb_scrlast, "Cache last output after flush to make _ command work (disable for performance)");

#endif
		true;
}
