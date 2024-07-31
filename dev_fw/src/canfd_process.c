#include "canfd.h"
#include "canfd_process.h"
#include "canfd_ftm.h"
#include "drv_canfdspi_api.h"
#include "drv_canfdspi_register.h"
#include "drv_canfdspi_defines.h"
#include "version_log.h"
#include "thread_usb.h"
#include "fw_api.h"

#include <logging/log.h>
#include <kernel.h>



LOG_MODULE_REGISTER( canfd_process );

static fit_canfd_ftm	ftm_cb;
static fit_canfd_motor_upgrade	mu_cb;
static S_ALL_INFO all_info;
static S_CANFD_PROCESS_RING canfd_ring;
static S_MONITOR	s_monitor = {0};
static S_CANFD_PROCESS_BUFF can_prcess_tx_buf;
static S_CANFD_PROCESS_RING_FUNC init_ring;

void FIT_CANFD_PROCESS_HMI_SN_PRODUCTION_Send(void);
void FIT_CANFD_PROCESS_HMI_VERSION_Send(void);

unsigned char send_array[66];
char hex_string[134];


int get_real_dlc_value(int dlc_value)
{
        if (dlc_value > 8)
        {
            switch(dlc_value) {
              case 9:
                  dlc_value = 12;
                  break;
              case 10:
                  dlc_value = 16;
                  break;
              case 11:
                  dlc_value = 20;
                  break;
              case 12:
                  dlc_value = 24;
                  break;
              case 13:
                  dlc_value = 32;
                  break;
              case 14:
                  dlc_value = 48;
                  break;
              case 15:
                  dlc_value = 64;
                  break;
              default:
                  //
                  break;
            }
        }

        return dlc_value;


}
void FIT_CANFD_PROCESS_callback(uint64_t *cb_data)
{
	S_CAN_INFO *can_info = (S_CAN_INFO*)cb_data; 

        send_array[0] = (can_info->EID >> 8) & 0xFF;
        send_array[1] = (uint64_t)can_info->EID & 0xFF;
        for (int i =0 ; i<64 ; i++)
        {
          send_array[i+2] = can_info->data[i];
        }
        hex_array_to_string(send_array, get_real_dlc_value(can_info->DLC)+2, hex_string);
        fw_write_USB_data(hex_string+1 , strlen(hex_string));
        memset(hex_string , 0 , 134);
	arch_nop();


}	

void FIT_CANFD_PROCESS_FTM_REGISTER(fit_canfd_ftm cb)
{
	ftm_cb = cb;
}

void FIT_CANFD_PROCESS_MOTOR_UPGRADE_REGISTER(fit_canfd_motor_upgrade cb)
{
	mu_cb= cb;
}

void FIT_CANFD_PROCESS_Ring_Init(void)
{
    canfd_ring.pIn = canfd_ring.pOut = (uint8_t *)canfd_ring.buf;       
    canfd_ring.pEnd =(uint8_t *)&canfd_ring.buf[E_CAN_TX_MAX];   
    canfd_ring.full = 0;      
}

uint8_t FIT_CANFD_PROCESS_RING_Put(uint8_t *c)
{
	if (canfd_ring.pIn == canfd_ring.pOut  &&  canfd_ring.full)
        return 0;           

  	 memcpy(canfd_ring.pIn, c, sizeof(S_CANFD_PROCESS_BUFF));
	
	 canfd_ring.pIn+= sizeof(S_CANFD_PROCESS_BUFF);
    if (canfd_ring.pIn >= canfd_ring.pEnd)        
        canfd_ring.pIn = (uint8_t *)canfd_ring.buf;         

    if (canfd_ring.pIn == canfd_ring.pOut)       
        canfd_ring.full = 1;          
    return 1;              
}

uint8_t FIT_CANFD_PROCESS_RING_Get(uint8_t *pc)
{
	if (canfd_ring.pIn == canfd_ring.pOut  &&  !canfd_ring.full)
		 return 0;			
	
	 //*pc = *canfd_ring.pOut;
	 
	  memcpy(pc, canfd_ring.pOut, sizeof(S_CANFD_PROCESS_BUFF));
	  canfd_ring.pOut += sizeof(S_CANFD_PROCESS_BUFF);
	 if (canfd_ring.pOut >= canfd_ring.pEnd)		
		 canfd_ring.pOut = (uint8_t *)canfd_ring.buf;		 
	
	 canfd_ring.full = 0;				 
	 return 1;				
}

