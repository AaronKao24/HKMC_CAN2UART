#include <stdio.h>

#include <pm/pm.h>
#include <device.h>
#include <drivers/display.h>
#include <logging/log.h>

#include "config.h"
#include "function.h"
#include "fw_api.h"
#include "hal_api.h"

#if DO_FW_TEST == 1


LOG_MODULE_REGISTER(HW_TEST);

extern struct system_parameter_buffer sys_param ;


#if FW_TEST_QSPI_ROM == 1

	uint8_t test_out[128] , test_in[128] ;
/**
* @fn test_qspi_flash_ROM
* @brief QSPI flash ROM read and write test
* @attention This test should not be turned on all the time, it will hurt the flash ROM, you can turn it off after the test!
*/
bool test_qspi_flash_ROM()
{
	int i ;

	fw_flash_erase(0x10000, 1);

	memset(test_in, 0, 0);
	for (i = 0;i < 128 ;i++ ) {
		test_out[ i ] = i ;
	}

	delay_msec(20);
	
	fw_flash_write(0x10000, 8, test_out);
	delay_msec(10);

	fw_flash_read(0x10000, 8, test_in);
	
	if (memcmp(test_out, test_in, 128) == 0) {
		return true ;
	}
	return false ;
}
#endif

char test_input_pin ;
char test_output_pin , old_out_pin ;
char in_pin_state ;
char out_pin_state , old_out_state ;
uint16_t test_brigntness , old_brigntness ;
uint16_t test_thermal_adc ;
uint16_t test_cc_adc ;

/**
* @fn hw_test
* @brief boot test hardware function
* @details 
* When the first power-on test after the circuit is installed, it is used to check whether there is a problem with the circuit
* Mainly to detect whether the connection with each component is normal.<br>
* The hardware test action performed when entering the main program.<br>
* You can use the flag setting to decide whether to do this test.<br>
* It is usually used when debugging hardware circuits.
*/
void hw_test(void)
{
	//LOG_INF( "BLE INIT = %s" , sys_param.bluetooth_init ? "OK" : "NG" );

	LOG_INF( "USB INIT = %s" , sys_param.usb_init ? "OK" : "NG" );
	
	LOG_INF( "ADC INIT = %s" , sys_param.adc_init ? "OK" : "NG" );
	
	LOG_INF( "PWM INIT = %s" , sys_param.pwm_init ? "OK" : "NG" );

#if FW_TEST_SPI_CAN == 1
	LOG_INF( "CAN TEST = %s" , sys_param.can_init ? "OK" : "NG" );
	if ( sys_param.can_init ) {

	}	
#endif

#if FW_TEST_SPI_LCM == 1
	LOG_INF( "LCM TEST = %s" , sys_param.lcd_init ? "OK" : "NG" );
	if ( sys_param.lcd_init ) {

	}
#endif

#if FW_TEST_QSPI_ROM == 1
	LOG_INF( "ROM INIT = %s" , sys_param.flash_rom_init ? "OK" : "NG" );
	if ( sys_param.flash_rom_init ) {
#if FW_QSPI_ROM_RE_TEST == 1

		bool test_res = test_qspi_flash_ROM() ;
		LOG_INF( "ROM R/W TEST = %s" , test_res ? "OK" : "NG" );
#endif
	}	
#endif

}

#else
void hw_test(void)
{
	// empty function
}
#endif