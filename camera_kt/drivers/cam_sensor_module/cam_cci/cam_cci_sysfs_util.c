// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <linux/debugfs.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <media/cam_sensor.h>
#include "cam_debug_util.h"
#include "cam_sensor_cmn_header.h"
#include "cam_sensor_io.h"
#include "cam_sensor_i2c.h"
#include "cam_sensor_dev.h"
#include "cam_cci_dev.h"
#include "cam_cci_api.h"
#include "cam_sensor_core.h"

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/fs.h>

#define TOTAL_BUFFER_SIZE       2000
#define INFO_BUFFER_SIZE        200
#define NUM_OF_CCI_READ_PARAMS      4
#define NUM_OF_CCI_WRITE_PARAMS     6
#define USAGE_STRING   "Read format: r/R, reg_addr(hex, e.g., 0x300a), addr_type, "\
	"data_type, slave_addr(8-bit write address in hex, e.g., 0xc0)\n"\
	"Write format: w/W, reg_addr(hex, e.g., 0x300a), reg_value(hex), "\
	"delay, addr_type, data_type, slave_addr(8-bit write address in hex, e.g., 0xc0)\n"
#define MAX_TIME_DELAY_MS 2000

/* "slot" + up to 2 digits (soc_info.index, 0-99) + nul */
#define CAM_SENSOR_SYSFS_SLOT_NAME_LEN (sizeof("slot") + 2)
/* "cci" + up to 1 digit (cci_idx = soc_info.index * MASTER_MAX + master, < 10) + nul */
#define CAM_CCI_SYSFS_NODE_NAME_LEN (sizeof("cci") + 1)

/*
 * cam_cci_parse_data() reads each token starting at p_line + CCI_CMD_PREFIX_LEN
 * (skipping the 'r'/'w' command char and its following comma) even for a bare,
 * argument-less token. CCI_INPUT_BUF_PAD is the extra headroom (beyond the raw
 * input length) allocated for input_buf so that read always lands on an
 * in-bounds, guaranteed-zero byte, no matter how short the last token is.
 */
#define CCI_CMD_PREFIX_LEN 2
#define CCI_INPUT_BUF_PAD 2

#define CAM_SENSOR_PWR_SYSFS_ATTR(_name, _mode, _show, _store)		\
	const struct cam_sensor_pwr_sysfs_attr cam_sensor_pwr_sysfs_attr_##_name = {	\
		.attr = { .name = __stringify(_name), .mode = _mode },			\
		.show = _show,								\
		.store = _store,							\
	}

#define CCI_SYSFS_ATTR(_name, _mode, _show, _store)		\
	const struct cci_sysfs_attr cci_sysfs_attr_##_name = {		\
			.attr = { .name = __stringify(_name), .mode = _mode },	\
			.show = _show,						\
			.store = _store,					\
	}

struct cam_sensor_pwr_sysfs_attr {
	/** @attr: Attribute for the sysfs node */
	struct attribute attr;
	/** @show: Show function for the node */
	ssize_t (*show)(struct cam_sensor_ctrl_t *device, char *buf);
	/** @store: Store function for the node */
	ssize_t (*store)(struct cam_sensor_ctrl_t *device, const char *buf,
			size_t count);
};

struct cci_sysfs_attr {
		/** @attr: Attribute for the sysfs node */
		struct attribute attr;
		/** @show: Show function for the node */
		ssize_t (*show)(struct cci_sysfs *device, char *buf);
		/** @store: Store function for the node */
		ssize_t (*store)(struct cci_sysfs *device, const char *buf,
						size_t count);
};

static struct kobject *cam_cci_base_kobject;
static struct kobject *cam_pwr_base_kobject;

int32_t sysfs_cam_sensor_power_up(void *ctrl)
{
	int rc;
	struct cam_sensor_ctrl_t *sensor = (struct cam_sensor_ctrl_t *) ctrl;

	if (!sensor || sensor->sysfs_state != false)
		return 0;

	CAM_DBG(CAM_CCI, "cam_sensor_power_up %d", sensor->soc_info.index);

	mutex_lock(&(sensor->cam_sensor_mutex));

	rc = cam_sensor_power_up(sensor);
	if (rc < 0)
		CAM_ERR(CAM_CCI, "Power up failed for sensor_id: 0x%x, slave_addr: 0x%x",
				sensor->sensordata->slave_info.sensor_id,
				sensor->sensordata->slave_info.sensor_slave_addr);
	else
		sensor->sysfs_state = true;

	mutex_unlock(&(sensor->cam_sensor_mutex));

	return 0;
}