uint8_t FIT_CANFD_PROCESS_RING_isNotProcessed(void)
{
	return (canfd_ring.pIn != canfd_ring.pOut) ? 1 : 0;
}

uint8_t FIT_CANFD_PROCESS_RING_isFull(void)
{
	return canfd_ring.full;
}

S_ALL_INFO* FIT_CNAFD_PROCESS_Data_Get(void)
{
	return &all_info;
}

void FIT_CANFD_PROCESS_BMS_MIX_SoC_Get(uint8_t *h_d, uint8_t *ten_d, uint8_t *uints_d, uint8_t *grid)
{
	if(s_monitor.M_CANID_EX_BMS_2A3 == 1)
	{
		global_battery_number = 2;
		if(all_info.bms.main_discharge_status.Full_Charged_Capacity == 0 ||
		   all_info.bms.ex_discharge_status.Full_Charged_Capacity == 0) return 0;
		
		uint8_t bms_soc = (all_info.bms.main_discharge_status.Full_Charged_Capacity * all_info.bms.main_bms_periodical_frame.State_of_Charge/2 +
						all_info.bms.ex_discharge_status.Full_Charged_Capacity * all_info.bms.ex_bms_periodical_frame.State_of_Charge/2)	/
						(all_info.bms.main_discharge_status.Full_Charged_Capacity +  all_info.bms.ex_discharge_status.Full_Charged_Capacity);

		if( bms_soc > 100)
		{
			HUNDRES_DIGIT(h_d, ten_d, uints_d, 100);
			BMS_BAR_CONVERSION(100, grid);
		}
		else
		{
			HUNDRES_DIGIT(h_d, ten_d, uints_d, bms_soc);
			BMS_BAR_CONVERSION(bms_soc, grid);
		}
	}
	else
	{
		global_battery_number = 1;
		if( (all_info.bms.main_bms_periodical_frame.State_of_Charge/2) > 100)
		{
			HUNDRES_DIGIT(h_d, ten_d, uints_d, 100);
			BMS_BAR_CONVERSION(100, grid);
		}
		else
		{
			HUNDRES_DIGIT(h_d, ten_d, uints_d, all_info.bms.main_bms_periodical_frame.State_of_Charge/2);
			BMS_BAR_CONVERSION(all_info.bms.main_bms_periodical_frame.State_of_Charge/2, grid);
		}
	}
}

void FIT_CANFD_PROCESS_BMS_MAIN_SoC_Get(uint8_t *h_d, uint8_t *ten_d, uint8_t *uints_d, uint8_t *grid)
{
	if( (all_info.bms.main_bms_periodical_frame.State_of_Charge/2) > 100)
	{
		HUNDRES_DIGIT(h_d, ten_d, uints_d, 100);
		BMS_BAR_CONVERSION(100, grid);
	}
	else
	{
		HUNDRES_DIGIT(h_d, ten_d, uints_d, all_info.bms.main_bms_periodical_frame.State_of_Charge/2);
		BMS_BAR_CONVERSION(all_info.bms.main_bms_periodical_frame.State_of_Charge/2, grid);
	}
}

void FIT_CANFD_PROCESS_BMS_EX_SoC_Get(uint8_t *h_d, uint8_t *ten_d, uint8_t *uints_d, uint8_t *grid)
{
	if( (all_info.bms.ex_bms_periodical_frame.State_of_Charge/2) > 100)
	{
		HUNDRES_DIGIT(h_d, ten_d, uints_d, 100);
		BMS_BAR_CONVERSION(100, grid);
	}
	else
	{
		HUNDRES_DIGIT(h_d, ten_d, uints_d, all_info.bms.ex_bms_periodical_frame.State_of_Charge/2);
		BMS_BAR_CONVERSION(all_info.bms.ex_bms_periodical_frame.State_of_Charge/2, grid);
	}
}

