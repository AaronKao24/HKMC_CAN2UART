/*******************************************************************************
* File Name       : canfd_process.h
* Description     : 
* Original Author : Sammy.hh.chen
* Created on      : Jan 11, 2023
*******************************************************************************/

#ifndef CANFD_PROCESS_H
#define	CANFD_PROCESS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#define E_CAN_TX_MAX 32
#define TENS_DIGIT(t, u , v)  				do {if(v==0){*t=*u=0;return;}uint8_t tmp=v%10;*u=tmp;tmp=v/10%10;*t=tmp;} while (0)
#define HUNDRES_DIGIT(h, t, u , v)  		do {if(v==0){*h=*t=*u=0;return;} \
									uint8_t tmp=v/100%10;*h=tmp;TENS_DIGIT(t, u, v);} while (0)
#define THOUSAND_DIGIT(th, h, t, u , v)  	do {if(v==0){*th=*h=*t=*u=0;return;} \
									uint8_t tmp=v/1000%10;*th=tmp;HUNDRES_DIGIT(h, t, u, v);} while (0)
#define TENS_THOUSAND_DIGIT(tth, th, h, t, u , v)  do {if(v==0){*tth=*th=*h=*t=*u=0;return;} \
									uint8_t tmp=v/10000%10;*tth=tmp;THOUSAND_DIGIT(th, h, t, u, v);} while (0)
									
#define H_THOUSAND_DIGIT(htth, tth, th, h, t, u , v)  do {if(v==0){*htth=*tth=*th=*h=*t=*u=0;return;} \
									uint8_t tmp=v/100000%10;*htth=tmp;TENS_THOUSAND_DIGIT(tth, th, h, t, u, v);} while (0)
									
#define BMS_BAR_CONVERSION(soc, v)  		    do {if(soc==0){*v=0;return;}else if(soc>0 && soc <=16){*v=1;} \
																			else if(soc>16 && soc <=28){*v=2;}\
																			else if(soc>28 && soc <=40){*v=3;}\
																			else if(soc>40 && soc <=52){*v=4;}\
																			else if(soc>52 && soc <=64){*v=5;}\
																			else if(soc>64 && soc <=76){*v=6;}\
																			else if(soc>76 && soc <=88){*v=7;}\
																			else if(soc>88 && soc <=100){*v=8;}} while (0)
#define WALK_ASSIST_LEVEL_MAX 5
#define WALK_ASSIST_LEVEL_MIN 0

/*	FTM	*/
typedef void (*fit_canfd_ftm)(uint8_t *);
typedef void (*fit_canfd_motor_upgrade)(uint8_t *);


/*	CAN DATA PROTOCOL	*/
///////////////////////////////////////////////////////////////////////////////
/*	Motor Upgrade PROTOCOL	*/
/*	005h	*/
typedef struct {
	uint8_t M_key1			:8;
	uint8_t M_key2			:8;
	uint8_t M_key3			:8;
	uint8_t M_key4			:8;
	uint8_t M_key5			:8;
	uint8_t M_key6			:8;
	uint8_t M_key7			:8;
	uint8_t M_key8			:8;
}S_WR_CONTROLLER_ERASE_CMD;

/*	006h	*/
typedef struct {
	uint8_t Cst				:8;
	uint8_t Result			:8;
	uint64_t unused			:48;
}S_CONTROLLER_ERASE_CMD_ACK;

/*	007h	*/
typedef struct {
	uint8_t Data_01			:8;
	uint8_t Data_02			:8;
	uint8_t Data_03			:8;
	uint8_t Data_04			:8;
	uint8_t Data_05			:8;
	uint8_t Data_06			:8;
	uint8_t Data_07			:8;
	uint8_t Data_08			:8;
}S_CONTROLLER_UPDATE_DATA;

/*	008h	*/
typedef struct {
	uint8_t Cst				:8;
	uint8_t Result			:8;
	uint64_t unused			:48;
}S_CONTROLLER_UPDATE_ACK;

/*	009h	*/
typedef struct {
	uint64_t unused;
}S_CONTROLLER_UPDATE_FINISH;

/*	00Ah	*/
typedef struct {
	uint8_t Cst				:8;
	uint8_t Result			:8;
	uint64_t unused			:48;
}S_CONTROLLER_UPDATE_FINISH_ACK;

///////////////////////////////////////////////////////////////////////////////

/*	300h	*/
typedef struct {
	uint8_t Mode			:8;
	uint8_t Error_status	:8;
	uint64_t unused			:48;
}S_RD_HMI_PERIODICAL_FRAME;

