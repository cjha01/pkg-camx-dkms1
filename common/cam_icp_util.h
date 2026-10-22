/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef _CAM_ICP_UTIL_H_
#define _CAM_ICP_UTIL_H_

#include <linux/types.h>

struct device;
struct firmware;

#define ICP_FW_MAX_CANDIDATES 5
#define ICP_FW_NAME_MAX_SIZE 48
#define ICP_FW_FALLBACK_BASE_NAME "CAMERA_ICP"

struct cam_icp_fw_candidate {
	char name[ICP_FW_NAME_MAX_SIZE];
};

/**
 * struct cam_icp_fw_cache
 * @fw:     Cached firmware blob; valid once @loaded is true. Kept alive for
 *          the driver lifetime to avoid repeated filesystem reads on re-boot.
 * @loaded: Set to true after the first successful firmware request; subsequent
 *          boots skip the disk lookup and reuse @fw directly.
 */
struct cam_icp_fw_cache {
	const struct firmware *fw;
	bool loaded;
};

/**
 * cam_icp_get_fw_candidates() - Build unique ICP firmware lookup list
 * @fw_name: Firmware name/path from DT (may include basename or full path)
 * @suffix: Extension to append when @fw_name has no firmware extension
 * @candidates: Output array of firmware candidate entries
 * @max_candidates: Number of entries available in @candidates
 *
 * Generates ordered candidates from DT name, resolved target directory and
 * fallback firmware basename.
 * Return: Number of candidates on success, negative errno on failure.
 */
int cam_icp_get_fw_candidates(const char *fw_name,
	const char *suffix,
	struct cam_icp_fw_candidate *candidates, int max_candidates);

/**
 * cam_icp_request_fw_from_candidates() - Request firmware from candidate list
 * @dev: Device used for firmware lookup
 * @candidates: Ordered candidate names to try
 * @count: Number of entries in @candidates
 * @fw: Output firmware handle when lookup succeeds
 *
 * Return: Selected candidate index on success, negative errno on failure.
 */
int cam_icp_request_fw_from_candidates(struct device *dev,
	const struct cam_icp_fw_candidate *candidates, int count,
	const struct firmware **fw);

#endif /* _CAM_ICP_UTIL_H_ */