void FIT_CANFD_PROCESS_MOTOR_Speed_Get(uint8_t *ten_d, uint8_t *uints_d)
{
	
	if(all_info.motor.mc_periodical_frame.Speed/10 >= 0x63)
		TENS_DIGIT(ten_d, uints_d, 0x63 );
	else
		TENS_DIGIT(ten_d, uints_d, all_info.motor.mc_periodical_frame.Speed/10);
}

void FIT_CANFD_PROCESS_MOTOR_TRIP_Get(uint8_t *h_d, uint8_t *ten_d, uint8_t *uints_d)
{	
	HUNDRES_DIGIT(h_d, ten_d, uints_d, all_info.motor.mc_periodical_frame.TRIP/10);
}

void FIT_CANFD_PROCESS_MOTOR_ODO_Get( uint8_t *tens_th_d, uint8_t *th_d, uint8_t* h_d,  
												    uint8_t *ten_d, uint8_t *uints_d)
{
	TENS_THOUSAND_DIGIT(tens_th_d, th_d, h_d, ten_d, uints_d, all_info.motor.mc_periodical_frame.ODO);
}

int8_t FIT_CANFD_PROCESS_MOTOR_MODE_Get(void)
{
	//if(all_info.motor.mc_periodical_frame.MODE > WALK_ASSIST_LEVEL_MAX)	
	//	return WALK_ASSIST_LEVEL_MAX;
	//else if (all_info.motor.mc_periodical_frame.MODE < WALK_ASSIST_LEVEL_MIN)
	//	return WALK_ASSIST_LEVEL_MIN;
	//else
		return all_info.motor.mc_periodical_frame.MODE ;
}

uint8_t FIT_CANFD_PROCESS_MOTOR_STATUS_Get(void)
{
	return all_info.motor.mc_periodical_frame.Status;
}

uint8_t FIT_CANFD_PROCESS_MOTOR_STATUS_LOCK_Get(void)
{
	return all_info.motor.mc_periodical_frame.Status & 0x01;
}

uint8_t FIT_CANFD_PROCESS_MOTOR_STATUS_BRAKE_Get(void)
{
	return (all_info.motor.mc_periodical_frame.Status)>>1 & 0x01;
}

uint8_t FIT_CANFD_PROCESS_MOTOR_STATUS_LIGHT_Get(void)
{
	return (all_info.motor.mc_periodical_frame.Status >> 2)& 0x01;
}

uint8_t FIT_CANFD_PROCESS_MOTOR_STATUS_REGENERATION_Get(void)
{
	return (all_info.motor.mc_periodical_frame.Status >> 3)& 0x01;
}

uint8_t FIT_CANFD_PROCESS_MOTOR_STATUS_ERROR_Get(void)
{
	return (all_info.motor.mc_periodical_frame.Status >> 4)& 0x01;
}

uint8_t FIT_CANFD_PROCESS_MOTOR_STATUS_UINT_MILE_Get(void)
{
	return (all_info.motor.mc_periodical_frame.Status >> 5) & 0x01;
}

uint8_t FIT_CANFD_PROCESS_MOTOR_STATUS_DERATING_Get(void)
{
	return (all_info.motor.mc_periodical_frame.Status >> 6)& 0x01;
}

uint8_t FIT_CANFD_PROCESS_MOTOR_STATUS_WALING_BOSST_Get(void)
{
	return (all_info.motor.mc_periodical_frame.Status >> 7)& 0x01;
}

uint8_t FIT_CANFD_PROCESS_MAIN_BMS_STATUS_CHARGING_Get(void)
{	
		if(all_info.bms.main_bms_periodical_frame.BMS_Status&0x01 == 1)
			return 0;
		else
			return 1;
}

void FIT_CANFD_PROCESS_REMAINING_DISTANCE_Get(uint8_t *h_d, uint8_t *ten_d, uint8_t *uints_d)
{	
	HUNDRES_DIGIT(h_d, ten_d, uints_d, all_info.motor.rd_remainging_data.RESTD);
}

/* Get now Torque Status by Vincent*/
uint8_t FIT_CANFD_PROCESS_SNR_INPUT_Get(void)
{
	return all_info.motor.mc_snr_input.Throttle;
}

