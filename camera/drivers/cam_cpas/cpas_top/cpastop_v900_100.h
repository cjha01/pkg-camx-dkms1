/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef _CPASTOP_V900_100_H_
#define _CPASTOP_V900_100_H_

/*
 * Nord CAMNOC is split into independent RT (bus id 2473, ERL base 0x7000)
 * and NRT (bus id 2470/2471 page-continuation, ERL base 0xB000) blocks -
 * confirmed via nord_swi.csv. Unlike the 690-clone that this file replaced,
 * every offset below is CAMNOC_RT/CAMNOC_NRT-relative (block base
 * subtracted) and traced to a real nord_swi.csv row, not copied from
 * Pakala/980.
 *
 * SBM per-port fault bit assignment (bit0 = slave error, bit(n) = nth
 * UBWC-capable port in listing order) follows the same undocumented but
 * precedented convention already used for 980 - no CAMNOC HPG doc is
 * present in this workspace to confirm exact bit semantics; addresses are
 * hardware-verified, this per-bit assignment is inferred by precedent.
 */

static struct cam_camnoc_irq_sbm cam_cpas_v900_100_irq_sbm_rt = {
	.sbm_enable = {
		.access_type = CAM_REG_TYPE_READ_WRITE,
		.enable = true,
		.offset = 0x240, /* CAM_NOC_RT_SBM_FAULTINEN0_LOW */
		.value = 0x01 |  /* RT_SBM_FAULTINEN0_LOW_PORT0_MASK - Slave error IRQ */
			0x02,   /* RT_SBM_FAULTINEN0_LOW_PORT1_MASK - RT_0 UBWC Encoder Error IRQ */
	},
	.sbm_status = {
		.access_type = CAM_REG_TYPE_READ,
		.enable = true,
		.offset = 0x248, /* CAM_NOC_RT_SBM_FAULTINSTATUS0_LOW */
	},
	/*
	 * FLAGOUTCLR0 is a distinct register from FAULTINEN0/FAULTINSTATUS0
	 * above, wired to unrelated internal signals (ip_catalog-confirmed:
	 * ICP clock-gating, SidebandManager FaultIn, DebugChain ExtLoad) -
	 * its bits do NOT mirror sbm_enable's port assignment, so this value
	 * must not be derived from sbm_enable.value.
	 */
	.sbm_clear = {
		.access_type = CAM_REG_TYPE_WRITE,
		.enable = true,
		.offset = 0x280, /* CAM_NOC_RT_SBM_FLAGOUTCLR0_LOW */
		.value = 0x7,
	}
};

static struct cam_camnoc_irq_sbm cam_cpas_v900_100_irq_sbm_nrt = {
	.sbm_enable = {
		.access_type = CAM_REG_TYPE_READ_WRITE,
		.enable = true,
		.offset = 0x240, /* CAM_NOC_NRT_SBM_FAULTINEN0_LOW */
		.value = 0x01 |  /* NRT_SBM_FAULTINEN0_LOW_PORT0_MASK - Slave Error */
			0x02 |   /* NRT_SBM_FAULTINEN0_LOW_PORT1_MASK - NRT_0 UBWC Enc */
			0x04 |   /* NRT_SBM_FAULTINEN0_LOW_PORT2_MASK - NRT_1 UBWC Enc */
			0x08 |   /* NRT_SBM_FAULTINEN0_LOW_PORT3_MASK - NRT_2 UBWC Dec */
			0x10,    /* NRT_SBM_FAULTINEN0_LOW_PORT4_MASK - NRT_3 UBWC Dec */
			/*
			 * PORT5_MASK (0x20) intentionally not enabled - nord_swi.csv
			 * confirms NRT_4 has no niu EncErr/DecErr registers (plain
			 * register-poke DMA port only), so there is no real error
			 * source behind this bit.
			 */
	},
	.sbm_status = {
		.access_type = CAM_REG_TYPE_READ,
		.enable = true,
		.offset = 0x248, /* CAM_NOC_NRT_SBM_FAULTINSTATUS0_LOW */
	},
	/*
	 * FLAGOUTCLR0 is a distinct register from FAULTINEN0/FAULTINSTATUS0
	 * above, wired to unrelated internal signals (ip_catalog-confirmed:
	 * ICP clock-gating, SidebandManager FaultIn, DebugChain ExtLoad) -
	 * its bits do NOT mirror sbm_enable's port assignment, so this value
	 * must not be derived from sbm_enable.value.
	 */
	.sbm_clear = {
		.access_type = CAM_REG_TYPE_WRITE,
		.enable = true,
		.offset = 0x280, /* CAM_NOC_NRT_SBM_FLAGOUTCLR0_LOW */
		.value = 0x7,
	}
};

