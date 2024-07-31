#ifndef _HAL_API_H
#define _HAL_API_H

#define PRODUCT_AREA_START		0
#define SETTING_AREA_START		0x2000
#define IMAGE_AREA_START		0x3000
#define PROGRAM_AREA_START		0x400000
#define RECORD_AREA_START		0x600000


struct product_info {
	char serial_no[16] ;
	int tester_id ;
	short year ;
	char mon ;
	char day ;
	char hour ;
	char min ;
	char sec ;
	char LCD:1 ; // pass or fail
	char BTN1:1 ; // pass or fail
	char BTN2:1 ; // pass or fail
	char BTN3:1 ; // pass or fail
	char USB:1 ; // pass or fail
	char ROM:1 ; // pass or fail
	char BLE:1 ; // pass or fail
	char CAN:1 ; // pass or fail
} ;


#define DEV_CONNECT_BLE	1
#define DEV_CONNECT_USB	2

void hal_return_command(unsigned char* pRes, int len, char dev );

int hal_get_trans_data(unsigned char* pRes, int len, char dev );
void hal_init_res(char use_dev);

bool hal_is_init_finish(void);


void hal_init_system_device( bool ble_p  , bool ble_c );


void hal_get_device_id(char *pbuf);
void hal_get_gap_mac(char *pbuf);

void run_rom_command( char *ppcmd ,int len ,char dev_use);
void output_device_datetime( char use_dev );
void output_device_mac_from(char use_dev);
void output_device_id_from(char use_dev);

struct system_parameter_buffer {
	bool flash_rom_init ;
	bool usb_init ;
	bool can_init ;
	bool lcd_init ;
	bool adc_init ;
	bool pwm_init ;
        //---------modification list for Ken---------
	bool ble_centeral_init ;
	bool ble_peripheral_init ;
} ;
extern struct system_parameter_buffer sys_param ;

//---------modification list for Ken---------
typedef struct {
        uint8_t  mac_adr_tpye;
	uint8_t  mac_adr[6];
} mass_production_addr_t;


void output_device_id_from(char use_dev);
void output_mac_from(char use_dev);

#endif
