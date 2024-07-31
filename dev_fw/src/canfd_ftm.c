#include <stdio.h>
#include <string.h>
#include "canfd_process.h"
#include "canfd_ftm.h"
#include "canfd.h"
#include "fw_api.h"
#include <logging/log.h>
#include <kernel.h>

LOG_MODULE_REGISTER( canfd_ftm );

static S_CANFD_PROCESS_BUFF can_ftm_tx_buf;
void FIT_CNAFD_FTM_DIAG_SWITCH_STATE(uint8_t isLock);
void FIT_CNAFD_FTM_GET_MAC_ADDR_CMD(void);
void FIT_CNAFD_FTM_SW_HW_VERSION(void);
void FIT_CNAFD_FTM_PCBA_PID_CMD(void);
void FIT_CNAFD_FTM_SET_PID_CMD(void);
void FIT_CNAFD_FTM_LCM_TEST_PATTERN_STATE(void);
void FIT_CNAFD_FTM_LCM_CANFD_TEST(void);
void FIT_CNAFD_FTM_BLE_TEST(void);
void FIT_CNAFD_FTM_FLASH_MEM_TEST(void);

void FIT_CANFD_FTM_callback(uint8_t *cb_data)
{
	// can ftm id process ...
	//LOG_INF("FIT_CANFD_FTM_callback.....");

	S_CAN_INFO *can_info = (S_CAN_INFO*)cb_data; 

	switch(can_info->data[0])
	{
			case 0x00:
				switch(can_info->data[1])
				{
					case 0xd0:
						FIT_CNAFD_FTM_DIAG_SWITCH_STATE(true);
						break;	

					case 0xd1:
						FIT_CNAFD_FTM_GET_MAC_ADDR_CMD();
						break;	

					case 0xd2:
						FIT_CNAFD_FTM_SW_HW_VERSION();
						break;

					case 0xd3:
						FIT_CNAFD_FTM_PCBA_PID_CMD();
						break;	

					case 0xd4:
						FIT_CNAFD_FTM_SET_PID_CMD();
						break;							

					case 0xd5:
						FIT_CNAFD_FTM_LCM_TEST_PATTERN_STATE();
						break;

					case 0xd6:
						FIT_CNAFD_FTM_LCM_CANFD_TEST();
						break;	

					case 0xd7:
						FIT_CNAFD_FTM_BLE_TEST();
						break;	

					case 0xd8:
						FIT_CNAFD_FTM_FLASH_MEM_TEST();
						break;	
					default:break;
	    		}	

			case 0x01:
				switch(can_info->data[1])
				{

					case 0xe0:
						can_ftm_tx_buf.tx_id = 0x7eb;
						can_ftm_tx_buf.payload.data = 0;
						can_ftm_tx_buf.payload.diag_command.Mode = 0x41;
						can_ftm_tx_buf.payload.diag_command.CMD = 0xd1;
						can_ftm_tx_buf.payload.diag_command.status = 0x00;						
						can_ftm_tx_buf.payload.diag_command.M_KEY_1 = 0xa4;
						can_ftm_tx_buf.payload.diag_command.M_KEY_2 = 0x56;
						FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_ftm_tx_buf);	
						break;

					case 0xe1:
						can_ftm_tx_buf.tx_id = 0x7eb;
						can_ftm_tx_buf.payload.data = 0;
						can_ftm_tx_buf.payload.diag_pcba_pid.Mode = 0x40;
						can_ftm_tx_buf.payload.diag_pcba_pid.CMD  = 0xd3;
						can_ftm_tx_buf.payload.diag_pcba_pid.PID0 = 0x00;						
						can_ftm_tx_buf.payload.diag_pcba_pid.PID1 = 0xa4;
						can_ftm_tx_buf.payload.diag_pcba_pid.PID2 = 0x56;
						FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_ftm_tx_buf);	
						break;						

					case 0xe2:

						break;	

					case 0xe3:

						break;	
						
					default:break;
	    		}
		default:break;
    }	
}	
void FIT_CNAFD_FTM_DIAG_SWITCH_STATE(uint8_t isLock)
{
	can_ftm_tx_buf.tx_id = 0x7eb;
	can_ftm_tx_buf.payload.data = 0;
	can_ftm_tx_buf.payload.diag_state.Mode = 0x40;
	can_ftm_tx_buf.payload.diag_state.CMD = 0xd0;
	can_ftm_tx_buf.payload.diag_state.status = isLock;						
	can_ftm_tx_buf.payload.diag_state.M_KEY_1 = 0xa4;
	can_ftm_tx_buf.payload.diag_state.M_KEY_2 = 0x56;

	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_ftm_tx_buf);	
}

