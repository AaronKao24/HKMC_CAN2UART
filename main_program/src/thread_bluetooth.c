
#include <stdio.h>
#include <zephyr.h>
#include <logging/log.h>
#include <date_time.h>
#include <device.h>
#include <sys/timeutil.h>

#include "fw_api.h"
#include "hal_api.h"
#include "config.h"
#include "function.h"
#include "version_log.h"
//---------modification list for Ken---------
#include <bluetooth/hci_vs.h>
//Factory test item use status by Ken


LOG_MODULE_REGISTER( thread_bluetooth );


extern struct product_info m_product ;

void run_rom_command( char *ppcmd ,int len ,char dev_use)
{
	// global_erase_flag = 1;
	int res ;
	char on_off ;
	char buf[11] ;

	res = hal_process_rom_action( ppcmd , &on_off , len, dev_use );
	switch (res){
	case 1 :	hal_return_command("INVALID\r\n", 9,dev_use);	break ; // NO Command
	case -1 :	break ;
	case -2 :	break ;
	case 2 :	hal_return_command("ERASE",5,dev_use);		break ; // sent ERASE string
	case 0 :	hal_return_command("SUCCESS\r\n", 9,dev_use);
	default :
		break ;
	}
}

void output_device_id_from(char use_dev)
{
	char buf[18] ;
	hal_get_device_id(buf);
	buf[16] = '\r';
	buf[17] = '\n';
	hal_return_command(buf , 18,use_dev);
}

void output_device_datetime( char use_dev ){
	struct tm *tm;
	uint8_t buf[32];
	int64_t dt_val ;
	time_t time_val ;
	date_time_now(&dt_val); // msec
	time_val = dt_val / 1000 ;
	tm = gmtime(&time_val);
	sprintf("%d/%02d/%02d %02d:%02d:%02d", (tm->tm_year + 1900) , tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec );
	hal_return_command( buf , 18, use_dev );
}

void ble_process_getting_command( char *ppcmd ,int len ){
	int res ;
	switch( ppcmd[0] ){
	case 'S' : // get serial number
			hal_return_command( m_product.serial_no , strlen( m_product.serial_no ),DEV_CONNECT_BLE);
			break ;
	case 'D' : // get date time
			output_device_datetime( DEV_CONNECT_BLE );
			break ;
	case 'V' : // get version
			hal_return_command(FW_VERSION_STRING , sizeof(FW_VERSION_STRING),DEV_CONNECT_BLE);	break ;
	case 'I' : // get device ID
			output_device_id_from( DEV_CONNECT_BLE );	break ;
	case 'M' : // get MAC
			output_device_mac_from( DEV_CONNECT_BLE );	break ;
	case 'R' : // reset
			NVIC_SystemReset();
	default :	break ;
	}
}

void ble_process_setting_command( char *ppcmd ,int len ){
	int res ;
	switch( ppcmd[0] ){
	case 'D' : // set date time
		{
			struct tm n_tmd ;
			uint16_t year = ((uint16_t)ppcmd[1]<<8) + ppcmd[2] ;
			n_tmd.tm_year = (year - 1900) ;
			n_tmd.tm_mon = ppcmd[3] ;
			n_tmd.tm_mday = ppcmd[4] ;
			n_tmd.tm_hour = ppcmd[5] ;
			n_tmd.tm_min = ppcmd[6] ;
			n_tmd.tm_sec = ppcmd[7] ;
			date_time_set(&n_tmd);
		}
               		break ;
	case 'R' : // reset
			NVIC_SystemReset();
	default :	break ;
	}
}

#define KW_CMD_SIZE	4
const char *ble_cmd[ KW_CMD_SIZE ] = {  "ROM" , "DEV" , "SET" , "GET" } ;

void process_ble_command(char *p_cmd ,int len)
{
	int i , cmd_idx = -1 ;
	for (i = 0 ; i < KW_CMD_SIZE ; i++) {
		if (strncmp(ble_cmd[i] , p_cmd , 3) == 0) {
			cmd_idx = i ;
			break ;
		}
	}
	switch (cmd_idx){
	case 0 :
		run_rom_command( &(p_cmd[3]) , len - 3 , DEV_CONNECT_BLE );
		break ;

	case 1 :
		hal_init_res(DEV_CONNECT_BLE);
		break ;
	case 2 :
		ble_process_setting_command( &(p_cmd[3]) , len - 3  );
		break ;
	case 3 :
		ble_process_getting_command( &(p_cmd[3]) , len - 3  );
		break ;

	default :
		return ;
	}
}




static void bluetooth_wdt_callback(int channel_id, void *user_data)
{
	LOG_ERR("BLE WDT %d callback, thread: %s\n", channel_id, k_thread_name_get((k_tid_t)user_data));

	/*
	 * If the issue could be resolved, call task_wdt_feed(channel_id) here
	 * to continue operation.
	 *
	 * Otherwise we can perform some cleanup and reset the device.
	 */


	// sys_reboot(SYS_REBOOT_COLD);
}

/**
* @fn thread_for_bluetooth
* @brief bluetooth thread
*/
void thread_for_bluetooth(void)
{
        //---------modification list for Ken---------
        uint16_t  cenrail_conn_handle=0xFFFF;
        int8_t    cur_conn_rssi=0;
        int8_t    cur_conn_txpower=0xFF;

        //=======================================================
        // Check hal is initial
        //=======================================================
	while( ! hal_is_init_finish() ) {
		delay_msec( 10 );
	}
        //=======================================================
        // Set wdt timer
        //=======================================================
	int ct_task_wdt_id = task_wdt_add(500U, bluetooth_wdt_callback, (void *)k_current_get());

        //=======================================================
        // Loop
        //=======================================================
	while( 1 ) {
		delay_msec( 10 );
		task_wdt_feed(ct_task_wdt_id);
#if 0
                //-----------------------------------------------
                // [ Peripheral working ]-->basic code for NUS, removed
                //-----------------------------------------------

#endif
                //-----------------------------------------------
                // [ Centrail working ]
                //-----------------------------------------------
                if(sys_param.ble_centeral_init){
                        arch_nop();
                        //---------------------------------------
                        // Be Connected
                        //---------------------------------------
                        if(g_ifactory_ble_conn){
                            //-----------------------------------
                            // Check Connected
                            //-----------------------------------
                            if(fw_is_ble_central_connected(&cenrail_conn_handle)){
                                if(g_ifactory_ble_report_step==1){
                                    read_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_CONN, cenrail_conn_handle, &g_ifactory_txpower);
                                    read_conn_rssi(cenrail_conn_handle, &g_ifactory_rssi);
                                    g_ifactory_ble_report_step=2;
                                }
                            }
                            //-----------------------------------
                            // Disconnect      -->change to Central disconnect / connected callback
                            //-----------------------------------
                            else{
                                g_ifactory_ble_conn = 0;
                                g_ifactory_ble_report_step=0;
                                cenrail_conn_handle=0xFFFF;
                                g_ifactory_rssi=0;
                                g_ifactory_txpower=0xFF;
                            }
                        }
                        //---------------------------------------
                        // Wait Connected
                        //---------------------------------------
                        else{
						}
                }   //end if(sys_param.ble_centeral_init)
	}   //end while loop
}

/**
* @def DISP_PRIORITY The priority of the Bluetooth communication process (the smaller the value, the higher the priority)
*/
#define BLE_PRIORITY 6
//K_THREAD_DEFINE(ble_communication_id, 8196, thread_for_bluetooth, NULL, NULL, NULL, BLE_PRIORITY, 0, 0);
