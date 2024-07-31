/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief Nordic UART Service Client sample
 */

#include <errno.h>
#include <zephyr.h>
#include <sys/byteorder.h>
#include <sys/printk.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_vs.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include <bluetooth/services/nus.h>
#include <bluetooth/services/nus_client.h>
#include <bluetooth/gatt_dm.h>
#include <bluetooth/scan.h>
#include <fw_api.h>

#include <settings/settings.h>

#include <drivers/uart.h>

#include <logging/log.h>

LOG_MODULE_REGISTER(fw_ble_central);


#define KEY_PASSKEY_ACCEPT DK_BTN1_MSK
#define KEY_PASSKEY_REJECT DK_BTN2_MSK

#define NUS_WRITE_TIMEOUT K_MSEC(150)
#define UART_WAIT_FOR_BUF_DELAY K_MSEC(50)
#define UART_RX_TIMEOUT 50


K_SEM_DEFINE(nus_write_sem, 0, 1);

static struct bt_conn *central_conn;
static struct bt_nus_client nus_client;

//---------modification list for Ken---------
static uint16_t             cur_handle=0xFFFF;

static void ble_data_sent(struct bt_nus_client *nus, uint8_t err, const uint8_t *const data, uint16_t len)
{
	ARG_UNUSED(nus);


	// sent => pdata , len

}

static uint8_t ble_data_received(struct bt_nus_client *nus,const uint8_t *pdata, uint16_t len)
{
	ARG_UNUSED(nus);

	int err;

	// receive => pdata , len

	return BT_GATT_ITER_CONTINUE;
}


static void discovery_complete(struct bt_gatt_dm *dm, void *context)
{
	struct bt_nus_client *nus = context;
	LOG_INF("Service discovery completed");

	bt_gatt_dm_data_print(dm);

	/*bt_nus_handles_assign(dm, nus);
	bt_nus_subscribe_receive(nus);*/

	bt_gatt_dm_data_release(dm);
}

static void discovery_service_not_found(struct bt_conn *conn, void *context)
{
	LOG_INF("Service not found");
}

static void discovery_error(struct bt_conn *conn, int err, void *context)
{
	LOG_WRN("Error while discovering GATT database: (%d)", err);
}

struct bt_gatt_dm_cb discovery_cb = {
	.completed         = discovery_complete,
	.service_not_found = discovery_service_not_found,
	.error_found       = discovery_error,
};

static void gatt_discover(struct bt_conn *conn)
{
	int err;

	if (conn != central_conn) {
		return;
	}

	err = bt_gatt_dm_start(conn,
			       BT_UUID_NUS_SERVICE,
			       &discovery_cb,
			       &nus_client);
	if (err) {
		LOG_ERR("could not start the discovery procedure, error code: %d", err);
	}
}

static void exchange_func(struct bt_conn *conn, uint8_t err, struct bt_gatt_exchange_params *params)
{
	if (!err) {
		printk("C_MTU exchange done");
	} else {
		printk("C_MTU exchange failed (err %" PRIu8 ")", err);
	}
}

static void connected(struct bt_conn *conn, uint8_t conn_err)
{
	char addr[BT_ADDR_LE_STR_LEN];
	int err;
	struct bt_conn_info info;

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

		if (conn_err) {
			LOG_INF("C_Failed to connect to %s (%d)", log_strdup(addr),	conn_err);

		if (central_conn == conn) {
			bt_conn_unref(central_conn);
			central_conn = NULL;

				err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
				if (err) {
					LOG_ERR("Scanning failed to start (err %d)", err);
				}

			}
			return;
		}

		printk("C_Connected: %s", log_strdup(addr));

	static struct bt_gatt_exchange_params exchange_params;

	exchange_params.func = exchange_func;
	err = bt_gatt_exchange_mtu(conn, &exchange_params);
	if (err) {
			printk("C_MTU exchange failed (err %d)", err);
	}

	bt_conn_get_info(conn, &info);
	if (info.role == BT_CONN_ROLE_CENTRAL) {
        g_ifactory_ble_conn=1;
		err = bt_conn_set_security(conn, BT_SECURITY_L1);
		if (err) {
			printk("C_Failed to set security: %d\n", err);

			gatt_discover(conn);
		}


	err = bt_scan_stop();
	if ((!err) && (err != -EALREADY)) {
		LOG_ERR("Stop LE scan failed (err %d)", err);
	}

        //---------modification list for Ken---------
	err = bt_hci_get_conn_handle( conn ,  &cur_handle);
	if (err) {
                arch_nop();
		return ;
	}
		}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];
	int err;

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("C_Disconnected: %s (reason %u)", log_strdup(addr),
		reason);

	if (central_conn != conn) {
		return;
	}

	bt_conn_unref(central_conn);
	central_conn = NULL;

	/*clear factory settings
	g_ifactory_ble_conn = 0;
	g_ifactory_ble_report_step=0;
	cur_handle=0xFFFF;
	g_ifactory_rssi=0;
	g_ifactory_txpower=0xFF;*/

	err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
	if (err) {
		LOG_ERR("Scanning failed to start (err %d)", err);
	}
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

