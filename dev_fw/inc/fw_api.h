#ifndef FW_API_H
#define FW_API_H

/////////////////////////////////
// dev_init.h
/////////////////////////////////
//---------modification list for Ken---------
#include <bluetooth/addr.h>
#include "hal_api.h"

/**
* @fn dev_check_is_output(int pin_val)
* @brief to check pin is output by pin number
* @param pin_val PIN Number ( 0 ~ 47 )
* @details When processing the GPIO command issued by the user, the program will first need to determine whether the GPIO is an input or output before determining whether to execute the GPIO action.
* @return output PIN = true ; input PIN = false
*/
bool dev_check_is_output(int pin_val);

/**
* @fn dev_check_is_input(int pin_val)
* @brief to check pin is input by pin number
* @param pin_val PIN Number ( 0 ~ 47 )
* @details When processing the GPIO command issued by the user, the program will first need to determine whether the GPIO is an input or output before determining whether to execute the GPIO action.
* @return input PIN = true ; output PIN = false
*/
bool dev_check_is_input(int pin_val);


////////////////////////////////
//  Virtual COM
////////////////////////////////

/**
* @fn fw_init_usb_virtual_COM()
* @brief USB virtual COM initial function.
* To get device tree handle. Then enable USB device, initial buffer, setting transfer callback and config request setting.
*/
int fw_init_usb_virtual_COM();

/**
* @fn fw_uninit_usb_virtual_COM()
* @brief USB virtual COM uninitial function.
* Release configuration settings. Disable USB device (such disable VBUS).
*/
int fw_uninit_usb_virtual_COM();

/**
* @fn fw_read_USB_data(unsigned char* pCmd, int len)
* @brief Read USB data to array buffer.
*/
int fw_read_USB_data(unsigned char* pCmd, int len);

/**
* @fn fw_write_USB_data(unsigned char* pRes, int len)
* @brief Write USB data by array buffer.
*/
void fw_write_USB_data(unsigned char* pRes, int len);


char fw_is_usb_connected(void);


////////////////////////////////
//  Bluetooth
////////////////////////////////

//---------modification list for Ken---------
int fw_ble_centeral_init(bool peripheral_use, bt_addr_le_t  *scan_mac_addr);

int fw_ble_peripheral_init(void);

struct bt_conn *fw_get_current_connect();


//---------modification list for Ken---------
void fw_ble_peripheral_write(unsigned char *pBuf,int len);
int fw_ble_peripheral_read(unsigned char *pBuf,int len);
char fw_is_ble_peripheral_connected(void);
unsigned int fw_ble_get_peripheral_passkey(void);
//unsigned char fw_ble_get_peripheral_state(void);
void fw_ble_turn_on_peripheral(bool onoff);

bool fw_is_bluetooth_connected(void);

void fw_ble_central_write(unsigned char *pBuf,int len);
int fw_ble_central_read(unsigned char *pBuf,int len);
char fw_is_ble_central_connected(uint16_t  *currnet_handle);

struct bt_conn *fw_get_current_connect();

//---------modification list for Ken---------
int read_conn_rssi(uint16_t handle, int8_t *rssi);
int read_tx_power(uint8_t handle_type, uint16_t handle, int8_t *tx_pwr_lvl);
////////////////////////////////
//  Delay
////////////////////////////////

/**
* @fn delay_msec(short value)
* @brief delsy mini-second
*
*/
void delay_msec(short value);

/**
* @fn delay_usec(short value)
* @brief delsy micro-second
*
*/
void delay_usec(short value);

///////////////////////////////
// IO PIN
///////////////////////////////

int fw_init_adc(void);

int get_thermal_adc();
int get_remote1_adc();
int get_remote2_adc();
int get_usb_cc_adc();



int fw_init_pwm(void);
void fw_pwm_set(uint16_t value);




////////////////////////////////
//  GPIO
////////////////////////////////

/**
* @fn fw_init_gpio()
* @brief Initialize gpio control. Get GPIO handle from device tree.
*/
void fw_init_gpio();

/**
* @fn fw_config_gpio( short pinNo , int flag )
* @brief Config GPIO pin. The setting is following property of drivers/gpio.h file.
* @param pinNo
* @param flag
* @return 
*/
int fw_config_gpio( short pinNo , int flag );

/**
* @fn fw_set_gpio_pin( short pinNo , int flag )
* @brief Set GPIO pin value. The setting is following property of drivers/gpio.h file.
* @param pinNo
* @param flag the parameter is following :
* <ul>
* <li> GPIO_INPUT
*  <ol>
*   <li> GPIO_OPEN_DRAIN         (GPIO_SINGLE_ENDED | GPIO_LINE_OPEN_DRAIN)
*   <li> GPIO_OPEN_SOURCE        (GPIO_SINGLE_ENDED | GPIO_LINE_OPEN_SOURCE)
*   <li> GPIO_PULL_UP
*   <li> GPIO_PULL_DOWN
* </ol>
* <li> GPIO_OUTPUT 
*   <li> GPIO_OUTPUT_LOW         (GPIO_OUTPUT | GPIO_OUTPUT_INIT_LOW)
*   <li> GPIO_OUTPUT_HIGH        (GPIO_OUTPUT | GPIO_OUTPUT_INIT_HIGH)
*   <li> GPIO_OUTPUT_INACTIVE    (GPIO_OUTPUT | GPIO_OUTPUT_INIT_LOW | GPIO_OUTPUT_INIT_LOGICAL)
*   <li> GPIO_OUTPUT_ACTIVE      (GPIO_OUTPUT | GPIO_OUTPUT_INIT_HIGH | GPIO_OUTPUT_INIT_LOGICAL)
* </ul>
* @return 0 is success
*/
int fw_set_gpio_pin( short pinNo , int flag );

int fw_get_gpio_pin( short pinNo );


/**
* @brief Define the callback function pointer.
*/
typedef void (*gpio_fun_callback)( const struct device *port, struct gpio_callback *cb, uint32_t pins );

/**
* @fn fw_setup_gpio_callback(int pinNo, gpio_fun_callback p_gfc)
*
*/
int fw_setup_gpio_callback(int pinNo, gpio_fun_callback p_gfc);

void fw_init_wdt(void);



int get_usb_cc1_adc();

int get_usb_cc2_adc();

int get_usb_cc_adc();

struct PW_KEY find_key();

void store_key(long address, uint8_t value[4]);

void erase_password();

////////////////////////////////////////
//   CAN
////////////////////////////////////////

//�w�q�O�_�}��      CAN Dubug mode

#define CAN_PAGE_DBG(x) do { \
    if (ifactor_state_arr[0] == 7) { \
        FIT_CANFD_PROCESS_DBG_MESSAGE_Send((x)); \
    } \
} while (0)

#define CAN_BT_DBG(x, y) do { \
    if (ifactor_state_arr[0] == 7) { \
        FIT_CANFD_PROCESS_DBG_BTN_Send((x), (y)); \
    } \
} while (0)

/**
* @fn fw_init_mcp251863
* @brief Initialize CANBus <br>
* Specification of relevant function specifications required for use with the mcp251863 IC; and
*/
int fw_init_mcp251863(void);

