/*
 * Copyright (c) 2018-2021, AT&T Intellectual Property.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "dp_test.h"
#include "dp_test_cmd_state.h"
#include "dp_test_sfp.h"
#include "if_var.h"
#include <rte_dev_info.h>
#include <rte_ethdev.h>
#include "dp_test_console.h"

enum dp_test_sfp_test_type {
	DP_TEST_SFP_NONE,
	DP_TEST_SFP_SFF8472,
	DP_TEST_SFP_SFF8079,
};

static int sfp_test = DP_TEST_SFP_NONE;

/*
 * Module eeprom data imported from
 * https://dev.dpdk.narkive.com/CGIVbz34/
 * dpdk-dev-patch-0-5-patches-to-get-the-information-and-data-of-eeprom
 */
static const uint8_t eeprom_dummy[512] = {
	0x03, 0x04, 0x07, 0x10, 0x00, 0x00, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x06, 0x67, 0x02, 0x00, 0x00,
	0x08, 0x03, 0x00, 0x1e, 0x4f, 0x45, 0x4d, 0x4f,
	0x45, 0x4d, 0x4f, 0x45, 0x4d, 0x4f, 0x45, 0x4d,
	0x4f, 0x45, 0x4d, 0x4f, 0x00, 0x00, 0x8b, 0x21,
	0x53, 0x46, 0x50, 0x2d, 0x31, 0x30, 0x47, 0x2d,
	0x53, 0x52, 0x2d, 0x49, 0x54, 0x20, 0x20, 0x20,
	0x41, 0x20, 0x20, 0x20, 0x03, 0x52, 0x00, 0x24,
	0x00, 0x3a, 0x00, 0x00, 0x57, 0x51, 0x31, 0x36,
	0x30, 0x34, 0x31, 0x32, 0x41, 0x31, 0x31, 0x35,
	0x20, 0x20, 0x20, 0x20, 0x31, 0x35, 0x31, 0x36,
	0x31, 0x30, 0x20, 0x20, 0x68, 0xfa, 0x03, 0x3b,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x50, 0x00, 0xfb, 0x00, 0x4b, 0x00, 0x00, 0x00,
	0x8c, 0xa0, 0x75, 0x30, 0x88, 0xb8, 0x79, 0x18,
	0x1d, 0x4c, 0x01, 0xf4, 0x1b, 0x58, 0x03, 0xe8,
	0x3d, 0xe9, 0x03, 0xe8, 0x27, 0x10, 0x04, 0xeb,
	0x27, 0x10, 0x00, 0x64, 0x1f, 0x07, 0x00, 0x7e,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2d,
	0x2c, 0x59, 0x81, 0x0a, 0x13, 0xc7, 0x17, 0x52,
	0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
	0x00, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xfa, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x00, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t eeprom_8079_dummy[] = {
	0x03, 0x04, 0x07, 0x10, 0x00, 0x00, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x06, 0x67, 0x02, 0x00, 0x00,
	0x08, 0x03, 0x00, 0x1e, 0x4f, 0x45, 0x4d, 0x4f,
	0x45, 0x4d, 0x4f, 0x45, 0x4d, 0x4f, 0x45, 0x4d,
	0x4f, 0x45, 0x4d, 0x4f, 0x00, 0x00, 0x8b, 0x21,
	0x53, 0x46, 0x50, 0x2d, 0x31, 0x30, 0x47, 0x2d,
	0x53, 0x52, 0x2d, 0x49, 0x54, 0x20, 0x20, 0x20,
	0x41, 0x20, 0x20, 0x20, 0x03, 0x52, 0x00, 0x24,
	0x00, 0x3a, 0x00, 0x00, 0x57, 0x51, 0x31, 0x36,
	0x30, 0x34, 0x31, 0x32, 0x41, 0x31, 0x31, 0x35,
	0x20, 0x20, 0x20, 0x20, 0x31, 0x35, 0x31, 0x36,
	0x31, 0x30, 0x20, 0x20, 0x68, 0xfa, 0x03, 0x3b,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

DP_DECL_TEST_SUITE(sfp);

DP_DECL_TEST_CASE(sfp, valid_sfp, NULL, NULL);

#define IIFNAME "sw_port_0_0"

int dp_test_get_module_info(struct rte_eth_dev *dev,
			    struct rte_eth_dev_module_info *modinfo)
{
	switch (sfp_test) {
	case DP_TEST_SFP_SFF8472:
		modinfo->type = RTE_ETH_MODULE_SFF_8472;
		modinfo->eeprom_len = RTE_ETH_MODULE_SFF_8472_LEN;
		return 0;
	case DP_TEST_SFP_SFF8079:
		modinfo->type = RTE_ETH_MODULE_SFF_8079;
		modinfo->eeprom_len = RTE_ETH_MODULE_SFF_8079_LEN;
		return 0;
	case DP_TEST_SFP_NONE:
		break;
	}
	return -EOPNOTSUPP;
}

int dp_test_get_module_eeprom(struct rte_eth_dev *dev,
			      struct rte_dev_eeprom_info *info)
{
	switch (sfp_test) {
	case DP_TEST_SFP_SFF8472:
		if (info->length < RTE_ETH_MODULE_SFF_8472_LEN)
			return -ENOSPC;

		memcpy(info->data, eeprom_dummy, RTE_ETH_MODULE_SFF_8472_LEN);
		return 0;
	case DP_TEST_SFP_SFF8079:
		if (info->length < RTE_ETH_MODULE_SFF_8079_LEN)
			return -ENOSPC;

		memcpy(info->data, eeprom_8079_dummy,
		       RTE_ETH_MODULE_SFF_8079_LEN);
		return 0;
	case DP_TEST_SFP_NONE:
		break;
	}
	return -EOPNOTSUPP;
}

static void dp_test_verify_intf_sfp_state(struct ifnet *ifp,
					  const char *file,
					  const char *function,
					  int line)
{
	json_object *jexp;
	char cmd_str[30];

	snprintf(cmd_str, 30, "ifconfig -v %s", ifp->if_name);
	dp_test_console_request_reply(cmd_str, false);
	jexp = dp_test_json_create(
		"{"
		"  \"interfaces\": "
		"  [ "
		"     { "
		"        \"name\": \"%s\", "
		"        \"xcvr_info\": {"
		"           \"identifier\": \"SFP/SFP+/SFP28\","
		"           \"xcvr_class\": \"10G Base-SR\","
		"           \"connector\": \"LC\","
		"           \"vendor_name\": \"OEMOEMOEMOEMOEMO\","
		"           \"vendor_pn\": \"SFP-10G-SR-IT\","
		"           \"vendor_sn\": \"WQ160412A115\","
		"           \"vendor_oui\": \"00.8b.21\","
		"           \"diag_type\": 104,"
		"           \"date\": \"2015-16-10\","
		"           \"class\": \"10G Base-SR\","
		"%s"
		"         }"
		"     } "
		"  ] "
		"}",
		ifp->if_name, sfp_test == DP_TEST_SFP_SFF8472 ?
		"           \"temperature_C\": 44.3477,"
		"           \"voltage_V\": 3.3034,"
		"           \"rx_power_mW\": 0.0001,"
		"           \"tx_power_mW\": 0.597,"
		"           \"high_rx_power_alarm_thresh\": 1," : "");

	_dp_test_check_json_poll_state(cmd_str, jexp, NULL,
				       DP_TEST_JSON_CHECK_SUBSET,
				       false, 0, false, file,
				       function, line);
	json_object_put(jexp);
}

DP_START_TEST(valid_sfp, sfp_dump_8472)
{
	struct ifnet *ifp;

	sfp_test = DP_TEST_SFP_SFF8472;
	ifp = dp_ifnet_byifname(IIFNAME);
	if (!ifp)
		return;

	dp_test_verify_intf_sfp_state(ifp, __FILE__, __func__, __LINE__);
	sfp_test = DP_TEST_SFP_NONE;
} DP_END_TEST;

DP_START_TEST(valid_sfp, sfp_dump_8079)
{
	struct ifnet *ifp;

	sfp_test = DP_TEST_SFP_SFF8079;
	ifp = dp_ifnet_byifname(IIFNAME);
	if (!ifp)
		return;

	dp_test_verify_intf_sfp_state(ifp, __FILE__, __func__, __LINE__);
	sfp_test = DP_TEST_SFP_NONE;
} DP_END_TEST;
