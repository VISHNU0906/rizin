// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-FileCopyrightText: 2009-2021 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

/**
 * \file: contains all the variables linked to dir.*
 */

// use dir.home and also adjust check for permissions in directory before choosing a home
#if __ANDROID__
static void core_config_set_default_sys_home(void) {
	char *h = rz_sys_getenv(RZ_SYS_HOME);
	if (RZ_STR_ISEMPTY(h) || RZ_STR_EQ(h, "/")) {
		rz_sys_setenv(RZ_SYS_HOME, "/data/local/tmp");
	}
	free(h);
}
#else
static void core_config_set_default_sys_home(void) {
	char *h = rz_sys_getenv(RZ_SYS_HOME);
	if (RZ_STR_ISEMPTY(h)) {
		rz_sys_setenv(RZ_SYS_HOME, "/");
	}
	free(h);
}
#endif

/* dir.source GET */
static bool core_config_get_string_dir_source(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	*value = core->bin->srcdir;
	return true;
}

/* dir.source SET */
static bool core_config_set_string_dir_source(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const char **value = (const char **)data;

	free(core->bin->srcdir);
	core->bin->srcdir = rz_str_dup(*value);
	return core->bin->srcdir != NULL;
}

static bool core_config_add_dir_variables(RzCore *core) {
	RzConfig *cfg = core->config;

	core_config_set_default_sys_home();

#if __ANDROID__
	const char *dir_projects = "/data/data/org.rizin.rizininstaller/rizin/projects";
#else
	const char *dir_projects = "";
#endif
	const char *pfx_const = rz_path_prefix(core->sys_path);
	char *projects = rz_path_home_prefix(RZ_PROJECTS);
	char *prefix = rz_sys_getenv("RZ_PREFIX");
	char *sys_home = rz_sys_getenv(RZ_SYS_HOME);
	char *sys_temp = rz_sys_getenv(RZ_SYS_TMP);
	char *sdb_magic = rz_path_system(core->sys_path, RZ_SDB_MAGIC);
	char *sys_plugins = rz_path_system(core->sys_path, RZ_PLUGINS);

	if (RZ_STR_ISNOTEMPTY(prefix)) {
		pfx_const = prefix;
	}
	if (RZ_STR_ISNOTEMPTY(dir_projects)) {
		dir_projects = dir_projects;
	}

	bool ok =
		/* dir variables */
		rz_config_add_i(cfg, "dir.depth", "Maximum depth when searching recursively for files", 10) &&
		rz_config_add_s(cfg, "dir.home", "Path for the home directory", sys_home) &&
		rz_config_add_s(cfg, "dir.libs", "Specify path to find libraries to load when bin.libs=true", "") &&
		rz_config_add_s(cfg, "dir.magic", "Path to rz_magic files", sdb_magic) &&
		rz_config_add_s(cfg, "dir.plugins", "Path to plugin files to be loaded at startup", sys_plugins) &&
		rz_config_add_s(cfg, "dir.prefix", "Default prefix rizin was compiled for", pfx_const) &&
		rz_config_add_s(cfg, "dir.projects", "Default path for projects", dir_projects) &&
		rz_config_add_s(cfg, "dir.tmp", "Path of the tmp directory", sys_temp) &&
		rz_config_add_s(cfg, "dir.types", "Default path to look for cparse type files", "/usr/include") &&
		/* dir binds */
		rz_config_add_s_bind(cfg, "dir.source", "Path to find source files", core_config_get_string_dir_source, core_config_set_string_dir_source);

	free(projects);
	free(sys_plugins);
	free(sdb_magic);
	free(sys_temp);
	free(sys_home);
	free(prefix);
	return ok;
}

/*



*/