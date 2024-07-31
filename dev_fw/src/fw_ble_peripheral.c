
#include <zephyr/types.h>
#include <zephyr.h>

#include <device.h>

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stats/stats.h>
#include <soc.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
//#include <bluetooth/services/nus.h>
#include <bluetooth/services/bms.h>

#include <settings/settings.h>

#include <stdio.h>

#include <nrfx_nvmc.h>
#include <mgmt/mcumgr/smp_udp.h>
#include <net/net_mgmt.h>
#include <net/net_event.h>
#include <net/net_conn_mgr.h>


#include <bluetooth/services/bms.h>
#include <logging/log.h>
#include "canfd_process.h"

#define LOG_MODULE_NAME fw_bluetooth
LOG_MODULE_REGISTER(LOG_MODULE_NAME);


#ifdef CONFIG_MCUMGR_CMD_FS_MGMT
#include <device.h>
#include <fs/fs.h>
#include "fs_mgmt/fs_mgmt.h"
#include <fs/littlefs.h>
#endif
#ifdef CONFIG_MCUMGR_CMD_OS_MGMT
#include "os_mgmt/os_mgmt.h"
#endif
#ifdef CONFIG_MCUMGR_CMD_IMG_MGMT
#include "img_mgmt/img_mgmt.h"
#endif
#ifdef CONFIG_MCUMGR_CMD_STAT_MGMT
#include "stat_mgmt/stat_mgmt.h"
#endif
#ifdef CONFIG_MCUMGR_CMD_SHELL_MGMT
#include "shell_mgmt/shell_mgmt.h"
#endif
#ifdef CONFIG_MCUMGR_CMD_FS_MGMT
#include "fs_mgmt/fs_mgmt.h"
#endif

#include "fw_api.h"

/* Define an example stats group; approximates seconds since boot. */
STATS_SECT_START(smp_svr_stats)
STATS_SECT_ENTRY(ticks)
STATS_SECT_END;

/* Assign a name to the `ticks` stat. */
STATS_NAME_START(smp_svr_stats)
STATS_NAME(smp_svr_stats, ticks)
STATS_NAME_END(smp_svr_stats);

/* Define an instance of the stats group. */
STATS_SECT_DECL(smp_svr_stats) smp_svr_stats;

#ifdef CONFIG_MCUMGR_CMD_FS_MGMT
FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(cstorage);
static struct fs_mount_t littlefs_mnt = {
	.type = FS_LITTLEFS,
	.fs_data = &cstorage,
	.storage_dev = (void *)FLASH_AREA_ID(storage),
	.mnt_point = "/lfs1"
};
#endif

#define PRIORITY 7

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN	(sizeof(DEVICE_NAME) - 1)


#define KEY_PASSKEY_ACCEPT DK_BTN1_MSK
#define KEY_PASSKEY_REJECT DK_BTN2_MSK

//---------modification list for Add Ebike-----------------------------------------------------
#define BT_UUID_EBIKE_VAL     BT_UUID_128_ENCODE(0x00010000, 0x452d, 0x4269, 0x6b65, 0x58706c6f7661)
#define BT_UUID_EBIKE_SERVICE BT_UUID_DECLARE_128( BT_UUID_EBIKE_VAL )
#define BT_UUID_EBIKE_NOTIFY  BT_UUID_DECLARE_128( BT_UUID_128_ENCODE(0x00010001, 0x452d, 0x4269, 0x6b65, 0x58706c6f7661))
#define BT_UUID_EBIKE_WRITE   BT_UUID_DECLARE_128( BT_UUID_128_ENCODE(0x00010010, 0x452d, 0x4269, 0x6b65, 0x58706c6f7661))

#define EBIKE_MAX_LEN 20
int connect_time = 0;
void ble_set_device_name(char* device_name );
int set_ble_passkey(char* device_name );
//-------------------------------------------------------------------------------------------

static K_SEM_DEFINE(ble_init_ok, 0, 1);

