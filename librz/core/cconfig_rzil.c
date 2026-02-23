// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-License-Identifier: LGPL-3.0-only

/**
 * \file: contains all the variables linked to RzIL
 */

static bool core_config_add_rzil_variables(RzCore *core) {
	RzConfig *cfg = core->config;
	return
		/* rzil variables */
		rz_config_add_b(cfg, "rzil.step.events.read", "enables/disables printing aezse read event", false) &&
		rz_config_add_b(cfg, "rzil.step.events.write", "enables/disables printing aezse write event", true) &&
		rz_config_add_l(cfg, "rzil.step.events.halt_on_exc", "Enables/disable exceptions the VM should halt if reached.", "div0", "fp_invalid_op", NULL) &&
		rz_config_set_options2(cfg, "rzil.step.events.halt_on_exc", "div0", "fp_div0", "fp_inexact", "fp_underflow", "fp_overflow", "fp_invalid_op", "none", "all", NULL);
}
