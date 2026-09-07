/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef _CAM_VFE_LITE90X_H_
#define _CAM_VFE_LITE90X_H_
#include "cam_vfe_camif_ver3.h"
#include "cam_vfe_top_ver4.h"
#include "cam_vfe_core.h"
#include "cam_vfe_bus_ver3.h"
#include "cam_irq_controller.h"

#define CAM_VFE_90X_NUM_DBG_REG 4


static struct cam_vfe_top_ver4_wr_client_desc vfe90x_wr_client_desc[] = {
	{
		.wm_id = 0,
		.desc = "RDI_0",
	},
	{
		.wm_id = 1,
		.desc = "RDI_1",
	},
	{
		.wm_id = 2,
		.desc = "RDI_2",
	},
	{
		.wm_id = 3,
		.desc = "RDI_3",
	},
	{
		.wm_id = 4,
		.desc = "RDI_4",
	},
	{
		.wm_id = 5,
		.desc = "RDI_5",
	},
	{
		.wm_id = 6,
		.desc = "RDI_6",
	},
	{
		.wm_id = 7,
		.desc = "RDI_7",
	},
};

static struct cam_irq_register_set vfe90x_top_irq_reg_set[2] = {
	{
		.mask_reg_offset   = 0x00000028,
		.clear_reg_offset  = 0x00000030,
		.status_reg_offset = 0x00000038,
		.test_set_val      = BIT(0),
		.test_sub_val      = BIT(0),
	},
	{
		.mask_reg_offset   = 0x0000002C,
		.clear_reg_offset  = 0x00000034,
		.status_reg_offset = 0x0000003C,
	},
};

static struct cam_irq_controller_reg_info vfe90x_top_irq_reg_info = {
	.num_registers = 2,
	.irq_reg_set = vfe90x_top_irq_reg_set,
	.global_irq_cmd_offset = 0x00000024,
	.global_clear_bitmask  = 0x00000001,
	.global_set_bitmask    = 0x00000010,
	.clear_all_bitmask     = 0xFFFFFFFF,
};

static uint32_t vfe90x_top_debug_reg[] = {
	0x000000E4,
	0x000000E8,
	0x000000EC,
	0x000000F0,
};

static struct cam_vfe_top_ver4_reg_offset_common vfe90x_top_common_reg = {
	.hw_version               = 0x00000000,
	.hw_capability            = 0x00000004,
	.diag_config              = 0x00000010,
	.diag_sensor_status_0     = 0x00000014,
	.diag_sensor_status_1     = 0x00000018,
	.bus_violation_status     = 0x00000464,
	.bus_overflow_status      = 0x00000468,
	.top_debug_cfg            = 0x000000E0,
	.num_top_debug_reg        = CAM_VFE_90X_NUM_DBG_REG,
	.top_debug                = vfe90x_top_debug_reg,
	.frame_timing_irq_reg_idx = CAM_IFE_IRQ_CAMIF_REG_STATUS1,
};

static struct cam_vfe_ver4_path_reg_data vfe90x_rdi_reg_data[8] = {
	{
		.sof_irq_mask                    = 0x1,
		.eof_irq_mask                    = 0x2,
		.error_irq_mask                  = 0x0,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 0x3,
	},
	{
		.sof_irq_mask                    = 0x4,
		.eof_irq_mask                    = 0x8,
		.error_irq_mask                  = 0x0,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 0x3,
	},
	{
		.sof_irq_mask                    = 0x10,
		.eof_irq_mask                    = 0x20,
		.error_irq_mask                  = 0x0,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 0x3,
	},
	{
		.sof_irq_mask                    = 0x40,
		.eof_irq_mask                    = 0x80,
		.error_irq_mask                  = 0x0,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 0x3,
	},
	{
		.sof_irq_mask                    = 0x100,
		.eof_irq_mask                    = 0x200,
		.error_irq_mask                  = 0x0,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 0x3,
	},
	{
		.sof_irq_mask                    = 0x400,
		.eof_irq_mask                    = 0x800,
		.error_irq_mask                  = 0x0,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 0x3,
	},
	{
		/*
		 * RDI6 - SOF/EOF bit positions per TOP_FUNC_IRQ_MASK_1
		 * (RDI6_SOF = bit 12, RDI6_EOF = bit 13), confirmed via
		 * ip_catalog swi_get_module_details field metadata.
		 */
		.sof_irq_mask                    = 0x1000,
		.eof_irq_mask                    = 0x2000,
		.error_irq_mask                  = 0x0,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 0x3,
	},
	{
		/* RDI7 - RDI7_SOF = bit 14, RDI7_EOF = bit 15 */
		.sof_irq_mask                    = 0x4000,
		.eof_irq_mask                    = 0x8000,
		.error_irq_mask                  = 0x0,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 0x3,
	},
};

