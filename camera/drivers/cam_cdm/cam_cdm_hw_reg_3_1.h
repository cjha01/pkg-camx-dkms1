/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include "cam_cdm.h"

#define CAM_CDM_PAUSE_CORE_DONE_MASK_3X    (0x1 << 1)

/*
 * CDM 3.x has 10 BL FIFOs and 10 IRQ sets, each at a stride of 0x100.
 * Total BL FIFO entries = 640 (shared across all contexts).
 * Per-context IRQ_USR_DATA registers are at 0x80, 0x180, 0x280 ... 0x980.
 */

/* Per-context usr_data register offsets */
static const uint32_t cdm_3_1_usr_data_ctx0 = 0x80;
static const uint32_t cdm_3_1_usr_data_ctx1 = 0x180;
static const uint32_t cdm_3_1_usr_data_ctx2 = 0x280;
static const uint32_t cdm_3_1_usr_data_ctx3 = 0x380;
static const uint32_t cdm_3_1_usr_data_ctx4 = 0x480;
static const uint32_t cdm_3_1_usr_data_ctx5 = 0x580;
static const uint32_t cdm_3_1_usr_data_ctx6 = 0x680;
static const uint32_t cdm_3_1_usr_data_ctx7 = 0x780;
static const uint32_t cdm_3_1_usr_data_ctx8 = 0x880;
static const uint32_t cdm_3_1_usr_data_ctx9 = 0x980;

/* BL FIFO pending request registers — each covers 2 FIFOs */
struct cam_cdm_bl_pending_req_reg_params cdm_hw_3_1_bl_pending_req0 = {
	.rb_offset        = 0x6c,
	.rb_mask          = 0x3ff,
	.rb_num_fifo      = 0x2,
	.rb_next_fifo_shift = 0x10,
};

struct cam_cdm_bl_pending_req_reg_params cdm_hw_3_1_bl_pending_req1 = {
	.rb_offset        = 0x70,
	.rb_mask          = 0x3ff,
	.rb_num_fifo      = 0x2,
	.rb_next_fifo_shift = 0x10,
};

/* IRQ register sets — 10 contexts, stride 0x100 */
static struct cam_cdm_irq_regs cdm_hw_3_1_irq0 = {
	.irq_mask      = 0x30,
	.irq_clear     = 0x34,
	.irq_clear_cmd = 0x38,
	.irq_set       = 0x3c,
	.irq_set_cmd   = 0x40,
	.irq_status    = 0x44,
};

static struct cam_cdm_irq_regs cdm_hw_3_1_irq1 = {
	.irq_mask      = 0x130,
	.irq_clear     = 0x134,
	.irq_clear_cmd = 0x138,
	.irq_set       = 0x13c,
	.irq_set_cmd   = 0x140,
	.irq_status    = 0x144,
};

static struct cam_cdm_irq_regs cdm_hw_3_1_irq2 = {
	.irq_mask      = 0x230,
	.irq_clear     = 0x234,
	.irq_clear_cmd = 0x238,
	.irq_set       = 0x23c,
	.irq_set_cmd   = 0x240,
	.irq_status    = 0x244,
};

static struct cam_cdm_irq_regs cdm_hw_3_1_irq3 = {
	.irq_mask      = 0x330,
	.irq_clear     = 0x334,
	.irq_clear_cmd = 0x338,
	.irq_set       = 0x33c,
	.irq_set_cmd   = 0x340,
	.irq_status    = 0x344,
};

static struct cam_cdm_irq_regs cdm_hw_3_1_irq4 = {
	.irq_mask      = 0x430,
	.irq_clear     = 0x434,
	.irq_clear_cmd = 0x438,
	.irq_set       = 0x43c,
	.irq_set_cmd   = 0x440,
	.irq_status    = 0x444,
};

static struct cam_cdm_irq_regs cdm_hw_3_1_irq5 = {
	.irq_mask      = 0x530,
	.irq_clear     = 0x534,
	.irq_clear_cmd = 0x538,
	.irq_set       = 0x53c,
	.irq_set_cmd   = 0x540,
	.irq_status    = 0x544,
};

