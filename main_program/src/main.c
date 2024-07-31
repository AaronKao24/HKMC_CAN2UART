#include <stdio.h>

#include <pm/pm.h>
#include <device.h>
#include <drivers/display.h>
#include <logging/log.h>
#include <drivers/gpio.h>
#include <hal/nrf_gpio.h>
#include <zephyr.h>
#include <drivers/flash.h>
#include <drivers/counter.h>

#include "fw_api.h"
#include "hal_api.h"
#include "function.h"
#include "config.h"
#include "fw_ble_hmi.h"
#include "nRF52840_pin_def.h"
#include "canfd_process.h"
#include <string.h>
#include "display_api.h"
#include <nrfx_gpiote.h>
#include <string.h>
#include <drivers/gpio.h>
#include <sys/time.h>
#include <canfd_motor_upgrade.h>
#include "version_log.h"

#ifdef CONFIG_NRFX_TEMP
#include <nrfx_temp.h>
#endif

#ifdef CONFIG_CPU_LOAD
#include <debug/cpu_load.h>
#endif
/** LOG_MODULE_REGISTER : to register main file log module */
LOG_MODULE_REGISTER( main );

bool do_normal_report_hmi ;
bool is_release_rom = false ;

#ifdef CONFIG_NRFX_TEMP
int32_t m_soc_temperature = 0;
#endif
/**
* @file main
* The main process is responsible for the action:
* - inside the button
* - ROM access
* - command to CAN Bus
* - Return command message to BLE
* - Return command messages to USB
* - controls for "lights"
*
*
* CAN Bus itinerary: Responsible for receiving messages
* BLE: Responsible for receiving messages
* USB: responsible for receiving messages
* Display: Responsible for screen output
*
*
* Consider the output time in display stroke: within about 10 msec for normal transfer; within 80 msec for full screen transfer
* 128 x 96 x 2 x 0.28usec ~= 6.88msec
* 
*/


/**
 * @fn disable_ds_1
 * @param dev Peripheral device indicator
 * @brief action before entering the OS period
 * Boot prevents deep sleep (system shutdown) on long timeout or "K_FOREVER" due to default resident policy. <br>
 * This must be done before any attempt to sleep, which means before the thread system starts between PRE_KERNEL_2 and POST_KERNEL. Do this at the beginning of PRE_KERNEL_2. <br>
 * The initialization level is PRE_KERNEL_1 -> PRE_KERNEL_2 -> POST_KERNEL -> APPLICATION, for the initialization level, please refer to:<br>
 * https://docs.zephyrproject.org/3.1.0/kernel/drivers/index.html <br>
 * Line set to SOFT OFF mode:<br>
 * This state consumes the least amount of power and requires a longer delay to return to the runtime active state.
 * The contents of the system (CPU and memory) will not be preserved, so the system will reboot as it would from initial power-up and kernel boot. <br>
 */
static int disable_ds_1(const struct device *dev)
{
	ARG_UNUSED(dev);

	pm_constraint_set(PM_STATE_SOFT_OFF);
	return 0;
}

SYS_INIT(disable_ds_1, PRE_KERNEL_2, 0);


uint8_t key_st[5] ;
uint8_t old_input_state = 0 ;
uint8_t input_state = 0 ;
uint8_t sample_time_01 = 0;
uint8_t sample_time_02 = 0;
E_DISPLAY_PAGE *display_page;
int st_val,cc1,cc2;
int wdt_channel_id;
E_OTA_UPGRADE ota_status = E_OTA_UPGRADE_ERASE_SEND;
uint64_t seconds;


void ota_start()
{
	switch(ota_status)
	{
		case E_OTA_UPGRADE_ERASE_SEND:
			//TODO: Send cmd 005
			FIT_CANFD_MOTORO_UPGRADE_START_send();
			ota_status = E_OTA_UPGRADE_ERASE_WAITING_ACK;
			break;
		
		case E_OTA_UPGRADE_ERASE_WAITING_ACK:
			//TODO: call function get 006 data;
			//TODO: if 006 data = 1;  ota_status  = E_OTA_UPGRADE_ERASE_SEND;
			//TODO: if 006 data = 0;  ota_staus = E_OTA_UPGRADE_UPDATE_DATA_SEND;
			if(1 == FIT_CANFD_MOTOR_UPGRADE_UPDATE_ERASE_ACK_Get())
			{
				ota_status = E_OTA_UPGRADE_ERASE_SEND;
			}
			else if(0 == FIT_CANFD_MOTOR_UPGRADE_UPDATE_ERASE_ACK_Get())
			{
				ota_status = E_OTA_UPGRADE_UPDATE_DATA_SEND;
				FIT_CANFD_MOTORO_UPGRADE_Memory_Free();
				FIT_CANFD_MOTORO_UPGRADE_Memory_Get();
			}
			break;

		case E_OTA_UPGRADE_UPDATE_DATA_SEND:
			if(0==FIT_CANFD_MOTORO_UPGRADE_UPDATE_DATA_send(E_OTA_UPGRADE_UPDATE_DATA_STATUS_BEGIN))
			{
				ota_status = E_OTA_UPGRADE_UPDATE_DATA_WAITING_ACK;
			}
			else
			{
				ota_status = E_OTA_UPGRADE_UPDATE_FINISH_SEND;
			}
			break;

		case E_OTA_UPGRADE_UPDATE_DATA_SEND_RETRY:
		
			FIT_CANFD_MOTORO_UPGRADE_UPDATE_DATA_send(E_OTA_UPGRADE_UPDATE_DATA_STATUS_RETRY);
			ota_status = E_OTA_UPGRADE_UPDATE_DATA_WAITING_ACK;
			break;

		case E_OTA_UPGRADE_UPDATE_DATA_WAITING_ACK:
			if(1 == FIT_CANFD_MOTOR_UPGRADE_UPDATE_DATA_ACK_Get())
			{
				ota_status = E_OTA_UPGRADE_UPDATE_DATA_SEND_RETRY;
			}
			else if(0 == FIT_CANFD_MOTOR_UPGRADE_UPDATE_DATA_ACK_Get())
			{
				ota_status = E_OTA_UPGRADE_UPDATE_DATA_SEND;
			}	
			break;

		case 	E_OTA_UPGRADE_UPDATE_FINISH_SEND:
			FIT_CANFD_MOTORO_UPGRADE_UPDATE_FINISH_send();
			ota_status = E_OTA_UPGRADE_UPDATE_FINISH_WAITING_ACK;
			break;

		case 	E_OTA_UPGRADE_UPDATE_FINISH_WAITING_ACK:
			if(1 == FIT_CANFD_MOTOR_UPGRADE_UPDATE_FINISH_ACK_Get())
			{
				ota_status = E_OTA_UPGRADE_ERASE_SEND;
			}
			else if(0 == FIT_CANFD_MOTOR_UPGRADE_UPDATE_FINISH_ACK_Get())
			{
				flag_do_mc_fw_update = false;
				is_release_rom = true;
                                delay_msec( 2500 );
                                prepare_hmi_connect_data_send();
                                is_request_ascii_str_pages = true;
                                hmi_req_ascii_count = 0;
			}	
			break;

		default:break;
		
	}
}

