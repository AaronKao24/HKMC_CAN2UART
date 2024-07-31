#include <stdio.h>
#include <stdlib.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/display.h>
#include <drivers/counter.h>
#include <logging/log.h>
#include <hal/nrf_gpio.h>
#include <sys/printk.h>
#include "fw_api.h"
#include "hal_api.h"
#include "canfd_process.h"
#include "nRF52840_pin_def.h"
#include "display_api.h"
#include <string.h>

/**
* @fn thread_for_display
* @brief display thread
*/

LOG_MODULE_REGISTER( thread_for_display );

void show_low_battery(uint8_t number);
void show_battery_charging(void);
void show_power_off_charging(void);
void show_walking_mode_page(void);
void show_walking_mode();
void general_error_page(void);
void serious_error_motor_page(void);
void serious_error_m_bms_page(void);
void serious_error_e_bms_page(void);
void show_pair_mode_page(void);
void show_reconnect_page(void);
void show_open_page(void);
void show_ftm_page(void);
void show_time(void);
void reset_charging_page(void);
void reset_walking_page(void);
void reset_pair_page(void);
void reset_reconnect_page(void);
void reset_error_page(void);
void reset_ftm_page(void);
void show_off_logo(void);
void reset_main_page(void);
void reset_sub_page01(void);
void reset_sub_page02(void);
void reset_battery_status_page(void);
void reset_reconnect_page(void);
void reset_power_off_charging_page(void);
void main_page_show(void);
void sub_page1_show(void);
void sub_page2_show(void);
void battery_status_show(void);
void draw_page(uint16_t color);
void draw_partial(uint16_t color, int width, int height, int x, int y);
void show_small_pics(long addr, uint16_t x, uint16_t y);
void show_big_pics(long addr, uint16_t x, uint16_t y);
void counter_init(void);
void radar_alert(void);
void alert_five_times(void);
void test_counter_interrupt_fn(const struct device *counter_dev, uint8_t chan_id, uint32_t ticks, void *user_data);

extern const struct device *st7789v_devs ;
extern struct display_buffer_descriptor desc ;
extern bool is_init_finish ;

#define ALARM_CHANNEL_ID 0

static uint32_t get_ble_passkey = 0;
static uint8_t ble_paired_n = 0;
static uint8_t walk_assist_n = 0;
struct counter_alarm_cfg alarm_cfg;

E_DISPLAY_PAGE t_display_page = E_DISPLAY_PAGE_IDLE, t_display_page_bk = E_DISPLAY_PAGE_IDLE;

int now_sec;
uint64_t now_usec;
uint32_t now_ticks;
uint8_t pic_info[4];
uint16_t picture1[1];
uint16_t picture2[1];

int  height;
int  width;
uint8_t  low_battery_number=0;
static uint8_t animation_fast_n = 0;
static uint8_t animation_slow_n = 0;

#if defined(CONFIG_COUNTER_TIMER3)
#define TIMER DT_LABEL(DT_NODELABEL(timer3))
#endif

void FIT_DISPLAY_NICE(void)
{
	if(!flag_global_system_boot_on) return;

	if(ifactor_state_arr[0] == -1 || 
	   ifactor_state_arr[0] == 0  || 
	   ifactor_state_arr[0] == 1  || 
	   ifactor_state_arr[0] == 3  )
        {
            if(lcm_test_flag == 0 )
            {
                t_display_page = E_DISPLAY_PAGE_FTM;
            }
            else
            {
                t_display_page = E_DISPLAY_PAGE_LCM_TEST;
            }
            
            return;
        }
	switch (flag_global_system_motor_error_activate || flag_global_system_main_bms_error_activate || 
		    flag_global_system_ex_bms_error_activate || flag_global_system_motro_error_general_activate ||
		    flag_global_system_motro_error_general_fw_err_activate)
		{
		case E_DISPLAY_ACTIVE_OFF:
			switch (flag_enter_bypass_mode)
			{
			case E_DISPLAY_ACTIVE_ON:
				t_display_page = E_DISPLAY_PAGE_BYPASS;
				global_page_index = 0;
				break;

			default:

				switch (flag_global_charging_activate)
				{
				case E_DISPLAY_ACTIVE_ON:
					t_display_page = E_DISPLAY_PAGE_CHARGING;
					global_page_index = 0;
					break;

				default:
					switch (flag_global_power_saving_activate)
					{
					case E_DISPLAY_ACTIVE_ON:
						t_display_page = E_DISPLAY_PAGE_POWER_SAVING;
						break;

					default:
						switch (flag_global_locked_activate)
						{
							case E_DISPLAY_ACTIVE_ON:
								t_display_page = E_DISPLAY_PAGE_LOCKED;
								global_page_index = 0;
								break;
						
							default:
								switch (flag_walk_mode_activate)
								{
									case E_DISPLAY_ACTIVE_ON:
										t_display_page = E_DISPLAY_PAGE_WALK_ASSIST;
										break;
								
									default:
										//TODO: Another value  is required to enter ble page.
										if(1 == g_ble_peripheral_state.bits.connected && flag_global_display_ble_page_showed ==false)
										{									
											if(0 == g_ble_peripheral_state.bits.bonded || g_ble_peripheral_state.bits.pairing_accept == 1)
												t_display_page = E_DISPLAY_PAGE_BLE_PAIRING_FIRST;
											else
												t_display_page = E_DISPLAY_PAGE_BLE_PAIRED;
										
											break;
										}
									
										switch (global_page_index)
										{
											case 0:
												t_display_page = E_DISPLAY_PAGE_MAIN;
												break;
										
											case 1:
												t_display_page = E_DISPLAY_PAGE_SUB;
												break;
										
											case 2:
												t_display_page = E_DISPLAY_PAGE_SUB2;
												break;
											case 3:
												t_display_page = E_DISPLAY_PAGE_BATTERY;
												break;
										}
										break;
								}
								break;
						}		
						break;
					}
					break;
				}
				break;
			}
			break;
		
		default:
			t_display_page = E_DISPLAY_PAGE_ERROR;
			global_page_index = 0;
			break;
		}

}

uint8_t *FIT_DISPLAY_PAGE_Get(void)
{
	return (uint8_t*)&t_display_page;
}

void thread_for_display(void)
{		
	while(1)
	{						
		/*	shutdown	*/
		if(flag_off_logo_activate)
		{
			fw_pwm_set(45);
			nrf_gpio_pin_set(PIN_LCM_CRT);
			nrf_gpio_cfg_output(PIN_LCM_CRT);
			display_blanking_off(st7789v_devs);
			show_off_logo();
			flag_off_logo_activate = 0;
			flag_global_system_boot_on = 0;
			t_display_page = t_display_page_bk = E_DISPLAY_PAGE_IDLE;
			global_page_index = 0;
			fw_ble_turn_on_peripheral(false);
			CAN_PAGE_DBG(E_DBG_MESSAGE_SHUTDOWN);	

			if(!flag_global_charging_activate)
			{
				fw_pwm_set(100);
				display_blanking_on(st7789v_devs);
				nrf_gpio_pin_clear(PIN_LCM_CRT);
				nrf_gpio_cfg_output(PIN_LCM_CRT);
				FIT_CANFD_PROCESS_SHUTDOWN();
			}
			else
			{
				reset_power_off_charging_page();
				FIT_CANFD_PROCESS_LIGHT_Send(false);
				CAN_PAGE_DBG(E_DBG_MESSAGE_LIGHT_OFF);
			}
		}
		
		if(t_display_page_bk !=t_display_page)
		{
			if(t_display_page_bk == E_DISPLAY_PAGE_POWER_SAVING)
			{
				fw_pwm_set(45);
				nrf_gpio_pin_set(PIN_LCM_CRT);
				nrf_gpio_cfg_output(PIN_LCM_CRT);
				display_blanking_off(st7789v_devs);
			}

			if(t_display_page_bk == E_DISPLAY_PAGE_CHARGING)
			{
				FIT_CANFD_PROCESS_WALK_ASSIST_LEVEL_Send(1);
			}
			
			switch (t_display_page)
				{					
				case E_DISPLAY_PAGE_MAIN:								
					reset_main_page();
					break;
					
				case E_DISPLAY_PAGE_SUB:
					reset_sub_page01();
					break;
					
				case E_DISPLAY_PAGE_SUB2:
					reset_sub_page02();
					break;

				case E_DISPLAY_PAGE_BATTERY:
					reset_battery_status_page();
					break;
				
				case E_DISPLAY_PAGE_WALK_ASSIST:
					walk_assist_n = 0;
					reset_walking_page();
					break;
					
				case E_DISPLAY_PAGE_LOCKED:

					break;
				
				case E_DISPLAY_PAGE_CHARGING:
					reset_charging_page();
					FIT_CANFD_PROCESS_LIGHT_Send(false);
					break;

				case E_DISPLAY_PAGE_BLE_PAIRED:
					flag_global_display_ble_page_counter = true;
					reset_reconnect_page();
					break;
				
				case E_DISPLAY_PAGE_BLE_PAIRING_FIRST:
					flag_global_display_ble_page_counter = true;
					get_ble_passkey = fw_ble_get_peripheral_passkey();
					H_THOUSAND_DIGIT(&global_dispaly_ble_passkey_code1_digit, &global_dispaly_ble_passkey_code2_digit, 
									 &global_dispaly_ble_passkey_code3_digit, &global_dispaly_ble_passkey_code4_digit,
									 &global_dispaly_ble_passkey_code5_digit, &global_dispaly_ble_passkey_code6_digit,
									 get_ble_passkey);
					reset_pair_page();
					break;

				case  E_DISPLAY_PAGE_POWER_SAVING:
					flag_button_lock = true;
					fw_pwm_set(100);
					nrf_gpio_pin_clear(PIN_LCM_CRT);
					nrf_gpio_cfg_output(PIN_LCM_CRT);
					display_blanking_on(st7789v_devs);
					break;

				case E_DISPLAY_PAGE_ERROR:
					reset_error_page();				
					break;

				case E_DISPLAY_PAGE_FTM:
					switch_flag = 0;
					draw_page(black);
					reset_ftm_page();
					FIT_CANFD_PROCESS_DBG_MESSAGE_Send(E_DBG_MESSAGE_PAGE_FTM);
					break;
				case E_DISPLAY_PAGE_BYPASS:

					break;
				default:
					draw_page(black);
					break;
				}
			//draw_page(black);
			t_display_page_bk = t_display_page;
		}
		
		switch (t_display_page)
			{				
			case E_DISPLAY_PAGE_IDLE:				
				if(flag_global_system_boot_on)
				{
					while( ! hal_is_init_finish() )
					{ 
						delay_msec(30);		
					}
					t_display_page = E_DISPLAY_PAGE_LOGO;
				}
				else
				{
					if(flag_global_charging_activate)
						show_power_off_charging();
				}
				break;
			//PUT			
			case E_DISPLAY_PAGE_LOGO:
				fw_pwm_set(45);
				nrf_gpio_pin_set(PIN_LCM_CRT);
				nrf_gpio_cfg_output(PIN_LCM_CRT);
	    			display_blanking_off(st7789v_devs);
	    			show_open_page();
				FIT_DISPLAY_NICE();
				break;
				
			case E_DISPLAY_PAGE_MAIN:
				FIT_DISPLAY_NICE();
				main_page_show();
				break;
				
			case E_DISPLAY_PAGE_SUB:
				FIT_DISPLAY_NICE();
				sub_page1_show();
				break;
				
			case E_DISPLAY_PAGE_SUB2:
				FIT_DISPLAY_NICE();
				sub_page2_show();
				break;

			case E_DISPLAY_PAGE_BATTERY:
				FIT_DISPLAY_NICE();
				battery_status_show();
				break;

			case E_DISPLAY_PAGE_WALK_ASSIST:
				FIT_DISPLAY_NICE();
				show_walking_mode_page();
				break;
				
			case E_DISPLAY_PAGE_LOCKED:
				FIT_DISPLAY_NICE();
				break;
				
			case E_DISPLAY_PAGE_CHARGING:
				show_battery_charging();
				FIT_DISPLAY_NICE();
				break;		

			case E_DISPLAY_PAGE_BLE_PAIRED:
				show_reconnect_page();
				if(flag_global_display_ble_page_showed || 0 == g_ble_peripheral_state.bits.connected)
				{
					FIT_DISPLAY_NICE();
					flag_global_display_ble_page_counter = false;
				}

				break;
				
			case E_DISPLAY_PAGE_BLE_PAIRING_FIRST:
				show_pair_mode_page();
				if(flag_global_display_ble_page_showed || 0 == g_ble_peripheral_state.bits.connected)
				{
					FIT_DISPLAY_NICE();
					flag_global_display_ble_page_counter = false;
				}
				break;			

			case E_DISPLAY_PAGE_POWER_SAVING:
				if(global_ebike_bluetooth && !flag_global_display_ble_page_showed)
					flag_global_display_ble_page_counter = true;
				FIT_DISPLAY_NICE();

				break;
			
			case E_DISPLAY_PAGE_ERROR:
				FIT_DISPLAY_NICE();
				general_error_page();
				serious_error_motor_page();
				serious_error_m_bms_page();
				serious_error_e_bms_page();
				break;				

			case E_DISPLAY_PAGE_FTM:
			        if (switch_flag == 0 )
			        {
					switch_flag = 1;
					display_blanking_off(st7789v_devs);
					nrf_gpio_pin_set(PIN_LCM_CRT);
					nrf_gpio_cfg_output(PIN_LCM_CRT);
					show_ftm_page();
			         }
			         else
			         {
					FIT_DISPLAY_NICE();
			         }
				 break;   				
			case E_DISPLAY_PAGE_BYPASS:
				FIT_DISPLAY_NICE();
				//show_bypass_page();				
				break;
				
			 case E_DISPLAY_PAGE_LCM_TEST:
				FIT_DISPLAY_NICE();
				break;
                        
				
			default:break;
			}
		
		delay_msec(50);
	}

}


/* 顯示主畫����
 * �數 : void
 * �傳 : void
*/

