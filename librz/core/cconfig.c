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

#include "cconfig_arch.c"
#include "cconfig_sign.c"

RZ_API int rz_core_config_init(RzCore *core) {
	RzConfig *cfg = core->config = rz_config_new(core);
	if (!cfg) {
		return 0;
	}
	return core_config_add_arch_variables(core) &&
		core_config_add_sign_variables(core) &&
		// to be removed later.
		true;
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

static bool core_config_in_space_iterator(const char *var_name, const char *desc, ut32 flags, void *user) {
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