/**
* @fn process_key_press
* @brief Handle the action response of the button press
* 
*/
void process_key_press(void)
{
	display_page = (E_DISPLAY_PAGE*)FIT_DISPLAY_PAGE_Get();
	if( (fw_get_gpio_pin(PIN_PWR_MODE_H) == 0) && (key_st[0] != 0) ) {
		key_st[0] = 0 ;
		LOG_INF("Press Assist Increase Button.");
		if(!flag_button_lock )	//Check if has no error
		{
			flag_global_up_button_press=true;
		}
		if((*display_page == E_DISPLAY_PAGE_BLE_PAIRING_FIRST) ||(*display_page == E_DISPLAY_PAGE_BLE_PAIRED) )
		{
			flag_ble_page_jump=true;
		}
		if(*display_page == E_DISPLAY_PAGE_CHARGING )
		{
			flag_global_up_button_press=true;
		}
	}
	if( (fw_get_gpio_pin(PIN_PWR_MODE_H) != 0) && (key_st[0] == 0) ){
		key_st[0] = 1 ;
		LOG_INF("Release Assist Increase Button.");
		if(!flag_button_lock )	
		{
			flag_global_up_button_press=false;
		}
		if(*display_page == E_DISPLAY_PAGE_CHARGING )
		{
			flag_global_up_button_press=false;
		}
		if((*display_page == E_DISPLAY_PAGE_BLE_PAIRING_FIRST) ||(*display_page == E_DISPLAY_PAGE_BLE_PAIRED) )
		{
			flag_ble_page_jump=false;
		}
	}


	if( (fw_get_gpio_pin(PIN_PWR_MODE_L) == 0) && (key_st[1] != 0) ) {
		key_st[1] = 0 ;
		LOG_INF("Press Assist Decrease Button");
		if(!flag_button_lock )	 //Check if has no error
		{
			flag_global_down_button_press=true;
		}
		if((*display_page == E_DISPLAY_PAGE_BLE_PAIRING_FIRST) ||(*display_page == E_DISPLAY_PAGE_BLE_PAIRED) )
		{
			flag_ble_page_jump=true;
		}
	}
	if( (fw_get_gpio_pin(PIN_PWR_MODE_L) != 0) && (key_st[1] == 0) ){
		key_st[1] = 1 ;
		LOG_INF("Release Assist Decrease Button.");
		if(!flag_button_lock )	
		{
			flag_global_down_button_press=false;
		}
		if((*display_page == E_DISPLAY_PAGE_BLE_PAIRING_FIRST) ||(*display_page == E_DISPLAY_PAGE_BLE_PAIRED) )
		{
			flag_ble_page_jump=false;
		}
	}



	if( (fw_get_gpio_pin(MCU_FUNCTION) == 0) && (key_st[2] != 0) ) {
		key_st[2] = 0 ;
		LOG_INF("Press Screen Scroll Button.");
		if(!flag_button_lock )	 //Check if has no error
		{
			flag_global_scroll_button_press=true;
			button_press_count +=1;	//Count how many button is be pressed
		}
		if((*display_page == E_DISPLAY_PAGE_BLE_PAIRING_FIRST) ||(*display_page == E_DISPLAY_PAGE_BLE_PAIRED) )
		{
			flag_ble_page_jump=true;
		}
	}
	if( (fw_get_gpio_pin(MCU_FUNCTION) != 0) && (key_st[2] == 0) ){
		key_st[2] = 1 ;
		LOG_INF("Release Screen Scroll Button.");
		if(!flag_button_lock )	
		{
			flag_global_scroll_button_press=false;
		}
		if(button_press_count>0)
		{
			button_press_count -=1;	//Count how many button is be released
		}
		if((*display_page == E_DISPLAY_PAGE_BLE_PAIRING_FIRST) ||(*display_page == E_DISPLAY_PAGE_BLE_PAIRED) )
		{
			flag_ble_page_jump=false;
		}
	}

	
	if( (fw_get_gpio_pin(PIN_PWR_KEY_DET) == 0) && (key_st[3] != 0) ) {
		key_st[3] = 0 ;
		LOG_INF("Press Power Key Button.");
		flag_global_power_button_press=true;
		button_press_count +=1;	//Count how many button is be pressed
		if((*display_page == E_DISPLAY_PAGE_BLE_PAIRING_FIRST) ||(*display_page == E_DISPLAY_PAGE_BLE_PAIRED) )
		{
			flag_ble_page_jump=true;
		}
	}
	if( (fw_get_gpio_pin(PIN_PWR_KEY_DET) != 0) && (key_st[3] == 0) ){
		key_st[3] = 1 ;
		LOG_INF("Release Power Key Button.");
		flag_global_power_button_press=false;
		if(button_press_count>0)
		{
			button_press_count -=1;	//Count how many button is be released
		}
		if((*display_page == E_DISPLAY_PAGE_BLE_PAIRING_FIRST) ||(*display_page == E_DISPLAY_PAGE_BLE_PAIRED) )
		{
			flag_ble_page_jump=false;
		}
	}
	if( (fw_get_gpio_pin(MCU_LIGHT_CRT) == 0) && (key_st[4] != 0) ) {
		key_st[4] = 0 ;
		LOG_INF("Press Light Key Button.");
		flag_global_light_button_press=true;
		if((*display_page == E_DISPLAY_PAGE_BLE_PAIRING_FIRST) ||(*display_page == E_DISPLAY_PAGE_BLE_PAIRED) )
		{
			flag_ble_page_jump=true;
		}
	}
	if( (fw_get_gpio_pin(MCU_LIGHT_CRT) != 0) && (key_st[4] == 0) ){
		key_st[4] = 1 ;
		LOG_INF("Release Light Key Button.");
		flag_global_light_button_press=false;
		if((*display_page == E_DISPLAY_PAGE_BLE_PAIRING_FIRST) ||(*display_page == E_DISPLAY_PAGE_BLE_PAIRED) )
		{
			flag_ble_page_jump=false;
		}
	}
}

