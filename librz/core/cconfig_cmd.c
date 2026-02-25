// SPDX-FileCopyrightText: 2026 deroad <deroad@kumo.xn--q9jyb4c>
// SPDX-FileCopyrightText: 2009-2021 pancake <pancake@nopcode.org>
// SPDX-License-Identifier: LGPL-3.0-only

/**
 * \file: contains all the variables linked to cmd.*
 */

static bool core_config_add_cmd_variables(RzCore *core) {
	RzConfig *cfg = core->config;
	return
		/* cmd variables */
		rz_config_add_s(cfg, "cmd.bp", "Run when a breakpoint is hit", "") &&
		rz_config_add_s(cfg, "cmd.cprompt", "Column visual prompt commands", "") &&
		rz_config_add_s(cfg, "cmd.fcn.delete", "Run when a function is deleted", "") &&
		rz_config_add_s(cfg, "cmd.fcn.new", "Run when new function is analyzed", "") &&
		rz_config_add_s(cfg, "cmd.fcn.rename", "Run when a function is renamed", "") &&
		rz_config_add_s(cfg, "cmd.gprompt", "Graph visual prompt commands", "") &&
		rz_config_add_s(cfg, "cmd.hit", "Command to run on every search hit.", "") &&
		rz_config_add_s(cfg, "cmd.load", "Run when binary is loaded", "") &&
		rz_config_add_s(cfg, "cmd.onsyscall", "Run when a syscall is hit", "") &&
		rz_config_add_s(cfg, "cmd.open", "Run when file is opened", "") &&
		rz_config_add_s(cfg, "cmd.prompt", "Prompt commands", "") &&
		rz_config_add_s(cfg, "cmd.stack", "Command to display the stack in visual debug mode", "") &&
		rz_config_add_s(cfg, "cmd.visual", "Replace current print mode", "") &&
		rz_config_add_s(cfg, "cmd.vprompt", "Visual prompt commands", "") &&

		true;
}

#if 0
	SETCB("cmd.hexcursor", "", &cb_cmd_hexcursor, "If set and cursor is enabled display given pf format string");
	SETCB("cmd.repeat", "false", &cb_cmdrepeat, "Empty command an alias for '..' (repeat last command)");
	SETCB("cmd.times", "", &cb_cmdtimes, "Run when a command is repeated (number prefix)");
	SETICB("cmd.depth", 10, &cb_cmddepth, "Maximum command depth");
	SETICB("cmd.hitinfo", 1, &cb_debug_hitinfo, "Show info when a tracepoint/breakpoint is hit");

	SETCB("cmd.esil.step", "", &cb_cmd_esil_step, "Command to run before performing a step in the emulator");
	SETCB("cmd.esil.stepout", "", &cb_cmd_esil_step_out, "Command to run after performing a step in the emulator");
	SETCB("cmd.esil.mdev", "", &cb_cmd_esil_mdev, "Command to run when memory device address is accessed");
	SETCB("cmd.esil.intr", "", &cb_cmd_esil_intr, "Command to run when an esil interrupt happens");
	SETCB("cmd.esil.trap", "", &cb_cmd_esil_trap, "Command to run when an esil trap happens");
	SETCB("cmd.esil.todo", "", &cb_cmd_esil_todo, "Command to run when the esil instruction contains TODO");
	SETCB("cmd.esil.ioer", "", &cb_cmd_esil_ioer, "Command to run when esil fails to IO (invalid read/write)");
#endif
