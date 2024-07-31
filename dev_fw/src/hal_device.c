
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr.h>
#include <device.h>
#include <usb/usb_device.h>
#include <date_time.h>

#include <hal/nrf_ficr.h>
#include <logging/log.h>

#include "nRF52840_PIN_DEF.h"
#include "dev_api.h"
#include "fw_api.h"
#include "hal_api.h"
#include "sw_api.h"
//---------modification list for Ken---------
#include <bluetooth/addr.h>
#include "canfd_process.h"

LOG_MODULE_REGISTER( hal_device );

struct system_parameter_buffer sys_param ;
struct product_info m_product ;

static bool is_dev_inited = false ;
void hal_return_command(unsigned char* pRes, int len, char dev )
{
	if( dev == DEV_CONNECT_USB  ){
		fw_write_USB_data(pRes,len);
	}else if( dev == DEV_CONNECT_BLE ){
//		fw_ble_peripheral_write(pRes,len);
	}
}

int hal_get_trans_data(unsigned char* pRes, int len, char dev )
{
	if( dev == DEV_CONNECT_USB  ){
		return fw_read_USB_data( pRes , len );
	}else if( dev == DEV_CONNECT_BLE ){
//		return fw_ble_peripheral_read(pRes,len);
	}
	return -1 ;
}

bool hal_is_init_finish(void)
{
	return is_dev_inited ;
}

bool test_res ;
bool is_skip_cdc_acm = false ;

//---------modification list for Ken---------
//static	bt_addr_le_t fit_mass_scan_mac_adr;
//---------modification list for Ken---------
void hal_init_mass_production_sys_device( bt_addr_le_t  *scan_mac_addr )
{
        sys_param.ble_centeral_init = (fw_ble_centeral_init(sys_param.ble_peripheral_init, scan_mac_addr)==0);
        arch_nop();
}

void hal_init_system_device( bool ble_p  , bool ble_c )
{
	is_dev_inited = false;

	LOG_INF("Init GPIO and Set Device PIN active ... ");
	fw_init_gpio();

	FIT_CANBUS_INIT();

	LOG_INF("Init ADC ... ");
	sys_param.adc_init = (fw_init_adc() == 0) ;

	LOG_INF("Init PWM ... ");
	sys_param.pwm_init = (fw_init_pwm() == 0) ;
	
	//if(global_bms_soc_level ==1)
	//{
		fw_pwm_set(45);
	//}
	//else 
	//{
	//	fw_pwm_set(45);
	//}


	LOG_INF("Init Flash ROM ... ");
	sys_param.flash_rom_init = fw_init_qspi_flash_rom();

	
	LOG_INF("Init USB Virtual COM ... ");
	if( fw_is_usb_dfu() ){
		usb_enable(NULL);
		is_skip_cdc_acm = true ;
	}

	//	test_res = test_qspi_flash_ROM() ;
	LOG_INF("Init BLE ... ");
	//---------modification list for Ken---------
	if(ble_p){
		sys_param.ble_peripheral_init = (fw_ble_peripheral_init()==0);
	}
	if( ! is_skip_cdc_acm ){
		LOG_INF("Init USB Virtual COM ... ");
		sys_param.usb_init = (fw_init_usb_virtual_COM() == 0) ;
	}

	
	LOG_INF("Init ST7789V2 ... ");
	sys_param.lcd_init = fw_st7789v2_init();


	//fw_write_USB_data(SYS_BTN_ASSIST_ICREASE, strlen(SYS_BTN_ASSIST_ICREASE ));
	//fw_write_USB_data(SYS_BTN_ASSIST_DECREASE, strlen(SYS_BTN_ASSIST_DECREASE ));	
	//fw_write_USB_data(SYS_BTN_SCREEN_SCROLL, strlen(SYS_BTN_SCREEN_SCROLL ));
	//fw_write_USB_data(SYS_BTN_POWER, strlen(SYS_BTN_POWER ));

	
	fw_init_wdt();

	is_dev_inited = true ;
}

/**
* @fn hal_get_gap_mac
* @param pbuf mac string (12 Char) buffer pointer
* @brief get ble mac address
* These registers are randomly generated in our production and they are unique to each device
* 2^46 different combinations。Because MSbits set "11" , so mac_1 | 0xC000
*/
void hal_get_gap_mac(char *pbuf)
{
	uint32_t mac_0 ;
	uint16_t mac_1 ;
	mac_1 = (uint16_t)((NRF_FICR->DEVICEADDR[1] | 0xC000) & 0xFFFF) ;
	mac_0 = NRF_FICR->DEVICEADDR[0] ;
	sprintf( pbuf , "%04X%08X" , mac_1 , mac_0 );
}

void hal_get_device_id(char *pbuf)
{
	uint32_t id0, id1;
	id0 = nrf_ficr_deviceid_get( NRF_FICR , 0 );
	id1 = nrf_ficr_deviceid_get( NRF_FICR , 1 );
	sprintf( pbuf , "%08X%08X" , id0 , id1 );
}

// PIN 23 H
int hal_process_pin_action(char *ppcmd, char *pres, int len)
{
	int i , pin_val ;
	// 23 H
	while ((ppcmd[0] == ' ') && (len > 0)) { // skip empty
		ppcmd++ , len-- ;
	}
	//23 H
	i = indexof_sp_char( ppcmd , len );
	pin_val = atoi( ppcmd ); 	// set PIN number
	if ((pin_val > 48) || (pin_val < 0)) {
		return -2 ; // value error
	}
	ppcmd += i, len -= i ; // H
	while ((ppcmd[0] == ' ') && (len > 0)) { // skip empty
		ppcmd++ , len-- ;
	}
	// process action
	switch(ppcmd[0]){
	case 'H':
	case 'L':
#if 0
		if (dev_check_is_output(pin_val)) 
		{
			fw_set_gpio_pin(pin_val, ppcmd[0] == 'H' ? 1 : 0);
		} else {
			return -3 ; // not output
		}
#endif
		break ;
	case '?':
	default:
		*pres = (char)fw_get_gpio_pin(pin_val);
		return 1 ;
	}
	return 0 ;
}

	int dbg_size ;
	long dbg_offset ;
	char dbg_udev ;