//int fw_normal_spi_init(void *pIntDev, uint8_t chipSelect, const uint32_t sckFreq);

void fw_normal_spi_init(void);
//int fw_normal_spi_trans( unsigned char DEV_NO , uint8_t chipSelect, unsigned char *wbuf , int wlen , unsigned char *rbuf , int rlen );

//int fw_normal_spi_trans( void *pIntDev , uint8_t chipSelect, unsigned char *wbuf , int wlen , unsigned char *rbuf , int rlen );
int fw_normal_spi_trans(  unsigned char *wbuf , int wlen , unsigned char *rbuf , int rlen );


////////////////////////////////////////
//   Flash ROM
////////////////////////////////////////


/**
* @fn fw_init_qspi_flash_rom
* @brief Flash ROM initialization
*/
bool fw_init_qspi_flash_rom(void);
int fw_flash_erase( long addr, int page );
int fw_flash_read(long addr, int size ,char *p_dat);
int fw_flash_write(long addr, int size ,char *p_dat);
int fw_flash_read_by_decode(long addr, int size ,char *p_dat);
int fw_flash_write_with_encode(long addr, int size ,char *p_dat);


struct PW_KEY {
    uint16_t save_number;
    uint8_t pw_value[4];
};


void ble_connect_page(void);

////////////////////////////////////////
//   LCD Module
////////////////////////////////////////

/**
* @fn fw_st7789v2_init
* @brief LCD module initialization
*/
bool fw_st7789v2_init(void);

bool fw_is_usb_dfu(void);

/*boot*/
extern uint8_t flag_global_system_boot_on;

/*locked*/
extern uint8_t  flag_global_locked_activate;

extern uint8_t global_motor_current_speed;
extern uint8_t global_motor_total_trip;
extern uint8_t global_motor_total_total;
extern uint8_t global_motor_assist_level;
extern uint8_t global_motor_system_status; 
extern uint8_t global_erase_flag;
extern uint8_t global_pic_error_flag;
extern uint8_t global_ebike_light;
extern int8_t  global_ebike_light_buf;
extern uint8_t global_ebike_bluetooth;
extern int8_t  global_ebike_bluetooth_buf;
extern int8_t  global_motor_sub01_assist_level_buf;
extern int8_t  global_motor_sub02_assist_level_buf;
extern uint8_t global_counter_flag;
extern uint8_t global_motor_current_speed_tens_digit;
extern uint8_t global_motor_current_speed_units_digit;
extern int8_t  global_motor_current_speed_tens_digit_buf;
extern int8_t  global_motor_current_speed_units_digit_buf;
extern uint8_t global_time_hour_tens_digit;
extern uint8_t global_time_hour_units_digit;
extern uint8_t global_time_min_tens_digit;
extern uint8_t global_time_min_units_digit;
extern uint8_t	global_motor_est_range_houndred_digit;
extern uint8_t global_motor_est_range_tens_digit;
extern uint8_t global_motor_est_range_units_digit;
extern int8_t  global_motor_est_range_houndred_digit_buf;
extern int8_t  global_motor_est_range_tens_digit_buf;
extern int8_t  global_motor_est_range_units_digit_buf;
extern uint8_t global_motor_odo_ten_thousands_digit;
extern uint8_t global_motor_odo_thousands_digit;
extern uint8_t global_motor_odo_hundreds_digit;
extern uint8_t global_motor_odo_tens_digit;
extern uint8_t global_motor_odo_units_digit;
extern int8_t global_motor_odo_ten_thousands_digit_buf;
extern int8_t global_motor_odo_thousands_digit_buf;
extern int8_t global_motor_odo_hundreds_digit_buf;
extern int8_t global_motor_odo_tens_digit_buf;
extern int8_t global_motor_odo_units_digit_buf;
extern uint8_t	global_motor_trip_hundred_digit;
extern uint8_t	global_motor_trip_tens_digit;
extern uint8_t	global_motor_trip_units_digit;
extern int8_t	global_motor_trip_hundred_digit_buf;
extern int8_t	global_motor_trip_tens_digit_buf;
extern int8_t	global_motor_trip_units_digit_buf;
extern uint8_t global_bms_soc_hundreds_digit;
extern uint8_t global_bms_soc_tens_digit;
extern uint8_t global_bms_soc_units_digit;
extern uint8_t global_bms_soc_level;
extern int8_t  global_charging_bms_soc_level_buf;
extern int8_t  global_lock_bms_soc_level_buf;
extern uint8_t global_assist_level;
extern uint8_t global_page_index;
extern bool    global_enter_charging;
extern bool    global_enter_power_off_charging;
extern bool    global_walking_mode;
extern bool    global_enter_walking;
extern bool    global_enter_lock;
extern uint8_t global_battery_number;
extern int8_t  global_battery_number_buf;
extern int8_t	global_time_hour_tens_digit_buf;
extern int8_t	global_time_hour_units_digit_buf;
extern int8_t	global_time_min_tens_digit_buf;
extern int8_t	global_time_min_units_digit_buf;
extern int8_t   global_main_bms_soc_level_buf;
extern int8_t   global_ex_bms_soc_level_buf;
extern int8_t   global_error_bms_soc_level_buf;
extern bool     global_enter_error;
extern int8_t  global_pair_bms_soc_level_buf;
extern bool    global_enter_pair;
extern int8_t  global_reconnect_bms_soc_level_buf;
extern bool    global_enter_reconnect;
extern bool    flag_global_up_button_press;
extern uint64_t up_key_press_timer_2000ms;
extern uint64_t down_key_press_timer_2000ms;
extern uint64_t light_press_timer_2000ms;
extern bool    flag_walk_mode_activate;
extern bool    flag_global_down_button_press;
extern bool    flag_global_light_button_release;
extern bool    flag_off_logo_activate;
extern bool    flag_global_power_button_press;
extern bool    flag_global_light_button_press;
extern uint64_t power_key_press_timer_2000ms;
extern uint64_t power_key_press_timer_4000ms;
extern uint64_t scroll_key_press_timer_4000ms;
extern uint64_t low_battery_count;
extern bool    global_enter_low_battery;
extern bool    flag_global_scroll_button_press;
extern bool    flag_clear_trip_activate;
extern bool    global_enter_status_pics;
extern bool    flag_global_charging_activate;
extern int8_t  main_global_page_index_buf;
extern int8_t  sub01_global_page_index_buf;
extern int8_t  sub02_global_page_index_buf;
extern int8_t  battery_status_global_page_index_buf;
extern int8_t  main_global_motor_assist_level_buf;
extern int8_t  sub01_global_motor_assist_level_buf;
extern int8_t  sub02_global_motor_assist_level_buf;
extern int8_t  battery_status_global_motor_assist_level_buf;
extern uint8_t global_bms_soc_new;
extern int8_t  global_bms_soc_buf;
extern uint8_t global_motor_est_range;
extern int8_t  global_motor_est_range_buf;
extern uint32_t global_motor_trip;
extern int32_t  global_motor_trip_buf;
extern int8_t  global_charging_bms_soc_buf;
extern int8_t  global_power_off_bms_soc_buf;
extern bool    flag_button_lock;
extern bool    show_time_flag;