void FIT_CANFD_TIMEOUT_ERROR_Get(void){
    //C E032
    if(flag_global_system_motor_can_lose_activate)
      global_CAN_time_error_code |= 0x01 ;
    //B1 E032
    if(flag_global_system_main_bms_can_lose_activate)
      global_CAN_time_error_code |= 0x02 ;
}



uint8_t ble_status_bk=0;

void process_ble_info(void)
{

	if(flag_global_charging_activate == 0 				&&
	   flag_walk_mode_activate == 0 					&&
	   flag_global_locked_activate == 0 				&&
	   flag_global_system_motor_error_activate == 0		&&
	   flag_global_system_main_bms_error_activate == 0	&&
	   flag_global_system_ex_bms_error_activate ==0		&&
	   flag_global_system_motro_error_general_activate ==0 &&
	   flag_global_system_motro_error_general_fw_err_activate == 0)
	{
		if( g_ble_peripheral_state.bits.pairing_complete == 1  && g_ble_peripheral_state.bits.bonded == 0 )
			global_ebike_bluetooth = g_ble_peripheral_state.bits.pairing_complete;
		else if(g_ble_peripheral_state.bits.bonded == 1 && flag_global_display_ble_page_showed == true)
			global_ebike_bluetooth = g_ble_peripheral_state.bits.pairing_complete;

		if(global_ebike_bluetooth==0 && g_ble_peripheral_state.bits.connected == 0)
			flag_global_display_ble_page_showed =false;
	}
	else
	{
		if(g_ble_peripheral_state.bits.pairing_complete >= 1 )
		{
			global_ebike_bluetooth = 1;
			flag_global_display_ble_page_showed = true;
		}
		else
		{
			global_ebike_bluetooth = 0;
		}
	}
	
	if(ble_status_bk != global_ebike_bluetooth)
	{
		//FIT_DISPLAY_TIME_CLEAN();
		ble_status_bk = global_ebike_bluetooth;
	}
	//TODO : flag_switch_speed_units is used to switch speed units by app side.
}

void process_canbus_info(void)
{
	;
}


struct hmi_pass_device_parameter hmi_moto ;
struct hmi_pass_device_parameter hmi_battery ;
struct hmi_pass_device_parameter hmi_controller ;
struct hmi_pass_system_info hmi_sys_info ;

uint8_t motor_serial_number_1[16] ;
//uint8_t motor_serial_number_2[16] ;
uint8_t motor_model_number[16] ;
uint8_t motor_sw_ver[16] ;
uint8_t motor_hw_ver[16] ;
uint8_t controller_serial_number_1[16] ;
uint8_t controller_serial_number_2[16] ;
uint8_t controller_sw_ver[16] ;
uint8_t battery_serial_number_1[16] ;
//uint8_t battery_serial_number_2[16] ;
uint8_t battery_model_number[16] ;
uint8_t battery_sw_ver[16] ;
uint8_t motor_manufacturer[16] ;
uint8_t battery_manufacturer[16] ;

void prepare_hmi_connect_data_send()
{
        int i;
        for( i=0; i<3; i++){
            FIT_CANFD_PROCESS_MC_FW_VERSION_Send();
            FIT_CANFD_PROCESS_MC_PRODUCTION_INFO_Send();
            FIT_CANFD_PROCESS_MC_SERIAL_NO_HIGH_Send();
            FIT_CANFD_PROCESS_MC_SERIAL_NO_LOW_Send();
            FIT_CANFD_PROCESS_MC_FW_VERSION_Send();
            FIT_CANFD_PROCESS_BMS_SN_PRODUCTION_Send();
            FIT_CANFD_PROCESS_BMS_FW_VERSION_Send();
            FIT_CANFD_PROCESS_MAIN_BMS_DISCHARGE_Send();
        }
}

