
#include <zephyr.h>
#include <device.h>
#include <drivers/watchdog.h>
#include <sys/reboot.h>
#include <task_wdt/task_wdt.h>
#include <sys/printk.h>
#include <stdbool.h>
#include <logging/log.h>


LOG_MODULE_REGISTER( fw_wdt );

void fw_init_wdt(void)
{
	int ret;
	const struct device *hw_wdt_dev = DEVICE_DT_GET( DT_COMPAT_GET_ANY_STATUS_OKAY(nordic_nrf_watchdog) );

	if (!device_is_ready(hw_wdt_dev)) {
		LOG_DBG("Hardware watchdog %s is not ready; ignoring it.\n", hw_wdt_dev->name);
		hw_wdt_dev = NULL;
	}

	ret = task_wdt_init(hw_wdt_dev);
	if (ret != 0) {
		LOG_DBG("task wdt init failure: %d\n", ret);
		return;
	}

}