int32_t sysfs_cam_sensor_power_down(void *ctrl)
{
	int rc;
	struct cam_sensor_ctrl_t *sensor = (struct cam_sensor_ctrl_t *) ctrl;

	if (!sensor || sensor->sysfs_state != true)
		return 0;

	CAM_DBG(CAM_CCI, "cam_sensor_power_down %d", sensor->soc_info.index);

	mutex_lock(&(sensor->cam_sensor_mutex));

	rc = cam_sensor_power_down(sensor);
	if (rc < 0)
		CAM_ERR(CAM_CCI, "Power down failed for sensor_id: 0x%x, slave_addr: 0x%x",
				sensor->sensordata->slave_info.sensor_id,
				sensor->sensordata->slave_info.sensor_slave_addr);
	else
		sensor->sysfs_state = false;

	mutex_unlock(&(sensor->cam_sensor_mutex));

	return 0;
}

static ssize_t sensor_pwr_show(struct cam_sensor_ctrl_t *sensor, char *buf)
{
	int count = 0;

	if (!sensor)
		return count;

	mutex_lock(&(sensor->cam_sensor_mutex));
	count = scnprintf(buf, PAGE_SIZE, "SENSOR STATE=%d POWER STATE=%d\n",
			sensor->sensor_state, sensor->sysfs_state);
	mutex_unlock(&(sensor->cam_sensor_mutex));

	return count;
}

static ssize_t sensor_pwr_store(struct cam_sensor_ctrl_t *ctrl,
		const char *buf, size_t count)
{
	int val = 0;

	if (!ctrl) {
		CAM_ERR(CAM_CCI, "INVALID SESNOR ");
		return 0;
	}

	sscanf(buf, "%d", &val);

	if (val == 1)
		sysfs_cam_sensor_power_up(ctrl);
	else if (val == 0)
		sysfs_cam_sensor_power_down(ctrl);
	else
		CAM_ERR(CAM_CCI, "INVALID VALUE ");

	return count;
}

static ssize_t sensor_name_show(struct cam_sensor_ctrl_t *sensor, char *buf)
{
	int count = 0;
	int cci_id = 0;
	struct cam_sensor_cci_client *cci_client;

	if (!sensor)
		return count;

	mutex_lock(&(sensor->cam_sensor_mutex));

	cci_client = sensor->io_master_info.cci_client;
	cci_id = ((uint32_t)MASTER_MAX * cci_client->cci_device) + cci_client->cci_i2c_master;

	CAM_DBG(CAM_CCI, "cci_dev=%d cci_master=%d cci_id=%d",
			cci_client->cci_device, cci_client->cci_i2c_master, cci_id);

	count = scnprintf(buf, PAGE_SIZE, "sensor_%d_cam-cci%d\n",
			sensor->soc_info.index, cci_id);

	mutex_unlock(&(sensor->cam_sensor_mutex));

	return count;
}

static ssize_t cam_sensor_pwr_sysfs_attr_show(struct kobject *kobj,
		struct attribute *__attr, char *buf)
{
	struct cam_sensor_pwr_sysfs_attr *attr = container_of(__attr,
			struct cam_sensor_pwr_sysfs_attr, attr);
	struct cam_sensor_ctrl_t *device = container_of(kobj,
			struct cam_sensor_ctrl_t, sysfs_kobj);

	if (!attr->show)
		return -EIO;

	return attr->show(device, buf);
}

static ssize_t cam_sensor_pwr_sysfs_attr_store(struct kobject *kobj,
		struct attribute *__attr, const char *buf, size_t count)
{
	struct cam_sensor_pwr_sysfs_attr *attr = container_of(__attr,
			struct cam_sensor_pwr_sysfs_attr, attr);
	struct cam_sensor_ctrl_t *device = container_of(kobj,
			struct cam_sensor_ctrl_t, sysfs_kobj);

	if (!attr->store)
		return -EIO;

	return attr->store(device, buf, count);
}

/* Dummy release function - we have nothing to do here */
static void cam_sensor_pwr_sysfs_release(struct kobject *kobj)
{
}