int8_t debug_rssi = 0 , debug_tx_power = 0 ;
char dbg_addr[64];
static void scan_filter_match(struct bt_scan_device_info *device_info,
			      struct bt_scan_filter_match *filter_match,
			      bool connectable)
{

	bt_addr_le_to_str(device_info->recv_info->addr, dbg_addr, sizeof(dbg_addr));
	debug_rssi = device_info->recv_info->rssi;

	/** Transmit power of the advertiser. */
	debug_tx_power = device_info->recv_info->tx_power;


	LOG_INF("Filters matched. Address: %s connectable: %d", log_strdup(dbg_addr), connectable);
}

static void scan_no_match_fun(struct bt_scan_device_info *device_info,bool connectable)
{
	memset(dbg_addr,0,sizeof(dbg_addr));
	bt_addr_le_to_str(device_info->recv_info->addr, dbg_addr, sizeof(dbg_addr));
}

static void scan_connecting_error(struct bt_scan_device_info *device_info)
{
	LOG_WRN("Connecting failed");
}

static void scan_connecting(struct bt_scan_device_info *device_info,
			    struct bt_conn *conn)
{
	central_conn = bt_conn_ref(conn);
}

BT_SCAN_CB_INIT(scan_cb, scan_filter_match, NULL,
		scan_connecting_error, scan_connecting);

//---------modification list for Ken---------
static	bt_addr_le_t FIT_CONN_MAC_ADR;

//---------modification list for Ken---------
static int scan_init(bt_addr_le_t *mac_adr_filter)
{
	int err;
	struct bt_scan_init_param scan_init = {
                //---------modification list for Ken---------
		.scan_param = NULL,
		.conn_param = BT_LE_CONN_PARAM_DEFAULT,
		.connect_if_match = 1,
	};

	bt_scan_init(&scan_init);
	bt_scan_cb_register(&scan_cb);

        //==================================================
        // [ Filter for Mac Address ]
        //==================================================
        //---------modification list for Ken---------
        //FIT_CONN_MAC_ADR.type = BT_ADDR_LE_RANDOM;
        //FIT_CONN_MAC_ADR.a.val[0] = 0x99;
        //FIT_CONN_MAC_ADR.a.val[1] = 0x3E;
        //FIT_CONN_MAC_ADR.a.val[2] = 0xD3;
        //FIT_CONN_MAC_ADR.a.val[3] = 0x1A;
        //FIT_CONN_MAC_ADR.a.val[4] = 0x30;
        //FIT_CONN_MAC_ADR.a.val[5] = 0xCB;
        //err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_ADDR, &FIT_CONN_MAC_ADR);

        //memcpy(&FIT_CONN_MAC_ADR, mac_adr_filter, sizeof(bt_addr_le_t));
        //arch_nop();
	err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_ADDR, mac_adr_filter);
	if (err) {
		LOG_ERR("Scanning filters cannot be set (err %d)", err);
		return err;
	}

	err = bt_scan_filter_enable(BT_SCAN_ADDR_FILTER, false);
	if (err) {
		LOG_ERR("Filters cannot be turned on (err %d)", err);
		return err;
	}

	LOG_INF("Scan module initialized");
	return err;
}

