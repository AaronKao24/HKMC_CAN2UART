#include <stdio.h>
#include <string.h>
#include <device.h>
#include <drivers/uart.h>
#include <zephyr.h>
#include <sys/ring_buffer.h>
#include <hal/nrf_power.h>
#include <nrfx_nvmc.h>
#include <usb/usb_device.h>
#include <pm/pm.h>
#include <device.h>
#include <soc.h>

#include "fw_api.h"
#include <logging/log.h>


LOG_MODULE_REGISTER(cdc_acm_echo, LOG_LEVEL_INF);

uint8_t ring_read_buffer[2048];
uint8_t ring_write_buffer[256];

struct ring_buf ring_read_buf;
struct ring_buf ring_write_buf;

/**
* @fn interrupt_handler
* @brief USB uart callback handle
*/
static void interrupt_handler(const struct device *dev, void *user_data)
{
	ARG_UNUSED(user_data);

	while (uart_irq_update(dev) && uart_irq_is_pending(dev)) {

		if (uart_irq_rx_ready(dev)) {
			int recv_len, rb_len;
			uint8_t buffer[256];
			size_t len = MIN(ring_buf_space_get(&ring_read_buf), sizeof(buffer));
			recv_len = uart_fifo_read(dev, buffer, len);
			rb_len = ring_buf_put(&ring_read_buf, buffer, recv_len);
			if (rb_len < recv_len) {
				LOG_ERR("Drop %u bytes", recv_len - rb_len);
			}
			LOG_DBG("tty fifo -> ring_read_buffer %d bytes", rb_len);
		}

		if (uart_irq_tx_ready(dev)) {
			uint8_t buffer[256];
			int rb_len, send_len, i ;
			rb_len = ring_buf_get(&ring_write_buf, buffer, sizeof(buffer));
			
			if (!rb_len) {
				LOG_DBG("Ring buffer empty, disable TX IRQ");
				uart_irq_tx_disable(dev);
				continue;
			}
			send_len = uart_fifo_fill(dev, buffer, rb_len);
			if (send_len < rb_len) {
				LOG_ERR("Drop %d bytes", rb_len - send_len);
			}
			LOG_DBG("ringbuf -> tty fifo %d bytes", send_len);
		}
		delay_msec( 10 );
	}
}

const struct device *usb_dev;
/**
* @
*
*/
int fw_init_usb_virtual_COM(void)
{
	uint32_t baudrate ;
	uint16_t uicr_d ;
	int ret;
	ret = usb_enable(NULL);
	if (ret != 0) {
		LOG_ERR("Failed to enable USB");
		return ret ;
	}
	memcpy( &uicr_d , NRF_UICR_BASE + 0x80 , 2 );
	if( uicr_d != 0xFFFF ){
		nrfx_nvmc_uicr_erase();
		delay_msec( 50 );
		if( uicr_d == 0xDF11 ){
			return 111 ;
		}
	}

	if( usb_dev != NULL ) {
		return -111 ;
	}
	usb_dev = device_get_binding("CDC_ACM_0");
	if (!usb_dev) {
		LOG_ERR("CDC ACM device not found");
		return -222 ;
	}

	ring_buf_init(&ring_read_buf, sizeof(ring_read_buffer), ring_read_buffer);
	ring_buf_init(&ring_write_buf, sizeof(ring_write_buffer), ring_write_buffer);

	LOG_INF("Wait for DTR");

	// They are optional, we use them to test the interrupt endpoint 
	ret = uart_line_ctrl_set(usb_dev, UART_LINE_CTRL_DCD, 1);
	if (ret) {
		LOG_WRN("Failed to set DCD, ret code %d", ret);
	}

	ret = uart_line_ctrl_set(usb_dev, UART_LINE_CTRL_DSR, 1);
	if (ret) {
		LOG_WRN("Failed to set DSR, ret code %d", ret);
	}

	// Wait 1 sec for the host to do all settings 
	k_busy_wait(1000000);

	ret = uart_line_ctrl_get(usb_dev, UART_LINE_CTRL_BAUD_RATE, &baudrate);
	if (ret) {
		LOG_WRN("Failed to get baudrate, ret code %d", ret);
	} else {
		LOG_INF("Baudrate detected: %d", baudrate);
	}

	uart_irq_callback_set(usb_dev, interrupt_handler);

	// Enable rx interrupts
	uart_irq_rx_enable(usb_dev);
	return 0 ;
}

bool fw_is_usb_dfu(void){
	uint16_t xuicr_d ;
	memcpy( &xuicr_d , NRF_UICR_BASE + 0x80 , 2 );
	if( xuicr_d == 0xDF11 ){
		nrfx_nvmc_uicr_erase();
		return true ;
	}
	return false ;
}

int fw_uninit_usb_virtual_COM()
{
	if (usb_dev == NULL) {
		return 0 ; // not init
	}
	usb_deconfig();
	usb_disable();
	usb_dev = NULL ;
	return 1 ; // uninit finish
}

int fw_read_USB_data(unsigned char* pCmd, int len)
{
	int rb_len = ring_buf_get( &ring_read_buf , pCmd , len );
	return rb_len ;
}

void fw_write_USB_data(unsigned char* pRes, int len)
{
	ring_buf_put( &ring_write_buf, pRes, len);
	uart_irq_tx_enable(usb_dev);
}

char fw_is_usb_connected(void)
{
	return nrf_power_usbregstatus_vbusdet_get(NRF_POWER) ? 1 : 0 ;
}

void fw_to_USB_DFU(void)
{
	uint16_t to_dfu = 0xDF11 ;
	nrfx_nvmc_bytes_write( NRF_UICR_BASE + 0x80 , &to_dfu , 2 );
	k_sleep(K_MSEC(100));
	while ( nrfx_nvmc_write_done_check() != true );
	k_sleep(K_MSEC(100));
	NVIC_SystemReset();
}

void fw_write_ui_version(char *ppcmd ,int len)
{
        int i;
        char pid_temp1[16] = "";
        int iresult = 0;
        char pid_temp2[16] = "";
        
        int pid_temp1_counter = fix_add_counter(&pid_temp1);
        int pid_temp2_counter = fix_add_counter(&pid_temp2);
        int uiid_temp_counter = fix_add_counter(&uiid_temp);
        uiid_len[0] = len;
         memset(uiid_temp , 0 , 30);
        for (i=0 ; i<len ;i++)
        {
            uiid_temp[i+uiid_temp_counter] = ppcmd[i];
        }
        
        fw_flash_read(PCBA_PID , 12 , pid_temp1 + pid_temp1_counter);
        fw_flash_read(ALLSYSTEM_PID , 12 , pid_temp2 + pid_temp2_counter);
        
        fw_flash_erase(0,1);
        fw_flash_write(UIID_LEN_VALUE , 1 , uiid_len);
        iresult = fw_flash_write(UI_ADD , len+(4 - len%4) , uiid_temp+uiid_temp_counter);
        if(iresult == 0) {
               fw_write_USB_data("SUCCESS\r\n", 9);                       
        }
        else {
               fw_write_USB_data("ERROR\r\n", 7);   
        }
        
        fw_flash_write(PCBA_PID , 12 , pid_temp1+pid_temp1_counter);
        fw_flash_write(ALLSYSTEM_PID , 12 , pid_temp2+pid_temp2_counter);

}
