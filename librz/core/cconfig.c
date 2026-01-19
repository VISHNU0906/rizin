// SPDX-FileCopyrightText: 2009-2021 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

#include <rz_core.h>
#include <rz_cons.h>
#include <rz_basefind.h>
#include <rz_th.h>
#include <rz_windows.h>
#include <rz_config.h>
#include <rz_util/rz_bits.h>
#include <rz_util/rz_str.h>
#include <rz_util/rz_bits.h>

#include "core_private.h"

#include "cconfig_arch.c"

/*

typedef struct config_opt_descr {
	const char *option;
	const char *description;
} ConfigOptDescr;

static bool boolify_var_cb(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value || rz_str_is_false(node->value)) {
		free(node->value);
		node->value = rz_str_dup(rz_str_bool(node->i_value));
	}
	return true;
}

static void set_options(RzConfigNode *node, ...) {
	va_list argp;
	char *option = NULL;
	va_start(argp, node);
	option = va_arg(argp, char *);
	while (option) {
		rz_list_append(node->options, option);
		option = va_arg(argp, char *);
	}
	va_end(argp);
}

*/

static bool isGdbPlugin(RzCore *core) {
	if (core->io && core->io->desc && core->io->desc->plugin) {
		if (core->io->desc->plugin->name && !strcmp(core->io->desc->plugin->name, "gdb")) {
			return true;
		}
	}
	return false;
}

static void print_node_options(RzConfigNode *node) {
	RzListIter *iter;
	char *option;
	rz_list_foreach (node->options, iter, option) {
		rz_cons_printf("%s\n", option);
	}
}

static int compareName(const RzAnalysisFunction *a, const RzAnalysisFunction *b, void *user) {
	return (a && b && a->name && b->name ? strcmp(a->name, b->name) : 0);
}

static int compareNameLen(const RzAnalysisFunction *a, const RzAnalysisFunction *b, void *user) {
	size_t la, lb;
	if (!a || !b || !a->name || !b->name) {
		return 0;
	}
	la = strlen(a->name);
	lb = strlen(a->name);
	return (la > lb) - (la < lb);
}

static int compareAddress(const RzAnalysisFunction *a, const RzAnalysisFunction *b, void *user) {
	return (a && b && a->addr && b->addr ? (a->addr > b->addr) - (a->addr < b->addr) : 0);
}

static int compareSize(const RzAnalysisFunction *a, const RzAnalysisFunction *b, void *user) {
	ut64 sa, sb;
	// return a && b && a->_size < b->_size;
	if (!a || !b) {
		return 0;
	}
	sa = rz_analysis_function_realsize(a);
	sb = rz_analysis_function_realsize(b);
	return (sa > sb) - (sa < sb);
}

static void update_asmarch_options(RzCore *core, RzConfigNode *node) {
	RzIterator *it = ht_sp_as_iter(core->rasm->plugins);
	RzAsmPlugin **val;
	if (core && node && core->rasm) {
		rz_list_purge(node->options);
		rz_iterator_foreach(it, val) {
			RzAsmPlugin *h = *val;
			SETOPTIONS(node, h->name, NULL);
		}
	}
	rz_iterator_free(it);
}

static void update_asmbits_options(RzCore *core, RzConfigNode *node) {
	if (core && core->rasm && core->rasm->cur && node) {
		int bits = core->rasm->cur->bits;
		int i;
		node->options->free = free;
		rz_list_purge(node->options);
		for (i = 1; i <= bits; i <<= 1) {
			if (i & bits) {
				SETOPTIONS(node, rz_str_newf("%d", i), NULL);
			}
		}
	}
}

static void update_asmfeatures_options(RzCore *core, RzConfigNode *node) {
	int i, argc;

	if (core && core->rasm && core->rasm->cur) {
		if (core->rasm->cur->features) {
			char *features = rz_str_dup(core->rasm->cur->features);
			rz_list_purge(node->options);
			argc = rz_str_split(features, ',');
			for (i = 0; i < argc; i++) {
				node->options->free = free;
				const char *feature = rz_str_word_get0(features, i);
				if (feature) {
					rz_list_append(node->options, rz_str_dup(feature));
				}
			}
			free(features);
		}
	}
}

static void update_asmplatforms_options(RzCore *core, RzConfigNode *node) {
	int i, argc;

	if (core && core->rasm && core->rasm->cur) {
		if (core->rasm->cur->platforms) {
			char *platforms = rz_str_dup(core->rasm->cur->platforms);
			rz_list_purge(node->options);
			argc = rz_str_split(platforms, ',');
			for (i = 0; i < argc; i++) {
				node->options->free = free;
				const char *feature = rz_str_word_get0(platforms, i);
				if (feature) {
					rz_list_append(node->options, rz_str_dup(feature));
				}
			}
			free(platforms);
		}
	}
}

static void update_asmparser_options(RzCore *core, RzConfigNode *node) {
	RzListIter *iter;
	RzParsePlugin *parser;
	if (core && node && core->parser && core->parser->parsers) {
		rz_list_purge(node->options);
		rz_list_foreach (core->parser->parsers, iter, parser) {
			SETOPTIONS(node, parser->name, NULL);
		}
	}
}

static void update_asmcpu_options(RzCore *core, RzConfigNode *node) {
	rz_return_if_fail(core && core->rasm);

	RzIterator *it = ht_sp_as_iter(core->rasm->plugins);
	RzAsmPlugin **val;
	const char *arch = rz_config_get(core->config, "asm.arch");
	if (!arch || !*arch) {
		return;
	}
	rz_list_purge(node->options);
	rz_iterator_foreach(it, val) {
		RzAsmPlugin *h = *val;
		if (h->cpus && !strcmp(arch, h->name)) {
			char *c = rz_str_dup(h->cpus);
			int i, n = rz_str_split(c, ',');
			for (i = 0; i < n; i++) {
				const char *word = rz_str_word_get0(c, i);
				if (word && *word) {
					node->options->free = free;
					SETOPTIONS(node, rz_str_dup(word), NULL);
				}
			}
			free(c);
		}
	}
	rz_iterator_free(it);
}

static bool cb_search_case_sensitive(void *_core, void *_node) {
	RzConfigNode *node = _node;
	const char *case_sensitive = node->value;
	RzCore *core = _core;
	if (case_sensitive) {
		if (!strcmp(case_sensitive, "smart")) {
			core->cons->grep_icase = RZ_CONS_SEARCH_CASE_SMART;
		} else if (!strcmp(case_sensitive, "insensitive")) {
			core->cons->grep_icase = RZ_CONS_SEARCH_CASE_INSENSITIVE;
		} else if (!strcmp(case_sensitive, "sensitive")) {
			core->cons->grep_icase = RZ_CONS_SEARCH_CASE_SENSITIVE;
		} else {
			goto err;
		}
		return true;
	}
err:
	RZ_LOG_ERROR("grep: search.sensitive: invalid value (%s), support only `smart, sensitive, insensitive`\n", case_sensitive);
	return false;
}

static bool cb_diff_sort(void *_core, void *_node) {
	RzConfigNode *node = _node;
	const char *column = node->value;
	RzCore *core = _core;
	if (column && strcmp(column, "?")) {
		if (!strcmp(column, "name")) {
			core->analysis->columnSort = (RzListComparator)compareName;
		} else if (!strcmp(column, "namelen")) {
			core->analysis->columnSort = (RzListComparator)compareNameLen;
		} else if (!strcmp(column, "addr")) {
			core->analysis->columnSort = (RzListComparator)compareAddress;
		} else if (!strcmp(column, "size")) {
			core->analysis->columnSort = (RzListComparator)compareSize;
		} else {
			goto fail;
		}
		return true;
	}
fail:
	RZ_LOG_ERROR("core: diff.sort: invalid value (%s), supported only `name, namelen, addr, type, size, dist`\n", column);
	return false;
}

static bool cb_asm_features_set(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (*node->value == '?') {
		update_asmfeatures_options(core, node);
		print_node_options(node);
		return 0;
	}
	RZ_FREE(core->rasm->features);
	if (node->value[0]) {
		core->rasm->features = rz_str_dup(node->value);
	}
	return 1;
}

static bool cb_asm_parser_set(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->value[0] == '?') {
		update_asmparser_options(core, node);
		print_node_options(node);
		return false;
	}

	return rz_parse_use(core->parser, node->value);
}

static bool cb_asm_os_set(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;

	const char *value = node->value;
	if (*value == '?') {
		print_node_options(node);
		return 0;
	}
	if (!value[0]) {
		value = RZ_SYS_OS;
	}
	return rz_core_arch_configure(core, /*arch*/ NULL, /*bits*/ 0, /*cpu*/ NULL, /*os*/ value, /*platform*/ NULL);
}

static bool cb_asm_cpu_set(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	const char *value = node->value;
	if (*value == '?') {
		update_asmcpu_options(core, node);
		/* print verbose help instead of plain option listing */
		RzCmdStateOutput state = { 0 };
		rz_cmd_state_output_init(&state, RZ_OUTPUT_MODE_STANDARD, core);
		rz_core_asm_cpu_plugin_print(core, &state, rz_config_get(core->config, "asm.arch"));
		rz_cmd_state_output_print(&state);
		rz_cmd_state_output_fini(&state);
		return 0;
	}

	return rz_core_arch_configure(core, /*arch*/ NULL, /*bits*/ 0, /*cpu*/ value, /*os*/ NULL, /*platform*/ NULL);
}

