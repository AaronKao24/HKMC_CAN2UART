#include <init.h>
#include <sys/__assert.h>
#include <sys/byteorder.h>
#include <hal/nrf_gpio.h>
#include <device.h>
#include <logging/log.h>
#include "nRF52840_pin_def.h"

/** LOG_MODULE_REGISTER : to register dev_int file log module */
LOG_MODULE_REGISTER(dev_init);

/**
* @file dev_init.c
* @brief Set the input and output status of GPIO at system startup.
* @deprecated When the bootloader starts, the devices defined in the device tree will start initializing. You have to set GPIO before device initialization.
* All devices in the device tree will be initialized one by one during POST_KERNEL. At this time, each GPIO needs to be set first to avoid interference.
* Or when the IC needs to pull Reset or Enable first, but the device tree does not have the definition of the PIN, then this initialization function needs to be executed first.
* This function is especially important in the power control of the boot sequence! The bootstrap section is described in zephyr/include/init.h.
* See function @ref user_init.
*/


/**
* @fn user_init
* @brief initialize device pin and output
* This function is defined in soc in the device tree. So the initial code will run before surrounding devices.
* You can find the settings in nrf52840dk_nrf52840.overlay file.
*/
static int user_init(const struct device *dev)
{
	nrf_gpio_cfg_output(PIN_CAN_SPI_CS);
	nrf_gpio_cfg_output(PIN_CAN_SPI_MOSI);
	nrf_gpio_cfg_output(PIN_CAN_SPI_SCK);
	nrf_gpio_cfg_output(PIN_CAN_STBY);
	nrf_gpio_cfg_input(PIN_CAN_SPI_MISO,NRF_GPIO_PIN_PULLUP);
	nrf_gpio_cfg_input(PIN_CAN_INT_0,NRF_GPIO_PIN_PULLUP);
	nrf_gpio_cfg_input(PIN_CAN_INT_1,NRF_GPIO_PIN_PULLUP);

	nrf_gpio_cfg_output(PIN_LCM_SPI_CS_N);
	nrf_gpio_cfg_output(PIN_LCM_SPI_SDA);
	nrf_gpio_cfg_output(PIN_LCM_SPI_WRX);
	nrf_gpio_cfg_output(PIN_LCM_SPI_SCK);
        
	nrf_gpio_pin_set(PIN_PWR_KEY_DET);	
	nrf_gpio_cfg_output(PIN_PWR_KEY_DET);
	nrf_gpio_cfg_output(PIN_LCM_RESET_N);



	nrf_gpio_cfg_input(PIN_VBUS_5V_DET,NRF_GPIO_PIN_PULLUP);
	nrf_gpio_cfg_input(PIN_CC1_DET,NRF_GPIO_PIN_PULLUP);
	nrf_gpio_cfg_input(PIN_CC2_DET,NRF_GPIO_PIN_PULLUP);

	nrf_gpio_cfg_input(PIN_PWR_MODE_H,NRF_GPIO_PIN_PULLUP);
	nrf_gpio_cfg_input(PIN_PWR_MODE_L,NRF_GPIO_PIN_PULLUP);
	nrf_gpio_cfg_input(MCU_FUNCTION,NRF_GPIO_PIN_PULLUP);	
	nrf_gpio_cfg_input(MCU_LIGHT_CRT,NRF_GPIO_PIN_PULLUP);

	nrf_gpio_cfg_output(PIN_DBG_UART_TX);
	nrf_gpio_cfg_input(PIN_DBG_UART_RX,NRF_GPIO_PIN_PULLUP);

	nrf_gpio_pin_set(PIN_USB_PRSW_EN);
	nrf_gpio_cfg_output(PIN_USB_PRSW_EN);

	nrf_gpio_pin_set(PIN_MF_SPI_SIO1);
	nrf_gpio_cfg_output(PIN_MF_SPI_SIO1);
	nrf_gpio_pin_set(PIN_MF_SPI_SIO2);
	nrf_gpio_cfg_output(PIN_MF_SPI_SIO2);
	nrf_gpio_pin_set(PIN_MF_SPI_SIO3);
	nrf_gpio_cfg_output(PIN_MF_SPI_SIO3);

	// USB_CTL1 USB_CTL2 USB_CTL3 : 1 0 X
	nrf_gpio_pin_set(PIN_USB_CTL1);
	//nrf_gpio_pin_clear(PIN_USB_CTL2);
	nrf_gpio_pin_clear(PIN_USB_CTL3);
	nrf_gpio_pin_set(PIN_MF_SPI_CS_N);
	nrf_gpio_cfg_output(PIN_MF_SPI_CS_N);

	nrf_gpio_cfg_output(PIN_USB_CTL1);
	//nrf_gpio_cfg_output(PIN_USB_CTL2);
	nrf_gpio_cfg_output(PIN_USB_CTL3);

	nrf_gpio_cfg_input(PIN_PID_A,NRF_GPIO_PIN_PULLUP);
	
	return 0;
}

const char gpio_out_pin[] = { 
 PIN_CAN_SPI_CS, PIN_CAN_SPI_MOSI, PIN_CAN_SPI_SCK, PIN_CAN_STBY, PIN_LCM_SPI_CS_N,
 PIN_LCM_SPI_SDA, PIN_LCM_SPI_SCK, PIN_LCM_CRT, PIN_LCM_RESET_N,PIN_PWR_KEY_DET, 
 PIN_MF_SPI_SIO0, PIN_MF_SPI_SCLK, PIN_USB_PRSW_EN
 } ;

bool dev_check_is_output(int pin_val)
{
	int i ;
	for (i = 0 ; i < 23 ; i++) {
		if (pin_val == gpio_out_pin[i]) {
			return true ;
		}
	}
	return false ;
}

const char gpio_in_pin[] = {
 PIN_CAN_SPI_MISO,PIN_CAN_INT_0,PIN_CAN_INT_1,PIN_VBUS_5V_DET,PIN_CC1_DET,
 PIN_CC2_DET,PIN_PWR_MODE_H,PIN_PWR_MODE_L,MCU_FUNCTION,MCU_LIGHT_CRT,
 PIN_PID_A
} ;


bool dev_check_is_input(int pin_val)
{
	int i ;
	for (i = 0 ; i < 18 ; i++) {
		if (pin_val == gpio_in_pin[i]) {
			return true ;
		}
	}
	return false ;
}

// foo : do nothing !
static int set_func(const struct device *dev)
{
	return 0;
}

// foo : do nothing !
static int read_func(const struct device *dev)
{
	return 0;
}

typedef int (*user_api_read)(const struct device *dev);
typedef int (*user_api_setup)(const struct device *dev);

__subsystem struct user_init_api {
	user_api_setup         board_setup;
	user_api_read          read;
};

static const struct user_init_api uset_init_api = {
	.board_setup = set_func,
	.read        = read_func,
};



/**
* @def INIT_DEVICE_PRIORITY
* @brief define this device init process priority
* @details Because it is expected that this initialization will be performed first, the priority value is given smaller.
*/
#define INIT_DEVICE_PRIORITY 1
DEVICE_DT_DEFINE( DT_NODELABEL(init0) , user_init , NULL, NULL, NULL, PRE_KERNEL_1, INIT_DEVICE_PRIORITY, &uset_init_api);
