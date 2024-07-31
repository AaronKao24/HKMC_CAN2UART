
#include <zephyr.h>
#include <device.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <drivers/can.h>
#include <drivers/spi.h>
#include <drivers/sensor.h>

#include <drivers/display.h>

#include "nRF52840_PIN_DEF.h"
#include "fw_api.h"


#include <logging/log.h>

LOG_MODULE_REGISTER( fw_st7789v2 );
struct display_buffer_descriptor desc = {0};

const struct device *st7789v_devs ;

bool fw_st7789v2_init(void)
{
	st7789v_devs = device_get_binding("ST7789V");
	if (!st7789v_devs) {
		return false ;
	}	
	return true ;
}