void FIT_CANFD_PROCESS_MC_HW_TYPE_Get(uint8_t *ht_h, uint8_t *ht_l)
{
	 *ht_h = all_info.motor.mc_fw_ver.HW_Type_H;
	 *ht_l = all_info.motor.mc_fw_ver.HW_Type_L;
}

void FIT_CANFD_PROCESS_MC_FMV_Get(uint8_t *fmv_h, uint8_t *fmv_l)
{
	*fmv_h = all_info.motor.mc_fw_ver.MC_FW_main_version_H;
	*fmv_l = all_info.motor.mc_fw_ver.MC_FW_main_version_L;
}
void FIT_CANFD_PROCESS_MC_FSV_Get(uint8_t *fsv_h, uint8_t *fsv_l)
{
	*fsv_h = all_info.motor.mc_fw_ver.MC_FW_sec_version_H;
	*fsv_l = all_info.motor.mc_fw_ver.MC_FW_sec_version_L;
}

void FIT_CANFD_PROCESS_MC_CN_Get(uint8_t *mc_cn)
{
	*mc_cn = all_info.motor.mc_fw_ver.CN;
}

void FIT_CANFD_PROCESS_MC_MODEL_Get(uint8_t *mc_model)
{
	*mc_model = all_info.motor.mc_fw_ver.MODEL;
}

void FIT_CANFD_PROCESS_MC_SERIAL_NO_LOW_Get(uint8_t *sn_l_01,uint8_t *sn_l_02,uint8_t *sn_l_03,uint8_t *sn_l_04,uint8_t *sn_l_05,uint8_t *sn_l_06,uint8_t *sn_l_07,uint8_t *sn_l_08)
{
	*sn_l_01 = all_info.motor.mc_sn_low.SN_L_01;
	*sn_l_02 = all_info.motor.mc_sn_low.SN_L_02;
	*sn_l_03 = all_info.motor.mc_sn_low.SN_L_03;
	*sn_l_04 = all_info.motor.mc_sn_low.SN_L_04;
	*sn_l_05 = all_info.motor.mc_sn_low.SN_L_05;
	*sn_l_06 = all_info.motor.mc_sn_low.SN_L_06;
	*sn_l_07 = all_info.motor.mc_sn_low.SN_L_07;
	*sn_l_08 = all_info.motor.mc_sn_low.SN_L_08;
}
void FIT_CANFD_PROCESS_MC_SERIAL_NO_HIGH_Get(uint8_t *sn_h_01,uint8_t *sn_h_02,uint8_t *sn_h_03,uint8_t *sn_h_04,uint8_t *sn_h_05,uint8_t *sn_h_06,uint8_t *sn_h_07,uint8_t *sn_h_08)
{
	*sn_h_01 = all_info.motor.mc_sn_high.SN_H_01;
	*sn_h_02 = all_info.motor.mc_sn_high.SN_H_02;
	*sn_h_03 = all_info.motor.mc_sn_high.SN_H_03;
	*sn_h_04 = all_info.motor.mc_sn_high.SN_H_04;
	*sn_h_05 = all_info.motor.mc_sn_high.SN_H_05;
	*sn_h_06 = all_info.motor.mc_sn_high.SN_H_06;
	*sn_h_07 = all_info.motor.mc_sn_high.SN_H_07;
	*sn_h_08 = all_info.motor.mc_sn_high.SN_H_08;
}

void FIT_CANFD_PROCESS_MC_MCOD_Get(uint8_t *mc_mcod)
{
	*mc_mcod = all_info.motor.mc_Production.MCOD;
}

void FIT_CANFD_PROCESS_MC_PRODUCTION_INFO_Get(uint16_t *mc_pd_d, uint16_t *mc_pd_m, uint16_t *mc_pd_y)
{	
	*mc_pd_d = all_info.motor.mc_Production.Production_date_day;
	*mc_pd_m = all_info.motor.mc_Production.Production_date_month;
	*mc_pd_y = all_info.motor.mc_Production.Production_date_year;
}

void FIT_CANFD_PROCESS_BMS_FW_VERSION_Get(uint16_t *bfwmv, uint16_t *bfwsv, uint16_t *bfwds)
{
	*bfwmv = all_info.bms.bms_fw_ver.BMS_fw_main_version;
	*bfwsv = all_info.bms.bms_fw_ver.BMS_fw_sec_version;
	*bfwds = all_info.bms.bms_fw_ver.BMS_fw_debug_serial;
}

