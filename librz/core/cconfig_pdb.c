// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-FileCopyrightText: 2009-2021 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

/**
 * \file: contains all the variables linked to RzPdb
 */

static bool core_config_add_pdb_variables(RzCore *core) {
	RzConfig *cfg = core->config;
	char *pdb_path = rz_path_home_prefix(RZ_PDB);

	bool ok =
		/* pdb variables */
		rz_config_add_s(cfg, "pdb.server", "Semi-colon separated list of base URLs for Microsoft symbol servers", "https://msdl.microsoft.com/download/symbols") &&
		rz_config_add_s(cfg, "pdb.symstore", "Path to downstream symbol store", pdb_path) &&
		rz_config_add_i(cfg, "pdb.extract", "Avoid extract of the pdb file, just download", 1) &&
		rz_config_add_b(cfg, "pdb.autoload", "Automatically load the required pdb files for loaded DLLs", false);

	free(pdb_path);
	return ok;
}