//---------modification list for Ken---------
int fw_ble_centeral_init(bool peripheral_use, bt_addr_le_t  *scan_mac_addr)
{
	int err;

	if (peripheral_use == false) {
		err = bt_enable(NULL);
		if (err) {
			printk("C_Bluetooth init failed (err %d)", err);
			return err;
		}
		printk("C_Bluetooth initialized");

		if (IS_ENABLED(CONFIG_SETTINGS)) {
			settings_load();
		}
	}

        //---------modification list for Ken---------
        memcpy(&FIT_CONN_MAC_ADR, scan_mac_addr, sizeof(bt_addr_le_t));
/*
        FIT_CONN_MAC_ADR.type = BT_ADDR_LE_RANDOM;
        FIT_CONN_MAC_ADR.a.val[0] = 0x99;
        FIT_CONN_MAC_ADR.a.val[1] = 0x3E;
        FIT_CONN_MAC_ADR.a.val[2] = 0xD3;
        FIT_CONN_MAC_ADR.a.val[3] = 0x1A;
        FIT_CONN_MAC_ADR.a.val[4] = 0x30;
        FIT_CONN_MAC_ADR.a.val[5] = 0xCB;
*/
	if ((err = scan_init(&FIT_CONN_MAC_ADR))!=0 ) {
		return err;
	}

/*	remove NUS
	if ((err = nus_client_init())!=0 ) {
		return err ;
	}*/

	printk("Starting Bluetooth Central UART example\n");


	err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
	if (err) {
		LOG_ERR("Scanning failed to start (err %d)", err);
		return err;
	}
	return 0 ;
}


//---------modification list for Ken---------
int read_conn_rssi(uint16_t handle, int8_t *rssi)
{
	struct net_buf *buf, *rsp = NULL;
	struct bt_hci_cp_read_rssi *cp;
	struct bt_hci_rp_read_rssi *rp;
	int err;

	buf = bt_hci_cmd_create(BT_HCI_OP_READ_RSSI, sizeof(*cp));
	if (!buf) {
		printk("Central Unable to allocate command buffer\n");
		return;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(handle);

	err = bt_hci_cmd_send_sync(BT_HCI_OP_READ_RSSI, buf, &rsp);
	if (err) {
		uint8_t reason = rsp ?
			((struct bt_hci_rp_read_rssi *)rsp->data)->status : 0;
		printk("Central Read RSSI err: %d reason 0x%02x\n", err, reason);
		return;
	}

	rp = (void *)rsp->data;
	*rssi = rp->rssi;

	net_buf_unref(rsp);
}


//---------modification list for Ken---------
int read_tx_power(uint8_t handle_type, uint16_t handle, int8_t *tx_pwr_lvl)
{
	struct bt_hci_cp_vs_read_tx_power_level *cp;
	struct bt_hci_rp_vs_read_tx_power_level *rp;
	struct net_buf *buf, *rsp = NULL;
	int    err;

	*tx_pwr_lvl = 0xFF;

	buf = bt_hci_cmd_create(BT_HCI_OP_VS_READ_TX_POWER_LEVEL,
				sizeof(*cp));
	if (!buf) {
		printk("Central Unable to allocate command buffer\n");
		return;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(handle);
	cp->handle_type = handle_type;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_VS_READ_TX_POWER_LEVEL,
				   buf, &rsp);
	if (err) {
		uint8_t reason = rsp ?
			((struct bt_hci_rp_vs_read_tx_power_level *)
			  rsp->data)->status : 0;
		printk("Central Read Tx power err: %d reason 0x%02x\n", err, reason);
		return;
	}

	rp = (void *)rsp->data;
	*tx_pwr_lvl = rp->tx_power_level;

	net_buf_unref(rsp);
}

//---------modification list for Ken---------
char fw_is_ble_central_connected(uint16_t  *currnet_handle)
{
        if( central_conn == NULL ){
            return 0 ;            // non-connection
        }
        else{
            *currnet_handle = cur_handle;
            return 1 ;            // connection
        }
}





