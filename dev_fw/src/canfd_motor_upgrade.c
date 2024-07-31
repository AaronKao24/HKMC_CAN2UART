#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "canfd_process.h"
#include "canfd_motor_upgrade.h"
#include "canfd.h"
#include "fw_api.h"
#include <logging/log.h>
#include <kernel.h>

LOG_MODULE_REGISTER( canfd_motor_upgrade );

static S_CANFD_PROCESS_BUFF can_motorup_tx_buf;
static int erase_status;
static char update_status;
static char finish_status;
static char retry_count;

static uint8_t *data_read = NULL;
static uint8_t page_num=0;
static uint16_t counter=0;

void FIT_CANFD_MOTORO_UPGRADE_START_send(void);
uint8_t FIT_CANFD_MOTORO_UPGRADE_UPDATE_DATA_send(int status_flag);
void FIT_CANFD_MOTORO_UPGRADE_UPDATE_FINISH_send(void);

void FIT_CANFD_MOTOR_UPGRADE_callback(uint8_t *cb_data)
{
	S_CAN_INFO *can_info = (S_CAN_INFO*)cb_data; 


	switch(can_info->EID)
	{
		case 0x006:
			erase_status = can_info->data[1];
			break;
			
		case 0x008:
			update_status = can_info->data[1];
			break;
			
		case 0x00A:
			finish_status = can_info->data[1];
			break;

		default:break;
    }	
}	

void FIT_CANFD_MOTORO_UPGRADE_START_send(void)
{
	fw_flash_read((Mortor_BIN_END_ADDRESS-8) , 8 , motor_fw_version);
	can_motorup_tx_buf.tx_id = E_CANBUS_TX_ID_005;
	can_motorup_tx_buf.payload.data = 0;
	can_motorup_tx_buf.payload.motor_erase_cmd.M_key1 = 0x30;
	can_motorup_tx_buf.payload.motor_erase_cmd.M_key2 = motor_fw_version[0];
	can_motorup_tx_buf.payload.motor_erase_cmd.M_key3 = motor_fw_version[1];
	can_motorup_tx_buf.payload.motor_erase_cmd.M_key4 = motor_fw_version[6];
	can_motorup_tx_buf.payload.motor_erase_cmd.M_key5 = 0x45;
	can_motorup_tx_buf.payload.motor_erase_cmd.M_key6 = 0x41;
	can_motorup_tx_buf.payload.motor_erase_cmd.M_key7 = 0x47;
	can_motorup_tx_buf.payload.motor_erase_cmd.M_key8 = 0x4F;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_motorup_tx_buf);	
	
	erase_status = -1;	
}

uint8_t FIT_CANFD_MOTORO_UPGRADE_UPDATE_DATA_send(int status_flag)
{

	update_status = -1;
	//判斷是否已經傳完80K檔案，如果傳完則回傳1並跳到E_OTA_UPGRADE_UPDATE_FINISH_SEND，傳送Finish命令
	if(counter>=4096)
	{ 	
		page_num+=1;
		counter = 0;
		if(page_num < 20)
			fw_flash_read((Mortor_BIN_START_ADDRESS+page_num*4096) ,4096 , data_read);
		else
			return 1;
	}

	switch(status_flag)
	{
		case E_OTA_UPGRADE_UPDATE_DATA_STATUS_BEGIN:
			retry_count = 0;
			can_motorup_tx_buf.tx_id = E_CANBUS_TX_ID_007;
			can_motorup_tx_buf.payload.data = 0;
			can_motorup_tx_buf.payload.motor_update_data_cmd.Data_01 = *(data_read+counter);
			can_motorup_tx_buf.payload.motor_update_data_cmd.Data_02 = *(data_read+1+counter);
			can_motorup_tx_buf.payload.motor_update_data_cmd.Data_03 = *(data_read+2+counter);
			can_motorup_tx_buf.payload.motor_update_data_cmd.Data_04 = *(data_read+3+counter);
			can_motorup_tx_buf.payload.motor_update_data_cmd.Data_05 = *(data_read+4+counter);
			can_motorup_tx_buf.payload.motor_update_data_cmd.Data_06 = *(data_read+5+counter);
			can_motorup_tx_buf.payload.motor_update_data_cmd.Data_07 = *(data_read+6+counter);
			can_motorup_tx_buf.payload.motor_update_data_cmd.Data_08 = *(data_read+7+counter);
			FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_motorup_tx_buf);
			counter += 8;
		break;
		
		case E_OTA_UPGRADE_UPDATE_DATA_STATUS_RETRY:
			if(retry_count == 0)
			{
				counter-=8;
			}
			can_motorup_tx_buf.tx_id = E_CANBUS_TX_ID_007;
			can_motorup_tx_buf.payload.data = 0;
			can_motorup_tx_buf.payload.motor_update_data_cmd.Data_01 = *(data_read+counter);
			can_motorup_tx_buf.payload.motor_update_data_cmd.Data_02 = *(data_read+1+counter);
			can_motorup_tx_buf.payload.motor_update_data_cmd.Data_03 = *(data_read+2+counter);
			can_motorup_tx_buf.payload.motor_update_data_cmd.Data_04 = *(data_read+3+counter);
			can_motorup_tx_buf.payload.motor_update_data_cmd.Data_05 = *(data_read+4+counter);
			can_motorup_tx_buf.payload.motor_update_data_cmd.Data_06 = *(data_read+5+counter);
			can_motorup_tx_buf.payload.motor_update_data_cmd.Data_07 = *(data_read+6+counter);
			can_motorup_tx_buf.payload.motor_update_data_cmd.Data_08 = *(data_read+7+counter);
			FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_motorup_tx_buf);			
			retry_count+=1;
		break;

		default:break;
	}
	return 0;
}

void FIT_CANFD_MOTORO_UPGRADE_UPDATE_FINISH_send(void)
{
	can_motorup_tx_buf.tx_id = E_CANBUS_TX_ID_009;
	can_motorup_tx_buf.payload.data = 0;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_motorup_tx_buf);		
}

void FIT_CNAFD_MOTOR_UPGRADE_Init(void)
{
	FIT_CANFD_PROCESS_MOTOR_UPGRADE_REGISTER(FIT_CANFD_MOTOR_UPGRADE_callback);
}

int8_t FIT_CANFD_MOTOR_UPGRADE_UPDATE_ERASE_ACK_Get(void)
{
	return erase_status;
}

int8_t FIT_CANFD_MOTOR_UPGRADE_UPDATE_DATA_ACK_Get(void)
{
	return update_status;
}

int8_t FIT_CANFD_MOTOR_UPGRADE_UPDATE_FINISH_ACK_Get(void)
{
	return finish_status;
}

void FIT_CANFD_MOTORO_UPGRADE_Memory_Get(void)
{
	page_num = 0;
	counter = 0;
	retry_count = 0;
	data_read = malloc(4096);
	fw_flash_read((Mortor_BIN_START_ADDRESS+page_num*4096) ,4096 , data_read);
}

void FIT_CANFD_MOTORO_UPGRADE_Memory_Free(void)
{
	if(data_read!=NULL)
		free(data_read);
}