static struct cam_camnoc_irq_err
	cam_cpas_v900_100_irq_err_rt[] = {
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_SLAVE_ERROR,
		.enable = true,
		.sbm_port = 0x1, /* RT_SBM_FAULTINSTATUS0_LOW_PORT0_MASK */
		.err_enable = {
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.enable = true,
			.offset = 0x8, /* CAM_NOC_RT_ERL_MAINCTL_LOW */
			.value = 0x1,
		},
		.err_status = {
			.access_type = CAM_REG_TYPE_READ,
			.enable = true,
			.offset = 0x10, /* CAM_NOC_RT_ERL_ERRVLD_LOW */
		},
		.err_clear = {
			.access_type = CAM_REG_TYPE_WRITE,
			.enable = true,
			.offset = 0x18, /* CAM_NOC_RT_ERL_ERRCLR_LOW */
			.value = 0x1,
		},
	},
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_TFE_UBWC_ENCODE_ERROR,
		.enable = true,
		.sbm_port = 0x2, /* RT_SBM_FAULTINSTATUS0_LOW_PORT1_MASK */
		.err_enable = {
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.enable = true,
			.offset = 0x47A0, /* RT_0 : NIU_ENCERREN_LOW (0xB7A0-0x7000) */
			.value = 0xF,
		},
		.err_status = {
			.access_type = CAM_REG_TYPE_READ,
			.enable = true,
			.offset = 0x4790, /* RT_0 : NIU_ENCERRSTATUS_LOW (0xB790-0x7000) */
		},
		.err_clear = {
			.access_type = CAM_REG_TYPE_WRITE,
			.enable = true,
			.offset = 0x4798, /* RT_0 : NIU_ENCERRCLR_LOW (0xB798-0x7000) */
			.value = 0x1,
		},
	},
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_RESERVED1,
		.enable = false,
	},
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_RESERVED2,
		.enable = false,
	},
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_CAMNOC_TEST,
		.enable = false,
		.sbm_port = 0x20, /* RT_SBM_FAULTINSTATUS0_LOW_PORT7_MASK */
		.err_enable = {
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.enable = true,
			.offset = 0x288, /* CAM_NOC_RT_SBM_FLAGOUTSET0_LOW */
			.value = 0x1,
		},
		.err_status = {
			.access_type = CAM_REG_TYPE_READ,
			.enable = true,
			.offset = 0x290, /* CAM_NOC_RT_SBM_FLAGOUTSTATUS0_LOW */
		},
		.err_clear = {
			.enable = false, /* CAM_NOC_RT_SBM_FLAGOUTCLR0_LOW */
		},
	},
};