void FIT_CANFD_PROCESS_BMS_CN_Get(uint8_t *bms_cn)
{
	*bms_cn = all_info.bms.bms_fw_ver.client_Number;
}

void FIT_CANFD_PROCESS_BMS_MODEL_Get(uint8_t *bms_model)
{
	*bms_model = all_info.bms.bms_fw_ver.Brand;
}

void FIT_CANFD_PROCESS_BMS_SN_Get(uint32_t *bms_sn)
{
	*bms_sn = all_info.bms.bms_SN_Production.Serial_number;
}

void FIT_CANFD_PROCESS_BMS_MCOD_Get(uint8_t *bms_mcod)
{
	*bms_mcod = all_info.bms.bms_SN_Production.Manufacture_code;
}

void FIT_CANFD_PROCESS_BMS_PRODUCTION_INFO_Get(uint8_t *bms_pd_d,uint8_t *bms_pd_m,uint8_t *bms_pd_y)
{
	*bms_pd_d = all_info.bms.bms_SN_Production.PD_D;
	*bms_pd_m = all_info.bms.bms_SN_Production.PD_M;	
	*bms_pd_y = all_info.bms.bms_SN_Production.PD_Y;
}

uint8_t* FIT_CANFD_PROCESS_MONITOR_Get(void)
{
	return (uint8_t*)&s_monitor;
}

/* Get HMI Status by YUN*/
void FIT_CANFD_PROCESS_MAIN_BMS_CYCLE_COUNT_Get(uint16_t *bms_cc)
{
	*bms_cc = all_info.bms.main_discharge_status.Cycle_Count;
}

/* Get HMI Status by YUN*/

void FIT_CANFD_PROCESS_MAIN_BMS_DISCHARGE_Send(void)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_285;
	can_prcess_tx_buf.payload.data = 0;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_EX_BMS_DISCHARGE_Send(void)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_2A5;
	can_prcess_tx_buf.payload.data = 0;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_UNLOCK_Send(void)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_103;
	can_prcess_tx_buf.payload.data = 0;
	can_prcess_tx_buf.payload.unlock_sys.M_KEY_1 = 0x31;
	can_prcess_tx_buf.payload.unlock_sys.M_KEY_2 = 0x32;
	can_prcess_tx_buf.payload.unlock_sys.M_KEY_3 = 0x33;
	can_prcess_tx_buf.payload.unlock_sys.M_KEY_4 = 0x34;
	can_prcess_tx_buf.payload.unlock_sys.M_KEY_5 = 0x35;
	can_prcess_tx_buf.payload.unlock_sys.M_KEY_6 = 0x36;	
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);
}

void FIT_CANFD_PROCESS_LOCK_Send(void)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_104;
	can_prcess_tx_buf.payload.data = 0;
	can_prcess_tx_buf.payload.lock_sys.M_KEY_1 = 0x55;
	can_prcess_tx_buf.payload.lock_sys.M_KEY_2 = 0x66;
	can_prcess_tx_buf.payload.lock_sys.M_KEY_3 = 0x77;
	can_prcess_tx_buf.payload.lock_sys.M_KEY_4 = 0x88;	
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_BMS_REAL_TIME_Send(void)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_2C1;
	can_prcess_tx_buf.payload.data = 0;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_WALK_ASSIST_Send(uint8_t isActivate)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_1CC;
	can_prcess_tx_buf.payload.data = 0;
	can_prcess_tx_buf.payload.walk_assist.WALK = isActivate;
	can_prcess_tx_buf.payload.walk_assist.M_KEY = 0x55;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_LIGHT_Send(uint8_t isActivate)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_1C2;
	can_prcess_tx_buf.payload.data = 0;
	can_prcess_tx_buf.payload.head_light.HLON = isActivate;
	can_prcess_tx_buf.payload.walk_assist.M_KEY = 0x44;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

/*Read Torque Status by Vincent*/ 
void FIT_CANFD_PROCESS_SNR_INPUT_Send(void)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_1AF;
	can_prcess_tx_buf.payload.data = 0;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