void prepare_hmi_connect_data()
{
        // page1 motor controller info
        FIT_CANFD_PROCESS_MC_MCOD_Get(&MC_MCOD);
        FIT_CANFD_PROCESS_MC_PRODUCTION_INFO_Get(&MC_DAY, &MC_MONTH, &MC_YEAR);
        FIT_CANFD_PROCESS_MC_SERIAL_NO_HIGH_Get(&MC_SN_H[0], &MC_SN_H[1], &MC_SN_H[2], &MC_SN_H[3], &MC_SN_H[4], &MC_SN_H[5], &MC_SN_H[6], &MC_SN_H[7]);
        FIT_CANFD_PROCESS_MC_SERIAL_NO_LOW_Get(&MC_SN_L[0], &MC_SN_L[1], &MC_SN_L[2], &MC_SN_L[3], &MC_SN_L[4], &MC_SN_L[5], &MC_SN_L[6], &MC_SN_L[7]);
        FIT_CANFD_PROCESS_MC_MODEL_Get(&MC_MODEL);
        
        FIT_CANFD_PROCESS_MC_FMV_Get(&MC_FW_M_V_H, &MC_FW_M_V_L);
        FIT_CANFD_PROCESS_MC_FSV_Get(&MC_FW_S_V_H, &MC_FW_S_V_L);
        FIT_CANFD_PROCESS_MC_HW_TYPE_Get(&MC_HW_Type_H, &MC_HW_Type_L);
        hmi_moto.manufacturer = MC_MCOD ;
        hmi_moto.manufacture_year = MC_YEAR ;
        hmi_moto.manufacture_month = MC_MONTH ;
        hmi_moto.manufacture_day = MC_DAY ;
        for (int i = 0; i < 8; i++) {
            hmi_moto.serial_number |= ((uint32_t)MC_SN_H[i] << (7 - i));
            hmi_moto.serial_number |= ((uint32_t)MC_SN_L[i] << (15 - i));
        }
        hmi_moto.model_number = MC_MODEL;
        hmi_moto.sw_ver = (((uint32_t)(MC_FW_M_V_H - 0x30) << 24) | 
                           ((uint32_t)(MC_FW_M_V_L - 0x30) << 16) | 
                           ((uint32_t)(MC_FW_S_V_H - 0x30) << 8) | 
                                      (MC_FW_S_V_L - 0x30));
        hmi_moto.hw_ver = ((uint16_t)MC_HW_Type_H << 8) | MC_HW_Type_H;

        // page3 battery info
        FIT_CANFD_PROCESS_BMS_MCOD_Get(&BMS_MCOD);
        FIT_CANFD_PROCESS_BMS_PRODUCTION_INFO_Get(&BMS_DAY, &BMS_MONTH, &BMS_YEAR);
        FIT_CANFD_PROCESS_BMS_SN_Get(&BMS_SSERIAL_NUMBER);
        FIT_CANFD_PROCESS_BMS_MODEL_Get(&BMS_MODEL);
        FIT_CANFD_PROCESS_BMS_FW_VERSION_Get(&BMS_FW_MAIN_VERSION, &BMS_FW_SECOND_VERSION, &BMS_FW_DEBUG_SERIAL);
	hmi_battery.manufacturer = BMS_MCOD ;
        hmi_battery.manufacture_year = BMS_YEAR ;
	hmi_battery.manufacture_month = BMS_MONTH ;
	hmi_battery.manufacture_day = BMS_DAY ;
        hmi_battery.serial_number = BMS_SSERIAL_NUMBER ;
        hmi_battery.model_number = BMS_MODEL ;
        hmi_battery.sw_ver = ((uint32_t)BMS_FW_MAIN_VERSION << 16) | BMS_FW_SECOND_VERSION ; 

	// page4 hmi_sys info
	hmi_sys_info.battery_system_year = BMS_YEAR ; // Plus 2000 to get A.D. year
	hmi_sys_info.battery_system_month = BMS_MONTH ;
	hmi_sys_info.battery_system_day = BMS_DAY ;

        //page11
        motor_serial_number_1[0] = (hmi_moto.serial_number>>24) & 0xFF ;
        motor_serial_number_1[1] = (hmi_moto.serial_number>>16) & 0xFF ;
        motor_serial_number_1[2] = (hmi_moto.serial_number>>8) & 0xFF ;
        motor_serial_number_1[3] = hmi_moto.serial_number & 0xFF ;
        //page13
        motor_model_number[0] = (hmi_moto.model_number>>8) & 0xFF ;
        motor_model_number[1] = hmi_moto.model_number & 0xFF ;
        //page14
        motor_sw_ver[0] = (hmi_moto.sw_ver>>24) & 0xFF ;
        motor_sw_ver[1] = (hmi_moto.sw_ver>>16) & 0xFF ;
        motor_sw_ver[2] = (hmi_moto.sw_ver>>8) & 0xFF ;
        motor_sw_ver[3] = hmi_moto.sw_ver & 0xFF ;
        //page15
        motor_hw_ver[0] = (hmi_moto.hw_ver>>8) & 0xFF ;
        motor_hw_ver[1] = hmi_moto.hw_ver & 0xFF ;
        //page16,17
        fw_flash_read(PCBA_PID , 12 , g_chPCBA_PID);
        fw_flash_read(ALLSYSTEM_PID , 12 , g_chAllsystem_PID );
        for (int i = 0; i < 16; i++) {
            controller_serial_number_1[i] = (uint8_t)g_chPCBA_PID[i];
            controller_serial_number_2[i] = (uint8_t)g_chAllsystem_PID[i];
        }
        //page19
        controller_sw_ver[0] = FW_VERSION_MAIN;
        controller_sw_ver[1] = FW_VERSION_SECOND;
        controller_sw_ver[2] = FW_VERSION_DEBUG_SERIAL;
        //page21
        battery_serial_number_1[0] = (hmi_battery.serial_number>>24) & 0xFF ;
        battery_serial_number_1[1] = (hmi_battery.serial_number>>16) & 0xFF ;
        battery_serial_number_1[2] = (hmi_battery.serial_number>>8) & 0xFF ;
        battery_serial_number_1[3] = hmi_battery.serial_number & 0xFF ;
        //page23
        motor_model_number[0] = (hmi_moto.model_number>>8) & 0xFF ;
        motor_model_number[1] = hmi_moto.model_number & 0xFF ;
        //page24
        battery_sw_ver[0] = (hmi_battery.sw_ver>>24) & 0xFF ;
        battery_sw_ver[1] = (hmi_battery.sw_ver>>16) & 0xFF ;
        battery_sw_ver[2] = (hmi_battery.sw_ver>>8) & 0xFF ;
        battery_sw_ver[3] = hmi_battery.sw_ver & 0xFF ;
        //page26
        motor_manufacturer[0] = (hmi_moto.manufacturer>>24) & 0xFF ;
        motor_manufacturer[1] = (hmi_moto.manufacturer>>16) & 0xFF ;
        motor_manufacturer[2] = (hmi_moto.manufacturer>>8) & 0xFF ;
        motor_manufacturer[3] = hmi_moto.manufacturer & 0xFF ;
        //page28
        battery_manufacturer[0] = (hmi_battery.manufacturer>>24) & 0xFF ;
        battery_manufacturer[1] = (hmi_battery.manufacturer>>16) & 0xFF ;
        battery_manufacturer[2] = (hmi_battery.manufacturer>>8) & 0xFF ;
        battery_manufacturer[3] = hmi_battery.manufacturer & 0xFF ;

        //page8
        FIT_CANFD_PROCESS_MAIN_BMS_CYCLE_COUNT_Get(&global_bms_cycle_count);
}