static const struct sysfs_ops cam_sensor_pwr_sysfs_ops = {
	.show = cam_sensor_pwr_sysfs_attr_show,
	.store = cam_sensor_pwr_sysfs_attr_store,
};

static const struct kobj_type cam_sensor_pwr_sysfs_ktype = {
	.sysfs_ops = &cam_sensor_pwr_sysfs_ops,
	.release = cam_sensor_pwr_sysfs_release,
};

static CAM_SENSOR_PWR_SYSFS_ATTR(pwr_enable, 0660, sensor_pwr_show, sensor_pwr_store);
static CAM_SENSOR_PWR_SYSFS_ATTR(name, 0440, sensor_name_show, NULL);

static const struct attribute *cam_sensor_pwr_sysfs_attr_list[] = {
	&cam_sensor_pwr_sysfs_attr_pwr_enable.attr,
	&cam_sensor_pwr_sysfs_attr_name.attr,
	NULL,
};

int32_t cam_sensor_add_device(void *ctrl_struct)
{
	char buff[CAM_SENSOR_SYSFS_SLOT_NAME_LEN];
	int rc;
	struct cam_sensor_ctrl_t *sensor = (struct cam_sensor_ctrl_t *) ctrl_struct;

	if (!sensor) {
		CAM_ERR(CAM_CCI, "Invalid/NULL arguments");
		return -EINVAL;
	}

	CAM_DBG(CAM_CCI, "SLOT_id=%d Name=%s cci_id=%d cci_master=%d",
			sensor->soc_info.index,
			sensor->device_name,
			sensor->io_master_info.cci_client->cci_device,
			sensor->io_master_info.cci_client->cci_i2c_master);

	if (!cam_pwr_base_kobject) {
		cam_pwr_base_kobject = kobject_create_and_add("cam-sensor",
				kernel_kobj);
		if (!cam_pwr_base_kobject)
			return -ENOMEM;
	}

	if (sensor->sysfs_kobj.state_initialized) {
		CAM_ERR(CAM_CCI, "sysfs object is already initialized");
		return 0;
	}

	scnprintf(buff, sizeof(buff), "slot%d", sensor->soc_info.index);

	rc = kobject_init_and_add(&sensor->sysfs_kobj, &cam_sensor_pwr_sysfs_ktype,
				cam_pwr_base_kobject, buff);
	if (rc) {
		CAM_ERR(CAM_CCI, "KOBJ_INIT AND ADD FAILED");
		return -ENOMEM;
	}

	rc = sysfs_create_files(&sensor->sysfs_kobj, cam_sensor_pwr_sysfs_attr_list);
	if (rc) {
		CAM_ERR(CAM_CCI, "failed to create the cam file in /sys/kernel/");
		return rc;
	}

	return 0;
}

static uint32_t cam_cci_parse_master(uint32_t cci_dev_id, uint32_t master_id,
		struct cam_sensor_cci_client *cci_client, uint32_t slave_addr)
{
	int32_t rc = 0;
	struct cci_device *cci_dev;
	const struct cam_hw_soc_info *soc_info = NULL;
	struct v4l2_subdev *sd;
	struct device *dev;

	sd = cam_cci_get_subdev(cci_dev_id);
	cci_dev = v4l2_get_subdevdata(sd);
	if (!cci_dev) {
		CAM_ERR(CAM_CCI, "Invalid params cci_dev: %p", cci_dev);
		rc = -EINVAL;
		return rc;
	}

	if (!cci_client) {
		CAM_ERR(CAM_CCI, "NULL argument cci_client");
		rc = -EINVAL;
		return rc;
	}

	soc_info = &cci_dev->soc_info;
	dev = soc_info->dev;

	CAM_DBG(CAM_CCI, "cci_dev_id %d, master_id: %d", cci_dev_id, master_id);

	cci_client->cci_device = cci_dev_id;
	cci_client->cci_i2c_master = master_id;
	cci_client->cci_subdev = sd;
	cci_client->sid = slave_addr >> 1;
	cci_client->i2c_freq_mode = I2C_FAST_PLUS_MODE;

	return rc;
}

