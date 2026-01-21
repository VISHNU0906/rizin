// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-License-Identifier: LGPL-3.0-only

#ifndef RZ_CONFIG_H
#define RZ_CONFIG_H

#include "rz_types.h"
#include "rz_util.h"
#include "rz_util/rz_serialize.h"

#ifdef __cplusplus
extern "C" {
#endif

RZ_LIB_VERSION_HEADER(rz_config);

typedef enum {
	RZ_CONFIG_VAR_BOOL = 0x00000001,
	RZ_CONFIG_VAR_INT = 0x00000002,
	RZ_CONFIG_VAR_STR = 0x00000004,
	RZ_CONFIG_VAR_LIST = 0x00000008,
	RZ_CONFIG_VAR_WRITABLE = 0x80000000,
} RzConfigVarFlags;

#define rz_config_var_is_bool(flags)     (flags & RZ_CONFIG_VAR_BOOL)
#define rz_config_var_is_int(flags)      (flags & RZ_CONFIG_VAR_INT)
#define rz_config_var_is_str(flags)      (flags & RZ_CONFIG_VAR_STR)
#define rz_config_var_is_writable(flags) (flags & RZ_CONFIG_VAR_WRITABLE)
#define rz_config_var_is_readonly(flags) (!rz_config_var_is_writable(flags))

typedef bool (*RzConfigCallbackGet)(void *user, void *data);
typedef bool (*RzConfigCallbackSet)(void *user, const void *data);
typedef bool (*RzConfigCallbackOpt)(void *user, RzList /*<char *>*/ **data);
typedef bool (*RzConfigIterator)(const char *name, const char *description, ut32 flags, void *user);

typedef struct rz_config_hold_t RzConfigHold;
typedef struct rz_config_t RzConfig;

#define rz_config_get_b rz_config_get_bool
#define rz_config_get_i rz_config_get_integer
#define rz_config_get_s rz_config_get_string

#define rz_config_set_b rz_config_set_bool
#define rz_config_set_i rz_config_set_integer
#define rz_config_set_s rz_config_set_string

#define rz_config_add_b rz_config_add_bool
#define rz_config_add_i rz_config_add_integer
#define rz_config_add_s rz_config_add_string
#define rz_config_add_l rz_config_add_list
#define rz_config_add_o rz_config_add_options

#define rz_config_add_b_bind rz_config_add_bool_bind
#define rz_config_add_i_bind rz_config_add_integer_bind
#define rz_config_add_s_bind rz_config_add_string_bind
#define rz_config_add_l_bind rz_config_add_list_bind
#define rz_config_add_o_bind rz_config_add_options_bind

#ifdef RZ_API

RZ_API RZ_OWN RzConfig *rz_config_new(RZ_NULLABLE void *user);
RZ_API RZ_OWN RzConfig *rz_config_clone(RZ_NONNULL RzConfig *cfg);
RZ_API void rz_config_free(RZ_NULLABLE RzConfig *cfg);
RZ_API void rz_config_lock(RZ_NONNULL RzConfig *cfg, bool enable_lock);
RZ_API void rz_config_set_readonly(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, bool read_only);
RZ_API void rz_config_iterate_vars(RZ_NONNULL const RzConfig *cfg, RZ_NONNULL RzConfigIterator iterator, RZ_NULLABLE void *user);

RZ_API bool rz_config_add_string(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, RZ_NULLABLE const char *value);
RZ_API bool rz_config_add_integer(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, ut64 value);
RZ_API bool rz_config_add_bool(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, bool value);
RZ_API bool rz_config_add_options(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, ...);
RZ_API bool rz_config_add_list(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, ...);

RZ_API bool rz_config_add_string_bind(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, RZ_NONNULL RzConfigCallbackGet getter, RZ_NONNULL RzConfigCallbackSet setter);
RZ_API bool rz_config_add_integer_bind(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, RZ_NONNULL RzConfigCallbackGet getter, RZ_NONNULL RzConfigCallbackSet setter);
RZ_API bool rz_config_add_bool_bind(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, RZ_NONNULL RzConfigCallbackGet getter, RZ_NONNULL RzConfigCallbackSet setter);
RZ_API bool rz_config_add_list_bind(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, RZ_NONNULL RzConfigCallbackGet getter, RZ_NONNULL RzConfigCallbackSet setter);
RZ_API bool rz_config_add_options_bind(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, RZ_NONNULL RzConfigCallbackGet getter, RZ_NONNULL RzConfigCallbackSet setter, RZ_NONNULL RzConfigCallbackOpt options);

RZ_API bool rz_config_set_string(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *value);
RZ_API bool rz_config_set_integer(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, ut64 value);
RZ_API bool rz_config_set_bool(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, bool value);
RZ_API bool rz_config_toggle_bool(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name);
RZ_API bool rz_config_set_list(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const RzList /*<char *>*/ *value);
RZ_API bool rz_config_set_list2(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, ...);
RZ_API bool rz_config_set_any(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *value);

RZ_API RZ_BORROW const char *rz_config_get_string(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name);
RZ_API ut64 rz_config_get_integer(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name);
RZ_API bool rz_config_get_bool(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name);
RZ_API RZ_BORROW const RzList /*<char *>*/ *rz_config_get_list(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name);

RZ_API RZ_OWN char *rz_config_get_as_string(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name);
RZ_API RZ_BORROW const RzList /*<char *>*/ *rz_config_get_options(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name);
RZ_API const ut32 rz_config_get_flags(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name);

/* hold.c */
RZ_API RzConfigHold *rz_config_hold_new(RZ_NONNULL RzConfig *cfg);
RZ_API RzConfigHold *rz_config_hold_new2(RZ_NONNULL RzConfig *cfg, ...);
RZ_API bool rz_config_hold_var(RZ_NONNULL RzConfigHold *hold, ...);
RZ_API void rz_config_hold_free(RZ_NULLABLE RzConfigHold *h);
RZ_API void rz_config_hold_restore(RZ_NULLABLE RzConfigHold *hold);
RZ_API void rz_config_hold_restore_and_free(RZ_NULLABLE RzConfigHold *hold);

/* serialize_config.c */
RZ_API void rz_serialize_config_save(RZ_NONNULL Sdb *db, RZ_NONNULL RzConfig *config);
RZ_API bool rz_serialize_config_load(RZ_NONNULL Sdb *db, RZ_NONNULL RzConfig *config, RZ_NULLABLE const char *const *exclude, RZ_NULLABLE RzSerializeResultInfo *res);

#endif

#ifdef __cplusplus
}
#endif

#endif
