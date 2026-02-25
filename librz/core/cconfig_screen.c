// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-FileCopyrightText: 2009-2021 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

/**
 * \file: contains all the variables linked to screen/terminal
 */

#if __ANDROID__
#define CORE_CONFIG_SCR_RESPONSIVE  true
#define CORE_CONFIG_SCR_WHEEL_SPEED 1
#else
#define CORE_CONFIG_SCR_RESPONSIVE  false
#define CORE_CONFIG_SCR_WHEEL_SPEED 4
#endif

#if __WINDOWS__
/* scr.vtmode GET */
static bool core_config_get_integer_scr_vtmode(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	*value = core->cons->vtmode;
	return true;
}

/* scr.vtmode SET */
static bool core_config_set_integer_scr_vtmode(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	core->cons->vtmode = *value;
	return true;
}
#endif

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
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	*value = core->cons->context->color_mode;
	return true;
}

/* scr.color SET */
static bool core_config_set_integer_scr_color(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	if (*value > COLOR_MODE_16M) {
		RZ_LOG_ERROR("config: invalid color mode %" PFMT64u ". must be 0 (none) or 1 (ansi) or 2 (256 colors) or 3 (truecolor)\n", *value);
		return false;
	}

	core->cons->context->color_mode = *value;
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
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	*value = core->cons->context->is_interactive;
	return true;
}

/* scr.interactive SET */
static bool core_config_set_bool_scr_interactive(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	core->cons->context->is_interactive = *value;
	return true;
}

/* scr.echo GET */
static bool core_config_get_bool_scr_echo(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	*value = core->cons->echo;
	return true;
}