static int32_t cam_cci_parse_data(const char *p_line,
		struct cam_sensor_i2c_reg_setting *reg_list,
		bool *is_read, struct cam_sensor_cci_client *cci_client,
		int32_t master_id, int32_t cci_dev_id)
{
	int32_t rc;
	int32_t num_params;
	struct cam_sensor_i2c_reg_array *reg_array = reg_list->reg_setting;
	uint32_t slave_addr;
	int32_t time_delay;

	if (!strlen(p_line))
		return -EINVAL;

	switch (p_line[0]) {
	case 'r':
	case 'R':
		*is_read = true;
		num_params = NUM_OF_CCI_READ_PARAMS;
		rc = sscanf(p_line + CCI_CMD_PREFIX_LEN, "%x,%d,%d,%x",
				&reg_array->reg_addr,
				(unsigned int *)&reg_list->addr_type,
				(unsigned int *)&reg_list->data_type,
				&slave_addr);
		break;
	case 'w':
	case 'W':
		*is_read = false;
		num_params = NUM_OF_CCI_WRITE_PARAMS;
		rc = sscanf(p_line + CCI_CMD_PREFIX_LEN, "%x,%x,%d,%d,%d,%x",
				&reg_array->reg_addr,
				&reg_array->reg_data,
				&time_delay,
				(unsigned int *)&reg_list->addr_type,
				(unsigned int *)&reg_list->data_type,
				&slave_addr);

		if (time_delay < 0)
			reg_array->delay = 0;
		else if (time_delay > MAX_TIME_DELAY_MS)
			reg_array->delay = MAX_TIME_DELAY_MS;
		else
			reg_array->delay = time_delay;

		break;
	default:
		return -EINVAL;
	}

	if (rc != num_params)
		return -EINVAL;

	rc = cam_cci_parse_master(cci_dev_id, master_id, cci_client, slave_addr);
	if (rc) {
		CAM_ERR(CAM_CCI, "%s: failed for cci_dev_id: %d, master_id: %d, rc: %d",
				__func__, cci_dev_id, master_id, rc);
		return -EINVAL;
	}

	return 0;
}

static ssize_t cci_show(struct cci_sysfs *obj, char *buf)
{
	int count = 0;

	mutex_lock(&(obj->cci_mutex));
	if (obj->display_buf) {
		count = scnprintf(buf, PAGE_SIZE, "%s\n", obj->display_buf);
		memset(obj->display_buf, '\0', TOTAL_BUFFER_SIZE);
	}
	mutex_unlock(&(obj->cci_mutex));

	return count;
}

static int32_t cci_do_read(struct cam_sensor_cci_client *cci_client,
		struct cam_sensor_i2c_reg_setting *read_write,
		char *line_buffer, char *display_buf)
{
	int32_t rc;

	rc = cam_cci_i2c_read(cci_client, read_write->reg_setting->reg_addr,
			&read_write->reg_setting->reg_data, read_write->addr_type,
			read_write->data_type);
	if (!rc) {
		scnprintf(line_buffer, INFO_BUFFER_SIZE, "Read: 0x%X, 0x%X, 0x%x, 0x%X rc: %u\n",
			read_write->reg_setting->reg_addr, read_write->reg_setting->reg_data,
			read_write->data_type, read_write->addr_type, rc);
		strlcat(display_buf, line_buffer, TOTAL_BUFFER_SIZE);
	} else if (rc < 0) {
		CAM_ERR(CAM_CCI, "cci read fail for reg: 0x%X, error code :%d",
				read_write->reg_setting->reg_addr, rc);
	}

	return rc;
}

static void cci_write_and_delay(struct cam_sensor_cci_client *cci_client,
		struct cam_sensor_i2c_reg_setting *read_write,
		char *line_buffer, char *display_buf)
{
	struct cam_cci_ctrl cci_ctrl;
	int32_t rc;

	memset(&cci_ctrl, 0, sizeof(cci_ctrl));
	read_write->size = 1;
	cci_ctrl.cci_info = cci_client;
	cci_ctrl.cmd = MSM_CCI_I2C_WRITE;
	cci_ctrl.cfg.cci_i2c_write_cfg.reg_setting = read_write->reg_setting;
	cci_ctrl.cfg.cci_i2c_write_cfg.data_type = read_write->data_type;
	cci_ctrl.cfg.cci_i2c_write_cfg.addr_type = read_write->addr_type;
	cci_ctrl.cfg.cci_i2c_write_cfg.size = read_write->size;

	rc = cam_cci_client_ops(cci_client->cci_subdev, VIDIOC_MSM_CCI_CFG, &cci_ctrl);
	if (rc) {
		scnprintf(line_buffer, INFO_BUFFER_SIZE,
				"Write Error: 0x%X, 0x%X, 0x%x, 0x%X rc: %d\n",
				read_write->reg_setting->reg_addr,
				read_write->reg_setting->reg_data,
				read_write->data_type,
				read_write->addr_type, rc);
		strlcat(display_buf, line_buffer, TOTAL_BUFFER_SIZE);
	}

	if (read_write->reg_setting->delay > 20)
		msleep(read_write->reg_setting->delay);
	else if (read_write->reg_setting->delay)
		usleep_range(read_write->reg_setting->delay * 1000,
				read_write->reg_setting->delay * 1000 + 1000);
}