static struct cam_camnoc_irq_err
	cam_cpas_v900_100_irq_err_nrt[] = {
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_SLAVE_ERROR,
		.enable = true,
		.sbm_port = 0x1, /* NRT_SBM_FAULTINSTATUS0_LOW_PORT0_MASK */
		.err_enable = {
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.enable = true,
			.offset = 0x8, /* CAM_NOC_NRT_ERL_MAINCTL_LOW */
			.value = 0x1,
		},
		.err_status = {
			.access_type = CAM_REG_TYPE_READ,
			.enable = true,
			.offset = 0x10, /* CAM_NOC_NRT_ERL_ERRVLD_LOW */
		},
		.err_clear = {
			.access_type = CAM_REG_TYPE_WRITE,
			.enable = true,
			.offset = 0x18, /* CAM_NOC_NRT_ERL_ERRCLR_LOW */
			.value = 0x1,
		},
	},
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_IPE_UBWC_ENCODE_ERROR,
		.enable = true,
		.sbm_port = 0x2, /* NRT_SBM_FAULTINSTATUS0_LOW_PORT1_MASK */
		.err_enable = {
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.enable = true,
			.offset = 0x43A0, /* NRT_0 : NIU_ENCERREN_LOW (0xF3A0-0xB000) */
			.value = 0xF,
		},
		.err_status = {
			.access_type = CAM_REG_TYPE_READ,
			.enable = true,
			.offset = 0x4390, /* NRT_0 : NIU_ENCERRSTATUS_LOW (0xF390-0xB000) */
		},
		.err_clear = {
			.access_type = CAM_REG_TYPE_WRITE,
			.enable = true,
			.offset = 0x4398, /* NRT_0 : NIU_ENCERRCLR_LOW (0xF398-0xB000) */
			.value = 0x1,
		},
	},
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_IPE_BPS_UBWC_ENCODE_ERROR,
		.enable = true,
		.sbm_port = 0x4, /* NRT_SBM_FAULTINSTATUS0_LOW_PORT2_MASK */
		.err_enable = {
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.enable = true,
			.offset = 0x45A0, /* NRT_1 : NIU_ENCERREN_LOW (0xF5A0-0xB000) */
			.value = 0xF,
		},
		.err_status = {
			.access_type = CAM_REG_TYPE_READ,
			.enable = true,
			.offset = 0x4590, /* NRT_1 : NIU_ENCERRSTATUS_LOW (0xF590-0xB000) */
		},
		.err_clear = {
			.access_type = CAM_REG_TYPE_WRITE,
			.enable = true,
			.offset = 0x4598, /* NRT_1 : NIU_ENCERRCLR_LOW (0xF598-0xB000) */
			.value = 0x1,
		},
	},
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_IPE0_UBWC_DECODE_ERROR,
		.enable = true,
		.sbm_port = 0x8, /* NRT_SBM_FAULTINSTATUS0_LOW_PORT3_MASK */
		.err_enable = {
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.enable = true,
			.offset = 0x4720, /* NRT_2 : NIU_DECERREN_LOW (0xF720-0xB000) */
			.value = 0xFF,
		},
		.err_status = {
			.access_type = CAM_REG_TYPE_READ,
			.enable = true,
			.offset = 0x4710, /* NRT_2 : NIU_DECERRSTATUS_LOW (0xF710-0xB000) */
		},
		.err_clear = {
			.access_type = CAM_REG_TYPE_WRITE,
			.enable = true,
			.offset = 0x4718, /* NRT_2 : NIU_DECERRCLR_LOW (0xF718-0xB000) */
			.value = 0xFF,
		},
	},
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_IPE1_UBWC_DECODE_ERROR,
		.enable = true,
		.sbm_port = 0x10, /* NRT_SBM_FAULTINSTATUS0_LOW_PORT4_MASK */
		.err_enable = {
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.enable = true,
			.offset = 0x4920, /* NRT_3 : NIU_DECERREN_LOW (0xF920-0xB000) */
			.value = 0xFF,
		},
		.err_status = {
			.access_type = CAM_REG_TYPE_READ,
			.enable = true,
			.offset = 0x4910, /* NRT_3 : NIU_DECERRSTATUS_LOW (0xF910-0xB000) */
		},
		.err_clear = {
			.access_type = CAM_REG_TYPE_WRITE,
			.enable = true,
			.offset = 0x4918, /* NRT_3 : NIU_DECERRCLR_LOW (0xF918-0xB000) */
			.value = 0xFF,
		},
	},
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_AHB_TIMEOUT,
		.enable = false,
		.sbm_port = 0x20, /* NRT_SBM_FAULTINSTATUS0_LOW_PORT5_MASK */
		.err_enable = {
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.enable = true,
			.offset = 0x288, /* CAM_NOC_NRT_SBM_FLAGOUTSET0_LOW */
			.value = 0xE,
		},
		.err_status = {
			.access_type = CAM_REG_TYPE_READ,
			.enable = true,
			.offset = 0x290, /* CAM_NOC_NRT_SBM_FLAGOUTSTATUS0_LOW */
		},
		.err_clear = {
			.enable = false, /* CAM_NOC_NRT_SBM_FLAGOUTCLR0_LOW */
		},
	},
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_RESERVED1,
		.enable = false,
	},
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_RESERVED2,
		.enable = false,
	},
	{
		.irq_type = CAM_CAMNOC_HW_IRQ_CAMNOC_TEST,
		.enable = false,
		.sbm_port = 0x400, /* NRT_SBM_FAULTINSTATUS0_LOW_PORT10_MASK */
		.err_enable = {
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.enable = true,
			.offset = 0x288, /* CAM_NOC_NRT_SBM_FLAGOUTSET0_LOW */
			.value = 0x2,
		},
		.err_status = {
			.access_type = CAM_REG_TYPE_READ,
			.enable = true,
			.offset = 0x290, /* CAM_NOC_NRT_SBM_FLAGOUTSTATUS0_LOW */
		},
		.err_clear = {
			.enable = false, /* CAM_NOC_NRT_SBM_FLAGOUTCLR0_LOW */
		},
	},
};