struct bt_conn *current_conn;
static struct bt_conn *auth_conn;

static int generateRandomNumber() {

    srand(k_uptime_get_32());
    return rand() % 900000 + 100000; 
}


char device_name_ble_wifi[28] ;
static const struct bt_data ad_sbc[] = {
	BT_DATA_BYTES(BT_DATA_GAP_APPEARANCE,
		      (CONFIG_BT_DEVICE_APPEARANCE >> 0) & 0xff,
		      (CONFIG_BT_DEVICE_APPEARANCE >> 8) & 0xff),
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, device_name_ble_wifi , sizeof( device_name_ble_wifi ) ),
};

static const struct bt_data bt_ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data bms_sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_BMS_VAL)),
};

static const struct bt_data sd_sbc[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_EBIKE_VAL),
};
#define BUFF_SIZE 256

unsigned char m_bt_data_buf[BUFF_SIZE] ;
int bt_input , bt_readed ;
int bt_Ebike_input , bt_Ebike_readed ;
char Product_SN[16];
char ble_passkey_number[6];

static unsigned int ebike_passkey;

struct
{
	int count;
	bt_addr_le_t addr;
} ble_Bonded;


static void verify_bond(const struct bt_bond_info *info, void *user_data);
//---------modification list for Add Ebike-----------------------------------------------------
static void ebike_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	arch_nop(); //send_enabled(value == BT_GATT_CCC_NOTIFY ? BT_NUS_SEND_STATUS_ENABLED : BT_NUS_SEND_STATUS_DISABLED);
}


//-------------------------------------------------------------------------------------------

static void connected(struct bt_conn *conn, uint8_t err)
{
	connect_time++;
	char addr[BT_ADDR_LE_STR_LEN];
	struct bt_conn_info info;

	if (err) {
		LOG_ERR("Connection failed (err %u)", err);
		return;
	}

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	printk("Connected: %s (Times: %d)\n", log_strdup(addr), connect_time);

	//William fixed central and peripheral role
	bt_conn_get_info(conn, &info);
	if (info.role == BT_CONN_ROLE_PERIPHERAL) {

	        if (bt_conn_set_security(conn, BT_SECURITY_L4)) {
	    // some connection bug hang in here. add clear BT all donded info when error
		        printk("Failed to set security\n");
		        bt_conn_disconnect(current_conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
				bt_conn_unref(current_conn);
				bt_unpair(BT_ID_DEFAULT, &ble_Bonded.addr); 
	        }
        }
	current_conn = bt_conn_ref(conn);

/*      Setting connection state      */
	g_ble_peripheral_state.bits.pairing_complete = 0;
	g_ble_peripheral_state.bits.pairing_accept = 0;

/*      Setting bonding passkey      */	
	//ebike_passkey = generateRandomNumber();

	bt_passkey_set(ebike_passkey);

	CAN_PAGE_DBG(E_DBG_MESSAGE_BLE_CONNECTED);

}

struct bt_conn *fw_get_current_connect(){
	return current_conn ;
}
bool fw_is_bluetooth_connected(void){
	return g_ble_peripheral_state.bits.pairing_complete;
}
extern int bt_hmi_init();
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];
        int error=0;

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	

	printk("Disconnected: %s (reason %u)\n", log_strdup(addr), reason);
	bt_hmi_init();

	if (auth_conn) {
		bt_conn_unref(auth_conn);
		auth_conn = NULL;
	}

	if (current_conn) {
		bt_conn_unref(current_conn);
		current_conn = NULL;
	}
// Clear connection state
	g_ble_peripheral_state.bits.connected = 0;
	g_ble_peripheral_state.bits.pairing_complete = 0;
	CAN_PAGE_DBG(E_DBG_MESSAGE_BLE_DISCONNECT);

// Stop Adv mode
	if(g_ble_peripheral_state.bits.turn_on == 0){
		error = bt_le_adv_stop();
		if(error)
		{
			arch_nop();
		}
	}
	normal_delay_count = 0;	//Clear delay count
}

