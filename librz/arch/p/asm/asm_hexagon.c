// SPDX-FileCopyrightText: 2021 Rot127 <rot127@posteo.com>
// SPDX-License-Identifier: LGPL-3.0-only

// LLVM commit: b6f51787f6c8e77143f0aef6b58ddc7c55741d5c
// LLVM commit date: 2023-11-15 07:10:59 -0800 (ISO 8601 format)
// Date of code generation: 2024-03-16 06:22:39-05:00
//========================================
// The following code is generated.
// Do not edit. Repository of code generator:
// https://github.com/rizinorg/rz-hexagon

#include <rz_types.h>
#include <rz_util.h>
#include <rz_asm.h>
#include <rz_lib.h>
#include <rz_util/rz_print.h>
#include <rz_vector.h>
#include <hexagon/hexagon.h>
#include <hexagon/hexagon_insn.h>
#include <hexagon/hexagon_arch.h>

static RZ_OWN RzPVector /*<RzAsmTokenPattern *>*/ *get_token_patterns() {
	RzPVector *pvec = rz_pvector_new(rz_asm_token_pattern_free);

	RzAsmTokenPattern *pat = RZ_NEW0(RzAsmTokenPattern);
	pat->type = RZ_ASM_TOKEN_META;
	pat->pattern = rz_str_dup(
		"^[\\[\\?\\/\\|\\\\\\{┌│└]|" // Packet prefix
		"(∎|[<\\}])[\\s:]endloop[01]{1,2}" // Endloop markers
	);
	rz_pvector_push(pvec, pat);

	pat = RZ_NEW0(RzAsmTokenPattern);
	pat->type = RZ_ASM_TOKEN_META;
	pat->pattern = rz_str_dup(
		"\\#{1,2}|\\}$|" // Immediate prefix, Closing packet bracket
		"\\.new|:n?t|:raw|<err>" // .new and jump hints
	);
	rz_pvector_push(pvec, pat);

	pat = RZ_NEW0(RzAsmTokenPattern);
	pat->type = RZ_ASM_TOKEN_REGISTER;
	pat->pattern = rz_str_dup(
		"[CNPRMQVO]\\d{1,2}(:\\d{1,2})?(in)?|(GP|HTID|UGP|LR|FP|SP|([A-Z]{2,}[0-9]{,1}))" // Registers and double registers
	);
	rz_pvector_push(pvec, pat);

	pat = RZ_NEW0(RzAsmTokenPattern);
	pat->type = RZ_ASM_TOKEN_NUMBER;
	pat->pattern = rz_str_dup(
		"0x(\\d|[abcdef])+" // Hexadecimal numbers
	);
	rz_pvector_push(pvec, pat);

	pat = RZ_NEW0(RzAsmTokenPattern);
	pat->type = RZ_ASM_TOKEN_MNEMONIC;
	pat->pattern = rz_str_dup(
		"\\w+_\\w+|[a-zA-Z]+\\d+[a-zA-Z]*" // Mnemonics with a decimal number in the name.
	);
	rz_pvector_push(pvec, pat);

	pat = RZ_NEW0(RzAsmTokenPattern);
	pat->type = RZ_ASM_TOKEN_NUMBER;
	pat->pattern = rz_str_dup(
		"\\d+" // Decimal numbers
	);
	rz_pvector_push(pvec, pat);

	pat = RZ_NEW0(RzAsmTokenPattern);
	pat->type = RZ_ASM_TOKEN_SEPARATOR;
	pat->pattern = rz_str_dup(
		"\\s+|" // Spaces and tabs
		"[,;\\.\\(\\)\\{\\}:]" // Brackets and others
	);
	rz_pvector_push(pvec, pat);

	pat = RZ_NEW0(RzAsmTokenPattern);
	pat->type = RZ_ASM_TOKEN_OPERATOR;
	pat->pattern = rz_str_dup(
		"[+*&+?=!^\\/|~\\-]{1,2}" // +,-,=,],[, ! (not the packet prefix)
	);
	rz_pvector_push(pvec, pat);

	pat = RZ_NEW0(RzAsmTokenPattern);
	pat->type = RZ_ASM_TOKEN_OPERATOR;
	pat->pattern = rz_str_dup(
		"\\]|\\[|<{1,2}|>{1,2}" // +,-,=,],[, ! (not the packet prefix)
	);
	rz_pvector_push(pvec, pat);

	pat = RZ_NEW0(RzAsmTokenPattern);
	pat->type = RZ_ASM_TOKEN_MNEMONIC;
	pat->pattern = rz_str_dup(
		"\\w+" // Alphanumeric mnemonics
	);
	rz_pvector_push(pvec, pat);

	return pvec;
}

