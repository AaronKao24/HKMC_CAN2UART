
#include <string.h>
#include <stdio.h>
#include <zephyr.h>
#include <drivers/display.h>
#include "version_log.h"
#include "fw_api.h"
#include "hal_api.h"
#include "function.h"
#include "config.h"
#include "canfd_process.h"
#include "canfd.h"

//Factory test item use status by Ken
#include <bluetooth/addr.h>
//Factory test item(button) use status by Aaron
#include "nRF52840_PIN_DEF.h"

//Factory test item(backlight) use status by Aaron
#include <hal/nrf_gpio.h>
#include "nRF52840_pin_def.h"

//#define FW_USB(x,y) fw_write_USB_data(x,sizeof(y))

#define	_HKMC_DEMO 1


//Factory test item use status by Ken
static	bt_addr_le_t  fit_mass_scan_mac_adr;
unsigned short do_crc(unsigned short crc, unsigned char *ptr, int len);

//for display driver
extern const struct device *st7789v_devs ;

//for bypass mode
static S_CANFD_PROCESS_BUFF can_bypass_tx_buf;
unsigned char hex_array[10]; 

unsigned char char_to_hex(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return 0; 
}


void string_to_hex_array(const char* str, unsigned char* hex_array, size_t array_size) {
    size_t len = strlen(str);
    size_t hex_len = len / 2;
    if (hex_len > array_size) {
        hex_len = array_size;
    }

    for (size_t i = 0; i < hex_len; i++) {
        hex_array[i] = (char_to_hex(str[i * 2]) << 4) | char_to_hex(str[i * 2 + 1]);
    }
}


char hex_to_char(unsigned char hex) {
    if (hex <= 9) {
        return '0' + hex;
    } else {
        return 'A' + (hex - 10);
    }
}


void hex_array_to_string(const unsigned char* hex_array, size_t array_size, char* str) {
    for (size_t i = 0; i < array_size; i++) {
        str[(i * 2)] = hex_to_char((hex_array[i] >> 4) & 0x0F);
        str[(i * 2) + 1] = hex_to_char(hex_array[i] & 0x0F);
    }
    str[(array_size * 2)] = '\n';
}



#if DO_FW_TEST == 1




#define KW_CMD_SIZE     34
const char *cmd_keywords[ KW_CMD_SIZE ] = {
 "FTM Alive" , "Versions" , "ProductID set" , "ProductID get" , "ProductID2 set" ,
  "ProductID2 get" , "NorFlash ReadWrite" , "LCM " , "BLE TEST" , "Keypad ON",
 "Keypad OFF", "LCM Backlight ON", "LCM Backlight OFF", "BT Address get" , "Penetration Protection On" , 
 "SetTimeout" , "HELP", "GET_CRC" , "Check Penetration status" , "A5" ,
  "DFU" , "ROM" , "UIID" , "SWITCH" , "WD ON" , "WD OFF",  "ByPassMode ON" , "ByPassMode OFF" , "ByPass Version" , "ByPass DATE" , 
  "ByPass Serial NO" , "CAN" , "SerialNo set" , "SerialNo get"} ;

int crc_flag = 0;
#define COLOR_SIZE     5
#define CRC_NUM 0xab73


const char *color_keywords[ COLOR_SIZE ] = { "RED" , "GRE" , "BLU" , "WHI" , "BLA"} ;

#endif

extern void fw_to_USB_DFU(void);

