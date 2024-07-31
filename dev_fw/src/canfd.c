/*******************************************************************************
* File Name       : canfd.c
* Description     : 
* Original Author : wangchao
* Created on      : Nov 19, 2020, 2:41 PM
*******************************************************************************/

#ifndef CANFD_C
#define	CANFD_C

//------------------------------------------------------------------------------
// Include
#include "canfd.h"
#include "drv_canfdspi_api.h"
#include "drv_canfdspi_register.h"
#include "drv_canfdspi_defines.h"

#include <logging/log.h>

LOG_MODULE_REGISTER( canfd );

//------------------------------------------------------------------------------
// Constants and Macro Definitions
//#define APP_USE_TX_INT
//#define APP_USE_RX_INT
//------------------------------------------------------------------------------
// Public Variables

//------------------------------------------------------------------------------

#define Sammy_debug 1
CAN_CONFIG config;

// Transmit objects
CAN_TX_FIFO_CONFIG txConfig;
CAN_TX_FIFO_EVENT txFlags;
CAN_TX_MSGOBJ txObj;
uint8_t txd[MAX_DATA_BYTES];

// Receive objects
CAN_RX_FIFO_CONFIG rxConfig;
REG_CiFLTOBJ fObj;
REG_CiMASK mObj;
CAN_RX_FIFO_EVENT rxFlags;
CAN_RX_MSGOBJ rxObj;
uint8_t rxd[MAX_DATA_BYTES];

uint8_t tec;
uint8_t rec;
CAN_ERROR_STATE errorFlags;
fit_can_cb can_cb;
S_CAN_INFO can_info;
int8_t result;

// Private Enumerated and Structure Definitions

//------------------------------------------------------------------------------
// Private Function Prototypes (i.e. static)

//------------------------------------------------------------------------------
// Private Variables (i.e. static)

////////////////////////////////////////////////////////////////////////////////
// Public Function Implementation
/*******************************************************************************
* Public Function : FIT_CANFD_Init
* Description     : 
* Purpose         :
* Input           : 
* Return          : 
* Calls           :    
* Called by       :
* Note			  : 
*******************************************************************************/
void FIT_CANFD_GetID_Data(CAN_TX_MSGOBJ *data, S_CANFD_PROCESS_BUFF *id)
{
	data->word[0] = 0; 	
	data->word[1] = 0;
	
	data->bF.id.SID = 0; 	
	data->bF.id.EID = 0;
	data->bF.ctrl.BRS = 0; //Wade231205:1==>0
	data->bF.ctrl.DLC = CAN_DLC_8; 
	data->bF.ctrl.FDF = 1; //Wade231201:0==>1 (CAN-FD)
	data->bF.ctrl.IDE = 1; //Wade231122:1==>0 
	data->bF.ctrl.RTR = 0;		
	data->bF.ctrl.SEQ = 1;
	
	memset(txd, 0 , (sizeof(txd)/sizeof(uint8_t)));
	data->bF.id.EID = id->tx_id; //Wade231122:EID==>SID
	memcpy(txd, id->payload.data8_t, sizeof(txd));
        //memcpy(txd, id->payload.data, sizeof(txd)); //Wade231201
}

