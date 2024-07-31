
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <zephyr.h>
#include "fw_api.h"
#include "hal_api.h"


/* Public constants ----------------------------------------------------------*/


/* Public variables ----------------------------------------------------------*/
//Debug Mode
int g_iDebug = 0;//0=release 1, 1=debug

/*boot*/
uint8_t flag_global_system_boot_on = 0;

/*locked*/
uint8_t  flag_global_locked_activate	= 0;

//LCM
uint8_t global_motor_current_speed_tens_digit=0;
uint8_t global_motor_current_speed_units_digit=0;
int8_t  global_motor_current_speed_tens_digit_buf=-1;
int8_t  global_motor_current_speed_units_digit_buf=-1;
uint8_t global_ebike_light=0;
int8_t  global_ebike_light_buf=-1;
uint8_t global_ebike_bluetooth=0;
int8_t  global_ebike_bluetooth_buf=-1;
uint8_t global_time_hour_tens_digit=0;
uint8_t global_time_hour_units_digit=0;
uint8_t global_time_min_tens_digit=0;
uint8_t global_time_min_units_digit=0;
uint8_t	global_motor_est_range_houndred_digit=0;
uint8_t	global_motor_est_range_tens_digit=0;
uint8_t	global_motor_est_range_units_digit=0;
int8_t	global_motor_est_range_houndred_digit_buf=-1;
int8_t	global_motor_est_range_tens_digit_buf=-1;
int8_t	global_motor_est_range_units_digit_buf=-1;
uint8_t global_bms_soc_hundreds_digit = 0;
uint8_t global_bms_soc_tens_digit = 0;
uint8_t global_bms_soc_units_digit = 0;
uint8_t global_main_bms_soc_hundreds_digit = 0;
uint8_t global_main_bms_soc_tens_digit = 0;
uint8_t global_main_bms_soc_units_digit = 0;
uint8_t global_ex_bms_soc_hundreds_digit = 0;
uint8_t global_ex_bms_soc_tens_digit = 0;
uint8_t global_ex_bms_soc_units_digit = 0;
uint8_t global_motor_assist_level=0;
uint8_t global_motor_system_status; 
uint8_t global_erase_flag = 0;
uint8_t global_pic_error_flag = 0;
uint8_t global_counter_flag= 0;
uint8_t global_bms_soc_level = 0;
uint8_t global_main_bms_soc_level = 0;
uint8_t global_ex_bms_soc_level = 0;
int8_t  global_main_bms_soc_level_buf=-1;
int8_t  global_ex_bms_soc_level_buf=-1;
int8_t  global_charging_bms_soc_level_buf=-1;
int8_t  global_lock_bms_soc_level_buf=-1;
uint8_t global_assist_level = 0;
uint8_t global_page_index = 0;
uint8_t global_motor_odo_ten_thousands_digit = 0;
uint8_t global_motor_odo_thousands_digit = 0;
uint8_t global_motor_odo_hundreds_digit = 0;
uint8_t global_motor_odo_tens_digit = 0;
uint8_t global_motor_odo_units_digit = 0;
int8_t global_motor_odo_ten_thousands_digit_buf =-1;
int8_t global_motor_odo_thousands_digit_buf=-1;
int8_t global_motor_odo_hundreds_digit_buf=-1;
int8_t global_motor_odo_tens_digit_buf=-1;
int8_t global_motor_odo_units_digit_buf=-1;
uint8_t	global_motor_trip_hundred_digit=0;
uint8_t	global_motor_trip_tens_digit=0;
uint8_t	global_motor_trip_units_digit=0;
int8_t	global_motor_trip_hundred_digit_buf=-1;
int8_t	global_motor_trip_tens_digit_buf=-1;
int8_t	global_motor_trip_units_digit_buf=-1;
bool    global_enter_charging=false;
bool    global_enter_power_off_charging=false;
bool    global_enter_low_battery=false;
bool    global_enter_walking=false;
bool    global_walking_mode=false;
bool    global_enter_lock=false;
bool    global_enter_error=false;
bool    global_enter_status_pics=false;
bool    flag_global_up_button_press=false;
uint8_t global_battery_number=1;
int8_t  global_battery_number_buf=-1;
int8_t	global_time_hour_tens_digit_buf=-1;
int8_t	global_time_hour_units_digit_buf=-1;
int8_t	global_time_min_tens_digit_buf=-1;
int8_t	global_time_min_units_digit_buf=-1;
int8_t  global_pair_bms_soc_level_buf=-1;
int8_t   global_error_bms_soc_level_buf=-1;
bool    global_enter_pair=false;
bool    show_time_flag = false;
int8_t  global_reconnect_bms_soc_level_buf=-1;
bool    global_enter_reconnect=false;
uint64_t up_key_press_timer_2000ms = 0;
uint64_t down_key_press_timer_2000ms = 0;
uint64_t light_press_timer_2000ms = 0;
uint64_t power_key_press_timer_2000ms = 0;
uint64_t power_key_press_timer_4000ms = 0;
uint64_t scroll_key_press_timer_4000ms = 0;
uint64_t low_battery_count = 0;
bool    flag_walk_mode_activate=false;
bool    flag_global_charging_activate=false;
bool    flag_clear_trip_activate=false;
bool    flag_global_down_button_press=false;
bool    flag_global_light_button_release=false;
bool    flag_off_logo_activate=false;
bool    flag_global_power_button_press=false;
bool    flag_global_scroll_button_press=false;
bool    flag_global_light_button_press=false;
bool    flag_global_reopen=false;
bool    flag_global_ota_finish=false;
bool    flag_show_off_logo=false;
bool    flag_button_lock=false;
int8_t  main_global_page_index_buf=-1;
int8_t  sub01_global_page_index_buf = -1;
int8_t  sub02_global_page_index_buf = -1;
int8_t  battery_status_global_page_index_buf = -1;
int8_t  main_global_motor_assist_level_buf = -1;
int8_t  sub01_global_motor_assist_level_buf = -1;
int8_t  sub02_global_motor_assist_level_buf = -1;
int8_t  battery_status_global_motor_assist_level_buf = -1;
uint8_t global_bms_soc_new=0;
int8_t  global_bms_soc_buf=-1;
uint8_t global_motor_est_range=0;
int8_t  global_motor_est_range_buf=-1;
uint32_t global_motor_trip=0;
int32_t  global_motor_trip_buf=-1;
uint8_t global_main_bms_soc=0;
int8_t  global_main_bms_soc_buf=-1;
uint8_t global_ex_bms_soc=0;
int8_t  global_ex_bms_soc_buf=-1;
int8_t  global_charging_bms_soc_buf=-1;
int8_t  global_power_off_bms_soc_buf=-1;
uint8_t global_unit_switch=0;
int8_t  global_unit_switch_buf=-1;
bool    flag_global_combo_press=false;
uint8_t button_press_count=0;
uint8_t global_distance_unit=0;
int8_t global_main_distance_unit_buf=-1;
int8_t global_sub01_distance_unit_buf=-1;
int8_t global_sub02_distance_unit_buf=-1;