static struct cam_cdm_irq_regs cdm_hw_3_1_irq6 = {
	.irq_mask      = 0x630,
	.irq_clear     = 0x634,
	.irq_clear_cmd = 0x638,
	.irq_set       = 0x63c,
	.irq_set_cmd   = 0x640,
	.irq_status    = 0x644,
};

static struct cam_cdm_irq_regs cdm_hw_3_1_irq7 = {
	.irq_mask      = 0x730,
	.irq_clear     = 0x734,
	.irq_clear_cmd = 0x738,
	.irq_set       = 0x73c,
	.irq_set_cmd   = 0x740,
	.irq_status    = 0x744,
};

static struct cam_cdm_irq_regs cdm_hw_3_1_irq8 = {
	.irq_mask      = 0x830,
	.irq_clear     = 0x834,
	.irq_clear_cmd = 0x838,
	.irq_set       = 0x83c,
	.irq_set_cmd   = 0x840,
	.irq_status    = 0x844,
};

static struct cam_cdm_irq_regs cdm_hw_3_1_irq9 = {
	.irq_mask      = 0x930,
	.irq_clear     = 0x934,
	.irq_clear_cmd = 0x938,
	.irq_set       = 0x93c,
	.irq_set_cmd   = 0x940,
	.irq_status    = 0x944,
};

/* BL FIFO register sets — 10 contexts, stride 0x100 */
static struct cam_cdm_bl_fifo_regs cdm_hw_3_1_bl_fifo0 = {
	.bl_fifo_base  = 0x50,
	.bl_fifo_len   = 0x54,
	.bl_fifo_store = 0x58,
	.bl_fifo_cfg   = 0x5c,
};

static struct cam_cdm_bl_fifo_regs cdm_hw_3_1_bl_fifo1 = {
	.bl_fifo_base  = 0x150,
	.bl_fifo_len   = 0x154,
	.bl_fifo_store = 0x158,
	.bl_fifo_cfg   = 0x15c,
};

static struct cam_cdm_bl_fifo_regs cdm_hw_3_1_bl_fifo2 = {
	.bl_fifo_base  = 0x250,
	.bl_fifo_len   = 0x254,
	.bl_fifo_store = 0x258,
	.bl_fifo_cfg   = 0x25c,
};

static struct cam_cdm_bl_fifo_regs cdm_hw_3_1_bl_fifo3 = {
	.bl_fifo_base  = 0x350,
	.bl_fifo_len   = 0x354,
	.bl_fifo_store = 0x358,
	.bl_fifo_cfg   = 0x35c,
};

static struct cam_cdm_bl_fifo_regs cdm_hw_3_1_bl_fifo4 = {
	.bl_fifo_base  = 0x450,
	.bl_fifo_len   = 0x454,
	.bl_fifo_store = 0x458,
	.bl_fifo_cfg   = 0x45c,
};

static struct cam_cdm_bl_fifo_regs cdm_hw_3_1_bl_fifo5 = {
	.bl_fifo_base  = 0x550,
	.bl_fifo_len   = 0x554,
	.bl_fifo_store = 0x558,
	.bl_fifo_cfg   = 0x55c,
};

static struct cam_cdm_bl_fifo_regs cdm_hw_3_1_bl_fifo6 = {
	.bl_fifo_base  = 0x650,
	.bl_fifo_len   = 0x654,
	.bl_fifo_store = 0x658,
	.bl_fifo_cfg   = 0x65c,
};

static struct cam_cdm_bl_fifo_regs cdm_hw_3_1_bl_fifo7 = {
	.bl_fifo_base  = 0x750,
	.bl_fifo_len   = 0x754,
	.bl_fifo_store = 0x758,
	.bl_fifo_cfg   = 0x75c,
};

static struct cam_cdm_bl_fifo_regs cdm_hw_3_1_bl_fifo8 = {
	.bl_fifo_base  = 0x850,
	.bl_fifo_len   = 0x854,
	.bl_fifo_store = 0x858,
	.bl_fifo_cfg   = 0x85c,
};

static struct cam_cdm_bl_fifo_regs cdm_hw_3_1_bl_fifo9 = {
	.bl_fifo_base  = 0x950,
	.bl_fifo_len   = 0x954,
	.bl_fifo_store = 0x958,
	.bl_fifo_cfg   = 0x95c,
};