void main_page_show(void)
{
	flag_button_lock = false;	
	if(!global_enter_status_pics)
	{
		show_small_pics(BG_MAINPAGE_STATUS_BLOCK,60,0);
		global_enter_status_pics=true;
	}
	//�Assist Level 底�����������字�
	if(main_global_motor_assist_level_buf==-1 || main_global_motor_assist_level_buf!=global_motor_assist_level )
	{		
		if(main_global_page_index_buf==-1 || main_global_page_index_buf!=global_page_index )
		{
			global_motor_current_speed_tens_digit_buf=-1;
			global_motor_current_speed_units_digit_buf=-1;
			global_main_bms_soc_level_buf=-1;
			global_enter_low_battery = false;
			switch (global_motor_assist_level){
			case 0 :
				show_small_pics(BG_MAIN_SUB_L0123_1_BLOCK,0,0);
				show_small_pics(BG_MAIN_BATTERY_2_BLOCK,260,0);
				show_big_pics(BG_MAIN_L0_3_BLOCK,0,44);				
				break ;
			case 1 :
				show_small_pics(BG_MAIN_SUB_L0123_1_BLOCK,0,0);
				show_small_pics(BG_MAIN_BATTERY_2_BLOCK,260,0);
				show_big_pics(BG_MAIN_L1_3_BLOCK,0,44);
				break ;
			case 2 :
				show_small_pics(BG_MAIN_SUB_L0123_1_BLOCK,0,0);
				show_small_pics(BG_MAIN_BATTERY_2_BLOCK,260,0);
				show_big_pics(BG_MAIN_L2_3_BLOCK,0,44);
				break ;
			case 3 :
				show_small_pics(BG_MAIN_SUB_L0123_1_BLOCK,0,0);
				show_small_pics(BG_MAIN_BATTERY_2_BLOCK,260,0);
				show_big_pics(BG_MAIN_L3_3_BLOCK,0,44);
				break ;
			case 4 :
				show_small_pics(BG_MAIN_SUB_LEVEL4_1_BLOCK,0,0);
				show_small_pics(BG_MAIN_BATTERY_2_BLOCK,260,0);
				show_big_pics(BG_MAIN_L4_3_BLOCK,0,44);
				break;
			default :
				break ;	
			}
			main_global_page_index_buf=global_page_index;
		}
		else
		{
			switch (global_motor_assist_level){
			case 0 :
				show_small_pics(BG_MAINPAGE_LEVEL_0_BLOCK,0,0);
				show_small_pics(BG_MAINPAGE_LINE_L0_1_BLOCK,260,44);
				show_small_pics(BG_MAINPAGE_LINE_L0_2_BLOCK,260,48);
				show_small_pics(BG_MAINPAGE_LINE_L0_3_BLOCK,260,52);
				show_small_pics(BG_MAINPAGE_LINE_L0_4_BLOCK,260,58);
				show_small_pics(BG_MAINPAGE_LINE_L0_5_BLOCK,264,66);
				show_small_pics(BG_MAINPAGE_LINE_L0_6_BLOCK,260,174);
				show_small_pics(BG_MAINPAGE_LINE_L0_7_BLOCK,260,180);
				show_small_pics(BG_MAINPAGE_LINE_L0_8_BLOCK,260,188);
				show_small_pics(BG_MAINPAGE_LINE_L0_9_BLOCK,260,192);
				break ;
			case 1 :
				show_small_pics(BG_MAINPAGE_LEVEL_1_BLOCK,0,0);
				show_small_pics(BG_MAINPAGE_LINE_L1_1_BLOCK,260,44);
				show_small_pics(BG_MAINPAGE_LINE_L1_2_BLOCK,260,48);
				show_small_pics(BG_MAINPAGE_LINE_L1_3_BLOCK,260,52);
				show_small_pics(BG_MAINPAGE_LINE_L1_4_BLOCK,260,58);
				show_small_pics(BG_MAINPAGE_LINE_L1_5_BLOCK,264,66);
				show_small_pics(BG_MAINPAGE_LINE_L1_6_BLOCK,260,174);
				show_small_pics(BG_MAINPAGE_LINE_L1_7_BLOCK,260,180);
				show_small_pics(BG_MAINPAGE_LINE_L1_8_BLOCK,260,188);
				show_small_pics(BG_MAINPAGE_LINE_L1_9_BLOCK,260,192);
				break ;
			case 2 :
				show_small_pics(BG_MAINPAGE_LEVEL_2_BLOCK,0,0);
				show_small_pics(BG_MAINPAGE_LINE_L2_1_BLOCK,260,44);
				show_small_pics(BG_MAINPAGE_LINE_L2_2_BLOCK,260,48);
				show_small_pics(BG_MAINPAGE_LINE_L2_3_BLOCK,260,52);
				show_small_pics(BG_MAINPAGE_LINE_L2_4_BLOCK,260,58);
				show_small_pics(BG_MAINPAGE_LINE_L2_5_BLOCK,264,66);
				show_small_pics(BG_MAINPAGE_LINE_L2_6_BLOCK,260,174);
				show_small_pics(BG_MAINPAGE_LINE_L2_7_BLOCK,260,180);
				show_small_pics(BG_MAINPAGE_LINE_L2_8_BLOCK,260,188);
				show_small_pics(BG_MAINPAGE_LINE_L2_9_BLOCK,260,192);
				break ;
			case 3 :
				show_small_pics(BG_MAINPAGE_LEVEL_3_BLOCK,0,0);
				show_small_pics(BG_MAINPAGE_LINE_L3_1_BLOCK,260,44);
				show_small_pics(BG_MAINPAGE_LINE_L3_2_BLOCK,260,48);
				show_small_pics(BG_MAINPAGE_LINE_L3_3_BLOCK,260,52);
				show_small_pics(BG_MAINPAGE_LINE_L3_4_BLOCK,260,58);
				show_small_pics(BG_MAINPAGE_LINE_L3_5_BLOCK,264,66);
				show_small_pics(BG_MAINPAGE_LINE_L3_6_BLOCK,260,174);
				show_small_pics(BG_MAINPAGE_LINE_L3_7_BLOCK,260,180);
				show_small_pics(BG_MAINPAGE_LINE_L3_8_BLOCK,260,188);
				show_small_pics(BG_MAINPAGE_LINE_L3_9_BLOCK,260,192);
				break ;
			case 4 :
				show_small_pics(BG_MAINPAGE_LEVEL_4_BLOCK,0,0);
				show_small_pics(BG_MAINPAGE_LINE_L4_1_BLOCK,260,44);
				show_small_pics(BG_MAINPAGE_LINE_L4_2_BLOCK,260,48);
				show_small_pics(BG_MAINPAGE_LINE_L4_3_BLOCK,260,52);
				show_small_pics(BG_MAINPAGE_LINE_L4_4_BLOCK,260,58);
				show_small_pics(BG_MAINPAGE_LINE_L4_5_BLOCK,264,66);
				show_small_pics(BG_MAINPAGE_LINE_L4_6_BLOCK,260,174);
				show_small_pics(BG_MAINPAGE_LINE_L4_7_BLOCK,260,180);
				show_small_pics(BG_MAINPAGE_LINE_L4_8_BLOCK,260,188);
				show_small_pics(BG_MAINPAGE_LINE_L4_9_BLOCK,260,192);
				break;
			default :
				break ;	
			}
		
		}
		main_global_motor_assist_level_buf=global_motor_assist_level;
	}
	//�單位公�制
	if(global_main_distance_unit_buf==-1 || global_main_distance_unit_buf != global_distance_unit )
	{
		switch (global_distance_unit){
		case 0 :
			show_small_pics(IC_KMH_BLOCK,120,191);
			break ;
		case 1 :
			show_small_pics(IC_MPH_BLOCK,120,191);
			break ;
		default :
			break ;	
		}
		global_main_distance_unit_buf=global_distance_unit;
	}
	//�腳踏�大�
	if(global_ebike_light_buf==-1 || global_ebike_light_buf!=global_ebike_light )
	{
		switch (global_ebike_light){
		case 0 :
			show_small_pics(IC_LIGHT_CLOSE_BLOCK,72,16);
			break ;
		case 1 :
			show_small_pics(IC_LIGHT_BLOCK,72,16);
			break ;
		default :
			break ;	
		}
		global_ebike_light_buf=global_ebike_light;
	}

	//�腳踏��芽
	if(global_ebike_bluetooth_buf==-1 || global_ebike_bluetooth_buf!=global_ebike_bluetooth )
	{
		switch (global_ebike_bluetooth){
		case 0 :
			show_small_pics(IC_BLUETOOTH_DISABLE_BLOCK,220,16);
			break ;
		case 1 :
			show_small_pics(IC_BLUETOOTH_BLOCK,220,16);
			break ;
		default :
			break ;	
		}
		global_ebike_bluetooth_buf=global_ebike_bluetooth;
	}
	
	if((g_ble_peripheral_state.bits.bonded >= 1 )&&(g_ble_peripheral_state.bits.connected >=1))
	{		
		show_time_flag = true;		
	}			
		
	if(show_time_flag)
	{
		show_time();
	}	

	//��0位數	
        if(global_motor_current_speed_tens_digit_buf==-1 || global_motor_current_speed_tens_digit_buf!=global_motor_current_speed_tens_digit )
	{
		switch (global_motor_current_speed_tens_digit){
		case 0 :
			show_small_pics(MAINPAGE_0_BLOCK,73,66);
			break ;
		case 1 :
			show_small_pics(MAINPAGE_1_BLOCK,73,66);
			break ;
		case 2 :
			show_small_pics(MAINPAGE_2_BLOCK,73,66);
			break ;
		case 3 :
			show_small_pics(MAINPAGE_3_BLOCK,73,66);
			break ;
		case 4 :
			show_small_pics(MAINPAGE_4_BLOCK,73,66);
			break;
		case 5 :
			show_small_pics(MAINPAGE_5_BLOCK,73,66);
			break;
		case 6 :
			show_small_pics(MAINPAGE_6_BLOCK,73,66);
			break;
		case 7 :
			show_small_pics(MAINPAGE_7_BLOCK,73,66);
			break;
		case 8 :
			show_small_pics(MAINPAGE_8_BLOCK,73,66);
			break;
		case 9 :
			show_small_pics(MAINPAGE_9_BLOCK,73,66);
			break;
		default :
			break ;	
		}
                global_motor_current_speed_tens_digit_buf=global_motor_current_speed_tens_digit;
	}

	//���個�
	if(global_motor_current_speed_units_digit_buf==-1 || global_motor_current_speed_units_digit_buf!=global_motor_current_speed_units_digit )
	{
		switch (global_motor_current_speed_units_digit){
		case 0 :
			show_small_pics(MAINPAGE_0_BLOCK,167,66);
			break ;
		case 1 :
			show_small_pics(MAINPAGE_1_BLOCK,167,66);
			break ;
		case 2 :
			show_small_pics(MAINPAGE_2_BLOCK,167,66);
			break ;
		case 3 :
			show_small_pics(MAINPAGE_3_BLOCK,167,66);
			break ;
		case 4 :
			show_small_pics(MAINPAGE_4_BLOCK,167,66);
			break;
		case 5 :
			show_small_pics(MAINPAGE_5_BLOCK,167,66);
			break;
		case 6 :
			show_small_pics(MAINPAGE_6_BLOCK,167,66);
			break;
		case 7 :
			show_small_pics(MAINPAGE_7_BLOCK,167,66);
			break;
		case 8 :
			show_small_pics(MAINPAGE_8_BLOCK,167,66);
			break;
		case 9 :
			show_small_pics(MAINPAGE_9_BLOCK,167,66);
			break;
		default :
			break ;
		}
		global_motor_current_speed_units_digit_buf=global_motor_current_speed_units_digit;
	}	
	
	//�電池電

	if(global_main_bms_soc_level_buf==-1 || global_main_bms_soc_level_buf!=global_bms_soc_level )
	{
		if(global_bms_soc_level!=1)
		{
			global_main_bms_soc_level_buf=global_bms_soc_level;
		}
		switch (global_bms_soc_level){
		case 1 :
			if(!global_enter_low_battery)
			{
				show_small_pics(IC_BATTERY_EMPTY_3_BLOCK,284,148);
				show_small_pics(IC_BATTERY_EMPTY_4_BLOCK,284,120);
				show_small_pics(IC_BATTERY_EMPTY_5_BLOCK,284,92);
				show_small_pics(IC_BATTERY_EMPTY_6_BLOCK,284,64);
				show_small_pics(IC_BATTERY_EMPTY_7_1_BLOCK,284,56);
				show_small_pics(IC_BATTERY_EMPTY_7_2_BLOCK,276,50);
				show_small_pics(IC_BATTERY_EMPTY_7_3_BLOCK,272,44);
				show_small_pics(IC_BATTERY_EMPTY_7_4_BLOCK,268,32);
				show_small_pics(IC_BATTERY_EMPTY_8_BLOCK,272,10);
				global_enter_low_battery=true;
			}
			if(low_battery_number>14)	low_battery_number=0;
			//if(low_battery_count>2800)
			//{
			//	reset_main_page();
			//}
			show_low_battery(low_battery_number);
			low_battery_number++;
			//low_battery_count++;
			global_main_bms_soc_level_buf=-1;	
			break ;
	
		case 2 :
			global_enter_low_battery=false;
			show_small_pics(IC_BATTERY_FULL_1_BLOCK,272,208);
			show_small_pics(IC_BATTERY_FULL_2_1_BLOCK,268,192);
			show_small_pics(IC_BATTERY_FULL_2_2_BLOCK,272,188);
			show_small_pics(IC_BATTERY_FULL_2_3_BLOCK,276,182);
			show_small_pics(IC_BATTERY_FULL_2_4_BLOCK,284,176);
			show_small_pics(IC_BATTERY_EMPTY_3_BLOCK,284,148);
			show_small_pics(IC_BATTERY_EMPTY_4_BLOCK,284,120);
			show_small_pics(IC_BATTERY_EMPTY_5_BLOCK,284,92);
			show_small_pics(IC_BATTERY_EMPTY_6_BLOCK,284,64);
			show_small_pics(IC_BATTERY_EMPTY_7_1_BLOCK,284,56);
			show_small_pics(IC_BATTERY_EMPTY_7_2_BLOCK,276,50);
			show_small_pics(IC_BATTERY_EMPTY_7_3_BLOCK,272,44);
			show_small_pics(IC_BATTERY_EMPTY_7_4_BLOCK,268,32);
			show_small_pics(IC_BATTERY_EMPTY_8_BLOCK,272,10);			
			break ;
		case 3 :
			global_enter_low_battery=false;
			show_small_pics(IC_BATTERY_FULL_1_BLOCK,272,208);
			show_small_pics(IC_BATTERY_FULL_2_1_BLOCK,268,192);
			show_small_pics(IC_BATTERY_FULL_2_2_BLOCK,272,188);
			show_small_pics(IC_BATTERY_FULL_2_3_BLOCK,276,182);
			show_small_pics(IC_BATTERY_FULL_2_4_BLOCK,284,176);
			show_small_pics(IC_BATTERY_FULL_3_BLOCK,284,148);
			show_small_pics(IC_BATTERY_EMPTY_4_BLOCK,284,120);
			show_small_pics(IC_BATTERY_EMPTY_5_BLOCK,284,92);
			show_small_pics(IC_BATTERY_EMPTY_6_BLOCK,284,64);
			show_small_pics(IC_BATTERY_EMPTY_7_1_BLOCK,284,56);
			show_small_pics(IC_BATTERY_EMPTY_7_2_BLOCK,276,50);
			show_small_pics(IC_BATTERY_EMPTY_7_3_BLOCK,272,44);
			show_small_pics(IC_BATTERY_EMPTY_7_4_BLOCK,268,32);
			show_small_pics(IC_BATTERY_EMPTY_8_BLOCK,272,10);
			break ;
		case 4 :
			global_enter_low_battery=false;
			show_small_pics(IC_BATTERY_FULL_1_BLOCK,272,208);
			show_small_pics(IC_BATTERY_FULL_2_1_BLOCK,268,192);
			show_small_pics(IC_BATTERY_FULL_2_2_BLOCK,272,188);
			show_small_pics(IC_BATTERY_FULL_2_3_BLOCK,276,182);
			show_small_pics(IC_BATTERY_FULL_2_4_BLOCK,284,176);
			show_small_pics(IC_BATTERY_FULL_3_BLOCK,284,148);
			show_small_pics(IC_BATTERY_FULL_4_BLOCK,284,120);
			show_small_pics(IC_BATTERY_EMPTY_5_BLOCK,284,92);
			show_small_pics(IC_BATTERY_EMPTY_6_BLOCK,284,64);
			show_small_pics(IC_BATTERY_EMPTY_7_1_BLOCK,284,56);
			show_small_pics(IC_BATTERY_EMPTY_7_2_BLOCK,276,50);
			show_small_pics(IC_BATTERY_EMPTY_7_3_BLOCK,272,44);
			show_small_pics(IC_BATTERY_EMPTY_7_4_BLOCK,268,32);
			show_small_pics(IC_BATTERY_EMPTY_8_BLOCK,272,10);
			break;
		case 5 :
			global_enter_low_battery=false;
			show_small_pics(IC_BATTERY_FULL_1_BLOCK,272,208);
			show_small_pics(IC_BATTERY_FULL_2_1_BLOCK,268,192);
			show_small_pics(IC_BATTERY_FULL_2_2_BLOCK,272,188);
			show_small_pics(IC_BATTERY_FULL_2_3_BLOCK,276,182);
			show_small_pics(IC_BATTERY_FULL_2_4_BLOCK,284,176);
			show_small_pics(IC_BATTERY_FULL_3_BLOCK,284,148);
			show_small_pics(IC_BATTERY_FULL_4_BLOCK,284,120);
			show_small_pics(IC_BATTERY_FULL_5_BLOCK,284,92);
			show_small_pics(IC_BATTERY_EMPTY_6_BLOCK,284,64);
			show_small_pics(IC_BATTERY_EMPTY_7_1_BLOCK,284,56);
			show_small_pics(IC_BATTERY_EMPTY_7_2_BLOCK,276,50);
			show_small_pics(IC_BATTERY_EMPTY_7_3_BLOCK,272,44);
			show_small_pics(IC_BATTERY_EMPTY_7_4_BLOCK,268,32);
			show_small_pics(IC_BATTERY_EMPTY_8_BLOCK,272,10);
			break;
		case 6 :
			global_enter_low_battery=false;
			show_small_pics(IC_BATTERY_FULL_1_BLOCK,272,208);
			show_small_pics(IC_BATTERY_FULL_2_1_BLOCK,268,192);
			show_small_pics(IC_BATTERY_FULL_2_2_BLOCK,272,188);
			show_small_pics(IC_BATTERY_FULL_2_3_BLOCK,276,182);
			show_small_pics(IC_BATTERY_FULL_2_4_BLOCK,284,176);
			show_small_pics(IC_BATTERY_FULL_3_BLOCK,284,148);
			show_small_pics(IC_BATTERY_FULL_4_BLOCK,284,120);
			show_small_pics(IC_BATTERY_FULL_5_BLOCK,284,92);
			show_small_pics(IC_BATTERY_FULL_6_BLOCK,284,64);
			show_small_pics(IC_BATTERY_EMPTY_7_1_BLOCK,284,56);
			show_small_pics(IC_BATTERY_EMPTY_7_2_BLOCK,276,50);
			show_small_pics(IC_BATTERY_EMPTY_7_3_BLOCK,272,44);
			show_small_pics(IC_BATTERY_EMPTY_7_4_BLOCK,268,32);
			show_small_pics(IC_BATTERY_EMPTY_8_BLOCK,272,10);
			break;
		case 7 :
			global_enter_low_battery=false;
			show_small_pics(IC_BATTERY_FULL_1_BLOCK,272,208);
			show_small_pics(IC_BATTERY_FULL_2_1_BLOCK,268,192);
			show_small_pics(IC_BATTERY_FULL_2_2_BLOCK,272,188);
			show_small_pics(IC_BATTERY_FULL_2_3_BLOCK,276,182);
			show_small_pics(IC_BATTERY_FULL_2_4_BLOCK,284,176);
			show_small_pics(IC_BATTERY_FULL_3_BLOCK,284,148);
			show_small_pics(IC_BATTERY_FULL_4_BLOCK,284,120);
			show_small_pics(IC_BATTERY_FULL_5_BLOCK,284,92);
			show_small_pics(IC_BATTERY_FULL_6_BLOCK,284,64);
			show_small_pics(IC_BATTERY_FULL_7_1_BLOCK,284,56);
			show_small_pics(IC_BATTERY_FULL_7_2_BLOCK,276,50);
			show_small_pics(IC_BATTERY_FULL_7_3_BLOCK,272,44);
			show_small_pics(IC_BATTERY_FULL_7_4_BLOCK,268,32);
			show_small_pics(IC_BATTERY_EMPTY_8_BLOCK,272,10);
			break;
		case 8 :
			global_enter_low_battery=false;
			show_small_pics(IC_BATTERY_FULL_1_BLOCK,272,208);
			show_small_pics(IC_BATTERY_FULL_2_1_BLOCK,268,192);
			show_small_pics(IC_BATTERY_FULL_2_2_BLOCK,272,188);
			show_small_pics(IC_BATTERY_FULL_2_3_BLOCK,276,182);
			show_small_pics(IC_BATTERY_FULL_2_4_BLOCK,284,176);
			show_small_pics(IC_BATTERY_FULL_3_BLOCK,284,148);
			show_small_pics(IC_BATTERY_FULL_4_BLOCK,284,120);
			show_small_pics(IC_BATTERY_FULL_5_BLOCK,284,92);
			show_small_pics(IC_BATTERY_FULL_6_BLOCK,284,64);
			show_small_pics(IC_BATTERY_FULL_7_1_BLOCK,284,56);
			show_small_pics(IC_BATTERY_FULL_7_2_BLOCK,276,50);
			show_small_pics(IC_BATTERY_FULL_7_3_BLOCK,272,44);
			show_small_pics(IC_BATTERY_FULL_7_4_BLOCK,268,32);
			show_small_pics(IC_BATTERY_FULL_8_BLOCK,272,10);
			break;

		default :
			break ;	
		}
	}
	if(ten_meter_alert_flag)
	{
		alert_five_times();
	}		
}

/* 顯示Sub_Page01�面資�
 * �數 : void
 * �傳 : void
*/

void sub_page1_show(void)
{
	flag_button_lock = false;
	if(!global_enter_status_pics)
	{
		show_small_pics(BG_MAINPAGE_STATUS_BLOCK,60,0);
		global_enter_status_pics=true;
	}
	//�Assist Level 底�����������字�
	if(sub01_global_motor_assist_level_buf==-1 || sub01_global_motor_assist_level_buf!=global_motor_assist_level )
	{		
		if(sub01_global_page_index_buf==-1 || sub01_global_page_index_buf!=global_page_index )
		{
			//global_motor_est_range_houndred_digit_buf=-1;
			//global_motor_est_range_tens_digit_buf=-1;
			//global_motor_est_range_units_digit_buf=-1;
			switch (global_motor_assist_level){
			case 0 :
				show_small_pics(BG_MAIN_SUB_L0123_1_BLOCK,0,0);
				show_small_pics(BG_SUB12_2_BLOCK,260,0);
				show_big_pics(BG_SUB1_L0_3_BLOCK,0,44);
				show_small_pics(SUBPAGE_PERCENT_GRAY_BLOCK,238,158);	
				show_small_pics(IC_EST_RANGE_0_BLOCK,52,68);
				show_small_pics(IC_BATTERY_0_BLOCK,195,68);
				break ;
			case 1 :
				show_small_pics(BG_MAIN_SUB_L0123_1_BLOCK,0,0);
				show_small_pics(BG_SUB12_2_BLOCK,260,0);
				show_big_pics(BG_SUB1_L1_3_BLOCK,0,44);
				show_small_pics(SUBPAGE_PERCENT_GRAY_BLOCK,238,158);
				show_small_pics(IC_EST_RANGE_1_BLOCK,52,68);
				show_small_pics(IC_BATTERY_1_BLOCK,195,68);
				break ;
			case 2 :
				show_small_pics(BG_MAIN_SUB_L0123_1_BLOCK,0,0);
				show_small_pics(BG_SUB12_2_BLOCK,260,0);
				show_big_pics(BG_SUB1_L2_3_BLOCK,0,44);
				show_small_pics(SUBPAGE_PERCENT_GRAY_BLOCK,238,158);
				show_small_pics(IC_EST_RANGE_2_BLOCK,52,68);
				show_small_pics(IC_BATTERY_2_BLOCK,195,68);
				break ;
			case 3 :
				show_small_pics(BG_MAIN_SUB_L0123_1_BLOCK,0,0);
				show_small_pics(BG_SUB12_2_BLOCK,260,0);
				show_big_pics(BG_SUB1_L3_3_BLOCK,0,44);
				show_small_pics(SUBPAGE_PERCENT_GRAY_BLOCK,238,158);				
				show_small_pics(IC_EST_RANGE_3_BLOCK,52,68);
				show_small_pics(IC_BATTERY_3_BLOCK,195,68);
				break ;
			case 4 :
				show_small_pics(BG_MAIN_SUB_LEVEL4_1_BLOCK,0,0);
				show_small_pics(BG_SUB12_2_BLOCK,260,0);
				show_big_pics(BG_SUB1_L4_3_BLOCK,0,44);
				show_small_pics(SUBPAGE_PERCENT_GRAY_BLOCK,238,158);			
				show_small_pics(IC_EST_RANGE_4_BLOCK,52,68);
				show_small_pics(IC_BATTERY_4_BLOCK,195,68);
				break;
			default :
				break ;	
			}
			sub01_global_page_index_buf=global_page_index;
		}
		else
		{
			switch (global_motor_assist_level){
			case 0 :
				show_small_pics(BG_SUBPAGE_LEVEL_0_BLOCK,0,0);
				show_small_pics(BG_SUBPAGE_1_LEVEL_0_BLOCK,52,0);
				show_small_pics(BG_SUBPAGE_2_LEVEL_0_BLOCK,52,180);
				show_small_pics(BG_SUBPAGE_3_LEVEL_0_BLOCK,276,0);
				show_small_pics(BG_SUBPAGE_4_LEVEL_0_BLOCK,300,64);
				show_small_pics(BG_SUBPAGE_5_LEVEL_0_BLOCK,284,172);
				show_small_pics(IC_EST_RANGE_0_BLOCK,52,68);
				show_small_pics(IC_BATTERY_0_BLOCK,195,68);
				break ;
			case 1 :
				show_small_pics(BG_SUBPAGE_LEVEL_1_BLOCK,0,0);
				show_small_pics(BG_SUBPAGE_1_LEVEL_1_BLOCK,52,0);
				show_small_pics(BG_SUBPAGE_2_LEVEL_1_BLOCK,52,180);
				show_small_pics(BG_SUBPAGE_3_LEVEL_1_BLOCK,276,0);
				show_small_pics(BG_SUBPAGE_4_LEVEL_1_BLOCK,300,64);
				show_small_pics(BG_SUBPAGE_5_LEVEL_1_BLOCK,284,172);
				show_small_pics(IC_EST_RANGE_1_BLOCK,52,68);
				show_small_pics(IC_BATTERY_1_BLOCK,195,68);
				break ;
			case 2 :
				show_small_pics(BG_SUBPAGE_LEVEL_2_BLOCK,0,0);
				show_small_pics(BG_SUBPAGE_1_LEVEL_2_BLOCK,52,0);
				show_small_pics(BG_SUBPAGE_2_LEVEL_2_BLOCK,52,180);
				show_small_pics(BG_SUBPAGE_3_LEVEL_2_BLOCK,276,0);
				show_small_pics(BG_SUBPAGE_4_LEVEL_2_BLOCK,300,64);
				show_small_pics(BG_SUBPAGE_5_LEVEL_2_BLOCK,284,172);
				show_small_pics(IC_EST_RANGE_2_BLOCK,52,68);
				show_small_pics(IC_BATTERY_2_BLOCK,195,68);
				break ;
			case 3 :
				show_small_pics(BG_SUBPAGE_LEVEL_3_BLOCK,0,0);
				show_small_pics(BG_SUBPAGE_1_LEVEL_3_BLOCK,52,0);
				show_small_pics(BG_SUBPAGE_2_LEVEL_3_BLOCK,52,180);
				show_small_pics(BG_SUBPAGE_3_LEVEL_3_BLOCK,276,0);
				show_small_pics(BG_SUBPAGE_4_LEVEL_3_BLOCK,300,64);
				show_small_pics(BG_SUBPAGE_5_LEVEL_3_BLOCK,284,172);
				show_small_pics(IC_EST_RANGE_3_BLOCK,52,68);
				show_small_pics(IC_BATTERY_3_BLOCK,195,68);
				break ;
			case 4 :
				show_small_pics(BG_SUBPAGE_LEVEL_4_BLOCK,0,0);
				show_small_pics(BG_SUBPAGE_1_LEVEL_4_BLOCK,52,0);
				show_small_pics(BG_SUBPAGE_2_LEVEL_4_BLOCK,52,180);
				show_small_pics(BG_SUBPAGE_3_LEVEL_4_BLOCK,276,0);
				show_small_pics(BG_SUBPAGE_4_LEVEL_4_BLOCK,300,64);
				show_small_pics(BG_SUBPAGE_5_LEVEL_4_BLOCK,284,172);
				show_small_pics(IC_EST_RANGE_4_BLOCK,52,68);
				show_small_pics(IC_BATTERY_4_BLOCK,195,68);
				break;
			default :
				break ;	
			}
		
		}
		sub01_global_motor_assist_level_buf=global_motor_assist_level;
	}
	//�單位公�制
	if(global_sub01_distance_unit_buf==-1 || global_sub01_distance_unit_buf != global_distance_unit )
	{
		switch (global_distance_unit){
		case 0 :
			show_small_pics(IC_KM_ODO_BLOCK,94,157);
			break ;
		case 1 :
			show_small_pics(IC_MI_ODO_BLOCK,94,157);
			break ;
		default :
			break ;	
		}
		global_sub01_distance_unit_buf=global_distance_unit;
	}		
	//�腳踏�大�
	if(global_ebike_light_buf==-1 || global_ebike_light_buf!=global_ebike_light )
	{
		switch (global_ebike_light){
		case 0 :
			show_small_pics(IC_LIGHT_CLOSE_BLOCK,72,16);
			break ;
		case 1 :
			show_small_pics(IC_LIGHT_BLOCK,72,16);
			break ;
		default :
			break ;	
		}
		global_ebike_light_buf=global_ebike_light;
	}

	//�腳踏��芽
	if(global_ebike_bluetooth_buf==-1 || global_ebike_bluetooth_buf!=global_ebike_bluetooth )
	{
		switch (global_ebike_bluetooth){
		case 0 :
			show_small_pics(IC_BLUETOOTH_DISABLE_BLOCK,220,16);
			break ;
		case 1 :
			show_small_pics(IC_BLUETOOTH_BLOCK,220,16);
			break ;
		default :
			break ;	
		}
		global_ebike_bluetooth_buf=global_ebike_bluetooth;
	}

	if((g_ble_peripheral_state.bits.bonded >= 1 )&&(g_ble_peripheral_state.bits.connected >=1))
	{		
		show_time_flag = true;		
	}			
		
	if(show_time_flag)
	{
		show_time();
	}	

	if(global_motor_est_range_buf==-1 || global_motor_est_range_buf!=global_motor_est_range )
	{
		show_small_pics(IC_MASK_DARK_GRAY_WIDE_BLOCK,73,109);
		if(global_motor_est_range>=100)
		{	
			//�百位數��
			switch (global_motor_est_range_houndred_digit){
				case 0 :
					show_small_pics(SUBPAGE_0_BLOCK,75,109);
					break ;
				case 1 :
					show_small_pics(SUBPAGE_1_BLOCK,75,109);
					break ;
				case 2 :
					show_small_pics(SUBPAGE_2_BLOCK,75,109);
					break ;
				case 3 :
					show_small_pics(SUBPAGE_3_BLOCK,75,109);
					break ;
				case 4 :
					show_small_pics(SUBPAGE_4_BLOCK,75,109);
					break;
				case 5 :
					show_small_pics(SUBPAGE_5_BLOCK,75,109);
					break;
				case 6 :
					show_small_pics(SUBPAGE_6_BLOCK,75,109);
					break;
				case 7 :
					show_small_pics(SUBPAGE_7_BLOCK,75,109);
					break;
				case 8 :
					show_small_pics(SUBPAGE_8_BLOCK,75,109);
					break;
				case 9 :
					show_small_pics(SUBPAGE_9_BLOCK,75,109);
					break;
				default :
					break ;	
				}
			//��位數��
			switch (global_motor_est_range_tens_digit){
				case 0 :
					show_small_pics(SUBPAGE_0_BLOCK,102,109);
					break ;
				case 1 :
					show_small_pics(SUBPAGE_1_BLOCK,102,109);
					break ;
				case 2 :
					show_small_pics(SUBPAGE_2_BLOCK,102,109);
					break ;
				case 3 :
					show_small_pics(SUBPAGE_3_BLOCK,102,109);
					break ;
				case 4 :
					show_small_pics(SUBPAGE_4_BLOCK,102,109);
					break;
				case 5 :
					show_small_pics(SUBPAGE_5_BLOCK,102,109);
					break;
				case 6 :
					show_small_pics(SUBPAGE_6_BLOCK,102,109);
					break;
				case 7 :
					show_small_pics(SUBPAGE_7_BLOCK,102,109);
					break;
				case 8 :
					show_small_pics(SUBPAGE_8_BLOCK,102,109);
					break;
				case 9 :
					show_small_pics(SUBPAGE_9_BLOCK,102,109);
					break;
				default :
					break ;	
				}
			//�個���
			switch (global_motor_est_range_units_digit){
				case 0 :
					show_small_pics(SUBPAGE_0_BLOCK,129,109);
					break ;
				case 1 :
					show_small_pics(SUBPAGE_1_BLOCK,129,109);
					break ;
				case 2 :
					show_small_pics(SUBPAGE_2_BLOCK,129,109);
					break ;
				case 3 :
					show_small_pics(SUBPAGE_3_BLOCK,129,109);
					break ;
				case 4 :
					show_small_pics(SUBPAGE_4_BLOCK,129,109);
					break;
				case 5 :
					show_small_pics(SUBPAGE_5_BLOCK,129,109);
					break;
				case 6 :
					show_small_pics(SUBPAGE_6_BLOCK,129,109);
					break;
				case 7 :
					show_small_pics(SUBPAGE_7_BLOCK,129,109);
					break;
				case 8 :
					show_small_pics(SUBPAGE_8_BLOCK,129,109);
					break;
				case 9 :
					show_small_pics(SUBPAGE_9_BLOCK,129,109);
					break;
				default :
					break ;	
				}
		}
		else if((global_motor_est_range>=10)&&(global_motor_est_range<100))
		{
			//��位數��
			switch (global_motor_est_range_tens_digit){
				case 0 :
					show_small_pics(SUBPAGE_0_BLOCK,89,109);
					break ;
				case 1 :
					show_small_pics(SUBPAGE_1_BLOCK,89,109);
					break ;
				case 2 :
					show_small_pics(SUBPAGE_2_BLOCK,89,109);
					break ;
				case 3 :
					show_small_pics(SUBPAGE_3_BLOCK,89,109);
					break ;
				case 4 :
					show_small_pics(SUBPAGE_4_BLOCK,89,109);
					break;
				case 5 :
					show_small_pics(SUBPAGE_5_BLOCK,89,109);
					break;
				case 6 :
					show_small_pics(SUBPAGE_6_BLOCK,89,109);
					break;
				case 7 :
					show_small_pics(SUBPAGE_7_BLOCK,89,109);
					break;
				case 8 :
					show_small_pics(SUBPAGE_8_BLOCK,89,109);
					break;
				case 9 :
					show_small_pics(SUBPAGE_9_BLOCK,89,109);
					break;
				default :
					break ;	
				}
			//�個���
			switch (global_motor_est_range_units_digit){
				case 0 :
					show_small_pics(SUBPAGE_0_BLOCK,116,109);
					break ;
				case 1 :
					show_small_pics(SUBPAGE_1_BLOCK,116,109);
					break ;
				case 2 :
					show_small_pics(SUBPAGE_2_BLOCK,116,109);
					break ;
				case 3 :
					show_small_pics(SUBPAGE_3_BLOCK,116,109);
					break ;
				case 4 :
					show_small_pics(SUBPAGE_4_BLOCK,116,109);
					break;
				case 5 :
					show_small_pics(SUBPAGE_5_BLOCK,116,109);
					break;
				case 6 :
					show_small_pics(SUBPAGE_6_BLOCK,116,109);
					break;
				case 7 :
					show_small_pics(SUBPAGE_7_BLOCK,116,109);
					break;
				case 8 :
					show_small_pics(SUBPAGE_8_BLOCK,116,109);
					break;
				case 9 :
					show_small_pics(SUBPAGE_9_BLOCK,116,109);
					break;
				default :
					break ;	
				}
		}		
		else
		{
			//�個���
			switch (global_motor_est_range_units_digit){
				case 0 :
					show_small_pics(SUBPAGE_0_BLOCK,102,109);
					break ;
				case 1 :
					show_small_pics(SUBPAGE_1_BLOCK,102,109);
					break ;
				case 2 :
					show_small_pics(SUBPAGE_2_BLOCK,102,109);
					break ;
				case 3 :
					show_small_pics(SUBPAGE_3_BLOCK,102,109);
					break ;
				case 4 :
					show_small_pics(SUBPAGE_4_BLOCK,102,109);
					break;
				case 5 :
					show_small_pics(SUBPAGE_5_BLOCK,102,109);
					break;
				case 6 :
					show_small_pics(SUBPAGE_6_BLOCK,102,109);
					break;
				case 7 :
					show_small_pics(SUBPAGE_7_BLOCK,102,109);
					break;
				case 8 :
					show_small_pics(SUBPAGE_8_BLOCK,102,109);
					break;
				case 9 :
					show_small_pics(SUBPAGE_9_BLOCK,102,109);
					break;
				default :
					break ;	
				}			
		}	
		global_motor_est_range_buf=global_motor_est_range; 		
	}
	
	//�BMS��
	if(global_bms_soc_buf==-1 || global_bms_soc_buf!=global_bms_soc_new )
	{
		show_small_pics(IC_MASK_GRAY_BLOCK,207,109);
		if(global_bms_soc_new==100)
		{	
			show_small_pics(SUBPAGE_RIGHT_1_BLOCK,209,109);//��
			show_small_pics(SUBPAGE_RIGHT_0_BLOCK,236,109);//��
			show_small_pics(SUBPAGE_RIGHT_0_BLOCK,263,109);//��
		}
		else if((global_bms_soc_new>=10)&&(global_bms_soc_new<100))
		{
			//�SOC���數��
			switch (global_bms_soc_tens_digit){
				case 1 :
					show_small_pics(SUBPAGE_RIGHT_1_BLOCK,223,109);
					break ;
				case 2 :
					show_small_pics(SUBPAGE_RIGHT_2_BLOCK,223,109);
					break ;
				case 3 :
					show_small_pics(SUBPAGE_RIGHT_3_BLOCK,223,109);
					break ;
				case 4 :
					show_small_pics(SUBPAGE_RIGHT_4_BLOCK,223,109);
					break;
				case 5 :
					show_small_pics(SUBPAGE_RIGHT_5_BLOCK,223,109);
					break;
				case 6 :
					show_small_pics(SUBPAGE_RIGHT_6_BLOCK,223,109);
					break;
				case 7 :
					show_small_pics(SUBPAGE_RIGHT_7_BLOCK,223,109);
					break;
				case 8 :
					show_small_pics(SUBPAGE_RIGHT_8_BLOCK,223,109);
					break;
				case 9 :
					show_small_pics(SUBPAGE_RIGHT_9_BLOCK,223,109);
					break;
				default :
					break ;	
				}
			//�SOC���數��
			switch (global_bms_soc_units_digit){
				case 0 :
					show_small_pics(SUBPAGE_RIGHT_0_BLOCK,250,109);
					break ;
				case 1 :
					show_small_pics(SUBPAGE_RIGHT_1_BLOCK,250,109);
					break ;
				case 2 :
					show_small_pics(SUBPAGE_RIGHT_2_BLOCK,250,109);
					break ;
				case 3 :
					show_small_pics(SUBPAGE_RIGHT_3_BLOCK,250,109);
					break ;
				case 4 :
					show_small_pics(SUBPAGE_RIGHT_4_BLOCK,250,109);
					break;
				case 5 :
					show_small_pics(SUBPAGE_RIGHT_5_BLOCK,250,109);
					break;
				case 6 :
					show_small_pics(SUBPAGE_RIGHT_6_BLOCK,250,109);
					break;
				case 7 :
					show_small_pics(SUBPAGE_RIGHT_7_BLOCK,250,109);
					break;
				case 8 :
					show_small_pics(SUBPAGE_RIGHT_8_BLOCK,250,109);
					break;
				case 9 :
					show_small_pics(SUBPAGE_RIGHT_9_BLOCK,250,109);
					break;
				default :
					break ;	
				}
		}		
		else
		{
				//�SOC��
				switch (global_bms_soc_units_digit){
				case 0 :
					show_small_pics(SUBPAGE_RIGHT_0_BLOCK,236,109);
					break ;
				case 1 :
					show_small_pics(SUBPAGE_RIGHT_1_BLOCK,236,109);
					break ;
				case 2 :
					show_small_pics(SUBPAGE_RIGHT_2_BLOCK,236,109);
					break ;
				case 3 :
					show_small_pics(SUBPAGE_RIGHT_3_BLOCK,236,109);
					break ;
				case 4 :
					show_small_pics(SUBPAGE_RIGHT_4_BLOCK,236,109);
					break;
				case 5 :
					show_small_pics(SUBPAGE_RIGHT_5_BLOCK,236,109);
					break;
				case 6 :
					show_small_pics(SUBPAGE_RIGHT_6_BLOCK,236,109);
					break;
				case 7 :
					show_small_pics(SUBPAGE_RIGHT_7_BLOCK,236,109);
					break;
				case 8 :
					show_small_pics(SUBPAGE_RIGHT_8_BLOCK,236,109);
					break;
				case 9 :
					show_small_pics(SUBPAGE_RIGHT_9_BLOCK,236,109);
					break;
				default :
					break ;	
				}			
		}	
		global_bms_soc_buf=global_bms_soc_new; 		
	}		
}