static struct cam_vfe_ver4_path_hw_info
	vfe90x_rdi_hw_info[] = {
	{
		.common_reg     = &vfe90x_top_common_reg,
		.reg_data       = &vfe90x_rdi_reg_data[0],
	},
	{
		.common_reg     = &vfe90x_top_common_reg,
		.reg_data       = &vfe90x_rdi_reg_data[1],
	},
	{
		.common_reg     = &vfe90x_top_common_reg,
		.reg_data       = &vfe90x_rdi_reg_data[2],
	},
	{
		.common_reg     = &vfe90x_top_common_reg,
		.reg_data       = &vfe90x_rdi_reg_data[3],
	},
	{
		.common_reg     = &vfe90x_top_common_reg,
		.reg_data       = &vfe90x_rdi_reg_data[4],
	},
	{
		.common_reg     = &vfe90x_top_common_reg,
		.reg_data       = &vfe90x_rdi_reg_data[5],
	},
	{
		.common_reg     = &vfe90x_top_common_reg,
		.reg_data       = &vfe90x_rdi_reg_data[6],
	},
	{
		.common_reg     = &vfe90x_top_common_reg,
		.reg_data       = &vfe90x_rdi_reg_data[7],
	},
};

static struct cam_vfe_top_ver4_debug_reg_info vfe90x_dbg_reg_info[CAM_VFE_90X_NUM_DBG_REG][8] = {
	VFE_DBG_INFO_ARRAY_4bit(
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved"
	),
	{
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
	},
	VFE_DBG_INFO_ARRAY_4bit(
		"csid_ccif_rdi0_vld_out",
		"csid_ccif_rdi1_vld_out",
		"csid_ccif_rdi3_vld_out",
		"csid_ccif_rdi4_vld_out",
		"csid_ccif_rdi5_vld_out",
		"unused",
		"unused",
		"unused"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"unused",
		"unused",
		"unused",
		"unused",
		"unused",
		"unused",
		"unused",
		"unused"
	),
};

static struct cam_vfe_top_ver4_hw_info vfe90x_top_hw_info = {
	.common_reg = &vfe90x_top_common_reg,
	.rdi_hw_info =   vfe90x_rdi_hw_info,
	.vfe_full_hw_info = {
		.common_reg     = &vfe90x_top_common_reg,
	},
	.wr_client_desc         = vfe90x_wr_client_desc,
	.num_mux = 8,
	.mux_type = {
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
	},
	.num_rdi        = ARRAY_SIZE(vfe90x_rdi_hw_info),
	.debug_reg_info = &vfe90x_dbg_reg_info,
};

static struct cam_irq_register_set vfe90x_bus_irq_reg[1] = {
		{
			.mask_reg_offset   = 0x00000418,
			.clear_reg_offset  = 0x00000420,
			.status_reg_offset = 0x00000428,
			.set_reg_offset    = 0x00000450,
		},
};