/* Scratch registers (12, same offsets as 2.x) */
static struct cam_cdm_scratch_reg cdm_3_1_scratch_reg0  = { .scratch_reg = 0x90 };
static struct cam_cdm_scratch_reg cdm_3_1_scratch_reg1  = { .scratch_reg = 0x94 };
static struct cam_cdm_scratch_reg cdm_3_1_scratch_reg2  = { .scratch_reg = 0x98 };
static struct cam_cdm_scratch_reg cdm_3_1_scratch_reg3  = { .scratch_reg = 0x9c };
static struct cam_cdm_scratch_reg cdm_3_1_scratch_reg4  = { .scratch_reg = 0xa0 };
static struct cam_cdm_scratch_reg cdm_3_1_scratch_reg5  = { .scratch_reg = 0xa4 };
static struct cam_cdm_scratch_reg cdm_3_1_scratch_reg6  = { .scratch_reg = 0xa8 };
static struct cam_cdm_scratch_reg cdm_3_1_scratch_reg7  = { .scratch_reg = 0xac };
static struct cam_cdm_scratch_reg cdm_3_1_scratch_reg8  = { .scratch_reg = 0xb0 };
static struct cam_cdm_scratch_reg cdm_3_1_scratch_reg9  = { .scratch_reg = 0xb4 };
static struct cam_cdm_scratch_reg cdm_3_1_scratch_reg10 = { .scratch_reg = 0xb8 };
static struct cam_cdm_scratch_reg cdm_3_1_scratch_reg11 = { .scratch_reg = 0xbc };

/* Performance monitor registers (2 sets, same offsets as 2.x) */
static struct cam_cdm_perf_mon_regs cdm_3_1_perf_mon0 = {
	.perf_mon_ctrl = 0x110,
	.perf_mon_0    = 0x114,
	.perf_mon_1    = 0x118,
	.perf_mon_2    = 0x11c,
};

static struct cam_cdm_perf_mon_regs cdm_3_1_perf_mon1 = {
	.perf_mon_ctrl = 0x120,
	.perf_mon_0    = 0x124,
	.perf_mon_1    = 0x128,
	.perf_mon_2    = 0x12c,
};

/* Composite wait status registers */
static struct cam_cdm_comp_wait_status cdm_3_1_comp_wait_status0 = {
	.comp_wait_status = 0x88,
};

static struct cam_cdm_comp_wait_status cdm_3_1_comp_wait_status1 = {
	.comp_wait_status = 0x8c,
};

/* ICL (Invalid Command Log) registers — same offsets as 2.x */
static struct cam_cdm_icl_data_regs cdm_3_1_icl_data = {
	.icl_last_data_0 = 0x1c0,
	.icl_last_data_1 = 0x1c4,
	.icl_last_data_2 = 0x1c8,
	.icl_inv_data    = 0x1cc,
};

static struct cam_cdm_icl_misc_regs cdm_3_1_icl_misc = {
	.icl_inv_bl_addr = 0x1d0,
	.icl_status      = 0x1d8,
};

static struct cam_cdm_icl_regs cdm_3_1_icl = {
	.data_regs = &cdm_3_1_icl_data,
	.misc_regs = &cdm_3_1_icl_misc,
};

/*
 * Common register map for CDM 3.1
 *
 * Key differences from CDM 2.2:
 *  - current_bl_cid (0xF8): CID moved out of current_bl_len[23:22]
 *  - arb_priority_cfg0/1 (0xE40/0xE44): priority config moved from CORE_CFG[23:20]
 *  - usr_data_ctx[]: per-context IRQ_USR_DATA registers
 *  - priority_group_bit_offset = 0 (unused — priority via arb_priority_cfg)
 */
