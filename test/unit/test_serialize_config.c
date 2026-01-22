// SPDX-FileCopyrightText: 2020 Florian Märkl <info@florianmaerkl.de>
// SPDX-License-Identifier: LGPL-3.0-only

#include <rz_config.h>
#include "minunit.h"
#include "test_sdb.h"

static Sdb *ref_db() {
	Sdb *db = sdb_new0();
	sdb_set(db, "somestring", "somevalue");
	sdb_set(db, "somebiggerint", "0x1337");
	sdb_set(db, "somebool", "true");
	sdb_set(db, "somelist", "a,b,c");
	return db;
}

static bool count_it(const RzConfig *conf, const char *name, const char *description, ut32 flags, void *user) {
	size_t *count = (size_t *)user;
	count[0]++;
	return true;
}

bool test_config_save() {
	RzConfig *config = rz_config_new(NULL);
	rz_config_add_s(config, "somestring", NULL, "somevalue");
	rz_config_add_i(config, "somebiggerint", NULL, 0x1337);
	rz_config_add_b(config, "somebool", NULL, true);
	rz_config_add_l(config, "somelist", NULL, "a", "b", "c", NULL);

	Sdb *db = sdb_new0();
	rz_serialize_config_save(db, config);
	rz_config_free(config);

	Sdb *expected = ref_db();
	assert_sdb_eq(db, expected, "config save");
	sdb_free(db);
	sdb_free(expected);
	mu_end;
}

bool test_config_load() {
	RzConfig *config = rz_config_new(NULL);
	rz_config_add_s(config, "somestring", NULL, "wrong");
	rz_config_add_i(config, "somebiggerint", NULL, 0);
	rz_config_add_b(config, "somebool", NULL, false);
	rz_config_add_l(config, "somelist", NULL, "z", "y", NULL);

	Sdb *db = ref_db();
	sdb_set(db, "sneaky", "not part of config");
	bool loaded = rz_serialize_config_load(db, config, NULL, NULL);
	sdb_free(db);
	mu_assert("load success", loaded);

	size_t count = 0;
	rz_config_iterate_vars(config, count_it, &count);
	mu_assert_eq(count, 4, "count after load");

	mu_assert_streq(rz_config_get_s(config, "somestring"), "somevalue", "loaded config string");
	mu_assert_eq_fmt(rz_config_get_i(config, "somebiggerint"), (ut64)0x1337, "loaded config bigger int", "0x%" PFMT64x);
	mu_assert_true(rz_config_get_b(config, "somebool"), "loaded config bool");
	const RzList *list = rz_config_get_l(config, "somelist");
	mu_assert_eq(rz_list_length(list), 3, "loaded config list");
	rz_config_free(config);
	mu_end;
}

bool test_config_load_exclude() {
	static const char *const exclude[] = {
		"somestring",
		"somebiggerint",
		NULL
	};
	RzConfig *config = rz_config_new(NULL);
	rz_config_add_s(config, "somestring", NULL, "someoldvalue");
	rz_config_add_i(config, "somebiggerint", NULL, 123);
	rz_config_add_b(config, "somebool", NULL, false);
	rz_config_add_l(config, "somelist", NULL, "z", "y", NULL);

	Sdb *db = ref_db();
	bool loaded = rz_serialize_config_load(db, config, exclude, NULL);
	sdb_free(db);
	mu_assert("load success", loaded);

	size_t count = 0;
	rz_config_iterate_vars(config, count_it, &count);

	mu_assert_eq(count, 4, "count after load");
	mu_assert_streq(rz_config_get_s(config, "somestring"), "someoldvalue", "excluded config string");
	mu_assert_eq_fmt(rz_config_get_i(config, "somebiggerint"), (ut64)123, "excluded config int", "%" PFMT64u);
	mu_assert_true(rz_config_get_b(config, "somebool"), "loaded config bool");
	const RzList *list = rz_config_get_l(config, "somelist");
	mu_assert_eq(rz_list_length(list), 3, "loaded config list");

	rz_config_free(config);
	mu_end;
}

int all_tests() {
	mu_run_test(test_config_save);
	mu_run_test(test_config_load);
	mu_run_test(test_config_load_exclude);
	return tests_passed != tests_run;
}

mu_main(all_tests)