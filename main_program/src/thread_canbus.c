
#include <stdio.h>

#include <zephyr.h>
#include "fw_api.h"
#include "hal_api.h"
#include <logging/log.h>

#include "fw_api.h"
#include "hal_api.h"
#include "function.h"
#include "canfd.h"
#include "canfd_process.h"
#include "drv_canfdspi_api.h"
#include "drv_canfdspi_register.h"
#include "drv_canfdspi_defines.h"
#include "sw_api.h"
/** @file */

LOG_MODULE_REGISTER( thread_canbus );

static S_CANFD_PROCESS_RING_FUNC canfd_ring_cb;
static E_CANBUS_STATE can_state = E_CANBUS_STATE_DONE;

extern struct system_parameter_buffer sys_param ;

void thread_for_canbus(void);
void FIT_CAN_STATE_Change(E_CANBUS_STATE change_state);

void FIT_CANBUS_INIT(void)
{
	can_state = E_CANBUS_STATE_INIT;
}
/**
* @def CAN_PRIORITY 
* CANBus The priority of the communication trip (the smaller the value, the higher the priority)
*/
#define CAN_PRIORITY 4
/**
* @def CAN_THREAD_STACK_SIZE CANBus Memory default size for communication process
*/
#define CAN_THREAD_STACK_SIZE 1024
/**
* @fn thread_for_canbus
* @brief CAN Thread
*/
K_THREAD_DEFINE(can_communication_id, CAN_THREAD_STACK_SIZE, thread_for_canbus, NULL, NULL, NULL, CAN_PRIORITY, 0, 0);

void FIT_CANFD_PROCESS_RING_REGISTER(uint8_t *data);
void FIT_CANFD_PROCESS_Tranmit(uint8_t *id);

/*******************************************************************************
* Private Function: FIT_CANFD_PROCESS_Tranmit
* Description     : 
* Purpose         :
* Input           : 
* Return          : 
* Calls           :    
* Called by       :
* Note			  :
*******************************************************************************/
void FIT_CANFD_PROCESS_Tranmit(uint8_t *id)
{
	S_CANFD_PROCESS_BUFF *buf_data = (S_CANFD_PROCESS_BUFF*)id;

	if(buf_data->tx_id == E_CANBUS_TX_ID_NONE ) return;

		if(!canfd_ring_cb.ring_isFull())
			canfd_ring_cb.ring_put(id);
}

/*******************************************************************************
* Private Function: FIT_CANFD_PROCESS_Tranmit
* Description     : 
* Purpose         :
* Input           : 
* Return          : 
* Calls           :    
* Called by       :
* Note			  :
*******************************************************************************/
void FIT_CANFD_PROCESS_RING_REGISTER(uint8_t *data)
{
	S_CANFD_PROCESS_RING_FUNC *ring_func = (S_CANFD_PROCESS_RING_FUNC *)data;
	memcpy(&canfd_ring_cb, ring_func, sizeof(S_CANFD_PROCESS_RING_FUNC));
}

/*******************************************************************************
* Private Function: thread_for_canbus
* Description     : 
* Purpose         :
* Input           : 
* Return          : 
* Calls           :    
* Called by       :
* Note			  :
*******************************************************************************/
void thread_for_canbus(void)
{

	CAN_RX_FIFO_EVENT rxFlags;
	
	FIT_CNAFD_PROCESS_Init();

	while(1)
	{
	     switch(can_state)
	     {
	      case E_CANBUS_STATE_INIT:
		        fw_normal_spi_init();
		        //sys_param.can_init =  FIT_CAN_Init();
                        sys_param.can_init =  FIT_CANFD_Init(); //Wade231122
			  	canfd_ring_cb.ring_init();
		        can_state = E_CANBUS_STATE_IDLE;
				global_ebike_light = 1;
				global_motor_assist_level = 1;	
	      break;

		  case E_CANBUS_STATE_IDLE:
		  	DRV_CANFDSPI_ReceiveChannelEventGet(APP_RX_FIFO, &rxFlags);
			if (canfd_ring_cb.ring_isNotProcessed()) can_state = E_CANBUS_STATE_TX;
			else if(rxFlags & CAN_RX_FIFO_NOT_EMPTY_EVENT) can_state = E_CANBUS_STATE_RX;
		  break;

	      case E_CANBUS_STATE_TX: //Wade: Here is import!!!
		  	canfd_ring_cb.ring_get(&canfd_ring_cb.ring_get_buff[0]);
		  	FIT_CANFD_TX((uint8_t*)&canfd_ring_cb.ring_get_buff[0]);
	        can_state = E_CANBUS_STATE_IDLE;
	      break;

	      case E_CANBUS_STATE_RX:      
	        FIT_CANFD_RX();
	        can_state = E_CANBUS_STATE_IDLE;
	      break;
	      default: break;
            }
		if(flag_do_mc_fw_update)
			k_sleep(K_USEC(100));
		else
			delay_msec(2);
        }
}