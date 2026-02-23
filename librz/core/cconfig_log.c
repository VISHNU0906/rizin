// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-License-Identifier: LGPL-3.0-only

/**
 * \file: contains all the variables linked to log.*
 */

#if RZ_BUILD_DEBUG

/* log.abortlevel GET */
static bool core_config_get_string_log_abortlevel(void *user, void *data) {
	const char **value = (const char **)data;

	RzLogLevel level = rz_log_get_abortlevel();
	*value = rz_log_level_to_string(level);
	return true;
}

/* log.abortlevel SET */
static bool core_config_set_string_log_abortlevel(void *user, const void *data) {
	const char **value = (const char **)data;

	RzLogLevel level = rz_log_level_from_string(*value);
	rz_log_set_abortlevel(level);
	return false;
}

static bool core_config_setup_abortlevel(RzConfig *cfg) {
	char *rz_abortlevel = rz_sys_getenv("RZ_ABORTLEVEL");
	RzLogLevel level = RZ_DEFAULT_LOGLVL_ABORT;
	if (RZ_STR_ISNOTEMPTY(rz_abortlevel)) {
		level = rz_log_level_from_string(rz_abortlevel);
	}
	rz_log_set_abortlevel(level);
	free(rz_abortlevel);

	return rz_config_add_s_bind(cfg, "log.abortlevel", "Target log level/severity when to abort", core_config_get_string_log_abortlevel, core_config_set_string_log_abortlevel) &&
		rz_config_set_options2(cfg, "log.abortlevel", "debug", "verbose", "info", "warning", "error", "fatal", NULL);
}

#endif /* RZ_BUILD_DEBUG */

/* log.level GET */
static bool core_config_get_string_log_level(void *user, void *data) {
	const char **value = (const char **)data;

	RzLogLevel level = rz_log_get_level();
	*value = rz_log_level_to_string(level);
	return true;
}

/* log.level SET */
static bool core_config_set_string_log_level(void *user, const void *data) {
	const char **value = (const char **)data;

	RzLogLevel level = rz_log_level_from_string(*value);
	rz_log_set_level(level);
	return false;
}

/* log.file GET */
static bool core_config_get_string_log_file(void *user, void *data) {
	const char **value = (const char **)data;

	*value = rz_log_get_file();
	return true;
}

/* log.file SET */
static bool core_config_set_string_log_file(void *user, const void *data) {
	const char **value = (const char **)data;

	return rz_log_set_file(*value);
}

/* log.show.sources GET */
static bool core_config_get_string_log_show_sources(void *user, void *data) {
	bool *value = (bool *)data;

	*value = rz_log_get_show_sources();
	return true;
}

/* log.show.sources SET */
static bool core_config_set_string_log_show_sources(void *user, const void *data) {
	const bool *value = (const bool *)data;

	rz_log_set_show_sources(*value);
	return true;
}

/* log.colors GET */
static bool core_config_get_string_log_colors(void *user, void *data) {
	bool *value = (bool *)data;

	*value = rz_log_get_colors();
	return true;
}

/* log.colors SET */
static bool core_config_set_string_log_colors(void *user, const void *data) {
	const bool *value = (const bool *)data;

	rz_log_set_colors(*value);
	return true;
}

/* log.events GET */
static bool core_config_get_string_log_events(void *user, void *data) {
	RzCore *core = (RzCore *)user;
	bool *value = (bool *)data;

	*value = core->log_events;
	return true;
}

/* log.events SET */
static bool core_config_set_string_log_events(void *user, const void *data) {
	RzCore *core = (RzCore *)user;
	const bool *value = (const bool *)data;

	core->log_events = *value;
	return true;
}

static bool core_config_add_log_variables(RzCore *core) {
	RzConfig *cfg = core->config;
	char *rz_loglevel = rz_sys_getenv("RZ_LOGLEVEL");
	char *rz_logfile = rz_sys_getenv("RZ_LOGFILE");
	char *rz_logshowsources = rz_sys_getenv("RZ_LOGSHOWSOURCES");
	char *rz_logcolors = rz_sys_getenv("RZ_LOGCOLORS");

	RzLogLevel level = rz_log_level_from_string(rz_loglevel);
	rz_log_set_level(level);
	rz_log_set_file(rz_logfile);
	rz_log_set_show_sources(rz_str_is_true(rz_logshowsources));
	rz_log_set_colors(rz_str_is_true(rz_logcolors));

	free(rz_loglevel);
	free(rz_logfile);
	free(rz_logshowsources);
	free(rz_logcolors);

	return
		/* log variables */
		rz_config_add_b_bind(cfg, "log.colors", "Should the log output use colors", core_config_get_string_log_colors, core_config_set_string_log_colors) &&
		rz_config_add_b_bind(cfg, "log.events", "Remote HTTP server to sync events with", core_config_get_string_log_events, core_config_set_string_log_events) &&
		rz_config_add_b_bind(cfg, "log.show.sources", "Should the log output contain src info (filename:lineno)", core_config_get_string_log_show_sources, core_config_set_string_log_show_sources) &&
		rz_config_add_s_bind(cfg, "log.file", "Logging output filename/path", core_config_get_string_log_file, core_config_set_string_log_file) &&
		rz_config_add_s_bind(cfg, "log.level", "Target log level/severity", core_config_get_string_log_level, core_config_set_string_log_level) &&
#if RZ_BUILD_DEBUG
		core_config_setup_abortlevel(cfg) &&
#endif /* RZ_BUILD_DEBUG */
		rz_config_set_options2(cfg, "log.level", "debug", "verbose", "info", "warning", "error", "fatal", NULL);
}