/*******************************************************************************
* Public Function : FIT_CANFD_Init
* Description     : 
* Purpose         :
* Input           : 
* Return          : 
* Calls           :    
* Called by       :
* Note			  : 
*******************************************************************************/
//void FIT_CANFD_Init( void )
int8_t FIT_CANFD_Init( void )
{ 
  
    // Reset device
       result = DRV_CANFDSPI_Reset();
       if (result != 0) return result;

          result = DRV_CANFDSPI_EccEnable();
       if (result != 0) return result;

          result = DRV_CANFDSPI_RamInit(0xff);
       if (result != 0) return result;

          // Configure device
          result = DRV_CANFDSPI_ConfigureObjectReset(&config);
       if (result != 0) return result;
          config.IsoCrcEnable = 1;
          config.StoreInTEF = 0;
          config.TXQEnable = 0;

          result = DRV_CANFDSPI_Configure(&config);
       if (result != 0) return result;
 
          // Setup TX FIFO
          result = DRV_CANFDSPI_TransmitChannelConfigureObjectReset(&txConfig);
       if (result != 0) return result;

       txConfig.FifoSize = 7;
          txConfig.PayLoadSize = CAN_PLSIZE_64;
          txConfig.TxPriority = 1;

          result = DRV_CANFDSPI_TransmitChannelConfigure(APP_TX_FIFO, &txConfig);
       if (result != 0) return result;
 
          // Setup RX FIFO
          result = DRV_CANFDSPI_ReceiveChannelConfigureObjectReset(&rxConfig);
       if (result != 0) return result;

          rxConfig.FifoSize = 15;
          rxConfig.PayLoadSize = CAN_PLSIZE_64;

          result = DRV_CANFDSPI_ReceiveChannelConfigure(APP_RX_FIFO, &rxConfig);
       if (result != 0) return result;

          // Setup RX Filter
       fObj.word = 0;
       fObj.bF.SID = 0x00;
       fObj.bF.SID11 =0;
       fObj.bF.EXIDE = 1;
       fObj.bF.EID = 0x9FF;
          result =  DRV_CANFDSPI_FilterObjectConfigure(CAN_FILTER0, &fObj.bF);
       if (result != 0) return result;
          result = DRV_CANFDSPI_FilterObjectConfigure(CAN_FILTER1, &fObj.bF);
       if (result != 0) return result;
          result = DRV_CANFDSPI_FilterObjectConfigure(CAN_FILTER2, &fObj.bF);
       if (result != 0) return result;
          result = DRV_CANFDSPI_FilterObjectConfigure(CAN_FILTER3, &fObj.bF);
       if (result != 0) return result;
          result = DRV_CANFDSPI_FilterObjectConfigure(CAN_FILTER4, &fObj.bF);
       if (result != 0) return result;

          // Setup RX Mask
    
          mObj.word = 0;
          mObj.bF.MSID = 0x00; //0 means don't care
          mObj.bF.MIDE = 0; // Only allow standard IDs
          mObj.bF.MSID11 = 0;
          mObj.bF.MEID = 0x000; 
          result = DRV_CANFDSPI_FilterMaskConfigure(CAN_FILTER0, &mObj.bF);
       if (result != 0) return result;
          mObj.word = 0;
          mObj.bF.MSID = 0x00; //0 means don't care
          mObj.bF.MSID11 = 0;
          mObj.bF.MEID = 0x100; 
          mObj.bF.MIDE = 0; // Only allow standard IDs

          result = DRV_CANFDSPI_FilterMaskConfigure(CAN_FILTER1, &mObj.bF);
       if (result != 0) return result;
 
          mObj.word = 0;
          mObj.bF.MSID = 0x00; //0 means don't care
          mObj.bF.MIDE = 0; // Only allow standard IDs
          mObj.bF.MSID11 = 0;
          mObj.bF.MEID = 0x200; 
          result = DRV_CANFDSPI_FilterMaskConfigure(CAN_FILTER2, &mObj.bF);
       if (result != 0) return result;
          mObj.word = 0;
          mObj.bF.MSID = 0x00; //0 means don't care
          mObj.bF.MIDE = 0; // Only allow standard IDs
          mObj.bF.MSID11 = 0;
          mObj.bF.MEID = 0x900; 
          result = DRV_CANFDSPI_FilterMaskConfigure(CAN_FILTER3, &mObj.bF);
       if (result != 0) return result;
          mObj.word = 0;
          mObj.bF.MSID = 0x00; //0 means don't care
          mObj.bF.MIDE = 0; // Only allow standard IDs
          mObj.bF.MSID11 = 0;
          mObj.bF.MEID = 0x700; 
          result = DRV_CANFDSPI_FilterMaskConfigure(CAN_FILTER4, &mObj.bF);
       if (result != 0) return result;
 
          // Link FIFO and Filter
          result = DRV_CANFDSPI_FilterToFifoLink(CAN_FILTER0, APP_RX_FIFO, true);
       if (result != 0) return result;
          result = DRV_CANFDSPI_FilterToFifoLink(CAN_FILTER1, APP_RX_FIFO, true);
       if (result != 0) return result;
          result = DRV_CANFDSPI_FilterToFifoLink(CAN_FILTER2, APP_RX_FIFO, true);
       if (result != 0) return result;
          result = DRV_CANFDSPI_FilterToFifoLink(CAN_FILTER3, APP_RX_FIFO, true);
       if (result != 0) return result;
          result = DRV_CANFDSPI_FilterToFifoLink(CAN_FILTER4, APP_RX_FIFO, true);
       if (result != 0) return result;

 
          // Setup Bit Time
          result = DRV_CANFDSPI_BitTimeConfigure(CAN_1000K_4M, CAN_SSP_MODE_AUTO, CAN_SYSCLK_40M);
       if (result != 0) return result;
 
          // Setup Transmit and Receive Interrupts
          result = DRV_CANFDSPI_GpioModeConfigure(GPIO_MODE_INT, GPIO_MODE_INT);
       if (result != 0) return result;
          result = DRV_CANFDSPI_TransmitChannelEventEnable(APP_TX_FIFO, CAN_TX_FIFO_NOT_FULL_EVENT);
       if (result != 0) return result;
          result = DRV_CANFDSPI_ReceiveChannelEventEnable(APP_RX_FIFO, CAN_RX_FIFO_NOT_EMPTY_EVENT);
       if (result != 0) return result;
          result = DRV_CANFDSPI_ModuleEventEnable(CAN_TX_EVENT | CAN_RX_EVENT);
       if (result != 0) return result;

          result = DRV_CANFDSPI_OperationModeSelect(CAN_NORMAL_MODE);
       if (result != 0) return result;
 
       return 0;

}