static bool cb_asm_arch_set(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	const char *value = node->value;
	if (value[0] == '?') {
		update_asmarch_options(core, node);
		if (strlen(value) > 1 && value[1] == '?') {
			/* print more verbose help instead of plain option values */
			RzCmdStateOutput state = { 0 };
			rz_cmd_state_output_init(&state, RZ_OUTPUT_MODE_STANDARD, core);
			rz_core_asm_plugins_print(core, &state, NULL);
			rz_cmd_state_output_print(&state);
			rz_cmd_state_output_fini(&state);
			return false;
		} else {
			print_node_options(node);
			return false;
		}
	}

	return rz_core_arch_configure(core, /*arch*/ value, /*bits*/ 0, /*cpu*/ NULL, /*os*/ NULL, /*platform*/ NULL);
}

static bool cb_asm_platform_set(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	const char *value = node->value;
	if (*value == '?') {
		update_asmplatforms_options(core, node);
		print_node_options(node);
		return 0;
	}

	return rz_core_arch_configure(core, /*arch*/ NULL, /*bits*/ 0, /*cpu*/ NULL, /*os*/ NULL, /*platform*/ value);
}

static bool cb_asm_bits_set(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;

	int value = node->i_value;
	if (node->value[0] == '?') {
		update_asmbits_options(core, node);
		print_node_options(node);
		return false;
	}

	return rz_core_arch_configure(core, /*arch*/ NULL, /*bits*/ value, /*cpu*/ NULL, /*os*/ NULL, /*platform*/ NULL);
}

static bool cb_debug_hitinfo(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->dbg->hitinfo = node->i_value;
	return true;
}

static bool cb_analysis_jmpretpoline(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.retpoline = node->i_value;
	return true;
}

static bool cb_analysis_armthumb(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.armthumb = node->i_value;
	return true;
}

static bool cb_analysis_depth(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.depth = node->i_value;
	return true;
}

static bool cb_analysis_graphdepth(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.graph_depth = node->i_value;
	return true;
}

static bool cb_analysis_afterjmp(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.afterjmp = node->i_value;
	return true;
}

static bool cb_analysis_aftertrap(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.aftertrap = node->i_value;
	return true;
}

static bool cb_analysis_delay(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.delay = node->i_value;
	return true;
}

static bool cb_analysis_vars(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.vars = node->i_value;
	return true;
}

static bool cb_analysis_nonull(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.nonull = node->i_value;
	return true;
}

static bool cb_analysis_strings(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		rz_config_set(core->config, "bin.strings", "false");
	}
	return true;
}

static bool cb_analysis_ignbithints(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.ignbithints = node->i_value;
	return true;
}

static bool cb_analysis_sleep(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->sleep = node->i_value;
	return true;
}

static bool cb_analysis_maxrefs(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->maxreflines = node->i_value;
	return true;
}

static bool cb_analysis_norevisit(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.norevisit = node->i_value;
	return true;
}

static bool cb_analysis_nopskip(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.nopskip = node->i_value;
	return true;
}

static bool cb_analysis_hpskip(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.hpskip = node->i_value;
	return true;
}

static void update_analysis_arch_options(RzCore *core, RzConfigNode *node) {
	RzIterator *it = ht_sp_as_iter(core->analysis->plugins);
	RzAnalysisPlugin **val;
	if (core && core->analysis && node) {
		rz_list_purge(node->options);
		rz_iterator_foreach(it, val) {
			RzAnalysisPlugin *h = *val;
			SETOPTIONS(node, h->name, NULL);
		}
	}
	rz_iterator_free(it);
}

static bool cb_analysis_recont(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.recont = node->i_value;
	return true;
}

static bool cb_analysis_ijmp(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.ijmp = node->i_value;
	return true;
}

static bool cb_asmsubvarmin(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->parser->minval = node->i_value;
	return true;
}

static bool cb_asmsubtail(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->parser->subtail = node->i_value;
	return true;
}

static bool cb_scrlast(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	rz_cons_singleton()->context->lastEnabled = node->i_value;
	return true;
}

static bool cb_scr_vi(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->cons->line->enable_vi_mode = node->i_value;
	return true;
}

static bool cb_scr_prompt_mode(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->cons->line->prompt_mode = node->i_value;
	return true;
}

static bool cb_scr_wideoff(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->print->wide_offsets = node->i_value;
	return true;
}

static bool cb_asmpseudo(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->rasm->pseudo = node->i_value;
	return true;
}

static bool cb_asmsubsec(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		core->print->flags |= RZ_PRINT_FLAGS_SECSUB;
	} else {
		core->print->flags &= (~RZ_PRINT_FLAGS_SECSUB);
	}
	rz_print_set_flags(core->print, core->print->flags);
	return true;
}

static bool cb_asmassembler(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	rz_asm_use_assembler(core->rasm, node->value);
	return true;
}

static bool cb_asm_varfold(void *core, void *node) {
	char *choice[] = { "none", "group", "hide" };
	RzConfigNode *n = node;
	const char *user_choice = n->value;
	for (int i = 0; i < sizeof(choice) / sizeof(choice[0]); i++) {
		if (!strcmp(choice[i], user_choice)) {
			return true;
		}
	}
	RZ_LOG_ERROR("asm.var.fold: Invalid choice. Only support `none, group, hide`\n");
	return false;
}

static bool cb_dbgbtdepth(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->dbg->btdepth = node->i_value;
	return true;
}

static bool cb_flag_realnames(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->flags->realnames = node->i_value;
	return true;
}

static bool cb_asmlineswidth(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->lineswidth = node->i_value;
	return true;
}

static bool cb_emustr(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		rz_config_set(core->config, "asm.emu", "true");
	}
	return true;
}

static bool cb_emuskip(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	if (*node->value == '?') {
		if (strlen(node->value) > 1 && node->value[1] == '?') {
			rz_cons_printf("Concatenation of meta types encoded as characters:\n"
				       "'d': data\n'c': code\n's': string\n'f': format\n'm': magic\n"
				       "'h': hide\n'C': comment\n'r': run\n"
				       "(default is 'ds' to skip data and strings)\n");
		} else {
			print_node_options(node);
		}
		return false;
	}
	return true;
}

static bool cb_asm_immhash(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->rasm->immdisp = node->i_value ? true : false;
	return true;
}

static bool cb_asm_invhex(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->rasm->invhex = node->i_value;
	return true;
}

static bool cb_asm_pcalign(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	int align = node->i_value;
	if (rz_bits_count_ones_ut64(align) != 1) {
		RZ_LOG_ERROR("Alignment is only defined for '>0' and 'n^2'. Is = %" PFMT64d "\n", node->i_value);
		return false;
	}
	core->rasm->pcalign = align;
	core->analysis->pcalign = align;
	return true;
}

static bool cb_binfilter(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->bin->filter = node->i_value;
	return true;
}

static bool cb_binat(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->binat = node->i_value;
	return true;
}

static bool cb_usextr(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->bin->use_xtr = node->i_value;
	return true;
}

static bool cb_strpurge(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (*node->value == '?') {
		rz_cons_printf(
			"There can be multiple entries separated by commas. No whitespace before/after entries.\n"
			"Possible entries:\n"
			"  all          : purge all strings\n"
			"  true         : use the false_positive() classifier in cbin.c\n"
			"  addr         : purge string at addr\n"
			"  addr1-addr2  : purge all strings in the range addr1-addr2 inclusive\n"
			"  !addr        : prevent purge of string at addr by prev entries\n"
			"  !addr1-addr2 : prevent purge of strings in range addr1-addr2 inclusive by prev entries\n"
			"Neither !true nor !false is supported.\n"
			"\n"
			"Examples:\n"
			"  e bin.str.purge=true,0-0xff,!0x1a\n"
			"    -- purge strings using the false_positive() classifier in cbin.c and also strings \n"
			"       with addresses in the range 0-0xff, but not the string at 0x1a.\n"
			"  e bin.str.purge=all,!0x1000-0x1fff\n"
			"    -- purge all strings except the strings with addresses in the range 0x1000-0x1fff.\n");
		return false;
	}
	free(core->bin->strpurge);
	core->bin->strpurge = !*node->value || !strcmp(node->value, "false")
		? NULL
		: rz_str_dup(node->value);
	return true;
}

static bool cb_maxname(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	RzCore *core = (RzCore *)user;
	core->parser->maxflagnamelen = node->i_value;
	return true;
}

static bool cb_midflags(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->value[0] == '?') {
		print_node_options(node);
		return false;
	}
	return true;
}

static bool cb_strfilter(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->value[0] == '?') {
		if (strlen(node->value) > 1 && node->value[1] == '?') {
			rz_cons_printf("Valid values for bin.str.filter:\n"
				       "a  only alphanumeric printable\n"
				       "8  only strings with utf8 chars\n"
				       "p  file/directory paths\n"
				       "e  email-like addresses\n"
				       "u  urls\n"
				       "i  IPv4 address-like strings\n"
				       "U  only uppercase strings\n"
				       "f  format-strings\n");
		} else {
			print_node_options(node);
		}
		return false;
	} else {
		core->bin->strfilter = node->value[0];
	}
	return true;
}