/* scr.echo SET */
static bool core_config_set_bool_scr_echo(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	core->cons->echo = *value;
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

/* scr.linesleep GET */
static bool core_config_get_integer_scr_linesleep(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	*value = core->cons->linesleep;
	return true;
}

/* scr.linesleep SET */
static bool core_config_set_integer_scr_linesleep(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	core->cons->linesleep = *value;
	return true;
}

/* scr.pagesize GET */
static bool core_config_get_integer_scr_pagesize(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	*value = core->cons->pagesize;
	return true;
}

/* scr.pagesize SET */
static bool core_config_set_integer_scr_pagesize(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	core->cons->pagesize = *value;
	return true;
}

/* scr.rows GET */
static bool core_config_get_integer_scr_rows(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	*value = core->cons->force_rows;
	return true;
}

/* scr.rows SET */
static bool core_config_set_integer_scr_rows(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	core->cons->force_rows = *value;
	rz_cons_get_size(NULL);
	return true;
}

/* scr.columns GET */
static bool core_config_get_integer_scr_columns(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	ut64 *value = (ut64 *)data;

	*value = core->cons->force_columns;
	return true;
}

/* scr.columns SET */
static bool core_config_set_integer_scr_columns(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const ut64 *value = (const ut64 *)data;

	core->cons->force_columns = *value;
	core->dbg->regcols = *value / 20;
	rz_cons_get_size(NULL);
	return true;
}

/* scr.flush GET */
static bool core_config_get_bool_scr_flush(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	*value = core->cons->flush;
	return true;
}

/* scr.flush SET */
static bool core_config_set_bool_scr_flush(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	core->cons->flush = *value;
	return true;
}

/* scr.visual.mode GET */
static bool core_config_get_string_scr_visual_mode(void *user, void *data) {
	const char **value = (const char **)data;
	RzCore *core = (RzCore *)user;
	const RzCoreVisual *visual = core->visual;

	switch (visual->printidx) {
	case RZ_CORE_VISUAL_MODE_PX:
		*value = "hexdump";
		break;
	case RZ_CORE_VISUAL_MODE_PD:
		*value = "disassembly";
		break;
	case RZ_CORE_VISUAL_MODE_DB:
		*value = "debug";
		break;
	case RZ_CORE_VISUAL_MODE_OV:
		*value = "colorblocks";
		break;
	case RZ_CORE_VISUAL_MODE_CD:
		*value = "strings";
		break;
	default:
		RZ_LOG_ERROR("core: scr.visual.mode invalid value '%d'\n", visual->printidx);
		return false;
	}

	return true;
}

/* scr.visual.mode SET */
static bool core_config_set_string_scr_visual_mode(void *user, const void *data) {
	const char **value = (const char **)data;
	RzCore *core = (RzCore *)user;
	RzCoreVisual *visual = core->visual;

	if (RZ_STR_EQ(*value, "hexdump")) {
		visual->printidx = RZ_CORE_VISUAL_MODE_PX;
	} else if (RZ_STR_EQ(*value, "disassembly")) {
		visual->printidx = RZ_CORE_VISUAL_MODE_PD;
	} else if (RZ_STR_EQ(*value, "debug")) {
		visual->printidx = RZ_CORE_VISUAL_MODE_DB;
	} else if (RZ_STR_EQ(*value, "color blocks")) {
		visual->printidx = RZ_CORE_VISUAL_MODE_OV;
	} else if (RZ_STR_EQ(*value, "strings")) {
		visual->printidx = RZ_CORE_VISUAL_MODE_CD;
	} else {
		RZ_LOG_ERROR("core: scr.visual.mode invalid value '%s'\n", *value);
		return false;
	}

	return true;
}

/* scr.breakword GET */
static bool core_config_get_string_scr_breakword(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	*value = core->cons->break_word;
	return true;
}

/* scr.breakword SET */
static bool core_config_set_string_scr_breakword(void *user, const void *data) {
	const char **value = (const char **)data;
	if (RZ_STR_ISNOTEMPTY(*value)) {
		rz_cons_breakword(*value);
	} else {
		rz_cons_breakword(NULL);
	}
	return true;
}

/* scr.highlight GET */
static bool core_config_get_string_scr_highlight(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	*value = core->cons->highlight;
	return true;
}

/* scr.highlight SET */
static bool core_config_set_string_scr_highlight(void *user, const void *data) {
	const char **value = (const char **)data;

	rz_cons_highlight(*value);
	return true;
}

/* scr.pager GET */
static bool core_config_get_string_scr_pager(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	*value = core->cons->pager;
	return true;
}

/* scr.pager SET */
static bool core_config_set_string_scr_pager(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	free(core->cons->pager);
	core->cons->pager = rz_str_dup(*value);
	return true;
}

/* scr.tee GET */
static bool core_config_get_string_scr_tee(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	*value = core->cons->teefile;
	return true;
}

/* scr.tee SET */
static bool core_config_set_string_scr_tee(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	free(core->cons->teefile);
	core->cons->teefile = rz_str_dup(*value);
	return true;
}

/* scr.strconv GET */
static bool core_config_get_string_scr_strconv(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	*value = core->print->strconv_mode;
	return true;
}

/* scr.strconv SET */
static bool core_config_set_string_scr_strconv(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	free(core->print->strconv_mode);
	core->print->strconv_mode = rz_str_dup(*value);
	return true;
}

/* scr.breaklines GET */
static bool core_config_get_bool_scr_breaklines(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzCons
	*value = core->cons->break_lines;
	return true;
}

/* scr.breaklines SET */
static bool core_config_set_bool_scr_breaklines(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create get function in RzCons
	core->cons->break_lines = *value;
	return true;
}

/* scr.bgfill GET */
static bool core_config_get_bool_scr_bgfill(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	*value = rz_print_has_flags(core->print, RZ_PRINT_FLAGS_BGFILL);
	return true;
}

/* scr.bgfill SET */
static bool core_config_set_bool_scr_bgfill(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	if (*value) {
		rz_print_add_flags(core->print, RZ_PRINT_FLAGS_BGFILL);
	} else {
		rz_print_remove_flags(core->print, RZ_PRINT_FLAGS_BGFILL);
	}
	return true;
}

/* scr.html GET */
static bool core_config_get_bool_scr_html(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzCons
	*value = core->cons->is_html;
	return true;
}

/* scr.html SET */
static bool core_config_set_bool_scr_html(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create get function in RzCons
	core->cons->is_html = *value;
	return true;
}

/* scr.highlight.grep GET */
static bool core_config_get_bool_scr_highlight_grep(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzCons
	*value = core->cons->grep_highlight;
	return true;
}

/* scr.highlight.grep SET */
static bool core_config_set_bool_scr_highlight_grep(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create get function in RzCons
	core->cons->grep_highlight = *value;
	return true;
}

/* scr.prompt.vi GET */
static bool core_config_get_bool_scr_prompt_vi(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzCons
	*value = core->cons->line->enable_vi_mode;
	return true;
}

/* scr.prompt.vi SET */
static bool core_config_set_bool_scr_prompt_vi(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create get function in RzCons
	core->cons->line->enable_vi_mode = *value;
	return true;
}

/* scr.prompt.mode GET */
static bool core_config_get_bool_scr_prompt_mode(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzCons
	*value = core->cons->line->prompt_mode;
	return true;
}

/* scr.prompt.mode SET */
static bool core_config_set_bool_scr_prompt_mode(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create get function in RzCons
	core->cons->line->prompt_mode = *value;
	return true;
}

/* scr.wideoff GET */
static bool core_config_get_bool_scr_wideoff(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzPrint
	*value = core->print->wide_offsets;
	return true;
}

/* scr.wideoff SET */
static bool core_config_set_bool_scr_wideoff(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create get function in RzPrint
	core->print->wide_offsets = *value;
	return true;
}

/* scr.last GET */
static bool core_config_get_bool_scr_last(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	// TODO: create get function in RzPrint
	*value = core->cons->context->last_enabled;
	return true;
}

/* scr.last SET */
static bool core_config_set_bool_scr_last(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	// TODO: create get function in RzPrint
	core->cons->context->last_enabled = *value;
	return true;
}

static bool core_config_add_scr_variables(RzCore *core) {
	RzConfig *cfg = core->config;

	// this should be initialized elsewhere?
	rz_cons_set_utf8(rz_cons_is_utf8());
	// must initialize the console WxH
	rz_cons_get_size(NULL);
	if (core->print->flags & RZ_PRINT_FLAGS_COLOR) {
		core->cons->context->color_mode = COLOR_MODE_16;
	} else {
		core->cons->context->color_mode = COLOR_MODE_DISABLED;
	}

	return
#if __WINDOWS__
		/* scr windows only binds */
		rz_config_add_i_bind(cfg, "scr.vtmode", "Use VT sequences on Windows (0: Disable, 1: Output, 2: Input & Output)", core_config_get_integer_scr_vtmode, core_config_set_integer_scr_vtmode) &&
#endif

		/* scr variables */
		rz_config_add_b(cfg, "scr.color.args", "Colorize arguments and variables of functions", true) &&
		rz_config_add_b(cfg, "scr.color.bytes", "Colorize bytes that represent the opcodes of the instruction", true) &&
		rz_config_add_b(cfg, "scr.color.ops", "Colorize numbers and registers in opcodes", true) &&
		rz_config_add_b(cfg, "scr.color.pipe", "Enable colors when using pipes", false) &&
		rz_config_add_b(cfg, "scr.confirmquit", "Confirm on quit", false) &&
		rz_config_add_b(cfg, "scr.dumpcols", "Prefer pC commands before p ones", false) &&
		rz_config_add_b(cfg, "scr.hist.ruler", "Show histogram ruler", true) &&
		rz_config_add_b(cfg, "scr.histsave", "Always save history on exit", true) &&
		rz_config_add_b(cfg, "scr.panelborder", "Specify panels border active area (0 by default)", false) &&
		rz_config_add_b(cfg, "scr.prompt.file", "Show user prompt file (used by rizin -q)", false) &&
		rz_config_add_b(cfg, "scr.prompt.flag", "Show flag name in the prompt", false) &&
		rz_config_add_b(cfg, "scr.prompt.flag.only", "Show the flag name only in the prompt", false) &&
		rz_config_add_b(cfg, "scr.prompt.sect", "Show section name in the prompt", false) &&
		rz_config_add_b(cfg, "scr.randpal", "Random color palete or just get the next one from 'eco'", false) &&
		rz_config_add_b(cfg, "scr.responsive", "Auto-adjust Visual depending on screen (e.g. unset asm.bytes)", CORE_CONFIG_SCR_RESPONSIVE) &&
		rz_config_add_b(cfg, "scr.slow", "Do slow stuff on visual mode like RzFlag.get_at(true)", true) &&
		rz_config_add_b(cfg, "scr.square", "Use square pixels or not", true) &&
		rz_config_add_b(cfg, "scr.wheel", "Mouse wheel in Visual; temporaryly disable/reenable by right click/Enter)", true) &&
		rz_config_add_b(cfg, "scr.wheel.nkey", "Use sn/sp and scr.nkey on wheel instead of scroll", false) &&
		rz_config_add_i(cfg, "scr.feedback", "Set visual feedback level (1=arrow on jump, 2=every key (useful for videos))", 1) &&
		rz_config_add_i(cfg, "scr.scrollbar", "Show flagzone (fz) scrollbar in visual mode (0=no,1=right,2=top,3=bottom)", 0) &&
		rz_config_add_i(cfg, "scr.wheel.speed", "Mouse wheel speed", CORE_CONFIG_SCR_WHEEL_SPEED) &&
		rz_config_add_s(cfg, "scr.layout", "Name of the selected layout", "") &&
		rz_config_add_s(cfg, "scr.seek", "Seek to the specified address on startup", "") &&
		rz_config_add_o(cfg, "scr.nkey", "Select visual seek mode (affects n/N visual commands)", "flag", "fun", "hit", NULL) &&

		/* scr binds */
		rz_config_add_b_bind(cfg, "scr.bgfill", "Fill background for ascii art when possible", core_config_get_bool_scr_bgfill, core_config_set_bool_scr_bgfill) &&
		rz_config_add_b_bind(cfg, "scr.breaklines", "Break lines in Visual instead of truncating them", core_config_get_bool_scr_breaklines, core_config_set_bool_scr_breaklines) &&
		rz_config_add_b_bind(cfg, "scr.color.grep", "Enable colors when using ~grep", core_config_get_bool_scr_color_grep, core_config_set_bool_scr_color_grep) &&
		rz_config_add_b_bind(cfg, "scr.echo", "Show rcons output in realtime to stderr and buffer", core_config_get_bool_scr_echo, core_config_set_bool_scr_echo) &&
		rz_config_add_b_bind(cfg, "scr.fgets", "Use fgets() instead of dietline for prompt input.", core_config_get_bool_scr_fgets, core_config_set_bool_scr_fgets) &&
		rz_config_add_b_bind(cfg, "scr.flush", "Force flush to console in realtime (breaks scripting)", core_config_get_bool_scr_flush, core_config_set_bool_scr_flush) &&
		rz_config_add_b_bind(cfg, "scr.highlight.grep", "Highlight (INVERT) the grepped words", core_config_get_bool_scr_highlight_grep, core_config_set_bool_scr_highlight_grep) &&
		rz_config_add_b_bind(cfg, "scr.html", "Disassembly uses HTML syntax", core_config_get_bool_scr_html, core_config_set_bool_scr_html) &&
		rz_config_add_b_bind(cfg, "scr.interactive", "Start in interactive mode", core_config_get_bool_scr_interactive, core_config_set_bool_scr_interactive) &&
		rz_config_add_b_bind(cfg, "scr.last", "Cache last output after flush to make _ command work (disable for performance)", core_config_get_bool_scr_last, core_config_set_bool_scr_last) &&
		rz_config_add_b_bind(cfg, "scr.null", "Show no output.", core_config_get_bool_scr_null, core_config_set_bool_scr_null) &&
		rz_config_add_b_bind(cfg, "scr.prompt", "Show user prompt (used by rizin -q)", core_config_get_bool_scr_prompt, core_config_set_bool_scr_prompt) &&
		rz_config_add_b_bind(cfg, "scr.prompt.mode", "Set prompt color based on vi mode", core_config_get_bool_scr_prompt_mode, core_config_set_bool_scr_prompt_mode) &&
		rz_config_add_b_bind(cfg, "scr.prompt.popup", "Show widget dropdown for autocomplete", core_config_get_bool_scr_prompt_popup, core_config_set_bool_scr_prompt_popup) &&
		rz_config_add_b_bind(cfg, "scr.prompt.vi", "Use vi mode for input prompt", core_config_get_bool_scr_prompt_vi, core_config_set_bool_scr_prompt_vi) &&
		rz_config_add_b_bind(cfg, "scr.utf8", "Show UTF-8 characters instead of ANSI.", core_config_get_bool_scr_utf8, core_config_set_bool_scr_utf8) &&
		rz_config_add_b_bind(cfg, "scr.utf8.curvy", "Show curved UTF-8 corners (requires scr.utf8).", core_config_get_bool_scr_utf8_curvy, core_config_set_bool_scr_utf8_curvy) &&
		rz_config_add_b_bind(cfg, "scr.wideoff", "Adjust offsets to match asm.bits", core_config_get_bool_scr_wideoff, core_config_set_bool_scr_wideoff) &&
		rz_config_add_i_bind(cfg, "scr.color", "Enable colors (0: none, 1: ansi, 2: 256 colors, 3: truecolor)", core_config_get_integer_scr_color, core_config_set_integer_scr_color) &&
		rz_config_add_i_bind(cfg, "scr.columns", "Force console max columns/width count (when 0 is automatic)", core_config_get_integer_scr_columns, core_config_set_integer_scr_columns) &&
		rz_config_add_i_bind(cfg, "scr.linesleep", "Flush sleeping some ms in every line", core_config_get_integer_scr_linesleep, core_config_set_integer_scr_linesleep) &&
		rz_config_add_i_bind(cfg, "scr.maxtab", "Change max number of auto completion suggestions", core_config_get_integer_scr_maxtab, core_config_set_integer_scr_maxtab) &&
		rz_config_add_i_bind(cfg, "scr.pagesize", "Flush in pages when scr.linesleep is != 0", core_config_get_integer_scr_pagesize, core_config_set_integer_scr_pagesize) &&
		rz_config_add_i_bind(cfg, "scr.rows", "Force console max rows/height count (when 0 is automatic)", core_config_get_integer_scr_rows, core_config_set_integer_scr_rows) &&
		rz_config_add_s_bind(cfg, "scr.breakword", "Emulate console break (^C) when a word is printed (useful for pD)", core_config_get_string_scr_breakword, core_config_set_string_scr_breakword) &&
		rz_config_add_s_bind(cfg, "scr.highlight", "Highlight that word at RzCons level", core_config_get_string_scr_highlight, core_config_set_string_scr_highlight) &&
		rz_config_add_s_bind(cfg, "scr.pager", "System program (or '..') to use when output exceeds screen boundaries", core_config_get_string_scr_pager, core_config_set_string_scr_pager) &&
		rz_config_add_s_bind(cfg, "scr.strconv", "Convert string before display. asciiesc = Escape non-printable ASCII characters. asciidot = Replace non-printable ASCII characters with '.'.", core_config_get_string_scr_strconv, core_config_set_string_scr_strconv) &&
		rz_config_add_s_bind(cfg, "scr.tee", "Pipe output to file of this name", core_config_get_string_scr_tee, core_config_set_string_scr_tee) &&
		rz_config_add_s_bind(cfg, "scr.visual.mode", "Current visual mode", core_config_get_string_scr_visual_mode, core_config_set_string_scr_visual_mode) &&
		rz_config_set_options2(cfg, "scr.strconv", "asciiesc", "asciidot", NULL) &&
		rz_config_set_options2(cfg, "scr.visual.mode", "hexdump", "disassembly", "debug", "colorblocks", "strings", NULL);
}