static bool hexagon_fini(void *user) {
	hexagon_state_fini(user);
	free(user);
	return true;
}

#define hexagon_getter_config(name, variable) \
	static bool hexagon_##name(void *user, void *data) { \
		bool *value = data; \
		HexState *state = user; \
		if (!state) { \
			return false; \
		} \
		*value = state->variable; \
		return true; \
	}

#define hexagon_setter_config(name, variable) \
	static bool hexagon_##name(void *user, const void *data) { \
		const bool *value = data; \
		HexState *state = user; \
		if (!state) { \
			return false; \
		} \
		state->variable = *value; \
		return true; \
	}

#define hexagon_config_callbacks(variable) \
	hexagon_setter_config(config_##variable##_set, variable); \
	hexagon_getter_config(config_##variable##_get, variable)

hexagon_config_callbacks(imm_hash);
hexagon_config_callbacks(imm_sign);
hexagon_config_callbacks(sdk);
hexagon_config_callbacks(reg_alias);

static bool hexagon_init(void **plugin_data) {
	HexState *state = hexagon_state_new();
	rz_return_val_if_fail(state, false);

	state->imm_hash = true;
	state->imm_sign = true;
	state->sdk = false;
	state->reg_alias = true;

	if (!state->token_patterns) {
		state->token_patterns = get_token_patterns();
	}
	rz_asm_compile_token_patterns(state->token_patterns);

	*plugin_data = state;
	return true;
}

RZ_API RZ_OWN RzConfig *hexagon_get_config(void *plugin_data) {
	rz_return_val_if_fail(plugin_data, NULL);
	HexState *state = plugin_data;

	RzConfig *cfg = rz_config_new(state);
	if (!cfg) {
		return NULL;
	}

	// Add nodes
	rz_config_add_b_bind(cfg, "plugins.hexagon.imm.hash", "Display ## before 32bit immediates and # before immidiates with other width.", hexagon_config_imm_hash_get, hexagon_config_imm_hash_set);
	rz_config_add_b_bind(cfg, "plugins.hexagon.imm.sign", "True: Print them with sign. False: Print signed immediates in unsigned representation.", hexagon_config_imm_sign_get, hexagon_config_imm_sign_set);
	rz_config_add_b_bind(cfg, "plugins.hexagon.sdk", "Print packet syntax in objdump style.", hexagon_config_sdk_get, hexagon_config_sdk_set);
	rz_config_add_b_bind(cfg, "plugins.hexagon.reg.alias", "Print the alias of registers (Alias from C0 = SA0).", hexagon_config_reg_alias_get, hexagon_config_reg_alias_set);

	return cfg;
}

/**
 * \brief Disassembles a hexagon opcode, write info to op and returns its size.
 *
 * \param a The current RzAsm struct.
 * \param op The RzAsmOp which is be filled with the reversed opcode information.
 * \param buf The buffer with the opcode.
 * \param l The size to read from the buffer.
 * \return int Size of the reversed opcode.
 */
static int disassemble(RzAsm *a, RzAsmOp *op, const ut8 *buf, int l) {
	rz_return_val_if_fail(a && op, -1);
	if (l < HEX_INSN_SIZE) {
		return -1;
	}

	ut32 addr = (ut32)a->pc;
	HexReversedOpcode rev = { .action = HEXAGON_DISAS, .ana_op = NULL, .asm_op = op, .state = NULL, .pkt_fully_decoded = false, .bytes_buf = buf, .bytes_buf_len = l };
	hexagon_reverse_opcode(&rev, addr, a, NULL);
	return op->size;
}

RzAsmPlugin rz_asm_plugin_hexagon = {
	.name = "hexagon",
	.arch = "hexagon",
	.author = "Rot127",
	.license = "LGPL3",
	.cpus = "v79llvm",
	.features = "HVX",
	.bits = 32,
	.desc = "Qualcomm Hexagon (QDSP6) V6",
	.init = &hexagon_init,
	.fini = &hexagon_fini,
	.disassemble = &disassemble,
	.get_config = &hexagon_get_config,
};

#ifndef RZ_PLUGIN_INCORE
RZ_API RzLibStruct rizin_plugin = {
	.type = RZ_LIB_TYPE_ASM,
	.data = &rz_asm_plugin_hexagon
};
#endif