/*******************************************************************************
* Public Function : FIT_CAN_Init
* Description     : 
* Purpose         :
* Input           : 
* Return          : 
* Calls           :    
* Called by       :
* Note			  : 
*******************************************************************************/
int8_t FIT_CAN_Init( void )
{	
	int8_t result; 
    // Reset device
    result = DRV_CANFDSPI_Reset();
	if (result != 0) return result;

    result = DRV_CANFDSPI_EccEnable();
	if (result != 0) return result;

    result = DRV_CANFDSPI_RamInit(0xff);
	if (result != 0) return result;

    // Configure device
    result = DRV_CANFDSPI_ConfigureObjectReset(&config);
	if (result != 0) return result;
    config.IsoCrcEnable = 1;
    config.StoreInTEF = 0;
    config.TXQEnable = 0;

    result = DRV_CANFDSPI_Configure(&config);
	if (result != 0) return result;
	
    // Setup TX FIFO
    result = DRV_CANFDSPI_TransmitChannelConfigureObjectReset(&txConfig);
	if (result != 0) return result;

	txConfig.FifoSize = 7;
    txConfig.PayLoadSize = CAN_PLSIZE_8;
    txConfig.TxPriority = 1;

    result = DRV_CANFDSPI_TransmitChannelConfigure(APP_TX_FIFO, &txConfig);
	if (result != 0) return result;
	
    // Setup RX FIFO
    result = DRV_CANFDSPI_ReceiveChannelConfigureObjectReset(&rxConfig);
	if (result != 0) return result;

    rxConfig.FifoSize = 15;
    rxConfig.PayLoadSize = CAN_PLSIZE_8;

    result = DRV_CANFDSPI_ReceiveChannelConfigure(APP_RX_FIFO, &rxConfig);
	if (result != 0) return result;

    // Setup RX Filter
	fObj.word = 0;
	fObj.bF.SID = 0x00;
	fObj.bF.SID11 =0;
	fObj.bF.EXIDE = 1;
	fObj.bF.EID = 0x7FF;
   	result =  DRV_CANFDSPI_FilterObjectConfigure(CAN_FILTER0, &fObj.bF);
	if (result != 0) return result;
    result = DRV_CANFDSPI_FilterObjectConfigure(CAN_FILTER1, &fObj.bF);
	if (result != 0) return result;
    result = DRV_CANFDSPI_FilterObjectConfigure(CAN_FILTER2, &fObj.bF);
	if (result != 0) return result;
    result = DRV_CANFDSPI_FilterObjectConfigure(CAN_FILTER3, &fObj.bF);
	if (result != 0) return result;
    result = DRV_CANFDSPI_FilterObjectConfigure(CAN_FILTER4, &fObj.bF);
	if (result != 0) return result;

    // Setup RX Mask allow standard ID and Extend ID
    
    mObj.word = 0;
    mObj.bF.MSID = 0x00; //0 means don't care
    mObj.bF.MIDE = 0; // allow standard ID and Extend ID
    mObj.bF.MSID11 = 0;
    mObj.bF.MEID = 0x000;	
    result = DRV_CANFDSPI_FilterMaskConfigure(CAN_FILTER0, &mObj.bF);
	if (result != 0) return result;
    mObj.word = 0;
    mObj.bF.MSID = 0x00; //0 means don't care
    mObj.bF.MSID11 = 0;
    mObj.bF.MEID = 0x100;	
    mObj.bF.MIDE = 0; // allow standard ID and Extend ID

    result = DRV_CANFDSPI_FilterMaskConfigure(CAN_FILTER1, &mObj.bF);
	if (result != 0) return result;
	
    mObj.word = 0;
    mObj.bF.MSID = 0x00; //0 means don't care
    mObj.bF.MIDE = 0; // allow standard ID and Extend ID
    mObj.bF.MSID11 = 0;
    mObj.bF.MEID = 0x200;	
    result = DRV_CANFDSPI_FilterMaskConfigure(CAN_FILTER2, &mObj.bF);
	if (result != 0) return result;
	mObj.word = 0;
    mObj.bF.MSID = 0x00; //0 means don't care
    mObj.bF.MIDE = 0; // allow standard ID and Extend ID
    mObj.bF.MSID11 = 0;
    mObj.bF.MEID = 0x300;	
    result = DRV_CANFDSPI_FilterMaskConfigure(CAN_FILTER3, &mObj.bF);
	if (result != 0) return result;
	mObj.word = 0;
    mObj.bF.MSID = 0x00; //0 means don't care
    mObj.bF.MIDE = 0; // allow standard ID and Extend ID
    mObj.bF.MSID11 = 0;
    mObj.bF.MEID = 0x700;	
    result = DRV_CANFDSPI_FilterMaskConfigure(CAN_FILTER4, &mObj.bF);
	if (result != 0) return result;
	
    // Link FIFO and Filter
    result = DRV_CANFDSPI_FilterToFifoLink(CAN_FILTER0, APP_RX_FIFO, true);
	if (result != 0) return result;
    result = DRV_CANFDSPI_FilterToFifoLink(CAN_FILTER1, APP_RX_FIFO, true);
	if (result != 0) return result;
    result = DRV_CANFDSPI_FilterToFifoLink(CAN_FILTER2, APP_RX_FIFO, true);
	if (result != 0) return result;
    result = DRV_CANFDSPI_FilterToFifoLink(CAN_FILTER3, APP_RX_FIFO, true);
	if (result != 0) return result;
    result = DRV_CANFDSPI_FilterToFifoLink(CAN_FILTER4, APP_RX_FIFO, true);
	if (result != 0) return result;

	
    // Setup Bit Time
    result = DRV_CANFDSPI_BitTimeConfigure(CAN_1000K_4M, CAN_SSP_MODE_AUTO, CAN_SYSCLK_40M);
	if (result != 0) return result;
	
    // Setup Transmit and Receive Interrupts
    result = DRV_CANFDSPI_GpioModeConfigure(GPIO_MODE_INT, GPIO_MODE_INT);
	if (result != 0) return result;
    result = DRV_CANFDSPI_TransmitChannelEventEnable(APP_TX_FIFO, CAN_TX_FIFO_NOT_FULL_EVENT);
	if (result != 0) return result;
    result = DRV_CANFDSPI_ReceiveChannelEventEnable(APP_RX_FIFO, CAN_RX_FIFO_NOT_EMPTY_EVENT);
	if (result != 0) return result;
    result = DRV_CANFDSPI_ModuleEventEnable(CAN_TX_EVENT | CAN_RX_EVENT);
	if (result != 0) return result;

    result = DRV_CANFDSPI_OperationModeSelect(CAN_CLASSIC_MODE);
	if (result != 0) return result;	
	
	return 0;

}