/*
 * QoS policy provenance (priority_lut / urgency / danger_lut / safe_lut /
 * qosgen / dynattr values below):
 *
 * These are CAMNOC bandwidth/latency *tuning-policy* values, NOT hardware
 * register-map facts. ip_catalog and nord_swi.csv expose only each register's
 * offset + reset-default (0x00) - never the programmed value - and no Nord
 * CPAS/CAMNOC QoS doc exists in this workspace, so there is no bit-level
 * ground truth to verify a tuned policy value against (this is true for every
 * cpastop_v*.h, not a Nord-specific gap).
 *
 * RT_0..RT_4 and all NRT ports are now sourced from Nord's own real-silicon
 * QNX/automotive CamNOC init (CHIP_ID_SA8797P branch of
 * autocam/qcx/platformmanager/.../platformcamnoc.cpp, cross-checked
 * byte-for-byte between its root and gen4p5 variants) - this is the same
 * physical CAMNOC_RT/CAMNOC_NRT NIU hardware as this header targets, just
 * driven from a different OS/driver stack, and is the closest thing to a
 * verified production value available without CAMNOC HPG (id 1681) access
 * (SSO-gated, unreachable from this workspace). Mapping is by physical port
 * index (RT_N/NRT_N), since both codebases address ports via the same
 * base-address + fixed-stride scheme, not by this header's port_type/role
 * label, which is a Nord-team annotation and not itself a HW fact:
 *   - RT_0..RT_4: prio_lut_low 0x65555544 / prio_lut_high 0x66666666 /
 *     urgency 0x1C40 / danger_lut 0xFFFFFF00 / safe_lut 0x000F (uniform
 *     across all five ports on real silicon - RT_1 previously looked like a
 *     distinct port from RT_0/RT_2..4, but is not).
 *   - RT_5/RT_6: only urgency is programmed on real silicon (0x4 / 0x3);
 *     priority_lut/danger_lut/safe_lut are left at hand-authored defaults
 *     below since the QNX driver never writes them for these two ports.
 *   - NRT_0/NRT_1: prio_lut_low/high 0x33333333 / urgency 0x30.
 *   - NRT_2/NRT_3/NRT_4: prio_lut_low/high 0x33333333 / urgency 0x3.
 *     (NRT_2/NRT_3 already carried this priority_lut pair; only their
 *     urgency changes here.) danger_lut/safe_lut are not programmed for any
 *     NRT port on real silicon, so hand-authored defaults are kept below.
 *   - ICP: unchanged (qosgen 0x50 / dynattr 0x10, matching cpastop_v692_100.h
 *     byte-for-byte). The QNX SA8797P/Nord branch does not program ICP QoS at
 *     all (that write path is gated to the unrelated CHIP_ID_SA8787P), so
 *     there is no real-silicon Nord value to adopt here; the 690/692-inherited
 *     value remains the best available reference.
 *   - CCU: newly added port (was entirely absent from this array; Nord's
 *     CAMNOC_RT block has 8 physical qosgen ports - rt_0..rt_6 plus ccu - per
 *     nord_swi.csv, but only 7 were represented here). qosgen_mainctl and
 *     dynattr_mainctl are left at 0x0 (hardware reset default): the
 *     "nord_qos_rt_recomded_seeting_part0..9" recommended-setting spreadsheet
 *     (systems-team CAMNOC_RT register review) lists Recommended Setting ==
 *     Default == 0x0 for every CCU_QOSGEN_MAINCTL/CCU_DYNATTR_MAINCTL row, and
 *     the same Recommended == Default pattern holds for rt_0..rt_6's own
 *     qosgen_mainctl/shaping rows, so no RT port has qosgen/dynattr tuning
 *     away from default on Nord. CCU's .port_type is a TODO (see below) -
 *     NRT-side QoS values are still being sourced separately and are not yet
 *     reflected here.
 *
 * Final production BW/QoS numbers are still a systems-team deliverable and
 * should be confirmed against the Nord CAMNOC BW budget before tape-out; the
 * values here are cross-referenced against Nord's own real-silicon QNX driver,
 * internally consistent, and safe as bring-up defaults.
 */