static int32_t cci_master_lock(struct cam_sensor_cci_client *cci_client)
{
	struct cam_cci_ctrl cci_ctrl;
	int32_t rc;

	memset(&cci_ctrl, 0, sizeof(cci_ctrl));
	cci_ctrl.cmd = MSM_CCI_I2C_SEQUENTIAL_XFER_LOCK;
	cci_ctrl.cci_info = cci_client;

	rc = cam_cci_client_ops(cci_client->cci_subdev, VIDIOC_MSM_CCI_CFG, &cci_ctrl);
	if (rc < 0)
		CAM_ERR(CAM_CCI, "MASTER LOCK FAILED");
	else
		CAM_DBG(CAM_CCI, "MASTER LOCK ACQUIRED");

	return rc;
}

static void cci_master_unlock(struct cam_sensor_cci_client *cci_client)
{
	struct cam_cci_ctrl cci_ctrl;
	int32_t rc;

	memset(&cci_ctrl, 0, sizeof(cci_ctrl));
	cci_ctrl.cmd = MSM_CCI_I2C_SEQUENTIAL_XFER_UNLOCK;
	cci_ctrl.cci_info = cci_client;

	rc = cam_cci_client_ops(cci_client->cci_subdev, VIDIOC_MSM_CCI_CFG, &cci_ctrl);
	if (rc < 0)
		CAM_ERR(CAM_CCI, "MASTER UNLOCK FAILED");
	else
		CAM_DBG(CAM_CCI, "MASTER LOCK RELEASED");
}

static size_t cci_process_cb(const char *buf, size_t count, int32_t master,
	int32_t bus, char *display_buf)
{
	int rc = 0;
	int master_lock = 0;
	char *token = NULL;
	char *input_buf = NULL;
	char *free_info_ptr = NULL;
	struct cam_sensor_cci_client cci_client;
	char line_buffer[INFO_BUFFER_SIZE];
	struct cam_sensor_i2c_reg_setting read_write;
	struct cam_sensor_i2c_reg_array reg_array;

	if (!display_buf || !buf || count == 0 || (count + CCI_INPUT_BUF_PAD > PAGE_SIZE)) {
		if (count + CCI_INPUT_BUF_PAD > PAGE_SIZE)
			CAM_ERR(CAM_CCI, "Too many commands ..");
		return -EINVAL;
	}

	memset(&cci_client, 0, sizeof(cci_client));

	input_buf = kzalloc(sizeof(char) * (count + CCI_INPUT_BUF_PAD), GFP_KERNEL);
	if (!input_buf)
		return -ENOMEM;

	free_info_ptr = input_buf;

	memcpy(input_buf, buf, count);
	input_buf[count + 1] = '\0';

	token = strsep(&input_buf, ";");
	while (token) {
		bool is_read = false;

		memset(&read_write, 0, sizeof(read_write));
		memset(&reg_array, 0, sizeof(reg_array));
		memset(line_buffer, 0, sizeof(line_buffer));

		read_write.reg_setting = &reg_array;

		rc = cam_cci_parse_data(token, &read_write, &is_read, &cci_client, master, bus);
		if (rc) {
			strlcat(display_buf, USAGE_STRING, TOTAL_BUFFER_SIZE);
			token = strsep(&input_buf, ";");
			CAM_ERR(CAM_CCI, "Incorrect syntax. Syntax: %s", USAGE_STRING);
			continue;
		}

		if (master_lock == 0) {
			rc = cci_master_lock(&cci_client);
			if (rc < 0) {
				count = rc;
				goto handle_error;
			}
			master_lock++;
		}

		if (is_read) {
			rc = cci_do_read(&cci_client, &read_write, line_buffer, display_buf);
			if (rc < 0) {
				count = rc;
				goto handle_error;
			}
		} else {
			if (read_write.addr_type <= CAMERA_SENSOR_I2C_TYPE_INVALID ||
					read_write.addr_type >= CAMERA_SENSOR_I2C_TYPE_MAX ||
					read_write.data_type <= CAMERA_SENSOR_I2C_TYPE_INVALID ||
					read_write.data_type >= CAMERA_SENSOR_I2C_TYPE_MAX) {
				count = -EINVAL;
				CAM_ERR(CAM_CCI, "cam cci write invalid input arguments");
				goto handle_error;
			}

			cci_write_and_delay(&cci_client, &read_write, line_buffer, display_buf);
		}

		token = strsep(&input_buf, ";");
	}

handle_error:
	if (master_lock > 0)
		cci_master_unlock(&cci_client);

	kfree(free_info_ptr);

	return count;
}

