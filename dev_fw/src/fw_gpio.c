
#include <drivers/gpio.h>
#include <device.h>

#include "nRF52840_PIN_DEF.h"
#include "fw_api.h"



static struct gpio_callback gpio_cb[48] ;
const struct device *dev_gpio0 , *dev_gpio1 ;

int fw_config_gpio( short pinNo , int flag )
{
	if( pinNo >= 48 ) return -1 ;
	if( pinNo < 32 ) return gpio_pin_configure(dev_gpio0, pinNo, flag);
	pinNo = pinNo % 32 ;
	return gpio_pin_configure(dev_gpio1, pinNo, flag);
}

int fw_set_gpio_pin( short pinNo , int flag )
{
	if( pinNo >= 48 ) return -1 ;
	if( pinNo < 32 ) return gpio_pin_set(dev_gpio0, pinNo, flag );
	pinNo = pinNo % 32 ;
	return gpio_pin_set(dev_gpio1, pinNo, flag );
}
int fw_get_gpio_pin( short pinNo )
{
	if( pinNo >= 48 ) return -1 ;
	if( pinNo < 32 ) return gpio_pin_get(dev_gpio0, pinNo );
	pinNo = pinNo % 32 ;
	return gpio_pin_get(dev_gpio1, pinNo );
}

int fw_setup_gpio_callback(int pinNo, gpio_fun_callback p_gfc)
{
	int ret , pbit ;
	if( pinNo >= 48 )
		return -1 ;
	pbit = pinNo % 32 ;
	gpio_init_callback( &gpio_cb[pinNo] , p_gfc ,BIT(pbit) );

	if( pinNo >= 32 ) {
		gpio_add_callback(dev_gpio1, &gpio_cb[pinNo]);
		ret = gpio_pin_interrupt_configure(dev_gpio1, pinNo, GPIO_INT_EDGE_RISING); // GPIO_INT_EDGE_TO_ACTIVE
	} else {
		gpio_add_callback(dev_gpio0, &gpio_cb[pinNo]);
		ret = gpio_pin_interrupt_configure(dev_gpio0, pinNo, GPIO_INT_EDGE_RISING); // GPIO_INT_EDGE_TO_ACTIVE
	}
	return ret ;
}


void fw_init_gpio(void)
{
	if ((dev_gpio0 != NULL) && (dev_gpio1 != NULL)) {
		return;
	}
	if (dev_gpio0 == NULL) {
		dev_gpio0 = device_get_binding((const char *)"GPIO_0");
	}
	if (dev_gpio1 == NULL) {
		dev_gpio1 = device_get_binding((const char *)"GPIO_1");
	}
}