/*BMS*/
extern uint8_t global_main_bms_soc_hundreds_digit;
extern uint8_t global_main_bms_soc_tens_digit;
extern uint8_t global_main_bms_soc_units_digit;
extern uint8_t global_ex_bms_soc_hundreds_digit;
extern uint8_t global_ex_bms_soc_tens_digit;
extern uint8_t global_ex_bms_soc_units_digit;
extern uint8_t global_main_bms_soc_level;
extern uint8_t global_ex_bms_soc_level;
extern uint8_t global_main_bms_soc;
extern int8_t  global_main_bms_soc_buf;
extern uint8_t global_ex_bms_soc;
extern int8_t  global_ex_bms_soc_buf;

/*error*/
extern uint8_t flag_global_system_motro_error_general_done;
extern uint8_t flag_global_system_motro_error_general_activate;
extern uint8_t flag_global_system_motro_error_general_fw_err_done;
extern uint8_t flag_global_system_motro_error_general_fw_err_activate;
extern uint8_t flag_global_system_motor_error_activate;
extern uint8_t flag_global_system_motor_can_lose_activate;

extern uint8_t flag_global_system_main_bms_error_activate;
extern uint8_t flag_global_system_main_bms_can_lose_activate;

extern uint8_t flag_global_system_ex_bms_error_activate;
extern uint8_t flag_hmi_error;

extern bool    global_enter_bms_error;
extern bool    global_enter_motor_error;
extern bool    global_enter_error_flag;
extern bool    global_enter_pair_mode;
extern bool    flag_global_reopen;
extern bool    flag_global_ota_finish;
extern bool    ten_meter_alert_flag;
extern bool    fifty_meter_lert_flag;
extern bool    ten_meter_recover_flag;
extern bool    fifty_meter_recover_flag;				
extern uint8_t global_unit_switch;
extern int8_t  global_unit_switch_buf;
extern bool    flag_global_combo_press;
extern bool    flag_show_off_logo;
extern bool    extra_battery_connected;	   
extern uint8_t button_press_count;
extern uint8_t global_distance_unit;
extern int8_t global_main_distance_unit_buf;
extern int8_t global_sub01_distance_unit_buf;
extern int8_t global_sub02_distance_unit_buf;
/*boot*/
extern uint8_t flag_global_system_boot_on;
/*locked*/
extern uint8_t  flag_global_locked_activate;
extern uint8_t	global_locked_title_digit;
extern int8_t	global_locked_title_digit_buf;
extern uint8_t	global_locked_digit;
extern int8_t	global_locked_digit_buf;

/*power_saving*/
extern uint8_t flag_global_power_saving_activate; 
extern uint64_t power_saving_timer_10min;

/*ble*/
extern uint8_t global_dispaly_ble_paired_digit;
extern int8_t global_dispaly_ble_paired_digit_buf;
extern uint8_t global_dispaly_ble_paired_title_digit;
extern int8_t global_dispaly_ble_paired_title_digit_buf;
extern uint8_t global_dispaly_ble_code_title_digit;
extern int8_t global_dispaly_ble_code_title_digit_buf;
extern uint8_t global_dispaly_ble_passkey_code1_digit;
extern int8_t global_dispaly_ble_passkey_code1_digit_buf;
extern uint8_t global_dispaly_ble_passkey_code2_digit;
extern int8_t global_dispaly_ble_passkey_code2_digit_buf;
extern uint8_t global_dispaly_ble_passkey_code3_digit;
extern int8_t global_dispaly_ble_passkey_code3_digit_buf;
extern uint8_t global_dispaly_ble_passkey_code4_digit;
extern int8_t global_dispaly_ble_passkey_code4_digit_buf;
extern uint8_t global_dispaly_ble_passkey_code5_digit;
extern int8_t global_dispaly_ble_passkey_code5_digit_buf;
extern uint8_t global_dispaly_ble_passkey_code6_digit;
extern int8_t global_dispaly_ble_passkey_code6_digit_buf;
extern bool flag_global_display_ble_page_counter;
extern bool flag_global_display_ble_page_showed;
extern bool flag_request_pairing;
extern int normal_delay_count;
/*ftm*/
extern bool global_enter_ftm;

/*bypass*/
extern uint8_t global_dispaly_bypass_icon_digit;
extern int8_t global_dispaly_bypass_icon_digit_buf;
extern uint8_t global_dispaly_bypass_title_digit;
extern int8_t global_dispaly_bypass_title_digit_buf;

/*ble jump*/
extern bool flag_ble_page_jump;
extern bool flag_time_start; 

/*ble jump*/
extern uint8_t flag_switch_speed_units;

/*FW_UPDATE*/
extern bool    flag_do_mc_fw_update;

/*Bypass_Mode*/
extern bool    flag_enter_bypass_mode;

/*USB_Plug*/
/*USB_Plug*/
extern uint8_t flag_usb_plugged;
extern int8_t flag_usb_plugged_buf;

bool fw_is_usb_dfu(void);
extern int motor_current_speed;
extern int motor_total_trip;
extern int motor_total_total;
extern int motor_assist_level;
extern int motor_system_status; 
extern int erase_flag;
#define black 0x0000
#define white 0xFFFF
#define red   0x00F8
#define green 0xE007
#define blue  0x1F00
#define gray  0X1084
#define yellow 0XE0FF