/* 顯示Sub Page 02�面資�
 * �數 : void
 * �傳 : void
*/

void sub_page2_show(void)
{
	flag_button_lock = false;
	if(!global_enter_status_pics)
	{
		show_small_pics(BG_MAINPAGE_STATUS_BLOCK,60,0);
		global_enter_status_pics=true;
	}	
	//�Assist Level 底�����������字�
	if(sub02_global_motor_assist_level_buf==-1 || sub02_global_motor_assist_level_buf!=global_motor_assist_level )
	{		
		if(sub02_global_page_index_buf==-1 || sub02_global_page_index_buf!=global_page_index )
		{
			global_motor_odo_ten_thousands_digit_buf=-1;
			global_motor_odo_thousands_digit_buf=-1;
			global_motor_odo_hundreds_digit_buf=-1;
			global_motor_odo_tens_digit_buf =-1;
			global_motor_odo_units_digit_buf =-1;
			//global_motor_trip_hundred_digit_buf=-1;
			//global_motor_trip_tens_digit_buf=-1;
			//global_motor_trip_units_digit_buf=-1;
			switch (global_motor_assist_level){
			case 0 :
				show_small_pics(BG_MAIN_SUB_L0123_1_BLOCK,0,0);
				show_small_pics(BG_SUB12_2_BLOCK,260,0);
				show_big_pics(BG_SUB2_L0_3_BLOCK,0,44);				
				show_small_pics(IC_ODO_0_BLOCK,87,68);
				show_small_pics(IC_TRIP_0_BLOCK,220,68);
				break ;
			case 1 :
				show_small_pics(BG_MAIN_SUB_L0123_1_BLOCK,0,0);
				show_small_pics(BG_SUB12_2_BLOCK,260,0);
				show_big_pics(BG_SUB2_L1_3_BLOCK,0,44);
				show_small_pics(IC_ODO_1_BLOCK,87,68);
				show_small_pics(IC_TRIP_1_BLOCK,220,68);
				break ;
			case 2 :
				show_small_pics(BG_MAIN_SUB_L0123_1_BLOCK,0,0);
				show_small_pics(BG_SUB12_2_BLOCK,260,0);
				show_big_pics(BG_SUB2_L2_3_BLOCK,0,44);
				show_small_pics(IC_ODO_2_BLOCK,87,68);
				show_small_pics(IC_TRIP_2_BLOCK,220,68);
				break ;
			case 3 :
				show_small_pics(BG_MAIN_SUB_L0123_1_BLOCK,0,0);
				show_small_pics(BG_SUB12_2_BLOCK,260,0);
				show_big_pics(BG_SUB2_L3_3_BLOCK,0,44);
				show_small_pics(IC_ODO_3_BLOCK,87,68);
				show_small_pics(IC_TRIP_3_BLOCK,220,68);
				break ;
			case 4 :
				show_small_pics(BG_MAIN_SUB_LEVEL4_1_BLOCK,0,0);
				show_small_pics(BG_SUB12_2_BLOCK,260,0);
				show_big_pics(BG_SUB2_L4_3_BLOCK,0,44);
				show_small_pics(IC_ODO_4_BLOCK,87,68);
				show_small_pics(IC_TRIP_4_BLOCK,220,68);
				break;
			default :
				break ;	
			}
			sub02_global_page_index_buf=global_page_index;
		}
		else
		{
			switch (global_motor_assist_level){
			case 0 :
				show_small_pics(BG_SUBPAGE_LEVEL_0_BLOCK,0,0);
				show_small_pics(BG_SUBPAGE_1_LEVEL_0_BLOCK,52,0);
				show_small_pics(BG_SUBPAGE_2_LEVEL_0_BLOCK,52,180);
				show_small_pics(BG_SUBPAGE_3_LEVEL_0_BLOCK,276,0);
				show_small_pics(BG_SUBPAGE_4_LEVEL_0_BLOCK,300,64);
				show_small_pics(BG_SUBPAGE_5_LEVEL_0_BLOCK,284,172);
				show_small_pics(IC_ODO_0_BLOCK,87,68);
				show_small_pics(IC_TRIP_0_BLOCK,220,68);
				break ;
			case 1 :
				show_small_pics(BG_SUBPAGE_LEVEL_1_BLOCK,0,0);
				show_small_pics(BG_SUBPAGE_1_LEVEL_1_BLOCK,52,0);
				show_small_pics(BG_SUBPAGE_2_LEVEL_1_BLOCK,52,180);
				show_small_pics(BG_SUBPAGE_3_LEVEL_1_BLOCK,276,0);
				show_small_pics(BG_SUBPAGE_4_LEVEL_1_BLOCK,300,64);
				show_small_pics(BG_SUBPAGE_5_LEVEL_1_BLOCK,284,172);
				show_small_pics(IC_ODO_1_BLOCK,87,68);
				show_small_pics(IC_TRIP_1_BLOCK,220,68);
				break ;
			case 2 :
				show_small_pics(BG_SUBPAGE_LEVEL_2_BLOCK,0,0);
				show_small_pics(BG_SUBPAGE_1_LEVEL_2_BLOCK,52,0);
				show_small_pics(BG_SUBPAGE_2_LEVEL_2_BLOCK,52,180);
				show_small_pics(BG_SUBPAGE_3_LEVEL_2_BLOCK,276,0);
				show_small_pics(BG_SUBPAGE_4_LEVEL_2_BLOCK,300,64);
				show_small_pics(BG_SUBPAGE_5_LEVEL_2_BLOCK,284,172);
				show_small_pics(IC_ODO_2_BLOCK,87,68);
				show_small_pics(IC_TRIP_2_BLOCK,220,68);
				break ;
			case 3 :
				show_small_pics(BG_SUBPAGE_LEVEL_3_BLOCK,0,0);
				show_small_pics(BG_SUBPAGE_1_LEVEL_3_BLOCK,52,0);
				show_small_pics(BG_SUBPAGE_2_LEVEL_3_BLOCK,52,180);
				show_small_pics(BG_SUBPAGE_3_LEVEL_3_BLOCK,276,0);
				show_small_pics(BG_SUBPAGE_4_LEVEL_3_BLOCK,300,64);
				show_small_pics(BG_SUBPAGE_5_LEVEL_3_BLOCK,284,172);
				show_small_pics(IC_ODO_3_BLOCK,87,68);
				show_small_pics(IC_TRIP_3_BLOCK,220,68);
				break ;
			case 4 :
				show_small_pics(BG_SUBPAGE_LEVEL_4_BLOCK,0,0);
				show_small_pics(BG_SUBPAGE_1_LEVEL_4_BLOCK,52,0);
				show_small_pics(BG_SUBPAGE_2_LEVEL_4_BLOCK,52,180);
				show_small_pics(BG_SUBPAGE_3_LEVEL_4_BLOCK,276,0);
				show_small_pics(BG_SUBPAGE_4_LEVEL_4_BLOCK,300,64);
				show_small_pics(BG_SUBPAGE_5_LEVEL_4_BLOCK,284,172);
				show_small_pics(IC_ODO_4_BLOCK,87,68);
				show_small_pics(IC_TRIP_4_BLOCK,220,68);
				break;
			default :
				break ;	
			}
		
		}
		sub02_global_motor_assist_level_buf=global_motor_assist_level;
	}
	//�單位公�制
	if(global_sub02_distance_unit_buf==-1 || global_sub02_distance_unit_buf != global_distance_unit )
	{
		switch (global_distance_unit){
		case 0 :
			show_small_pics(IC_KM_ODO_BLOCK,94,157);
			show_small_pics(IC_KM_TRIP_BLOCK,235,157);
			break ;
		case 1 :
			show_small_pics(IC_MI_ODO_BLOCK,94,157);
			show_small_pics(IC_MI_TRIP_BLOCK,235,157);
			break ;
		default :
			break ;	
		}
		global_sub02_distance_unit_buf=global_distance_unit;
	}		
	
	//�腳踏�大�
	if(global_ebike_light_buf==-1 || global_ebike_light_buf!=global_ebike_light )
	{
		switch (global_ebike_light){
		case 0 :
			show_small_pics(IC_LIGHT_CLOSE_BLOCK,72,16);
			break ;
		case 1 :
			show_small_pics(IC_LIGHT_BLOCK,72,16);
			break ;
		default :
			break ;	
		}
		global_ebike_light_buf=global_ebike_light;
	}

	//�腳踏��芽
	if(global_ebike_bluetooth_buf==-1 || global_ebike_bluetooth_buf!=global_ebike_bluetooth )
	{
		switch (global_ebike_bluetooth){
		case 0 :
			show_small_pics(IC_BLUETOOTH_DISABLE_BLOCK,220,16);
			break ;
		case 1 :
			show_small_pics(IC_BLUETOOTH_BLOCK,220,16);
			break ;
		default :
			break ;	
		}
		global_ebike_bluetooth_buf=global_ebike_bluetooth;
	}		
		
	if((g_ble_peripheral_state.bits.bonded >= 1 )&&(g_ble_peripheral_state.bits.connected >=1))
	{		
		show_time_flag = true;		
	}			
		
	if(show_time_flag)
	{
		show_time();
	}	

	//�ODO��
	if(global_motor_odo_ten_thousands_digit_buf==-1 || global_motor_odo_ten_thousands_digit_buf!=global_motor_odo_ten_thousands_digit )
	{
		switch (global_motor_odo_ten_thousands_digit){
		case 0 :
			show_small_pics(SUBPAGE_0_BLOCK,55,109);
			break ;
		case 1 :
			show_small_pics(SUBPAGE_1_BLOCK,55,109);
			break ;
		case 2 :
			show_small_pics(SUBPAGE_2_BLOCK,55,109);
			break ;
		case 3 :
			show_small_pics(SUBPAGE_3_BLOCK,55,109);
			break ;
		case 4 :
			show_small_pics(SUBPAGE_4_BLOCK,55,109);
			break;
		case 5 :
			show_small_pics(SUBPAGE_5_BLOCK,55,109);
			break;
		case 6 :
			show_small_pics(SUBPAGE_6_BLOCK,55,109);
			break;
		case 7 :
			show_small_pics(SUBPAGE_7_BLOCK,55,109);
			break;
		case 8 :
			show_small_pics(SUBPAGE_8_BLOCK,55,109);
			break;
		case 9 :
			show_small_pics(SUBPAGE_9_BLOCK,55,109);
			break;
		default :
			break ;	
		}
		global_motor_odo_ten_thousands_digit_buf=global_motor_odo_ten_thousands_digit;
	}
	//�ODO��
	if(global_motor_odo_thousands_digit_buf==-1 || global_motor_odo_thousands_digit_buf!=global_motor_odo_thousands_digit )
	{
		switch (global_motor_odo_thousands_digit){
		case 0 :
			show_small_pics(SUBPAGE_0_BLOCK,82,109);
			break ;
		case 1 :
			show_small_pics(SUBPAGE_1_BLOCK,82,109);
			break ;
		case 2 :
			show_small_pics(SUBPAGE_2_BLOCK,82,109);
			break ;
		case 3 :
			show_small_pics(SUBPAGE_3_BLOCK,82,109);
			break ;
		case 4 :
			show_small_pics(SUBPAGE_4_BLOCK,82,109);
			break;
		case 5 :
			show_small_pics(SUBPAGE_5_BLOCK,82,109);
			break;
		case 6 :
			show_small_pics(SUBPAGE_6_BLOCK,82,109);
			break;
		case 7 :
			show_small_pics(SUBPAGE_7_BLOCK,82,109);
			break;
		case 8 :
			show_small_pics(SUBPAGE_8_BLOCK,82,109);
			break;
		case 9 :
			show_small_pics(SUBPAGE_9_BLOCK,82,109);
			break;
		default :
			break ;	
		}
		global_motor_odo_thousands_digit_buf=global_motor_odo_thousands_digit;
	}

	//�ODO��
	if(global_motor_odo_hundreds_digit_buf==-1 || global_motor_odo_hundreds_digit_buf!=global_motor_odo_hundreds_digit )
	{
		switch (global_motor_odo_hundreds_digit){
		case 0 :
			show_small_pics(SUBPAGE_0_BLOCK,109,109);
			break ;
		case 1 :
			show_small_pics(SUBPAGE_1_BLOCK,109,109);
			break ;
		case 2 :
			show_small_pics(SUBPAGE_2_BLOCK,109,109);
			break ;
		case 3 :
			show_small_pics(SUBPAGE_3_BLOCK,109,109);
			break ;
		case 4 :
			show_small_pics(SUBPAGE_4_BLOCK,109,109);
			break;
		case 5 :
			show_small_pics(SUBPAGE_5_BLOCK,109,109);
			break;
		case 6 :
			show_small_pics(SUBPAGE_6_BLOCK,109,109);
			break;
		case 7 :
			show_small_pics(SUBPAGE_7_BLOCK,109,109);
			break;
		case 8 :
			show_small_pics(SUBPAGE_8_BLOCK,109,109);
			break;
		case 9 :
			show_small_pics(SUBPAGE_9_BLOCK,109,109);
			break;
		default :
			break ;	
		}
		global_motor_odo_hundreds_digit_buf=global_motor_odo_hundreds_digit;
	}

	//�ODO��
	if(global_motor_odo_tens_digit_buf==-1 || global_motor_odo_tens_digit_buf!=global_motor_odo_tens_digit )
	{
		switch (global_motor_odo_tens_digit){
		case 0 :
			show_small_pics(SUBPAGE_0_BLOCK,136,109);
			break ;
		case 1 :
			show_small_pics(SUBPAGE_1_BLOCK,136,109);
			break ;
		case 2 :
			show_small_pics(SUBPAGE_2_BLOCK,136,109);
			break ;
		case 3 :
			show_small_pics(SUBPAGE_3_BLOCK,136,109);
			break ;
		case 4 :
			show_small_pics(SUBPAGE_4_BLOCK,136,109);
			break;
		case 5 :
			show_small_pics(SUBPAGE_5_BLOCK,136,109);
			break;
		case 6 :
			show_small_pics(SUBPAGE_6_BLOCK,136,109);
			break;
		case 7 :
			show_small_pics(SUBPAGE_7_BLOCK,136,109);
			break;
		case 8 :
			show_small_pics(SUBPAGE_8_BLOCK,136,109);
			break;
		case 9 :
			show_small_pics(SUBPAGE_9_BLOCK,136,109);
			break;
		default :
			break ;	
		}
		global_motor_odo_tens_digit_buf=global_motor_odo_tens_digit;
	}

	//�ODO��
	if(global_motor_odo_units_digit_buf==-1 || global_motor_odo_units_digit_buf!=global_motor_odo_units_digit )
	{
		switch (global_motor_odo_units_digit){
		case 0 :
			show_small_pics(SUBPAGE_0_BLOCK,163,109);
			break ;
		case 1 :
			show_small_pics(SUBPAGE_1_BLOCK,163,109);
			break ;
		case 2 :
			show_small_pics(SUBPAGE_2_BLOCK,163,109);
			break ;
		case 3 :
			show_small_pics(SUBPAGE_3_BLOCK,163,109);
			break ;
		case 4 :
			show_small_pics(SUBPAGE_4_BLOCK,163,109);
			break;
		case 5 :
			show_small_pics(SUBPAGE_5_BLOCK,163,109);
			break;
		case 6 :
			show_small_pics(SUBPAGE_6_BLOCK,163,109);
			break;
		case 7 :
			show_small_pics(SUBPAGE_7_BLOCK,163,109);
			break;
		case 8 :
			show_small_pics(SUBPAGE_8_BLOCK,163,109);
			break;
		case 9 :
			show_small_pics(SUBPAGE_9_BLOCK,163,109);
			break;
		default :
			break ;	
		}
		global_motor_odo_units_digit_buf=global_motor_odo_units_digit;
	}

	if(global_motor_trip_buf==-1 || global_motor_trip_buf!=global_motor_trip )
	{
		show_small_pics(IC_MASK_GRAY_BLOCK,212,109);
		if(global_motor_trip >=100)
		{	
			//�百位數��
			switch (global_motor_trip_hundred_digit){
				case 0 :
					show_small_pics(SUBPAGE_RIGHT_0_BLOCK,214,109);
					break ;
				case 1 :
					show_small_pics(SUBPAGE_RIGHT_1_BLOCK,214,109);
					break ;
				case 2 :
					show_small_pics(SUBPAGE_RIGHT_2_BLOCK,214,109);
					break ;
				case 3 :
					show_small_pics(SUBPAGE_RIGHT_3_BLOCK,214,109);
					break ;
				case 4 :
					show_small_pics(SUBPAGE_RIGHT_4_BLOCK,214,109);
					break;
				case 5 :
					show_small_pics(SUBPAGE_RIGHT_5_BLOCK,214,109);
					break;
				case 6 :
					show_small_pics(SUBPAGE_RIGHT_6_BLOCK,214,109);
					break;
				case 7 :
					show_small_pics(SUBPAGE_RIGHT_7_BLOCK,214,109);
					break;
				case 8 :
					show_small_pics(SUBPAGE_RIGHT_8_BLOCK,214,109);
					break;
				case 9 :
					show_small_pics(SUBPAGE_RIGHT_9_BLOCK,214,109);
					break;
				default :
					break ;	
				}
			//��位數��
			switch (global_motor_trip_tens_digit){
				case 0 :
					show_small_pics(SUBPAGE_RIGHT_0_BLOCK,241,109);
					break ;
				case 1 :
					show_small_pics(SUBPAGE_RIGHT_1_BLOCK,241,109);
					break ;
				case 2 :
					show_small_pics(SUBPAGE_RIGHT_2_BLOCK,241,109);
					break ;
				case 3 :
					show_small_pics(SUBPAGE_RIGHT_3_BLOCK,241,109);
					break ;
				case 4 :
					show_small_pics(SUBPAGE_RIGHT_4_BLOCK,241,109);
					break;
				case 5 :
					show_small_pics(SUBPAGE_RIGHT_5_BLOCK,241,109);
					break;
				case 6 :
					show_small_pics(SUBPAGE_RIGHT_6_BLOCK,241,109);
					break;
				case 7 :
					show_small_pics(SUBPAGE_RIGHT_7_BLOCK,241,109);
					break;
				case 8 :
					show_small_pics(SUBPAGE_RIGHT_8_BLOCK,241,109);
					break;
				case 9 :
					show_small_pics(SUBPAGE_RIGHT_9_BLOCK,241,109);
					break;
				default :
					break ;	
				}
			//�個���
			switch (global_motor_trip_units_digit){
				case 0 :
					show_small_pics(SUBPAGE_RIGHT_0_BLOCK,268,109);
					break ;
				case 1 :
					show_small_pics(SUBPAGE_RIGHT_1_BLOCK,268,109);
					break ;
				case 2 :
					show_small_pics(SUBPAGE_RIGHT_2_BLOCK,268,109);
					break ;
				case 3 :
					show_small_pics(SUBPAGE_RIGHT_3_BLOCK,268,109);
					break ;
				case 4 :
					show_small_pics(SUBPAGE_RIGHT_4_BLOCK,268,109);
					break;
				case 5 :
					show_small_pics(SUBPAGE_RIGHT_5_BLOCK,268,109);
					break;
				case 6 :
					show_small_pics(SUBPAGE_RIGHT_6_BLOCK,268,109);
					break;
				case 7 :
					show_small_pics(SUBPAGE_RIGHT_7_BLOCK,268,109);
					break;
				case 8 :
					show_small_pics(SUBPAGE_RIGHT_8_BLOCK,268,109);
					break;
				case 9 :
					show_small_pics(SUBPAGE_RIGHT_9_BLOCK,268,109);
					break;
				default :
					break ;	
				}
		}
		else if((global_motor_trip>=10)&&(global_motor_trip<100))
		{
			//��位數��
			switch (global_motor_trip_tens_digit){
				case 0 :
					show_small_pics(SUBPAGE_RIGHT_0_BLOCK,225,109);
					break ;
				case 1 :
					show_small_pics(SUBPAGE_RIGHT_1_BLOCK,225,109);
					break ;
				case 2 :
					show_small_pics(SUBPAGE_RIGHT_2_BLOCK,225,109);
					break ;
				case 3 :
					show_small_pics(SUBPAGE_RIGHT_3_BLOCK,225,109);
					break ;
				case 4 :
					show_small_pics(SUBPAGE_RIGHT_4_BLOCK,225,109);
					break;
				case 5 :
					show_small_pics(SUBPAGE_RIGHT_5_BLOCK,225,109);
					break;
				case 6 :
					show_small_pics(SUBPAGE_RIGHT_6_BLOCK,225,109);
					break;
				case 7 :
					show_small_pics(SUBPAGE_RIGHT_7_BLOCK,225,109);
					break;
				case 8 :
					show_small_pics(SUBPAGE_RIGHT_8_BLOCK,225,109);
					break;
				case 9 :
					show_small_pics(SUBPAGE_RIGHT_9_BLOCK,225,109);
					break;
				default :
					break ;	
				}
			//�個���
			switch (global_motor_trip_units_digit){
				case 0 :
					show_small_pics(SUBPAGE_RIGHT_0_BLOCK,252,109);
					break ;
				case 1 :
					show_small_pics(SUBPAGE_RIGHT_1_BLOCK,252,109);
					break ;
				case 2 :
					show_small_pics(SUBPAGE_RIGHT_2_BLOCK,252,109);
					break ;
				case 3 :
					show_small_pics(SUBPAGE_RIGHT_3_BLOCK,252,109);
					break ;
				case 4 :
					show_small_pics(SUBPAGE_RIGHT_4_BLOCK,252,109);
					break;
				case 5 :
					show_small_pics(SUBPAGE_RIGHT_5_BLOCK,252,109);
					break;
				case 6 :
					show_small_pics(SUBPAGE_RIGHT_6_BLOCK,252,109);
					break;
				case 7 :
					show_small_pics(SUBPAGE_RIGHT_7_BLOCK,252,109);
					break;
				case 8 :
					show_small_pics(SUBPAGE_RIGHT_8_BLOCK,252,109);
					break;
				case 9 :
					show_small_pics(SUBPAGE_RIGHT_9_BLOCK,252,109);
					break;
				default :
					break ;	
				}
		}		
		else
		{
						//�個���
			switch (global_motor_trip_units_digit){
				case 0 :
					show_small_pics(SUBPAGE_RIGHT_0_BLOCK,238,109);
					break ;
				case 1 :
					show_small_pics(SUBPAGE_RIGHT_1_BLOCK,238,109);
					break ;
				case 2 :
					show_small_pics(SUBPAGE_RIGHT_2_BLOCK,238,109);
					break ;
				case 3 :
					show_small_pics(SUBPAGE_RIGHT_3_BLOCK,238,109);
					break ;
				case 4 :
					show_small_pics(SUBPAGE_RIGHT_4_BLOCK,238,109);
					break;
				case 5 :
					show_small_pics(SUBPAGE_RIGHT_5_BLOCK,238,109);
					break;
				case 6 :
					show_small_pics(SUBPAGE_RIGHT_6_BLOCK,238,109);
					break;
				case 7 :
					show_small_pics(SUBPAGE_RIGHT_7_BLOCK,238,109);
					break;
				case 8 :
					show_small_pics(SUBPAGE_RIGHT_8_BLOCK,238,109);
					break;
				case 9 :
					show_small_pics(SUBPAGE_RIGHT_9_BLOCK,238,109);
					break;
				default :
					break ;	
				}			
		}	
		global_motor_trip_buf=global_motor_trip; 		
	}	
}