static bool cb_binforce(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	rz_bin_force_plugin(core->bin, node->value);
	return true;
}

static bool cb_bindemangle(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->bin->demangle = node->i_value;
	return true;
}

static bool cb_bindemangle_flags(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (*node->value == '?') {
		print_node_options(node);
		return false;
	}
	if (!strcmp(node->value, "all")) {
		rz_bin_demangle_with_flags(core->bin, RZ_DEMANGLER_FLAG_ENABLE_ALL);
	} else if (!strcmp(node->value, "base")) {
		rz_bin_demangle_with_flags(core->bin, RZ_DEMANGLER_FLAG_BASE);
	} else if (!strcmp(node->value, "simplify")) {
		rz_bin_demangle_with_flags(core->bin, RZ_DEMANGLER_FLAG_SIMPLIFY);
	} else {
		RZ_LOG_ERROR("config: invalid option for bin.demangle.flags\n");
		return false;
	}
	return true;
}

static bool cb_asmsyntax(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (*node->value == '?') {
		print_node_options(node);
		return false;
	} else {
		int syntax = rz_asm_syntax_from_string(node->value);
		if (syntax == -1) {
			return false;
		}
		rz_asm_set_syntax(core->rasm, syntax);
	}
	return true;
}

static bool cb_bigendian(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	return rz_core_set_endianness(core, node->i_value);
}

static bool cb_cfgdatefmt(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	snprintf(core->print->datefmt, 32, "%s", node->value);
	return true;
}

static bool cb_timezone(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->print->datezone = node->i_value;
	return true;
}

static bool cb_cfgdebug(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (!core) {
		return false;
	}
	if (core->io) {
		rz_config_set_b(core->config, "io.va", !node->i_value);
	}
	if (core->dbg && node->i_value) {
		const char *dbgbackend = rz_config_get(core->config, "dbg.backend");
		core->bin->is_debugger = true;
		rz_debug_use(core->dbg, dbgbackend);
		if (!strcmp(dbgbackend, "bf")) {
			rz_config_set(core->config, "asm.arch", "bf");
		}
		if (core->file) {
			rz_debug_select(core->dbg, rz_io_fd_get_pid(core->io, core->file->fd),
				rz_io_fd_get_tid(core->io, core->file->fd));
		}
	} else {
		rz_debug_use(core->dbg, NULL);
		core->bin->is_debugger = false;
	}
	return true;
}

static bool cb_dirhome(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->value) {
		rz_sys_setenv(RZ_SYS_HOME, node->value);
	}
	return true;
}

static bool cb_dirtmp(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	char *value = RZ_STR_ISNOTEMPTY(node->value) ? node->value : NULL;
	rz_sys_setenv(RZ_SYS_TMP, value);
	return true;
}

static bool cb_dirsrc(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	RzCore *core = (RzCore *)user;
	free(core->bin->srcdir);
	core->bin->srcdir = rz_str_dup(node->value);
	return true;
}

static bool cb_str_escbslash(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->print->esc_bslash = node->i_value;
	return true;
}

static bool cb_search_max_threads(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->value[0] == '?') {
		rz_cons_printf("Available cores: %d\n", rz_th_physical_core_number());
		return false;
	}
	return true;
}

static bool cb_search_to(void *user, void *data) {
	RzCore *core = user;
	RzConfigNode *node = (RzConfigNode *)data;
	ut64 from = rz_config_get_i(core->config, "search.from");
	if (node->i_value < from) {
		rz_cons_printf("search.to cannot be smaller than search.from.\n");
		return false;
	}
	return true;
}

static void check_reload_bin_str_search(RzCore *core) {
	if (core->bin && rz_config_get_b(core->config, "str.search.reload")) {
		RzBinFile *bf = rz_bin_cur(core->bin);
		if (bf && bf->o) {
			rz_bin_object_reset_strings(core->bin, bf, bf->o);
		}
	}
}

static bool cb_search_str_min_length(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value < 1) {
		RZ_LOG_ERROR("search.str.min_length cannot be less than 1.\n");
		return false;
	} else if (node->i_value >= core->bin->str_search_cfg.max_length) {
		RZ_LOG_ERROR("search.str.max_length cannot be greater or equal to %" PFMTSZu ".\n", core->bin->str_search_cfg.max_length);
		return false;
	}

	core->bin->str_search_cfg.min_length = node->i_value;
	check_reload_bin_str_search(core);
	return true;
}

static bool cb_search_str_max_length(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;

	size_t min_buffer_size = RZ_MIN(core->bin->str_search_cfg.min_length, RZ_BIN_STRING_SEARCH_BUFFER_SIZE);
	if (node->i_value < min_buffer_size) {
		RZ_LOG_ERROR("search.str.max_length cannot be less than %" PFMTSZu ".\n", min_buffer_size);
		return false;
	}

	core->bin->str_search_cfg.max_length = node->i_value;
	check_reload_bin_str_search(core);
	return true;
}

static bool cb_search_str_max_region_size(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value < RZ_BIN_STRING_SEARCH_MAX_REGION_SIZE) {
		RZ_LOG_ERROR("search.str.max_region_size cannot be less than " RZ_STR(RZ_BIN_STRING_SEARCH_MAX_REGION_SIZE) ".\n");
		return false;
	}
	core->bin->str_search_cfg.max_region_size = node->i_value;
	check_reload_bin_str_search(core);
	return true;
}

static bool cb_search_str_raw_alignment(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value < 8 || rz_bits_count_ones_ut64(node->i_value) != 1) {
		RZ_LOG_ERROR("search.str.raw_alignment cannot be less than 8 and must be n^2.\n");
		return false;
	}
	core->bin->str_search_cfg.raw_alignment = node->i_value;
	return true;
}

static bool cb_search_str_check_ascii_freq(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->value[0] == '?') {
		rz_cons_printf("true\nfalse\n");
		return false;
	} else if (!rz_str_is_bool(node->value)) {
		RZ_LOG_ERROR("Invalid value for search.str.check_ascii_freq (%s).\n", node->value);
		return false;
	}
	core->bin->str_search_cfg.check_ascii_freq = rz_str_is_true(node->value);
	check_reload_bin_str_search(core);
	return true;
}

static bool find_encoding(RzConfigNode *node, RzStrEnc *encoding) {
	RzListIter *iter;
	const char *option;
	rz_list_foreach (node->options, iter, option) {
		if (rz_str_casecmp(option, node->value)) {
			continue;
		}
		free(node->value);
		node->value = rz_str_dup(option);
		*encoding = rz_str_enc_string_as_type(option);
		return true;
	}
	if (rz_list_empty(node->options)) {
		// Edge case when the node was just initialized but the options
		// were not added yet.
		*encoding = rz_str_enc_string_as_type(node->value);
		return true;
	}
	return false;
}

static bool cb_str_encoding(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	RzStrEnc encoding = RZ_STRING_ENC_GUESS;
	bool found_enc = find_encoding(node, &encoding);
	if (node->value[0] == '?') {
		print_node_options(node);
		rz_cons_printf("  -- if string's 2nd & 4th bytes are 0 then utf16le else "
			       "if 2nd - 4th & 6th bytes are 0 & no char > 0x10ffff then utf32le else "
			       "if utf8 char detected then utf8 else 8bit\n");
		return false;
	} else if (RZ_STR_EQ("settings", node->value) || (!RZ_STR_EQ("guess", node->value) && !found_enc)) {
		RZ_LOG_ERROR("Invalid value for str.encoding (%s).\n", node->value);
		return false;
	}

	core->bin->str_search_cfg.string_encoding = encoding;
	check_reload_bin_str_search(core);
	return true;
}

static bool cb_str_search_mode(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->value[0] == '?') {
		print_node_options(node);
		return false;
	} else if (!rz_str_casecmp("auto", node->value)) {
		core->bin->str_search_cfg.mode = RZ_BIN_STRING_SEARCH_MODE_AUTO;
		return true;
	} else if (!rz_str_casecmp("rosections", node->value)) {
		core->bin->str_search_cfg.mode = RZ_BIN_STRING_SEARCH_MODE_READ_ONLY_SECTIONS;
		return true;
	} else if (!rz_str_casecmp("raw", node->value)) {
		core->bin->str_search_cfg.mode = RZ_BIN_STRING_SEARCH_MODE_RAW_BINARY;
		return true;
	}
	RZ_LOG_ERROR("Invalid value for str.search.mode (%s).\n", node->value);
	check_reload_bin_str_search(core);
	return false;
}

static bool cb_completion_maxtab(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->cons->line->completion.args_limit = node->i_value;
	return true;
}

static bool cb_cfg_fortunes(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	// TODO CN_BOOL option does not receive the right hand side of assignment as an argument
	if (node->value[0] == '?') {
		rz_core_fortune_list(core);
		return false;
	}
	return true;
}