#define BG_BATTERY_STATUS_L0_BLOCK	8200
#define BG_BATTERY_STATUS_L1_BLOCK	133644
#define BG_BATTERY_STATUS_L2_BLOCK	259088
#define BG_BATTERY_STATUS_L3_BLOCK	384532
#define BG_BATTERY_STATUS_L4_BLOCK	509976
#define BG_CHARGING_LEFT_ON_BLOCK	635420
#define BG_CHARGING_RIGHT_OFF_BLOCK	664224
#define BG_CHARGING_RIGHT_ON_BLOCK	693028
#define BG_MAIN_BATTERY_2_BLOCK	721832
#define BG_MAIN_L0_3_BLOCK	727116
#define BG_MAIN_L1_3_BLOCK	852560
#define BG_MAIN_L2_3_BLOCK	978004
#define BG_MAIN_L3_3_BLOCK	1103448
#define BG_MAIN_L4_3_BLOCK	1228892
#define BG_MAIN_SUB_L0123_1_BLOCK	1354336
#define BG_MAIN_SUB_LEVEL4_1_BLOCK	1359620
#define BG_MAINPAGE_BLOCK	1364904
#define BG_MAINPAGE_LEVEL_0_BLOCK	1443308
#define BG_MAINPAGE_LEVEL_1_BLOCK	1472112
#define BG_MAINPAGE_LEVEL_2_BLOCK	1500916
#define BG_MAINPAGE_LEVEL_3_BLOCK	1529720
#define BG_MAINPAGE_LEVEL_4_BLOCK	1558524
#define BG_MAINPAGE_LINE_L0_1_BLOCK	1587328
#define BG_MAINPAGE_LINE_L0_2_BLOCK	1587396
#define BG_MAINPAGE_LINE_L0_3_BLOCK	1587496
#define BG_MAINPAGE_LINE_L0_4_BLOCK	1587756
#define BG_MAINPAGE_LINE_L0_5_BLOCK	1588144
#define BG_MAINPAGE_LINE_L0_6_BLOCK	1592468
#define BG_MAINPAGE_LINE_L0_7_BLOCK	1592856
#define BG_MAINPAGE_LINE_L0_8_BLOCK	1593116
#define BG_MAINPAGE_LINE_L0_9_BLOCK	1593216
#define BG_MAINPAGE_LINE_L1_1_BLOCK	1593284
#define BG_MAINPAGE_LINE_L1_2_BLOCK	1593352
#define BG_MAINPAGE_LINE_L1_3_BLOCK	1593452
#define BG_MAINPAGE_LINE_L1_4_BLOCK	1593712
#define BG_MAINPAGE_LINE_L1_5_BLOCK	1594100
#define BG_MAINPAGE_LINE_L1_6_BLOCK	1598424
#define BG_MAINPAGE_LINE_L1_7_BLOCK	1598812
#define BG_MAINPAGE_LINE_L1_8_BLOCK	1599072
#define BG_MAINPAGE_LINE_L1_9_BLOCK	1599172
#define BG_MAINPAGE_LINE_L2_1_BLOCK	1599240
#define BG_MAINPAGE_LINE_L2_2_BLOCK	1599308
#define BG_MAINPAGE_LINE_L2_3_BLOCK	1599408
#define BG_MAINPAGE_LINE_L2_4_BLOCK	1599668
#define BG_MAINPAGE_LINE_L2_5_BLOCK	1600056
#define BG_MAINPAGE_LINE_L2_6_BLOCK	1604380
#define BG_MAINPAGE_LINE_L2_7_BLOCK	1604768
#define BG_MAINPAGE_LINE_L2_8_BLOCK	1605028
#define BG_MAINPAGE_LINE_L2_9_BLOCK	1605128
#define BG_MAINPAGE_LINE_L3_1_BLOCK	1605196
#define BG_MAINPAGE_LINE_L3_2_BLOCK	1605264
#define BG_MAINPAGE_LINE_L3_3_BLOCK	1605364
#define BG_MAINPAGE_LINE_L3_4_BLOCK	1605624
#define BG_MAINPAGE_LINE_L3_5_BLOCK	1606012
#define BG_MAINPAGE_LINE_L3_6_BLOCK	1610336
#define BG_MAINPAGE_LINE_L3_7_BLOCK	1610724
#define BG_MAINPAGE_LINE_L3_8_BLOCK	1610984
#define BG_MAINPAGE_LINE_L3_9_BLOCK	1611084
#define BG_MAINPAGE_LINE_L4_1_BLOCK	1611152
#define BG_MAINPAGE_LINE_L4_2_BLOCK	1611220
#define BG_MAINPAGE_LINE_L4_3_BLOCK	1611320
#define BG_MAINPAGE_LINE_L4_4_BLOCK	1611580
#define BG_MAINPAGE_LINE_L4_5_BLOCK	1611968
#define BG_MAINPAGE_LINE_L4_6_BLOCK	1616292
#define BG_MAINPAGE_LINE_L4_7_BLOCK	1616680
#define BG_MAINPAGE_LINE_L4_8_BLOCK	1616940
#define BG_MAINPAGE_LINE_L4_9_BLOCK	1617040
#define BG_MAINPAGE_STATUS_BLOCK	1617108
#define BG_POWEROFF_CHARGING_BLOCK	1634712
#define BG_SUB1_L0_3_BLOCK	1788316
#define BG_SUB1_L1_3_BLOCK	1913760
#define BG_SUB1_L2_3_BLOCK	2039204
#define BG_SUB1_L3_3_BLOCK	2164648
#define BG_SUB1_L4_3_BLOCK	2290092
#define BG_SUB2_L0_3_BLOCK	2415536
#define BG_SUB2_L1_3_BLOCK	2540980
#define BG_SUB2_L2_3_BLOCK	2666424
#define BG_SUB2_L3_3_BLOCK	2791868
#define BG_SUB2_L4_3_BLOCK	2917312
#define BG_SUB12_2_BLOCK	3042756
#define BG_SUBPAGE_1_LEVEL_0_BLOCK	3048040
#define BG_SUBPAGE_1_LEVEL_1_BLOCK	3049004
#define BG_SUBPAGE_1_LEVEL_2_BLOCK	3049968
#define BG_SUBPAGE_1_LEVEL_3_BLOCK	3050932
#define BG_SUBPAGE_1_LEVEL_4_BLOCK	3051896
#define BG_SUBPAGE_2_LEVEL_0_BLOCK	3052860
#define BG_SUBPAGE_2_LEVEL_1_BLOCK	3053824
#define BG_SUBPAGE_2_LEVEL_2_BLOCK	3054788
#define BG_SUBPAGE_2_LEVEL_3_BLOCK	3055752
#define BG_SUBPAGE_2_LEVEL_4_BLOCK	3056716
#define BG_SUBPAGE_3_LEVEL_0_BLOCK	3057680
#define BG_SUBPAGE_3_LEVEL_1_BLOCK	3063316
#define BG_SUBPAGE_3_LEVEL_2_BLOCK	3068952
#define BG_SUBPAGE_3_LEVEL_3_BLOCK	3074588
#define BG_SUBPAGE_3_LEVEL_4_BLOCK	3080224
#define BG_SUBPAGE_4_LEVEL_0_BLOCK	3085860
#define BG_SUBPAGE_4_LEVEL_1_BLOCK	3090184
#define BG_SUBPAGE_4_LEVEL_2_BLOCK	3094508
#define BG_SUBPAGE_4_LEVEL_3_BLOCK	3098832
#define BG_SUBPAGE_4_LEVEL_4_BLOCK	3103156
#define BG_SUBPAGE_5_LEVEL_0_BLOCK	3107480
#define BG_SUBPAGE_5_LEVEL_1_BLOCK	3112380
#define BG_SUBPAGE_5_LEVEL_2_BLOCK	3117280
#define BG_SUBPAGE_5_LEVEL_3_BLOCK	3122180
#define BG_SUBPAGE_5_LEVEL_4_BLOCK	3127080
#define BG_SUBPAGE_LEVEL_0_BLOCK	3131980
#define BG_SUBPAGE_LEVEL_1_BLOCK	3156944
#define BG_SUBPAGE_LEVEL_2_BLOCK	3181908
#define BG_SUBPAGE_LEVEL_3_BLOCK	3206872
#define BG_SUBPAGE_LEVEL_4_BLOCK	3231836
#define BG_WLPE_LEFT_BLOCK	3256800
#define BG_WLPE_RIGHT_BLOCK	3285604
#define CHARGING_0_BLOCK	3314408
#define CHARGING_1_BLOCK	3315948
#define CHARGING_2_BLOCK	3317488
#define CHARGING_3_BLOCK	3319028
#define CHARGING_4_BLOCK	3320568
#define CHARGING_5_BLOCK	3322108
#define CHARGING_6_BLOCK	3323648
#define CHARGING_7_BLOCK	3325188
#define CHARGING_8_BLOCK	3326728
#define CHARGING_9_BLOCK	3328268
#define IC_BATTERY_0_BLOCK	3329808
#define IC_BATTERY_1_BLOCK	3335636
#define IC_BATTERY_2_BLOCK	3341464
#define IC_BATTERY_3_BLOCK	3347292
#define IC_BATTERY_4_BLOCK	3353120
#define IC_BATTERY_EMPTY_1_BLOCK	3358948
#define IC_BATTERY_EMPTY_2_1_BLOCK	3361064
#define IC_BATTERY_EMPTY_2_2_BLOCK	3362732
#define IC_BATTERY_EMPTY_2_3_BLOCK	3363504
#define IC_BATTERY_EMPTY_2_4_BLOCK	3364212
#define IC_BATTERY_EMPTY_3_BLOCK	3364792
#define IC_BATTERY_EMPTY_4_BLOCK	3366812
#define IC_BATTERY_EMPTY_5_BLOCK	3368832
#define IC_BATTERY_EMPTY_6_BLOCK	3370852
#define IC_BATTERY_EMPTY_7_1_BLOCK	3372872
#define IC_BATTERY_EMPTY_7_2_BLOCK	3373452
#define IC_BATTERY_EMPTY_7_3_BLOCK	3374160
#define IC_BATTERY_EMPTY_7_4_BLOCK	3374932
#define IC_BATTERY_EMPTY_8_BLOCK	3376600
#define IC_BATTERY_FULL_1_BLOCK	3378716
#define IC_BATTERY_FULL_2_1_BLOCK	3380832
#define IC_BATTERY_FULL_2_2_BLOCK	3382500
#define IC_BATTERY_FULL_2_3_BLOCK	3383272
#define IC_BATTERY_FULL_2_4_BLOCK	3383980
#define IC_BATTERY_FULL_3_BLOCK	3384560
#define IC_BATTERY_FULL_4_BLOCK	3386580
#define IC_BATTERY_FULL_5_BLOCK	3388600
#define IC_BATTERY_FULL_6_BLOCK	3390620
#define IC_BATTERY_FULL_7_1_BLOCK	3392640
#define IC_BATTERY_FULL_7_2_BLOCK	3393220
#define IC_BATTERY_FULL_7_3_BLOCK	3393928
#define IC_BATTERY_FULL_7_4_BLOCK	3394700
#define IC_BATTERY_FULL_8_BLOCK	3396368
#define IC_BATTERY_TITLE_L0_BLOCK	3398484
#define IC_BATTERY_TITLE_L1_BLOCK	3405208
#define IC_BATTERY_TITLE_L2_BLOCK	3411932
#define IC_BATTERY_TITLE_L3_BLOCK	3418656
#define IC_BATTERY_TITLE_L4_BLOCK	3425380
#define IC_BATTERY1_NAME_BLOCK	3432104
#define IC_BATTERY2_NAME_BLOCK	3435020
#define IC_BLUETOOTH_BLOCK	3437936
#define IC_BLUETOOTH_DISABLE_BLOCK	3439476
#define IC_CHARGING_0_BLOCK	3441016
#define IC_CHARGING_1_BLOCK	3455996
#define IC_CHARGING_2_BLOCK	3470976
#define IC_CHARGING_3_BLOCK	3485956
#define IC_CHARGING_4_BLOCK	3500936
#define IC_CHARGING_5_BLOCK	3515916
#define IC_CHARGING_6_BLOCK	3530896
#define IC_CHARGING_7_BLOCK	3545876
#define IC_CHARGING_8_BLOCK	3560856
#define IC_CHARGING_LEFT_1_BLOCK	3575836
#define IC_CHARGING_LEFT_2A_BLOCK	3579200
#define IC_CHARGING_LEFT_2B_BLOCK	3580452
#define IC_CHARGING_LEFT_2C_BLOCK	3580840
#define IC_CHARGING_LEFT_2D_BLOCK	3581196
#define IC_CHARGING_LEFT_2E_BLOCK	3581520
#define IC_CHARGING_LEFT_3_BLOCK	3581812
#define IC_CHARGING_LEFT_4_BLOCK	3583832
#define IC_CHARGING_LEFT_5_BLOCK	3585852
#define IC_CHARGING_LEFT_6_BLOCK	3587872
#define IC_CHARGING_LEFT_7A_BLOCK	3589892
#define IC_CHARGING_LEFT_7B_BLOCK	3590184
#define IC_CHARGING_LEFT_7C_BLOCK	3590508
#define IC_CHARGING_LEFT_7D_BLOCK	3590864
#define IC_CHARGING_LEFT_7E_BLOCK	3591252
#define IC_CHARGING_LEFT_8_BLOCK	3592504
#define IC_CHARGING_LEFT_EMPTY_1_BLOCK	3595868
#define IC_CHARGING_LEFT_EMPTY_2A_BLOCK	3599232
#define IC_CHARGING_LEFT_EMPTY_2B_BLOCK	3600484
#define IC_CHARGING_LEFT_EMPTY_2C_BLOCK	3600872
#define IC_CHARGING_LEFT_EMPTY_2D_BLOCK	3601228
#define IC_CHARGING_LEFT_EMPTY_2E_BLOCK	3601552
#define IC_CHARGING_LEFT_EMPTY_3_BLOCK	3601844
#define IC_CHARGING_LEFT_EMPTY_4_BLOCK	3603864
#define IC_CHARGING_LEFT_EMPTY_5_BLOCK	3605884
#define IC_CHARGING_LEFT_EMPTY_6_BLOCK	3607904
#define IC_CHARGING_LEFT_EMPTY_7A_BLOCK	3609924
#define IC_CHARGING_LEFT_EMPTY_7B_BLOCK	3610216
#define IC_CHARGING_LEFT_EMPTY_7C_BLOCK	3610540
#define IC_CHARGING_LEFT_EMPTY_7D_BLOCK	3610896
#define IC_CHARGING_LEFT_EMPTY_7E_BLOCK	3611284
#define IC_CHARGING_LEFT_EMPTY_8_BLOCK	3612536
#define IC_CHARGING_RIGHT_1_BLOCK	3615900
#define IC_CHARGING_RIGHT_2A_BLOCK	3619264
#define IC_CHARGING_RIGHT_2B_BLOCK	3620516
#define IC_CHARGING_RIGHT_2C_BLOCK	3620904
#define IC_CHARGING_RIGHT_2D_BLOCK	3621260
#define IC_CHARGING_RIGHT_2E_BLOCK	3621584
#define IC_CHARGING_RIGHT_3_BLOCK	3621876
#define IC_CHARGING_RIGHT_4_BLOCK	3623896
#define IC_CHARGING_RIGHT_5_BLOCK	3625916
#define IC_CHARGING_RIGHT_6_BLOCK	3627936
#define IC_CHARGING_RIGHT_7A_BLOCK	3629956
#define IC_CHARGING_RIGHT_7B_BLOCK	3630248
#define IC_CHARGING_RIGHT_7C_BLOCK	3630572
#define IC_CHARGING_RIGHT_7D_BLOCK	3630928
#define IC_CHARGING_RIGHT_7E_BLOCK	3631316
#define IC_CHARGING_RIGHT_8_BLOCK	3632568
#define IC_CHARGING_RIGHT_EMPTY_1_BLOCK	3635932
#define IC_CHARGING_RIGHT_EMPTY_2A_BLOCK	3639296
#define IC_CHARGING_RIGHT_EMPTY_2B_BLOCK	3640548
#define IC_CHARGING_RIGHT_EMPTY_2C_BLOCK	3640936
#define IC_CHARGING_RIGHT_EMPTY_2D_BLOCK	3641292
#define IC_CHARGING_RIGHT_EMPTY_2E_BLOCK	3641616
#define IC_CHARGING_RIGHT_EMPTY_3_BLOCK	3641908
#define IC_CHARGING_RIGHT_EMPTY_4_BLOCK	3643928
#define IC_CHARGING_RIGHT_EMPTY_5_BLOCK	3645948
#define IC_CHARGING_RIGHT_EMPTY_6_BLOCK	3647968
#define IC_CHARGING_RIGHT_EMPTY_7A_BLOCK	3649988
#define IC_CHARGING_RIGHT_EMPTY_7B_BLOCK	3650280
#define IC_CHARGING_RIGHT_EMPTY_7C_BLOCK	3650604
#define IC_CHARGING_RIGHT_EMPTY_7D_BLOCK	3650960
#define IC_CHARGING_RIGHT_EMPTY_7E_BLOCK	3651348
#define IC_CHARGING_RIGHT_EMPTY_8_BLOCK	3652600
#define IC_ERR_0_BLOCK	3655964
#define IC_ERR_1_BLOCK	3657984
#define IC_ERR_2_BLOCK	3660004
#define IC_ERR_3_BLOCK	3662024
#define IC_ERR_4_BLOCK	3664044
#define IC_ERR_5_BLOCK	3666064
#define IC_ERR_6_BLOCK	3668084
#define IC_ERR_7_BLOCK	3670104
#define IC_ERR_8_BLOCK	3672124
#define IC_ERR_9_BLOCK	3674144
#define IC_ERROR_MASK_BLOCK	3676164
#define IC_ERRORCODE_B_BLOCK	3692168
#define IC_ERRORCODE_C_BLOCK	3694476
#define IC_ERRORCODE_E_BLOCK	3696784
#define IC_EST_RANGE_0_BLOCK	3699092
#define IC_EST_RANGE_1_BLOCK	3706264
#define IC_EST_RANGE_2_BLOCK	3713436
#define IC_EST_RANGE_3_BLOCK	3720608
#define IC_EST_RANGE_4_BLOCK	3727780
#define IC_FTM_BLOCK	3734952
#define IC_KM_ODO_BLOCK	3747500
#define IC_KM_TRIP_BLOCK	3749424
#define IC_KMH_BLOCK	3751348
#define IC_LIGHT_BLOCK	3755832
#define IC_LIGHT_CLOSE_BLOCK	3757372
#define IC_LOCKED_BLOCK	3758912
#define IC_LOGO_1_BLOCK	3773700
#define IC_LOGO_2_BLOCK	3808424
#define IC_LOGO_3_BLOCK	3843148
#define IC_LOGO_4_BLOCK	3877872
#define IC_LOGO_5_BLOCK	3912596
#define IC_LOGO_6_BLOCK	3947320
#define IC_LOGO_7_BLOCK	3982044
#define IC_LOGO_8_BLOCK	4016768
#define IC_LOGO_9_BLOCK	4051492
#define IC_LOGO_10_BLOCK	4086216
#define IC_LOGO_11_BLOCK	4120940
#define IC_LOGO_12_BLOCK	4155664
#define IC_LOWBATTERY_1_1_BLOCK	4190388
#define IC_LOWBATTERY_1_2_BLOCK	4192504
#define IC_LOWBATTERY_1_3_BLOCK	4194172
#define IC_LOWBATTERY_1_4_BLOCK	4194944
#define IC_LOWBATTERY_1_5_BLOCK	4195652
#define IC_LOWBATTERY_2_1_BLOCK	4196232
#define IC_LOWBATTERY_2_2_BLOCK	4198348
#define IC_LOWBATTERY_2_3_BLOCK	4200016
#define IC_LOWBATTERY_2_4_BLOCK	4200788
#define IC_LOWBATTERY_2_5_BLOCK	4201496
#define IC_LOWBATTERY_3_1_BLOCK	4202076
#define IC_LOWBATTERY_3_2_BLOCK	4204192
#define IC_LOWBATTERY_3_3_BLOCK	4205860
#define IC_LOWBATTERY_3_4_BLOCK	4206632
#define IC_LOWBATTERY_3_5_BLOCK	4207340
#define IC_LOWBATTERY_4_1_BLOCK	4207920
#define IC_LOWBATTERY_4_2_BLOCK	4210036
#define IC_LOWBATTERY_4_3_BLOCK	4211704
#define IC_LOWBATTERY_4_4_BLOCK	4212476
#define IC_LOWBATTERY_4_5_BLOCK	4213184
#define IC_LOWBATTERY_5_1_BLOCK	4213764
#define IC_LOWBATTERY_5_2_BLOCK	4215880
#define IC_LOWBATTERY_5_3_BLOCK	4217548
#define IC_LOWBATTERY_5_4_BLOCK	4218320
#define IC_LOWBATTERY_5_5_BLOCK	4219028
#define IC_LOWBATTERY_6_1_BLOCK	4219608
#define IC_LOWBATTERY_6_2_BLOCK	4221724
#define IC_LOWBATTERY_6_3_BLOCK	4223392
#define IC_LOWBATTERY_6_4_BLOCK	4224164
#define IC_LOWBATTERY_6_5_BLOCK	4224872
#define IC_LOWBATTERY_7_1_BLOCK	4225452
#define IC_LOWBATTERY_7_2_BLOCK	4227568
#define IC_LOWBATTERY_7_3_BLOCK	4229236
#define IC_LOWBATTERY_7_4_BLOCK	4230008
#define IC_LOWBATTERY_7_5_BLOCK	4230716
#define IC_LOWBATTERY_8_1_BLOCK	4231296
#define IC_LOWBATTERY_8_2_BLOCK	4233412
#define IC_LOWBATTERY_8_3_BLOCK	4235080
#define IC_LOWBATTERY_8_4_BLOCK	4235852
#define IC_LOWBATTERY_8_5_BLOCK	4236560
#define IC_MASK_BLACK_BLOCK	4237140
#define IC_MASK_CHARGING_BLOCK	4242264
#define IC_MASK_DARK_GRAY_BLOCK	4249468
#define IC_MASK_DARK_GRAY_WIDE_BLOCK	4254592
#define IC_MASK_GRAY_BLOCK	4261508
#define IC_MASK_GRAY_WIDE_BLOCK	4266632
#define IC_MI_ODO_BLOCK	4273548
#define IC_MI_TRIP_BLOCK	4275472
#define IC_MPH_BLOCK	4277396
#define IC_NO_EXTRA_BATTERY_BLOCK	4281880
#define IC_ODO_0_BLOCK	4282844
#define IC_ODO_1_BLOCK	4286432
#define IC_ODO_2_BLOCK	4290020
#define IC_ODO_3_BLOCK	4293608
#define IC_ODO_4_BLOCK	4297196
#define IC_OFF_LOGO_BLOCK	4300784
#define IC_PAIRING_BLOCK	4326132
#define IC_RECONNECT_1_BLOCK	4335096
#define IC_RECONNECT_2_BLOCK	4348924
#define IC_RECONNECT_3_BLOCK	4362752
#define IC_TIME_MASK_BLOCK	4376580
#define IC_TITLE_CHARGING_BLOCK	4384584
#define IC_TITLE_ERROR_BLOCK	4394828
#define IC_TITLE_LOCKED_BLOCK	4401552
#define IC_TITLE_WALKMODE_BLOCK	4410516
#define IC_TRIP_0_BLOCK	4416920
#define IC_TRIP_1_BLOCK	4420508
#define IC_TRIP_2_BLOCK	4424096
#define IC_TRIP_3_BLOCK	4427684
#define IC_TRIP_4_BLOCK	4431272
#define IC_WALK_1_BLOCK	4434860
#define IC_WALK_2_BLOCK	4449648
#define IC_WALK_3_BLOCK	4464436
#define IC_WALK_4_BLOCK	4479224
#define IC_WALK_5_BLOCK	4494012
#define IC_WALK_6_BLOCK	4508800
#define IC_WALK_7_BLOCK	4523588
#define IC_WALK_8_BLOCK	4538376
#define MAINPAGE_0_BLOCK	4553164
#define MAINPAGE_1_BLOCK	4570448
#define MAINPAGE_2_BLOCK	4587732
#define MAINPAGE_3_BLOCK	4605016
#define MAINPAGE_4_BLOCK	4622300
#define MAINPAGE_5_BLOCK	4639584
#define MAINPAGE_6_BLOCK	4656868
#define MAINPAGE_7_BLOCK	4674152
#define MAINPAGE_8_BLOCK	4691436
#define MAINPAGE_9_BLOCK	4708720
#define RADAR_QUICK_01_BLOCK	4726004
#define RADAR_QUICK_02_BLOCK	4734008
#define RADAR_QUICK_03_BLOCK	4742012
#define RADAR_QUICK_04_BLOCK	4750016
#define RADAR_QUICK_05_BLOCK	4758020
#define RADAR_QUICK_06_BLOCK	4766024
#define RADAR_QUICK_07_BLOCK	4774028
#define RADAR_QUICK_08_BLOCK	4782032
#define RADAR_QUICK_09_BLOCK	4790036
#define RADAR_QUICK_10_BLOCK	4798040
#define RADAR_SLOW_01_BLOCK	4806044
#define RADAR_SLOW_02_BLOCK	4814048
#define RADAR_SLOW_03_BLOCK	4822052
#define RADAR_SLOW_04_BLOCK	4830056
#define RADAR_SLOW_05_BLOCK	4838060
#define RADAR_SLOW_06_BLOCK	4846064
#define RADAR_SLOW_07_BLOCK	4854068
#define RADAR_SLOW_08_BLOCK	4862072
#define RADAR_SLOW_09_BLOCK	4870076
#define RADAR_SLOW_10_BLOCK	4878080
#define SUBPAGE_0_BLOCK	4886084
#define SUBPAGE_1_BLOCK	4887624
#define SUBPAGE_2_BLOCK	4889164
#define SUBPAGE_3_BLOCK	4890704
#define SUBPAGE_4_BLOCK	4892244
#define SUBPAGE_5_BLOCK	4893784
#define SUBPAGE_6_BLOCK	4895324
#define SUBPAGE_7_BLOCK	4896864
#define SUBPAGE_8_BLOCK	4898404
#define SUBPAGE_9_BLOCK	4899944
#define SUBPAGE_DOT_BLOCK	4901484
#define SUBPAGE_PERCENT_BLACK_BLOCK	4902000
#define SUBPAGE_PERCENT_DARK_GRAY_BLOCK	4902804
#define SUBPAGE_PERCENT_GRAY_BLOCK	4903608
#define SUBPAGE_RIGHT_0_BLOCK	4904412
#define SUBPAGE_RIGHT_1_BLOCK	4905952
#define SUBPAGE_RIGHT_2_BLOCK	4907492
#define SUBPAGE_RIGHT_3_BLOCK	4909032
#define SUBPAGE_RIGHT_4_BLOCK	4910572
#define SUBPAGE_RIGHT_5_BLOCK	4912112
#define SUBPAGE_RIGHT_6_BLOCK	4913652
#define SUBPAGE_RIGHT_7_BLOCK	4915192
#define SUBPAGE_RIGHT_8_BLOCK	4916732
#define SUBPAGE_RIGHT_9_BLOCK	4918272
#define SUBPAGE_RIGHT_DOT_BLOCK	4919812
#define TIME_0_BLOCK	4920328
#define TIME_1_BLOCK	4921100
#define TIME_2_BLOCK	4921872
#define TIME_3_BLOCK	4922644
#define TIME_4_BLOCK	4923416
#define TIME_5_BLOCK	4924188
#define TIME_6_BLOCK	4924960
#define TIME_7_BLOCK	4925732
#define TIME_8_BLOCK	4926504
#define TIME_9_BLOCK	4927276
#define TIME_COLON_BLOCK	4928048
#define UI_END_ADDRESS	4928436
#define Password_Save_Start_Address 5242880
#define Password_Save_End_Address 5246976
#define Mortor_BIN_START_ADDRESS 7340032
#define Mortor_BIN_END_ADDRESS   7421952