/* 顯示Battery_Status�面資�
 * �數 : void
 * �傳 : void
*/

void battery_status_show(void)
{
	flag_button_lock = false;
	if(!global_enter_status_pics)
	{
		show_small_pics(BG_MAINPAGE_STATUS_BLOCK,60,0);
		global_enter_status_pics=true;
	}
	//�Assist Level 底�����������字�
	if(battery_status_global_motor_assist_level_buf==-1 || battery_status_global_motor_assist_level_buf!=global_motor_assist_level )
	{		
		if(battery_status_global_page_index_buf==-1 || battery_status_global_page_index_buf!=global_page_index )
		{
			switch (global_motor_assist_level){
			case 0 :
				show_small_pics(BG_MAIN_SUB_L0123_1_BLOCK,0,0);
				show_small_pics(BG_SUB12_2_BLOCK,260,0);
				show_big_pics(BG_BATTERY_STATUS_L0_BLOCK,0,44);
				show_small_pics(IC_BATTERY_TITLE_L0_BLOCK,115,64);
				show_small_pics(IC_BATTERY1_NAME_BLOCK,81,99);				
				show_small_pics(IC_BATTERY2_NAME_BLOCK,217,99);
				break ;
			case 1 :
				show_small_pics(BG_MAIN_SUB_L0123_1_BLOCK,0,0);
				show_small_pics(BG_SUB12_2_BLOCK,260,0);
				show_big_pics(BG_BATTERY_STATUS_L1_BLOCK,0,44);
				show_small_pics(IC_BATTERY_TITLE_L1_BLOCK,115,64);
				show_small_pics(IC_BATTERY1_NAME_BLOCK,81,99);				
				show_small_pics(IC_BATTERY2_NAME_BLOCK,217,99);
				break ;
			case 2 :
				show_small_pics(BG_MAIN_SUB_L0123_1_BLOCK,0,0);
				show_small_pics(BG_SUB12_2_BLOCK,260,0);
				show_big_pics(BG_BATTERY_STATUS_L2_BLOCK,0,44);
				show_small_pics(IC_BATTERY_TITLE_L2_BLOCK,115,64);
				show_small_pics(IC_BATTERY1_NAME_BLOCK,81,99);				
				show_small_pics(IC_BATTERY2_NAME_BLOCK,217,99);
				break ;
			case 3 :
				show_small_pics(BG_MAIN_SUB_L0123_1_BLOCK,0,0);
				show_small_pics(BG_SUB12_2_BLOCK,260,0);
				show_big_pics(BG_BATTERY_STATUS_L3_BLOCK,0,44);
				show_small_pics(IC_BATTERY_TITLE_L3_BLOCK,115,64);
				show_small_pics(IC_BATTERY1_NAME_BLOCK,81,99);				
				show_small_pics(IC_BATTERY2_NAME_BLOCK,217,99);
				break ;
			case 4 :
				show_small_pics(BG_MAIN_SUB_LEVEL4_1_BLOCK,0,0);
				show_small_pics(BG_SUB12_2_BLOCK,260,0);
				show_big_pics(BG_BATTERY_STATUS_L4_BLOCK,0,44);
				show_small_pics(IC_BATTERY_TITLE_L4_BLOCK,115,64);
				show_small_pics(IC_BATTERY1_NAME_BLOCK,81,99);				
				show_small_pics(IC_BATTERY2_NAME_BLOCK,217,99);
				break;
			default :
				break ;	
			}
			battery_status_global_page_index_buf=global_page_index;
		}
		else
		{
			switch (global_motor_assist_level){
			case 0 :
				show_small_pics(BG_SUBPAGE_LEVEL_0_BLOCK,0,0);
				show_small_pics(BG_SUBPAGE_1_LEVEL_0_BLOCK,52,0);
				show_small_pics(BG_SUBPAGE_2_LEVEL_0_BLOCK,52,180);
				show_small_pics(BG_SUBPAGE_3_LEVEL_0_BLOCK,276,0);
				show_small_pics(BG_SUBPAGE_4_LEVEL_0_BLOCK,300,64);
				show_small_pics(BG_SUBPAGE_5_LEVEL_0_BLOCK,284,172);
				show_small_pics(IC_BATTERY_TITLE_L0_BLOCK,115,64);
				break ;
			case 1 :
				show_small_pics(BG_SUBPAGE_LEVEL_1_BLOCK,0,0);
				show_small_pics(BG_SUBPAGE_1_LEVEL_1_BLOCK,52,0);
				show_small_pics(BG_SUBPAGE_2_LEVEL_1_BLOCK,52,180);
				show_small_pics(BG_SUBPAGE_3_LEVEL_1_BLOCK,276,0);
				show_small_pics(BG_SUBPAGE_4_LEVEL_1_BLOCK,300,64);
				show_small_pics(BG_SUBPAGE_5_LEVEL_1_BLOCK,284,172);
				show_small_pics(IC_BATTERY_TITLE_L1_BLOCK,115,64);
				break ;
			case 2 :
				show_small_pics(BG_SUBPAGE_LEVEL_2_BLOCK,0,0);
				show_small_pics(BG_SUBPAGE_1_LEVEL_2_BLOCK,52,0);
				show_small_pics(BG_SUBPAGE_2_LEVEL_2_BLOCK,52,180);
				show_small_pics(BG_SUBPAGE_3_LEVEL_2_BLOCK,276,0);
				show_small_pics(BG_SUBPAGE_4_LEVEL_2_BLOCK,300,64);
				show_small_pics(BG_SUBPAGE_5_LEVEL_2_BLOCK,284,172);
				show_small_pics(IC_BATTERY_TITLE_L2_BLOCK,115,64);
				break ;
			case 3 :
				show_small_pics(BG_SUBPAGE_LEVEL_3_BLOCK,0,0);
				show_small_pics(BG_SUBPAGE_1_LEVEL_3_BLOCK,52,0);
				show_small_pics(BG_SUBPAGE_2_LEVEL_3_BLOCK,52,180);
				show_small_pics(BG_SUBPAGE_3_LEVEL_3_BLOCK,276,0);
				show_small_pics(BG_SUBPAGE_4_LEVEL_3_BLOCK,300,64);
				show_small_pics(BG_SUBPAGE_5_LEVEL_3_BLOCK,284,172);
				show_small_pics(IC_BATTERY_TITLE_L3_BLOCK,115,64);
				break ;
			case 4 :
				show_small_pics(BG_SUBPAGE_LEVEL_4_BLOCK,0,0);
				show_small_pics(BG_SUBPAGE_1_LEVEL_4_BLOCK,52,0);
				show_small_pics(BG_SUBPAGE_2_LEVEL_4_BLOCK,52,180);
				show_small_pics(BG_SUBPAGE_3_LEVEL_4_BLOCK,276,0);
				show_small_pics(BG_SUBPAGE_4_LEVEL_4_BLOCK,300,64);
				show_small_pics(BG_SUBPAGE_5_LEVEL_4_BLOCK,284,172);
				show_small_pics(IC_BATTERY_TITLE_L4_BLOCK,115,64);
				break;
			default :
				break ;	
			}
		
		}
		battery_status_global_motor_assist_level_buf=global_motor_assist_level;
	}
	//�腳踏�大�
	if(global_ebike_light_buf==-1 || global_ebike_light_buf!=global_ebike_light )
	{
		switch (global_ebike_light){
		case 0 :
			show_small_pics(IC_LIGHT_CLOSE_BLOCK,72,16);
			break ;
		case 1 :
			show_small_pics(IC_LIGHT_BLOCK,72,16);
			break ;
		default :
			break ;	
		}
		global_ebike_light_buf=global_ebike_light;
	}

	//�腳踏��芽
	if(global_ebike_bluetooth_buf==-1 || global_ebike_bluetooth_buf!=global_ebike_bluetooth )
	{
		switch (global_ebike_bluetooth){
		case 0 :
			show_small_pics(IC_BLUETOOTH_DISABLE_BLOCK,220,16);
			break ;
		case 1 :
			show_small_pics(IC_BLUETOOTH_BLOCK,220,16);
			break ;
		default :
			break ;	
		}
		global_ebike_bluetooth_buf=global_ebike_bluetooth;
	}
	
	if((g_ble_peripheral_state.bits.bonded >= 1 )&&(g_ble_peripheral_state.bits.connected >=1))
	{		
		show_time_flag = true;		
	}			
		
	if(show_time_flag)
	{
		show_time();
	}
			
	//�MAIN_BMS��
	if(global_main_bms_soc_buf==-1 || global_main_bms_soc_buf!=global_main_bms_soc )
	{
		show_small_pics(IC_MASK_DARK_GRAY_WIDE_BLOCK,55,137);
		if(global_main_bms_soc==100)
		{								
			show_small_pics(SUBPAGE_PERCENT_DARK_GRAY_BLOCK,137,149);
			show_small_pics(SUBPAGE_1_BLOCK,56,137);//��
			show_small_pics(SUBPAGE_0_BLOCK,83,137);//��
			show_small_pics(SUBPAGE_0_BLOCK,110,137);//��
		}
		else if((global_main_bms_soc>=10)&&(global_main_bms_soc<100))
		{
			show_small_pics(SUBPAGE_PERCENT_DARK_GRAY_BLOCK,124,149);
			//�SOC���數��
			switch (global_main_bms_soc_tens_digit){
				case 1 :
					show_small_pics(SUBPAGE_1_BLOCK,70,137);
					break ;
				case 2 :
					show_small_pics(SUBPAGE_2_BLOCK,70,137);
					break ;
				case 3 :
					show_small_pics(SUBPAGE_3_BLOCK,70,137);
					break ;
				case 4 :
					show_small_pics(SUBPAGE_4_BLOCK,70,137);
					break;
				case 5 :
					show_small_pics(SUBPAGE_5_BLOCK,70,137);
					break;
				case 6 :
					show_small_pics(SUBPAGE_6_BLOCK,70,137);
					break;
				case 7 :
					show_small_pics(SUBPAGE_7_BLOCK,70,137);
					break;
				case 8 :
					show_small_pics(SUBPAGE_8_BLOCK,70,137);
					break;
				case 9 :
					show_small_pics(SUBPAGE_9_BLOCK,70,137);
					break;
				default :
					break ;	
				}
			//�SOC���數��
			switch (global_main_bms_soc_units_digit){
				case 0 :
					show_small_pics(SUBPAGE_0_BLOCK,97,137);
					break ;
				case 1 :
					show_small_pics(SUBPAGE_1_BLOCK,97,137);
					break ;
				case 2 :
					show_small_pics(SUBPAGE_2_BLOCK,97,137);
					break ;
				case 3 :
					show_small_pics(SUBPAGE_3_BLOCK,97,137);
					break ;
				case 4 :
					show_small_pics(SUBPAGE_4_BLOCK,97,137);
					break;
				case 5 :
					show_small_pics(SUBPAGE_5_BLOCK,97,137);
					break;
				case 6 :
					show_small_pics(SUBPAGE_6_BLOCK,97,137);
					break;
				case 7 :
					show_small_pics(SUBPAGE_7_BLOCK,97,137);
					break;
				case 8 :
					show_small_pics(SUBPAGE_8_BLOCK,97,137);
					break;
				case 9 :
					show_small_pics(SUBPAGE_9_BLOCK,97,137);
					break;
				default :
					break ;	
				}
		}		
		else
		{
				show_small_pics(SUBPAGE_PERCENT_DARK_GRAY_BLOCK,110,149);
				//�SOC��
				switch (global_main_bms_soc_units_digit){
				case 0 :
					show_small_pics(SUBPAGE_0_BLOCK,83,137);
					break ;
				case 1 :
					show_small_pics(SUBPAGE_1_BLOCK,83,137);
					break ;
				case 2 :
					show_small_pics(SUBPAGE_2_BLOCK,83,137);
					break ;
				case 3 :
					show_small_pics(SUBPAGE_3_BLOCK,83,137);
					break ;
				case 4 :
					show_small_pics(SUBPAGE_4_BLOCK,83,137);
					break;
				case 5 :
					show_small_pics(SUBPAGE_5_BLOCK,83,137);
					break;
				case 6 :
					show_small_pics(SUBPAGE_6_BLOCK,83,137);
					break;
				case 7 :
					show_small_pics(SUBPAGE_7_BLOCK,83,137);
					break;
				case 8 :
					show_small_pics(SUBPAGE_8_BLOCK,83,137);
					break;
				case 9 :
					show_small_pics(SUBPAGE_9_BLOCK,83,137);
					break;
				default :
					break ;	
				}			
		}	
		global_main_bms_soc_buf=global_main_bms_soc; 		
	}
	//�EX_BMS��
	if(global_battery_number!=1)
	{
		extra_battery_connected = false;
		if(global_ex_bms_soc_buf==-1 || global_ex_bms_soc_buf!=global_ex_bms_soc )
		{
			show_small_pics(IC_MASK_GRAY_WIDE_BLOCK,191,137);
			if(global_ex_bms_soc==100)
			{	
				show_small_pics(SUBPAGE_PERCENT_GRAY_BLOCK,273,149);
				show_small_pics(SUBPAGE_RIGHT_1_BLOCK,192,137);//��
				show_small_pics(SUBPAGE_RIGHT_0_BLOCK,219,137);//��
				show_small_pics(SUBPAGE_RIGHT_0_BLOCK,246,137);//��
			}
			else if((global_ex_bms_soc>=10)&&(global_ex_bms_soc<100))
			{
				show_small_pics(SUBPAGE_PERCENT_GRAY_BLOCK,260,149);
				//�SOC���數��
				switch (global_ex_bms_soc_tens_digit){
					case 1 :
						show_small_pics(SUBPAGE_RIGHT_1_BLOCK,206,137);
						break ;
					case 2 :
						show_small_pics(SUBPAGE_RIGHT_2_BLOCK,206,137);
						break ;
					case 3 :
						show_small_pics(SUBPAGE_RIGHT_3_BLOCK,206,137);
						break ;
					case 4 :
						show_small_pics(SUBPAGE_RIGHT_4_BLOCK,206,137);
						break;
					case 5 :
						show_small_pics(SUBPAGE_RIGHT_5_BLOCK,206,137);
						break;
					case 6 :
						show_small_pics(SUBPAGE_RIGHT_6_BLOCK,206,137);
						break;
					case 7 :
						show_small_pics(SUBPAGE_RIGHT_7_BLOCK,206,137);
						break;
					case 8 :
						show_small_pics(SUBPAGE_RIGHT_8_BLOCK,206,137);
						break;
					case 9 :
						show_small_pics(SUBPAGE_RIGHT_9_BLOCK,206,137);
						break;
					default :
						break ;	
					}
				//�SOC���數��
				switch (global_ex_bms_soc_units_digit){
					case 0 :
						show_small_pics(SUBPAGE_RIGHT_0_BLOCK,233,137);
						break ;
					case 1 :
						show_small_pics(SUBPAGE_RIGHT_1_BLOCK,233,137);
						break ;
					case 2 :
						show_small_pics(SUBPAGE_RIGHT_2_BLOCK,233,137);
						break ;
					case 3 :
						show_small_pics(SUBPAGE_RIGHT_3_BLOCK,233,137);
						break ;
					case 4 :
						show_small_pics(SUBPAGE_RIGHT_4_BLOCK,233,137);
						break;
					case 5 :
						show_small_pics(SUBPAGE_RIGHT_5_BLOCK,233,137);
						break;
					case 6 :
						show_small_pics(SUBPAGE_RIGHT_6_BLOCK,233,137);
						break;
					case 7 :
						show_small_pics(SUBPAGE_RIGHT_7_BLOCK,233,137);
						break;
					case 8 :
						show_small_pics(SUBPAGE_RIGHT_8_BLOCK,233,137);
						break;
					case 9 :
						show_small_pics(SUBPAGE_RIGHT_9_BLOCK,233,137);
						break;
					default :
						break ;	
					}
			}		
			else
			{
					show_small_pics(SUBPAGE_PERCENT_GRAY_BLOCK,246,149);
					//�SOC��
					switch (global_ex_bms_soc_units_digit){
					case 0 :
						show_small_pics(SUBPAGE_RIGHT_0_BLOCK,219,137);
						break ;
					case 1 :
						show_small_pics(SUBPAGE_RIGHT_1_BLOCK,219,137);
						break ;
					case 2 :
						show_small_pics(SUBPAGE_RIGHT_2_BLOCK,219,137);
						break ;
					case 3 :
						show_small_pics(SUBPAGE_RIGHT_3_BLOCK,219,137);
						break ;
					case 4 :
						show_small_pics(SUBPAGE_RIGHT_4_BLOCK,219,137);
						break;
					case 5 :
						show_small_pics(SUBPAGE_RIGHT_5_BLOCK,219,137);
						break;
					case 6 :
						show_small_pics(SUBPAGE_RIGHT_6_BLOCK,219,137);
						break;
					case 7 :
						show_small_pics(SUBPAGE_RIGHT_7_BLOCK,219,137);
						break;
					case 8 :
						show_small_pics(SUBPAGE_RIGHT_8_BLOCK,219,137);
						break;
					case 9 :
						show_small_pics(SUBPAGE_RIGHT_9_BLOCK,219,137);
						break;
					default :
						break ;	
					}			
			}	
			global_ex_bms_soc_buf=global_ex_bms_soc; 		
		}
	}
	else
	{
		global_ex_bms_soc_buf=-1;
		if(!extra_battery_connected)
		{
			show_small_pics(IC_MASK_GRAY_WIDE_BLOCK,191,137);
			show_small_pics(IC_NO_EXTRA_BATTERY_BLOCK,223,147);
			extra_battery_connected = true;
		}
	}			
}

