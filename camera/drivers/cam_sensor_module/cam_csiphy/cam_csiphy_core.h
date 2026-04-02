/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2018, 2020-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef _CAM_CSIPHY_CORE_H_
#define _CAM_CSIPHY_CORE_H_

#include <linux/irqreturn.h>
#include "cam_csiphy_dev.h"
#include <cam_mem_mgr.h>
#include <cam_req_mgr_util.h>
#include <cam_io_util.h>

/**
 * struct g_csiphy_data - Per-PHY global state shared across CSIPHY instances
 * @base_address:           I/O-remapped register base address of this PHY instance
 * @data_rate_aux_mask:     Bitmask tracking which data rates are active for
 *                          auxiliary (AON) clients on this PHY
 * @aon_sel_param:          Pointer to AON camera-select register parameters used
 *                          to route the Always-On camera to this PHY
 * @qmargin_data:           Pointer to qmargin sweep data holding per-lane DPHY
 *                          calibration and status captures for quality margin tuning
 * @cpas_handle:            CPAS client handle acquired for this PHY; used to vote
 *                          for power and bandwidth through the CPAS layer
 * @computed_cdr_value:     CDR (Clock Data Recovery) tuning value computed during
 *                          the qmargin sweep for this PHY instance
 * @aon_cam_id:             Identifier of the Always-On camera mapped to this PHY
 * @is_3phase:              Non-zero if this PHY is operating in C-PHY 3-phase mode;
 *                          zero for D-PHY 2-phase mode
 * @is_configured_for_main: True if this PHY is currently configured for the main
 *                          (non-AON) camera usecase
 */
struct g_csiphy_data {
	void __iomem                       *base_address;
	uint64_t                            data_rate_aux_mask;
	struct cam_csiphy_aon_sel_params_t *aon_sel_param;
	struct csiphy_qmargin_sweep_data   *qmargin_data;
	uint32_t                            cpas_handle;
	uint32_t                            computed_cdr_value;
	uint32_t                            aon_cam_id;
	uint8_t                             is_3phase;
	bool                                is_configured_for_main;
};

extern struct g_csiphy_data g_phy_data[MAX_CSIPHY];

/**
 * @csiphy_dev: CSIPhy device structure
 *
 * This API programs CSIPhy IRQ  registers
 */
void cam_csiphy_cphy_irq_config(struct csiphy_device *csiphy_dev);

/**
 * @csiphy_dev: CSIPhy device structure
 *
 * This API resets CSIPhy hardware
 */
void cam_csiphy_reset(struct csiphy_device *csiphy_dev);

/**
 * @csiphy_dev: CSIPhy device structure
 * @arg:    Camera control command argument
 *
 * This API handles the camera control argument reached to CSIPhy
 */
int cam_csiphy_core_cfg(void *csiphy_dev, void *arg);

/**
 * @irq_num: IRQ number
 * @data: CSIPhy device structure
 *
 * This API handles CSIPhy IRQs
 */
irqreturn_t cam_csiphy_irq(int irq_num, void *data);

/**
 * @csiphy_dev: CSIPhy device structure
 *
 * This API handles the CSIPhy close
 */
void cam_csiphy_shutdown(struct csiphy_device *csiphy_dev);

/**
 * @soc_idx : CSIPHY cell index
 *
 * This API registers base address per soc_idx
 */
int cam_csiphy_register_baseaddress(struct csiphy_device *csiphy_dev);

/**
 * @get_access : Get Access for the Main Camera over AON Camera
 * @phy_idx    : To acquire the correct PHY hw to do the operation with
 *
 * This API provides Utility/helper function to program the MUX for
 * correct PHY hw.
 *
 */
int cam_csiphy_util_update_aon_ops(bool get_access, uint32_t phy_idx);


/**
 * @csiphy_dev : CSIPhy device structure
 *
 * This API updates the auxiliary settings mask for the current data rate
 *
 */
void cam_csiphy_update_auxiliary_mask(struct csiphy_device *csiphy_dev);

/**
 * @data:    Qmargin CSID register tuning feedback
 * @phy_idx: PHY idx
 *
 */
void cam_csiphy_update_qmargin_csid_vals(void *data, int phy_idx);

/**
 * @csiphy_dev: CSIPhy device structure
 *
 * This API allows to print all the cphy/dphy specific status registers
 */
int cam_csiphy_dump_status_reg(struct csiphy_device *csiphy_dev);

/**
 * @phy_idx         : To acquire the correct PHY hw to do the operation with
 * @aon_camera_id   : AON Camera ID
 *
 * This API provides Utility/helper function to register AON Hw supprot for
 * correct PHY hw.
 *
 */
int cam_csiphy_util_update_aon_registration(uint32_t phy_idx, uint32_t aon_camera_id);
#endif /* _CAM_CSIPHY_CORE_H_ */