//for ISO13849 messages print;
#define SYS_POWER_ON 	 			"SYSTEM BOOT ON\r\n"
#define SYS_POWER_OFF  				"SYSTEM BOOT OFF\r\n"
#define SYS_BTN_ASSIST_ICREASE 		"BTN_ASSIST_ICREASE DONE\r\n"
#define SYS_BTN_ASSIST_DECREASE  	"BTN_ASSIST_DECREASE DONE\r\n"
#define SYS_BTN_SCREEN_SCROLL  		"BTN_SCREEN_SCROLL DONE\r\n"
#define SYS_BTN_POWER  				"BTN_POWER DONE\r\n"
#define SYS_CAN_INITED 				"CANBUS INITED\r\n"

//Factory test item use status by Miller
#define FTM_INIT_STRING "FTM Ready\n"
#define FTM_ER_MSG_STRING "Error Message=Non\r\n"
#define FTM_ER_MSG_STRING2 "Error Message=ProductID set Error\r\n"
#define FTM_ER_MSG_STRING3 "Error Message=ProductID get Error\r\n"
#define FTM_ER_MSG_STRING4 "Error Message=ProductID2 set Error\r\n"
#define FTM_ER_MSG_STRING5 "Error Message=ProductID2 get Error\r\n"
#define FTM_ER_MSG_STRING6 "Error Message=NorFlash ReadWrite Error\r\n"
#define FTM_ER_MSG_STRING7 "Error Message=LCM test Error\r\n"
#define FTM_ER_MSG_STRING8 "Error Message=BLE test Error\r\n"
#define FTM_ER_MSG_STRING9 "Error Message=Keyoad test Error\r\n"
#define FTM_ER_MSG_STRING11 "Error Message=LCM Backlight test Error\r\n"
#define FTM_ER_MSG_STRING13 "Error Message=BT Address get Error\r\n"
#define FTM_ER_MSG_STRING14 "Error Message=Switch Error\r\n"
#define FTM_ER_MSG_STRING15 "Error Message=Serial Number set Error\r\n"
#define FTM_ER_MSG_STRING16 "Error Message=Serial Number get Error\r\n"