void show_walking_mode_page(void)
{
	if(!global_enter_status_pics)
	{
		show_small_pics(BG_MAINPAGE_STATUS_BLOCK,60,0);
		global_enter_status_pics=true;
	}
	if(!global_enter_walking)
	{
		show_small_pics(BG_WLPE_LEFT_BLOCK,0,0);
		show_small_pics(BG_WLPE_RIGHT_BLOCK,260,0);
		show_big_pics(BG_MAINPAGE_BLOCK,60,44);
		show_small_pics(IC_TITLE_WALKMODE_BLOCK,116,60);
		global_enter_walking = true;
	}

	//�腳踏�大�
	if(global_ebike_light_buf==-1 || global_ebike_light_buf!=global_ebike_light )
	{
		switch (global_ebike_light){
		case 0 :
			show_small_pics(IC_LIGHT_CLOSE_BLOCK,72,16);
			break ;
		case 1 :
			show_small_pics(IC_LIGHT_BLOCK,72,16);
			break ;
		default :
			break ;	
		}
		global_ebike_light_buf=global_ebike_light;
	}

	//�腳踏��芽
	if(global_ebike_bluetooth_buf==-1 || global_ebike_bluetooth_buf!=global_ebike_bluetooth )
	{
		switch (global_ebike_bluetooth){
		case 0 :
			show_small_pics(IC_BLUETOOTH_DISABLE_BLOCK,220,16);
			break ;
		case 1 :
			show_small_pics(IC_BLUETOOTH_BLOCK,220,16);
			break ;
		default :
			break ;	
		}
		global_ebike_bluetooth_buf=global_ebike_bluetooth;
	}		
	show_walking_mode();
}



/* �放 Charging �面
 * �數 : void
 * �傳 : void
 */

void show_battery_charging(void)
{
	flag_button_lock = true;
	if(!global_enter_status_pics)
	{
		show_small_pics(BG_MAINPAGE_STATUS_BLOCK,60,0);
		global_enter_status_pics=true;
	}
	if(!global_enter_charging)
	{
		show_big_pics(BG_MAINPAGE_BLOCK,60,44);
		show_small_pics(IC_TITLE_CHARGING_BLOCK,96,60);
		show_small_pics(SUBPAGE_PERCENT_BLACK_BLOCK,189,121);
		global_enter_charging = true;
	}
	//�電池個數
	if(global_battery_number_buf==-1 || global_battery_number_buf!=global_battery_number )
	{
		switch (global_battery_number){
		case 1 :
			global_main_bms_soc_level_buf=-1;
			show_small_pics(BG_CHARGING_LEFT_ON_BLOCK,0,0);
			show_small_pics(BG_CHARGING_RIGHT_OFF_BLOCK,260,0);
			break ;
		case 2 :
			global_main_bms_soc_level_buf=-1;
			global_ex_bms_soc_level_buf=-1; 
			show_small_pics(BG_CHARGING_LEFT_ON_BLOCK,0,0);
			show_small_pics(BG_CHARGING_RIGHT_ON_BLOCK,260,0);
			break ;
		default :
			break ;	
		}
		global_battery_number_buf=global_battery_number;
	}
	//�腳踏�大�
	if(global_ebike_light_buf==-1 || global_ebike_light_buf!=global_ebike_light )
	{
		switch (global_ebike_light){
		case 0 :
			show_small_pics(IC_LIGHT_CLOSE_BLOCK,72,16);
			break ;
		case 1 :
			show_small_pics(IC_LIGHT_BLOCK,72,16);
			break ;
		default :
			break ;	
		}
		global_ebike_light_buf=global_ebike_light;
	}

	//�腳踏��芽
	if(global_ebike_bluetooth_buf==-1 || global_ebike_bluetooth_buf!=global_ebike_bluetooth )
	{
		switch (global_ebike_bluetooth){
		case 0 :
			show_small_pics(IC_BLUETOOTH_DISABLE_BLOCK,220,16);
			break ;
		case 1 :
			show_small_pics(IC_BLUETOOTH_BLOCK,220,16);
			break ;
		default :
			break ;	
		}
		global_ebike_bluetooth_buf=global_ebike_bluetooth;
	}
	
	if((g_ble_peripheral_state.bits.bonded >= 1 )&&(g_ble_peripheral_state.bits.connected >=1))
	{		
		show_time_flag = true;		
	}			
		
	if(show_time_flag)
	{
		show_time();
	}
		
	if(global_charging_bms_soc_buf==-1 || global_charging_bms_soc_buf!=global_bms_soc_new )
	{
		show_small_pics(IC_MASK_BLACK_BLOCK,106,109);
		if(global_bms_soc_new==100)
		{	
			show_small_pics(CHARGING_1_BLOCK,108,109);//��
			show_small_pics(CHARGING_0_BLOCK,135,109);//��
			show_small_pics(CHARGING_0_BLOCK,162,109);//��
		}
		else if((global_bms_soc_new>=10)&&(global_bms_soc_new<100))
		{
			//�SOC���數��
			switch (global_bms_soc_tens_digit){
				case 1 :
					show_small_pics(CHARGING_1_BLOCK,135,109);
					break ;
				case 2 :
					show_small_pics(CHARGING_2_BLOCK,135,109);
					break ;
				case 3 :
					show_small_pics(CHARGING_3_BLOCK,135,109);
					break ;
				case 4 :
					show_small_pics(CHARGING_4_BLOCK,135,109);
					break;
				case 5 :
					show_small_pics(CHARGING_5_BLOCK,135,109);
					break;
				case 6 :
					show_small_pics(CHARGING_6_BLOCK,135,109);
					break;
				case 7 :
					show_small_pics(CHARGING_7_BLOCK,135,109);
					break;
				case 8 :
					show_small_pics(CHARGING_8_BLOCK,135,109);
					break;
				case 9 :
					show_small_pics(CHARGING_9_BLOCK,135,109);
					break;
				default :
					break ;	
				}
			//�SOC���數��
			switch (global_bms_soc_units_digit){
				case 0 :
					show_small_pics(CHARGING_0_BLOCK,162,109);
					break ;
				case 1 :
					show_small_pics(CHARGING_1_BLOCK,162,109);
					break ;
				case 2 :
					show_small_pics(CHARGING_2_BLOCK,162,109);
					break ;
				case 3 :
					show_small_pics(CHARGING_3_BLOCK,162,109);
					break ;
				case 4 :
					show_small_pics(CHARGING_4_BLOCK,162,109);
					break;
				case 5 :
					show_small_pics(CHARGING_5_BLOCK,162,109);
					break;
				case 6 :
					show_small_pics(CHARGING_6_BLOCK,162,109);
					break;
				case 7 :
					show_small_pics(CHARGING_7_BLOCK,162,109);
					break;
				case 8 :
					show_small_pics(CHARGING_8_BLOCK,162,109);
					break;
				case 9 :
					show_small_pics(CHARGING_9_BLOCK,162,109);
					break;
				default :
					break ;	
				}
		}		
		else
		{
				//�SOC��
				switch (global_bms_soc_units_digit){
				case 0 :
					show_small_pics(CHARGING_0_BLOCK,162,109);
					break ;
				case 1 :
					show_small_pics(CHARGING_1_BLOCK,162,109);
					break ;
				case 2 :
					show_small_pics(CHARGING_2_BLOCK,162,109);
					break ;
				case 3 :
					show_small_pics(CHARGING_3_BLOCK,162,109);
					break ;
				case 4 :
					show_small_pics(CHARGING_4_BLOCK,162,109);
					break;
				case 5 :
					show_small_pics(CHARGING_5_BLOCK,162,109);
					break;
				case 6 :
					show_small_pics(CHARGING_6_BLOCK,162,109);
					break;
				case 7 :
					show_small_pics(CHARGING_7_BLOCK,162,109);
					break;
				case 8 :
					show_small_pics(CHARGING_8_BLOCK,162,109);
					break;
				case 9 :
					show_small_pics(CHARGING_9_BLOCK,162,109);
					break;
				default :
					break ;	
				}			
		}	
		global_charging_bms_soc_buf=global_bms_soc_new; 		
	}
	//�電池格
	if(global_charging_bms_soc_level_buf==-1 || global_charging_bms_soc_level_buf!=global_bms_soc_level )
	{
		switch (global_bms_soc_level){
		case 0 :
			show_small_pics(IC_CHARGING_0_BLOCK,56,150);
			break ;
		case 1 :
			show_small_pics(IC_CHARGING_1_BLOCK,56,150);
			break ;
	
		case 2 :
			show_small_pics(IC_CHARGING_2_BLOCK,56,150);
			break ;
		case 3 :
			show_small_pics(IC_CHARGING_3_BLOCK,56,150);
			break ;
		case 4 :
			show_small_pics(IC_CHARGING_4_BLOCK,56,150);
			break;
		case 5 :
			show_small_pics(IC_CHARGING_5_BLOCK,56,150);
			break;
		case 6 :
			show_small_pics(IC_CHARGING_6_BLOCK,56,150);
			break;
		case 7 :
			show_small_pics(IC_CHARGING_7_BLOCK,56,150);
			break;
		case 8 :
			show_small_pics(IC_CHARGING_8_BLOCK,56,150);
			break;
		default :
			break ;	
		}
		global_charging_bms_soc_level_buf=global_bms_soc_level;
	}
	//�主�電池��格
	if(global_battery_number==2)
	{
		//�電池格
		if(global_main_bms_soc_level_buf==-1 || global_main_bms_soc_level_buf!=global_main_bms_soc_level )
		{
			switch (global_main_bms_soc_level){
			case 0 :
				break ;
			case 1 :
				show_small_pics(IC_CHARGING_LEFT_1_BLOCK,0,204);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_2A_BLOCK,0,192);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_2B_BLOCK,0,188);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_2C_BLOCK,0,184);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_2D_BLOCK,0,180);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_2E_BLOCK,0,176);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_3_BLOCK,0,148);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_4_BLOCK,0,120);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_5_BLOCK,0,92);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_6_BLOCK,0,64);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7A_BLOCK,0,60);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7B_BLOCK,0,56);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7C_BLOCK,0,52);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7D_BLOCK,0,48);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7E_BLOCK,0,36);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_8_BLOCK,0,8);
				break ;
	
			case 2 :
				show_small_pics(IC_CHARGING_LEFT_1_BLOCK,0,204);
				show_small_pics(IC_CHARGING_LEFT_2A_BLOCK,0,192);
				show_small_pics(IC_CHARGING_LEFT_2B_BLOCK,0,188);
				show_small_pics(IC_CHARGING_LEFT_2C_BLOCK,0,184);
				show_small_pics(IC_CHARGING_LEFT_2D_BLOCK,0,180);
				show_small_pics(IC_CHARGING_LEFT_2E_BLOCK,0,176);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_3_BLOCK,0,148);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_4_BLOCK,0,120);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_5_BLOCK,0,92);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_6_BLOCK,0,64);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7A_BLOCK,0,60);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7B_BLOCK,0,56);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7C_BLOCK,0,52);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7D_BLOCK,0,48);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7E_BLOCK,0,36);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_8_BLOCK,0,8);
				break ;
			case 3 :
				show_small_pics(IC_CHARGING_LEFT_1_BLOCK,0,204);
				show_small_pics(IC_CHARGING_LEFT_2A_BLOCK,0,192);
				show_small_pics(IC_CHARGING_LEFT_2B_BLOCK,0,188);
				show_small_pics(IC_CHARGING_LEFT_2C_BLOCK,0,184);
				show_small_pics(IC_CHARGING_LEFT_2D_BLOCK,0,180);
				show_small_pics(IC_CHARGING_LEFT_2E_BLOCK,0,176);
				show_small_pics(IC_CHARGING_LEFT_3_BLOCK,0,148);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_4_BLOCK,0,120);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_5_BLOCK,0,92);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_6_BLOCK,0,64);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7A_BLOCK,0,60);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7B_BLOCK,0,56);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7C_BLOCK,0,52);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7D_BLOCK,0,48);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7E_BLOCK,0,36);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_8_BLOCK,0,8);
				break ;
			case 4 :
				show_small_pics(IC_CHARGING_LEFT_1_BLOCK,0,204);
				show_small_pics(IC_CHARGING_LEFT_2A_BLOCK,0,192);
				show_small_pics(IC_CHARGING_LEFT_2B_BLOCK,0,188);
				show_small_pics(IC_CHARGING_LEFT_2C_BLOCK,0,184);
				show_small_pics(IC_CHARGING_LEFT_2D_BLOCK,0,180);
				show_small_pics(IC_CHARGING_LEFT_2E_BLOCK,0,176);
				show_small_pics(IC_CHARGING_LEFT_3_BLOCK,0,148);
				show_small_pics(IC_CHARGING_LEFT_4_BLOCK,0,120);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_5_BLOCK,0,92);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_6_BLOCK,0,64);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7A_BLOCK,0,60);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7B_BLOCK,0,56);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7C_BLOCK,0,52);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7D_BLOCK,0,48);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7E_BLOCK,0,36);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_8_BLOCK,0,8);
				break;
			case 5 :
				show_small_pics(IC_CHARGING_LEFT_1_BLOCK,0,204);
				show_small_pics(IC_CHARGING_LEFT_2A_BLOCK,0,192);
				show_small_pics(IC_CHARGING_LEFT_2B_BLOCK,0,188);
				show_small_pics(IC_CHARGING_LEFT_2C_BLOCK,0,184);
				show_small_pics(IC_CHARGING_LEFT_2D_BLOCK,0,180);
				show_small_pics(IC_CHARGING_LEFT_2E_BLOCK,0,176);
				show_small_pics(IC_CHARGING_LEFT_3_BLOCK,0,148);
				show_small_pics(IC_CHARGING_LEFT_4_BLOCK,0,120);
				show_small_pics(IC_CHARGING_LEFT_5_BLOCK,0,92);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_6_BLOCK,0,64);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7A_BLOCK,0,60);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7B_BLOCK,0,56);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7C_BLOCK,0,52);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7D_BLOCK,0,48);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7E_BLOCK,0,36);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_8_BLOCK,0,8);
				break;
			case 6 :
				show_small_pics(IC_CHARGING_LEFT_1_BLOCK,0,204);
				show_small_pics(IC_CHARGING_LEFT_2A_BLOCK,0,192);
				show_small_pics(IC_CHARGING_LEFT_2B_BLOCK,0,188);
				show_small_pics(IC_CHARGING_LEFT_2C_BLOCK,0,184);
				show_small_pics(IC_CHARGING_LEFT_2D_BLOCK,0,180);
				show_small_pics(IC_CHARGING_LEFT_2E_BLOCK,0,176);
				show_small_pics(IC_CHARGING_LEFT_3_BLOCK,0,148);
				show_small_pics(IC_CHARGING_LEFT_4_BLOCK,0,120);
				show_small_pics(IC_CHARGING_LEFT_5_BLOCK,0,92);
				show_small_pics(IC_CHARGING_LEFT_6_BLOCK,0,64);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7A_BLOCK,0,60);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7B_BLOCK,0,56);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7C_BLOCK,0,52);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7D_BLOCK,0,48);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7E_BLOCK,0,36);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_8_BLOCK,0,8);
				break;
			case 7 :
				show_small_pics(IC_CHARGING_LEFT_1_BLOCK,0,204);
				show_small_pics(IC_CHARGING_LEFT_2A_BLOCK,0,192);
				show_small_pics(IC_CHARGING_LEFT_2B_BLOCK,0,188);
				show_small_pics(IC_CHARGING_LEFT_2C_BLOCK,0,184);
				show_small_pics(IC_CHARGING_LEFT_2D_BLOCK,0,180);
				show_small_pics(IC_CHARGING_LEFT_2E_BLOCK,0,176);
				show_small_pics(IC_CHARGING_LEFT_3_BLOCK,0,148);
				show_small_pics(IC_CHARGING_LEFT_4_BLOCK,0,120);
				show_small_pics(IC_CHARGING_LEFT_5_BLOCK,0,92);
				show_small_pics(IC_CHARGING_LEFT_6_BLOCK,0,64);
				show_small_pics(IC_CHARGING_LEFT_7A_BLOCK,0,60);
				show_small_pics(IC_CHARGING_LEFT_7B_BLOCK,0,56);
				show_small_pics(IC_CHARGING_LEFT_7C_BLOCK,0,52);
				show_small_pics(IC_CHARGING_LEFT_7D_BLOCK,0,48);
				show_small_pics(IC_CHARGING_LEFT_7E_BLOCK,0,36);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_8_BLOCK,0,8);
				break;
			case 8 :
				show_small_pics(IC_CHARGING_LEFT_1_BLOCK,0,204);
				show_small_pics(IC_CHARGING_LEFT_2A_BLOCK,0,192);
				show_small_pics(IC_CHARGING_LEFT_2B_BLOCK,0,188);
				show_small_pics(IC_CHARGING_LEFT_2C_BLOCK,0,184);
				show_small_pics(IC_CHARGING_LEFT_2D_BLOCK,0,180);
				show_small_pics(IC_CHARGING_LEFT_2E_BLOCK,0,176);
				show_small_pics(IC_CHARGING_LEFT_3_BLOCK,0,148);
				show_small_pics(IC_CHARGING_LEFT_4_BLOCK,0,120);
				show_small_pics(IC_CHARGING_LEFT_5_BLOCK,0,92);
				show_small_pics(IC_CHARGING_LEFT_6_BLOCK,0,64);
				show_small_pics(IC_CHARGING_LEFT_7A_BLOCK,0,60);
				show_small_pics(IC_CHARGING_LEFT_7B_BLOCK,0,56);
				show_small_pics(IC_CHARGING_LEFT_7C_BLOCK,0,52);
				show_small_pics(IC_CHARGING_LEFT_7D_BLOCK,0,48);
				show_small_pics(IC_CHARGING_LEFT_7E_BLOCK,0,36);
				show_small_pics(IC_CHARGING_LEFT_8_BLOCK,0,8);
				break;
			default :
				break ;	
			}
			global_main_bms_soc_level_buf=global_main_bms_soc_level;
		}
		//�電池格
		if(global_ex_bms_soc_level_buf==-1 || global_ex_bms_soc_level_buf!=global_ex_bms_soc_level )
		{
			switch (global_ex_bms_soc_level){
			case 0 :
				break ;
			case 1 :
				show_small_pics(IC_CHARGING_RIGHT_1_BLOCK,260,204);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_2A_BLOCK,268,192);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_2B_BLOCK,272,188);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_2C_BLOCK,276,184);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_2D_BLOCK,280,180);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_2E_BLOCK,284,176);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_3_BLOCK,284,148);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_4_BLOCK,284,120);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_5_BLOCK,284,92);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_6_BLOCK,284,64);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7A_BLOCK,284,60);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7B_BLOCK,280,56);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7C_BLOCK,276,52);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7D_BLOCK,272,48);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7E_BLOCK,268,36);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_8_BLOCK,260,8);
				break ;
	
			case 2 :
				show_small_pics(IC_CHARGING_RIGHT_1_BLOCK,260,204);
				show_small_pics(IC_CHARGING_RIGHT_2A_BLOCK,268,192);
				show_small_pics(IC_CHARGING_RIGHT_2B_BLOCK,272,188);
				show_small_pics(IC_CHARGING_RIGHT_2C_BLOCK,276,184);
				show_small_pics(IC_CHARGING_RIGHT_2D_BLOCK,280,180);
				show_small_pics(IC_CHARGING_RIGHT_2E_BLOCK,284,176);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_3_BLOCK,284,148);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_4_BLOCK,284,120);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_5_BLOCK,284,92);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_6_BLOCK,284,64);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7A_BLOCK,284,60);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7B_BLOCK,280,56);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7C_BLOCK,276,52);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7D_BLOCK,272,48);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7E_BLOCK,268,36);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_8_BLOCK,260,8);
				break ;
			case 3 :
				show_small_pics(IC_CHARGING_RIGHT_1_BLOCK,260,204);
				show_small_pics(IC_CHARGING_RIGHT_2A_BLOCK,268,192);
				show_small_pics(IC_CHARGING_RIGHT_2B_BLOCK,272,188);
				show_small_pics(IC_CHARGING_RIGHT_2C_BLOCK,276,184);
				show_small_pics(IC_CHARGING_RIGHT_2D_BLOCK,280,180);
				show_small_pics(IC_CHARGING_RIGHT_2E_BLOCK,284,176);
				show_small_pics(IC_CHARGING_RIGHT_3_BLOCK,284,148);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_4_BLOCK,284,120);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_5_BLOCK,284,92);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_6_BLOCK,284,64);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7A_BLOCK,284,60);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7B_BLOCK,280,56);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7C_BLOCK,276,52);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7D_BLOCK,272,48);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7E_BLOCK,268,36);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_8_BLOCK,260,8);
				break ;
			case 4 :
				show_small_pics(IC_CHARGING_RIGHT_1_BLOCK,260,204);
				show_small_pics(IC_CHARGING_RIGHT_2A_BLOCK,268,192);
				show_small_pics(IC_CHARGING_RIGHT_2B_BLOCK,272,188);
				show_small_pics(IC_CHARGING_RIGHT_2C_BLOCK,276,184);
				show_small_pics(IC_CHARGING_RIGHT_2D_BLOCK,280,180);
				show_small_pics(IC_CHARGING_RIGHT_2E_BLOCK,284,176);
				show_small_pics(IC_CHARGING_RIGHT_3_BLOCK,284,148);
				show_small_pics(IC_CHARGING_RIGHT_4_BLOCK,284,120);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_5_BLOCK,284,92);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_6_BLOCK,284,64);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7A_BLOCK,284,60);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7B_BLOCK,280,56);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7C_BLOCK,276,52);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7D_BLOCK,272,48);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7E_BLOCK,268,36);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_8_BLOCK,260,8);
				break;
			case 5 :
				show_small_pics(IC_CHARGING_RIGHT_1_BLOCK,260,204);
				show_small_pics(IC_CHARGING_RIGHT_2A_BLOCK,268,192);
				show_small_pics(IC_CHARGING_RIGHT_2B_BLOCK,272,188);
				show_small_pics(IC_CHARGING_RIGHT_2C_BLOCK,276,184);
				show_small_pics(IC_CHARGING_RIGHT_2D_BLOCK,280,180);
				show_small_pics(IC_CHARGING_RIGHT_2E_BLOCK,284,176);
				show_small_pics(IC_CHARGING_RIGHT_3_BLOCK,284,148);
				show_small_pics(IC_CHARGING_RIGHT_4_BLOCK,284,120);
				show_small_pics(IC_CHARGING_RIGHT_5_BLOCK,284,92);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_6_BLOCK,284,64);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7A_BLOCK,284,60);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7B_BLOCK,280,56);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7C_BLOCK,276,52);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7D_BLOCK,272,48);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7E_BLOCK,268,36);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_8_BLOCK,260,8);
				break;
			case 6 :
				show_small_pics(IC_CHARGING_RIGHT_1_BLOCK,260,204);
				show_small_pics(IC_CHARGING_RIGHT_2A_BLOCK,268,192);
				show_small_pics(IC_CHARGING_RIGHT_2B_BLOCK,272,188);
				show_small_pics(IC_CHARGING_RIGHT_2C_BLOCK,276,184);
				show_small_pics(IC_CHARGING_RIGHT_2D_BLOCK,280,180);
				show_small_pics(IC_CHARGING_RIGHT_2E_BLOCK,284,176);
				show_small_pics(IC_CHARGING_RIGHT_3_BLOCK,284,148);
				show_small_pics(IC_CHARGING_RIGHT_4_BLOCK,284,120);
				show_small_pics(IC_CHARGING_RIGHT_5_BLOCK,284,92);
				show_small_pics(IC_CHARGING_RIGHT_6_BLOCK,284,64);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7A_BLOCK,284,60);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7B_BLOCK,280,56);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7C_BLOCK,276,52);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7D_BLOCK,272,48);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_7E_BLOCK,268,36);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_8_BLOCK,260,8);
				break;
			case 7 :
				show_small_pics(IC_CHARGING_RIGHT_1_BLOCK,260,204);
				show_small_pics(IC_CHARGING_RIGHT_2A_BLOCK,268,192);
				show_small_pics(IC_CHARGING_RIGHT_2B_BLOCK,272,188);
				show_small_pics(IC_CHARGING_RIGHT_2C_BLOCK,276,184);
				show_small_pics(IC_CHARGING_RIGHT_2D_BLOCK,280,180);
				show_small_pics(IC_CHARGING_RIGHT_2E_BLOCK,284,176);
				show_small_pics(IC_CHARGING_RIGHT_3_BLOCK,284,148);
				show_small_pics(IC_CHARGING_RIGHT_4_BLOCK,284,120);
				show_small_pics(IC_CHARGING_RIGHT_5_BLOCK,284,92);
				show_small_pics(IC_CHARGING_RIGHT_6_BLOCK,284,64);
				show_small_pics(IC_CHARGING_RIGHT_7A_BLOCK,284,60);
				show_small_pics(IC_CHARGING_RIGHT_7B_BLOCK,280,56);
				show_small_pics(IC_CHARGING_RIGHT_7C_BLOCK,276,52);
				show_small_pics(IC_CHARGING_RIGHT_7D_BLOCK,272,48);
				show_small_pics(IC_CHARGING_RIGHT_7E_BLOCK,268,36);
				show_small_pics(IC_CHARGING_RIGHT_EMPTY_8_BLOCK,260,8);
				break;
			case 8 :
				show_small_pics(IC_CHARGING_RIGHT_1_BLOCK,260,204);
				show_small_pics(IC_CHARGING_RIGHT_2A_BLOCK,268,192);
				show_small_pics(IC_CHARGING_RIGHT_2B_BLOCK,272,188);
				show_small_pics(IC_CHARGING_RIGHT_2C_BLOCK,276,184);
				show_small_pics(IC_CHARGING_RIGHT_2D_BLOCK,280,180);
				show_small_pics(IC_CHARGING_RIGHT_2E_BLOCK,284,176);
				show_small_pics(IC_CHARGING_RIGHT_3_BLOCK,284,148);
				show_small_pics(IC_CHARGING_RIGHT_4_BLOCK,284,120);
				show_small_pics(IC_CHARGING_RIGHT_5_BLOCK,284,92);
				show_small_pics(IC_CHARGING_RIGHT_6_BLOCK,284,64);
				show_small_pics(IC_CHARGING_RIGHT_7A_BLOCK,284,60);
				show_small_pics(IC_CHARGING_RIGHT_7B_BLOCK,280,56);
				show_small_pics(IC_CHARGING_RIGHT_7C_BLOCK,276,52);
				show_small_pics(IC_CHARGING_RIGHT_7D_BLOCK,272,48);
				show_small_pics(IC_CHARGING_RIGHT_7E_BLOCK,268,36);
				show_small_pics(IC_CHARGING_RIGHT_8_BLOCK,260,8);
				break;
			default :
				break ;	
			}
			global_ex_bms_soc_level_buf=global_ex_bms_soc_level;
		}
	}
	else
	{		
		//�電池格
		if(global_main_bms_soc_level_buf==-1 || global_main_bms_soc_level_buf!=global_main_bms_soc_level )
		{
			switch (global_main_bms_soc_level){
			case 0 :
				break ;
			case 1 :
				show_small_pics(IC_CHARGING_LEFT_1_BLOCK,0,204);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_2A_BLOCK,0,192);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_2B_BLOCK,0,188);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_2C_BLOCK,0,184);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_2D_BLOCK,0,180);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_2E_BLOCK,0,176);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_3_BLOCK,0,148);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_4_BLOCK,0,120);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_5_BLOCK,0,92);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_6_BLOCK,0,64);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7A_BLOCK,0,60);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7B_BLOCK,0,56);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7C_BLOCK,0,52);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7D_BLOCK,0,48);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7E_BLOCK,0,36);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_8_BLOCK,0,8);
				break ;
	
			case 2 :
				show_small_pics(IC_CHARGING_LEFT_1_BLOCK,0,204);
				show_small_pics(IC_CHARGING_LEFT_2A_BLOCK,0,192);
				show_small_pics(IC_CHARGING_LEFT_2B_BLOCK,0,188);
				show_small_pics(IC_CHARGING_LEFT_2C_BLOCK,0,184);
				show_small_pics(IC_CHARGING_LEFT_2D_BLOCK,0,180);
				show_small_pics(IC_CHARGING_LEFT_2E_BLOCK,0,176);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_3_BLOCK,0,148);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_4_BLOCK,0,120);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_5_BLOCK,0,92);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_6_BLOCK,0,64);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7A_BLOCK,0,60);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7B_BLOCK,0,56);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7C_BLOCK,0,52);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7D_BLOCK,0,48);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7E_BLOCK,0,36);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_8_BLOCK,0,8);
				break ;
			case 3 :
				show_small_pics(IC_CHARGING_LEFT_1_BLOCK,0,204);
				show_small_pics(IC_CHARGING_LEFT_2A_BLOCK,0,192);
				show_small_pics(IC_CHARGING_LEFT_2B_BLOCK,0,188);
				show_small_pics(IC_CHARGING_LEFT_2C_BLOCK,0,184);
				show_small_pics(IC_CHARGING_LEFT_2D_BLOCK,0,180);
				show_small_pics(IC_CHARGING_LEFT_2E_BLOCK,0,176);
				show_small_pics(IC_CHARGING_LEFT_3_BLOCK,0,148);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_4_BLOCK,0,120);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_5_BLOCK,0,92);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_6_BLOCK,0,64);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7A_BLOCK,0,60);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7B_BLOCK,0,56);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7C_BLOCK,0,52);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7D_BLOCK,0,48);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7E_BLOCK,0,36);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_8_BLOCK,0,8);
				break ;
			case 4 :
				show_small_pics(IC_CHARGING_LEFT_1_BLOCK,0,204);
				show_small_pics(IC_CHARGING_LEFT_2A_BLOCK,0,192);
				show_small_pics(IC_CHARGING_LEFT_2B_BLOCK,0,188);
				show_small_pics(IC_CHARGING_LEFT_2C_BLOCK,0,184);
				show_small_pics(IC_CHARGING_LEFT_2D_BLOCK,0,180);
				show_small_pics(IC_CHARGING_LEFT_2E_BLOCK,0,176);
				show_small_pics(IC_CHARGING_LEFT_3_BLOCK,0,148);
				show_small_pics(IC_CHARGING_LEFT_4_BLOCK,0,120);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_5_BLOCK,0,92);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_6_BLOCK,0,64);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7A_BLOCK,0,60);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7B_BLOCK,0,56);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7C_BLOCK,0,52);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7D_BLOCK,0,48);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7E_BLOCK,0,36);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_8_BLOCK,0,8);
				break;
			case 5 :
				show_small_pics(IC_CHARGING_LEFT_1_BLOCK,0,204);
				show_small_pics(IC_CHARGING_LEFT_2A_BLOCK,0,192);
				show_small_pics(IC_CHARGING_LEFT_2B_BLOCK,0,188);
				show_small_pics(IC_CHARGING_LEFT_2C_BLOCK,0,184);
				show_small_pics(IC_CHARGING_LEFT_2D_BLOCK,0,180);
				show_small_pics(IC_CHARGING_LEFT_2E_BLOCK,0,176);
				show_small_pics(IC_CHARGING_LEFT_3_BLOCK,0,148);
				show_small_pics(IC_CHARGING_LEFT_4_BLOCK,0,120);
				show_small_pics(IC_CHARGING_LEFT_5_BLOCK,0,92);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_6_BLOCK,0,64);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7A_BLOCK,0,60);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7B_BLOCK,0,56);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7C_BLOCK,0,52);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7D_BLOCK,0,48);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7E_BLOCK,0,36);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_8_BLOCK,0,8);
				break;
			case 6 :
				show_small_pics(IC_CHARGING_LEFT_1_BLOCK,0,204);
				show_small_pics(IC_CHARGING_LEFT_2A_BLOCK,0,192);
				show_small_pics(IC_CHARGING_LEFT_2B_BLOCK,0,188);
				show_small_pics(IC_CHARGING_LEFT_2C_BLOCK,0,184);
				show_small_pics(IC_CHARGING_LEFT_2D_BLOCK,0,180);
				show_small_pics(IC_CHARGING_LEFT_2E_BLOCK,0,176);
				show_small_pics(IC_CHARGING_LEFT_3_BLOCK,0,148);
				show_small_pics(IC_CHARGING_LEFT_4_BLOCK,0,120);
				show_small_pics(IC_CHARGING_LEFT_5_BLOCK,0,92);
				show_small_pics(IC_CHARGING_LEFT_6_BLOCK,0,64);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7A_BLOCK,0,60);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7B_BLOCK,0,56);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7C_BLOCK,0,52);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7D_BLOCK,0,48);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_7E_BLOCK,0,36);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_8_BLOCK,0,8);
				break;
			case 7 :
				show_small_pics(IC_CHARGING_LEFT_1_BLOCK,0,204);
				show_small_pics(IC_CHARGING_LEFT_2A_BLOCK,0,192);
				show_small_pics(IC_CHARGING_LEFT_2B_BLOCK,0,188);
				show_small_pics(IC_CHARGING_LEFT_2C_BLOCK,0,184);
				show_small_pics(IC_CHARGING_LEFT_2D_BLOCK,0,180);
				show_small_pics(IC_CHARGING_LEFT_2E_BLOCK,0,176);
				show_small_pics(IC_CHARGING_LEFT_3_BLOCK,0,148);
				show_small_pics(IC_CHARGING_LEFT_4_BLOCK,0,120);
				show_small_pics(IC_CHARGING_LEFT_5_BLOCK,0,92);
				show_small_pics(IC_CHARGING_LEFT_6_BLOCK,0,64);
				show_small_pics(IC_CHARGING_LEFT_7A_BLOCK,0,60);
				show_small_pics(IC_CHARGING_LEFT_7B_BLOCK,0,56);
				show_small_pics(IC_CHARGING_LEFT_7C_BLOCK,0,52);
				show_small_pics(IC_CHARGING_LEFT_7D_BLOCK,0,48);
				show_small_pics(IC_CHARGING_LEFT_7E_BLOCK,0,36);
				show_small_pics(IC_CHARGING_LEFT_EMPTY_8_BLOCK,0,8);
				break;
			case 8 :
				show_small_pics(IC_CHARGING_LEFT_1_BLOCK,0,204);
				show_small_pics(IC_CHARGING_LEFT_2A_BLOCK,0,192);
				show_small_pics(IC_CHARGING_LEFT_2B_BLOCK,0,188);
				show_small_pics(IC_CHARGING_LEFT_2C_BLOCK,0,184);
				show_small_pics(IC_CHARGING_LEFT_2D_BLOCK,0,180);
				show_small_pics(IC_CHARGING_LEFT_2E_BLOCK,0,176);
				show_small_pics(IC_CHARGING_LEFT_3_BLOCK,0,148);
				show_small_pics(IC_CHARGING_LEFT_4_BLOCK,0,120);
				show_small_pics(IC_CHARGING_LEFT_5_BLOCK,0,92);
				show_small_pics(IC_CHARGING_LEFT_6_BLOCK,0,64);
				show_small_pics(IC_CHARGING_LEFT_7A_BLOCK,0,60);
				show_small_pics(IC_CHARGING_LEFT_7B_BLOCK,0,56);
				show_small_pics(IC_CHARGING_LEFT_7C_BLOCK,0,52);
				show_small_pics(IC_CHARGING_LEFT_7D_BLOCK,0,48);
				show_small_pics(IC_CHARGING_LEFT_7E_BLOCK,0,36);
				show_small_pics(IC_CHARGING_LEFT_8_BLOCK,0,8);
				break;
			default :
				break ;	
			}
			global_main_bms_soc_level_buf=global_main_bms_soc_level;
		}
	}
}