/*	301h	*/
typedef struct {
	uint16_t HMI_fw_main_version;
	uint16_t HMI_fw_sec_version;
	uint16_t HMI_fw_debug_serial;
	uint8_t HMI_hw_main_version;
	uint8_t Brand;
}S_RD_HMI_VERSION;

/*	302h	*/
typedef struct {
	uint32_t Serial_number;
	uint8_t Manufacture_code;
	uint8_t PD_D;
	uint8_t PD_M;
	uint8_t PD_Y;
}S_RD_HMI_SN_PRODUCTION;

/* Bypass_Mode*/
typedef struct {
	uint8_t Send_Data1		:8;
	uint8_t Send_Data2		:8;
	uint8_t Send_Data3		:8;
	uint8_t Send_Data4		:8;
	uint8_t Send_Data5		:8;
	uint8_t Send_Data6		:8;
	uint8_t Send_Data7		:8;
	uint8_t Send_Data8		:8;
}S_WR_BYPASS_MODE_CMD;


/*	283h-2A3h	*/
typedef struct {
	uint16_t Total_battery;
	int16_t CDC_current;
	uint8_t BMS_Status;
	int8_t Cell_temp;
	uint8_t State_of_Charge;
}S_BMS_PERIODICAL_FRAME;

/*	284h-2A4h	*/
typedef struct {
	uint16_t BMS_warning_alarm;
}S_RD_BMS_WARNING_ALARM;

/*	285h-2A5h	*/
typedef struct {
	uint32_t Total_Discharge_Capacity;
	uint16_t Cycle_Count;
	uint16_t Full_Charged_Capacity;
}S_RD_BMS_DISCHARGE_STATUS;

/*	2C1h	*/
typedef struct {
	uint8_t RTC_YEAR;
	uint8_t RTC_MONTH;
	uint8_t RTC_DAY;
	uint8_t RTC_HOUR;
	uint8_t RTC_MIN;
	uint8_t RTC_SEC;
}S_RD_BMS_REAL_TIME;

/*	280h	*/
typedef struct {
	uint16_t BMS_fw_main_version;
	uint16_t BMS_fw_sec_version;
	uint16_t BMS_fw_debug_serial;
	uint8_t client_Number;
	uint8_t Brand;
}S_RD_BMS_FW_VERSION;

/*	282h	*/
typedef struct {
	uint32_t Serial_number;
	uint8_t Manufacture_code;
	uint8_t PD_D;
	uint8_t PD_M;
	uint8_t PD_Y;
}S_RD_BMS_SN_PRODUCTION;

typedef struct {
	S_RD_BMS_FW_VERSION		bms_fw_ver;					
	S_RD_BMS_SN_PRODUCTION		bms_SN_Production;			
	S_BMS_PERIODICAL_FRAME 		main_bms_periodical_frame;	
	S_RD_BMS_WARNING_ALARM 		main_bms_warning_alarm;
	S_RD_BMS_DISCHARGE_STATUS	main_discharge_status;
	S_BMS_PERIODICAL_FRAME 		ex_bms_periodical_frame;
	S_RD_BMS_WARNING_ALARM 		ex_bms_warning_alarm;
	S_RD_BMS_DISCHARGE_STATUS	ex_discharge_status;

	S_RD_BMS_REAL_TIME			bms_real_time;
}S_BMS_INFO;

/*	100h	*/
typedef struct {
	uint32_t MC_Warning_alarm	:32;
	uint32_t unused				:32;
}S_MC_WARNING_ALARM;

/*	101h	*/
typedef struct {
	uint8_t M_KEY_1	:8;
	uint8_t M_KEY_2	:8;
	uint8_t M_KEY_3	:8;
	uint8_t M_KEY_4	:8;
	uint32_t unused	:32;
}S_WAKE_UP;

/*	102h	*/
typedef struct {
	uint8_t M_KEY_1	:8;
	uint8_t M_KEY_2	:8;
	uint8_t M_KEY_3	:8;
	uint8_t M_KEY_4	:8;
	uint32_t unused	:32;
}S_SHUT_DOWN;

/*	103h	*/
typedef struct {
	uint8_t M_KEY_1	:8;
	uint8_t M_KEY_2	:8;
	uint8_t M_KEY_3	:8;
	uint8_t M_KEY_4	:8;
	uint8_t M_KEY_5	:8;
	uint8_t M_KEY_6	:8;
	uint32_t unused	:16;
}S_UNLOCK_SYSTEM;

/*	103h return	*/
typedef struct {
	uint8_t Success	:8;
	uint64_t unused	:56;
}S_RETURN_UNLOCK_SYSTEM;