/*error*/
uint8_t flag_global_system_motro_error_general_done = 0;
uint8_t flag_global_system_motro_error_general_activate = 0;
uint8_t flag_global_system_motro_error_general_fw_err_done = 0;
uint8_t flag_global_system_motro_error_general_fw_err_activate = 0;
uint8_t flag_global_system_motor_error_activate = 0;
uint8_t flag_global_system_motor_can_lose_activate = 0;

uint8_t flag_global_system_main_bms_error_activate = 0;
uint8_t flag_global_system_main_bms_can_lose_activate =0;

uint8_t flag_global_system_ex_bms_error_activate = 0;
uint8_t flag_hmi_error = 0;

/*power_saving*/
uint8_t flag_global_power_saving_activate = 0; 
uint64_t power_saving_timer_10min = 0;

bool    global_enter_bms_error=false;
bool    global_enter_pair_mode=false;
bool    global_enter_motor_error=false;
bool    global_enter_error_flag=false; 
bool    extra_battery_connected=false;
bool    ten_meter_alert_flag=false;
bool    fifty_meter_lert_flag=false;
bool    ten_meter_recover_flag=false;
bool    fifty_meter_recover_flag=false;			
	 

/*ble*/
uint8_t global_dispaly_ble_paired_digit = 0;
int8_t global_dispaly_ble_paired_digit_buf = -1;
uint8_t global_dispaly_ble_paired_title_digit = 0;
int8_t global_dispaly_ble_paired_title_digit_buf = -1;
uint8_t global_dispaly_ble_code_title_digit = 0;
int8_t global_dispaly_ble_code_title_digit_buf = -1;
uint8_t global_dispaly_ble_passkey_code1_digit = 0;
int8_t global_dispaly_ble_passkey_code1_digit_buf = -1;
uint8_t global_dispaly_ble_passkey_code2_digit = 0;
int8_t global_dispaly_ble_passkey_code2_digit_buf = -1;
uint8_t global_dispaly_ble_passkey_code3_digit = 0;
int8_t global_dispaly_ble_passkey_code3_digit_buf = -1;
uint8_t global_dispaly_ble_passkey_code4_digit = 0;
int8_t global_dispaly_ble_passkey_code4_digit_buf = -1;
uint8_t global_dispaly_ble_passkey_code5_digit = 0;
int8_t global_dispaly_ble_passkey_code5_digit_buf = -1;
uint8_t global_dispaly_ble_passkey_code6_digit = 0;
int8_t global_dispaly_ble_passkey_code6_digit_buf = -1;
bool flag_global_display_ble_page_counter = 0;
bool flag_global_display_ble_page_showed = 0;