void show_power_off_charging(void)
{
	flag_button_lock = true;
	if(!global_enter_power_off_charging)
	{
		show_big_pics(BG_POWEROFF_CHARGING_BLOCK,0,0);
		show_small_pics(SUBPAGE_PERCENT_BLACK_BLOCK,224,118);//%
		global_enter_power_off_charging = true;
	}
	
	if(global_power_off_bms_soc_buf==-1 || global_power_off_bms_soc_buf!=global_bms_soc_new )
	{
		show_small_pics(IC_MASK_CHARGING_BLOCK,123,102);
		if(global_bms_soc_new==100)
		{	
			show_small_pics(IC_ERR_1_BLOCK,125,102);//��
			show_small_pics(IC_ERR_0_BLOCK,158,102);//��
			show_small_pics(IC_ERR_0_BLOCK,191,102);//��			
		}
		else if((global_bms_soc_new>=10)&&(global_bms_soc_new<100))
		{
			//�SOC���數��
			switch (global_bms_soc_tens_digit){
				case 1 :
					show_small_pics(IC_ERR_1_BLOCK,158,102);
					break ;
				case 2 :
					show_small_pics(IC_ERR_2_BLOCK,158,102);
					break ;
				case 3 :
					show_small_pics(IC_ERR_3_BLOCK,158,102);
					break ;
				case 4 :
					show_small_pics(IC_ERR_4_BLOCK,158,102);
					break;
				case 5 :
					show_small_pics(IC_ERR_5_BLOCK,158,102);
					break;
				case 6 :
					show_small_pics(IC_ERR_6_BLOCK,158,102);
					break;
				case 7 :
					show_small_pics(IC_ERR_7_BLOCK,158,102);
					break;
				case 8 :
					show_small_pics(IC_ERR_8_BLOCK,158,102);
					break;
				case 9 :
					show_small_pics(IC_ERR_9_BLOCK,158,102);
					break;
				default :
					break ;	
				}
			//�SOC���數��
			switch (global_bms_soc_units_digit){
				case 0 :
					show_small_pics(IC_ERR_0_BLOCK,191,102);
					break ;
				case 1 :
					show_small_pics(IC_ERR_1_BLOCK,191,102);
					break ;
				case 2 :
					show_small_pics(IC_ERR_2_BLOCK,191,102);
					break ;
				case 3 :
					show_small_pics(IC_ERR_3_BLOCK,191,102);
					break ;
				case 4 :
					show_small_pics(IC_ERR_4_BLOCK,191,102);
					break;
				case 5 :
					show_small_pics(IC_ERR_5_BLOCK,191,102);
					break;
				case 6 :
					show_small_pics(IC_ERR_6_BLOCK,191,102);
					break;
				case 7 :
					show_small_pics(IC_ERR_7_BLOCK,191,102);
					break;
				case 8 :
					show_small_pics(IC_ERR_8_BLOCK,191,102);
					break;
				case 9 :
					show_small_pics(IC_ERR_9_BLOCK,191,102);
					break;
				default :
					break ;	
				}
		}		
		else
		{
				//�SOC��
				switch (global_bms_soc_units_digit){
				case 0 :
					show_small_pics(IC_ERR_0_BLOCK,191,102);
					break ;
				case 1 :
					show_small_pics(IC_ERR_1_BLOCK,191,102);
					break ;
				case 2 :
					show_small_pics(IC_ERR_2_BLOCK,191,102);
					break ;
				case 3 :
					show_small_pics(IC_ERR_3_BLOCK,191,102);
					break ;
				case 4 :
					show_small_pics(IC_ERR_4_BLOCK,191,102);
					break;
				case 5 :
					show_small_pics(IC_ERR_5_BLOCK,191,102);
					break;
				case 6 :
					show_small_pics(IC_ERR_6_BLOCK,191,102);
					break;
				case 7 :
					show_small_pics(IC_ERR_7_BLOCK,191,102);
					break;
				case 8 :
					show_small_pics(IC_ERR_8_BLOCK,191,102);
					break;
				case 9 :
					show_small_pics(IC_ERR_9_BLOCK,191,102);
					break;
				default :
					break ;	
				}			
		}
		global_power_off_bms_soc_buf=global_bms_soc_new;
	}
}

/* �放 Walk Mode�畫
 * �數 : void
 * �傳 : void
 */

void show_walking_mode(void)
{
	for(int n=0;n<8;n++)
	{
		if(flag_walk_mode_activate)
		{
			show_small_pics(IC_WALK_1_BLOCK +(IC_WALK_2_BLOCK-IC_WALK_1_BLOCK)*n ,123,102);
			delay_msec(240);
		}
	}	
}

/* �放 ���畫
 * �數 : void
 * �傳 : void
 */


void show_off_logo(void)
{
	flag_button_lock = true;
	draw_page(black);
	show_small_pics(IC_OFF_LOGO_BLOCK,72,84);
	delay_msec(2000);	
	draw_page(black);
}

/* �放 ���畫
 * �數 : void
 * �傳 : void
 */

void show_open_page(void)
{
	//���面
	draw_page(black);
	display_blanking_off(st7789v_devs);

	for(int n=0;n<12;n++)
	{
		show_small_pics(IC_LOGO_1_BLOCK +(IC_LOGO_2_BLOCK-IC_LOGO_1_BLOCK)*n ,90,58);	
		delay_msec(150);
	}
} 

/* �放 low battery �畫
 * �數 : void
 * �傳 : void
 */


void show_low_battery(uint8_t number)
{
	switch (number){
		case 0 :
			show_small_pics(IC_LOWBATTERY_1_1_BLOCK,272,208);
			show_small_pics(IC_LOWBATTERY_1_2_BLOCK,268,192);
			show_small_pics(IC_LOWBATTERY_1_3_BLOCK,272,188);
			show_small_pics(IC_LOWBATTERY_1_4_BLOCK,276,182);
			show_small_pics(IC_LOWBATTERY_1_5_BLOCK,284,176);
			break ;
		case 1 :
			show_small_pics(IC_LOWBATTERY_2_1_BLOCK,272,208);
			show_small_pics(IC_LOWBATTERY_2_2_BLOCK,268,192);
			show_small_pics(IC_LOWBATTERY_2_3_BLOCK,272,188);
			show_small_pics(IC_LOWBATTERY_2_4_BLOCK,276,182);
			show_small_pics(IC_LOWBATTERY_2_5_BLOCK,284,176);
			break ;
		case 2 :
			show_small_pics(IC_LOWBATTERY_3_1_BLOCK,272,208);
			show_small_pics(IC_LOWBATTERY_3_2_BLOCK,268,192);
			show_small_pics(IC_LOWBATTERY_3_3_BLOCK,272,188);
			show_small_pics(IC_LOWBATTERY_3_4_BLOCK,276,182);
			show_small_pics(IC_LOWBATTERY_3_5_BLOCK,284,176);
			break ;
		case 3 :
			show_small_pics(IC_LOWBATTERY_4_1_BLOCK,272,208);
			show_small_pics(IC_LOWBATTERY_4_2_BLOCK,268,192);
			show_small_pics(IC_LOWBATTERY_4_3_BLOCK,272,188);
			show_small_pics(IC_LOWBATTERY_4_4_BLOCK,276,182);
			show_small_pics(IC_LOWBATTERY_4_5_BLOCK,284,176);
			break ;
		case 4 :
			show_small_pics(IC_LOWBATTERY_5_1_BLOCK,272,208);
			show_small_pics(IC_LOWBATTERY_5_2_BLOCK,268,192);
			show_small_pics(IC_LOWBATTERY_5_3_BLOCK,272,188);
			show_small_pics(IC_LOWBATTERY_5_4_BLOCK,276,182);
			show_small_pics(IC_LOWBATTERY_5_5_BLOCK,284,176);
			break;
		case 5 :
			show_small_pics(IC_LOWBATTERY_6_1_BLOCK,272,208);
			show_small_pics(IC_LOWBATTERY_6_2_BLOCK,268,192);
			show_small_pics(IC_LOWBATTERY_6_3_BLOCK,272,188);
			show_small_pics(IC_LOWBATTERY_6_4_BLOCK,276,182);
			show_small_pics(IC_LOWBATTERY_6_5_BLOCK,284,176);
			break;
		case 6 :
			show_small_pics(IC_LOWBATTERY_7_1_BLOCK,272,208);
			show_small_pics(IC_LOWBATTERY_7_2_BLOCK,268,192);
			show_small_pics(IC_LOWBATTERY_7_3_BLOCK,272,188);
			show_small_pics(IC_LOWBATTERY_7_4_BLOCK,276,182);
			show_small_pics(IC_LOWBATTERY_7_5_BLOCK,284,176);
			break;
		case 7 :
			show_small_pics(IC_LOWBATTERY_8_1_BLOCK,272,208);
			show_small_pics(IC_LOWBATTERY_8_2_BLOCK,268,192);
			show_small_pics(IC_LOWBATTERY_8_3_BLOCK,272,188);
			show_small_pics(IC_LOWBATTERY_8_4_BLOCK,276,182);
			show_small_pics(IC_LOWBATTERY_8_5_BLOCK,284,176);
			break;
		case 8 :
			show_small_pics(IC_LOWBATTERY_7_1_BLOCK,272,208);
			show_small_pics(IC_LOWBATTERY_7_2_BLOCK,268,192);
			show_small_pics(IC_LOWBATTERY_7_3_BLOCK,272,188);
			show_small_pics(IC_LOWBATTERY_7_4_BLOCK,276,182);
			show_small_pics(IC_LOWBATTERY_7_5_BLOCK,284,176);
			break;
		case 9 :
			show_small_pics(IC_LOWBATTERY_6_1_BLOCK,272,208);
			show_small_pics(IC_LOWBATTERY_6_2_BLOCK,268,192);
			show_small_pics(IC_LOWBATTERY_6_3_BLOCK,272,188);
			show_small_pics(IC_LOWBATTERY_6_4_BLOCK,276,182);
			show_small_pics(IC_LOWBATTERY_6_5_BLOCK,284,176);
			break;
		case 10 :
			show_small_pics(IC_LOWBATTERY_5_1_BLOCK,272,208);
			show_small_pics(IC_LOWBATTERY_5_2_BLOCK,268,192);
			show_small_pics(IC_LOWBATTERY_5_3_BLOCK,272,188);
			show_small_pics(IC_LOWBATTERY_5_4_BLOCK,276,182);
			show_small_pics(IC_LOWBATTERY_5_5_BLOCK,284,176);
			break;
		case 11 :
			show_small_pics(IC_LOWBATTERY_4_1_BLOCK,272,208);
			show_small_pics(IC_LOWBATTERY_4_2_BLOCK,268,192);
			show_small_pics(IC_LOWBATTERY_4_3_BLOCK,272,188);
			show_small_pics(IC_LOWBATTERY_4_4_BLOCK,276,182);
			show_small_pics(IC_LOWBATTERY_4_5_BLOCK,284,176);
			break;
		case 12 :
			show_small_pics(IC_LOWBATTERY_3_1_BLOCK,272,208);
			show_small_pics(IC_LOWBATTERY_3_2_BLOCK,268,192);
			show_small_pics(IC_LOWBATTERY_3_3_BLOCK,272,188);
			show_small_pics(IC_LOWBATTERY_3_4_BLOCK,276,182);
			show_small_pics(IC_LOWBATTERY_3_5_BLOCK,284,176);
			break;
		case 13 :
			show_small_pics(IC_LOWBATTERY_2_1_BLOCK,272,208);
			show_small_pics(IC_LOWBATTERY_2_2_BLOCK,268,192);
			show_small_pics(IC_LOWBATTERY_2_3_BLOCK,272,188);
			show_small_pics(IC_LOWBATTERY_2_4_BLOCK,276,182);
			show_small_pics(IC_LOWBATTERY_2_5_BLOCK,284,176);
			break;
		case 14 :
			show_small_pics(IC_LOWBATTERY_1_1_BLOCK,272,208);
			show_small_pics(IC_LOWBATTERY_1_2_BLOCK,268,192);
			show_small_pics(IC_LOWBATTERY_1_3_BLOCK,272,188);
			show_small_pics(IC_LOWBATTERY_1_4_BLOCK,276,182);
			show_small_pics(IC_LOWBATTERY_1_5_BLOCK,284,176);
			break;
		default :
			break ;	
		}		
}

/* 顯示���面
 * �數 : void
 * �傳 : void
 */


