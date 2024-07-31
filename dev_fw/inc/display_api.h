/*******************************************************************************
* File Name       : dispaly_api.h
* Description     : 
* Original Author : Sammy.hh.chen
* Created on      : Feb 24, 2023
*******************************************************************************/

#ifndef DISPLAY_API_H
#define	DISPLAY_API_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>

#define SAMMY_LCM_TEST 0
#define HIGH_SPD_CHARGING 1
#define MIDDLE_SPD_CHARGING 15
#define NOMAL_SPD_CHARGING 30
#define LOW_SPD_CHARGING 60
#define CHARGING_SPD NOMAL_SPD_CHARGING

typedef enum {
	E_DISPLAY_ACTIVE_OFF = 0,
	E_DISPLAY_ACTIVE_ON = 1,
}E_DISPLAY_ACTIVE;

typedef enum {
	E_DISPLAY_PAGE_IDLE = 0x00,
	E_DISPLAY_PAGE_LOGO,
	E_DISPLAY_PAGE_MAIN,
	E_DISPLAY_PAGE_SUB,
	E_DISPLAY_PAGE_SUB2,
	E_DISPLAY_PAGE_BATTERY,
	E_DISPLAY_PAGE_WALK_ASSIST,
	E_DISPLAY_PAGE_LOCKED,
	E_DISPLAY_PAGE_CHARGING,
	E_DISPLAY_PAGE_BLE_PAIRING_FIRST,
	E_DISPLAY_PAGE_BLE_PAIRED,
	E_DISPLAY_PAGE_POWER_SAVING,
	E_DISPLAY_PAGE_ERROR,
	E_DISPLAY_PAGE_FTM,
	E_DISPLAY_PAGE_BYPASS,
	E_DISPLAY_PAGE_LCM_TEST,
	E_DISPLAY_PAGE_MAX,
}E_DISPLAY_PAGE;

typedef struct {
	uint32_t error_status;
}S_MOTOR_ERROR_ITEM;

typedef struct {
	uint16_t error_status;
}S_M_BMS_ERROR_ITEM;

typedef struct {
	uint16_t error_status;
}S_E_BMS_ERROR_ITEM;

//------------------------------------------------------------------------------
uint8_t *FIT_DISPLAY_PAGE_Get(void);


//------------------------------------------------------------------------------
#ifdef	__cplusplus
}
#endif

#endif	/* CANFD_H */