/*ftm*/
bool global_enter_ftm=false;

/*ble jump*/
bool flag_ble_page_jump = 0;
bool flag_time_start = 0;

/*ble jump*/
uint8_t flag_switch_speed_units = 0;

/*FW_UPDATE*/
bool    flag_do_mc_fw_update=false;

/*Bypass_Mode*/
bool    flag_enter_bypass_mode=false;

/*USB_Plug*/
uint8_t flag_usb_plugged = 0;
int8_t flag_usb_plugged_buf = -1;

//Factory test item use status by Miller
int g_ifactory_state = -1;  //-1:INIT FTM Mode 0:FTM Mode 1:Test Mode 2:Release Mode 3:keypad Mode 4:diag mode 5:system test mode 6:Penetration Protection On 7:switch to user mode
char g_chAllsystem_PID[16];
char g_chBT_MAC_address[16];
char g_chPCBA_PID[16];
char BLE_DEVICE_NAME[9];
char uiid_temp[30];
char motor_fw_version[8] __attribute__ ((aligned (4)));
char CAN_SEND_DATA[8] __attribute__ ((aligned (4)));
char hmi_serial_number[16] __attribute__ ((aligned (4)));
char passkey_number[8] __attribute__ ((aligned (4)));
//char hmi_serial_number[16];

//Aaron NPI USER mode switch
int switch_flag = 0;
int lcm_test_flag = 0;
int lcm_test_last = 0;

int ifactor_state_arr[1]; //-1:INIT FTM Mode 0:FTM Mode 1:Test Mode 2:Release Mode 3:keypad Mode 4:diag mode 5:system test mode 6:Penetration Protection On 7:switch to user mode
int watchdog_flag_arr[1]; // 0: OFF 1: ON
int uiid_len[1];

//Factory test item use status by Ken
uint8_t  g_ifactory_ble_conn=0;
uint8_t  g_ifactory_ble_report_step=0;        //step=0 is non-work, step=1 is read, step=2 is report
int8_t   g_ifactory_rssi=0;
int8_t   g_ifactory_txpower=0xFF;

//ble states by Ken
ble_peripheral_state_t  g_ble_peripheral_state;

//BLE states by YUN
bool flag_throttle_status       = 0;
bool flag_PINcode_WRONG         = 0;//1:PINcode != HMIcode

uint32_t total_seconds          = 0;


uint32_t global_motor_error_code      = 0;
uint16_t global_battery_error_code    = 0;
uint8_t  global_CAN_time_error_code    = 0;
uint8_t  global_time_min              = 0;
uint16_t  global_bms_cycle_count       = 0;

uint8_t MC_MCOD                 = 0;
uint16_t MC_YEAR                = 0;   
uint16_t MC_MONTH               = 0;
uint16_t MC_DAY                 = 0;
uint8_t MC_SN_H[8]              = {0};
uint8_t MC_SN_L[8]              = {0};
uint8_t MC_MODEL                = 0;
uint8_t MC_FW_M_V_H             = 0; 
uint8_t MC_FW_M_V_L             = 0;
uint8_t MC_FW_S_V_H             = 0;  
uint8_t MC_FW_S_V_L             = 0; 
uint8_t MC_HW_Type_H            = 0;
uint8_t MC_HW_Type_L            = 0;

uint8_t  BMS_MCOD                 = 0;
uint8_t  BMS_YEAR                 = 0;
uint8_t  BMS_MONTH                = 0;
uint8_t  BMS_DAY                  = 0;
uint32_t BMS_SSERIAL_NUMBER       = 0; 
uint8_t  BMS_MODEL                = 0; 
uint16_t BMS_FW_MAIN_VERSION      = 0; 
uint16_t BMS_FW_SECOND_VERSION    = 0; 
uint16_t BMS_FW_DEBUG_SERIAL      = 0; 

//BLE states by YUN