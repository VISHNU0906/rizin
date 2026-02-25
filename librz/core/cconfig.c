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

#define CONFIG_IS_HELP(str) RZ_STR_EQ(str, "?")

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

static const RzBinInfo *core_config_get_bin_info(RzCore *core) {
	RzBinObject *obj = rz_bin_cur_object(core->bin);
	if (!obj) {
		return NULL;
	}

	return rz_bin_object_get_info(obj);
}

static void check_reload_bin_str_search(RzCore *core) {
	if (!core->bin || !rz_config_get_b(core->config, "str.search.reload")) {
		return;
	}

	RzBinFile *bf = rz_bin_cur(core->bin);
	if (bf && bf->o) {
		rz_bin_object_reset_strings(core->bin, bf, bf->o);
	}
}
static bool core_config_disabled_set(void *user, const void *data) {
	return false;
}

#include "cconfig_arch.c"
#include "cconfig_basefind.c"
#include "cconfig_cmd.c"
#include "cconfig_conf.c"
#include "cconfig_dbg.c"
#include "cconfig_dir.c"
#include "cconfig_elf.c"
#include "cconfig_file.c"
#include "cconfig_key.c"
#include "cconfig_log.c"
#include "cconfig_pdb.c"
#include "cconfig_prj.c"
#include "cconfig_rop.c"
#include "cconfig_rzil.c"
#include "cconfig_screen.c"
#include "cconfig_sign.c"
#include "cconfig_various.c"
#include "cconfig_zoom.c"

typedef struct core_config_print_s {
	RzCmdStateOutput *state;
	const char *str;
	char color_name[32];
	char color_value[32];
	char color_meta[32];
	char reset_str[32];
} CoreConfigPrint;

static void core_config_print_array_as_json(PJ *pj, const char *key, const RzList /*<char *>*/ *list, bool allow_empty) {
	const char *entry;
	const RzListIter *it;
	if (rz_list_empty(list) && !allow_empty) {
		return;
	}
	pj_ka(pj, key);
	rz_list_foreach (list, it, entry) {
		pj_s(pj, entry);
	}
	pj_end(pj);
}

static void core_config_print_var_as_json(PJ *pj, const RzConfig *conf, const char *var_name, ut32 flags) {
	if (rz_config_var_is_int(flags)) {
		pj_kn(pj, var_name, rz_config_get_i(conf, var_name));
	} else if (rz_config_var_is_bool(flags)) {
		pj_kb(pj, var_name, rz_config_get_b(conf, var_name));
	} else if (rz_config_var_is_list(flags)) {
		const RzList *list = rz_config_get_l(conf, var_name);
		core_config_print_array_as_json(pj, var_name, list, true);
	} else {
		const char *value = rz_config_get_s(conf, var_name);
		pj_ks(pj, var_name, value);
	}
}

static void core_config_print_array_as_string(const RzList /*<char *>*/ *list, bool allow_empty) {
	const char *entry;
	const RzListIter *it;
	if (rz_list_empty(list) && !allow_empty) {
		return;
	}

	rz_cons_print("[");
	rz_list_foreach (list, it, entry) {
		if (rz_list_head(list) != it) {
			rz_cons_printf(", %s", entry);
		} else {
			rz_cons_print(entry);
		}
	}
	rz_cons_print("]");
}

static void core_config_print_var_as_string(const RzConfig *conf, const char *var_name, ut32 flags) {
	if (rz_config_var_is_int(flags)) {
		ut64 value = rz_config_get_i(conf, var_name);
		if (value > 0x1000) {
			rz_cons_printf("0x%" PFMT64x, value);
		} else {
			rz_cons_printf("%" PFMT64u, value);
		}
	} else if (rz_config_var_is_bool(flags)) {
		bool value = rz_config_get_b(conf, var_name);
		rz_cons_print(rz_str_bool(value));
	} else if (rz_config_var_is_list(flags)) {
		const RzList *list = rz_config_get_l(conf, var_name);
		core_config_print_array_as_string(list, true);
	} else {
		const char *value = rz_config_get_s(conf, var_name);
		rz_cons_print(value);
	}
}

static bool core_config_print_iterator(const RzConfig *conf, const char *var_name, const char *desc, ut32 flags, void *user) {
	CoreConfigPrint *ccp = (CoreConfigPrint *)user;
	if (RZ_STR_ISNOTEMPTY(ccp->str) && !rz_str_startswith(var_name, ccp->str)) {
		return true;
	}

	RzOutputMode mode = ccp->state->mode;
	PJ *pj = ccp->state->d.pj;

	char *es = NULL;
	const RzList *opts = rz_config_get_options(conf, var_name);
	const char *vtype = rz_config_get_type(conf, var_name);

	switch (mode) {
	case RZ_OUTPUT_MODE_JSON:
		core_config_print_var_as_json(pj, conf, var_name, flags);
		return true;
		break;
	case RZ_OUTPUT_MODE_LONG_JSON:
		pj_o(pj);
		pj_ks(pj, "name", var_name);
		core_config_print_var_as_json(pj, conf, var_name, flags);
		pj_ks(pj, "type", vtype);
		es = rz_str_escape(desc);
		if (es) {
			pj_ks(pj, "desc", es);
			free(es);
		}
		pj_kb(pj, "ro", rz_config_var_is_readonly(flags));
		core_config_print_array_as_json(pj, "options", opts, false);
		pj_end(pj);
		break;
	case RZ_OUTPUT_MODE_LONG: {
		const char *ro_str = rz_config_var_is_readonly(flags) ? "ro-" : "";
		rz_cons_printf("%s%20s = %s", ccp->color_name, var_name, ccp->color_value);
		core_config_print_var_as_string(conf, var_name, flags);
		rz_cons_printf(" %s(%s%s); %s%s ", ccp->color_meta, ro_str, vtype, ccp->reset_str, desc);
		core_config_print_array_as_string(opts, false);
		rz_cons_println("");
		break;
	}
	case RZ_OUTPUT_MODE_QUIET:
		rz_cons_printf("%s=", var_name);
		core_config_print_var_as_string(conf, var_name, flags);
		rz_cons_println("");
		break;
	case RZ_OUTPUT_MODE_STANDARD: {
		rz_cons_printf("%s%20s: %s%s\n", ccp->color_name, var_name, ccp->reset_str, desc);
		break;
	}
	case RZ_OUTPUT_MODE_STR_BUF:
		rz_strbuf_appendf(ccp->state->d.sbuf, "%20s: %10s - %s\n", var_name, vtype, desc);
		break;
	default:
		rz_warn_if_reached();
		break;
	}

	return true;
}