#define SENT_CGM_NORMAL		20
#define SENT_CGM_N7		(SENT_CGM_NORMAL+1)
#define SENT_CGM_N8		(SENT_CGM_NORMAL+2)
#define SENT_CGM_N9		(SENT_CGM_NORMAL+3)
#define SENT_CGM_N10		(SENT_CGM_NORMAL+4)
#define SENT_CGM_N40		(SENT_CGM_NORMAL+5)
int ble_init = 0 ; 
int normal_delay_count ; 
void process_ble_report(void)
{
	if( ! fw_is_bluetooth_connected() ){
		is_do_normal_report_hmi = false ;
		return ;
	}
	if( is_hmi_connect_flow_start ){		
			switch( hmi_sent_step_count ){
                    case 0 :	prepare_hmi_connect_data();			break ;
                    case 1 :	sent_page( 1 , (uint8_t *)&hmi_moto );		break ;
                    case 2 :	sent_page( 2 , (uint8_t *)&hmi_controller );	break ;
                    case 3 :	sent_page( 3 , (uint8_t *)&hmi_battery );	break ;
                    case 4 :	sent_page( 4 , (uint8_t *)&hmi_sys_info );	break ;
                    case 5 :	is_hmi_connect_flow_start = false ;	
                                flag_time_start = false;
                                hmi_sent_step_count = -1 ;	
                                is_do_normal_report_hmi = true;		
                                return ;
            }
            hmi_sent_step_count++ ;
			return ;
	}
	if( is_have_hmi_event ){
		switch( hmi_event_step_count ){
		case 0 :	sent_page( 6 , (uint8_t *)&m_event_data );
				is_have_hmi_event = false ;
				is_do_normal_report_hmi = true ;
				hmi_event_step_count = -1 ;			return ;
		}
		hmi_event_step_count++ ;
		return ;
	}
	if( is_by_pass_mode ){
		if( is_read_can_msg4pass ){
			sent_page( 31 , (uint8_t *)&m_can_buf );
			is_read_can_msg4pass = false ;
		}
		return ;
	}
        if( is_request_ascii_str_pages ) {
                switch( hmi_req_ascii_count ){
                  case 0 :	prepare_hmi_connect_data();		                  break ;
                  case 1 :	sent_page( 11 , (uint8_t *)motor_serial_number_1 );       break ;
                  case 2 :	sent_page( 13 , (uint8_t *)motor_model_number );          break ;
                  case 3 :	sent_page( 14 , (uint8_t *)motor_sw_ver );                break ;
                  case 4 :	sent_page( 15 , (uint8_t *)motor_hw_ver );                break ;
                  case 5 :	sent_page( 16 , (uint8_t *)controller_serial_number_1 );  break ;
                  case 6 :	sent_page( 17 , (uint8_t *)controller_serial_number_2 );  break ;
                  case 7 :      sent_page( 19 , (uint8_t *)controller_sw_ver );           break ;
                  case 8 :	sent_page( 21 , (uint8_t *)battery_serial_number_1 );     break ;
                  case 9 :	sent_page( 23 , (uint8_t *)battery_model_number );        break ;
                  case 10 :	sent_page( 24 , (uint8_t *)battery_sw_ver );              break ;
                  case 11 :	sent_page( 26 , (uint8_t *)motor_manufacturer );          break ;
                  case 12 :	sent_page( 28 , (uint8_t *)battery_manufacturer );        break ;
                  case 13 :	is_request_ascii_str_pages = false ;
  				hmi_req_ascii_count = -1 ;                                return ;
		}
		hmi_req_ascii_count++ ;

		return ;
	}
	if( is_do_normal_report_hmi ) {
		normal_delay_count++ ;
		if(normal_delay_count > SENT_CGM_NORMAL ){ // 50 * 20 = 1 sec sent one times
			switch( normal_delay_count ) {
			case SENT_CGM_N7 :	sent_page( 7 , (uint8_t *)&m_normal_data );	break ;
			case SENT_CGM_N8 :	sent_page( 8 , (uint8_t *)&m_normal_data );	break ;
			case SENT_CGM_N9 :	sent_page( 9 , (uint8_t *)&m_normal_data );	break ;
			case SENT_CGM_N10 :	sent_page( 10 , (uint8_t *)&m_normal_data );	break ;
			case SENT_CGM_N40 :	sent_page( 10 , (uint8_t *)&m_normal_data );
						normal_delay_count = 0 ;			break ;
			}
		}
		return ;
	}
       
}

extern int get_thermal_adc();
int st_val ,m_remote1, m_remote2;
int gpio_out_dbg = 0 ;
/**
* @fn check_if_charger_enable
* @brief check if support power for USB
* @details For external mobile phones, decide whether to switch the power supply function
*/
void check_if_charger_enable(void)
{
     int det_5v ;
     flash_adc();
     det_5v = get_vbus_5v_det();
     if( ((get_usb_cc_adc() > 8800) && (get_usb_cc_adc() < 9600) ) && (det_5v == 1) ){ // Data Transmission
      usb_load_switch_control( 0 );
      return ;
     }
     if( ( ((get_usb_cc_adc() > 8800) && (get_usb_cc_adc() < 9600) ) ||   // BC 1.2 => det 5V = Lo && CC = 5V
           ((get_usb_cc_adc() > 4800) && (get_usb_cc_adc() < 5400) ) ) && // PD charging => det 5V = Lo && CC = 1.698V
         (det_5v == 0) ){
      usb_load_switch_control( 1 );
     }
     st_val = get_thermal_adc();
     m_remote1 = get_remote1_adc();
     m_remote2 = get_remote2_adc();
     cc1 = get_usb_cc1_adc() ;
     cc2 = get_usb_cc2_adc() ;
     // CC1 < 5V && CC1 < CC2 => USB_CTL1 USB_CTL2 USB_CTL3 : 0 0 1
     // CC2 < 5V && CC2 < CC1 => USB_CTL1 USB_CTL2 USB_CTL3 : 0 0 1
    if( ( (cc1 < 8800) && (cc1 < cc2) ) || ( (cc2 < 8800) && (cc2 < cc1) ) )
    {
            if(st_val < 1000)
            {
                    sample_time_01 ++;
                    if(sample_time_01 == 100)
                    {
                            fw_set_gpio_pin( PIN_USB_CTL1 , 0 );
                            fw_set_gpio_pin( PIN_USB_CTL3 , 0 );
                            gpio_out_dbg = 3001 ;
                            sample_time_01 = 0;
                            sample_time_02 = 0;
                    }
            }
            else if(st_val > 1350)
            {
                    sample_time_02++;
                    if(sample_time_02==100)
                    {
                            fw_set_gpio_pin( PIN_USB_CTL1 , 0 );
                            fw_set_gpio_pin( PIN_USB_CTL3 , 1 );
                            gpio_out_dbg = 3001 ;
                            sample_time_01 = 0;
                            sample_time_02 = 0;
                    }
            }
            else
            {
                    ;
            }
    }

     // cc1 > 5V && cc2 > 5V
     if( (cc1 > 8800) && (cc2 > 8800) ) {
      fw_set_gpio_pin( PIN_USB_CTL1 , 1 );
      fw_set_gpio_pin( PIN_USB_CTL3 , 1 );
      gpio_out_dbg = 3101 ;
     }
     
// Modification Vincent 2023/08//08
     if( (cc1 > 15300) || (cc2 > 15300) )
     {
	 flag_usb_plugged = 1;
     }
     if( (cc1 < 15300) && (cc2 < 15300) )
     {
	 flag_usb_plugged = 0;
     }


     if(flag_usb_plugged_buf !=flag_usb_plugged)
     {
	     if(flag_usb_plugged)
	     {
		CAN_PAGE_DBG(E_DBG_MESSAGE_USB_PLUG_IN);
	     }
	     else
	     {
		CAN_PAGE_DBG(E_DBG_MESSAGE_USB_PLUG_OUT);
	     }
	     flag_usb_plugged_buf =flag_usb_plugged;	
     }
// Modification Vincent 2023/08//08
}