/*	104h	*/
typedef struct {
	uint8_t M_KEY_1	:8;
	uint8_t M_KEY_2	:8;
	uint8_t M_KEY_3	:8;
	uint8_t M_KEY_4	:8;
	uint32_t unused	:32;
}S_LOCK_SYSTEM;

/*	110h	*/
typedef struct {
	uint8_t HW_Type_H;
	uint8_t HW_Type_L;
	uint8_t MC_FW_main_version_H;
	uint8_t MC_FW_main_version_L;
	uint8_t MC_FW_sec_version_H;
	uint8_t MC_FW_sec_version_L;
	uint8_t CN;
	uint8_t MODEL;
}S_RD_MC_FW_VERSION;

/*	113h	*/
typedef struct {
	uint8_t SN_L_01;
	uint8_t SN_L_02;
	uint8_t SN_L_03;
	uint8_t SN_L_04;
	uint8_t SN_L_05;
	uint8_t SN_L_06;
	uint8_t SN_L_07;
	uint8_t SN_L_08;
}S_RD_MC_SERIAL_NO_LOW;

/*	114h	*/
typedef struct {
	uint8_t SN_H_01;
	uint8_t SN_H_02;
	uint8_t SN_H_03;
	uint8_t SN_H_04;
	uint8_t SN_H_05;
	uint8_t SN_H_06;
	uint8_t SN_H_07;
	uint8_t SN_H_08;
}S_RD_MC_SERIAL_NO_HIGH;

/*	117h	*/
typedef struct {
	uint8_t  MCOD;
	uint16_t Production_date_day;
	uint16_t Production_date_month;
	uint16_t Production_date_year;	
	uint8_t unused;
}S_RD_MC_PRODUCTION_INFO;

/*	13Dh	*/
typedef struct {
	uint64_t unused	:56;
	uint8_t  M_KEY	:8;	
}S_WR_MC_CLEAR_TRIP;

/*	13Eh	*/
typedef struct {
	int8_t ASSIST	:8;
	uint64_t unused	:48;
	uint8_t M_KEY	:8;
}S_WR_MC_ASSIST_SETTING;

/*	13Fh	*/
typedef struct {
	uint8_t UINT	:8;
	uint64_t unused	:48;
	uint8_t M_KEY	:8;
}S_WR_MC_UINT_SETTING;

/*	182h	*/
typedef struct {
	uint16_t UIN;
	uint16_t IIN;
	uint16_t HPO;
	uint8_t CRPM;
	uint8_t Human_Torque;
}S_RD_MC_ACTUAL_OPERATION_DATA;

/*	183h	*/
typedef struct {
	uint16_t RESTD	:16;
	uint16_t RESTT	:16;
	uint32_t unused	:32;
}S_RD_REMAINING_DATA;

/*	184h	*/
typedef struct {
	uint16_t TRIPK;
	uint16_t MRPM;
	uint16_t HPO;
	uint16_t MPO;
}S_RD_REAL_TIME_DATA;

/*	18Ah	*/
typedef struct {
	uint16_t Speed;
	uint16_t TRIP;
	uint16_t ODO;
	int8_t MODE;
	uint8_t Status;
}S_MC_PERIODICAL_FRAME;

/*	1AFh	*/
typedef struct {
	uint8_t Throttle :8;
	uint8_t TORQUE	 :8;
	uint8_t SPEEDS   :8;
	uint8_t BRKS     :8;
	uint32_t unused	 :32;
}S_RD_MC_SNR_INPUT;

/*	1B0h	*/
typedef struct {
	uint8_t Throttle :8;
	uint8_t TORQUE   :8;
	uint8_t SPEEDS   :8;
	uint8_t BRKS     :8;
	uint32_t unused	 :24;
	uint8_t M_KEY	 :8;
}S_WR_MC_SNR_SETTING;

/*	1C2h	*/
typedef struct {
	uint8_t HLON	:8;
	uint64_t unused	:48;
	uint8_t M_KEY	:8;
}S_WR_MC_HEAD_LIGHT;

/*	1CCh	*/
typedef struct {
	uint8_t WALK	:8;
	uint64_t unused	:48;
	uint8_t M_KEY	:8;
}S_WR_MC_WALK_ASSIST;

/*	7e3h	*/
typedef struct {
	uint8_t Mode	:8;
	uint8_t CMD		:8;
	uint8_t status	:8;
	uint8_t M_KEY_1	:8;
	uint8_t M_KEY_2	:8;	
}S_DIAG_STATE;