/*******************************************************************************
* Public Function : FIT_CANFD_Test
* Description     : 
* Purpose         :
* Input           : 
* Return          : 
* Calls           :    
* Called by       :
* Note			  : 
*******************************************************************************/
void FIT_CAN_SEND_SID( uint32_t sid, uint8_t send_data[8] )
{
    uint8_t attempts = 50;
    uint8_t n;
    int16_t i;

    txObj.word[0] = 0;
    txObj.word[1] = 0;
	
    txObj.bF.id.SID = sid;
    txObj.bF.id.EID = 0;
	
    txObj.bF.ctrl.BRS = 1;
    txObj.bF.ctrl.DLC = 8;
    txObj.bF.ctrl.FDF = 1;
    txObj.bF.ctrl.IDE = 0;
    txObj.bF.ctrl.RTR = 0;              ///Note a remote frame request
    txObj.bF.ctrl.SEQ = 1;

    txd[0] = send_data[0];
    txd[1] = send_data[1];
    txd[2] = send_data[2];
    txd[3] = send_data[3];
    txd[4] = send_data[4];
    txd[5] = send_data[5];
    txd[6] = send_data[6];
    txd[7] = send_data[7];	



	do {
#ifdef APP_USE_TX_INT
        Delay_us(50);
#else
        DRV_CANFDSPI_TransmitChannelEventGet(APP_TX_FIFO, &txFlags);
#endif
        if (attempts == 0) {
            DRV_CANFDSPI_ErrorCountStateGet(&tec, &rec, &errorFlags);
            return;
        }
        attempts--;
    }
#ifdef APP_USE_TX_INT
    while (!APP_TX_INT());
#else
    while (!(txFlags & CAN_TX_FIFO_NOT_FULL_EVENT));
#endif

    for (i = 0; i < 10; i++)
    {
        n = DRV_CANFDSPI_DlcToDataBytes(txObj.bF.ctrl.DLC);
		DRV_CANFDSPI_TransmitChannelLoad(APP_TX_FIFO, &txObj, txd, n, true);
		delay_msec(100);
    }
}