static bool cb_cfg_fortunes_file(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->value[0] == '?') {
		rz_core_fortune_list_types();
		return false;
	}
	return true;
}

static bool cb_cmdtimes(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->cmdtimes = node->value;
	return true;
}

static bool cb_cmdrepeat(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->cmdrepeat = node->i_value;
	return true;
}

static bool cb_scrnull(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->cons->null = node->i_value;
	return true;
}

static bool cb_color(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		core->print->flags |= RZ_PRINT_FLAGS_COLOR;
	} else {
		core->print->flags &= (~RZ_PRINT_FLAGS_COLOR);
	}
	if (!strcmp(node->value, "true")) {
		node->i_value = 1;
	} else if (!strcmp(node->value, "false")) {
		node->i_value = 0;
	}
	rz_cons_singleton()->context->color_mode = (node->i_value > COLOR_MODE_16M)
		? COLOR_MODE_16M
		: node->i_value;
	rz_cons_pal_update_event();
	rz_print_set_flags(core->print, core->print->flags);
	return true;
}

static bool cb_color_getter(void *user, RzConfigNode *node) {
	(void)user;
	node->i_value = rz_cons_singleton()->context->color_mode;
	char buf[128];
	rz_config_node_value_format_i(buf, sizeof(buf), rz_cons_singleton()->context->color_mode, node);
	if (!node->value || strcmp(node->value, buf) != 0) {
		free(node->value);
		node->value = rz_str_dup(buf);
	}
	return true;
}

static bool cb_decoff(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		core->print->flags |= RZ_PRINT_FLAGS_ADDRDEC;
	} else {
		core->print->flags &= (~RZ_PRINT_FLAGS_ADDRDEC);
	}
	rz_print_set_flags(core->print, core->print->flags);
	return true;
}

static bool cb_dbgbep(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	if (*node->value == '?') {
		print_node_options(node);
		return false;
	}
	return true;
}

static bool cb_dbg_btalgo(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (*node->value == '?') {
		print_node_options(node);
		return false;
	}
	free(core->dbg->btalgo);
	core->dbg->btalgo = rz_str_dup(node->value);
	return true;
}

static bool cb_dbg_libs(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	free(core->dbg->glob_libs);
	core->dbg->glob_libs = rz_str_dup(node->value);
	return true;
}

static bool cb_dbg_unlibs(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	free(core->dbg->glob_unlibs);
	core->dbg->glob_unlibs = rz_str_dup(node->value);
	return true;
}

static bool cb_dbg_bpinmaps(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->dbg->bp->bpinmaps = node->i_value;
	return true;
}

static bool cb_dbg_forks(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->dbg->trace_forks = node->i_value;
	if (core->bin->is_debugger) {
		rz_debug_attach(core->dbg, core->dbg->pid);
	}
	return true;
}

static bool cb_dbg_gdb_page_size(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value < 64) { // 64 is hardcoded min packet size
		return false;
	}
	if (isGdbPlugin(core)) {
		char cmd[64];
		snprintf(cmd, sizeof(cmd), "page_size %" PFMT64d, node->i_value);
		free(rz_io_system(core->io, cmd));
	}
	return true;
}

static bool cb_dbg_gdb_retries(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value <= 0) {
		return false;
	}
	if (isGdbPlugin(core)) {
		char cmd[64];
		snprintf(cmd, sizeof(cmd), "retries %" PFMT64d, node->i_value);
		free(rz_io_system(core->io, cmd));
	}
	return true;
}

static bool cb_dbg_execs(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
#if __linux__
	RzCore *core = (RzCore *)user;
	core->dbg->trace_execs = node->i_value;
	if (core->bin->is_debugger) {
		rz_debug_attach(core->dbg, core->dbg->pid);
	}
#else
	if (node->i_value) {
		RZ_LOG_WARN("core: dbg.execs: not supported in this platform.\n");
	}
#endif
	return true;
}

static bool cb_dbg_clone(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->dbg->trace_clone = node->i_value;
	if (core->bin->is_debugger) {
		rz_debug_attach(core->dbg, core->dbg->pid);
	}
	return true;
}

static bool cb_dbg_follow_child(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->dbg->follow_child = node->i_value;
	return true;
}

static bool cb_dbg_create_new_console(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->dbg->create_new_console = node->i_value;
	return true;
}

static bool cb_dbg_trace_continue(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->dbg->trace_continue = node->i_value;
	return true;
}

static bool cb_dbg_aftersc(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->dbg->trace_aftersyscall = node->i_value;
	if (core->bin->is_debugger) {
		rz_debug_attach(core->dbg, core->dbg->pid);
	}
	return true;
}

static bool cb_runprofile(void *user, void *data) {
	RzCore *r = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	free((void *)r->io->runprofile);
	if (!node || !*(node->value)) {
		r->io->runprofile = NULL;
	} else {
		r->io->runprofile = rz_str_dup(node->value);
	}
	return true;
}

static bool cb_dbg_args(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (!node || !*(node->value)) {
		core->io->args = NULL;
	} else {
		core->io->args = rz_str_dup(node->value);
	}
	return true;
}

static bool cb_dbgbackend(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (!strcmp(node->value, "?")) {
		RzCmdStateOutput state = { 0 };
		rz_cmd_state_output_init(&state, RZ_OUTPUT_MODE_QUIET, core);
		rz_core_debug_plugins_print(core, &state);
		rz_cmd_state_output_print(&state);
		rz_cmd_state_output_fini(&state);
		return false;
	}
	if (!strcmp(node->value, "bf")) {
		// hack
		rz_config_set(core->config, "asm.arch", "bf");
	}
	rz_debug_use(core->dbg, node->value);
	return true;
}

static bool cb_gotolimit(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (core->analysis->esil) {
		core->analysis->esil_goto_limit = node->i_value;
	}
	return true;
}

static bool cb_esilverbose(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (core->analysis->esil) {
		core->analysis->esil->verbose = node->i_value;
	}
	return true;
}

static bool cb_esilstackdepth(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value < 3) {
		RZ_LOG_ERROR("core: esil.stack.depth: value must be greater than 2\n");
		node->i_value = 32;
	}
	return true;
}

static bool cb_fixrows(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	rz_cons_singleton()->fix_rows = (int)node->i_value;
	return true;
}

static bool cb_fixcolumns(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	rz_cons_singleton()->fix_columns = atoi(node->value);
	return true;
}

static bool cb_rows(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	rz_cons_singleton()->force_rows = node->i_value;
	return true;
}

static bool cb_cmd_hexcursor(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->print->cfmt = node->value;
	return true;
}

static bool cb_hexcompact(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		core->print->flags |= RZ_PRINT_FLAGS_COMPACT;
	} else {
		core->print->flags &= (~RZ_PRINT_FLAGS_COMPACT);
	}
	return true;
}

static bool cb_hex_pairs(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->print->pairs = node->i_value;
	return true;
}

static bool cb_hex_nodot(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		core->print->flags |= RZ_PRINT_FLAGS_NODOT;
	} else {
		core->print->flags &= ~RZ_PRINT_FLAGS_NODOT;
	}
	return true;
}

static bool cb_hex_section(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		core->print->flags |= RZ_PRINT_FLAGS_SECTION;
	} else {
		core->print->flags &= ~RZ_PRINT_FLAGS_SECTION;
	}
	return true;
}

static bool cb_hex_align(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		core->print->flags |= RZ_PRINT_FLAGS_ALIGN;
	} else {
		core->print->flags &= ~RZ_PRINT_FLAGS_ALIGN;
	}
	return true;
}

static bool cb_io_unalloc(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		core->print->flags |= RZ_PRINT_FLAGS_UNALLOC;
	} else {
		core->print->flags &= ~RZ_PRINT_FLAGS_UNALLOC;
	}
	return true;
}

static bool cb_io_unalloc_ch(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->print->io_unalloc_ch = *node->value ? node->value[0] : ' ';
	return true;
}

static bool cb_hex_header(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		core->print->flags |= RZ_PRINT_FLAGS_HEADER;
	} else {
		core->print->flags &= ~RZ_PRINT_FLAGS_HEADER;
	}
	return true;
}

static bool cb_hex_bytes(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		core->print->flags &= ~RZ_PRINT_FLAGS_NONHEX;
	} else {
		core->print->flags |= RZ_PRINT_FLAGS_NONHEX;
	}
	return true;
}

static bool cb_hex_ascii(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		core->print->flags &= ~RZ_PRINT_FLAGS_NONASCII;
	} else {
		core->print->flags |= RZ_PRINT_FLAGS_NONASCII;
	}
	return true;
}

static bool cb_hex_style(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		core->print->flags |= RZ_PRINT_FLAGS_STYLE;
	} else {
		core->print->flags &= ~RZ_PRINT_FLAGS_STYLE;
	}
	return true;
}

static bool cb_hex_hdroff(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		core->print->flags |= RZ_PRINT_FLAGS_HDROFF;
	} else {
		core->print->flags &= ~RZ_PRINT_FLAGS_HDROFF;
	}
	return true;
}

static bool cb_log_events(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->log_events = node->i_value;
	return true;
}

