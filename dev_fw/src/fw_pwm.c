#include <zephyr/types.h>
#include <zephyr.h>
#include <drivers/pwm.h>
#include <drivers/led.h>

#include <device.h>
#include <soc.h>


#define LED_PWM_NODE_ID		DT_INST(0, pwm_leds)
#define LED_PWM_DEV_NAME	DT_INST_PROP_OR(0, label, "PWM_0")
#define LED_PWM_LABEL(led_node_id) DT_PROP_OR(led_node_id, label, NULL),

const char *led_label[] = {
	DT_FOREACH_CHILD(LED_PWM_NODE_ID, LED_PWM_LABEL)
};



static const struct device *led_pwm0;

#define PWM_PERIOD 1024

/**
* @brief init pwm ! Get device instance.
*/
int fw_init_pwm(void)
{
	if( led_pwm0 != NULL ) {
		return -2 ;
	}
	led_pwm0 = device_get_binding(LED_PWM_DEV_NAME);
	if( led_pwm0 == NULL ) {
		return -1 ;
	}
	return 0 ;
}

/**
* @brief set pwm strength
* @param value pwm wave strength value, during 0~100
*/
static uint16_t priv_value = 0;
void fw_pwm_set(uint16_t value)
{
	if(priv_value != value)
	{
		pwm_pin_set_cycles(led_pwm0, 39, 64000 , value * 640 , 0) ;
		priv_value = value;
	}
}
