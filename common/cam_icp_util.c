// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <linux/string.h>
#include <linux/firmware.h>

#include "cam_debug_util.h"
#include "cam_cpas_api.h"
#include "cam_icp_util.h"

static bool cam_icp_fw_candidate_exists(
	struct cam_icp_fw_candidate *candidates, int count, const char *name)
{
	int i;

	for (i = 0; i < count; i++) {
		if (!strcmp(candidates[i].name, name))
			return true;
	}

	return false;
}

static int cam_icp_add_fw_candidate(
	struct cam_icp_fw_candidate *candidates, int count, int max,
	const char *name)
{
	if (!name || !name[0])
		return count;

	if (count >= max)
		return count;

	if (cam_icp_fw_candidate_exists(candidates, count, name))
		return count;

	strscpy(candidates[count].name, name, ICP_FW_NAME_MAX_SIZE);
	return count + 1;
}

static bool cam_icp_fw_name_has_ext(const char *name)
{
	size_t len;

	if (!name || !name[0])
		return false;

	len = strlen(name);
	if (len >= strlen(".elf") &&
		!strcmp(name + len - strlen(".elf"), ".elf"))
		return true;

	if (len >= strlen(".mbn") &&
		!strcmp(name + len - strlen(".mbn"), ".mbn"))
		return true;

	return false;
}

int cam_icp_get_fw_candidates(const char *fw_name,
	const char *suffix,
	struct cam_icp_fw_candidate *candidates, int max_candidates)
{
	char candidate[ICP_FW_NAME_MAX_SIZE];
	const char *basename;
	const char *fw_name_suffix = NULL;
	int count = 0;
	const char *target_dir = NULL;
	int rc;

	if (!candidates || !suffix || max_candidates <= 0)
		return -EINVAL;

	rc = cam_cpas_get_target_name(&target_dir);
	if (rc)
		target_dir = NULL;

	if (fw_name && fw_name[0]) {
		basename = strrchr(fw_name, '/');
		basename = basename ? basename + 1 : fw_name;
		fw_name_suffix = cam_icp_fw_name_has_ext(basename) ? "" : suffix;

		/* 1. exact DT path */
		if (snprintf(candidate, sizeof(candidate), "%s%s",
			fw_name, fw_name_suffix) >= sizeof(candidate))
			return -ENAMETOOLONG;

		count = cam_icp_add_fw_candidate(candidates, count,
			max_candidates, candidate);

		/* 2. qcom/<target>/<basename> */
		if (target_dir && target_dir[0]) {
			if (snprintf(candidate, sizeof(candidate), "qcom/%s/%s%s",
				target_dir, basename, fw_name_suffix) >= sizeof(candidate))
				return -ENAMETOOLONG;

			count = cam_icp_add_fw_candidate(candidates, count,
				max_candidates, candidate);
		}

		/* 3. basename at root */
		if (snprintf(candidate, sizeof(candidate), "%s%s",
			basename, fw_name_suffix) >= sizeof(candidate))
			return -ENAMETOOLONG;

		count = cam_icp_add_fw_candidate(candidates, count,
			max_candidates, candidate);
	}

	/* 1. qcom/<target>/<fallback_name> */
	if (target_dir && target_dir[0]) {
		if (snprintf(candidate, sizeof(candidate), "qcom/%s/%s%s",
			target_dir, ICP_FW_FALLBACK_BASE_NAME, suffix) >= sizeof(candidate))
			return -ENAMETOOLONG;

		count = cam_icp_add_fw_candidate(candidates, count,
			max_candidates, candidate);
	}

	/* 2. <fallback_name> at root */
	if (snprintf(candidate, sizeof(candidate), "%s%s",
		ICP_FW_FALLBACK_BASE_NAME, suffix) >= sizeof(candidate))
		return -ENAMETOOLONG;

	count = cam_icp_add_fw_candidate(candidates, count,
		max_candidates, candidate);

	return count;
}

int cam_icp_request_fw_from_candidates(struct device *dev,
	const struct cam_icp_fw_candidate *candidates, int count,
	const struct firmware **fw)
{
	int i = 0, rc = -ENOENT;
	int candidate_rc[ICP_FW_MAX_CANDIDATES] = {0};

	for (i = 0; i < count; i++) {
		rc = firmware_request_nowarn(fw, candidates[i].name, dev);
		if (!rc) {
			CAM_DBG(CAM_ICP, "Firmware request success : %s",
				candidates[i].name);
			return i;
		}

		candidate_rc[i] = rc;

		CAM_DBG(CAM_ICP, "Firmware candidate %s not found rc=%d",
			candidates[i].name, rc);
	}

	CAM_ERR(CAM_ICP, "All firmware candidates exhausted count=%d", count);
	for (i = 0; i < count; i++)
		CAM_ERR(CAM_ICP, "Firmware candidate[%d]: %s Failed, rc=%d",
			i, candidates[i].name, candidate_rc[i]);

	return -ENOENT;
}