static uint32_t vfe90x_out_port_mid[][4] = {
	{16, 0, 0, 0},
	{17, 0, 0, 0},
	{18, 0, 0, 0},
	{19, 0, 0, 0},
	{20, 0, 0, 0},
	{21, 0, 0, 0},
	{22, 0, 0, 0},
	{23, 0, 0, 0},
};

static struct cam_vfe_bus_ver3_hw_info vfe90x_bus_hw_info = {
	.common_reg = {
		.hw_version                       = 0x00000400,
		.cgc_ovd                          = 0x00000408,
		.pwr_iso_cfg                      = 0x0000045C,
		.overflow_status_clear            = 0x00000460,
		.ccif_violation_status            = 0x00000464,
		.overflow_status                  = 0x00000468,
		.image_size_violation_status      = 0x00000470,
		.debug_status_top_cfg             = 0x000004F0,
		.debug_status_top                 = 0x000004F4,
		.test_bus_ctrl                    = 0x00000544,
		.irq_reg_info = {
			.num_registers            = 1,
			.irq_reg_set              = vfe90x_bus_irq_reg,
			.global_irq_cmd_offset    = 0x00000430,
			.global_clear_bitmask     = 0x00000001,
		},
	},
	.num_client = 8,
	.bus_client_reg = {
		/* BUS Client 0 RDI0 */
		{
			.cfg                      = 0x00000C00,
			.image_addr               = 0x00000C04,
			.frame_incr               = 0x00000C08,
			.image_cfg_0              = 0x00000C0C,
			.image_cfg_1              = 0x00000C10,
			.image_cfg_2              = 0x00000C14,
			.packer_cfg               = 0x00000C18,
			.frame_header_addr        = 0x00000C20,
			.frame_header_incr        = 0x00000C24,
			.frame_header_cfg         = 0x00000C28,
			.line_done_cfg            = 0x00000C2C,
			.irq_subsample_period     = 0x00000C30,
			.irq_subsample_pattern    = 0x00000C34,
			.mmu_prefetch_cfg         = 0x00000C60,
			.mmu_prefetch_max_offset  = 0x00000C64,
			.system_cache_cfg         = 0x00000C68,
			.addr_status_0            = 0x00000C90,
			.addr_status_1            = 0x00000C94,
			.addr_status_2            = 0x00000C98,
			.addr_status_3            = 0x00000C9C,
			.debug_status_cfg         = 0x00000C7C,
			.debug_status_0           = 0x00000C80,
			.debug_status_1           = 0x00000C84,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.ubwc_regs                = NULL,
			.supported_formats        = BIT_ULL(CAM_FORMAT_MIPI_RAW_10) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_6) | BIT_ULL(CAM_FORMAT_MIPI_RAW_8) |
				BIT_ULL(CAM_FORMAT_YUV422) | BIT_ULL(CAM_FORMAT_MIPI_RAW_12) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_14) | BIT_ULL(CAM_FORMAT_MIPI_RAW_16) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_20) | BIT_ULL(CAM_FORMAT_PLAIN128) |
				BIT_ULL(CAM_FORMAT_PLAIN32_20) | BIT_ULL(CAM_FORMAT_PLAIN8) |
				BIT_ULL(CAM_FORMAT_PLAIN16_10) | BIT_ULL(CAM_FORMAT_PLAIN16_12) |
				BIT_ULL(CAM_FORMAT_PLAIN16_14) | BIT_ULL(CAM_FORMAT_PLAIN16_16) |
				BIT_ULL(CAM_FORMAT_PLAIN64) | BIT_ULL(CAM_FORMAT_YUV422_10),
		},
		/* BUS Client 1 RDI1 */
		{
			.cfg                      = 0x00000D00,
			.image_addr               = 0x00000D04,
			.frame_incr               = 0x00000D08,
			.image_cfg_0              = 0x00000D0C,
			.image_cfg_1              = 0x00000D10,
			.image_cfg_2              = 0x00000D14,
			.packer_cfg               = 0x00000D18,
			.frame_header_addr        = 0x00000D20,
			.frame_header_incr        = 0x00000D24,
			.frame_header_cfg         = 0x00000D28,
			.line_done_cfg            = 0x00000D2C,
			.irq_subsample_period     = 0x00000D30,
			.irq_subsample_pattern    = 0x00000D34,
			.mmu_prefetch_cfg         = 0x00000D60,
			.mmu_prefetch_max_offset  = 0x00000D64,
			.system_cache_cfg         = 0x00000D68,
			.addr_status_0            = 0x00000D90,
			.addr_status_1            = 0x00000D94,
			.addr_status_2            = 0x00000D98,
			.addr_status_3            = 0x00000D9C,
			.debug_status_cfg         = 0x00000D7C,
			.debug_status_0           = 0x00000D80,
			.debug_status_1           = 0x00000D84,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_1,
			.ubwc_regs                = NULL,
			.supported_formats        = BIT_ULL(CAM_FORMAT_MIPI_RAW_10) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_6) | BIT_ULL(CAM_FORMAT_MIPI_RAW_8) |
				BIT_ULL(CAM_FORMAT_YUV422) | BIT_ULL(CAM_FORMAT_MIPI_RAW_12) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_14) | BIT_ULL(CAM_FORMAT_MIPI_RAW_16) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_20) | BIT_ULL(CAM_FORMAT_PLAIN128) |
				BIT_ULL(CAM_FORMAT_PLAIN32_20) | BIT_ULL(CAM_FORMAT_PLAIN8) |
				BIT_ULL(CAM_FORMAT_PLAIN16_10) | BIT_ULL(CAM_FORMAT_PLAIN16_12) |
				BIT_ULL(CAM_FORMAT_PLAIN16_14) | BIT_ULL(CAM_FORMAT_PLAIN16_16) |
				BIT_ULL(CAM_FORMAT_PLAIN64) | BIT_ULL(CAM_FORMAT_YUV422_10),
		},
		/* BUS Client 2 RDI2 */
		{
			.cfg                      = 0x00000E00,
			.image_addr               = 0x00000E04,
			.frame_incr               = 0x00000E08,
			.image_cfg_0              = 0x00000E0C,
			.image_cfg_1              = 0x00000E10,
			.image_cfg_2              = 0x00000E14,
			.packer_cfg               = 0x00000E18,
			.frame_header_addr        = 0x00000E20,
			.frame_header_incr        = 0x00000E24,
			.frame_header_cfg         = 0x00000E28,
			.line_done_cfg            = 0x00000E2C,
			.irq_subsample_period     = 0x00000E30,
			.irq_subsample_pattern    = 0x00000E34,
			.mmu_prefetch_cfg         = 0x00000E60,
			.mmu_prefetch_max_offset  = 0x00000E64,
			.system_cache_cfg         = 0x00000E68,
			.addr_status_0            = 0x00000E90,
			.addr_status_1            = 0x00000E94,
			.addr_status_2            = 0x00000E98,
			.addr_status_3            = 0x00000E9C,
			.debug_status_cfg         = 0x00000E7C,
			.debug_status_0           = 0x00000E80,
			.debug_status_1           = 0x00000E84,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_2,
			.ubwc_regs                = NULL,
			.supported_formats        = BIT_ULL(CAM_FORMAT_MIPI_RAW_10) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_6) | BIT_ULL(CAM_FORMAT_MIPI_RAW_8) |
				BIT_ULL(CAM_FORMAT_YUV422) | BIT_ULL(CAM_FORMAT_MIPI_RAW_12) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_14) | BIT_ULL(CAM_FORMAT_MIPI_RAW_16) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_20) | BIT_ULL(CAM_FORMAT_PLAIN128) |
				BIT_ULL(CAM_FORMAT_PLAIN32_20) | BIT_ULL(CAM_FORMAT_PLAIN8) |
				BIT_ULL(CAM_FORMAT_PLAIN16_10) | BIT_ULL(CAM_FORMAT_PLAIN16_12) |
				BIT_ULL(CAM_FORMAT_PLAIN16_14) | BIT_ULL(CAM_FORMAT_PLAIN16_16) |
				BIT_ULL(CAM_FORMAT_PLAIN64) | BIT_ULL(CAM_FORMAT_YUV422_10),
		},
		/* BUS Client 3 RDI3 */
		{
			.cfg                      = 0x00000F00,
			.image_addr               = 0x00000F04,
			.frame_incr               = 0x00000F08,
			.image_cfg_0              = 0x00000F0C,
			.image_cfg_1              = 0x00000F10,
			.image_cfg_2              = 0x00000F14,
			.packer_cfg               = 0x00000F18,
			.frame_header_addr        = 0x00000F20,
			.frame_header_incr        = 0x00000F24,
			.frame_header_cfg         = 0x00000F28,
			.line_done_cfg            = 0x00000F2C,
			.irq_subsample_period     = 0x00000F30,
			.irq_subsample_pattern    = 0x00000F34,
			.mmu_prefetch_cfg         = 0x00000F60,
			.mmu_prefetch_max_offset  = 0x00000F64,
			.system_cache_cfg         = 0x00000F68,
			.addr_status_0            = 0x00000F90,
			.addr_status_1            = 0x00000F94,
			.addr_status_2            = 0x00000F98,
			.addr_status_3            = 0x00000F9C,
			.debug_status_cfg         = 0x00000F7C,
			.debug_status_0           = 0x00000F80,
			.debug_status_1           = 0x00000F84,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_3,
			.ubwc_regs                = NULL,
			.supported_formats        = BIT_ULL(CAM_FORMAT_MIPI_RAW_10) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_6) | BIT_ULL(CAM_FORMAT_MIPI_RAW_8) |
				BIT_ULL(CAM_FORMAT_YUV422) | BIT_ULL(CAM_FORMAT_MIPI_RAW_12) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_14) | BIT_ULL(CAM_FORMAT_MIPI_RAW_16) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_20) | BIT_ULL(CAM_FORMAT_PLAIN128) |
				BIT_ULL(CAM_FORMAT_PLAIN32_20) | BIT_ULL(CAM_FORMAT_PLAIN8) |
				BIT_ULL(CAM_FORMAT_PLAIN16_10) | BIT_ULL(CAM_FORMAT_PLAIN16_12) |
				BIT_ULL(CAM_FORMAT_PLAIN16_14) | BIT_ULL(CAM_FORMAT_PLAIN16_16) |
				BIT_ULL(CAM_FORMAT_PLAIN64) | BIT_ULL(CAM_FORMAT_YUV422_10),
		},
		/* BUS Client 4 RDI4 */
		{
			.cfg                      = 0x00001000,
			.image_addr               = 0x00001004,
			.frame_incr               = 0x00001008,
			.image_cfg_0              = 0x0000100C,
			.image_cfg_1              = 0x00001010,
			.image_cfg_2              = 0x00001014,
			.packer_cfg               = 0x00001018,
			.frame_header_addr        = 0x00001020,
			.frame_header_incr        = 0x00001024,
			.frame_header_cfg         = 0x00001028,
			.line_done_cfg            = 0x0000102C,
			.irq_subsample_period     = 0x00001030,
			.irq_subsample_pattern    = 0x00001034,
			.mmu_prefetch_cfg         = 0x00001060,
			.mmu_prefetch_max_offset  = 0x00001064,
			.system_cache_cfg         = 0x00001068,
			.addr_status_0            = 0x00001090,
			.addr_status_1            = 0x00001094,
			.addr_status_2            = 0x00001098,
			.addr_status_3            = 0x0000109C,
			.debug_status_cfg         = 0x0000107C,
			.debug_status_0           = 0x00001080,
			.debug_status_1           = 0x00001084,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_4,
			.ubwc_regs                = NULL,
			.supported_formats        = BIT_ULL(CAM_FORMAT_MIPI_RAW_10) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_6) | BIT_ULL(CAM_FORMAT_MIPI_RAW_8) |
				BIT_ULL(CAM_FORMAT_YUV422) | BIT_ULL(CAM_FORMAT_MIPI_RAW_12) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_14) | BIT_ULL(CAM_FORMAT_MIPI_RAW_16) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_20) | BIT_ULL(CAM_FORMAT_PLAIN128) |
				BIT_ULL(CAM_FORMAT_PLAIN32_20) | BIT_ULL(CAM_FORMAT_PLAIN8) |
				BIT_ULL(CAM_FORMAT_PLAIN16_10) | BIT_ULL(CAM_FORMAT_PLAIN16_12) |
				BIT_ULL(CAM_FORMAT_PLAIN16_14) | BIT_ULL(CAM_FORMAT_PLAIN16_16) |
				BIT_ULL(CAM_FORMAT_PLAIN64) | BIT_ULL(CAM_FORMAT_YUV422_10),
		},
		/* BUS Client 5 RDI5 */
		{
			.cfg                      = 0x00001100,
			.image_addr               = 0x00001104,
			.frame_incr               = 0x00001108,
			.image_cfg_0              = 0x0000110C,
			.image_cfg_1              = 0x00001110,
			.image_cfg_2              = 0x00001114,
			.packer_cfg               = 0x00001118,
			.frame_header_addr        = 0x00001120,
			.frame_header_incr        = 0x00001124,
			.frame_header_cfg         = 0x00001128,
			.line_done_cfg            = 0x0000112C,
			.irq_subsample_period     = 0x00001130,
			.irq_subsample_pattern    = 0x00001134,
			.mmu_prefetch_cfg         = 0x00001160,
			.mmu_prefetch_max_offset  = 0x00001164,
			.system_cache_cfg         = 0x00001168,
			.addr_status_0            = 0x00001190,
			.addr_status_1            = 0x00001194,
			.addr_status_2            = 0x00001198,
			.addr_status_3            = 0x0000119C,
			.debug_status_cfg         = 0x0000117C,
			.debug_status_0           = 0x00001180,
			.debug_status_1           = 0x00001184,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_5,
			.ubwc_regs                = NULL,
			.supported_formats        = BIT_ULL(CAM_FORMAT_MIPI_RAW_10) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_6) | BIT_ULL(CAM_FORMAT_MIPI_RAW_8) |
				BIT_ULL(CAM_FORMAT_YUV422) | BIT_ULL(CAM_FORMAT_MIPI_RAW_12) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_14) | BIT_ULL(CAM_FORMAT_MIPI_RAW_16) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_20) | BIT_ULL(CAM_FORMAT_PLAIN128) |
				BIT_ULL(CAM_FORMAT_PLAIN32_20) | BIT_ULL(CAM_FORMAT_PLAIN8) |
				BIT_ULL(CAM_FORMAT_PLAIN16_10) | BIT_ULL(CAM_FORMAT_PLAIN16_12) |
				BIT_ULL(CAM_FORMAT_PLAIN16_14) | BIT_ULL(CAM_FORMAT_PLAIN16_16) |
				BIT_ULL(CAM_FORMAT_PLAIN64) | BIT_ULL(CAM_FORMAT_YUV422_10),
		},
		/* BUS Client 6 RDI6 (CSID adds YUV_CHROMA_CONVERSION, no bus-side change) */
		{
			.cfg                      = 0x00001200,
			.image_addr               = 0x00001204,
			.frame_incr               = 0x00001208,
			.image_cfg_0              = 0x0000120C,
			.image_cfg_1              = 0x00001210,
			.image_cfg_2              = 0x00001214,
			.packer_cfg               = 0x00001218,
			.frame_header_addr        = 0x00001220,
			.frame_header_incr        = 0x00001224,
			.frame_header_cfg         = 0x00001228,
			.line_done_cfg            = 0x0000122C,
			.irq_subsample_period     = 0x00001230,
			.irq_subsample_pattern    = 0x00001234,
			.mmu_prefetch_cfg         = 0x00001260,
			.mmu_prefetch_max_offset  = 0x00001264,
			.system_cache_cfg         = 0x00001268,
			.addr_status_0            = 0x00001290,
			.addr_status_1            = 0x00001294,
			.addr_status_2            = 0x00001298,
			.addr_status_3            = 0x0000129C,
			.debug_status_cfg         = 0x0000127C,
			.debug_status_0           = 0x00001280,
			.debug_status_1           = 0x00001284,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_6,
			.ubwc_regs                = NULL,
			.supported_formats        = BIT_ULL(CAM_FORMAT_MIPI_RAW_10) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_6) | BIT_ULL(CAM_FORMAT_MIPI_RAW_8) |
				BIT_ULL(CAM_FORMAT_YUV422) | BIT_ULL(CAM_FORMAT_MIPI_RAW_12) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_14) | BIT_ULL(CAM_FORMAT_MIPI_RAW_16) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_20) | BIT_ULL(CAM_FORMAT_PLAIN128) |
				BIT_ULL(CAM_FORMAT_PLAIN32_20) | BIT_ULL(CAM_FORMAT_PLAIN8) |
				BIT_ULL(CAM_FORMAT_PLAIN16_10) | BIT_ULL(CAM_FORMAT_PLAIN16_12) |
				BIT_ULL(CAM_FORMAT_PLAIN16_14) | BIT_ULL(CAM_FORMAT_PLAIN16_16) |
				BIT_ULL(CAM_FORMAT_PLAIN64) | BIT_ULL(CAM_FORMAT_YUV422_10),
		},
		/* BUS Client 7 RDI7 (CSID adds YUV_CHROMA_CONVERSION, no bus-side change) */
		{
			.cfg                      = 0x00001300,
			.image_addr               = 0x00001304,
			.frame_incr               = 0x00001308,
			.image_cfg_0              = 0x0000130C,
			.image_cfg_1              = 0x00001310,
			.image_cfg_2              = 0x00001314,
			.packer_cfg               = 0x00001318,
			.frame_header_addr        = 0x00001320,
			.frame_header_incr        = 0x00001324,
			.frame_header_cfg         = 0x00001328,
			.line_done_cfg            = 0x0000132C,
			.irq_subsample_period     = 0x00001330,
			.irq_subsample_pattern    = 0x00001334,
			.mmu_prefetch_cfg         = 0x00001360,
			.mmu_prefetch_max_offset  = 0x00001364,
			.system_cache_cfg         = 0x00001368,
			.addr_status_0            = 0x00001390,
			.addr_status_1            = 0x00001394,
			.addr_status_2            = 0x00001398,
			.addr_status_3            = 0x0000139C,
			.debug_status_cfg         = 0x0000137C,
			.debug_status_0           = 0x00001380,
			.debug_status_1           = 0x00001384,
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_7,
			.ubwc_regs                = NULL,
			.supported_formats        = BIT_ULL(CAM_FORMAT_MIPI_RAW_10) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_6) | BIT_ULL(CAM_FORMAT_MIPI_RAW_8) |
				BIT_ULL(CAM_FORMAT_YUV422) | BIT_ULL(CAM_FORMAT_MIPI_RAW_12) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_14) | BIT_ULL(CAM_FORMAT_MIPI_RAW_16) |
				BIT_ULL(CAM_FORMAT_MIPI_RAW_20) | BIT_ULL(CAM_FORMAT_PLAIN128) |
				BIT_ULL(CAM_FORMAT_PLAIN32_20) | BIT_ULL(CAM_FORMAT_PLAIN8) |
				BIT_ULL(CAM_FORMAT_PLAIN16_10) | BIT_ULL(CAM_FORMAT_PLAIN16_12) |
				BIT_ULL(CAM_FORMAT_PLAIN16_14) | BIT_ULL(CAM_FORMAT_PLAIN16_16) |
				BIT_ULL(CAM_FORMAT_PLAIN64) | BIT_ULL(CAM_FORMAT_YUV422_10),
		},
	},
	.num_out = 8,
	.vfe_out_hw_info = {
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_RDI0,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_1,
			.num_wm        = 1,
			.mid           = vfe90x_out_port_mid[0],
			.num_mid       = 1,
			.line_based    = 1,
			.wm_idx        = {
				0,
			},
			.name          = {
				"LITE_0",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_RDI1,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_2,
			.num_wm        = 1,
			.mid           = vfe90x_out_port_mid[1],
			.num_mid       = 1,
			.line_based    = 1,
			.wm_idx        = {
				1,
			},
			.name          = {
				"LITE_1",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_RDI2,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_3,
			.num_wm        = 1,
			.mid           = vfe90x_out_port_mid[2],
			.num_mid       = 1,
			.line_based    = 1,
			.wm_idx        = {
				2,
			},
			.name          = {
				"LITE_2",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_RDI3,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_4,
			.num_wm        = 1,
			.mid           = vfe90x_out_port_mid[3],
			.num_mid       = 1,
			.line_based    = 1,
			.wm_idx        = {
				3,
			},
			.name          = {
				"LITE_3",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_RDI4,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_5,
			.num_wm        = 1,
			.mid           = vfe90x_out_port_mid[4],
			.num_mid       = 1,
			.line_based    = 1,
			.wm_idx        = {
				4,
			},
			.name          = {
				"LITE_4",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_RDI5,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_6,
			.num_wm        = 1,
			.mid           = vfe90x_out_port_mid[5],
			.num_mid       = 1,
			.line_based    = 1,
			.wm_idx        = {
				5,
			},
			.name          = {
				"LITE_5",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_RDI6,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_7,
			.num_wm        = 1,
			.mid           = vfe90x_out_port_mid[6],
			.num_mid       = 1,
			.line_based    = 1,
			.wm_idx        = {
				6,
			},
			.name          = {
				"LITE_6",
			},
		},
		{
			.vfe_out_type  = CAM_VFE_BUS_VER3_VFE_OUT_RDI7,
			.max_width     = -1,
			.max_height    = -1,
			.source_group  = CAM_VFE_BUS_VER3_SRC_GRP_8,
			.num_wm        = 1,
			.mid           = vfe90x_out_port_mid[7],
			.num_mid       = 1,
			.line_based    = 1,
			.wm_idx        = {
				7,
			},
			.name          = {
				"LITE_7",
			},
		},
	},
	.num_comp_grp    = 8,
	.support_consumed_addr = true,
	.comp_done_mask = {
		BIT(0), BIT(1), BIT(2), BIT(3), BIT(4), BIT(5), BIT(6), BIT(7)
	},
	.top_irq_shift   = 0,
	.max_out_res = CAM_ISP_IFE_OUT_RES_BASE + 56,
};

static struct cam_vfe_irq_hw_info vfe90x_irq_hw_info = {
	.reset_mask    = 0,
	.supported_irq = CAM_VFE_HW_IRQ_CAP_LITE_EXT_CSID,
	.top_irq_reg   = &vfe90x_top_irq_reg_info,
};

static struct cam_vfe_hw_info cam_vfe_lite90x_hw_info = {
	.irq_hw_info                   = &vfe90x_irq_hw_info,

	.bus_version                   = CAM_VFE_BUS_VER_3_0,
	.bus_hw_info                   = &vfe90x_bus_hw_info,

	.top_version                   = CAM_VFE_TOP_VER_4_0,
	.top_hw_info                   = &vfe90x_top_hw_info,
};

#endif /* _CAM_VFE_LITE90X_H_ */