static ssize_t cci_store(struct cci_sysfs *obj, const char *buf, size_t count)
{
	if (!obj) {
		CAM_ERR(CAM_CCI, "INVALID CCI ENTRY ");
		return 0;
	}
	mutex_lock(&(obj->cci_mutex));
	count = cci_process_cb(buf, count, obj->master, obj->cci_dev, obj->display_buf);
	mutex_unlock(&(obj->cci_mutex));

	return count;
}

static CCI_SYSFS_ATTR(i2c_rw, 0660, cci_show, cci_store);

static ssize_t cci_sysfs_attr_show(struct kobject *kobj,
		struct attribute *__attr, char *buf)
{
	struct cci_sysfs_attr *attr = container_of(__attr,
			struct cci_sysfs_attr, attr);
	struct cci_sysfs *device = container_of(kobj,
			struct cci_sysfs, sysfs_kobj);

	if (!attr->show)
		return -EIO;

	return attr->show(device, buf);
}

static ssize_t cci_sysfs_attr_store(struct kobject *kobj,
		struct attribute *__attr, const char *buf, size_t count)
{
	struct cci_sysfs_attr *attr = container_of(__attr,
			struct cci_sysfs_attr, attr);
	struct cci_sysfs *device = container_of(kobj,
			struct cci_sysfs, sysfs_kobj);

	if (!attr->store)
		return -EIO;

	return attr->store(device, buf, count);
}

/* Dummy release function - we have nothing to do here */
static void cci_sysfs_release(struct kobject *kobj)
{
}

static const struct sysfs_ops cci_sysfs_ops = {
	.show = cci_sysfs_attr_show,
	.store = cci_sysfs_attr_store,
};

static const struct kobj_type cci_sysfs_ktype = {
	.sysfs_ops = &cci_sysfs_ops,
	.release = cci_sysfs_release,
};

static const struct attribute *cci_sysfs_attr_list[] = {
	&cci_sysfs_attr_i2c_rw.attr,
	NULL,
};