#define FTM_API_STATUS_STRING "API status=ok\r\n"
#define FTM_API_STATUS_STRING2 "API status=fail\r\n"
#define FTM_ALIVE_STRING1 "Recv:\r\n"
#define FTM_ALIVE_STRING2 "usb cable connected\r\n"
#define FTM_ALIVE_STRING3 "Error Message=Non\r\n"
#define FTM_ALIVE_STRING4 "API status=ok\r\n"
#define FTM_PID_GET_STRING "ProductID="
#define FTM_PID_GET_STRING2 "ProductID2="
#define FTM_SERIAL_NO_GET_STRING "Serial No="
#define KEYPAD_ON "Keypad ON\r\n"
#define KEYPAD_OFF "Keypad OFF\r\n"
#define FTM_BT_ADDR_GET_STRING "BT Addr="
#define UI_VER_STRING "software version = g5_ui_v"
#define MCU_VER_STRING "mcu version="
#define VER_HW_STRING "hardware version = N/A\r\n"
#define NA_STRING "N/A"
#define BLE_TXPOWER_STRING "Power = "
#define BLE_RSSI_STRING "RSSI = "
#define NEW_LINE "\r\n"
#define UNKNOW_COMMAND "Error Message=Unknow Command\r\n"
#define API_NOSUPPORT "API status=not support\r\n"
#define HELP_STRING "FTM Alive\nVersions\nProductID set\nProductID get\nProductID2 set\nProductID2 get\nNorFlash ReadWrite\nLCM\nBLE TEST\nKeypad ON\nKeypad OFF\nLCM Backlight ON\nLCM Backlight OF\nBT Address get\nPenetration Protection On\nSetTimeout\r\n"
#define PPON "ON:PPON\r\n"
#define PPOFF "OFF:PPOFF_"
#define MEMORY_TEST_OK "MEMORY CRC check PASS\r\n"


