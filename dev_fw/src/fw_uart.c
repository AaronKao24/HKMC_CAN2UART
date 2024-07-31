#include <stdio.h>
#include <zephyr/types.h>
#include <zephyr.h>
#include <drivers/uart.h>

#include <device.h>
#include <soc.h>

#include <settings/settings.h>


#include "nRF52840_pin_def.h"
#include "fw_api.h"

#include <logging/log.h>

LOG_MODULE_REGISTER( fw_uart );

#define UART_BUF_SIZE 256
#define UART_WAIT_FOR_BUF_DELAY K_MSEC(50)
#define UART_WAIT_FOR_RX CONFIG_BT_NUS_UART_RX_WAIT_TIME

static const struct device *p_dev_uart0;

uint8_t rcvTmp0[UART_BUF_SIZE];
uint8_t u0_data[UART_BUF_SIZE];
short u0_in , u0_get ;

int buf_len = 0 ;
static K_FIFO_DEFINE(fifo_uart_rx_data);
static void uart_cb0(const struct device *dev, struct uart_event *evt, void *user_data)
{
	int i , len ;
	uint8_t *rx_buf , *tx_buf ;
	ARG_UNUSED(dev);

	switch (evt->type) {
	case UART_TX_DONE:
		break;

	case UART_RX_RDY:
		rx_buf = evt->data.rx.buf ;
		len = evt->data.rx.len;
		LOG_INF( "UART<%s" , log_strdup( &(rx_buf[buf_len]) ) );
		for (i = 0; i < len; i++) {
			u0_data[u0_in]  = rx_buf[buf_len+i] ;
			u0_in = (u0_in + 1) % UART_BUF_SIZE ;
		}
		buf_len += len ;
		if ( buf_len > 128 ) {
			uart_rx_disable(p_dev_uart0);
		}
		break;

	case UART_RX_DISABLED:
		buf_len = 0 ;
		memset( rcvTmp0 , 0 , 256 );
		uart_rx_enable(p_dev_uart0, rcvTmp0, UART_BUF_SIZE , UART_WAIT_FOR_RX) ;
		break;

	case UART_RX_BUF_REQUEST:
		uart_rx_buf_rsp(p_dev_uart0, rcvTmp0 , UART_BUF_SIZE );
		break;

	case UART_RX_BUF_RELEASED:
		break;

	case UART_TX_ABORTED:

		break;

	default:
		break;
	}
}

int fw_uart0_init(void)
{
	int err;
	u0_in = 0 , u0_get = 0 ;
	if( p_dev_uart0 != NULL ) {
		return 0 ; // success
	}
	if ((p_dev_uart0 = device_get_binding(DT_LABEL(DT_NODELABEL(uart0)))) == 0) goto CREATE_EXCEPTION ;

	if ((err = uart_callback_set(p_dev_uart0, uart_cb0, NULL)) != 0) goto SETTING_EXCEPTION ;

	if ((err = uart_rx_enable(p_dev_uart0, rcvTmp0, UART_BUF_SIZE , UART_WAIT_FOR_RX)) != 0) goto SETTING_EXCEPTION ;
	return 0 ;
CREATE_EXCEPTION :
	return -ENXIO ;
SETTING_EXCEPTION :
	return err ;
}

int fw_uart0_write(unsigned char *pBuf,int len)
{
	return uart_tx(p_dev_uart0, pBuf , len, SYS_FOREVER_MS);
}

int fw_uart0_read(unsigned char *pBuf,int len)
{
	int i ;

	for (i = 0 ;i < len ;i++ ) {
		if (u0_get == u0_in ) {
			break ;
		}
		pBuf[i] = u0_data[ u0_get ] ;
		u0_get = (u0_get+1) % UART_BUF_SIZE ;
	}
	return i ;
}