static bool cb_hexcomments(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		core->print->flags |= RZ_PRINT_FLAGS_COMMENT;
	} else {
		core->print->flags &= ~RZ_PRINT_FLAGS_COMMENT;
	}
	return true;
}

static bool cb_iopcache(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if ((bool)node->i_value) {
		if (core) {
			rz_config_set_b(core->config, "io.pcache.read", true);
			rz_config_set_b(core->config, "io.pcache.write", true);
		}
	} else {
		if (core && core->io) {
			rz_io_desc_cache_fini_all(core->io);
			rz_config_set_b(core->config, "io.pcache.read", false);
			rz_config_set_b(core->config, "io.pcache.write", false);
		}
	}
	return true;
}

static bool cb_iopcacheread(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if ((bool)node->i_value) {
		if (core && core->io) {
			core->io->p_cache |= 1;
		}
	} else {
		if (core && core->io && core->io->p_cache) {
			core->io->p_cache &= 2;
			if (!(core->io->p_cache & 2)) {
				rz_io_desc_cache_fini_all(core->io);
				rz_config_set_b(core->config, "io.pcache", false);
			}
		}
	}
	return true;
}

static bool cb_iopcachewrite(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if ((bool)node->i_value) {
		if (core && core->io) {
			core->io->p_cache |= 2;
		}
	} else {
		if (core && core->io && core->io->p_cache) {
			core->io->p_cache &= 1;
			if (!(core->io->p_cache & 1)) {
				rz_io_desc_cache_fini_all(core->io);
				rz_config_set_b(core->config, "io.pcache", false);
			}
		}
	}
	return true;
}

RZ_API bool rz_core_esil_cmd(RzAnalysisEsil *esil, const char *cmd, ut64 a1, ut64 a2) {
	if (cmd && *cmd) {
		RzCore *core = esil->analysis->core;
		rz_core_cmdf(core, "%s %" PFMT64d " %" PFMT64d, cmd, a1, a2);
		return core->num->value;
	}
	return false;
}

static void config_print_node(RzConfig *cfg, RzConfigNode *node, RzCmdStateOutput *state) {
	rz_return_if_fail(cfg && node && state);
	char *option;
	bool isFirst;
	RzOutputMode mode = state->mode;
	PJ *pj = state->d.pj;
	RzListIter *iter;
	char *es = NULL;

	switch (mode) {
	case RZ_OUTPUT_MODE_JSON:
		if (rz_str_isnumber(node->value)) {
			pj_kn(pj, node->name, rz_num_math(NULL, node->value));
			return;
		} else if (rz_str_is_bool(node->value)) {
			pj_kb(pj, node->name, (node->value));
			return;
		} else {
			pj_ks(pj, node->name, node->value);
		}
		break;
	case RZ_OUTPUT_MODE_LONG_JSON:
		pj_o(pj);
		pj_ks(pj, "name", node->name);
		if (rz_str_isnumber(node->value)) {
			pj_kn(pj, "value", rz_num_math(NULL, node->value));
		} else if (rz_str_is_bool(node->value)) {
			pj_kb(pj, "value", (node->value));
		} else {
			pj_ks(pj, "value", node->value);
		}
		pj_ks(pj, "type", rz_config_node_type(node));
		es = rz_str_escape(node->desc);
		if (es) {
			pj_ks(pj, "desc", es);
			free(es);
		}
		pj_kb(pj, "ro", rz_config_node_is_ro(node));
		if (!rz_list_empty(node->options)) {
			pj_ka(pj, "options");
			rz_list_foreach (node->options, iter, option) {
				pj_s(pj, option);
			}
			pj_end(pj);
		}
		pj_end(pj);
		break;
	case RZ_OUTPUT_MODE_LONG: {
		bool color_enabled = rz_config_get_i(cfg, "scr.color") > 0;
		char color_name[32], color_value[32], color_meta[32], reset_str[32];

		if (color_enabled) {
			RzColor color_name_val = rz_cons_pal_get("label");
			RzColor color_value_val = rz_cons_pal_get("args");
			RzColor color_meta_val = rz_cons_pal_get("comment");
			RzColor reset_val = rz_cons_pal_get("help");
			rz_cons_rgb_str(color_name, sizeof(color_name), &color_name_val);
			rz_cons_rgb_str(color_value, sizeof(color_value), &color_value_val);
			rz_cons_rgb_str(color_meta, sizeof(color_meta), &color_meta_val);
			rz_cons_rgb_str(reset_str, sizeof(reset_str), &reset_val);
		} else {
			color_name[0] = color_value[0] = color_meta[0] = reset_str[0] = '\0';
		}
		const char *ro_str = rz_config_node_is_ro(node) ? "(ro)" : "";
		rz_cons_printf("%s%20s = %s%s %s%s; %s%s", color_name, node->name, color_value, node->value, color_meta, ro_str, reset_str, node->desc);
		if (!rz_list_empty(node->options)) {
			isFirst = true;
			rz_cons_printf(" [");
			rz_list_foreach (node->options, iter, option) {
				if (isFirst) {
					isFirst = false;
				} else {
					rz_cons_printf(", ");
				}
				rz_cons_printf("%s", option);
			}
			rz_cons_printf("]");
		}
		rz_cons_println("");
		break;
	}
	case RZ_OUTPUT_MODE_QUIET:
		rz_cons_printf("%s=%s\n", node->name, node->value);
		break;
	case RZ_OUTPUT_MODE_STANDARD: {
		bool color_enabled = rz_config_get_i(cfg, "scr.color") > 0;
		char color_str[32], reset_str[32];

		if (color_enabled) {
			RzColor color_val = rz_cons_pal_get("label");
			RzColor reset_val = rz_cons_pal_get("help");
			rz_cons_rgb_str(color_str, sizeof(color_str), &color_val);
			rz_cons_rgb_str(reset_str, sizeof(reset_str), &reset_val);
		} else {
			color_str[0] = reset_str[0] = '\0';
		}

		rz_cons_printf("%s%20s: %s%s\n", color_str, node->name, reset_str, node->desc ? node->desc : "");
		break;
	}
	case RZ_OUTPUT_MODE_STR_BUF:
		rz_strbuf_appendf(state->d.sbuf, "%20s: %10s - %s\n", node->name,
			rz_config_node_type(node),
			node->desc ? node->desc : "");
		break;
	default:
		rz_warn_if_reached();
		break;
	}
}

/**
 * \brief Prints the configuation variables with their description and its values
 *
 * \param cfg reference to RzConfig
 * \param str reference to the key that can be passed to filter the output
 * \param state reference to RzCmdStateOutput
 */
RZ_API void rz_core_config_print_all(RzConfig *cfg, const char *str, RzCmdStateOutput *state) {
	rz_return_if_fail(cfg);
	RzConfigNode *node;
	RzListIter *iter;
	PJ *pj = state->d.pj;
	RzOutputMode mode = state->mode;

	if (mode == RZ_OUTPUT_MODE_LONG_JSON) {
		pj_a(pj);
	} else if (mode == RZ_OUTPUT_MODE_JSON) {
		pj_o(pj);
	}

	rz_list_foreach (cfg->nodes, iter, node) {
		if (rz_str_startswith(node->name, str)) {
			config_print_node(cfg, node, state);
		}
	}

	if (mode == RZ_OUTPUT_MODE_LONG_JSON || mode == RZ_OUTPUT_MODE_JSON) {
		pj_end(pj);
	}
}

static bool cb_cmd_esil_ioer(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (core && core->analysis && core->analysis->esil) {
		core->analysis->esil->cmd = rz_core_esil_cmd;
		free(core->analysis->esil->cmd_ioer);
		core->analysis->esil->cmd_ioer = rz_str_dup(node->value);
	}
	return true;
}

static bool cb_cmd_esil_todo(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (core && core->analysis && core->analysis->esil) {
		core->analysis->esil->cmd = rz_core_esil_cmd;
		free(core->analysis->esil->cmd_todo);
		core->analysis->esil->cmd_todo = rz_str_dup(node->value);
	}
	return true;
}

static bool cb_cmd_esil_intr(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (core && core->analysis && core->analysis->esil) {
		core->analysis->esil->cmd = rz_core_esil_cmd;
		free(core->analysis->esil->cmd_intr);
		core->analysis->esil->cmd_intr = rz_str_dup(node->value);
	}
	return true;
}

static bool cb_mdevrange(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (core && core->analysis && core->analysis->esil) {
		core->analysis->esil->cmd = rz_core_esil_cmd;
		free(core->analysis->esil->mdev_range);
		core->analysis->esil->mdev_range = rz_str_dup(node->value);
	}
	return true;
}

static bool cb_cmd_esil_step(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (core && core->analysis && core->analysis->esil) {
		core->analysis->esil->cmd = rz_core_esil_cmd;
		free(core->analysis->esil->cmd_step);
		core->analysis->esil->cmd_step = rz_str_dup(node->value);
	}
	return true;
}

static bool cb_cmd_esil_step_out(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (core && core->analysis && core->analysis->esil) {
		core->analysis->esil->cmd = rz_core_esil_cmd;
		free(core->analysis->esil->cmd_step_out);
		core->analysis->esil->cmd_step_out = rz_str_dup(node->value);
	}
	return true;
}