/* RT ports - all offsets relative to CAMNOC_RT block base (0x7000) */
static struct cam_camnoc_specific
	cam_cpas_v900_100_camnoc_specific_rt[] = {
	{
		/* RT_0 - UBWC encode write port */
		.port_type = CAM_CAMNOC_IFE_UBWC,
		.port_name = "RT_0",
		.enable = true,
		.priority_lut_low = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4630, /* RT_0 : NIU_PRIORITYLUT_LOW (0xB630-0x7000) */
			.value = 0x55554444,
		},
		.priority_lut_high = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4634, /* RT_0 : NIU_PRIORITYLUT_HIGH (0xB634-0x7000) */
			.value = 0x66555555,
		},
		.urgency = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4638, /* RT_0 : NIU_URGENCY_LOW (0xB638-0x7000) */
			.value = 0x1C40,
		},
		.danger_lut = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4640, /* RT_0 : NIU_DANGERLUT_LOW (0xB640-0x7000) */
			.value = 0xFFFFFF00,
		},
		.safe_lut = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4648, /* RT_0 : NIU_SAFELUT_LOW (0xB648-0x7000) */
			.value = 0x000F,
		},
	},
	{
		/* RT_1 - UBWC encode write port */
		.port_type = CAM_CAMNOC_IFE_UBWC,
		.port_name = "RT_1",
		.enable = true,
		.priority_lut_low = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4830, /* RT_1 : NIU_PRIORITYLUT_LOW (0xB830-0x7000) */
			.value = 0x55554444,
		},
		.priority_lut_high = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4834, /* RT_1 : NIU_PRIORITYLUT_HIGH (0xB834-0x7000) */
			.value = 0x66555555,
		},
		.urgency = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4838, /* RT_1 : NIU_URGENCY_LOW (0xB838-0x7000) */
			.value = 0x1C40,
		},
		.danger_lut = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4840, /* RT_1 : NIU_DANGERLUT_LOW (0xB840-0x7000) */
			.value = 0xFFFFFF00,
		},
		.safe_lut = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4848, /* RT_1 : NIU_SAFELUT_LOW (0xB848-0x7000) */
			.value = 0x000F,
		},
	},
	{
		/* RT_2 - plain write port, no UBWC enc/dec regs in SWI */
		.port_type = CAM_CAMNOC_IFE_LINEAR,
		.port_name = "RT_2",
		.enable = true,
		.priority_lut_low = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4A30, /* RT_2 : NIU_PRIORITYLUT_LOW (0xBA30-0x7000) */
			.value = 0x55554444,
		},
		.priority_lut_high = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4A34, /* RT_2 : NIU_PRIORITYLUT_HIGH (0xBA34-0x7000) */
			.value = 0x66555555,
		},
		.urgency = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4A38, /* RT_2 : NIU_URGENCY_LOW (0xBA38-0x7000) */
			.value = 0x1C40,
		},
		.danger_lut = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4A40, /* RT_2 : NIU_DANGERLUT_LOW (0xBA40-0x7000) */
			.value = 0xFFFFFF00,
		},
		.safe_lut = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4A48, /* RT_2 : NIU_SAFELUT_LOW (0xBA48-0x7000) */
			.value = 0x000F,
		},
	},
	{
		/* RT_3 - plain write port, no UBWC enc/dec regs in SWI */
		.port_type = CAM_CAMNOC_IFE_LINEAR,
		.port_name = "RT_3",
		.enable = true,
		.priority_lut_low = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4C30, /* RT_3 : NIU_PRIORITYLUT_LOW (0xBC30-0x7000) */
			.value = 0x55554444,
		},
		.priority_lut_high = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4C34, /* RT_3 : NIU_PRIORITYLUT_HIGH (0xBC34-0x7000) */
			.value = 0x66555555,
		},
		.urgency = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4C38, /* RT_3 : NIU_URGENCY_LOW (0xBC38-0x7000) */
			.value = 0x1C40,
		},
		.danger_lut = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4C40, /* RT_3 : NIU_DANGERLUT_LOW (0xBC40-0x7000) */
			.value = 0xFFFFFF00,
		},
		.safe_lut = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4C48, /* RT_3 : NIU_SAFELUT_LOW (0xBC48-0x7000) */
			.value = 0x000F,
		},
	},
	{
		/* RT_4 - plain write port, no UBWC enc/dec regs in SWI */
		.port_type = CAM_CAMNOC_IFE_LINEAR,
		.port_name = "RT_4",
		.enable = true,
		.priority_lut_low = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4E30, /* RT_4 : NIU_PRIORITYLUT_LOW (0xBE30-0x7000) */
			.value = 0x55554444,
		},
		.priority_lut_high = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4E34, /* RT_4 : NIU_PRIORITYLUT_HIGH (0xBE34-0x7000) */
			.value = 0x66555555,
		},
		.urgency = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4E38, /* RT_4 : NIU_URGENCY_LOW (0xBE38-0x7000) */
			.value = 0x1C40,
		},
		.danger_lut = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4E40, /* RT_4 : NIU_DANGERLUT_LOW (0xBE40-0x7000) */
			.value = 0xFFFFFF00,
		},
		.safe_lut = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4E48, /* RT_4 : NIU_SAFELUT_LOW (0xBE48-0x7000) */
			.value = 0x000F,
		},
	},
	{
		/*
		 * RT_5 - plain read port. SWI has NIU_MAXRD_LOW @ 0x5010
		 * (0xC010-0x7000) with no matching struct field
		 * (struct cam_camnoc_specific only has maxwr_low) - left
		 * unpopulated rather than fabricating a write path, same
		 * as Nord's other "real register, no generic driver field"
		 * gaps (see nord_ipp_mc_banked_crop_drop_regs.md).
		 */
		.port_type = CAM_CAMNOC_IFE_RDI_RD,
		.port_name = "RT_5",
		.enable = true,
		.priority_lut_low = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x5030, /* RT_5 : NIU_PRIORITYLUT_LOW (0xC030-0x7000) */
			.value = 0x0,
		},
		.priority_lut_high = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x5034, /* RT_5 : NIU_PRIORITYLUT_HIGH (0xC034-0x7000) */
			.value = 0x0,
		},
		.urgency = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x5038, /* RT_5 : NIU_URGENCY_LOW (0xC038-0x7000) */
			.value = 0x4,
		},
		.danger_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x5040, /* RT_5 : NIU_DANGERLUT_LOW (0xC040-0x7000) */
			.value = 0x0,
		},
		.safe_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x5048, /* RT_5 : NIU_SAFELUT_LOW (0xC048-0x7000) */
			.value = 0x0000FFFF,
		},
	},
	{
		/* RT_6 - plain read port, same MaxRd caveat as RT_5 */
		.port_type = CAM_CAMNOC_IFE_RDI_RD,
		.port_name = "RT_6",
		.enable = true,
		.priority_lut_low = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x5230, /* RT_6 : NIU_PRIORITYLUT_LOW (0xC230-0x7000) */
			.value = 0x0,
		},
		.priority_lut_high = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x5234, /* RT_6 : NIU_PRIORITYLUT_HIGH (0xC234-0x7000) */
			.value = 0x0,
		},
		.urgency = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x5238, /* RT_6 : NIU_URGENCY_LOW (0xC238-0x7000) */
			.value = 0x3,
		},
		.danger_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x5240, /* RT_6 : NIU_DANGERLUT_LOW (0xC240-0x7000) */
			.value = 0x0,
		},
		.safe_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x5248, /* RT_6 : NIU_SAFELUT_LOW (0xC248-0x7000) */
			.value = 0x0000FFFF,
		},
	},
	{
		/*
		 * CCU - 8th physical CAMNOC_RT qosgen port (cam_noc_rt_ccu_*
		 * in nord_swi.csv, generation 2473), previously missing from
		 * this array entirely even though rt_0..rt_6 were all present.
		 * Unlike rt_0..rt_6, CCU has zero niu_* registers on Nord (no
		 * PriorityLut/Urgency/DangerLut/SafeLut backing) - confirmed
		 * by exhaustive nord_swi.csv grep - so only qosgen_mainctl /
		 * dynattr_mainctl are populated here, mirroring the ICP entry's
		 * shape below.
		 *
		 * TODO: no CAM_CAMNOC_* value in enum cam_camnoc_port_type
		 * (cam_cpastop_hw.h) represents a CCU-style safety/QoS-only
		 * port; add one and set .port_type once available.
		 */
		.port_name = "CCU",
		.enable = true,
		.qosgen_mainctl = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x6708, /* CCU : QOSGEN_MAINCTL_LOW (0xD708-0x7000) */
			.value = 0x0,
		},
		.dynattr_mainctl = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x5788, /* CCU : DYNATTR_MAINCTL_LOW (0xC788-0x7000) */
			.value = 0x0,
		},
	},
};