/*	7e3h	*/
typedef struct {
	uint8_t Mode	:8;
	uint8_t CMD		:8;
	uint8_t status	:8;
	uint8_t M_KEY_1	:8;
	uint8_t M_KEY_2	:8;	
}S_DIAG_COMMAND;


/*	7e3h	*/
typedef struct {
	uint8_t Mode	:8;
	uint8_t CMD		:8;
	uint16_t SW		:16;
	uint16_t HW		:16;

}S_DIAG_HWSW_VERSION;

/*	7e3h	*/
typedef struct {
	uint8_t Mode	:8;
	uint8_t CMD		:8;
	uint16_t PID0	:16;
	uint16_t PID1	:16;
	uint16_t PID2	:16;
}S_DIAG_PCBA_PID;

/*	7e3h	*/
typedef struct {
	uint8_t Mode	:8;
	uint8_t CMD		:8;
	uint16_t PID0	:16;
	uint16_t PID1	:16;
	uint16_t PID2	:16;
}S_DIAG_SET_PID;


/*	7e3h	*/
typedef struct {
	uint8_t Mode	:8;
	uint8_t CMD		:8;
	uint16_t Header	:8;
	uint16_t PID1	:16;
	uint16_t End	:8;
}S_DIAG_FLASH_TEST;

/*	7e3h	*/
typedef struct {
	uint8_t Mode	:8;
	uint8_t CMD		:8;
	uint8_t Pattern	:8;
}S_DIAG_LCM_TEST;

/*	7e3h	*/
typedef struct {
	uint8_t Mode	:8;
	uint8_t CMD		:8;
	uint16_t CHK	:16;
}S_DIAG_CANFD_TEST;


/*	7e3h	*/
typedef struct {
	uint8_t Mode	:8;
	uint8_t CMD		:8;
	uint8_t Result	:8;
	uint8_t PeakPWR	:8;
	uint8_t RSSI	:8;

}S_DIAG_BLE_TEST;

/*	7e3h	*/
typedef struct {
	uint8_t Mode	:8;
	uint8_t CMD		:8;
	uint16_t CHK	:16;
}S_DIAG_KEY_CODE;

/*	7e3h	*/
typedef struct {
	uint8_t Mode	:8;
	uint8_t CMD		:8;
	uint16_t CHK	:16;
}S_DIAG_LCM_BACKLIGHT;


/*	7e3h	*/
typedef struct {
	uint8_t Mode	:8;
	uint8_t CMD		:8;
	uint16_t MAC0	:16;
	uint16_t MAC1	:16;
	uint16_t MAC2	:16;
}S_DIAG_MAC_ADDRESS;

typedef enum {
	E_DBG_MESSAGE_BOOT 			= 0x01,
	E_DBG_MESSAGE_SHUTDOWN		= 0x02,
	E_DBG_MESSAGE_USB_PLUG_IN	= 0x03,
	E_DBG_MESSAGE_USB_PLUG_OUT	= 0x04,
	E_DBG_MESSAGE_BLE_CONNECTED = 0x05,
	E_DBG_MESSAGE_BLE_DISCONNECT= 0x06,
	E_DBG_MESSAGE_LIGHT_ON		= 0x07,
	E_DBG_MESSAGE_LIGHT_OFF		= 0x08,
	E_DBG_MESSAGE_POWER_SAVING	= 0x09,
	
	E_DBG_MESSAGE_PAGE_MAIN				= 0x10,
	E_DBG_MESSAGE_PAGE_SUB				= 0x11,
	E_DBG_MESSAGE_PAGE_SUB2				= 0x12,
	E_DBG_MESSAGE_PAGE_WALK_ASSIST		= 0x13,
	E_DBG_MESSAGE_PAGE_CHARGING			= 0x14,
	E_DBG_MESSAGE_PAGE_LOCK				= 0x15,
	E_DBG_MESSAGE_PAGE_BLE_CONNECTING	= 0x16,
	E_DBG_MESSAGE_PAGE_MOTOR_ERR		= 0x17,
	E_DBG_MESSAGE_PAGE_M_BMS_ERR		= 0x18,
	E_DBG_MESSAGE_PAGE_E_BMS_ERR		= 0x19,
	E_DBG_MESSAGE_PAGE_FTM				= 0x1A,
	E_DBG_MESSAGE_PAGE_HMI_ERR			= 0x1B,
	
	E_DBG_MESSAGE_BTN_ASSIST_INCREASE		= 0x20,
	E_DBG_MESSAGE_BTN_ASSIST_DECREASE		= 0x21,
	E_DBG_MESSAGE_BTN_SCRENN_CROLL			= 0x22,
	E_DBG_MESSAGE_BTN_POWER					= 0x23,

        E_DBG_MESSAGE_UNIT_KM                   = 0x24,
        E_DBG_MESSAGE_UNIT_MILE                 = 0x25,
}E_DBG_MESSAGE_ID;
	