static bool cb_cmd_esil_mdev(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (core && core->analysis && core->analysis->esil) {
		core->analysis->esil->cmd = rz_core_esil_cmd;
		free(core->analysis->esil->cmd_mdev);
		core->analysis->esil->cmd_mdev = rz_str_dup(node->value);
	}
	return true;
}

static bool cb_cmd_esil_trap(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (core && core->analysis && core->analysis->esil) {
		core->analysis->esil->cmd = rz_core_esil_cmd;
		core->analysis->esil->cmd_trap = rz_str_dup(node->value);
	}
	return true;
}

static bool cb_cmddepth(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	int c = RZ_MAX(((RzConfigNode *)data)->i_value, 0);
	core->max_cmd_depth = c;
	core->cons->context->cmd_depth = c;
	return true;
}

static bool cb_hexcols(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	int c = RZ_MIN(1024, RZ_MAX(((RzConfigNode *)data)->i_value, 0));
	core->print->cols = c; // & ~1;
	core->dbg->regcols = c / 4;
	return true;
}

static bool cb_hexstride(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	((RzCore *)user)->print->stride = node->i_value;
	return true;
}

static bool cb_search_kwidx(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->search->n_kws = node->i_value;
	return true;
}

static bool cb_io_cache_mode(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		core->io->cachemode = true;
	} else {
		core->io->cachemode = false;
	}
	return true;
}

static bool cb_io_cache_read(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		core->io->cached |= RZ_PERM_R;
	} else {
		core->io->cached &= ~RZ_PERM_R;
	}
	return true;
}

static bool cb_io_cache_write(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		core->io->cached |= RZ_PERM_W;
	} else {
		core->io->cached &= ~RZ_PERM_W;
	}
	return true;
}

static bool cb_io_cache(void *user, void *data) {
	(void)cb_io_cache_read(user, data);
	(void)cb_io_cache_write(user, data);
	return true;
}

static bool cb_ioaslr(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value != core->io->aslr) {
		core->io->aslr = node->i_value;
	}
	return true;
}

static bool cb_io_pava(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->print->pava = node->i_value;
	if (node->i_value && core->io->va) {
		RZ_LOG_WARN("core: you may probably want to disable io.va too\n");
	}
	return true;
}

static bool cb_iova(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value != core->io->va) {
		core->io->va = node->i_value;
		/* ugly fix for rizin -d ... "rizin is going to die soon ..." */
		if (core->io->desc) {
			rz_core_block_read(core);
		}
	}
	return true;
}

static bool cb_ioff(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->io->ff = node->i_value;
	return true;
}

static bool cb_io_oxff(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->io->Oxff = node->i_value;
	return true;
}

static bool cb_filepath(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	char *pikaboo = strstr(node->value, "://");
	if (pikaboo) {
		if (pikaboo[3] == '/') {
			rz_config_set(core->config, "file.lastpath", node->value);
			char *ovalue = node->value;
			node->value = rz_str_dup(pikaboo + 3);
			free(ovalue);
			return true;
		}
		return false;
	}
	rz_config_set(core->config, "file.lastpath", node->value);
	return true;
}

static bool cb_ioautofd(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->io->autofd = node->i_value;
	return true;
}

static bool cb_scr_color_grep(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;

	/* Let cons know we have a new pager. */
	core->cons->grep_color = node->i_value;
	return true;
}

static bool cb_scr_color_grep_highlight(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->cons->grep_highlight = node->i_value;
	return true;
}

static bool cb_pager(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (!strcmp(node->value, "?")) {
		RZ_LOG_ERROR("usage: scr.pager must be '..' for internal less, or the path to a program in $PATH\n");
		return false;
	}
	/* Let cons know we have a new pager. */
	free(core->cons->pager);
	core->cons->pager = rz_str_dup(node->value);
	return true;
}

static bool cb_breaklines(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	rz_cons_singleton()->break_lines = node->i_value;
	return true;
}

static bool cb_scr_gadgets(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->scr_gadgets = node->i_value;
	return true;
}

static bool cb_scrbreakword(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	if (*node->value) {
		rz_cons_breakword(node->value);
	} else {
		rz_cons_breakword(NULL);
	}
	return true;
}

static bool cb_scrcolumns(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	RzCore *core = (RzCore *)user;
	int n = atoi(node->value);
	core->cons->force_columns = n;
	core->dbg->regcols = n / 20;
	rz_cons_get_size(NULL);
	return true;
}

static bool cb_scrfgets(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->cons->user_fgets = node->i_value
		? NULL
		: (void *)rz_core_fgets;
	core->cons->user_fgets_user = core;
	return true;
}

static bool cb_scrhtml(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	rz_cons_singleton()->is_html = node->i_value;
	// TODO: control error and restore old value (return false?) show errormsg?
	return true;
}

static bool cb_scrhighlight(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	rz_cons_highlight(node->value);
	return true;
}

#if __WINDOWS__
static bool scr_vtmode(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	if (rz_str_is_true(node->value)) {
		node->i_value = RZ_VIRT_TERM_MODE_OUTPUT_ONLY;
	}
	node->i_value = node->i_value > RZ_VIRT_TERM_MODE_COMPLETE ? RZ_VIRT_TERM_MODE_COMPLETE : node->i_value;
	RzCons *cons = rz_cons_singleton();
	cons->line->vtmode = cons->vtmode = node->i_value;

	DWORD mode;
	HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
	GetConsoleMode(input, &mode);
	if (node->i_value == RZ_VIRT_TERM_MODE_COMPLETE) {
		SetConsoleMode(input, mode & ENABLE_VIRTUAL_TERMINAL_INPUT);
		cons->term_raw |= ENABLE_VIRTUAL_TERMINAL_INPUT;
	} else {
		SetConsoleMode(input, mode & ~ENABLE_VIRTUAL_TERMINAL_INPUT);
		cons->term_raw &= ~ENABLE_VIRTUAL_TERMINAL_INPUT;
	}
	HANDLE streams[] = { GetStdHandle(STD_OUTPUT_HANDLE), GetStdHandle(STD_ERROR_HANDLE) };
	int i;
	if (node->i_value > RZ_VIRT_TERM_MODE_DISABLE) {
		for (i = 0; i < RZ_ARRAY_SIZE(streams); i++) {
			GetConsoleMode(streams[i], &mode);
			SetConsoleMode(streams[i],
				mode | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
		}
	} else {
		for (i = 0; i < RZ_ARRAY_SIZE(streams); i++) {
			GetConsoleMode(streams[i], &mode);
			SetConsoleMode(streams[i],
				mode & ~ENABLE_VIRTUAL_TERMINAL_PROCESSING & ~ENABLE_WRAP_AT_EOL_OUTPUT);
		}
	}
	return true;
}
#endif

static bool cb_screcho(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	rz_cons_singleton()->echo = node->i_value;
	return true;
}

static bool cb_scrlinesleep(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	rz_cons_singleton()->linesleep = node->i_value;
	return true;
}

static bool cb_scrpagesize(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	rz_cons_singleton()->pagesize = node->i_value;
	return true;
}

static bool cb_scrflush(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	rz_cons_singleton()->flush = node->i_value;
	return true;
}

static bool cb_scrstrconv(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->value[0] == '?') {
		if (strlen(node->value) > 1 && node->value[1] == '?') {
			rz_cons_printf("Valid values for scr.strconv:\n"
				       "  asciiesc  convert to ascii with non-ascii chars escaped\n"
				       "  asciidot  convert to ascii with non-ascii chars turned into a dot (except control chars stated below)\n"
				       "\n"
				       "Ascii chars are in the range 0x20-0x7e. Always escaped control chars are alert (\\a),\n"
				       "backspace (\\b), formfeed (\\f), newline (\\n), carriage return (\\r), horizontal tab (\\t)\n"
				       "and vertical tab (\\v). Also, double quotes (\\\") are always escaped, but backslashes (\\\\)\n"
				       "are only escaped if str.escbslash = true.\n");
		} else {
			print_node_options(node);
		}
		return false;
	} else {
		free((char *)core->print->strconv_mode);
		core->print->strconv_mode = rz_str_dup(node->value);
	}
	return true;
}

static bool cb_graphformat(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	if (!strcmp(node->value, "?")) {
		rz_cons_printf("png\njpg\npdf\nps\nsvg\njson\n");
		return false;
	}
	return true;
}

static bool cb_exectrap(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	RzCore *core = (RzCore *)user;
	if (core->analysis && core->analysis->esil) {
		core->analysis->esil->exectrap = node->i_value;
	}
	return true;
}

static bool cb_iotrap(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	RzCore *core = (RzCore *)user;
	if (core->analysis && core->analysis->esil) {
		core->analysis->esil->iotrap = node->i_value;
	}
	return true;
}

static bool cb_scr_bgfill(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		core->print->flags |= RZ_PRINT_FLAGS_BGFILL;
	} else {
		core->print->flags &= (~RZ_PRINT_FLAGS_BGFILL);
	}
	rz_print_set_flags(core->print, core->print->flags);
	return true;
}

static bool cb_scrint(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	rz_cons_singleton()->context->is_interactive = node->i_value;
	return true;
}

