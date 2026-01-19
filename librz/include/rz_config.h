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

typedef bool (*RzConfigCallback)(void *user, void *data);

typedef struct rz_config_hold_t RzConfigHold;
typedef struct rz_config_t RzConfig;

#ifdef RZ_API

RZ_API RZ_OWN RzConfig *rz_config_new(RZ_NULLABLE void *user);
RZ_API RZ_OWN RzConfig *rz_config_clone(RZ_NONNULL RzConfig *cfg);
RZ_API void rz_config_free(RZ_NULLABLE RzConfig *cfg);
RZ_API void rz_config_lock(RZ_NONNULL RzConfig *cfg, bool enable_lock);
RZ_API void rz_config_set_readonly(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, bool read_only);

RZ_API bool rz_config_add_string(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, RZ_NULLABLE const char *value);
RZ_API bool rz_config_add_integer(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, ut64 value);
RZ_API bool rz_config_add_boolean(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, bool value);
RZ_API bool rz_config_add_options(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, ...);
RZ_API bool rz_config_add_string_bind(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, RZ_NONNULL RzConfigCallback getter, RZ_NONNULL RzConfigCallback setter);
RZ_API bool rz_config_add_integer_bind(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, RZ_NONNULL RzConfigCallback getter, RZ_NONNULL RzConfigCallback setter);
RZ_API bool rz_config_add_boolean_bind(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *desc, RZ_NONNULL RzConfigCallback getter, RZ_NONNULL RzConfigCallback setter);

RZ_API bool rz_config_set_string(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, RZ_NULLABLE const char *value);
RZ_API bool rz_config_set_integer(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, ut64 value);
RZ_API bool rz_config_set_boolean(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name, bool value);

RZ_API RZ_BORROW const char *rz_config_get_string(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name);
RZ_API ut64 rz_config_get_integer(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name);
RZ_API bool rz_config_get_boolean(RZ_NONNULL RzConfig *cfg, RZ_NONNULL const char *name);

/* hold.c */
RZ_API RzConfigHold *rz_config_hold_new(RZ_NONNULL RzConfig *cfg);
RZ_API bool rz_config_hold_variable(RZ_NONNULL RzConfigHold *hold, ...);
RZ_API void rz_config_hold_restore(RZ_NULLABLE RzConfigHold *hold);
RZ_API void rz_config_hold_free(RZ_NULLABLE RzConfigHold *h);

/* serialize_config.c */
RZ_API void rz_serialize_config_save(RZ_NONNULL Sdb *db, RZ_NONNULL RzConfig *config);
RZ_API bool rz_serialize_config_load(RZ_NONNULL Sdb *db, RZ_NONNULL RzConfig *config, RZ_NULLABLE const char *const *exclude, RZ_NULLABLE RzSerializeResultInfo *res);
#endif

#ifdef __cplusplus
}
#endif

#endif