/**
 * \brief Prints the configuation variables with their description and its values
 *
 * \param cfg reference to RzConfig
 * \param str reference to the key that can be passed to filter the output
 * \param state reference to RzCmdStateOutput
 */
RZ_IPI void rz_core_config_print_all(RzConfig *cfg, const char *str, RzCmdStateOutput *state) {
	rz_return_if_fail(cfg);

	CoreConfigPrint ccp = { 0 };
	ccp.state = state;
	ccp.str = str;

	PJ *pj = state->d.pj;
	const bool color_enabled = rz_config_get_i(cfg, "scr.color") > 0;

	switch (state->mode) {
	case RZ_OUTPUT_MODE_LONG_JSON:
		pj_a(pj);
		break;
	case RZ_OUTPUT_MODE_JSON:
		pj_o(pj);
		break;
	case RZ_OUTPUT_MODE_LONG:
		if (color_enabled) {
			RzColor color_name_val = rz_cons_pal_get("label");
			RzColor color_value_val = rz_cons_pal_get("args");
			RzColor color_meta_val = rz_cons_pal_get("comment");
			RzColor reset_val = rz_cons_pal_get("help");
			rz_cons_rgb_str(ccp.color_name, sizeof(ccp.color_name), &color_name_val);
			rz_cons_rgb_str(ccp.color_value, sizeof(ccp.color_value), &color_value_val);
			rz_cons_rgb_str(ccp.color_meta, sizeof(ccp.color_meta), &color_meta_val);
			rz_cons_rgb_str(ccp.reset_str, sizeof(ccp.reset_str), &reset_val);
		}
		break;
	case RZ_OUTPUT_MODE_STANDARD:
		if (color_enabled) {
			RzColor color_val = rz_cons_pal_get("label");
			RzColor reset_val = rz_cons_pal_get("help");
			rz_cons_rgb_str(ccp.color_name, sizeof(ccp.color_name), &color_val);
			rz_cons_rgb_str(ccp.reset_str, sizeof(ccp.reset_str), &reset_val);
		}
		break;
	default:
		break;
	}

	rz_config_iterate_vars(cfg, core_config_print_iterator, &ccp);

	if (state->mode == RZ_OUTPUT_MODE_LONG_JSON || state->mode == RZ_OUTPUT_MODE_JSON) {
		pj_end(pj);
	}
}

RZ_API int rz_core_config_init(RzCore *core) {
	RzConfig *cfg = core->config = rz_config_new(core);
	if (!cfg) {
		return 0;
	}

	bool ok = core_config_add_dir_variables(core) &&
		core_config_add_cfg_variables(core) &&
		core_config_add_log_variables(core) &&
		core_config_add_arch_variables(core) &&
		core_config_add_scr_variables(core) &&
		core_config_add_pdb_variables(core) &&
		core_config_add_elf_variables(core) &&
		core_config_add_dbg_variables(core) &&
		core_config_add_cmd_variables(core) &&
		core_config_add_prj_variables(core) &&
		core_config_add_basefind_variables(core) &&
		core_config_add_rzil_variables(core) &&
		core_config_add_file_variables(core) &&
		core_config_add_rop_variables(core) &&
		core_config_add_sign_variables(core) &&
		core_config_add_key_variables(core) &&
		core_config_add_zoom_variables(core) &&
		core_config_add_various_variables(core);
	if (!ok) {
		RZ_LOG_ERROR("failed to initialize core config\n");
		return false;
	}
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

typedef struct core_config_in_space_s {
	const char *space;
	RzList /*<char *>*/ *list;
} CoreConfigInSpace;

static bool core_config_in_space_iterator(const RzConfig *conf, const char *var_name, const char *desc, ut32 flags, void *user) {
	CoreConfigInSpace *ccis = (CoreConfigInSpace *)user;

	char *name = rz_str_dup(var_name);
	if (!name) {
		return false;
	}

	char *dot = strchr(name, '.');
	if (dot) {
		*dot = 0;
	}

	if (RZ_STR_ISNOTEMPTY(ccis->space)) {
		if (0 == strcmp(name, ccis->space) && dot && !rz_list_find(ccis->list, dot + 1, (RzListComparator)strcmp, NULL)) {
			rz_list_append(ccis->list, rz_str_dup(dot + 1));
		}
	} else {
		if (!rz_list_find(ccis->list, name, (RzListComparator)strcmp, NULL)) {
			rz_list_append(ccis->list, rz_str_dup(name));
		}
	}
	free(name);

	return true;
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

	CoreConfigInSpace ctx = { space, list };

	rz_config_iterate_vars(core->config, core_config_in_space_iterator, &ctx);

	return list;
}