/* NRT ports - all offsets relative to CAMNOC_NRT block base (0xB000) */
static struct cam_camnoc_specific
	cam_cpas_v900_100_camnoc_specific_nrt[] = {
	{
		/*
		 * NRT_0 - IPE_WR_0 UBWC encode write port (HPG "CAMNOC-NRT
		 * Block Diagram"). SWI niu_EncCtl_Low/niu_MaxWr_Low confirm
		 * encode+write direction; no per-instance IPE_WR enum value
		 * exists, CAM_CAMNOC_IPE_WR is the generic IPE-write type.
		 */
		.port_type = CAM_CAMNOC_IPE_WR,
		.port_name = "NRT_0",
		.enable = true,
		.priority_lut_low = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4230, /* NRT_0 : NIU_PRIORITYLUT_LOW (0xF230-0xB000) */
			.value = 0x33333333,
		},
		.priority_lut_high = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4234, /* NRT_0 : NIU_PRIORITYLUT_HIGH (0xF234-0xB000) */
			.value = 0x33333333,
		},
		.urgency = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4238, /* NRT_0 : NIU_URGENCY_LOW (0xF238-0xB000) */
			.value = 0x30,
		},
		.danger_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4240, /* NRT_0 : NIU_DANGERLUT_LOW (0xF240-0xB000) */
			.value = 0x0,
		},
		.safe_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4248, /* NRT_0 : NIU_SAFELUT_LOW (0xF248-0xB000) */
			.value = 0x0000FFFF,
		},
	},
	{
		/*
		 * NRT_1 - IPE_WR_1 UBWC encode write port, same rationale
		 * as NRT_0 above.
		 */
		.port_type = CAM_CAMNOC_IPE_WR,
		.port_name = "NRT_1",
		.enable = true,
		.priority_lut_low = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4430, /* NRT_1 : NIU_PRIORITYLUT_LOW (0xF430-0xB000) */
			.value = 0x33333333,
		},
		.priority_lut_high = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4434, /* NRT_1 : NIU_PRIORITYLUT_HIGH (0xF434-0xB000) */
			.value = 0x33333333,
		},
		.urgency = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4438, /* NRT_1 : NIU_URGENCY_LOW (0xF438-0xB000) */
			.value = 0x30,
		},
		.danger_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4440, /* NRT_1 : NIU_DANGERLUT_LOW (0xF440-0xB000) */
			.value = 0x0,
		},
		.safe_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4448, /* NRT_1 : NIU_SAFELUT_LOW (0xF448-0xB000) */
			.value = 0x0000FFFF,
		},
	},
	{
		/* NRT_2 - UBWC decode read port */
		.port_type = CAM_CAMNOC_IPE0_RD,
		.port_name = "NRT_2",
		.enable = true,
		.priority_lut_low = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4630, /* NRT_2 : NIU_PRIORITYLUT_LOW (0xF630-0xB000) */
			.value = 0x33333333,
		},
		.priority_lut_high = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4634, /* NRT_2 : NIU_PRIORITYLUT_HIGH (0xF634-0xB000) */
			.value = 0x33333333,
		},
		.urgency = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4638, /* NRT_2 : NIU_URGENCY_LOW (0xF638-0xB000) */
			.value = 0x3,
		},
		.danger_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4640, /* NRT_2 : NIU_DANGERLUT_LOW (0xF640-0xB000) */
			.value = 0x0,
		},
		.safe_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4648, /* NRT_2 : NIU_SAFELUT_LOW (0xF648-0xB000) */
			.value = 0x0000FFFF,
		},
	},
	{
		/* NRT_3 - UBWC decode read port */
		.port_type = CAM_CAMNOC_IPE1_RD,
		.port_name = "NRT_3",
		.enable = true,
		.priority_lut_low = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4830, /* NRT_3 : NIU_PRIORITYLUT_LOW (0xF830-0xB000) */
			.value = 0x33333333,
		},
		.priority_lut_high = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4834, /* NRT_3 : NIU_PRIORITYLUT_HIGH (0xF834-0xB000) */
			.value = 0x33333333,
		},
		.urgency = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4838, /* NRT_3 : NIU_URGENCY_LOW (0xF838-0xB000) */
			.value = 0x3,
		},
		.danger_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4840, /* NRT_3 : NIU_DANGERLUT_LOW (0xF840-0xB000) */
			.value = 0x0,
		},
		.safe_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x4848, /* NRT_3 : NIU_SAFELUT_LOW (0xF848-0xB000) */
			.value = 0x0000FFFF,
		},
	},
	{
		/*
		 * NRT_4 - CDM_IPE0/CDM_IPE1 port (HPG "CAMNOC-NRT Block
		 * Diagram"). Plain register-poke DMA, no UBWC enc/dec regs in
		 * SWI - only NIU_MAXRD_LOW @ 0x5010 (0x10000-0xB000+0x0010)
		 * with no matching struct field, same left-unpopulated
		 * treatment as RT_5/RT_6.
		 */
		.port_type = CAM_CAMNOC_CDM,
		.port_name = "NRT_4",
		.enable = true,
		.priority_lut_low = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x5030, /* NRT_4 : NIU_PRIORITYLUT_LOW (0x5030) */
			.value = 0x33333333,
		},
		.priority_lut_high = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x5034, /* NRT_4 : NIU_PRIORITYLUT_HIGH (0x5034) */
			.value = 0x33333333,
		},
		.urgency = {
			.enable = true,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x5038, /* NRT_4 : NIU_URGENCY_LOW (0x5038) */
			.value = 0x3,
		},
		.danger_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x5040, /* NRT_4 : NIU_DANGERLUT_LOW (0x5040) */
			.value = 0x0,
		},
		.safe_lut = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x5048, /* NRT_4 : NIU_SAFELUT_LOW (0x5048) */
			.value = 0x0000FFFF,
		},
	},
	{
		/*
		 * ICP - combined RD/WR port. Nord SWI shows two physical ICP
		 * instances (xm_icp_0, xm_icp_1); only icp_0's addresses are
		 * used here, matching the single "ICP_RD_WR" entry convention
		 * already used by every other cpastop_v*.h in this tree
		 * (none of which represent dual ICP instances either).
		 */
		.port_type = CAM_CAMNOC_ICP,
		.port_name = "ICP",
		.enable = false,
		.qosgen_mainctl = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x3A88, /* xm_icp_0 : QOSGEN_MAINCTL_LOW (0xEA88-0xB000) */
			.value = 0x0,
		},
		.dynattr_mainctl = {
			.enable = false,
			.access_type = CAM_REG_TYPE_READ_WRITE,
			.masked_value = 0x0,
			.offset = 0x5488, /* xm_icp_0 : DYNATTR_MAINCTL_LOW, ip_catalog+nord_swi.csv confirmed */
			.value = 0x0,
		},
	},
};