static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if((bt_conn_get_security(conn))==BT_SECURITY_L4) {
		g_ble_peripheral_state.bits.pairing_complete = 1;
		g_ble_peripheral_state.bits.connected = 1;
	}
}

static struct bt_conn_cb conn_callbacks = {
	.connected    = connected,
	.disconnected = disconnected,
	.security_changed = security_changed,
};

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	printk("Passkey for %s: %06u\n", log_strdup(addr), passkey);	

}

static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Pairing cancelled: %s\n", log_strdup(addr));

	g_ble_peripheral_state.bits.pairing_complete = 0;
	g_ble_peripheral_state.bits.connected = 0;
}

static void auth_pairing_complete(struct bt_conn *conn, bool bonded)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Pairing completed: %s, bonded: %d\n", log_strdup(addr), bonded);

	g_ble_peripheral_state.bits.pairing_complete = 1;
	g_ble_peripheral_state.bits.bonded = 1;
// verify bond & save bonded BDA.
	bt_foreach_bond(BT_ID_DEFAULT, verify_bond, NULL);

}

static void auth_pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Pairing failed conn: %s, reason %d\n", log_strdup(addr), reason);

	bt_conn_disconnect(conn, BT_HCI_ERR_AUTH_FAIL);

	g_ble_peripheral_state.bits.pairing_complete = 0;
	g_ble_peripheral_state.bits.connected = 0;
}

//*****************************************************************
// Pairing accept callback
// Author: William
// Date: 2023/09/25
//*****************************************************************
static enum bt_security_err auth_pairing_accept(struct bt_conn *conn,
				const struct bt_conn_pairing_feat *const feat)
{
 //   ebike_passkey = passkey;	
	g_ble_peripheral_state.bits.pairing_accept = 1;
	g_ble_peripheral_state.bits.connected = 1;

}


// Modification Ken 2023/07/06
static void bond_deleted(uint8_t id, const bt_addr_le_t *peer)
{
//=========================================================
// bond information delete complete
//=========================================================
        g_ble_peripheral_state.bits.bonded = 0;
}


static struct bt_conn_auth_cb conn_auth_callbacks = {

	.passkey_display = auth_passkey_display,
	.passkey_entry = NULL,
	.cancel = auth_cancel,
	.pairing_complete = auth_pairing_complete,
	.pairing_failed = auth_pairing_failed,
        // Modification Ken 2023/07/06
	.bond_deleted = bond_deleted,
	.pairing_accept = auth_pairing_accept,
};

void fw_update_started(void){}
void fw_update_stoped(void){}
void fw_update_pending(void){}
void fw_update_confirmed(void)
{
printk("DFU CONF" );
}

struct img_mgmt_dfu_callbacks_t dfu_cb = {
    .dfu_started_cb = fw_update_started,
    .dfu_stopped_cb = fw_update_stoped,
    .dfu_pending_cb = fw_update_pending,
    .dfu_confirmed_cb = fw_update_confirmed
};

void error(void)
{
	LOG_ERR("Bluetooth fail" );
	while (true) {
		/* Spin for ever */
		k_sleep(K_MSEC(1000));
	}
}

static void num_comp_reply(bool accept)
{
	if (accept) {
		bt_conn_auth_passkey_confirm(auth_conn);
		printk("Numeric Match, conn %p", (void *)auth_conn);
	} else {
		bt_conn_auth_cancel(auth_conn);
		printk("Numeric Reject, conn %p", (void *)auth_conn);
	}

	bt_conn_unref(auth_conn);
	auth_conn = NULL;
}


#define EVENT_MASK (NET_EVENT_L4_CONNECTED | NET_EVENT_L4_DISCONNECTED)

static struct net_mgmt_event_callback mgmt_cb;