/* Set Throttle On/Off by Vincent */
void FIT_CANFD_PROCESS_SNR_SETTING_Send(uint8_t isActivate)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_1B0;
	can_prcess_tx_buf.payload.data = 0;
	can_prcess_tx_buf.payload.snr_setting.Throttle = isActivate;
	can_prcess_tx_buf.payload.snr_setting.TORQUE = 0x03;
	can_prcess_tx_buf.payload.snr_setting.SPEEDS = 0x00;
	can_prcess_tx_buf.payload.snr_setting.BRKS = 0x00;
	can_prcess_tx_buf.payload.snr_setting.unused = 0x000000;
	can_prcess_tx_buf.payload.snr_setting.M_KEY = 0x55;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);	

}

void FIT_CANFD_PROCESS_SHUTDOWN_Send(void)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_102;
	can_prcess_tx_buf.payload.data = 0;
	can_prcess_tx_buf.payload.shutdown.M_KEY_1 = 0x66;
	can_prcess_tx_buf.payload.shutdown.M_KEY_2 = 0xBB;
	can_prcess_tx_buf.payload.shutdown.M_KEY_3 = 0x66;
	can_prcess_tx_buf.payload.shutdown.M_KEY_4 = 0xBB;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_CLEAR_TRIP_Send(void)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_13D;
	can_prcess_tx_buf.payload.data = 0;
	can_prcess_tx_buf.payload.clear_trip.M_KEY = 0x55;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_WALK_ASSIST_LEVEL_Send(uint8_t level)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_13E;
	can_prcess_tx_buf.payload.data = 0;
	can_prcess_tx_buf.payload.assist_set.ASSIST = level;
	can_prcess_tx_buf.payload.assist_set.M_KEY = 0x55;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_HMI_VERSION_Send(void)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_301;
	can_prcess_tx_buf.payload.data = 0;
	can_prcess_tx_buf.payload.hmi_version.HMI_fw_main_version		= FW_VERSION_MAIN;
	can_prcess_tx_buf.payload.hmi_version.HMI_fw_sec_version	 	= FW_VERSION_SECOND;
	can_prcess_tx_buf.payload.hmi_version.HMI_fw_debug_serial		= FW_VERSION_DEBUG_SERIAL;
	can_prcess_tx_buf.payload.hmi_version.HMI_hw_main_version		= HW_VERSION;
	can_prcess_tx_buf.payload.hmi_version.Brand						= '3';
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_HMI_SN_PRODUCTION_Send(void)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_302;
	can_prcess_tx_buf.payload.data = 0;
	can_prcess_tx_buf.payload.hmi_sn_production.Serial_number = 0x12345678;
	can_prcess_tx_buf.payload.hmi_sn_production.Manufacture_code = 0xFF;
	can_prcess_tx_buf.payload.hmi_sn_production.PD_Y = 0x16;
	can_prcess_tx_buf.payload.hmi_sn_production.PD_M = 0x03;
	can_prcess_tx_buf.payload.hmi_sn_production.PD_D = 0x07;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_UINTS_SET__Send(uint8_t isActivate)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_13F;
	can_prcess_tx_buf.payload.data = 0;
	can_prcess_tx_buf.payload.uint_set.UINT = isActivate;
	can_prcess_tx_buf.payload.uint_set.M_KEY = 0x55;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_HMI_PERIODICAL_Send(void) //Wade231120: For mmWave test (every 25ms)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_300;
	can_prcess_tx_buf.payload.data = 0x0A;

	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_REMAINING_DATA_Send(void) //Wade231120: For mmWave test (every 100ms)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_301;
	can_prcess_tx_buf.payload.data = 0x1110;

	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_MC_FW_VERSION_Send(void)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_110;
	can_prcess_tx_buf.payload.data = 0;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_SID_374_get(void) //Wade231120
{
	can_prcess_tx_buf.tx_id = 0x777;
	can_prcess_tx_buf.payload.data = 0;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_MC_SERIAL_NO_LOW_Send(void)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_113;
	can_prcess_tx_buf.payload.data = 0;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_MC_SERIAL_NO_HIGH_Send(void)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_114;
	can_prcess_tx_buf.payload.data = 0;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_MC_PRODUCTION_INFO_Send(void)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_117;
	can_prcess_tx_buf.payload.data = 0;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_BMS_FW_VERSION_Send(void)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_280;
	can_prcess_tx_buf.payload.data = 0;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_BMS_SN_PRODUCTION_Send(void)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_282;
	can_prcess_tx_buf.payload.data = 0;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_DBG_MESSAGE_Send(uint8_t id)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_320;
	can_prcess_tx_buf.payload.data = 0;
	can_prcess_tx_buf.payload.DBG_message.MSG_ID = id;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_DBG_BTN_Send(uint8_t id, uint8_t click)
{
	can_prcess_tx_buf.tx_id = E_CANBUS_TX_ID_320;
	can_prcess_tx_buf.payload.data = 0;
	can_prcess_tx_buf.payload.DBG_message.MSG_ID = id;
	can_prcess_tx_buf.payload.DBG_message.MSG_PL0 = click;
	FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_prcess_tx_buf);		
}