void FIT_CNAFD_FTM_GET_MAC_ADDR_CMD(void)
{
	can_ftm_tx_buf.tx_id = 0x7eb;
	can_ftm_tx_buf.payload.data = 0;
	can_ftm_tx_buf.payload.diag_mac_address.Mode = 0x40;
	can_ftm_tx_buf.payload.diag_mac_address.CMD = 0xd1;
	can_ftm_tx_buf.payload.diag_mac_address.MAC0 = 0x00;
	can_ftm_tx_buf.payload.diag_mac_address.MAC1 = 0x00;
	can_ftm_tx_buf.payload.diag_mac_address.MAC2 = 0x00;

	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_ftm_tx_buf);	
}

void FIT_CNAFD_FTM_SW_HW_VERSION(void)
{
	can_ftm_tx_buf.tx_id = 0x7eb;
	can_ftm_tx_buf.payload.data = 0;
	can_ftm_tx_buf.payload.diag_hwsw_version.Mode = 0x40;
	can_ftm_tx_buf.payload.diag_hwsw_version.CMD = 0xd2;
	can_ftm_tx_buf.payload.diag_hwsw_version.SW = 0x00; 					
	can_ftm_tx_buf.payload.diag_hwsw_version.HW = 0xa4;

	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_ftm_tx_buf);	
}

void FIT_CNAFD_FTM_PCBA_PID_CMD(void)
{
	can_ftm_tx_buf.tx_id = 0x7eb;
	can_ftm_tx_buf.payload.data = 0;
	can_ftm_tx_buf.payload.diag_pcba_pid.Mode = 0x40;
	can_ftm_tx_buf.payload.diag_pcba_pid.CMD = 0xd3;
	can_ftm_tx_buf.payload.diag_pcba_pid.PID0 = 0x00;						
	can_ftm_tx_buf.payload.diag_pcba_pid.PID1 = 0xa4;
	can_ftm_tx_buf.payload.diag_pcba_pid.PID2 = 0x56;

	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_ftm_tx_buf);	
}

void FIT_CNAFD_FTM_SET_PID_CMD(void)
{
	can_ftm_tx_buf.tx_id = 0x7eb;
	can_ftm_tx_buf.payload.data = 0;
	can_ftm_tx_buf.payload.diag_set_pid.Mode = 0x40;
	can_ftm_tx_buf.payload.diag_set_pid.CMD = 0xd4;
	can_ftm_tx_buf.payload.diag_set_pid.PID0 = 0x00;						
	can_ftm_tx_buf.payload.diag_set_pid.PID1 = 0xa4;
	can_ftm_tx_buf.payload.diag_set_pid.PID2 = 0x56;

	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_ftm_tx_buf);	
}

void FIT_CNAFD_FTM_LCM_TEST_PATTERN_STATE(void)
{
	can_ftm_tx_buf.tx_id = 0x7eb;
	can_ftm_tx_buf.payload.data = 0;
	can_ftm_tx_buf.payload.diag_lcm_test.Mode = 0x40;
	can_ftm_tx_buf.payload.diag_lcm_test.CMD = 0xd5;

	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_ftm_tx_buf);	
}

void FIT_CNAFD_FTM_LCM_CANFD_TEST(void)
{
	can_ftm_tx_buf.tx_id = 0x7eb;
	can_ftm_tx_buf.payload.data = 0;
	can_ftm_tx_buf.payload.diag_canfd_test.Mode = 0x40;
	can_ftm_tx_buf.payload.diag_canfd_test.CMD = 0xd6;
	can_ftm_tx_buf.payload.diag_canfd_test.CHK = 0xAA55;
        if(ifactor_state_arr[0]==1){
            FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_ftm_tx_buf);
        }
		
}

void FIT_CNAFD_FTM_BLE_TEST(void)
{
	can_ftm_tx_buf.tx_id = 0x7eb;
	can_ftm_tx_buf.payload.data = 0;
	can_ftm_tx_buf.payload.diag_state.Mode = 0x40;
	can_ftm_tx_buf.payload.diag_ble_test.CMD = 0xd7;
	can_ftm_tx_buf.payload.diag_ble_test.Result = 0x00; 					
	can_ftm_tx_buf.payload.diag_ble_test.PeakPWR = 0xa4;
	can_ftm_tx_buf.payload.diag_ble_test.RSSI = 0x56;

	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_ftm_tx_buf);	
}

void FIT_CNAFD_FTM_FLASH_MEM_TEST(void)
{
	can_ftm_tx_buf.tx_id = 0x7eb;
	can_ftm_tx_buf.payload.data = 0;
	can_ftm_tx_buf.payload.diag_key_code.Mode = 0x40;
	can_ftm_tx_buf.payload.diag_key_code.CMD = 0xd8;
	can_ftm_tx_buf.payload.diag_key_code.CHK = 0x00;


	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_ftm_tx_buf);	
}

void FIT_CNAFD_FTM_Init(void)
{
	FIT_CANFD_PROCESS_FTM_REGISTER(FIT_CANFD_FTM_callback);
}