static void event_handler(struct net_mgmt_event_callback *cb,
			  uint32_t mgmt_event, struct net_if *iface)
{
	if ((mgmt_event & EVENT_MASK) != mgmt_event) {
		return;
	}

	if (mgmt_event == NET_EVENT_L4_CONNECTED) {
		printk("Network connected");

		if (smp_udp_open() < 0) {
			LOG_ERR("could not open smp udp");
		}

		return;
	}

	if (mgmt_event == NET_EVENT_L4_DISCONNECTED) {
		printk("Network disconnected");
		smp_udp_close();
		return;
	}
}

void start_smp_udp(void)
{
	net_mgmt_init_event_callback(&mgmt_cb, event_handler, EVENT_MASK);
	net_mgmt_add_event_callback(&mgmt_cb);
	net_conn_mgr_resend_status();
}
static const uint8_t bms_auth_code[] = {'A', 'B', 'C', 'D'};
static bool bms_authorize(struct bt_conn *conn,
			  struct bt_bms_authorize_params *params)
{
	if ((params->code_len == sizeof(bms_auth_code)) &&
	    (memcmp(bms_auth_code, params->code, sizeof(bms_auth_code)) == 0)) {
		printk("Authorization of BMS operation is successful\n");
		return true;
	}

	printk("Authorization of BMS operation has failed\n");
	return false;
}

static struct bt_bms_cb bms_callbacks = {
	.authorize = bms_authorize,
};

static int bms_init(void)
{
	struct bt_bms_init_params init_params = {0};

	// Enable all possible operation codes 
	init_params.features.delete_requesting.supported = true;
	init_params.features.delete_rest.supported = true;
	init_params.features.delete_all.supported = true;

	// Require authorization code for operations that also delete bonding information for other devices than the requesting client.
	
	init_params.features.delete_rest.authorize = true;
	init_params.features.delete_all.authorize = true;

	init_params.cbs = &bms_callbacks;

	return bt_bms_init(&init_params);
}

//*****************************************************************
// Init peripheral function
// Author: Isenteck and KenFeng
//*****************************************************************
int fw_ble_peripheral_init(void)
{
	int err = 0;

	err = STATS_INIT_AND_REG(smp_svr_stats, STATS_SIZE_32, "smp_svr_stats");

	if (err < 0) {
		LOG_ERR("Error initializing stats system [%d]", err);
		return err ;
	}
        //=========================================================
        // init register
        //=========================================================
	bt_input = 0 ;
	bt_readed = 0 ;
	
	fw_flash_read(SERIAL_NO , 16 , Product_SN);
	if(Product_SN[0] == 0xFF)
	{
		ebike_passkey = 123123;		
	}
	else
	{
		ebike_passkey = set_ble_passkey(Product_SN);
	}

	bt_passkey_set(ebike_passkey);
        g_ble_peripheral_state.byte = 0;

        //=========================================================
        // Register the built-in mcumgr command handlers.
        //=========================================================
#ifdef CONFIG_MCUMGR_CMD_FS_MGMT
	err = fs_mount(&littlefs_mnt);
	if (err < 0) {
		LOG_ERR("Error mounting littlefs [%d]", err);
		return err ;
	}

	fs_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_CMD_OS_MGMT
	os_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_CMD_IMG_MGMT
	img_mgmt_register_group();
	img_mgmt_register_callbacks( &dfu_cb );
#endif
#ifdef CONFIG_MCUMGR_CMD_STAT_MGMT
	stat_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_CMD_SHELL_MGMT
	shell_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_CMD_FS_MGMT
	fs_mgmt_register_group();
#endif

#ifdef CONFIG_MCUMGR_SMP_UDP
	start_smp_udp();
#endif

        //=========================================================
        // set ble callback function
        //=========================================================
	bt_conn_cb_register(&conn_callbacks);
	bt_conn_auth_cb_register(&conn_auth_callbacks);


        //=========================================================
        // build ble sdk
        //=========================================================
	err = bt_enable(NULL);
	if (err) {
		return err;
	}

	printk("Bluetooth initialized, build time = %s", __TIME__);

        //=========================================================
        //
        //=========================================================
	k_sem_give(&ble_init_ok);

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}
		//=========================================================
    	// set name of adv packet & set device name (suggest after bt_enable() function)
    	//=========================================================
	
	ble_set_device_name(Product_SN);
	memcpy( device_name_ble_wifi , BLE_DEVICE_NAME , strlen(BLE_DEVICE_NAME));
	// load_device_name( device_name_ble_wifi );
	bt_set_name(BLE_DEVICE_NAME);


    	//=========================================================
    	// init gatt services
    	//=========================================================
        err = bms_init();
	if (err) {
		printk("Failed to init BMS (err:%d)\n", err);
		return;
	}