/*	320h	*/
typedef struct {
	uint8_t MSG_ID	:8;
	uint8_t MSG_PL0	:8;
	uint8_t MSG_PL1	:8;
	uint8_t MSG_PL2	:8;
	uint8_t MSG_PL3	:8;
	uint8_t MSG_PL4	:8;
	uint8_t MSG_PL5	:8;
	uint8_t MSG_PL6	:8;
}S_DBG_MESSAGE;


typedef struct {
	S_MC_WARNING_ALARM		mc_warrning_alarm;
	S_MC_PERIODICAL_FRAME	mc_periodical_frame;
	S_RD_REMAINING_DATA		rd_remainging_data;
	S_RD_MC_SNR_INPUT       mc_snr_input;
	S_RD_MC_FW_VERSION		mc_fw_ver;
	S_RD_MC_SERIAL_NO_LOW		mc_sn_low;
	S_RD_MC_SERIAL_NO_HIGH          mc_sn_high;
	S_RD_MC_PRODUCTION_INFO		mc_Production;
}S_MOTOR_INFO;

//Wade231120: For mmWave receive (CAN std: 8 byte)+++
typedef struct {
        //byte 0
        uint8_t   RR_St           :3;
	uint8_t   RR_AudioWarn    :2;
        uint8_t   RR_LedWarn      :3;
        //byte 1
        uint8_t   RR_DOWSt        :2;
        uint8_t   RR_BSDSt        :2;
        uint8_t   RR_LCASt        :2;
        uint8_t   RR_RCWSt        :2;
        //byte 2
        uint8_t   RR_RCTASt       :2;
        uint8_t   RR_PASSt        :2;
        uint8_t   RR_DOWWarn      :2;
        uint8_t   RR_BSDWarn      :2;
        //byte 3
        uint8_t   RR_LCAWarn      :2;
        uint8_t   RR_RCWWarn      :2;
        uint8_t   RR_RCTAWarn     :2;
        uint8_t   RR_PAWarn       :2;
        //byte 4~7
        uint32_t  unused          :32;

}S_RR_WARN; //374h

typedef struct {
        S_RR_WARN               rr_warn;

}S_MMWAVE_INFO;
//Wade231108: For mmWave---

typedef struct {
	S_BMS_INFO		bms;
	S_MOTOR_INFO	motor;
        S_MMWAVE_INFO   mmwave; //Wade231120: For mmWave
}S_ALL_INFO;


/*	CAN TX */
typedef enum {
  E_CANBUS_TX_ID_005 	= 0x005,
  E_CANBUS_TX_ID_007 	= 0x007,
  E_CANBUS_TX_ID_009 	= 0x009,
  E_CANBUS_TX_ID_102 	= 0x102,
  
  E_CANBUS_TX_ID_103	= 0x103,
  E_CANBUS_TX_ID_104	= 0x104,
  E_CANBUS_TX_ID_110	= 0x110,
  E_CANBUS_TX_ID_113	= 0x113,
  E_CANBUS_TX_ID_114	= 0x114,
  E_CANBUS_TX_ID_117	= 0x117,
  E_CANBUS_TX_ID_13D	= 0x13D,
  E_CANBUS_TX_ID_13E	= 0x13E,
  E_CANBUS_TX_ID_13F	= 0x13F,
  E_CANBUS_TX_ID_183	= 0x183,
  E_CANBUS_TX_ID_1AF	= 0x1AF,
  E_CANBUS_TX_ID_1B0	= 0x1B0,
  E_CANBUS_TX_ID_1C2	= 0x1C2,
  E_CANBUS_TX_ID_1CC	= 0x1CC,
  E_CANBUS_TX_ID_280	= 0x280,
  E_CANBUS_TX_ID_282	= 0x282,
  E_CANBUS_TX_ID_285	= 0x285,
  E_CANBUS_TX_ID_2A5	= 0x2A5,
  E_CANBUS_TX_ID_2C1	= 0x2C1,
  E_CANBUS_TX_ID_300	= 0x300,
  E_CANBUS_TX_ID_301	= 0x301,
  E_CANBUS_TX_ID_302	= 0x302,
  E_CANBUS_TX_ID_320	= 0x320,
  E_CANBUS_TX_ID_NONE = 0xffffffff,
}E_CANBUS_TX_ID; 