#define FLASH_TEST_REGION_OFFSET 0x0000
#define FLASH_SECTOR_SIZE        4096

#define FLASH_DEVICE DT_LABEL(DT_INST(0, nordic_qspi_nor))
int cmp_err,rc;
int fail_f = 0 ;
	uint8_t test2_out[128] , test2_in[128] ;
void main_test(void)
{
//	const uint8_t expected[] = { 0x55, 0xaa, 0x66, 0x99 };
	const size_t len = sizeof(test2_out);
	const struct device *flash_dev;
	int i;


	flash_dev = device_get_binding(FLASH_DEVICE);

	memset(test2_in, 0, 0);
	for (i = 0;i < 128 ;i++ ) {
		test2_out[ i ] = i ;
	}

	if (!flash_dev) {
	fail_f = 1 ;
		return;
	}

	/* Write protection needs to be disabled before each write or
	 * erase, since the flash component turns on write protection
	 * automatically after completion of write and erase
	 * operations.
	 */

	rc = flash_erase(flash_dev, FLASH_TEST_REGION_OFFSET,
			 FLASH_SECTOR_SIZE);
		delay_msec( 10 );
	rc = flash_read(flash_dev, FLASH_TEST_REGION_OFFSET, test2_in, len);
	if (rc != 0) {
	fail_f = 2 ;
		return;
	}
		delay_msec( 10 );
	rc = flash_write(flash_dev, FLASH_TEST_REGION_OFFSET, test2_out, len);
	if (rc != 0) {
	
	fail_f = 3 ;
		return;
	}

		delay_msec( 10 );
	rc = flash_read(flash_dev, FLASH_TEST_REGION_OFFSET, test2_in, len);
	if (rc != 0) {
	fail_f = 4 ;
		return;
	}

	if (memcmp(test2_out, test2_in , len) == 0) {
		cmp_err = 0 ;
	} else {
		cmp_err = 0 ;
		for (i = 0;i < 128 ;i++ ) {
			if( test2_out[ i ] != test2_in[i] )cmp_err++;
		}
	}
}

#if 1

int hour;
int mins;
int remaining_seconds,temp_seconds;


void read_and_convert_time() {
    struct timeval tv;
    int ret = gettimeofday(&tv, NULL);
    if (ret != 0) {
        return;
    }
    if((g_ble_peripheral_state.bits.bonded >= 1 )&&(g_ble_peripheral_state.bits.connected >=1)&&(!flag_time_start))
    {
	flag_time_start = true;
	temp_seconds = tv.tv_sec;
    }

    seconds = (tv.tv_sec-temp_seconds+total_seconds);
    
    hour = (seconds / 3600) % 24;
    mins = ((seconds % 3600) / 60);
    remaining_seconds = (seconds % 3600) % 60;


        global_time_min = mins;
	global_time_min_tens_digit =  mins/ 10;
	global_time_min_units_digit = mins % 10;
	global_time_hour_tens_digit =  hour/10;
	global_time_hour_units_digit = hour % 10;
}
#endif

extern const struct device *spi1_devs ;
uint8_t wr_cmd[4] ;
uint8_t wr_dat[4] ;
uint8_t rd_buf[4] ;
/**
* @fn main
* @brief main itinerary
* As the main itinerary, it is responsible for accepting and dispatching orders from the mobile phone or computer to this bicycle. <br>
* And take out the data from the motor controller or power controller, and transmit it to the rider together with the peripheral devices. <br>
* Or accept the rider's command such as: refueling, braking and other actions, and pass the command out. <br>
* Finally, there is also the job of recording some driving information.
*/


//testKen
S_MONITOR *monitor_data;
uint64_t timer_100ms = 0; //Wade231120: For mmWave test
uint8_t timer_CANID_18A = 0;
uint8_t timer_CANID_18A_count = 0;
uint8_t timer_CANID_283 = 0;
uint8_t timer_CANID_283_count = 0;
uint8_t timer_CANID_2A3 = 0;
uint8_t timer_CANID_2A3_count = 0;
uint8_t timer_CANID_104_lock_count = 0;
uint8_t timer_CANID_103_unlock_count = 0;
uint8_t timer_CANID_102_shut_down_count = 0;
uint64_t timer_ble_page_counter = 0;
bool flag_long_press_down_btn = false;