//Factory test item command case by Aaron
#define FTM_Alive 0
#define Versions 1
#define ProductID_set 2
#define ProductID_get 3
#define ProductID2_set 4
#define ProductID2_get 5
#define NorFlash_Read_Write 6
#define LCM 7
#define BLE_TEST 8
#define Keypad_ON 9
#define Keypad_OFF 10
#define Backlight_ON 11
#define Backlight_OFF 12
#define BT_Address_get 13
#define Penetration_Protection_On 14
#define Set_Timeout 15
#define HELP 16
#define GET_CRC 17
#define Penetration_Protection_Off 18
#define Protection_Password 19
#define DFU4USB 20
#define ROM_CMD 21
#define UIID 22
#define SWITCH 23
#define WD_ON 24
#define WD_OFF 25
#define ByPassMode_ON 26
#define ByPassMode_OFF 27
#define ByPass_Version 28
#define ByPass_DATE 29
#define ByPass_Serial_NO 30
#define CAN 31
#define SerialNo_set 32
#define SerialNo_get 33
#define LCM_RED 0
#define LCM_GREEN 1
#define LCM_BLUE 2
#define LCM_WHITE 3
#define LCM_BLACK 4
//Factory test item command case by Aaron

//BLE COMMAND item use status by YUN
#define ASSIST_LEVEL_LIMIT 6
#define LOCK_PASS_WORD   0 //