typedef union {
    uint64_t data;
    uint8_t data8_t[8];
   	S_WR_MC_UINT_SETTING uint_set;
	S_WR_MC_CLEAR_TRIP clear_trip;
	S_WR_MC_ASSIST_SETTING assist_set;
	S_WR_MC_HEAD_LIGHT	head_light;
	S_WR_MC_WALK_ASSIST	walk_assist;
	S_RETURN_UNLOCK_SYSTEM return_unlock_success;
	S_LOCK_SYSTEM	return_lock;
	S_SHUT_DOWN shutdown;
	S_UNLOCK_SYSTEM unlock_sys;
	S_LOCK_SYSTEM lock_sys;
	S_RD_HMI_VERSION hmi_version;
	S_RD_HMI_SN_PRODUCTION hmi_sn_production;
	S_RD_REMAINING_DATA	remaining_data;
	S_RD_MC_SNR_INPUT       mc_snr_input;
	S_RD_HMI_PERIODICAL_FRAME	hmi_periodical;
	S_WR_MC_SNR_SETTING	snr_setting;

	/*	FTM Mode	*/
	S_DIAG_STATE diag_state;	
	S_DIAG_COMMAND diag_command;
	S_DIAG_HWSW_VERSION diag_hwsw_version;
	S_DIAG_PCBA_PID diag_pcba_pid;
	S_DIAG_SET_PID diag_set_pid;
	S_DIAG_FLASH_TEST diag_flash_test;
	S_DIAG_LCM_TEST	diag_lcm_test;
	S_DIAG_CANFD_TEST diag_canfd_test;
	S_DIAG_BLE_TEST diag_ble_test;
	S_DIAG_KEY_CODE	diag_key_code;
	S_DIAG_LCM_BACKLIGHT diag_lcm_backlight;
	S_DIAG_MAC_ADDRESS diag_mac_address;

	/*	Debug Message	*/
	S_DBG_MESSAGE DBG_message;
	
	/*	Motor Upgrade	*/
	S_WR_CONTROLLER_ERASE_CMD 	motor_erase_cmd;
	S_CONTROLLER_UPDATE_DATA	motor_update_data_cmd;
	S_CONTROLLER_UPDATE_FINISH	motor_update_finish_cmd;
	S_WR_BYPASS_MODE_CMD		bypass_mode_cmd;
	
} U_CAN_DATA;

typedef struct {
	E_CANBUS_TX_ID tx_id;
	U_CAN_DATA payload;
}S_CANFD_PROCESS_BUFF;

/*	State machine	*/
typedef enum {
  E_CANBUS_STATE_INIT,
  E_CANBUS_STATE_IDLE,
  E_CANBUS_STATE_TX,
  E_CANBUS_STATE_RX,
  E_CANBUS_STATE_DONE,
}E_CANBUS_STATE; 

/*	Ring buffer	*/
typedef void (*fit_ring_init)(void);
typedef uint8_t (*fit_ring_put)(uint8_t *);
typedef uint8_t (*fit_ring_get)(uint8_t *);
typedef uint8_t (*fit_ring_isNotProcessed)(void);
typedef uint8_t (*fit_ring_isfull)(void);

typedef struct {
	S_CANFD_PROCESS_BUFF buf[E_CAN_TX_MAX];
	uint8_t *pIn;
	uint8_t *pOut;
	uint8_t *pEnd;
	uint8_t full;
}S_CANFD_PROCESS_RING;

typedef struct {
	fit_ring_init 	ring_init;
	fit_ring_put	ring_put;
	fit_ring_get	ring_get;
	fit_ring_isNotProcessed	ring_isNotProcessed;
	fit_ring_isfull	ring_isFull;
	S_CANFD_PROCESS_BUFF ring_get_buff[1];
}S_CANFD_PROCESS_RING_FUNC;

/*	Monitor	*/
typedef struct {	
	uint8_t M_CANID_WARRING_ALARM_100;
	uint8_t M_CANID_WARRING_ALARM_100_GENERAL_ERROR;
	uint8_t M_CANID_WARRING_ALARM_100_GENERAL_FW_ERROR;
	uint8_t	M_CANID_EX_BMS_2A3;
	uint8_t	M_CANID_EX_BMS_DETECT_2A3;
	uint8_t	M_CANID_EX_BMS_DETECT_18A;
	uint8_t	M_CANID_EX_BMS_283;
	uint8_t	M_CANID_EX_BMS_DETECT_283;
	uint8_t M_CANID_WARRING_ALARM_284;
	uint8_t M_CANID_WARRING_ALARM_2A4;
	uint8_t	M_CANID_SHUTDOWN_102;
	uint8_t	M_CANID_UNLOCK_SYS_103;
	uint8_t	M_CANID_LOCK_SYS_104;
	uint8_t	M_CANID_WALK_ASSIST_1CC;
}S_MONITOR;