static struct cam_cdm_common_regs cdm_hw_3_1_cmn_reg_offset = {
	.cdm_hw_version        = 0x0,
	.cam_version           = NULL,
	.rst_cmd               = 0x10,
	.cgc_cfg               = 0x14,
	.core_cfg              = 0x18,
	.core_en               = 0x1c,
	.fe_cfg                = 0x20,
	.cdm_status            = 0x28,
	.irq_context_status    = 0x2c,
	.bl_fifo_rb            = 0x60,
	.bl_fifo_base_rb       = 0x64,
	.bl_fifo_len_rb        = 0x68,
	.usr_data              = 0x80,
	.wait_status           = 0x84,
	.last_ahb_addr         = 0xd0,
	.last_ahb_data         = 0xd4,
	.core_debug            = 0xd8,
	.last_ahb_err_addr     = 0xe0,
	.last_ahb_err_data     = 0xe4,
	.current_bl_base       = 0xe8,
	.current_bl_len        = 0xec,
	.current_used_ahb_base = 0xf0,
	.debug_status          = 0xf4,
	.current_bl_cid        = 0xf8,
	.bus_misr_cfg0         = 0x100,
	.bus_misr_cfg1         = 0x104,
	.bus_misr_rd_val       = 0x108,
	.arb_priority_cfg0     = 0xe40,
	.arb_priority_cfg1     = 0xe44,
	.pending_req = {
		&cdm_hw_3_1_bl_pending_req0,
		&cdm_hw_3_1_bl_pending_req1,
	},
	.comp_wait = {
		&cdm_3_1_comp_wait_status0,
		&cdm_3_1_comp_wait_status1,
	},
	.perf_mon = {
		&cdm_3_1_perf_mon0,
		&cdm_3_1_perf_mon1,
	},
	.scratch = {
		&cdm_3_1_scratch_reg0,
		&cdm_3_1_scratch_reg1,
		&cdm_3_1_scratch_reg2,
		&cdm_3_1_scratch_reg3,
		&cdm_3_1_scratch_reg4,
		&cdm_3_1_scratch_reg5,
		&cdm_3_1_scratch_reg6,
		&cdm_3_1_scratch_reg7,
		&cdm_3_1_scratch_reg8,
		&cdm_3_1_scratch_reg9,
		&cdm_3_1_scratch_reg10,
		&cdm_3_1_scratch_reg11,
	},
	.perf_reg  = NULL,
	.icl_reg   = &cdm_3_1_icl,
	.spare     = 0xe7c,
	.priority_group_bit_offset = 0,
	.usr_data_ctx = {
		&cdm_3_1_usr_data_ctx0,
		&cdm_3_1_usr_data_ctx1,
		&cdm_3_1_usr_data_ctx2,
		&cdm_3_1_usr_data_ctx3,
		&cdm_3_1_usr_data_ctx4,
		&cdm_3_1_usr_data_ctx5,
		&cdm_3_1_usr_data_ctx6,
		&cdm_3_1_usr_data_ctx7,
		&cdm_3_1_usr_data_ctx8,
		&cdm_3_1_usr_data_ctx9,
	},
};

static struct cam_cdm_common_reg_data cdm_hw_3_1_cmn_reg_data = {
	.num_bl_fifo             = 0xA,
	.num_bl_fifo_irq         = 0xA,
	.num_bl_pending_req_reg  = 0x2,
	.num_scratch_reg         = 0xc,
	.has_per_ctx_usr_data    = true,
};

struct cam_cdm_hw_reg_offset cam_cdm_3_1_reg_offset = {
	.cmn_reg     = &cdm_hw_3_1_cmn_reg_offset,
	.bl_fifo_reg = {
		&cdm_hw_3_1_bl_fifo0,
		&cdm_hw_3_1_bl_fifo1,
		&cdm_hw_3_1_bl_fifo2,
		&cdm_hw_3_1_bl_fifo3,
		&cdm_hw_3_1_bl_fifo4,
		&cdm_hw_3_1_bl_fifo5,
		&cdm_hw_3_1_bl_fifo6,
		&cdm_hw_3_1_bl_fifo7,
		&cdm_hw_3_1_bl_fifo8,
		&cdm_hw_3_1_bl_fifo9,
	},
	.irq_reg = {
		&cdm_hw_3_1_irq0,
		&cdm_hw_3_1_irq1,
		&cdm_hw_3_1_irq2,
		&cdm_hw_3_1_irq3,
		&cdm_hw_3_1_irq4,
		&cdm_hw_3_1_irq5,
		&cdm_hw_3_1_irq6,
		&cdm_hw_3_1_irq7,
		&cdm_hw_3_1_irq8,
		&cdm_hw_3_1_irq9,
	},
	.reg_data = &cdm_hw_3_1_cmn_reg_data,
};
