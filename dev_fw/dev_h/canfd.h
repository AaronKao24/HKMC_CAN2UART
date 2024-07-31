/*******************************************************************************
* File Name       : canfd.h
* Description     : 
* Original Author : wangchao
* Created on      : Nov 19, 2020, 2:41 PM
*******************************************************************************/

#ifndef CANFD_H
#define	CANFD_H

#ifdef	__cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Conditional Definitions
#ifdef CANFD_C
    #define CANFD_PUBLIC
    #define CANFD_CONST
#else
    #define CANFD_PUBLIC extern
    #define CANFD_CONST  const
#endif

//#define FIT_CANFD 
//------------------------------------------------------------------------------    
// Include
#include <stdio.h>
#include "canfd_process.h"
//------------------------------------------------------------------------------
// Public Constants and Macro Definitions
// Transmit Channels
#define APP_TX_FIFO CAN_FIFO_CH2

// Receive Channels
#define APP_RX_FIFO CAN_FIFO_CH1

/*--------------------- Macro for exam----------------------------------------*/    

/*--------------------- Macro for exam----------------------------------------*/    
    
//------------------------------------------------------------------------------
// Public Enumerated and Structure Definitions
typedef void (*fit_can_cb)(uint8_t *);
	
typedef struct {
    uint32_t SID : 11;
    uint32_t EID : 18;
    uint8_t DLC;
    uint8_t data[64];
}S_CAN_INFO;



//------------------------------------------------------------------------------
// Public Variables

//------------------------------------------------------------------------------
// Public Function Prototypes
//void FIT_CANFD_Init( void );
int8_t FIT_CANFD_Init( void );
int8_t FIT_CAN_Init( void );
void FIT_CAN_SEND_SID( uint32_t sid, uint8_t send_data[8] );  
void FIT_CANFD_Test_Receiver (void); 
void FIT_CANFD_RX(void);
void FIT_CANFD_Init_cb(fit_can_cb cb);


void FIT_CANFD_TX(uint8_t *id);



/******************************************************************************/    
#ifdef	__cplusplus
}
#endif

#endif	/* CANFD_H */

