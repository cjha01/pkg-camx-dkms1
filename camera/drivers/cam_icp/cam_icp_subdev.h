/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef _CAM_ICP_SUBDEV_H_
#define _CAM_ICP_SUBDEV_H_

struct camera_firmware;

/**
 * @brief : API to register ICP subdev to platform framework.
 * @return struct platform_device pointer on on success, or ERR_PTR() on error.
 */
int cam_icp_init_module(void);

/**
 * @brief : API to remove ICP subdev from platform framework.
 */
void cam_icp_exit_module(void);

/**
 * @brief : Returns the per-instance camera_firmware struct for the given
 *          ICP device index, so each ICP subdev's HW core operates on its
 *          own firmware/IOMMU-domain state instead of a shared global.
 * @return Pointer to the per-instance struct, or NULL if device_idx is
 *         out of range.
 */
struct camera_firmware *cam_icp_subdev_get_fw(uint32_t device_idx);

#endif /* _CAM_ICP_SUBDEV_H_ */