void show_pair_mode_page(void)
{
	flag_button_lock = true;
	if(!global_enter_status_pics)
	{
		show_small_pics(BG_MAINPAGE_STATUS_BLOCK,60,0);
		global_enter_status_pics=true;
	}
	//�腳踏�大�
	if(global_ebike_light_buf==-1 || global_ebike_light_buf!=global_ebike_light )
	{
		switch (global_ebike_light){
		case 0 :
			show_small_pics(IC_LIGHT_CLOSE_BLOCK,72,16);
			break ;
		case 1 :
			show_small_pics(IC_LIGHT_BLOCK,72,16);
			break ;
		default :
			break ;	
		}
		global_ebike_light_buf=global_ebike_light;
	}

	//�腳踏��芽
	if(global_ebike_bluetooth_buf==-1 || global_ebike_bluetooth_buf!=global_ebike_bluetooth )
	{
		switch (global_ebike_bluetooth){
		case 0 :
			show_small_pics(IC_BLUETOOTH_DISABLE_BLOCK,220,16);
			break ;
		case 1 :
			show_small_pics(IC_BLUETOOTH_BLOCK,220,16);
			break ;
		default :
			break ;	
		}
		global_ebike_bluetooth_buf=global_ebike_bluetooth;
	}
	if(!global_enter_pair_mode)
	{
		show_small_pics(BG_WLPE_LEFT_BLOCK,0,0);
		show_small_pics(BG_WLPE_RIGHT_BLOCK,260,0);
		show_big_pics(BG_MAINPAGE_BLOCK,60,44);
		show_small_pics(IC_PAIRING_BLOCK,104,60);
		global_enter_pair_mode = true;
	}
	
	//�Pair Code 01
	if(global_dispaly_ble_passkey_code1_digit_buf==-1 || global_dispaly_ble_passkey_code1_digit_buf != global_dispaly_ble_passkey_code1_digit )
	{
		switch (global_dispaly_ble_passkey_code1_digit){
		case 0 :
			show_small_pics(IC_ERR_0_BLOCK,63,109);
			break ;
		case 1 :
			show_small_pics(IC_ERR_1_BLOCK,63,109);
			break ;
		case 2 :
			show_small_pics(IC_ERR_2_BLOCK,63,109);
			break ;
		case 3 :
			show_small_pics(IC_ERR_3_BLOCK,63,109);
			break ;
		case 4 :
			show_small_pics(IC_ERR_4_BLOCK,63,109);
			break ;
		case 5 :
			show_small_pics(IC_ERR_5_BLOCK,63,109);
			break ;
		case 6 :
			show_small_pics(IC_ERR_6_BLOCK,63,109);
			break ;
		case 7 :
			show_small_pics(IC_ERR_7_BLOCK,63,109);
			break ;
		case 8 :
			show_small_pics(IC_ERR_8_BLOCK,63,109);
			break ;
		case 9 :
			show_small_pics(IC_ERR_9_BLOCK,63,109);
			break ;
		default :
			break ;	
		}
		global_dispaly_ble_passkey_code1_digit_buf=global_dispaly_ble_passkey_code1_digit;
	}
	//�Pair Code 02
	if(global_dispaly_ble_passkey_code2_digit_buf==-1 || global_dispaly_ble_passkey_code2_digit_buf != global_dispaly_ble_passkey_code2_digit )
	{
		switch (global_dispaly_ble_passkey_code2_digit){
		case 0 :
			show_small_pics(IC_ERR_0_BLOCK,96,109);
			break ;
		case 1 :
			show_small_pics(IC_ERR_1_BLOCK,96,109);
			break ;
		case 2 :
			show_small_pics(IC_ERR_2_BLOCK,96,109);
			break ;
		case 3 :
			show_small_pics(IC_ERR_3_BLOCK,96,109);
			break ;
		case 4 :
			show_small_pics(IC_ERR_4_BLOCK,96,109);
			break ;
		case 5 :
			show_small_pics(IC_ERR_5_BLOCK,96,109);
			break ;
		case 6 :
			show_small_pics(IC_ERR_6_BLOCK,96,109);
			break ;
		case 7 :
			show_small_pics(IC_ERR_7_BLOCK,96,109);
			break ;
		case 8 :
			show_small_pics(IC_ERR_8_BLOCK,96,109);
			break ;
		case 9 :
			show_small_pics(IC_ERR_9_BLOCK,96,109);
			break ;
		default :
			break ;	
		}
		global_dispaly_ble_passkey_code2_digit_buf=global_dispaly_ble_passkey_code2_digit;
	}
	//�Pair Code 03
	if(global_dispaly_ble_passkey_code3_digit_buf==-1 || global_dispaly_ble_passkey_code3_digit_buf != global_dispaly_ble_passkey_code3_digit )
	{
		switch (global_dispaly_ble_passkey_code3_digit){
		case 0 :
			show_small_pics(IC_ERR_0_BLOCK,129,109);
			break ;
		case 1 :
			show_small_pics(IC_ERR_1_BLOCK,129,109);
			break ;
		case 2 :
			show_small_pics(IC_ERR_2_BLOCK,129,109);
			break ;
		case 3 :
			show_small_pics(IC_ERR_3_BLOCK,129,109);
			break ;
		case 4 :
			show_small_pics(IC_ERR_4_BLOCK,129,109);
			break ;
		case 5 :
			show_small_pics(IC_ERR_5_BLOCK,129,109);
			break ;
		case 6 :
			show_small_pics(IC_ERR_6_BLOCK,129,109);
			break ;
		case 7 :
			show_small_pics(IC_ERR_7_BLOCK,129,109);
			break ;
		case 8 :
			show_small_pics(IC_ERR_8_BLOCK,129,109);
			break ;
		case 9 :
			show_small_pics(IC_ERR_9_BLOCK,129,109);
			break ;
		default :
			break ;	
		}
		global_dispaly_ble_passkey_code3_digit_buf=global_dispaly_ble_passkey_code3_digit;
	}
	//�Pair Code 04
	if(global_dispaly_ble_passkey_code4_digit_buf==-1 || global_dispaly_ble_passkey_code4_digit_buf != global_dispaly_ble_passkey_code4_digit )
	{
		switch (global_dispaly_ble_passkey_code4_digit){
		case 0 :
			show_small_pics(IC_ERR_0_BLOCK,162,109);
			break ;
		case 1 :
			show_small_pics(IC_ERR_1_BLOCK,162,109);
			break ;
		case 2 :
			show_small_pics(IC_ERR_2_BLOCK,162,109);
			break ;
		case 3 :
			show_small_pics(IC_ERR_3_BLOCK,162,109);
			break ;
		case 4 :
			show_small_pics(IC_ERR_4_BLOCK,162,109);
			break ;
		case 5 :
			show_small_pics(IC_ERR_5_BLOCK,162,109);
			break ;
		case 6 :
			show_small_pics(IC_ERR_6_BLOCK,162,109);
			break ;
		case 7 :
			show_small_pics(IC_ERR_7_BLOCK,162,109);
			break ;
		case 8 :
			show_small_pics(IC_ERR_8_BLOCK,162,109);
			break ;
		case 9 :
			show_small_pics(IC_ERR_9_BLOCK,162,109);
			break ;
		default :
			break ;	
		}
		global_dispaly_ble_passkey_code4_digit_buf=global_dispaly_ble_passkey_code4_digit;
	}
	//�Pair Code 05
	if(global_dispaly_ble_passkey_code5_digit_buf==-1 || global_dispaly_ble_passkey_code5_digit_buf != global_dispaly_ble_passkey_code5_digit )
	{
		switch (global_dispaly_ble_passkey_code5_digit){
		case 0 :
			show_small_pics(IC_ERR_0_BLOCK,195,109);
			break ;
		case 1 :
			show_small_pics(IC_ERR_1_BLOCK,195,109);
			break ;
		case 2 :
			show_small_pics(IC_ERR_2_BLOCK,195,109);
			break ;
		case 3 :
			show_small_pics(IC_ERR_3_BLOCK,195,109);
			break ;
		case 4 :
			show_small_pics(IC_ERR_4_BLOCK,195,109);
			break ;
		case 5 :
			show_small_pics(IC_ERR_5_BLOCK,195,109);
			break ;
		case 6 :
			show_small_pics(IC_ERR_6_BLOCK,195,109);
			break ;
		case 7 :
			show_small_pics(IC_ERR_7_BLOCK,195,109);
			break ;
		case 8 :
			show_small_pics(IC_ERR_8_BLOCK,195,109);
			break ;
		case 9 :
			show_small_pics(IC_ERR_9_BLOCK,195,109);
			break ;
		default :
			break ;	
		}
		global_dispaly_ble_passkey_code5_digit_buf=global_dispaly_ble_passkey_code5_digit;
	}
		//�Pair Code 06
	if(global_dispaly_ble_passkey_code6_digit_buf==-1 || global_dispaly_ble_passkey_code6_digit_buf != global_dispaly_ble_passkey_code6_digit )
	{
		switch (global_dispaly_ble_passkey_code6_digit){
		case 0 :
			show_small_pics(IC_ERR_0_BLOCK,228,109);
			break ;
		case 1 :
			show_small_pics(IC_ERR_1_BLOCK,228,109);
			break ;
		case 2 :
			show_small_pics(IC_ERR_2_BLOCK,228,109);
			break ;
		case 3 :
			show_small_pics(IC_ERR_3_BLOCK,228,109);
			break ;
		case 4 :
			show_small_pics(IC_ERR_4_BLOCK,228,109);
			break ;
		case 5 :
			show_small_pics(IC_ERR_5_BLOCK,228,109);
			break ;
		case 6 :
			show_small_pics(IC_ERR_6_BLOCK,228,109);
			break ;
		case 7 :
			show_small_pics(IC_ERR_7_BLOCK,228,109);
			break ;
		case 8 :
			show_small_pics(IC_ERR_8_BLOCK,228,109);
			break ;
		case 9 :
			show_small_pics(IC_ERR_9_BLOCK,228,109);
			break ;
		default :
			break ;	
		}
		global_dispaly_ble_passkey_code6_digit_buf=global_dispaly_ble_passkey_code6_digit;
	}
}

void show_reconnect_page(void)
{
	flag_button_lock = true;
	if(!global_enter_status_pics)
	{
		show_small_pics(BG_MAINPAGE_STATUS_BLOCK,60,0);
		global_enter_status_pics=true;
	}
	//�腳踏�大�
	if(global_ebike_light_buf==-1 || global_ebike_light_buf!=global_ebike_light )
	{
		switch (global_ebike_light){
		case 0 :
			show_small_pics(IC_LIGHT_CLOSE_BLOCK,72,16);
			break ;
		case 1 :
			show_small_pics(IC_LIGHT_BLOCK,72,16);
			break ;
		default :
			break ;	
		}
		global_ebike_light_buf=global_ebike_light;
	}

	//�腳踏��芽
	if(global_ebike_bluetooth_buf==-1 || global_ebike_bluetooth_buf!=global_ebike_bluetooth )
	{
		switch (global_ebike_bluetooth){
		case 0 :
			show_small_pics(IC_BLUETOOTH_DISABLE_BLOCK,220,16);
			break ;
		case 1 :
			show_small_pics(IC_BLUETOOTH_BLOCK,220,16);
			break ;
		default :
			break ;	
		}
		global_ebike_bluetooth_buf=global_ebike_bluetooth;
	}
	if(!global_enter_pair_mode)
	{
		show_small_pics(BG_WLPE_LEFT_BLOCK,0,0);
		show_small_pics(BG_WLPE_RIGHT_BLOCK,260,0);
		show_big_pics(BG_MAINPAGE_BLOCK,60,44);
		show_small_pics(IC_PAIRING_BLOCK,104,60);
		global_enter_pair_mode = true;
	}

	if(ble_paired_n>2) ble_paired_n = 0;
	show_small_pics(IC_RECONNECT_1_BLOCK +(IC_RECONNECT_2_BLOCK-IC_RECONNECT_1_BLOCK)*ble_paired_n ,88,122);
	ble_paired_n++;
}


/* 顯示一�錯誤�
 * �數 : void
 * �傳 : void
 */


void general_error_page(void)
{
	uint32_t morto_err = 0;

	flag_button_lock = true;
	if(!global_enter_status_pics)
	{
		show_small_pics(BG_MAINPAGE_STATUS_BLOCK,60,0);
		global_enter_status_pics=true;
	}
	if(!global_enter_error)
	{
		show_small_pics(BG_WLPE_LEFT_BLOCK,0,0);
		show_small_pics(BG_WLPE_RIGHT_BLOCK,260,0);
		show_big_pics(BG_MAINPAGE_BLOCK,60,44);
		show_small_pics(IC_TITLE_ERROR_BLOCK,118,60);
		global_enter_error = true;
	}
	
	if( flag_global_system_motro_error_general_activate ||
		flag_global_system_motro_error_general_fw_err_activate)
	{		
		FIT_CANFD_PROCESS_MOTOR_ERROR_Get(&morto_err);
		for(int i = 27; i < 32; i++ )
		{	
			if((morto_err >> i) & 0x01)
			{
				//show_small_pics(IC_ERROR_MASK_BLOCK,60,107);
				switch (i)
				{
					case 27:
						if(!flag_global_system_motro_error_general_done)
						{
							show_small_pics(IC_ERRORCODE_C_BLOCK,77,109);
							show_small_pics(IC_ERRORCODE_E_BLOCK,121,109);
							show_small_pics(IC_ERR_0_BLOCK,155,109);
							show_small_pics(IC_ERR_2_BLOCK,185,109);
							show_small_pics(IC_ERR_7_BLOCK,215,109);
							flag_global_system_motro_error_general_done = true; 
							for(int j = 0; j < 5; j++)
							{			
								if(flag_off_logo_activate)
									break;
							
								delay_msec(1000);
							}
						}
						break;
						
					case 31:
						if(!flag_global_system_motro_error_general_fw_err_done)
						{
							show_small_pics(IC_ERRORCODE_C_BLOCK,77,109);
							show_small_pics(IC_ERRORCODE_E_BLOCK,121,109);
							show_small_pics(IC_ERR_0_BLOCK,155,109);
							show_small_pics(IC_ERR_3_BLOCK,185,109);
							show_small_pics(IC_ERR_1_BLOCK,215,109);
							flag_global_system_motro_error_general_fw_err_done = true;
							for(int j = 0; j < 5; j++)
							{			
								if(flag_off_logo_activate)
									break;
							
								delay_msec(1000);
							}
						}
						break;
						
					default:
						break;
				}
			}
		}	
		if( flag_global_system_main_bms_error_activate ||
		 	flag_global_system_ex_bms_error_activate ||
		 	flag_global_system_motor_error_activate	)
			show_small_pics(IC_ERROR_MASK_BLOCK,60,107);
		 	
	}
}

/* 顯示馬��誤�面
 * �數 : void
 * �傳 : void
 */

void serious_error_motor_page(void)
{
	uint32_t morto_err = 0;

	flag_button_lock = true;
	if(!global_enter_status_pics)
	{
		show_small_pics(BG_MAINPAGE_STATUS_BLOCK,60,0);
		global_enter_status_pics=true;
	}
	if(!global_enter_error)
	{
		show_small_pics(BG_WLPE_LEFT_BLOCK,0,0);
		show_small_pics(BG_WLPE_RIGHT_BLOCK,260,0);
		show_big_pics(BG_MAINPAGE_BLOCK,60,44);
		show_small_pics(IC_TITLE_ERROR_BLOCK,118,60);
		global_enter_error = true;
	}
	
	if(flag_global_system_motor_error_activate)
	{
		FIT_CANFD_PROCESS_MOTOR_ERROR_Get(&morto_err);
		for(int i = 0; i < 32; i++ )
		{
			if(flag_off_logo_activate)
				break;
			
			if((morto_err >> i) & 0x01)
			{
				//show_small_pics(IC_ERROR_MASK_BLOCK,60,107);
				switch (i)
				{
					case 2:
						show_small_pics(IC_ERRORCODE_C_BLOCK,77,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,121,109);
						show_small_pics(IC_ERR_0_BLOCK,155,109);
						show_small_pics(IC_ERR_0_BLOCK,185,109);
						show_small_pics(IC_ERR_2_BLOCK,215,109);
						delay_msec(3000);
						break;
						
					case 3:
						show_small_pics(IC_ERRORCODE_C_BLOCK,77,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,121,109);
						show_small_pics(IC_ERR_0_BLOCK,155,109);
						show_small_pics(IC_ERR_0_BLOCK,185,109);
						show_small_pics(IC_ERR_3_BLOCK,215,109);
						delay_msec(3000);
						break;
						
					case 4:
						show_small_pics(IC_ERRORCODE_C_BLOCK,77,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,121,109);
						show_small_pics(IC_ERR_0_BLOCK,155,109);
						show_small_pics(IC_ERR_0_BLOCK,185,109);
						show_small_pics(IC_ERR_4_BLOCK,215,109);
						delay_msec(3000);
						break;
						
					case 5:
						show_small_pics(IC_ERRORCODE_C_BLOCK,77,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,121,109);
						show_small_pics(IC_ERR_0_BLOCK,155,109);
						show_small_pics(IC_ERR_0_BLOCK,185,109);
						show_small_pics(IC_ERR_5_BLOCK,215,109);
						delay_msec(3000);
						break;
						
					case 6:
						show_small_pics(IC_ERRORCODE_C_BLOCK,77,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,121,109);
						show_small_pics(IC_ERR_0_BLOCK,155,109);
						show_small_pics(IC_ERR_0_BLOCK,185,109);
						show_small_pics(IC_ERR_6_BLOCK,215,109);
						delay_msec(3000);
						break;
						
					case 7:
						show_small_pics(IC_ERRORCODE_C_BLOCK,77,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,121,109);
						show_small_pics(IC_ERR_0_BLOCK,155,109);
						show_small_pics(IC_ERR_0_BLOCK,185,109);
						show_small_pics(IC_ERR_7_BLOCK,215,109);
						delay_msec(3000);
						break;
						
					case 8:
						show_small_pics(IC_ERRORCODE_C_BLOCK,77,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,121,109);
						show_small_pics(IC_ERR_0_BLOCK,155,109);
						show_small_pics(IC_ERR_0_BLOCK,185,109);
						show_small_pics(IC_ERR_8_BLOCK,215,109);
						delay_msec(3000);
						break;
						
					case 9:
						show_small_pics(IC_ERRORCODE_C_BLOCK,77,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,121,109);
						show_small_pics(IC_ERR_0_BLOCK,155,109);
						show_small_pics(IC_ERR_0_BLOCK,185,109);
						show_small_pics(IC_ERR_9_BLOCK,215,109);
						delay_msec(3000);
						break;
					
					case 10:
						show_small_pics(IC_ERRORCODE_C_BLOCK,77,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,121,109);
						show_small_pics(IC_ERR_0_BLOCK,155,109);
						show_small_pics(IC_ERR_1_BLOCK,185,109);
						show_small_pics(IC_ERR_0_BLOCK,215,109);
						delay_msec(3000);
						break;
						
					case 11:
						show_small_pics(IC_ERRORCODE_C_BLOCK,77,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,121,109);
						show_small_pics(IC_ERR_0_BLOCK,155,109);
						show_small_pics(IC_ERR_1_BLOCK,185,109);
						show_small_pics(IC_ERR_1_BLOCK,215,109);
						delay_msec(3000);
						break;
						
					case 12:
						show_small_pics(IC_ERRORCODE_C_BLOCK,77,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,121,109);
						show_small_pics(IC_ERR_0_BLOCK,155,109);
						show_small_pics(IC_ERR_1_BLOCK,185,109);
						show_small_pics(IC_ERR_2_BLOCK,215,109);
						delay_msec(3000);
						break;
						
					case 13:
						show_small_pics(IC_ERRORCODE_C_BLOCK,77,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,121,109);
						show_small_pics(IC_ERR_0_BLOCK,155,109);
						show_small_pics(IC_ERR_1_BLOCK,185,109);
						show_small_pics(IC_ERR_3_BLOCK,215,109);
						delay_msec(3000);
						break;
						
					case 14:
						show_small_pics(IC_ERRORCODE_C_BLOCK,77,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,121,109);
						show_small_pics(IC_ERR_0_BLOCK,155,109);
						show_small_pics(IC_ERR_1_BLOCK,185,109);
						show_small_pics(IC_ERR_4_BLOCK,215,109);
						delay_msec(3000);
						break;
						
					case 21:
						show_small_pics(IC_ERRORCODE_C_BLOCK,77,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,121,109);
						show_small_pics(IC_ERR_0_BLOCK,155,109);
						show_small_pics(IC_ERR_2_BLOCK,185,109);
						show_small_pics(IC_ERR_1_BLOCK,215,109);
						delay_msec(3000);
						break;
						
					case 22:
						show_small_pics(IC_ERRORCODE_C_BLOCK,77,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,121,109);
						show_small_pics(IC_ERR_0_BLOCK,155,109);
						show_small_pics(IC_ERR_2_BLOCK,185,109);
						show_small_pics(IC_ERR_2_BLOCK,215,109);
						delay_msec(3000);
						break;
					default:
						break;
				}
			}
		}
		if( flag_global_system_main_bms_error_activate 		||
		 	flag_global_system_ex_bms_error_activate 		)
			show_small_pics(IC_ERROR_MASK_BLOCK,60,107);
	}	
}

/* 顯示主電池錯誤�
 * �數 : void
 * �傳 : void
 */

void serious_error_m_bms_page(void)
{
	uint16_t bms_error = 0;

	flag_button_lock = true;
	if(!global_enter_status_pics)
	{
		show_small_pics(BG_MAINPAGE_STATUS_BLOCK,60,0);
		global_enter_status_pics=true;
	}
	if(!global_enter_error)
	{
		show_small_pics(BG_WLPE_LEFT_BLOCK,0,0);
		show_small_pics(BG_WLPE_RIGHT_BLOCK,260,0);
		show_big_pics(BG_MAINPAGE_BLOCK,60,44);
		show_small_pics(IC_TITLE_ERROR_BLOCK,118,60);
		global_enter_error = true;
	}
	
	if(flag_global_system_main_bms_error_activate)
	{
		FIT_CANFD_PROCESS_BMS_MAIN_ERROR_Get(&bms_error);
		for(int i = 0; i < 16; i++ )
		{
			if(flag_off_logo_activate)
				break;
			
			if((bms_error >> i) & 0x01)
			{
				//show_small_pics(IC_ERROR_MASK_BLOCK,60,107);	
				switch (i)
				{
					case 0:							
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_1_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_0_BLOCK,200,109);
						show_small_pics(IC_ERR_1_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 1:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_1_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_0_BLOCK,200,109);
						show_small_pics(IC_ERR_2_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 2:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_1_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_0_BLOCK,200,109);
						show_small_pics(IC_ERR_3_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 3:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_1_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_0_BLOCK,200,109);
						show_small_pics(IC_ERR_4_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 4:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_1_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_0_BLOCK,200,109);
						show_small_pics(IC_ERR_5_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 5:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_1_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_0_BLOCK,200,109);
						show_small_pics(IC_ERR_6_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 6:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_1_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_0_BLOCK,200,109);
						show_small_pics(IC_ERR_7_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 7:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_1_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_0_BLOCK,200,109);
						show_small_pics(IC_ERR_8_BLOCK,230,109);
						delay_msec(3000);
						break;
					
					case 8:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_1_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_0_BLOCK,200,109);
						show_small_pics(IC_ERR_9_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 9:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_1_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_1_BLOCK,200,109);
						show_small_pics(IC_ERR_0_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 10:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_1_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_1_BLOCK,200,109);
						show_small_pics(IC_ERR_1_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 11:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_1_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_1_BLOCK,200,109);
						show_small_pics(IC_ERR_2_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 12:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_1_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_1_BLOCK,200,109);
						show_small_pics(IC_ERR_3_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 13:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_1_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_1_BLOCK,200,109);
						show_small_pics(IC_ERR_4_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 14:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_1_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_1_BLOCK,200,109);
						show_small_pics(IC_ERR_5_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 15:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_1_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_1_BLOCK,200,109);
						show_small_pics(IC_ERR_6_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					default:
						break;
				}
			}
		}
		if( flag_global_system_ex_bms_error_activate 		||
		 	flag_global_system_motor_error_activate 		|| 
		 	flag_global_system_motro_error_general_activate ||
			flag_global_system_motro_error_general_fw_err_activate)
			show_small_pics(IC_ERROR_MASK_BLOCK,60,107);		
	}
}

/* 顯示�電池錯誤�
 * �數 : void
 * �傳 : void
 */