/*******************************************************************************
* Private Function: FIT_CANFD_Test_Receiver
* Description     : 
* Purpose         :
* Input           : 
* Return          : 
* Calls           :    
* Called by       :
* Note			  :
*******************************************************************************/
void FIT_CANFD_Test_Receiver(void)
{
     // Check if FIFO is not empty
#ifdef APP_USE_RX_INT    
    if (!nINT1_Get()) {
#else
    DRV_CANFDSPI_ReceiveChannelEventGet(APP_RX_FIFO, &rxFlags);

    if (rxFlags & CAN_RX_FIFO_NOT_EMPTY_EVENT) {
#endif
        // Get message
        DRV_CANFDSPI_ReceiveMessageGet(APP_RX_FIFO, &rxObj, rxd, MAX_DATA_BYTES);
		for(int i = 0; i < MAX_DATA_BYTES; i++)
        	LOG_INF("Receive message's ID = %04x, and rxd[%d] = %02x", rxObj.bF.id.SID, i,  rxd[i]);
    }
}

/*******************************************************************************
* Private Function: FIT_CANFD_TX
* Description     : 
* Purpose         :
* Input           : 
* Return          : 
* Calls           :    
* Called by       :
* Note			  :
*******************************************************************************/
void FIT_CANFD_TX(uint8_t *id)
{
    CAN_TX_MSGOBJ can_tx = {0};
    uint8_t attempts = 50;
    uint8_t n;
	S_CANFD_PROCESS_BUFF *pId = (S_CANFD_PROCESS_BUFF*)id;

	FIT_CANFD_GetID_Data(&can_tx, pId);
	do {

        DRV_CANFDSPI_TransmitChannelEventGet(APP_TX_FIFO, &txFlags);
        if (attempts == 0) {
            DRV_CANFDSPI_ErrorCountStateGet(&tec, &rec, &errorFlags);
            return;
        }
        attempts--;
    }
    while (!(txFlags & CAN_TX_FIFO_NOT_FULL_EVENT));


    n = DRV_CANFDSPI_DlcToDataBytes(can_tx.bF.ctrl.DLC);
    DRV_CANFDSPI_TransmitChannelLoad(APP_TX_FIFO, &can_tx, txd, n, true);		
   // LOG_INF("Transmit message's ID = %04x, and txd[0] = %02x", can_tx.bF.id.EID, txd[0]);
}

/*******************************************************************************
* Private Function: FIT_CANFD_RX
* Description     : 
* Purpose         :
* Input           : 
* Return          : 
* Calls           :    
* Called by       :
* Note			  :
*******************************************************************************/
void FIT_CANFD_RX(void)
{
     // Check if FIFO is not empty
#ifdef APP_USE_RX_INT    
    if (!nINT1_Get()) {
#else
  //  DRV_CANFDSPI_ReceiveChannelEventGet(APP_RX_FIFO, &rxFlags);

  //  if (rxFlags & CAN_RX_FIFO_NOT_EMPTY_EVENT) {
#endif
        // Get message
#ifdef FIT_CANFD
        DRV_CANFDSPI_ReceiveMessageGet(APP_RX_FIFO, &rxObj, rxd, MAX_DATA_BYTES);
		for(int i = 0; i < MAX_DATA_BYTES; i++)
        	LOG_INF("Receive message's ID = %04x, and rxd[%d] = %02x", rxObj.bF.id.SID, i,  rxd[i]);
#else
		DRV_CANFDSPI_ReceiveMessageGet(APP_RX_FIFO, &rxObj, rxd, MAX_DATA_BYTES);

		can_info.SID = rxObj.bF.id.SID;		
		can_info.EID = rxObj.bF.id.EID;
		can_info.DLC = rxObj.bF.ctrl.DLC;

		for(int i = 0; i < 64; i++)
			can_info.data[i] = rxd[i];
		
		can_cb((uint64_t*)&can_info);

		//for(int i = 0; i < 8; i++)
        //LOG_INF("Receive message's ID = %04x %04x, and rxd[%d] = %02x", rxObj.bF.id.SID, rxObj.bF.id.EID, i,  rxd[i]);
	
#endif
    //}
}

/*******************************************************************************
* Private Function: FIT_CANFD_Init_cb
* Description     : 
* Purpose         :
* Input           : 
* Return          : 
* Calls           :    
* Called by       :
* Note			  :
*******************************************************************************/
void FIT_CANFD_Init_cb(fit_can_cb cb)
{
	can_cb = cb;
}

#undef CANFD_C
#endif  /* CANFD_C */