#define IMG_SZ_BUF	896

/*
* -> OK
* <- [][][] [][]...128Byte...[] [CRC]
* -> OK / NG
* ...
* <- [][][] [][]...128Byte...[] [CRC]
* -> OK / NG
*/
void do_big_data_trans(long offset , int size,char dev_use)
{
	int i , len , cnt = 0 , read_sz ;
	unsigned char chsum , buf[IMG_SZ_BUF + 10] ;
	dbg_udev = dev_use ;
	if(size > IMG_SZ_BUF){
		read_sz = IMG_SZ_BUF, size -= IMG_SZ_BUF;
	} else {
		read_sz = size ;
	}

	delay_msec( 50 );
	hal_return_command( "??", 2, dev_use );

	len = 0 ;
	while( 1 ) {
		if( (++cnt) > 500 ) { // fail when over 5sec
			break ;
		}
		delay_msec(3);
		len += hal_get_trans_data( &(buf[len]) , (IMG_SZ_BUF + 1 -len) , dev_use );
		if (len <= read_sz) {
			continue ;
		}
		cnt = 0 ;
		len = 0 ;
		chsum = 0 ;
		for( i = 0 ; i < read_sz ; i++ ) {
			chsum ^= buf[i] ;
		}
		dbg_size = size ;
		dbg_offset = offset ;

		if( chsum != buf[i] ) {
			hal_return_command("NG",2, dev_use );
		}else{
			if( fw_flash_write( offset , read_sz , buf ) != 0 ){
				hal_return_command("NG",2, dev_use );
			} else {
				hal_return_command("OK",2, dev_use );
			}
		}
		if( size < 0 ) break ;
		offset += read_sz ;
		if(size > IMG_SZ_BUF){
			read_sz = IMG_SZ_BUF ;
		} else {
			read_sz = size ;
		}
		size -= IMG_SZ_BUF ;
	}
}
/**
* @fn hal_process_rom_action(char *ppcmd, char *pres, int len)
* @brief Process ROM Command
* @param ppcmd ROM command string pointer
* @param pres Result string pointer of ROM command.
* @param len  ROM command string length.
* Process folloing command as : <br>
* ROM W 1000h 9 AABBCCDD1234567890 <br>
* ROM E 1000h 1 <br>
* ROM R 1000h 9 <br>
* ROM B 1000h 9513 <br>
*/
int hal_process_rom_action(char *ppcmd, char *pres, int len, char dev_use)
{
	char cmd_tp ;
	int i , size ;
	long offset ;
	// W 1000h 9 AABBCCDD1234567890
	while ((ppcmd[0] == ' ') && (len > 0)) { // skip empty
		ppcmd++ , len-- ;
	}
	cmd_tp = ppcmd[0] ;
	ppcmd++ ; // 1000h 9 AABBCCDD1234567890
	while ((ppcmd[0] == ' ') && (len > 0)) { // skip empty
		ppcmd++ , len-- ;
	}
	//1000h 9 AABBCCDD1234567890
	i = indexof_sp_char( ppcmd , len );
	offset = hex_to_long( ppcmd , i );
	ppcmd += i, len -= i ;
	// 9 AABBCCDD1234567890
	while ((ppcmd[0] == ' ') && (len > 0)) { // skip empty
		ppcmd++ , len-- ;
	}
	i = indexof_sp_char( ppcmd , len );
	if( i > 0 ) ppcmd[i] = 0 ;
	size = atoi( ppcmd );

	 dbg_size = size ;
	 dbg_offset = offset ;

	switch (cmd_tp){
	case 'W':
	{
		unsigned char hex_buf[256];
		ppcmd += (i + 1) , len -= i ;  // AABBCCDD1234567890
		str_to_hex_ary(ppcmd, hex_buf, len);
	}
		break ;
	case 'E': //1
		fw_flash_erase( offset , size );
		return 2 ;
	case 'B':
		do_big_data_trans(offset , size, dev_use);

		break ;
	case 'R': //9
		
		return 0 ; // read ROM data OK
	default :
		break ;
	}
	return 1 ; // No Command
}

void delay_msec(short value)
{
	k_sleep(K_MSEC( value ));
}

// sleep unit is : 100 usec 
void delay_usec(short value)
{
	//k_sleep(K_NSEC( value ));
	int i ;
	volatile int cnt = 0 ;
	value *= 7 ;
	for( i = 0 ; i < value ; i+=2 ) {
		cnt = (cnt + i) % 7 ;
	}
}



int get_vbus_5v_det(void){
	if(fw_get_gpio_pin(PIN_VBUS_5V_DET) != 0){
		return 1 ;
	}
	return 0 ;
}

int get_pid_a_det(void){
	if(fw_get_gpio_pin(PIN_PID_A) != 0){
		return 1 ;
	}
	return 0 ;
}

int usb_control_1(uint8_t on_off){
	return fw_set_gpio_pin(PIN_USB_CTL1,on_off);
}

int usb_control_3(uint8_t on_off){
	return fw_set_gpio_pin(PIN_USB_CTL3,on_off);
}


int usb_load_switch_control(uint8_t on_off){
	return fw_set_gpio_pin(PIN_USB_PRSW_EN,on_off);
}