static void sys_timer_handler(struct k_timer *dummy)
{		
	//Wade231120: For mmWave test +++
        //FIT_CANFD_PROCESS_HMI_PERIODICAL_Send(); //every 25ms

        timer_100ms +=1;
        if(timer_100ms >= 4)
        {
          //FIT_CANFD_PROCESS_REMAINING_DATA_Send(); //every 100ms
          timer_100ms = 0;
        }
        //Wade231120: For mmWave test ---  
		/*	for ble conter*/
		if(flag_global_display_ble_page_counter)
		{					
			power_saving_timer_10min = 0;
			flag_global_power_saving_activate = false;	
			timer_ble_page_counter++;

			if( flag_global_system_motor_error_activate || 
				flag_global_system_main_bms_error_activate || 
		    	flag_global_system_ex_bms_error_activate || 
		    	flag_global_system_motro_error_general_activate ||
		    	flag_global_system_motro_error_general_fw_err_activate||
				flag_ble_page_jump )
				{
		    		timer_ble_page_counter = 600;
					flag_ble_page_jump = false;
				}
			if(timer_ble_page_counter>=600)
			{				
				flag_global_display_ble_page_counter = false;
				flag_global_display_ble_page_showed = true;//required set to false when ble disconnect.
				timer_ble_page_counter = 0;
			}
		}
		else
		{
			timer_ble_page_counter  = 0;
		}
	if(flag_global_up_button_press)
		{
			up_key_press_timer_2000ms++;			
		}
		else
		{	
			if(up_key_press_timer_2000ms>0)//Up Button Release
			{
				display_page = (E_DISPLAY_PAGE*)FIT_DISPLAY_PAGE_Get();
				if( *display_page != E_DISPLAY_PAGE_ERROR &&
					*display_page != E_DISPLAY_PAGE_CHARGING &&
					*display_page != E_DISPLAY_PAGE_WALK_ASSIST)
				{
					if(!flag_global_power_saving_activate) 
					{
						global_motor_assist_level++;
						if(global_motor_assist_level>4)
						{
							global_motor_assist_level=4;	
						}
					}
				}
				up_key_press_timer_2000ms=0;	
				/* cancel power saving counter*/
				power_saving_timer_10min = 0;
				flag_global_power_saving_activate = false;	
				FIT_CANFD_PROCESS_DBG_BTN_Send(E_DBG_MESSAGE_BTN_ASSIST_INCREASE, 1);
			}
			else
			{
				;
			}				
		}
		//Light Control 
		if(flag_global_light_button_press)
		{
			light_press_timer_2000ms++;
			if(light_press_timer_2000ms>=0)
			{
				if((global_ebike_light==0)&&(flag_global_light_button_release==false))
				{
					if(!flag_global_power_saving_activate)
					{
						flag_global_light_button_release=true;
						global_ebike_light=1;		
						CAN_PAGE_DBG(E_DBG_MESSAGE_LIGHT_ON);
					}
					/* cancel power saving counter*/
					power_saving_timer_10min = 0;
					flag_global_power_saving_activate = false;	
					CAN_BT_DBG(E_DBG_MESSAGE_BTN_ASSIST_INCREASE, 2);
				}
				else if((global_ebike_light==1)&&(flag_global_light_button_release==false))
				{
					if(!flag_global_power_saving_activate)
					{
						flag_global_light_button_release=true;
						global_ebike_light=0;
						CAN_PAGE_DBG(E_DBG_MESSAGE_LIGHT_OFF);
					}
					/* cancel power saving counter*/
					power_saving_timer_10min = 0;
					flag_global_power_saving_activate = false;	
					CAN_BT_DBG(E_DBG_MESSAGE_BTN_ASSIST_INCREASE, 2);
				}
				
			}	
		}
		else
		{
			if(light_press_timer_2000ms>=0)//Light Button Press Time >= 2secs
			{
				flag_global_light_button_release=false;
				light_press_timer_2000ms = 0;
			}
			else
			{
				light_press_timer_2000ms = 0;
			}
		}			

		//walk_assist
		if(flag_global_down_button_press==true)
		{
			down_key_press_timer_2000ms++;
			if(down_key_press_timer_2000ms>=100)
			{
				if(!flag_global_power_saving_activate)
				{
					flag_walk_mode_activate=true;
				}
				/* cancel power saving counter*/
				power_saving_timer_10min = 0;
				flag_global_power_saving_activate = false;	
			}
		}
		else
		{

			if((down_key_press_timer_2000ms<100)&&(down_key_press_timer_2000ms>0))//ÔøΩPÔøΩ_ÔøΩuÔøΩÔøΩ
			{
				if( !flag_global_power_saving_activate)
				{
					if(global_motor_assist_level==0)
					{
						;
					}
					else
					{	
						global_motor_assist_level--;
					}
				}
				down_key_press_timer_2000ms=0;		
				/* cancel power saving counter*/
				power_saving_timer_10min = 0;
				flag_global_power_saving_activate = false;	
			}
			else if(down_key_press_timer_2000ms>=100)	
			{			
				flag_walk_mode_activate=false;
				down_key_press_timer_2000ms=0;
			}
			else
			{
				;
			}	
		}

		//§Êñ∑âÂªÊµÊ≠Â‰Ω
		if((button_press_count==2)&&(flag_global_combo_press == false))
		{
			power_key_press_timer_4000ms++;
			if(power_key_press_timer_4000ms>=2)
			{
				flag_global_combo_press = true;
				power_key_press_timer_4000ms = 0;
				if(global_distance_unit==0)
				{
					global_distance_unit = 1;				
				}
				else 
				{
					global_distance_unit = 0;
				}
			}	
		}
		else if((button_press_count==1)&&(flag_global_power_button_press==true)&&(flag_global_combo_press==false))
		{
			power_key_press_timer_4000ms++;
			if(power_key_press_timer_4000ms>=200)//Ëß∏Áôº
			{
				if(flag_global_system_boot_on == true)
				{
					flag_off_logo_activate=true;
				}
				else
				{
					flag_global_system_boot_on = true;
					global_ebike_light=1;
				}
				flag_global_power_button_press =0;	
				power_key_press_timer_4000ms = 0;
				/* cancel power saving counter*/
				power_saving_timer_10min = 0;
				flag_global_power_saving_activate = false;	
			}
		}
		else
		{	if(flag_global_combo_press == false)
			{
				if((power_key_press_timer_4000ms<200)&&(power_key_press_timer_4000ms>0))
				{
					power_key_press_timer_4000ms=0;	
					/* cancel power saving counter*/
					power_saving_timer_10min = 0;
					flag_global_power_saving_activate = false;							
				}
				else if(power_key_press_timer_4000ms==0)
				{
					;
				}
			}
		}
		
		//§Êñ∑âÂõÈâÊâ≠ï‰
		if((button_press_count==2)&&(flag_global_combo_press == false))
		{
			scroll_key_press_timer_4000ms++;
			if(scroll_key_press_timer_4000ms>=2)
			{
				flag_global_combo_press = true;
				scroll_key_press_timer_4000ms = 0;
				if(global_distance_unit==0)
				{
					global_distance_unit = 1;
				}
				else 
				{
					global_distance_unit = 0;
				}
				/* cancel power saving counter*/
				power_saving_timer_10min = 0;
				flag_global_power_saving_activate = false;	
			}	
		}
		else if((button_press_count==1)&&(flag_global_scroll_button_press==true)&&(flag_global_combo_press==false))
		{
			scroll_key_press_timer_4000ms++;
			if(scroll_key_press_timer_4000ms>=400)//Ëß∏Áôº∑Ê4Áß
			{
				if(global_page_index==2)
				{	
					FIT_CANFD_PROCESS_CLEAR_TRIP_Send();
					flag_clear_trip_activate = true;
					/* cancel power saving counter*/
					power_saving_timer_10min = 0;
					flag_global_power_saving_activate = false;	
				}
				flag_global_scroll_button_press =false;	
			        scroll_key_press_timer_4000ms = 0;			
			}
		}
		else if((button_press_count==0)&&(flag_global_combo_press==true))
		{
			flag_global_combo_press = false;
			scroll_key_press_timer_4000ms = 0;
		}
		else
		{
			if(flag_global_combo_press == false)
			{
				if((scroll_key_press_timer_4000ms<400)&&(scroll_key_press_timer_4000ms>0))
				{
					if( !flag_global_charging_activate  			&&
			  	 	!flag_walk_mode_activate  				&&
			  		!flag_global_locked_activate  				&&
			   		!flag_global_system_motor_error_activate 		&&
			   		!flag_global_system_main_bms_error_activate 		&&
			   		!flag_global_system_ex_bms_error_activate 		&&
			  		!flag_global_system_motro_error_general_activate	&&
			   		!flag_global_system_motro_error_general_fw_err_activate &&	
	   				!flag_global_power_saving_activate)
					{	
						global_page_index++;
						if(global_page_index==4)
						{
							global_page_index=0;	
						}
					}
					scroll_key_press_timer_4000ms=0;							
					power_saving_timer_10min = 0;
					flag_global_power_saving_activate = false;	
				}
				else if(scroll_key_press_timer_4000ms==0)
				{
					;
				}
			}
		}	
}

