/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef CAM_ICP_V1_CORE_H
#define CAM_ICP_V1_CORE_H

#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/platform_device.h>

#include "cam_icp_hw_intf.h"
#include "hfi_intf.h"
#include "cam_icp_util.h"

#define ICP_V1_QGIC_BASE            0
#define ICP_V1_BASE                 1
#define ICP_V1_CSR_BASE             2

#define ICP_V1_HOST_INT             0x1
#define ICP_V1_WDT_0                0x2
#define ICP_V1_WDT_1                0x4

#define ICP_V1_CSR_ACCESS           0x3C

#define ELF_GUARD_PAGE              (2 * 1024 * 1024)

/**
 * struct cam_icp_v1_device_core_info
 * @fw_buf: smmu alloc/mapped fw buffer
 * @fw_buf_len: fw buffer length
 * @irq_cb: IRQ callback
 * @cpas_handle: CPAS handle for ICP_V1
 * @hfi_handle: hfi handle for ICP V1
 * @cpas_start: state variable for cpas
 * @fw_cache: cached fw blob; reused across multiple icp subdev open call
 */
struct cam_icp_v1_device_core_info {
	uint32_t fw_buf;
	uintptr_t fw_kva_addr;
	uint64_t fw_buf_len;
	struct cam_icp_irq_cb irq_cb;
	uint32_t cpas_handle;
	int32_t hfi_handle;
	bool cpas_start;
	struct cam_icp_fw_cache fw_cache;
};

int cam_icp_v1_init_hw(void *device_priv,
	void *init_hw_args, uint32_t arg_size);
int cam_icp_v1_deinit_hw(void *device_priv,
	void *init_hw_args, uint32_t arg_size);
int cam_icp_v1_process_cmd(void *device_priv, uint32_t cmd_type,
	void *cmd_args, uint32_t arg_size);

irqreturn_t cam_icp_v1_irq(int irq_num, void *data);

void cam_icp_v1_irq_raise(void *priv);
void cam_icp_v1_irq_enable(void *priv);
void __iomem *cam_icp_v1_iface_addr(void *priv);
void cam_icp_v1_populate_hfi_ops(const struct hfi_ops **hfi_proc_ops);

#endif /* CAM_ICP_V1_CORE_H */