void process_usb_command(char *p_cmd ,int len)
{
	int i , j , k , l , cmd_idx = -1 ;

        char pp_check[17] = "";
        char pid_temp[16] = "";
        char pid2_temp[16] = "";
        
        char can_data_temp[256];

        char can_id[4] = {0};
        char send_data[12];

        for (i = 0 ; i < KW_CMD_SIZE ; i++) {
            if (strncmp(cmd_keywords[i] , p_cmd , strlen(cmd_keywords[i])) == 0){
                cmd_idx = i;
            } 
            
        }
        
        if (cmd_idx == CAN)
        {
            
            if(_HKMC_DEMO)
            {
                
                memset(send_data, 0 , 12);

                if (strncmp( &(p_cmd[3]) , "900" , 3) == 0)
                {
                    if(strncmp(p_cmd[6] , '0' ,1) == 0 &&strncmp(p_cmd[7] , '1' ,1) == 0)
                    {
                        send_data[0] =  '9';
                        send_data[1] =  '1';
                        send_data[2] =  '0';
                        send_data[3] =  '0';
                        send_data[4] =  '1';
                        send_data[5] =  '0';
                        send_data[6] =  '1';
                        send_data[7] =  '0';
                        send_data[8] =  '0';
                        send_data[9] =  '0';
                        send_data[10] = '0';
                        send_data[11] = '\n';
                        fw_write_USB_data(send_data , 12);
                    }
                    else if(strncmp(p_cmd[6] , '0' ,1) == 0 &&strncmp(p_cmd[7] , '4' ,1) == 0)
                    {
                        send_data[0] =  '9';
                        send_data[1] =  '1';
                        send_data[2] =  '0';
                        send_data[3] =  '0';
                        send_data[4] =  '4';
                        send_data[5] =  '0';
                        send_data[6] =  '1';
                        send_data[7] =  '0';
                        send_data[8] =  '0';
                        send_data[9] =  '0';
                        send_data[10] = '0';
                        send_data[11] = '\n';
                        fw_write_USB_data(send_data , 12);
                    }
                }
                if (strncmp( &(p_cmd[3]) , "902" , 3) == 0)
                {
                    if(strncmp(p_cmd[6] , '0' ,1) == 0 &&strncmp(p_cmd[7] , '1' ,1) == 0)
                    {
                        send_data[0] =  '9';
                        send_data[1] =  '1';
                        send_data[2] =  '2';
                        send_data[3] =  '0';
                        send_data[4] =  '1';
                        send_data[5] =  '0';
                        send_data[6] =  '1';
                        send_data[7] =  '0';
                        send_data[8] =  '0';
                        send_data[9] =  '0';
                        send_data[10] = '0';
                        send_data[11] = '\n';
                        fw_write_USB_data(send_data , 12);
                    }
                    else if(strncmp(p_cmd[6] , '0' ,1) == 0 &&strncmp(p_cmd[7] , '4' ,1) == 0)
                    {
                        send_data[0] =  '9';
                        send_data[1] =  '1';
                        send_data[2] =  '2';
                        send_data[3] =  '0';
                        send_data[4] =  '4';
                        send_data[5] =  '0';
                        send_data[6] =  '1';
                        send_data[7] =  '0';
                        send_data[8] =  '0';
                        send_data[9] =  '0';
                        send_data[10] = '0';
                        send_data[11] = '\n';
                        fw_write_USB_data(send_data ,12);
                    }
                }
                if (strncmp( &(p_cmd[3]) , "904" , 3) == 0)
                {
                    if(strncmp(p_cmd[6] , '0' ,1) == 0 &&strncmp(p_cmd[7] , '1' ,1) == 0)
                    {
                        send_data[0] =  '9';
                        send_data[1] =  '1';
                        send_data[2] =  '4';
                        send_data[3] =  '0';
                        send_data[4] =  '1';
                        send_data[5] =  '0';
                        send_data[6] =  '1';
                        send_data[7] =  '0';
                        send_data[8] =  '0';
                        send_data[9] =  '0';
                        send_data[10] = '0';
                        send_data[11] = '\n';
                        fw_write_USB_data(send_data , 12);
                        arch_nop();
                    }
                    else if(strncmp(p_cmd[6] , '0' ,1) == 0 &&strncmp(p_cmd[7] , '4' ,1) == 0)
                    {
                        send_data[0] =  '9';
                        send_data[1] =  '1';
                        send_data[2] =  '4';
                        send_data[3] =  '0';
                        send_data[4] =  '4';
                        send_data[5] =  '0';
                        send_data[6] =  '1';
                        send_data[7] =  '0';
                        send_data[8] =  '0';
                        send_data[9] =  '0';
                        send_data[10] = '0';
                        send_data[11] = '\n';
                        fw_write_USB_data(send_data , 12);
                    }
                }
                 
            }
        
            memset(can_data_temp , 0 , strlen(can_data_temp));
            can_data_temp[0] = '0';
            for (int i=0 ; i < 21 ;i++ )
            {
                can_data_temp[i+1] = p_cmd[i+3];
            }
            string_to_hex_array(can_data_temp, hex_array, sizeof(hex_array));
            can_bypass_tx_buf.tx_id = (hex_array[0]*256)+hex_array[1];
            can_bypass_tx_buf.payload.data = 0;
            can_bypass_tx_buf.payload.bypass_mode_cmd.Send_Data1 = hex_array[2];
            can_bypass_tx_buf.payload.bypass_mode_cmd.Send_Data2 = hex_array[3];
            can_bypass_tx_buf.payload.bypass_mode_cmd.Send_Data3 = hex_array[4];
            can_bypass_tx_buf.payload.bypass_mode_cmd.Send_Data4 = hex_array[5];
            can_bypass_tx_buf.payload.bypass_mode_cmd.Send_Data5 = hex_array[6];
            can_bypass_tx_buf.payload.bypass_mode_cmd.Send_Data6 = hex_array[7];
            can_bypass_tx_buf.payload.bypass_mode_cmd.Send_Data7 = hex_array[8];
            can_bypass_tx_buf.payload.bypass_mode_cmd.Send_Data8 = hex_array[9];
            FIT_CANFD_PROCESS_Tranmit((uint8_t*)&can_bypass_tx_buf);
        }
        else
        {
            
        }
}


/**
* @fn thread_for_usb
* @brief USB thread
*/
void thread_for_usb(void)
{	
	char m_cmd_line[256] ;
	int len = 0 ;
        int len_buf = 0;
	memset(m_cmd_line , 0 , 256);

	while( 1 ) {
		delay_msec( 10 );
		if (fw_is_ble_peripheral_connected()) {
			//continue ;
		}
		if ( !fw_is_usb_connected()) {
			continue ;
		}
		len += fw_read_USB_data( &(m_cmd_line[len]) , (255-len) );
                len_buf = len;
		if (len < 3) {
			continue ;
		}
		if (m_cmd_line[len-1] == '\r' || m_cmd_line[len-1] == '\n') {
			process_usb_command( m_cmd_line , len );
			memset(m_cmd_line , 0 , 256);
			len = 0 ;
                        len_buf = 0;
		}
                
	}
}

/**
* @def USB_PRIORITY 
* The priority of the USB communication process (the smaller the value, the higher the priority)
* @def USB_THREAD_STACK
* Memory size of USB communication process
*/
#define USB_PRIORITY		 5
#define USB_THREAD_STACK	 2048
K_THREAD_DEFINE(usb_communication_id, USB_THREAD_STACK , thread_for_usb, NULL, NULL, NULL, USB_PRIORITY, 0, 0);


