#include <stdio.h>

#include <pm/pm.h>
#include <device.h>
#include <drivers/display.h>
#include <logging/log.h>

#include "config.h"
#include "function.h"
#include "fw_api.h"
#include "hal_api.h"


LOG_MODULE_REGISTER( hw_test );

/**
* @fn hw_init_log
* @brief boot test hardware function
* @details 
* When the first power-on test after the circuit is installed, it is used to check whether there is a problem with the circuit
* Mainly to detect whether the connection with each component is normal.<br>
* The hardware test action performed when entering the main program.<br>
* You can use the flag setting to decide whether to do this test.<br>
* It is usually used when debugging hardware circuits.
*/
void hw_init_log(void)
{
	printf("Vicent Chang is FIT0882!\n");
	LOG_INF( "BLE Peripheral INIT = %s" , sys_param.ble_peripheral_init ? "OK" : "NG" );
	LOG_INF( "BLE Centeral INIT = %s" , sys_param.ble_centeral_init ? "OK" : "NG" );

	LOG_INF( "USB INIT = %s" , sys_param.usb_init ? "OK" : "NG" );
	
	LOG_INF( "ADC INIT = %s" , sys_param.adc_init ? "OK" : "NG" );
	
	LOG_INF( "PWM INIT = %s" , sys_param.pwm_init ? "OK" : "NG" );

	LOG_INF( "CAN TEST = %s" , sys_param.can_init ? "OK" : "NG" );

	LOG_INF( "LCM TEST = %s" , sys_param.lcd_init ? "OK" : "NG" );

	LOG_INF( "ROM INIT = %s" , sys_param.flash_rom_init ? "OK" : "NG" );

}

void hal_init_res(char use_dev)
{
	char buf[128] ;
	int size ;
	memset( buf , 0 , 128 );

	sprintf( buf , "BLE Peripheral INIT = %s\r\n\0" , sys_param.ble_peripheral_init ? "OK" : "NG" );
	size = strlen( buf );
	hal_return_command(buf, size, use_dev);

	sprintf( buf , "BLE Centeral INIT = %s\r\n\0" , sys_param.ble_centeral_init ? "OK" : "NG" );
	size = strlen( buf );
	hal_return_command(buf, size, use_dev);

	sprintf( buf , "USB INIT = %s\r\n\0" , sys_param.usb_init ? "OK" : "NG" );
	size = strlen( buf );
	hal_return_command(buf, size, use_dev);

	sprintf( buf , "ADC INIT = %s\r\n\0" , sys_param.adc_init ? "OK" : "NG" );
	size = strlen( buf );
	hal_return_command(buf, size, use_dev);

	sprintf( buf , "PWM INIT = %s\r\n\0" , sys_param.pwm_init ? "OK" : "NG" );
	size = strlen( buf );
	hal_return_command(buf, size, use_dev);

	sprintf( buf , "CAN TEST = %s\r\n\0" , sys_param.can_init ? "OK" : "NG" );
	size = strlen( buf );
	hal_return_command(buf, size, use_dev);

	sprintf( buf , "LCM TEST = %s\r\n\0" , sys_param.lcd_init ? "OK" : "NG" );
	size = strlen( buf );
	hal_return_command(buf, size, use_dev);

	sprintf( buf , "ROM INIT = %s\r\n\0" , sys_param.flash_rom_init ? "OK" : "NG" );
	size = strlen( buf );
	hal_return_command(buf, size, use_dev);

}