static struct cam_camnoc_err_logger_info cam900_cpas100_err_logger_offsets = {
	.mainctrl     =  0x0008, /* CAM_NOC_x_ERL_MAINCTL_LOW */
	.errvld       =  0x0010, /* CAM_NOC_x_ERL_ERRVLD_LOW */
	.errlog0_low  =  0x0020, /* CAM_NOC_x_ERL_ERRLOG0_LOW */
	.errlog0_high =  0x0024, /* CAM_NOC_x_ERL_ERRLOG0_HIGH */
	.errlog1_low  =  0x0028, /* CAM_NOC_x_ERL_ERRLOG1_LOW */
	.errlog1_high =  0x002c, /* CAM_NOC_x_ERL_ERRLOG1_HIGH */
	.errlog2_low  =  0x0030, /* CAM_NOC_x_ERL_ERRLOG2_LOW */
	.errlog2_high =  0x0034, /* CAM_NOC_x_ERL_ERRLOG2_HIGH */
	.errlog3_low  =  0x0038, /* CAM_NOC_x_ERL_ERRLOG3_LOW */
	.errlog3_high =  0x003c, /* CAM_NOC_x_ERL_ERRLOG3_HIGH */
};

static struct cam_cpas_hw_errata_wa_list cam900_cpas100_errata_wa_list = {
	.camnoc_flush_slave_pending_trans = {
		.enable = false,
		.data.reg_info = {
			.access_type = CAM_REG_TYPE_READ,
			.offset = 0x300, /* sbm_SenseIn0_Low */
			.mask = 0xE0000, /* Bits 17, 18, 19 */
			.value = 0, /* expected to be 0 */
		},
	},
	.enable_icp_clk_for_qchannel = {
		.enable = true,
	},
};