static void timer_stop_handler(struct k_timer *timer)
{
  //  k_timer_stop(&my_timer);
    arch_nop();
}

static void set_io_high_level(void)
{
	nrf_gpio_cfg(
		40,
		NRF_GPIO_PIN_DIR_OUTPUT,
		NRF_GPIO_PIN_INPUT_DISCONNECT,
		NRF_GPIO_PIN_NOPULL,
		NRF_GPIO_PIN_H0H1,
		NRF_GPIO_PIN_NOSENSE);

	nrf_gpio_cfg(
		8,
		NRF_GPIO_PIN_DIR_OUTPUT,
		NRF_GPIO_PIN_INPUT_DISCONNECT,
		NRF_GPIO_PIN_NOPULL,
		NRF_GPIO_PIN_H0H1,
		NRF_GPIO_PIN_NOSENSE);
}

K_TIMER_DEFINE(sys_timer, sys_timer_handler, timer_stop_handler);

#ifdef CONFIG_NRFX_TEMP
static void temp_handler(int32_t temperature)
{ 
    m_soc_temperature = temperature;
    printk("Temperature (Celcius): %d \n",temperature / 4);
}

static void soc_temperature_peripheral_init(void)
{
    int err;
    nrfx_temp_config_t temp_config = NRFX_TEMP_DEFAULT_CONFIG;
    err = nrfx_temp_init(&temp_config,temp_handler);
    IRQ_CONNECT(TEMP_IRQn,5,nrfx_temp_irq_handler,NULL,0);
    if (err!=NRFX_SUCCESS) 
    {	
	    printk("soc_temperature_peripheral_init error %d\n", err);
    }
}
#endif

void watchdog_callback(int channel_id, void *user_data)
{
    // ®ÁÄóËÇÂô®Ë∂ÖÊÇÂü∑Ë°åÁûË™øΩÂ
    // Ø‰ª•®Ê≠§ïËÜÁõ∏âÁç‰Ôºå‰Â¶ÇÂ†±äÈåØË™§ÊçÊñ∞üÂÁ≥ªÁµ±
    LOG_INF("Watchdog timeout for channel %d\n", channel_id);
    NVIC_SystemReset();
}

void wdt_register(void)
{
    wdt_channel_id = task_wdt_add(CONFIG_TASK_WDT_MIN_TIMEOUT, watchdog_callback, NULL);
    if (wdt_channel_id < 0) {
        LOG_INF("Failed to add task watchdog timeout\n");
        return;
    }
}

void main(void)
{
        
	int ii = 1 ;
	global_erase_flag = 0;
		
#ifdef CONFIG_CPU_LOAD
        cpu_load_init(); 
#endif

#ifdef CONFIG_NRFX_TEMP
	soc_temperature_peripheral_init();
#endif

//	log_core_init();
    //---------modification list for Ken---------
	hal_init_system_device( FW_BLE_PERIPHERAL , FW_BLE_CENTRAL );
	k_timer_start(&sys_timer, K_SECONDS(0), K_MSEC(10));
	memset(key_st,0,5);
	//hw_test();
	//start_scan();

        //FTM flag init                 
        fw_flash_read(FACTORY_STATUS , 1 , ifactor_state_arr);
        if(ifactor_state_arr[0] == 255){
            ifactor_state_arr[0] = 7;
        }
        else if(ifactor_state_arr[0] != 6)
        {
            ifactor_state_arr[0] = 7;
        }
        else 
        {
            ;//other 
        }
        
        //Watchdog flag init
        fw_flash_read(WD_ADDRESS , 1 , watchdog_flag_arr);
        if(watchdog_flag_arr[0] == 255)
        {
            watchdog_flag_arr[0] = 0;
        }
        else if(watchdog_flag_arr[0] == 1)
        {
		    wdt_register();
        }
	//wdt_register();
	//set io to high level 
	set_io_high_level();
	//boot
	flag_global_system_boot_on = true;
	//dbg_message
	CAN_PAGE_DBG(E_DBG_MESSAGE_BOOT);
	FIT_CANFD_PROCESS_MAIN_BMS_DISCHARGE_Send();
        
	
	// Enter main loop.
	while ( 1 ) {
#ifdef 	CONFIG_CPU_LOAD	
        cpu_load_get();
#endif
		
#ifdef CONFIG_NRFX_TEMP
        if(NRFX_SUCCESS == nrfx_temp_measure()){
            LOG_INF("soc temperature is %d",m_soc_temperature/4);
        }
#endif
//Turn off Watchdog flag and force enter into watchdog
		if(watchdog_flag_arr[0])
		{
			task_wdt_feed(wdt_channel_id);
		}

		if(flag_do_mc_fw_update)
		{
			ota_start();
			k_sleep(K_USEC(100));
		}
		else
		{
			process_canbus_info();
			process_ble_report();
			delay_msec( 50 );	
			if((normal_delay_count > SENT_CGM_NORMAL) || is_request_ascii_str_pages || is_hmi_connect_flow_start){
				continue ;
			}
			process_ble_info();
			process_key_press();
			check_if_charger_enable();					
			if( is_release_rom ){
				fw_flash_erase( 0x700000 , 64 ); // 256K / 4096 = 64
				is_release_rom = false ;
				LOG_INF("Finish to Erase Program buffer in flash ROM");
			}
		}
	}
}