void FIT_CANFD_PROCESS_BMS_REAL_TIME_Get(uint8_t *s_tens_hour, uint8_t *s_u_hour, uint8_t *s_tens_mins	, uint8_t *s_u_mins)
{
	TENS_DIGIT(s_tens_hour, s_u_hour, all_info.bms.bms_real_time.RTC_HOUR);
	TENS_DIGIT(s_tens_mins, s_u_mins, all_info.bms.bms_real_time.RTC_MIN);
}

void FIT_CANFD_PROCESS_MOTOR_ERROR_Get(uint32_t *err)
{
	*err = all_info.motor.mc_warrning_alarm.MC_Warning_alarm;
}															

void FIT_CANFD_PROCESS_BMS_MAIN_ERROR_Get(uint16_t *err)
{
	*err = all_info.bms.main_bms_warning_alarm.BMS_warning_alarm;
}															

void FIT_CANFD_PROCESS_BMS_EX_ERROR_Get(uint16_t *err)
{
	*err =  all_info.bms.ex_bms_warning_alarm.BMS_warning_alarm;
}

void FIT_CANFD_PROCESS_SHUTDOWN(void)
{
	if(s_monitor.M_CANID_SHUTDOWN_102 == false)
		s_monitor.M_CANID_SHUTDOWN_102 = true;
}

void FIT_CANFD_PROCESS_UNLOCK_SYS(void)
{
	s_monitor.M_CANID_UNLOCK_SYS_103 = true;
}

void FIT_CANFD_PROCESS_LOCK_SYS(void)
{
	s_monitor.M_CANID_LOCK_SYS_104 = true;
}

void FIT_CANFD_PROCESS_WALK_ASSIST(uint8_t isActivate)
{
	s_monitor.M_CANID_WALK_ASSIST_1CC = isActivate;

	if(isActivate == 0)
		FIT_CANFD_PROCESS_WALK_ASSIST_Send(false);
}

void FIT_CANFD_PROCESS_register_cb(void)
{
	init_ring.ring_init 			= &FIT_CANFD_PROCESS_Ring_Init;
	init_ring.ring_put 				= &FIT_CANFD_PROCESS_RING_Put;
	init_ring.ring_get 				= &FIT_CANFD_PROCESS_RING_Get;
	init_ring.ring_isNotProcessed 	= &FIT_CANFD_PROCESS_RING_isNotProcessed;
	init_ring.ring_isFull 			= &FIT_CANFD_PROCESS_RING_isFull;
	memset(init_ring.ring_get_buff, 0, sizeof(S_CANFD_PROCESS_BUFF));
	
	FIT_CANFD_PROCESS_RING_REGISTER((uint8_t *)&init_ring);
	FIT_CANFD_Init_cb(FIT_CANFD_PROCESS_callback);
	FIT_CNAFD_FTM_Init();
	FIT_CNAFD_MOTOR_UPGRADE_Init();
}

void FIT_CNAFD_PROCESS_Init(void)
{	
	memset(&all_info, 0, sizeof(S_ALL_INFO));
	memset(&canfd_ring, 0, sizeof(S_CANFD_PROCESS_RING));
	all_info.bms.main_bms_periodical_frame.BMS_Status = 0xFF;
	
	FIT_CANFD_PROCESS_register_cb();
}