//------------------------------------------------------------------------------
void FIT_CNAFD_PROCESS_Init(void);
void FIT_CANFD_PROCESS_Tranmit(uint8_t *id);
void FIT_CANFD_PROCESS_RING_REGISTER(uint8_t *data);
void FIT_CANFD_PROCESS_FTM_REGISTER(fit_canfd_ftm cb);
void FIT_CANFD_PROCESS_MOTOR_UPGRADE_REGISTER(fit_canfd_motor_upgrade cb);
void FIT_CANFD_PROCESS_MOTOR_Speed_Get(uint8_t *ten_d, uint8_t *uints_d);
void FIT_CANFD_PROCESS_MOTOR_TRIP_Get(uint8_t *h_d, uint8_t *ten_d, uint8_t *uints_d);
void FIT_CANFD_PROCESS_MOTOR_ODO_Get( uint8_t *tens_th_d, uint8_t *th_d, uint8_t* h_d,  
												    uint8_t *ten_d, uint8_t *uints_d);
int8_t FIT_CANFD_PROCESS_MOTOR_MODE_Get(void);
uint8_t FIT_CANFD_PROCESS_MOTOR_STATUS_Get(void);
uint8_t FIT_CANFD_PROCESS_MOTOR_STATUS_LOCK_Get(void);
uint8_t FIT_CANFD_PROCESS_MOTOR_STATUS_BRAKE_Get(void);
uint8_t FIT_CANFD_PROCESS_MOTOR_STATUS_LIGHT_Get(void);
uint8_t FIT_CANFD_PROCESS_MOTOR_STATUS_REGENERATION_Get(void);
uint8_t FIT_CANFD_PROCESS_MOTOR_STATUS_ERROR_Get(void);
uint8_t FIT_CANFD_PROCESS_MOTOR_STATUS_UINT_MILE_Get(void);
uint8_t FIT_CANFD_PROCESS_MOTOR_STATUS_DERATING_Get(void);
uint8_t FIT_CANFD_PROCESS_MOTOR_STATUS_WALING_BOSST_Get(void);
uint8_t FIT_CANFD_PROCESS_MAIN_BMS_STATUS_CHARGING_Get(void);

void FIT_CANFD_PROCESS_REMAINING_DISTANCE_Get(uint8_t *h_d, uint8_t *ten_d, uint8_t *uints_d);

uint8_t* FIT_CANFD_PROCESS_MONITOR_Get(void);
S_ALL_INFO* FIT_CNAFD_PROCESS_Data_Get(void);
void FIT_CANFD_PROCESS_BMS_MIX_SoC_Get(uint8_t *h_d, uint8_t *ten_d, uint8_t *uints_d, uint8_t *grid);
void FIT_CANFD_PROCESS_BMS_MAIN_SoC_Get(uint8_t *h_d, uint8_t *ten_d, uint8_t *uints_d, uint8_t *grid);
void FIT_CANFD_PROCESS_BMS_EX_SoC_Get(uint8_t *h_d, uint8_t *ten_d, uint8_t *uints_d, uint8_t *grid);

void FIT_CANFD_PROCESS_SHUTDOWN(void);
void FIT_CANFD_PROCESS_MAIN_BMS_DISCHARGE_Send(void);
void FIT_CANFD_PROCESS_MAIN_BMS_CYCLE_COUNT_Get(uint16_t *bms_cc);
void FIT_CANFD_PROCESS_EX_BMS_DISCHARGE_Send(void);
void FIT_CANFD_PROCESS_UNLOCK_Send(void);
void FIT_CANFD_PROCESS_LOCK_Send(void);
void FIT_CANFD_PROCESS_BMS_REAL_TIME_Send(void);
void FIT_CANFD_PROCESS_BMS_REAL_TIME_Get(uint8_t *s_tens_hour, uint8_t *s_u_hour,uint8_t *s_tens_mins	, uint8_t *s_u_mins);