static bool cb_scrnkey(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	if (!strcmp(node->value, "help") || *node->value == '?') {
		print_node_options(node);
		return false;
	}
	return true;
}

static bool cb_scr_histblock(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->print->histblock = node->i_value;
	return true;
}

static bool cb_scrprompt(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->print->scr_prompt = node->i_value;
	core->cons->line->echo = node->i_value;
	return true;
}

static bool cb_scrrows(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	int n = atoi(node->value);
	core->cons->force_rows = n;
	return true;
}

static bool cb_contiguous(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->search->contiguous = node->i_value;
	return true;
}

static bool cb_searchalign(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (rz_bits_count_ones_ut64(node->i_value) != 1) {
		RZ_LOG_ERROR("Alignment is only defined for '>0' and 'n^2'. Is = %" PFMT64d "\n", node->i_value);
		return false;
	}
	core->search->align = node->i_value;
	return true;
}

static bool cb_segoff(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		core->print->flags |= RZ_PRINT_FLAGS_SEGOFF;
	} else {
		core->print->flags &= (((ut32)-1) & (~RZ_PRINT_FLAGS_SEGOFF));
	}
	return true;
}

static bool cb_seggrn(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->rasm->seggrn = node->i_value;
	core->analysis->seggrn = node->i_value;
	core->print->seggrn = node->i_value;
	return true;
}

static bool cb_stopthreads(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->dbg->stop_all_threads = node->i_value;
	return true;
}

static bool cb_scr_prompt_popup(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->cons->show_autocomplete_widget = node->i_value;
	return true;
}

static bool cb_swstep(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->dbg->swstep = node->i_value;
	return true;
}

static bool cb_consbreak(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->dbg->consbreak = node->i_value;
	return true;
}

static bool cb_teefile(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	rz_cons_singleton()->teefile = node->value;
	return true;
}

static bool cb_trace(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->dbg->trace->enabled = node->i_value;
	return true;
}

static bool cb_tracetag(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->dbg->trace->tag = node->i_value;
	return true;
}

static bool cb_utf8(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->rasm->utf8 = (bool)node->i_value;
	rz_cons_set_utf8((bool)node->i_value);
	return true;
}

static bool cb_utf8_curvy(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->cons->use_utf8_curvy = node->i_value;
	return true;
}

static bool cb_visual_mode(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value > RZ_CORE_VISUAL_MODE_CD) {
		node->i_value = RZ_CORE_VISUAL_MODE_PX;
	}
	RzCore *core = (RzCore *)user;
	((RzCoreVisual *)core->visual)->printidx = node->i_value;
	return true;
}

static bool cb_dotted(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->cons->dotted_lines = node->i_value;
	return true;
}

static bool cb_zoombyte(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	switch (*node->value) {
	case 'p':
	case 'f':
	case 's':
	case '0':
	case 'F':
	case 'e':
	case 'h':
		core->print->zoom->mode = *node->value;
		break;
	default:
		RZ_LOG_ERROR("core: invalid zoom.byte value. See pz? for help\n");
		rz_cons_printf("pzp\npzf\npzs\npz0\npzF\npze\npzh\n");
		return false;
	}
	return true;
}

static bool cb_binverbose(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->bin->verbose = node->i_value;
	return true;
}

static bool cb_binhashesdefault(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (*node->value == '?') {
		print_node_options(node);
		rz_cons_printf("Multiple algorithms can be specified in a comma-separated list (no spaces).\n");
		return false;
	}
	rz_list_free(core->bin->default_hashes);
	core->bin->default_hashes = rz_str_split_duplist(node->value, ",", true);
	if (!core->bin->default_hashes) {
		core->bin->default_hashes = rz_list_new();
	}
	return true;
}

static bool cb_binstrings(void *user, void *data) {
	const ut32 req = RZ_BIN_REQ_STRINGS;
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->i_value) {
		core->bin->filter_rules |= req;
	} else {
		core->bin->filter_rules &= ~req;
	}
	return true;
}

static bool cb_bindbginfo(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (!core || !core->bin) {
		return false;
	}
	core->bin->want_dbginfo = node->i_value;
	return true;
}

static bool cb_dwo_path(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (!(core && core->bin && node)) {
		return false;
	}
	if (RZ_STR_ISNOTEMPTY(node->value) && rz_bin_cur(core->bin)) {
		rz_core_bin_apply_dwarf(core, rz_bin_cur(core->bin));
	}
	return true;
}

static bool cb_binprefix(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (!core || !core->bin) {
		return false;
	}
	if (node->value && *node->value) {
		if (!strcmp(node->value, "auto")) {
			if (!core->bin->file) {
				return false;
			}
			char *name = (char *)rz_file_basename(core->bin->file);
			if (name) {
				rz_name_filter(name, strlen(name), true);
				rz_str_filter(name);
				core->bin->prefix = rz_str_dup(name);
				free(name);
			}
		} else {
			core->bin->prefix = node->value;
		}
	}
	return true;
}

static ConfigOptDescr search_in_opts[] = {
	{ "raw", "all raw memory (behaves as 'file')" },
	{ "file", "all the file" },
	{ "block", "current read block" },
	{ "range", "search only within the range" },
	{ "analysis.bb", "current basic-block" },
	{ "analysis.fcn", "current function" },
	{ "bin.section", "current section" },
	{ "bin.sections[.rwx]", "all bin sections (with matching perm)" },
	{ "bin.segment", "current segment" },
	{ "bin.segments[.rwx]", "all bin segments (with matching perm)" },
	{ "dbg.heap", "debugger heap" },
	{ "dbg.stack", "debugger stack" },
	{ "dbg.program", "debugger program (exec only)" },
	{ "dbg.map", "current debugger memory map" },
	{ "dbg.maps[.rwx]", "all debugger memory maps (with matching perm)" },
	{ "io.map", "current io map" },
	{ "io.maps[.rwx]", "all io maps (with matching perm)" },
	{ "io.sky[.rwx]", "all skyline segments (with matching perm)" },
};

static bool cb_search_in(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->value[0] != '?') {
		return true;
	} else if (strlen(node->value) > 1 && node->value[1] == '?') {
		rz_cons_printf("Valid values for search.in (depends on .from/.to and io.va):\n");
		for (size_t i = 0; i < RZ_ARRAY_SIZE(search_in_opts); ++i) {
			rz_cons_printf("%-18s - %s\n", search_in_opts[i].option, search_in_opts[i].description);
		}
	} else {
		print_node_options(node);
	}
	return false;
}

static bool cb_search_show_progress(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	if (rz_str_is_false(node->value)) {
		free(node->value);
		node->value = rz_str_dup("false");
	} else if (!rz_str_cmp_list("num_hits intervals", node->value, ' ')) {
		RZ_LOG_ERROR("search.show_progress: invalid value (%s), supported only `false, num_hits, intervals`\n", node->value);
		return false;
	}
	return true;
}

static bool cb_analysis_in(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->value[0] != '?') {
		core->analysis->opt.noncode = (strchr(node->value, 'x') == NULL);
		return true;
	} else if (strlen(node->value) > 1 && node->value[1] == '?') {
		rz_cons_printf("Valid values for analysis.in (depends on .from/.to and io.va):\n");
		for (size_t i = 0; i < RZ_ARRAY_SIZE(search_in_opts); ++i) {
			rz_cons_printf("%-18s - %s\n", search_in_opts[i].option, search_in_opts[i].description);
		}
	} else {
		print_node_options(node);
	}
	return false;
}

static bool cb_zoom_in(void *user, void *data) {
	RzConfigNode *node = (RzConfigNode *)data;
	if (node->value[0] != '?') {
		return true;
	} else if (strlen(node->value) > 1 && node->value[1] == '?') {
		rz_cons_printf("Valid values for zoom.in (depends on .from/.to and io.va):\n");
		for (size_t i = 0; i < RZ_ARRAY_SIZE(search_in_opts); ++i) {
			rz_cons_printf("%-18s - %s\n", search_in_opts[i].option, search_in_opts[i].description);
		}
	} else {
		print_node_options(node);
	}
	return false;
}

static int __dbg_swstep_getter(void *user, RzConfigNode *node) {
	RzCore *core = (RzCore *)user;
	node->i_value = core->dbg->swstep;
	return true;
}

static bool cb_analysis_from(RzCore *core, RzConfigNode *node) {
	if (rz_config_get_i(core->config, "analysis.limits")) {
		ut64 to = rz_config_get_i(core->config, "analysis.to");
		rz_analysis_set_limits(core->analysis, node->i_value, to);
	}
	return true;
}

static bool cb_analysis_to(RzCore *core, RzConfigNode *node) {
	if (rz_config_get_i(core->config, "analysis.limits")) {
		ut64 from = rz_config_get_i(core->config, "analysis.from");
		rz_analysis_set_limits(core->analysis, from, node->i_value);
	}
	return true;
}

static bool cb_analysis_limits(void *user, RzConfigNode *node) {
	RzCore *core = (RzCore *)user;
	if (node->i_value) {
		rz_analysis_set_limits(core->analysis,
			rz_config_get_i(core->config, "analysis.from"),
			rz_config_get_i(core->config, "analysis.to"));
	} else {
		rz_analysis_unset_limits(core->analysis);
	}
	return true;
}