static struct cam_camnoc_info cam900_cpas100_camnoc_info_rt = {
	.specific = &cam_cpas_v900_100_camnoc_specific_rt[0],
	.specific_size = ARRAY_SIZE(cam_cpas_v900_100_camnoc_specific_rt),
	.irq_sbm = &cam_cpas_v900_100_irq_sbm_rt,
	.irq_err = &cam_cpas_v900_100_irq_err_rt[0],
	.irq_err_size = ARRAY_SIZE(cam_cpas_v900_100_irq_err_rt),
	.err_logger = &cam900_cpas100_err_logger_offsets,
	.errata_wa_list = &cam900_cpas100_errata_wa_list,
	.test_irq_info = {
		.sbm_enable_mask = 0x20,
		.sbm_clear_mask = 0x4,
	}
};

static struct cam_camnoc_info cam900_cpas100_camnoc_info_nrt = {
	.specific = &cam_cpas_v900_100_camnoc_specific_nrt[0],
	.specific_size = ARRAY_SIZE(cam_cpas_v900_100_camnoc_specific_nrt),
	.irq_sbm = &cam_cpas_v900_100_irq_sbm_nrt,
	.irq_err = &cam_cpas_v900_100_irq_err_nrt[0],
	.irq_err_size = ARRAY_SIZE(cam_cpas_v900_100_irq_err_nrt),
	.err_logger = &cam900_cpas100_err_logger_offsets,
	.errata_wa_list = &cam900_cpas100_errata_wa_list,
	.test_irq_info = {
		.sbm_enable_mask = 0x400,
		.sbm_clear_mask = 0x1,
	}
};

static struct cam_cpas_camnoc_qchannel cam900_cpas100_qchannel_info_rt = {
	.qchannel_ctrl   = 0xD0, /* CAMERA_TOP CAMNOC_RT_QCHANNEL_CTRL (0x10D0-0x1000) */
	.qchannel_status = 0xD4, /* CAMERA_TOP CAMNOC_RT_QCHANNEL_STATUS (0x10D4-0x1000) */
};

static struct cam_cpas_camnoc_qchannel cam900_cpas100_qchannel_info_nrt = {
	.qchannel_ctrl   = 0xD8, /* CAMERA_TOP CAMNOC_NRT_QCHANNEL_CTRL (0x10D8-0x1000) */
	.qchannel_status = 0xDC, /* CAMERA_TOP CAMNOC_NRT_QCHANNEL_STATUS (0x10DC-0x1000) */
};
static struct cam_cpas_info cam900_cpas100_cpas_info = {
	.hw_caps_info = {
		.num_caps_registers = 1,
		.hw_caps_offsets = {0x8}, /* CAMERA_TOP HW_CAPABILITY (0x1008-0x1000); Nord has 1 caps reg (0xDC is NRT QChannel status, not caps) */
	},
	.qchannel_info = {&cam900_cpas100_qchannel_info_rt,
		&cam900_cpas100_qchannel_info_nrt},
	.num_qchannel = 2,
};

#endif /* _CPASTOP_V900_100_H_ */