void FIT_CANFD_PROCESS_WALK_ASSIST_Send(uint8_t isActivate);
void FIT_CANFD_PROCESS_LIGHT_Send(uint8_t isActivate);
void FIT_CANFD_PROCESS_SNR_INPUT_Send(void);
uint8_t FIT_CANFD_PROCESS_SNR_INPUT_Get(void);
void FIT_CANFD_PROCESS_SNR_SETTING_Send(uint8_t isActivate);
void FIT_CANFD_PROCESS_SHUTDOWN_Send(void);
void FIT_CANFD_PROCESS_CLEAR_TRIP_Send(void);
void FIT_CANFD_PROCESS_WALK_ASSIST_LEVEL_Send(uint8_t level);


void FIT_CANFD_PROCESS_UNLOCK_SYS(void);
void FIT_CANFD_PROCESS_LOCK_SYS(void);
void FIT_CANFD_PROCESS_WALK_ASSIST(uint8_t isActivate);


void FIT_CANFD_PROCESS_MOTOR_ERROR_Get(uint32_t *err);
void FIT_CANFD_PROCESS_BMS_EX_ERROR_Get(uint16_t *err);
void FIT_CANFD_PROCESS_BMS_MAIN_ERROR_Get(uint16_t *err);
void FIT_CANFD_PROCESS_UINTS_SET__Send(uint8_t isActivate);
void FIT_CANFD_PROCESS_REMAINING_DATA_Send(void);
void FIT_CANFD_PROCESS_MC_HW_TYPE_Get(uint8_t *ht_h, uint8_t *ht_l);
void FIT_CANFD_PROCESS_MC_FMV_Get(uint8_t *fmv_h, uint8_t *fmv_l);
void FIT_CANFD_PROCESS_MC_FSV_Get(uint8_t *fsv_h, uint8_t *fsv_l);
void FIT_CANFD_PROCESS_MC_CN_Get(uint8_t *mc_cn);
void FIT_CANFD_PROCESS_MC_MODEL_Get(uint8_t *mc_model);
void FIT_CANFD_PROCESS_MC_SERIAL_NO_LOW_Get(uint8_t *sn_l_01,uint8_t *sn_l_02,uint8_t *sn_l_03,uint8_t *sn_l_04,uint8_t *sn_l_05,uint8_t *sn_l_06,uint8_t *sn_l_07,uint8_t *sn_l_08);
void FIT_CANFD_PROCESS_MC_SERIAL_NO_HIGH_Get(uint8_t *sn_h_01,uint8_t *sn_h_02,uint8_t *sn_h_03,uint8_t *sn_h_04,uint8_t *sn_h_05,uint8_t *sn_h_06,uint8_t *sn_h_07,uint8_t *sn_h_08);
void FIT_CANFD_PROCESS_MC_MCOD_Get(uint8_t *mc_mcod);
void FIT_CANFD_PROCESS_MC_PRODUCTION_INFO_Get(uint16_t *mc_pd_d,uint16_t *mc_pd_m,uint16_t *mc_pd_y);
void FIT_CANFD_PROCESS_BMS_FW_VERSION_Get(uint16_t *bfwmv,uint16_t *bfwsv,uint16_t *bfwds);
void FIT_CANFD_PROCESS_BMS_CN_Get(uint8_t *bms_cn);
void FIT_CANFD_PROCESS_BMS_MODEL_Get(uint8_t *bms_model);
void FIT_CANFD_PROCESS_BMS_SN_Get(uint32_t *bms_sn);
void FIT_CANFD_PROCESS_BMS_MCOD_Get(uint8_t *bms_mcod);
void FIT_CANFD_PROCESS_BMS_PRODUCTION_INFO_Get(uint8_t *bms_pd_d, uint8_t *bms_pd_m, uint8_t *bms_pd_y);
void FIT_CANFD_PROCESS_MC_FW_VERSION_Send(void);
void FIT_CANFD_PROCESS_SID_374_get(void); //Wade231120
void FIT_CANFD_PROCESS_MC_SERIAL_NO_LOW_Send(void);
void FIT_CANFD_PROCESS_MC_SERIAL_NO_HIGH_Send(void);
void FIT_CANFD_PROCESS_MC_PRODUCTION_INFO_Send(void);
void FIT_CANFD_PROCESS_BMS_FW_VERSION_Send(void);
void FIT_CANFD_PROCESS_BMS_SN_PRODUCTION_Send(void);


void FIT_CANFD_PROCESS_DBG_MESSAGE_Send(uint8_t id);
void FIT_CANFD_PROCESS_DBG_BTN_Send(uint8_t id, uint8_t click);

void FIT_CANFD_PROCESS_HMI_PERIODICAL_Send(void);

void FIT_CANBUS_INIT(void);



//------------------------------------------------------------------------------
#ifdef	__cplusplus
}
#endif

#endif	/* CANFD_H */