static bool cb_analysis_rnr(void *user, RzConfigNode *node) {
	RzCore *core = (RzCore *)user;
	core->analysis->recursive_noreturn = node->i_value;
	return true;
}

static bool cb_analysis_jmptbl(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.jmptbl = node->i_value;
	return true;
}

static bool cb_analysis_jmptblmax(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.jmptbl_maxcount = node->i_value;
	return true;
}

static bool cb_analysis_jmptblmaxoffset(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.jmptbl_maxoffset = node->i_value > UT32_MAX ? UT32_MAX : node->i_value;
	return true;
}

static bool cb_analysis_cjmpref(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.cjmpref = node->i_value;
	return true;
}

static bool cb_analysis_jmpref(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.jmpref = node->i_value;
	return true;
}

static bool cb_analysis_jmpabove(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.jmpabove = node->i_value;
	return true;
}

static bool cb_analysis_loads(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.loads = node->i_value;
	return true;
}

static bool cb_analysis_followdatarefs(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.followdatarefs = node->i_value;
	return true;
}

static bool cb_analysis_jmpmid(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.jmpmid = node->i_value;
	return true;
}

static bool cb_analysis_searchstringrefs(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.searchstringrefs = node->i_value;
	return true;
}

static bool cb_analysis_pushret(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.pushret = node->i_value;
	return true;
}

static bool cb_analysis_brokenrefs(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.followbrokenfcnsrefs = node->i_value;
	return true;
}

static bool cb_analysis_trycatch(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.trycatch = node->i_value;
	return true;
}

static bool cb_analysis_bb_max_size(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.bb_max_size = RZ_MIN(node->i_value, RZ_ANALYSIS_BLOCK_MAX_SIZE);
	return true;
}

static bool cb_analysis_fcn_max_size(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->analysis->opt.fcn_max_size = node->i_value;
	return true;
}

static bool cb_analysis_cpp_abi(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;

	if (*node->value == '?') {
		print_node_options(node);
		return false;
	}

	if (*node->value) {
		if (strcmp(node->value, "itanium") == 0) {
			core->analysis->cpp_abi = RZ_ANALYSIS_CPP_ABI_ITANIUM;
			return true;
		} else if (strcmp(node->value, "msvc") == 0) {
			core->analysis->cpp_abi = RZ_ANALYSIS_CPP_ABI_MSVC;
			return true;
		}
		RZ_LOG_ERROR("core: analysis.cpp.abi: cannot find '%s'\n", node->value);
	}
	return false;
}

static bool cb_malloc(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;

	if (node->value) {
		if (!strcmp("jemalloc", node->value) || !strcmp("glibc", node->value)) {
			if (core->dbg) {
				core->dbg->malloc = data;
			}
		}
	}
	return true;
}

static bool cb_log_config_level(void *coreptr, void *nodeptr) {
	RzConfigNode *node = (RzConfigNode *)nodeptr;
	rz_log_set_level(node->i_value);
	return true;
}

#if RZ_BUILD_DEBUG
static bool cb_log_config_abortlevel(void *coreptr, void *nodeptr) {
	RzConfigNode *node = (RzConfigNode *)nodeptr;
	rz_log_set_abortlevel(node->i_value);
	return true;
}
#endif /* RZ_BUILD_DEBUG */

static bool cb_log_file(void *coreptr, void *nodeptr) {
	RzConfigNode *node = (RzConfigNode *)nodeptr;
	if (!rz_log_set_file(node->value)) {
		RZ_LOG_ERROR("log.file: cannot open '%s'\n", node->value);
		return false;
	}
	return true;
}

static bool cb_log_config_show_sources(void *coreptr, void *nodeptr) {
	RzConfigNode *node = (RzConfigNode *)nodeptr;
	const char *value = node->value;
	if (rz_str_is_true(value)) {
		rz_log_set_show_sources(true);
	} else if (rz_str_is_false(value)) {
		rz_log_set_show_sources(false);
	} else {
		return false;
	}
	return true;
}

static bool cb_log_config_colors(void *coreptr, void *nodeptr) {
	RzConfigNode *node = (RzConfigNode *)nodeptr;
	const char *value = node->value;
	if (rz_str_is_true(value)) {
		rz_log_set_colors(true);
	} else if (rz_str_is_false(value)) {
		rz_log_set_colors(false);
	} else {
		return false;
	}
	return true;
}

static bool cb_dbg_verbose(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	RzConfigNode *node = (RzConfigNode *)data;
	core->dbg->verbose = node->i_value;
	return true;
}

static bool cb_flirt(void *user, void *data) {
	rz_return_val_if_fail(data, false);
	RzConfigNode *node = (RzConfigNode *)data;
	if (*node->value == '?') {
		print_node_options(node);
		return false;
	}
	return true;
}

static bool rzil_halt_on_exec(void *user, void *data) {
	rz_return_val_if_fail(data, false);
	RzConfigNode *node = (RzConfigNode *)data;
	if (*node->value == '?') {
		print_node_options(node);
		return false;
	}
	return true;
}

RZ_API int rz_core_config_init(RzCore *core) {
	int i;
	char buf[128], *p, *tmpdir;
	RzConfigNode *n;
	RzConfig *cfg = core->config = rz_config_new(core);
	if (!cfg) {
		return 0;
	}
	cfg->num = core->num;
	/* dir.prefix is used in other modules, set it first */
	{
		char *pfx = rz_sys_getenv("RZ_PREFIX");
		if (!pfx) {
			const char *pfx_const = rz_path_prefix(core->sys_path);
			SETCB("dir.prefix", pfx_const, NULL, "Default prefix rizin was compiled for");
		} else {
			SETCB("dir.prefix", pfx, NULL, "Default prefix rizin was compiled for");
			free(pfx);
		}
	}
#if __ANDROID__
	{ // use dir.home and also adjust check for permissions in directory before choosing a home
		char *h = rz_sys_getenv(RZ_SYS_HOME);
		if (h) {
			if (!strcmp(h, "/")) {
				rz_sys_setenv(RZ_SYS_HOME, "/data/local/tmp");
			}
			free(h);
		}
	}
#endif

	rz_config_lock(cfg, true);
	return true;
}

RZ_API void rz_core_parse_rizinrc(RzCore *r) {
	bool has_debug = rz_sys_getenv_asbool("RZ_DEBUG");
	char *rcfile = rz_sys_getenv("RZ_RCFILE");
	char *homerc = NULL;
	if (!RZ_STR_ISEMPTY(rcfile)) {
		homerc = rcfile;
	} else {
		free(rcfile);
		homerc = rz_path_home_rc();
	}
	if (homerc && rz_file_is_regular(homerc)) {
		if (has_debug) {
			RZ_LOG_INFO("USER CONFIG loaded from %s\n", homerc);
		}
		rz_core_cmd_file(r, homerc);
	}
	free(homerc);
	homerc = rz_path_home_config_rc();
	if (homerc && rz_file_is_regular(homerc)) {
		if (has_debug) {
			RZ_LOG_INFO("USER CONFIG loaded from %s\n", homerc);
		}
		rz_core_cmd_file(r, homerc);
	}
	free(homerc);
	homerc = rz_path_home_config_rcdir();
	if (homerc) {
		if (rz_file_is_directory(homerc)) {
			char *file;
			RzListIter *iter;
			RzList *files = rz_sys_dir(homerc);
			rz_list_foreach (files, iter, file) {
				if (*file != '.') {
					char *path = rz_str_newf("%s/%s", homerc, file);
					if (rz_file_is_regular(path)) {
						if (has_debug) {
							RZ_LOG_INFO("USER CONFIG loaded from %s\n", homerc);
						}
						rz_core_cmd_file(r, path);
					}
					free(path);
				}
			}
			rz_list_free(files);
		}
		free(homerc);
	}
}

/**
 * \brief Get config variable spaces
 * \param core The RzCore instance
 * \param space The config space
 * \return RzList of config variable spaces
 */
RZ_API RZ_OWN RzList /*<char *>*/ *rz_core_config_in_space(RZ_NONNULL RzCore *core, RZ_NULLABLE const char *space) {
	rz_return_val_if_fail(core && core->config, NULL);
	RzList *list = rz_list_new();
	if (!list) {
		return NULL;
	}
	RzConfigNode *node;
	RzListIter *iter;
	rz_list_foreach (core->config->nodes, iter, node) {
		char *name = rz_str_dup(node->name);
		if (!name) {
			continue;
		}
		char *dot = strchr(name, '.');
		if (dot) {
			*dot = 0;
		}

		if (RZ_STR_ISNOTEMPTY(space)) {
			if (0 == strcmp(name, space) && dot && !rz_list_find(list, dot + 1, (RzListComparator)strcmp, NULL)) {
				rz_list_append(list, rz_str_dup(dot + 1));
			}
		} else {
			if (!rz_list_find(list, name, (RzListComparator)strcmp, NULL)) {
				rz_list_append(list, rz_str_dup(name));
			}
		}
		free(name);
	}
	return list;
}
