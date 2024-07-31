
#include <zephyr.h>
#include <device.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <drivers/can.h>
#include <drivers/spi.h>
#include <drivers/sensor.h>

#include "nRF52840_PIN_DEF.h"
#include "fw_api.h"


#include <logging/log.h>

LOG_MODULE_REGISTER( fw_spi );

/**
* @brief define spi : 1 (CAN) , 3 (LCM) buffer
*/

static struct spi_buf spi1_rx;
static struct spi_buf_set spi1_rx_bufs = {
	.buffers = &spi1_rx,
	.count = 1,
};

static struct spi_buf spi1_tx;
static struct spi_buf_set spi1_tx_bufs = {
	.buffers = &spi1_tx,
	.count = 1,
};

static struct spi_buf spim3_rx;
static struct spi_buf_set spim3_rx_bufs = {
	.buffers = &spim3_rx,
	.count = 1,
};

static struct spi_buf spim3_tx;
static struct spi_buf_set spim3_tx_bufs = {
	.buffers = &spim3_tx,
	.count = 1,
};

/**
* @brief define spi 1, 3 CS PIN config 
*/

struct spi_cs_control *CAN_ctrl =
	&(struct spi_cs_control) {
		.gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0)),
		.delay = 2,
		.gpio_pin = PIN_CAN_SPI_CS,
		.gpio_dt_flags = GPIO_ACTIVE_LOW
	};

struct spi_cs_control *LCM_ctrl =
	&(struct spi_cs_control) {
		.gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0)),
		.delay = 0,
		.gpio_pin = PIN_LCM_SPI_CS_N,
		.gpio_dt_flags = GPIO_ACTIVE_LOW
	};

const struct device *spi1_devs ;
static struct spi_config spi1_cfg ;
static const struct device *spim3_devs ;
static struct spi_config spim3_cfg ;

/**
* @brief init : 0 1 3 spi device 
*/
//int fw_normal_spi_init(void *pIntDev, uint8_t chipSelect, const uint32_t sckFreq)
//{
//	if( spi1_devs != NULL ) {
//		return -1 ;
//	}
//	spi1_devs = device_get_binding("SPI_1");
//	pIntDev = spi1_devs ;
//	spi1_cfg.operation = SPI_WORD_SET(8);
//	spi1_cfg.frequency = sckFreq ;
//	CAN_ctrl->gpio_pin = chipSelect ;
//	spi1_cfg.cs = CAN_ctrl ;
//	return 0 ;
//}

void fw_normal_spi_init(void)
{
	if( spi1_devs != NULL ) {
		return -1 ;
	}
	spi1_devs = device_get_binding("SPI_1");
	//pIntDev = spi1_devs ;
	spi1_cfg.operation = SPI_WORD_SET(8);
	spi1_cfg.frequency = 5000000U ;
	CAN_ctrl->gpio_pin = PIN_CAN_SPI_CS ;
	spi1_cfg.cs = CAN_ctrl ;
	//spi1_cfg.cs = NULL ;	
	//return 0 ;
}

void fw_fast_spi_init(void)
{
	if (spim3_devs != NULL) {
		return ;
	}
	spim3_devs = device_get_binding("SPIM_3");
	spim3_cfg.operation = SPI_WORD_SET(8);
	spim3_cfg.frequency = 32000000U;
	spim3_cfg.cs = LCM_ctrl ;
}

int fw_normal_spi_write( unsigned char *wbuf , int wlen )
{
	if (spi1_devs == NULL) {
		return -9000 ;
	}
	spi1_tx.buf = wbuf , spi1_tx.len = wlen ;
	return spi_write(spi1_devs, &spi1_cfg, &spi1_tx_bufs );
}

int fw_fast_spi_write( unsigned char *wbuf , int wlen )
{
	if (spim3_devs == NULL) {
		return -9000 ;
	}
	spim3_tx.buf = wbuf , spim3_tx.len = wlen ;
	return spi_write(spim3_devs, &spim3_cfg, &spim3_tx_bufs );
}

int fw_normal_spi_read( unsigned char DEV_NO , unsigned char *rbuf , int rlen )
{
	if (spi1_devs == NULL) {
		return -9000 ;
	}
	spi1_rx.buf = rbuf , spi1_rx.len = rlen ;
	return spi_read(spi1_devs, &spi1_cfg, &spi1_rx_bufs );
}


int fw_fast_spi_read( unsigned char DEV_NO , unsigned char *rbuf , int rlen )
{
	if (spim3_devs == NULL) {
		return -9000 ;
	}
	spim3_rx.buf = rbuf , spim3_rx.len = rlen ;
	return spi_read(spim3_devs, &spim3_cfg, &spim3_rx_bufs );
}
// (*MCP251XFD_SPITransfer_Func)(void *pIntDev, uint8_t chipSelect, uint8_t *txData, uint8_t *rxData, size_t size);
//int fw_normal_spi_trans( void *pIntDev , uint8_t chipSelect, unsigned char *wbuf , int wlen , unsigned char *rbuf , int rlen )
//{
//	if (spi1_devs == NULL) {
//		return -9000 ;
//	}
//	// , int wlen 
//	//  
//	if( rlen == 0 ) {
//		spi1_tx.len = rlen ;
//	} else {
//		
//	}
//	spi1_tx.buf = wbuf , spi1_rx.buf = rbuf , spi1_tx.len = wlen , spi1_rx.len = rlen ;
//	return spi_transceive(spi1_devs, &spi1_cfg, &spi1_tx_bufs , &spi1_rx_bufs );
//}

int fw_normal_spi_trans(  unsigned char *wbuf , int wlen , unsigned char *rbuf , int rlen )
{
	if (spi1_devs == NULL) {
		return -9000 ;
	}
	// , int wlen 
	//  
	if( rlen == 0 ) {
		spi1_tx.len = rlen ;
	} else {
		
	}
	spi1_tx.buf = wbuf , spi1_rx.buf = rbuf , spi1_tx.len = wlen , spi1_rx.len = rlen ;
	return !spi_transceive(spi1_devs, &spi1_cfg, &spi1_tx_bufs , &spi1_rx_bufs );
}

int fw_fast_spi_trans( unsigned char DEV_NO , unsigned char *wbuf , int wlen , unsigned char *rbuf , int rlen )
{

	if (spim3_devs == NULL) {
		return -9000 ;
	}
	spim3_tx.buf = wbuf , spim3_tx.len = wlen , spim3_rx.buf = rbuf , spim3_rx.len = rlen ;
	return spi_transceive(spim3_devs, &spim3_cfg, &spim3_tx_bufs , &spim3_rx_bufs );
}