int  cam_sysfs_add_cci(void *cci_device_ptr)
{
	int rc = 0;
	char buff[CAM_CCI_SYSFS_NODE_NAME_LEN];
	struct device_node *of_node;
	struct device *dev;
	struct cam_hw_soc_info *soc_info;
	uint32_t i = 0;
	uint32_t idx;
	uint32_t cci_idx;
	int32_t num_of_map_idx = 0;
	struct cci_device *cci_dev = (struct cci_device *) cci_device_ptr;

	if (!cci_dev) {
		CAM_DBG(CAM_CCI, "CCI DEV NULL");
		return -EINVAL;
	}
	soc_info = &cci_dev->soc_info;

	if (!soc_info->dev) {
		CAM_DBG(CAM_CCI, "SOCINFO DEV NULL");
		return -EINVAL;
	}

	dev = soc_info->dev;

	if (!dev->of_node) {
		CAM_DBG(CAM_CCI, "CCI OF_NODE NULL");
		return -EINVAL;
	}

	of_node = dev->of_node;

	if (!cam_cci_base_kobject) {
		cam_cci_base_kobject = kobject_create_and_add("cam-cci",
				kernel_kobj);
		if (!cam_cci_base_kobject)
			return -ENOMEM;
	}

	num_of_map_idx = of_property_count_u32_elems(
			of_node, "pctrl-idx-mapping");
	if (num_of_map_idx <= 0 || num_of_map_idx > MASTER_MAX) {
		CAM_ERR(CAM_CCI, "Reading pctrl-idx-mapping failed");
		return -EINVAL;
	}

	cci_dev->num_masters = num_of_map_idx;

	for (i = 0; i < num_of_map_idx; i++) {
		of_property_read_u32_index(of_node,
				"pctrl-idx-mapping", i, &idx);
		cci_dev->cci_master_sysfs[i].master = idx;
		cci_dev->cci_master_sysfs[i].cci_dev = soc_info->index;
		cci_idx = (soc_info->index * MASTER_MAX) + idx;

		scnprintf(buff, sizeof(buff), "cci%d", cci_idx);

		rc = kobject_init_and_add(&cci_dev->cci_master_sysfs[i].sysfs_kobj,
				&cci_sysfs_ktype, cam_cci_base_kobject, buff);
		if (rc) {
			CAM_ERR(CAM_CCI, "KOBJ_INIT AND ADD FAILED");
			return -ENOMEM;
		}

		rc = sysfs_create_files(&cci_dev->cci_master_sysfs[i].sysfs_kobj,
				cci_sysfs_attr_list);
		if (rc) {
			CAM_ERR(CAM_CCI, "failed to create the cam file in /sys/kernel/");
			return rc;
		}

		mutex_init(&(cci_dev->cci_master_sysfs[i].cci_mutex));

		if ((cci_dev->cci_master_sysfs[i].sysfs_kobj.state_initialized) &&
				!cci_dev->cci_master_sysfs[i].display_buf) {
			cci_dev->cci_master_sysfs[i].display_buf =
				kzalloc(sizeof(char) * TOTAL_BUFFER_SIZE, GFP_KERNEL);

			if (!cci_dev->cci_master_sysfs[i].display_buf)
				return -ENOMEM;
		}
	}

	return rc;
}

void cam_sysfs_exit(void)
{
	if (cam_cci_base_kobject) {
		kobject_put(cam_cci_base_kobject);
		cam_cci_base_kobject = NULL;
	}
	if (cam_pwr_base_kobject) {
		kobject_put(cam_pwr_base_kobject);
		cam_pwr_base_kobject = NULL;
	}
}

int32_t cam_sysfs_remove_cci(void *cci_device_ptr)
{
	uint32_t i = 0;
	struct cci_device *cci_dev = (struct cci_device *) cci_device_ptr;
	struct cci_sysfs *obj = NULL;

	if (!cci_dev) {
		CAM_DBG(CAM_CCI, "CCI DEV NULL. CCI SYSFS REMOVAL FAILED");
		return -EINVAL;
	}

	for (i = 0; i < cci_dev->num_masters; i++) {
		obj = &cci_dev->cci_master_sysfs[i];

		mutex_lock(&(obj->cci_mutex));

		if (!obj->sysfs_kobj.state_initialized) {
			mutex_unlock(&(obj->cci_mutex));
			continue;
		}

		sysfs_remove_files(&obj->sysfs_kobj, cci_sysfs_attr_list);
		kobject_put(&obj->sysfs_kobj);

		kfree(obj->display_buf);
		obj->display_buf = NULL;

		mutex_unlock(&(obj->cci_mutex));
	}

	return 0;
}

int32_t cam_sensor_remove_device(void *ctrl)
{
	struct cam_sensor_ctrl_t *sensor = (struct cam_sensor_ctrl_t *) ctrl;
	int rc = 0;

	if (!sensor) {
		CAM_DBG(CAM_CCI, "CCI DEV NULL. CCI SYSFS REMOVAL FAILED");
		return -EINVAL;
	}

	mutex_lock(&(sensor->cam_sensor_mutex));

	if (!sensor->sysfs_kobj.state_initialized) {
		mutex_unlock(&(sensor->cam_sensor_mutex));
		return 1;
	}

	if (sensor->sysfs_state) {
		CAM_ERR(CAM_CCI, "SENSOR %d POWERED ON. POWEROFF before teardown",
				sensor->soc_info.index);
		rc = cam_sensor_power_down(sensor);
		if (rc < 0) {
			CAM_ERR(CAM_CCI, "SENSOR %d POWEROFF DONE",
				sensor->soc_info.index);
			sensor->sysfs_state = false;
		}
	}

	sysfs_remove_files(&sensor->sysfs_kobj, cam_sensor_pwr_sysfs_attr_list);
	kobject_put(&sensor->sysfs_kobj);

	mutex_unlock(&(sensor->cam_sensor_mutex));

	return 0;
}