#ifdef CONFIG_MCUMGR_SMP_BT
	smp_bt_register();
#endif
//	err = bt_nus_init(&nus_cb);
	if (err) {
		LOG_ERR("Failed to initialize UART service (err: %d)", err);
		return err ;
	}

        //=========================================================
        // check bond state
        //=========================================================
        ble_Bonded.count = 0;
        g_ble_peripheral_state.bits.bonded = 0;
	bt_foreach_bond(BT_ID_DEFAULT, verify_bond, NULL);
	if(ble_Bonded.count){
		g_ble_peripheral_state.bits.bonded = 1;
//			err = bt_unpair(BT_ID_DEFAULT, &ble_Bonded.addr);   //for test to clear bond phone
//			if (err) {
//				LOG_ERR("Failed Bond Deleted (err: %d)", err);
//			}
        }
        //=========================================================
        // Start ADV mode
        //=========================================================
	fw_ble_turn_on_peripheral(true);
	printk("Starting Nordic UART service example\n");

	return 0 ;
}


void ble_adv_restart(void){	
        fw_ble_turn_on_peripheral(true);
}

void ble_set_device_name(char* device_name ){
	BLE_DEVICE_NAME[0] = 'X';
	BLE_DEVICE_NAME[1] = 'I';
	BLE_DEVICE_NAME[2] = 'O';
	BLE_DEVICE_NAME[3] = 'N';
	for(int length = 0;length < 16;length++)
	{
		if(device_name[length] == '\0')
		{
			if(length <= 4)
			{
				BLE_DEVICE_NAME[4] = 'A';
				BLE_DEVICE_NAME[5] = 'B';
				BLE_DEVICE_NAME[6] = 'C';
				BLE_DEVICE_NAME[7] = 'D';
				BLE_DEVICE_NAME[8] = 'E';
			}
			else
			{
				BLE_DEVICE_NAME[4] = device_name[length-5];
				BLE_DEVICE_NAME[5] = device_name[length-4];
				BLE_DEVICE_NAME[6] = device_name[length-3];
				BLE_DEVICE_NAME[7] = device_name[length-2];
				BLE_DEVICE_NAME[8] = device_name[length-1];
				ble_passkey_number[0] = device_name[length-5]-0x30;
				ble_passkey_number[1] = device_name[length-4]-0x30;
				ble_passkey_number[2] = device_name[length-3]-0x30;
				ble_passkey_number[3] = device_name[length-2]-0x30;
				ble_passkey_number[4] = device_name[length-1]-0x30;
				ble_passkey_number[5] = 0x05;
			}
			break;
		}
	}
	fw_flash_write(PASSKEY_ID , 8 , passkey_number );
}