extern bool flag_throttle_status;
extern bool flag_PINcode_WRONG;

extern uint16_t global_controller_error_code;
extern uint32_t global_motor_error_code;
extern uint16_t global_battery_error_code;
extern uint8_t  global_CAN_time_error_code;
extern uint8_t  global_time_min;
extern uint16_t  global_bms_cycle_count;

extern uint32_t total_seconds;

extern uint8_t MC_MCOD;
extern uint8_t MC_SN_H[8];
extern uint8_t MC_SN_L[8];
extern uint8_t MC_MODEL;
extern uint8_t MC_FW_M_V_H; 
extern uint8_t MC_FW_M_V_L;
extern uint8_t MC_FW_S_V_H;  
extern uint8_t MC_FW_S_V_L; 
extern uint8_t MC_HW_Type_H;
extern uint8_t MC_HW_Type_L;
extern uint16_t MC_YEAR; 
extern uint16_t MC_MONTH;
extern uint16_t MC_DAY;

extern uint8_t  BMS_MCOD;
extern uint8_t  BMS_YEAR;
extern uint8_t  BMS_MONTH;
extern uint8_t  BMS_DAY;
extern uint32_t BMS_SSERIAL_NUMBER; 
extern uint8_t  BMS_MODEL; 
extern uint16_t BMS_FW_MAIN_VERSION; 
extern uint16_t BMS_FW_SECOND_VERSION; 
extern uint16_t BMS_FW_DEBUG_SERIAL; 

void prepare_hmi_connect_data_send();
//BLE COMMAND item use status by YUN




extern int g_iDebug;//0=release 1, 1=debug
extern int g_ifactory_state;//0:FTM Mode 1:Release Mode 2:keypad Mode 
extern char g_chPCBA_PID[16];
extern char BLE_DEVICE_NAME[9];
extern char g_chAllsystem_PID[16];
extern char g_chBT_MAC_address[16];
extern char uiid_temp[30];
extern int uiid_len[1];
extern int ifactor_state_arr[1];
extern int watchdog_flag_arr[1];
extern char motor_fw_version[8];
extern char CAN_SEND_DATA[8];
extern int switch_flag;
extern int lcm_test_flag;
extern int lcm_test_last;
extern char hmi_serial_number[16];
extern char passkey_number[8];

//Factory test item use status by Miller
#define FACTORY_STATUS                  4096
#define WD_ADDRESS                      5000
#define PCBA_PID                        0
#define ALLSYSTEM_PID                   1000
#define SERIAL_NO                       1500
#define PASSKEY_ID                      5600000
#define UI_ADD                          2000
#define UIID_LEN_VALUE                  2500
#define BT_MAC_ADDRESS                  4092120

//Factory test item use status by Ken
extern uint8_t  g_ifactory_ble_conn;
extern uint8_t  g_ifactory_ble_report_step;
extern int8_t   g_ifactory_rssi;
extern int8_t   g_ifactory_txpower;

//BLE peripheral status by Ken
typedef union {
        uint8_t byte;
        struct{
            uint8_t   turn_on:1;
            uint8_t   connected:1;
            uint8_t   bonded:1;

            uint8_t   pairing_complete:1;
            uint8_t   pairing_accept:1;
            uint8_t   reserved:3;
        }bits;
} ble_peripheral_state_t;

extern ble_peripheral_state_t  g_ble_peripheral_state;

void hal_init_mass_production_sys_device( bt_addr_le_t  *scan_mac_addr );

#endif
