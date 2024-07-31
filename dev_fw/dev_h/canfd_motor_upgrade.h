/*******************************************************************************
* File Name       : canfd_motor_upgrade.h
* Description     : 
* Original Author : Sammy.hh.chen
* Created on      : Mar 22, 2023
*******************************************************************************/

#ifndef CANFD_MOTOR_UPGRADE_H
#define	CANFD_MOTOR_UPGRADE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>

//------------------------------------------------------------------------------
void FIT_CNAFD_MOTOR_UPGRADE_Init(void);
void FIT_CANFD_MOTORO_UPGRADE_START_send(void);
uint8_t FIT_CANFD_MOTORO_UPGRADE_UPDATE_DATA_send(int status_flag);
int8_t FIT_CANFD_MOTOR_UPGRADE_UPDATE_ERASE_ACK_Get(void);
int8_t FIT_CANFD_MOTOR_UPGRADE_UPDATE_DATA_ACK_Get(void);
int8_t FIT_CANFD_MOTOR_UPGRADE_UPDATE_FINISH_ACK_Get(void);
void FIT_CANFD_MOTORO_UPGRADE_Memory_Get(void);
void FIT_CANFD_MOTORO_UPGRADE_Memory_Free(void);
void FIT_CANFD_MOTORO_UPGRADE_START_send(void);
uint8_t FIT_CANFD_MOTORO_UPGRADE_UPDATE_DATA_send(int status_flag);
void FIT_CANFD_MOTORO_UPGRADE_UPDATE_FINISH_send(void);
//------------------------------------------------------------------------------
#ifdef	__cplusplus
}
#endif
typedef enum {
	E_OTA_UPGRADE_ERASE_SEND,
	E_OTA_UPGRADE_ERASE_WAITING_ACK,
	E_OTA_UPGRADE_UPDATE_DATA_SEND,
	E_OTA_UPGRADE_UPDATE_DATA_SEND_RETRY,
	E_OTA_UPGRADE_UPDATE_DATA_WAITING_ACK,
	E_OTA_UPGRADE_UPDATE_FINISH_SEND,
	E_OTA_UPGRADE_UPDATE_FINISH_WAITING_ACK,
}E_OTA_UPGRADE;

typedef enum {
	E_OTA_UPGRADE_UPDATE_DATA_STATUS_RETRY,
	E_OTA_UPGRADE_UPDATE_DATA_STATUS_BEGIN,
}E_OTA_UPGRADE_UPDATE_DATA_STATUS;

#endif	/* CANFD_MOTOR_UPGRADE_H */