void serious_error_e_bms_page(void)
{
	uint16_t bms_error = 0;

	flag_button_lock = true;
	if(!global_enter_status_pics)
	{
		show_small_pics(BG_MAINPAGE_STATUS_BLOCK,60,0);
		global_enter_status_pics=true;
	}
	if(!global_enter_error)
	{
		show_small_pics(BG_WLPE_LEFT_BLOCK,0,0);
		show_small_pics(BG_WLPE_RIGHT_BLOCK,260,0);
		show_big_pics(BG_MAINPAGE_BLOCK,60,44);
		show_small_pics(IC_TITLE_ERROR_BLOCK,118,60);
		global_enter_error = true;
	}
	
	if(flag_global_system_ex_bms_error_activate)
	{
		FIT_CANFD_PROCESS_BMS_EX_ERROR_Get(&bms_error);
		for(int i = 0; i < 16; i++ )
		{			
			if(flag_off_logo_activate)
				break;
			
			if((bms_error >> i) & 0x01)
						{
				//show_small_pics(IC_ERROR_MASK_BLOCK,60,107);	
				switch (i)
				{
					case 0:							
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_2_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_0_BLOCK,200,109);
						show_small_pics(IC_ERR_1_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 1:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_2_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_0_BLOCK,200,109);
						show_small_pics(IC_ERR_2_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 2:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_2_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_0_BLOCK,200,109);
						show_small_pics(IC_ERR_3_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 3:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_2_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_0_BLOCK,200,109);
						show_small_pics(IC_ERR_4_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 4:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_2_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_0_BLOCK,200,109);
						show_small_pics(IC_ERR_5_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 5:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_2_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_0_BLOCK,200,109);
						show_small_pics(IC_ERR_6_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 6:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_2_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_0_BLOCK,200,109);
						show_small_pics(IC_ERR_7_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 7:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_2_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_0_BLOCK,200,109);
						show_small_pics(IC_ERR_8_BLOCK,230,109);
						delay_msec(3000);
						break;
					
					case 8:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_2_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_0_BLOCK,200,109);
						show_small_pics(IC_ERR_9_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 9:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_2_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_1_BLOCK,200,109);
						show_small_pics(IC_ERR_0_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 10:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_2_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_1_BLOCK,200,109);
						show_small_pics(IC_ERR_1_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 11:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_2_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_1_BLOCK,200,109);
						show_small_pics(IC_ERR_2_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 12:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_2_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_1_BLOCK,200,109);
						show_small_pics(IC_ERR_3_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 13:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_2_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_1_BLOCK,200,109);
						show_small_pics(IC_ERR_4_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 14:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_2_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_1_BLOCK,200,109);
						show_small_pics(IC_ERR_5_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					case 15:
						show_small_pics(IC_ERRORCODE_B_BLOCK,62,109);
						show_small_pics(IC_ERR_2_BLOCK,96,109);
						show_small_pics(IC_ERRORCODE_E_BLOCK,136,109);
						show_small_pics(IC_ERR_0_BLOCK,170,109);
						show_small_pics(IC_ERR_1_BLOCK,200,109);
						show_small_pics(IC_ERR_6_BLOCK,230,109);
						delay_msec(3000);
						break;
						
					default:
						break;
				}
			}
		}
		if( flag_global_system_main_bms_error_activate 		||
		 	flag_global_system_motor_error_activate 		|| 
		 	flag_global_system_motro_error_general_activate ||
			flag_global_system_motro_error_general_fw_err_activate)
			show_small_pics(IC_ERROR_MASK_BLOCK,60,107);	
	}
}

void show_time(void)
{
		//����0位數
		if(global_time_hour_tens_digit_buf==-1 || global_time_hour_tens_digit_buf!=global_time_hour_tens_digit )
		{
			switch (global_time_hour_tens_digit){
			case 0 :
				show_small_pics(TIME_0_BLOCK,116,16);
				break ;
			case 1 :
				show_small_pics(TIME_1_BLOCK,116,16);
				break ;
			case 2 :
				show_small_pics(TIME_2_BLOCK,116,16);
				break ;
			case 3 :
				show_small_pics(TIME_3_BLOCK,116,16);
				break ;
			case 4 :
				show_small_pics(TIME_4_BLOCK,116,16);
				break;
			case 5 :
				show_small_pics(TIME_5_BLOCK,116,16);
				break;
			case 6 :
				show_small_pics(TIME_6_BLOCK,116,16);
				break;
			case 7 :
				show_small_pics(TIME_7_BLOCK,116,16);
				break;
			case 8 :
				show_small_pics(TIME_8_BLOCK,116,16);
				break;
			case 9 :
				show_small_pics(TIME_9_BLOCK,116,16);
				break;
			default :
				break ;	
			}
			global_time_hour_tens_digit_buf=global_time_hour_tens_digit;
		}
		//�����個�
		if(global_time_hour_units_digit_buf==-1 || global_time_hour_units_digit_buf!=global_time_hour_units_digit )
		{
			switch (global_time_hour_units_digit){
			case 0 :
				show_small_pics(TIME_0_BLOCK,136,16);
				break ;
			case 1 :
				show_small_pics(TIME_1_BLOCK,136,16);
				break ;
			case 2 :
				show_small_pics(TIME_2_BLOCK,136,16);
				break ;
			case 3 :
				show_small_pics(TIME_3_BLOCK,136,16);
				break ;
			case 4 :
				show_small_pics(TIME_4_BLOCK,136,16);
				break;
			case 5 :
				show_small_pics(TIME_5_BLOCK,136,16);
				break;
			case 6 :
				show_small_pics(TIME_6_BLOCK,136,16);
				break;
			case 7 :
				show_small_pics(TIME_7_BLOCK,136,16);
				break;
			case 8 :
				show_small_pics(TIME_8_BLOCK,136,16);
				break;
			case 9 :
				show_small_pics(TIME_9_BLOCK,136,16);
				break;
			default :
				break ;
			}
			global_time_hour_units_digit_buf=global_time_hour_units_digit;
		}
		//����0位數
		if(global_time_min_tens_digit_buf==-1 || global_time_min_tens_digit_buf!=global_time_min_tens_digit )
		{
			switch (global_time_min_tens_digit){
			case 0 :
				show_small_pics(TIME_0_BLOCK,168,16);
				break ;
			case 1 :
				show_small_pics(TIME_1_BLOCK,168,16);
				break ;
			case 2 :
				show_small_pics(TIME_2_BLOCK,168,16);
				break ;
			case 3 :
				show_small_pics(TIME_3_BLOCK,168,16);
				break ;
			case 4 :
				show_small_pics(TIME_4_BLOCK,168,16);
				break;
			case 5 :
				show_small_pics(TIME_5_BLOCK,168,16);
				break;
			case 6 :
				show_small_pics(TIME_6_BLOCK,168,16);
				break;
			case 7 :
				show_small_pics(TIME_7_BLOCK,168,16);
				break;
			case 8 :
				show_small_pics(TIME_8_BLOCK,168,16);
				break;
			case 9 :
				show_small_pics(TIME_9_BLOCK,168,16);
				break;
			default :
				break ;
			}
			global_time_min_tens_digit_buf=global_time_min_tens_digit;
		}
		//�����個�
		if(global_time_min_units_digit_buf==-1 || global_time_min_units_digit_buf!=global_time_min_units_digit )
		{
			show_small_pics(TIME_COLON_BLOCK,156,16);
			switch (global_time_min_units_digit){
			case 0 :
				show_small_pics(TIME_0_BLOCK,188,16);
				break ;
			case 1 :
				show_small_pics(TIME_1_BLOCK,188,16);
				break ;
			case 2 :
				show_small_pics(TIME_2_BLOCK,188,16);
				break ;
			case 3 :
				show_small_pics(TIME_3_BLOCK,188,16);
				break ;
			case 4 :
				show_small_pics(TIME_4_BLOCK,188,16);
				break;
			case 5 :
				show_small_pics(TIME_5_BLOCK,188,16);
				break;
			case 6 :
				show_small_pics(TIME_6_BLOCK,188,16);
				break;
			case 7 :
				show_small_pics(TIME_7_BLOCK,188,16);
				break;
			case 8 :
				show_small_pics(TIME_8_BLOCK,188,16);
				break;
			case 9 :
				show_small_pics(TIME_9_BLOCK,188,16);
				break;
			default :
				break ;
			}
			global_time_min_units_digit_buf=global_time_min_units_digit; 
		}
}

/* Show FTM Page�數
 * �數 : void
 * �傳 : void
 */

void show_ftm_page(void)
{
	flag_button_lock = true;

	if(!global_enter_ftm)
	{
		show_small_pics(IC_FTM_BLOCK,104,92);
		global_enter_ftm = true;
	}
}

/* Reset Main Page�數
 * �數 : void
 * �傳 : void
 */

void reset_main_page(void)
{
	global_ebike_light_buf=-1;
	global_ebike_bluetooth_buf=-1;
	low_battery_count = 0;
	main_global_motor_assist_level_buf=-1;
	global_motor_current_speed_tens_digit_buf=-1;
	global_motor_current_speed_units_digit_buf=-1;
	global_main_bms_soc_level_buf=-1;
	global_enter_low_battery = false;
	main_global_page_index_buf=-1;
	global_main_distance_unit_buf=-1;
	global_enter_status_pics=false;
	global_time_hour_tens_digit_buf=-1;
	global_time_hour_units_digit_buf=-1;
	global_time_min_tens_digit_buf=-1;
	global_time_min_units_digit_buf=-1;
}

/* Reset Sub Page 01�數
 * �數 : void
 * �傳 : void
 */

void reset_sub_page01(void)
{
	global_ebike_light_buf=-1;
	global_ebike_bluetooth_buf=-1;
	sub01_global_motor_assist_level_buf=-1;
	//global_motor_est_range_houndred_digit_buf=-1;
	//global_motor_est_range_tens_digit_buf=-1;
	//global_motor_est_range_units_digit_buf=-1;
	sub01_global_page_index_buf = -1;
	global_bms_soc_buf = -1;
	global_motor_est_range_buf=-1;
	global_sub01_distance_unit_buf=-1;
	global_enter_status_pics=false;
	global_time_hour_tens_digit_buf=-1;
	global_time_hour_units_digit_buf=-1;
	global_time_min_tens_digit_buf=-1;
	global_time_min_units_digit_buf=-1;
}

/* Reset Sub Page 02�數
 * �數 : void
 * �傳 : void
 */

void reset_sub_page02(void)
{
	global_ebike_light_buf=-1;
	global_ebike_bluetooth_buf=-1;
	sub02_global_motor_assist_level_buf=-1;
	global_motor_odo_ten_thousands_digit_buf=-1;
	global_motor_odo_thousands_digit_buf=-1;
	global_motor_odo_hundreds_digit_buf=-1;
	global_motor_odo_tens_digit_buf =-1;
	global_motor_odo_units_digit_buf =-1;
	//global_motor_trip_hundred_digit_buf=-1;
	//global_motor_trip_tens_digit_buf=-1;
	//global_motor_trip_units_digit_buf=-1;
	global_motor_trip_buf=-1;
	sub02_global_page_index_buf = -1;
	global_sub02_distance_unit_buf=-1;
	global_enter_status_pics=false;
	global_time_hour_tens_digit_buf=-1;
	global_time_hour_units_digit_buf=-1;
	global_time_min_tens_digit_buf=-1;
	global_time_min_units_digit_buf=-1;
}

/* Reset Battery Status�數
 * �數 : void
 * �傳 : void
 */

void reset_battery_status_page(void)
{
	global_ebike_light_buf=-1;
	global_ebike_bluetooth_buf=-1;
	battery_status_global_motor_assist_level_buf=-1;
	global_main_bms_soc_buf=-1;
	global_ex_bms_soc_buf=-1;
	battery_status_global_page_index_buf = -1;
	extra_battery_connected=false;
	global_enter_status_pics=false;	
	global_time_hour_tens_digit_buf=-1;
	global_time_hour_units_digit_buf=-1;
	global_time_min_tens_digit_buf=-1;
	global_time_min_units_digit_buf=-1;
}

/* Reset Walking Mode�數
 * �數 : void
 * �傳 : void
 */

void reset_walking_page(void)
{
	global_ebike_light_buf=-1;
	global_ebike_bluetooth_buf=-1;
	global_enter_walking=false;
	global_enter_status_pics=false;	
}

/* Reset Error Mode�數
 * �數 : void
 * �傳 : void
 */

void reset_error_page(void)
{
	global_ebike_light_buf=-1;
	global_ebike_bluetooth_buf=-1;
	global_enter_error=false;	
	global_enter_status_pics=false;
}

/* Reset FTM Mode�數
 * �數 : void
 * �傳 : void
 */

void reset_ftm_page(void)
{
	global_ebike_light_buf=-1;
	global_ebike_bluetooth_buf=-1;
	global_enter_ftm=false;
	global_enter_status_pics=false;
}	

/* Reset Charging Mode�數
 * �數 : void
 * �傳 : void
 */

void reset_charging_page(void)
{
	global_ebike_light_buf=-1;
	global_ebike_bluetooth_buf=-1; 
	global_charging_bms_soc_buf = -1;
	global_charging_bms_soc_level_buf=-1;
	global_battery_number_buf=-1;
	global_main_bms_soc_level_buf=-1;
	global_ex_bms_soc_level_buf=-1; 
	global_enter_charging=false;
	global_enter_status_pics=false;
	global_time_hour_tens_digit_buf=-1;
	global_time_hour_units_digit_buf=-1;
	global_time_min_tens_digit_buf=-1;
	global_time_min_units_digit_buf=-1;	
}

/* Reset Powert_Off Charging Mode�數
 * �數 : void
 * �傳 : void
 */

void reset_power_off_charging_page(void)
{
	global_power_off_bms_soc_buf = -1;
	global_enter_power_off_charging = false;	
}

/* Reset Pairing Mode�數
 * �數 : void
 * �傳 : void
 */

void reset_pair_page(void)
{
	global_ebike_light_buf=-1;
	global_ebike_bluetooth_buf=-1;
	global_dispaly_ble_passkey_code1_digit_buf = -1;
	global_dispaly_ble_passkey_code2_digit_buf = -1;
	global_dispaly_ble_passkey_code3_digit_buf = -1;
	global_dispaly_ble_passkey_code4_digit_buf = -1;
	global_dispaly_ble_passkey_code5_digit_buf = -1;
	global_dispaly_ble_passkey_code6_digit_buf = -1;
	global_enter_pair_mode=false;
	global_enter_status_pics=false;	
}

/* Reset Reconnect Mode�數
 * �數 : void
 * �傳 : void
 */

void reset_reconnect_page(void)
{
	global_ebike_light_buf=-1;
	global_ebike_bluetooth_buf=-1;
	global_enter_pair_mode=false;
	global_enter_status_pics=false;	
}

void radar_alert(void)
{
	if(ten_meter_alert_flag)
	{
		if(animation_fast_n > 9 ) animation_fast_n = 0;
		switch (animation_fast_n){
			case 0 :
				fw_set_gpio_pin( PIN_PWR_KEY_DET , 1 );
				break ;
			case 1 :
				fw_set_gpio_pin( PIN_PWR_KEY_DET , 1 );
				break ;
			case 2 :
				fw_set_gpio_pin( PIN_PWR_KEY_DET , 1 );
				break ;
			case 3 :
				fw_set_gpio_pin( PIN_PWR_KEY_DET , 1 );
				break ;
			case 4 :
				fw_set_gpio_pin( PIN_PWR_KEY_DET , 0 );
				break;
			case 5 :
				fw_set_gpio_pin( PIN_PWR_KEY_DET , 0 );
				break;
			case 6 :
				fw_set_gpio_pin( PIN_PWR_KEY_DET , 0 );
				break;
			case 7 :
				fw_set_gpio_pin( PIN_PWR_KEY_DET , 0 );
				break;
			case 8 :
				fw_set_gpio_pin( PIN_PWR_KEY_DET , 1 );
				break;
			case 9 :
				fw_set_gpio_pin( PIN_PWR_KEY_DET , 1 );
				break;
			default :
				break ;
			}
		show_small_pics(RADAR_QUICK_01_BLOCK +(RADAR_QUICK_02_BLOCK-RADAR_QUICK_01_BLOCK)*animation_fast_n ,60,217);
		animation_fast_n++;
		ten_meter_recover_flag = true;
	}

	if((ten_meter_recover_flag == true) && (ten_meter_alert_flag == false))
	{
		draw_partial(black, 200, 20, 60, 218);
		ten_meter_recover_flag = false;
		animation_fast_n = 0;
                nrf_gpio_pin_set(PIN_PWR_KEY_DET);	
                nrf_gpio_cfg_output(PIN_PWR_KEY_DET);	
	}
	if(fifty_meter_lert_flag)
	{
		for(int n=0; n<5; n++)
		{
			if(animation_slow_n > 9 ) animation_slow_n = 0;
				switch (animation_slow_n){
				case 0 :
					fw_set_gpio_pin( PIN_PWR_KEY_DET , 1 );
					break ;
				case 1 :
					fw_set_gpio_pin( PIN_PWR_KEY_DET , 1 );
					break ;
				case 2 :
					fw_set_gpio_pin( PIN_PWR_KEY_DET , 1 );
					break ;
				case 3 :
					fw_set_gpio_pin( PIN_PWR_KEY_DET , 1 );
					break ;
				case 4 :
					fw_set_gpio_pin( PIN_PWR_KEY_DET , 1 );
					break;
				case 5 :
					fw_set_gpio_pin( PIN_PWR_KEY_DET , 0 );
					break;
				case 6 :
					fw_set_gpio_pin( PIN_PWR_KEY_DET , 0 );
					break;
				case 7 :
					fw_set_gpio_pin( PIN_PWR_KEY_DET , 0 );
					break;
				case 8 :
					fw_set_gpio_pin( PIN_PWR_KEY_DET , 0 );
					break;
				case 9 :
					fw_set_gpio_pin( PIN_PWR_KEY_DET , 0 );
					break;
				default :
					break ;
				}
			show_small_pics(RADAR_SLOW_01_BLOCK +(RADAR_SLOW_02_BLOCK-RADAR_SLOW_01_BLOCK)*animation_slow_n ,60,217);
			animation_slow_n++;
			delay_msec(40);
			fifty_meter_recover_flag = true;
		}
	}

	if((fifty_meter_recover_flag == true) && (fifty_meter_lert_flag == false))
	{
		draw_partial(black, 200, 20, 60, 218);
		fifty_meter_recover_flag = false;
		animation_slow_n = 0;
                nrf_gpio_pin_set(PIN_PWR_KEY_DET);	
                nrf_gpio_cfg_output(PIN_PWR_KEY_DET);	
	}       
}

void alert_five_times(void)
{
	int n,time;
	for(time =0;time<20;time++)
	{
		for(n=0;n<10;n++)
		{
			show_small_pics(RADAR_QUICK_01_BLOCK +(RADAR_QUICK_02_BLOCK-RADAR_QUICK_01_BLOCK)*n ,60,217);
			delay_msec(40);
			if(n==0)
			{
				fw_set_gpio_pin( PIN_PWR_KEY_DET , 1 );
			}
			else if(n==1)
			{
				fw_set_gpio_pin( PIN_PWR_KEY_DET , 1 );
			}
			else if(n==2)
			{
				fw_set_gpio_pin( PIN_PWR_KEY_DET , 1 );
			}
			else if(n==3)
			{
				fw_set_gpio_pin( PIN_PWR_KEY_DET , 1 );
			}
			else if(n==4)
			{
				fw_set_gpio_pin( PIN_PWR_KEY_DET , 0 );
			}
			else if(n==5)
			{
				fw_set_gpio_pin( PIN_PWR_KEY_DET , 0 );
			}
			else if(n==6)
			{
				fw_set_gpio_pin( PIN_PWR_KEY_DET , 0 );
			}
			else if(n==7)
			{
				fw_set_gpio_pin( PIN_PWR_KEY_DET , 0 );
			}
			else if(n==8)
			{
				fw_set_gpio_pin( PIN_PWR_KEY_DET , 1 );
			}
			else if(n==9)
			{
				fw_set_gpio_pin( PIN_PWR_KEY_DET , 1 );
			}	
		}
	}
	draw_partial(black, 200, 20, 60, 218);	
        nrf_gpio_pin_set(PIN_PWR_KEY_DET);	
        nrf_gpio_cfg_output(PIN_PWR_KEY_DET);	
}
		
		
	



/* 讀������
 * �數 : 記憶體䀁X座��Y座�
 * �傳 : void
 */

void show_small_pics(long addr, uint16_t x, uint16_t y)
{
	 fw_flash_read(addr , 4 , pic_info );
	 width = ((int)pic_info[0] << 8) | pic_info[1] ;
	 height = ((int)pic_info[2] << 8) | pic_info[3] ;
	 if(width>320||width<0||height>240||height<0)
	 {
	  global_pic_error_flag =1 ;
	 }

	 uint8_t *pics_line_buf = malloc(18432);

	 if((width*height*2)>18432)
	 {
		desc.buf_size = width*height;
		desc.height = height/2;
		desc.width = width;
		desc.pitch = width;
		if(global_pic_error_flag ==0)
		{
			for(int i=0;i<2;i++)
			{
				fw_flash_read((addr + 4 + (width*(height/2)*2)*i), (width*(height/2)*2), pics_line_buf);
				display_write(st7789v_devs,x,(y+(height/2)*i),&desc,pics_line_buf);
			}
		}			 
	 }
	 else
	 {
		desc.buf_size = width*height*2;
		desc.height = height ;
		desc.width = width ;
		desc.pitch = width ;

		 if(global_pic_error_flag == 0)
		 {
			fw_flash_read(addr + 4 , (width*height *2), pics_line_buf);
			display_write(st7789v_devs,x,y,&desc,pics_line_buf);
		 }
	 }
	 free(pics_line_buf);
}

/* 讀����
 * �數 : 記憶體䀁X座��Y座�
 * �傳 : void
 */

void show_big_pics(long addr, uint16_t x, uint16_t y)
{
	fw_flash_read(addr , 4 , pic_info );
	width = ((int)pic_info[0] << 8) | pic_info[1] ;
	height = ((int)pic_info[2] << 8) | pic_info[3] ;
	if(width>320||width<0||height>240||height<0)
	{
		global_pic_error_flag =1 ;
	}
	desc.buf_size = width*(height/6)*2;
	desc.height = height/6;
	desc.width = width;
	desc.pitch = width;

	uint8_t *pics_line_buf = malloc(25600) ;

	if(global_pic_error_flag ==0)
	{
		for(int i=0;i<6;i++)
		{
			fw_flash_read((addr + 4 + (width*(height/6)*2)*i), (width*(height/6)*2), pics_line_buf);
			display_write(st7789v_devs,x,(y+(height/6)*i),&desc,pics_line_buf);
		}
	}
	free(pics_line_buf);	
}

/* 將整�填滿���
 * �數 : RGB565 顏色仢�
 * �傳 : void
 */

void draw_page(uint16_t color)
{
	uint16_t *picture_pics = malloc(12800) ;
	desc.buf_size = 25600;
	desc.height = 40;
	desc.width = 320;
	desc.pitch = 320;
	
	for(int i=0;i<12800;i++)
	{ 
		picture_pics[i]=color;
	}	

	for(int i=0;i<6;i++)
	{
		display_write(st7789v_devs,0,40*i,&desc,picture_pics);
	}
	free(picture_pics);	
}


/* 將��個�置填滿���
 * �數 : �數 : RGB565 顏色仢����寬�����X座��Y座�
 * �傳 : void
 */

void draw_partial(uint16_t color, int width, int height, int x, int y)
{
	desc.buf_size = width*2;
	desc.height = 1;
	desc.width = width;
	desc.pitch = width;
	uint16_t picture_partial[width];
	
	for(int i=0;i<width;i++)
	{ 
		picture_partial[i]=color;
	}	

	for(int i=0;i<height;i++)
	{
		display_write(st7789v_devs,x,i+y,&desc,picture_partial);
	}	
}



/**
* @def DISP_PRIORITY Display the priority of the itinerary (the smaller the value, the higher the priority)
*/
#define DISP_PRIORITY 6
#define DISPLAY_STACK_SIZE 4096
//K_THREAD_DEFINE(lcd_display_id, DISPLAY_STACK_SIZE, thread_for_display, NULL, NULL, NULL, DISP_PRIORITY, 0, 0);