int set_ble_passkey(char* device_name ){
	
	int ble_passkey = 0;

	for(int length = 0;length < 16;length++)
	{
		if(device_name[length] == '\0')
		{
			if(length <= 4)
			{
				ble_passkey_number[0] = 0x01;
				ble_passkey_number[1] = 0x02;
				ble_passkey_number[2] = 0x03;
				ble_passkey_number[3] = 0x01;
				ble_passkey_number[4] = 0x02;
				ble_passkey_number[5] = 0x03;
			}
			else
			{
				ble_passkey_number[0] = device_name[length-5]-0x30;
				ble_passkey_number[1] = device_name[length-4]-0x30;
				ble_passkey_number[2] = device_name[length-3]-0x30;
				ble_passkey_number[3] = device_name[length-2]-0x30;
				ble_passkey_number[4] = device_name[length-1]-0x30;
				ble_passkey_number[5] = 0x05;
			}
			break;
		}
	}
	ble_passkey = (ble_passkey_number[0]*100000)+(ble_passkey_number[1]*10000)+(ble_passkey_number[2]*1000)+(ble_passkey_number[3]*100)+(ble_passkey_number[4]*10)+ble_passkey_number[5];
	if(ble_passkey_number[0] == 0x00)
	{
		ble_passkey = ble_passkey *10;
		if(ble_passkey_number[1] == 0x00)
		{
			ble_passkey = ble_passkey *10;
			if(ble_passkey_number[2] == 0x00)
			{
				ble_passkey = ble_passkey *10;
				if(ble_passkey_number[3] == 0x00)
				{
					ble_passkey = ble_passkey *10;
					if(ble_passkey_number[4] == 0x00)
					{
						ble_passkey = ble_passkey *10;
					}
				}
			}
		}
	}
	return ble_passkey;
}

void stat_tick()
{
	STATS_INC(smp_svr_stats, ticks);
}

char fw_is_ble_peripheral_connected(void)
{
	if( current_conn == NULL ) return 0 ;
	return 1 ;
}


unsigned int fw_ble_get_peripheral_passkey(void)
{
    return  ebike_passkey;
}

//*****************************************************************
// Check if bonding phone
// Author: KenFeng
// Date: 2023/03/16
//*****************************************************************
static void verify_bond(const struct bt_bond_info *info, void *user_data)
{
    char str[32];

    ble_Bonded.count = 1;

    bt_addr_le_copy(&ble_Bonded.addr, &info->addr);

    bt_addr_to_str(&info->addr.a, str, 18);

    printk("verify_bond: %s, ble_Bonded.count=%d\n", log_strdup(str), ble_Bonded.count);
}


//*****************************************************************
// Turn on/off BLE function
// Author: KenFeng
// Date: 2023/03/16
//*****************************************************************
void fw_ble_turn_on_peripheral(bool onoff)
{
    int err = 0;

    if(onoff==true)
    {
        //---------------------------------------------------------
        // Already turn on
        //---------------------------------------------------------
        if(g_ble_peripheral_state.bits.turn_on){
            return;
        }
        //---------------------------------------------------------
        // Start turn on
        //---------------------------------------------------------
        else{
            //-----------------------------------------------------
            // start adv
            //-----------------------------------------------------
            err = bt_le_adv_start(BT_LE_ADV_CONN, ad_sbc, ARRAY_SIZE(ad_sbc), sd_sbc, ARRAY_SIZE(sd_sbc));
            if (err) {
    		LOG_ERR("Advertising failed to start (err %d)", err);
    		return;
            }
            //-----------------------------------------------------
            // set state
            //-----------------------------------------------------
            g_ble_peripheral_state.bits.turn_on = 1;
        }
    }
    else
    {
        //---------------------------------------------------------
        // Start turn off
        //---------------------------------------------------------
        if(g_ble_peripheral_state.bits.turn_on){
            //-----------------------------------------------------
            // set state
            //-----------------------------------------------------
			bt_conn_disconnect(current_conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
			bt_conn_unref(current_conn);
                bt_le_adv_stop();
                bt_scan_stop();
            g_ble_peripheral_state.bits.turn_on = 0;
            //-----------------------------------------------------
            // disconnect
            //-----------------------------------------------------
            if(g_ble_peripheral_state.bits.connected){
		bt_conn_disconnect(current_conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
		bt_conn_unref(current_conn);
            }
            //-----------------------------------------------------
            // stop adv
            //-----------------------------------------------------
            else{
                bt_le_adv_stop();
            }
        }
        //---------------------------------------------------------
        // Already turn off
        //---------------------------------------------------------
        else{
            return;
        }
    }
}
//*****************************************************************


