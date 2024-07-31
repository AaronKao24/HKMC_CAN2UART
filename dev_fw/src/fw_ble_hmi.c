/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include "fw_ble_hmi.h"
#include "config.h"
#include "fw_api.h"
#include <logging/log.h>
#include "canfd_process.h"

LOG_MODULE_REGISTER(fw_ble_hmi);

static struct bt_hmi_cb hmi_cb;

struct hmi_pass_normal_data m_normal_data ;
struct hmi_pass_event_data m_event_data ;
struct hmi_by_pass_can_msg m_can_buf ;

char hmi_sent_step_count = -1 ;
char hmi_event_step_count = -1 ;
char hmi_req_ascii_count = -1 ;

bool is_hmi_connect_flow_start ;
bool is_request_ascii_str_pages ;
bool is_have_hmi_event ;
bool is_do_normal_report_hmi ;

bool is_by_pass_mode ;
bool is_read_can_msg4pass ;
bool is_sent_to_can ;
bool is_mc_fw_start = false;

uint8_t PW_LIMIT=100;
struct PW_KEY hmi_code;
long hmi_code_address;

uint16_t ReadValue;
unsigned int searchAddress = Password_Save_Start_Address;
unsigned int maxSearchAddress = Password_Save_End_Address;
static uint8_t pw_info[4]; 



void set_main_light(uint8_t on_off){
	if(on_off)
	{
		FIT_CANFD_PROCESS_LIGHT_Send(true);
		CAN_PAGE_DBG(E_DBG_MESSAGE_LIGHT_ON);
	}
	else
	{
		FIT_CANFD_PROCESS_LIGHT_Send(false);
		CAN_PAGE_DBG(E_DBG_MESSAGE_LIGHT_OFF);
	}
}
void set_turn_light(uint8_t on_off){}
void to_shut_down(){
	if(flag_global_system_boot_on == true)
	{
		flag_off_logo_activate=true;
	}
}

static void bt_hmi_receive_cb(struct bt_conn *conn, const uint8_t *const data, uint16_t len);

void set_have_hmi_event(){
	is_have_hmi_event = true ;
	hmi_event_step_count = 0 ;
}

//=========================================Lock/Unlock=======================================
//struct PW_KEY result = find_key();
struct PW_KEY find_key(){

    struct PW_KEY result;
    int i;

	for (i = 0; searchAddress < maxSearchAddress; i++, searchAddress += 0x04) 
	{
	        // 檢查��位置�KEY�是��xFFFF
		fw_flash_read(searchAddress , 4 , pw_info );
	        if ((pw_info[0] == 0xFF) && (pw_info[1] == 0xFF) && (pw_info[2] == 0xFF) && (pw_info[3] == 0xFF))
		{
	            break; // �到了��止�索
		}
	}
    //�是完全沒���碼����第0筆�0xFFFFFFFF
    if(i ==0)
    {			
	result.save_number = 0;
	result.pw_value[0] = 0xFF;
	result.pw_value[1] = 0xFF;
	result.pw_value[2] = 0xFF;
	result.pw_value[3] = 0xFF;
    }
    else //���傳0xFFFFFFFF ��一筆����
    {
	result.save_number = i;
	fw_flash_read((searchAddress-0x04) , 4 , pw_info );
	result.pw_value[0] = pw_info[0];
	result.pw_value[1] = pw_info[1];
	result.pw_value[2] = pw_info[2];
	result.pw_value[3] = pw_info[3];
    }
    searchAddress = Password_Save_Start_Address;
    maxSearchAddress = Password_Save_End_Address; 
    return result;
}


void store_key(long address, uint8_t value[4])
{
	uint8_t password_value[4] __attribute__ ((aligned (4)));
	password_value[0] = value[0];
	password_value[1] = value[1];
	password_value[2] = value[2];
	password_value[3] = value[3];
	fw_flash_write(address, 4 , password_value);
}

void erase_password()
{
	fw_flash_erase(Password_Save_Start_Address , 1);
}

bool compare_code(uint8_t pin_code[4], uint8_t hmi_code[4])
{
        int i;
        for( i=0; i<4; i++)
        { 
            if(pin_code[i] != hmi_code[i])  
                 return false;
        }
        return true;
}

void lock(uint8_t pin_code[4])
{
	int code_no;
        hmi_code = find_key();

        if(compare_code(pin_code, hmi_code.pw_value) == false)
        {
            //hmi_code != PIN_code, update hmi_code.
            code_no = hmi_code.save_number;
            if(code_no == PW_LIMIT){
                erase_password();
                hmi_code_address = Password_Save_Start_Address;
            }
            else{
                hmi_code_address = Password_Save_Start_Address + hmi_code.save_number*4;
            }
            store_key(hmi_code_address, pin_code);
        }
        else
        {
            //HMI_code = PIN_code, don't update.
        }
        FIT_CANFD_PROCESS_LOCK_Send();
}

void unlock(uint8_t pin_code[4])
{
    hmi_code = find_key();

    if(compare_code(pin_code, hmi_code.pw_value) == false){
        //PIN_code != hmi_code, update error flag. still lock
        flag_PINcode_WRONG = true;
    }else{
        //PIN_code = HMI_code, unlock.
        flag_PINcode_WRONG = false;
        FIT_CANFD_PROCESS_UNLOCK_Send();
    }
}
//=========================================Lock/Unlock=======================================

static void hmi_ccc_cfg_changed(const struct bt_gatt_attr *attr,
				  uint16_t value)
{
	//if (hmi_cb.send_enabled) {
		LOG_DBG("Notification has been turned %s",
			value == BT_GATT_CCC_NOTIFY ? "on" : "off");
		//hmi_cb.send_enabled(value == BT_GATT_CCC_NOTIFY ?
			//BT_HMI_SEND_STATUS_ENABLED : BT_HMI_SEND_STATUS_DISABLED);
                  if( is_hmi_connect_flow_start ) return ;
                  is_hmi_connect_flow_start = true ;
                  is_do_normal_report_hmi = false ;
                  hmi_sent_step_count = 0 ;
	//}
}

static void hmi_rx_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	
}

void set_units(uint8_t km_mile){
        if(flag_switch_speed_units != km_mile )
	{
		FIT_CANFD_PROCESS_UINTS_SET__Send(km_mile);
                //flag_switch_speed_units = km_mile;
                if(km_mile){
                    FIT_CANFD_PROCESS_DBG_MESSAGE_Send(E_DBG_MESSAGE_UNIT_KM);
                }
                else{
                    FIT_CANFD_PROCESS_DBG_MESSAGE_Send(E_DBG_MESSAGE_UNIT_MILE);
                }
	}
	else
	{
	}
}

void set_assist_mode (uint8_t assist_level){
    
    if(global_motor_assist_level != assist_level && assist_level < ASSIST_LEVEL_LIMIT)
	{
		FIT_CANFD_PROCESS_WALK_ASSIST_LEVEL_Send(assist_level);
        }
	else
	{
	}
}


bool check_serious_error (uint16_t hmi_error, uint32_t motor_error, uint16_t battery_error){
    
    uint32_t tmp = motor_error & 0x77FFFFFF;
    if((tmp != 0)||(hmi_error != 0)||(battery_error != 0))
    {
       return true;
    }else
    {
        return false;
    }
}



#define BUFF_SIZE 256
//---------modification list for Add Ebike-----------------------------------------------------
extern unsigned char m_bt_data_buf[] ;
extern int bt_input , bt_readed ;

static ssize_t on_receive(struct bt_conn *conn,
			  const struct bt_gatt_attr *attr,
			  const void *buf,
			  uint16_t len,
			  uint16_t offset,
			  uint8_t flags)
{
	int i ;
	char *pdat = (char*) buf ;
	LOG_INF("Received data, handle %d, conn %p",
		attr->handle, (void *)conn);
	for (i = 0 ;i < len ;i++ ) {
		m_bt_data_buf[ bt_input ] = pdat[i] ;
		bt_input = (bt_input+1) % BUFF_SIZE ;
	}
	//if (hmi_cb.received) {
		bt_hmi_receive_cb(conn, buf, len);
	//}
	return len;
}

static void on_sent(struct bt_conn *conn, void *user_data)
{
	ARG_UNUSED(user_data);

	LOG_DBG("Data send, conn %p", (void *)conn);

	if (hmi_cb.sent) {
		hmi_cb.sent(conn);
	}
}

BT_GATT_SERVICE_DEFINE(ebike_svc,
BT_GATT_PRIMARY_SERVICE(BT_UUID_HMI_SERVICE),
	BT_GATT_CHARACTERISTIC(BT_UUID_HMI_TX,
			       BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ,
			       NULL, NULL, NULL),
	BT_GATT_CCC(hmi_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),


	BT_GATT_CHARACTERISTIC(BT_UUID_HMI_RX,
			       BT_GATT_CHRC_WRITE | BT_GATT_CHRC_BROADCAST | BT_GATT_CHRC_WRITE_WITHOUT_RESP ,
			       BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
			       NULL, on_receive, NULL),
	BT_GATT_CCC(hmi_rx_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),


);

extern bool is_release_rom ;
static int bt_hmi_send(struct bt_conn *conn, const uint8_t *data, uint16_t len);
uint8_t retBuf[3] , prog_chsum , debug_dat ;
uint8_t ui_retBuf[3] , ui_prog_chsum , ui_debug_dat ;
uint8_t progv[256] __attribute__ ((aligned (16)));
uint8_t ui_progv[256] __attribute__ ((aligned (16)));
long ui_prog_offset ;
long prog_offset ;
int idx , size ;
int ui_idx , ui_size ;
uint8_t status ;
uint8_t ui_status ;
int debug_op_code = 0xff ;
static void bt_hmi_receive_cb(struct bt_conn *conn, const uint8_t *const data, uint16_t len)
{
	int i ;
	char addr[BT_ADDR_LE_STR_LEN] = {0};
	char op_code = data[0] ;
        
        // lock/unlock use
        uint8_t pin_code[4]={0};

        bool b_decryptCheckflag = false, serious_error= false;

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, ARRAY_SIZE(addr));
      
        
        serious_error = check_serious_error(flag_hmi_error, global_motor_error_code, global_battery_error_code);
        if((serious_error != 0) && (op_code != 8) && (op_code != 9))
        {
		return ;// if serious error occur, user can only shut down
	}

        if(flag_global_locked_activate == 1)
        {
		if((op_code != 5) && (op_code != 8) && (op_code != 9)){
                  return ;// Lock status can only "shut down" or "unlock"
                }
	}

	switch( op_code ){
	case 1 :	set_assist_mode( data[1] ) ;                                    break ;
	case 2 :	m_event_data.wheel_size = ((uint16_t)data[1] << 8) | data[2] ;	break ;
	case 3 :	m_event_data.speed_limit = ((uint16_t)data[1] << 8) | data[2] ;	break ;
	case 4 :        
                //Set code & lock
                        for( i=0; i<4; i++) pin_code[i] = data[(i+1)];
                        lock(pin_code);
                        break;
        case 5 :
                //comepare code & unlock 
                        for( i=0; i<4; i++) pin_code[i] = data[(i+1)];
                        unlock(pin_code);
                        break;
	case 7 :	set_main_light( data[1] ) ;
			set_turn_light( data[2] ) ;                                     break ;
	case 8 :	to_shut_down();							break ;
	case 9 :
        		//m_event_data.shut_down_timer = 0 ;
        		//for( i = 1 ; i < 5 ; i++ )
        		//	m_event_data.shut_down_timer = (m_event_data.shut_down_timer << 8) | data[i] ;
        		//if(data[1] == 3 && data[2] == 1){
        		//	g_ble_peripheral_state.bits.connected = 0;
                        //g_ble_peripheral_state.bits.pairing_complete = 0;
                        //}
        		break ;
	case 10 :
        		if( (data[1]=='H') && (data[2]=='M') && (data[3]=='C')&& (data[4]=='B') ) {
        			is_request_ascii_str_pages = true ;
                                hmi_req_ascii_count = 0;
        		}
        		break ;  
        case 11 :	m_event_data.sensitivity = data[1] ;                            break ;
	case 30 :	is_by_pass_mode = (data[1] == 1) ;				break ;
	//case 31 :
 //       		if( is_by_pass_mode ) {
 //       			if( (data[1]=='R') && (data[2]=='C') && (data[3]=='B')&& (data[4]=='P') ) 
 //       				is_read_can_msg4pass = true ;
 //       		}
 //       		break ;
	//case 32 :
 //       		if( is_by_pass_mode ) {
 //       			m_can_buf.can_msg_id = 0 ;
 //       			for( i = 1 ; i < 5 ; i++ )
 //       				m_can_buf.can_msg_id = (m_can_buf.can_msg_id<<8) | data[i] ;
 //       			m_can_buf.can_data_len = data[5] ;
 //       			memcpy( m_can_buf.can_data , &(data[6]) , 8 );
 //       			// TODO : call can command api 

 //       			is_sent_to_can = true ;
 //       		}
 //       		break ;
	case 44 ://set units km/mile
                        set_units( data[1] ) ;	                                        break;
	case 50 ://get time from app
        		m_event_data.rtc_hour = data[8];
        		m_event_data.rtc_minute = data[9];
        		m_event_data.rtc_second = data[10];
   
                        total_seconds = ((uint32_t)m_event_data.rtc_hour*3600)+((uint16_t)m_event_data.rtc_minute*60)+m_event_data.rtc_second;
                        break;
        case 80 :
                       FIT_CANFD_PROCESS_SNR_SETTING_Send((data[1] == 1));
                       break;
        
	case 99 :
		{
			struct bt_conn *cur_conn = fw_get_current_connect();
			idx = (data[2] << 8) | data[1];
			status = data[3];
			size = data[4];
    
			// 0x01 Start, 0x03 Retry, 0x04 Error
			switch (status) {
			case 1: 
			       prog_offset = 0x700000;
			       is_mc_fw_start = true;
			       break;
			case 2:
			       break;
			case 3:
			       is_release_rom = false;			       
			       break;
			    }

			memset(progv, 0xFF, sizeof(progv));
			for (i = 0; i < size; i++) {
			      progv[i] = data[i + 5];
			}

			prog_chsum = 0;
			for (i = 1; i < (len - 1); i++) {
			      prog_chsum ^= data[i];
			}
			if(is_mc_fw_start)
			{
				fw_flash_write(prog_offset, 240, progv);
				debug_dat = data[len - 1];
				prog_offset += size;

				retBuf[0] = 0xAA;
				retBuf[1] = data[1];
				retBuf[2] = data[2];

				if (prog_chsum != data[len - 1]) {
				     retBuf[1] = 0;
				     retBuf[2] = 0; // Checksum Error
				}

				bt_hmi_send(cur_conn, retBuf, 3);
			}
			if(status ==3)
			{
				flag_do_mc_fw_update = true;
			}
		}
		break ;
		case 100 :
		{
			struct bt_conn *cur_conn = fw_get_current_connect();
			ui_idx = (data[2] << 8) | data[1];
			ui_status = data[3];
			ui_size = data[4];
    
			// 0x01 Start, 0x03 Retry, 0x04 Error
			switch (ui_status) {
			case 1: 
			       ui_prog_offset = UI_END_ADDRESS;
			       break;
			case 2:
			       break;
			case 3:       
			       break;
			    }

			memset(ui_progv, 0xFF, sizeof(ui_progv));
			for (i = 0; i < ui_size; i++) {
			      ui_progv[i] = data[i + 5];
			}

			ui_prog_chsum = 0;
			for (i = 1; i < (len - 1); i++) {
			      ui_prog_chsum ^= data[i];
			}

			fw_flash_write(ui_prog_offset, 240, ui_progv);
			ui_debug_dat = data[len - 1];
			ui_prog_offset += ui_size;

			ui_retBuf[0] = 0xAB;
			ui_retBuf[1] = data[1];
			ui_retBuf[2] = data[2];

			if (ui_prog_chsum != data[len - 1]) {
			     ui_retBuf[1] = 0;
			     ui_retBuf[2] = 0; // Checksum Error
			}

			bt_hmi_send(cur_conn, ui_retBuf, 3);
		}
		break ;
	}
}



static void bt_hmi_send_enabled(enum bt_hmi_send_status status)
{

}


void bt_hmi_sent(struct bt_conn *conn)
{
	// Hello ! This module was sent data !!
}

struct bt_conn *p_conn ;
int bt_hmi_init()
{
	hmi_sent_step_count = -1 ;
	hmi_event_step_count = -1 ;
	hmi_req_ascii_count = -1 ;
	is_hmi_connect_flow_start = false ;
	is_request_ascii_str_pages = false ;
	is_by_pass_mode = false ;
	is_read_can_msg4pass = false ;
	is_sent_to_can = false ;
	is_have_hmi_event = false ;
	is_do_normal_report_hmi = false ;
	hmi_cb.received = bt_hmi_receive_cb ;
	hmi_cb.send_enabled = bt_hmi_send_enabled ;
	hmi_cb.sent = bt_hmi_sent ;
	return 0;
}


/**@brief Send data.
 *
 * @details This function sends data to a connected peer, or all connected
 *          peers.
 *
 * @param[in] conn Pointer to connection object, or NULL to send to all
 *                 connected peers.
 * @param[in] data Pointer to a data buffer.
 * @param[in] len  Length of the data in the buffer.
 *
 * @retval 0 If the data is sent.
 *           Otherwise, a negative value is returned.
 */
static int bt_hmi_send(struct bt_conn *conn, const uint8_t *data, uint16_t len)
{

	struct bt_gatt_notify_params params = {0};
	const struct bt_gatt_attr *attr = &ebike_svc.attrs[2];

	params.attr = attr;
	params.data = data;
	params.len = len;
	params.func = on_sent;

	if (!conn) {
		LOG_DBG("Notification send to all connected peers");
		return bt_gatt_notify_cb(NULL, &params);
	} else if (bt_gatt_is_subscribed(conn, attr, BT_GATT_CCC_NOTIFY)) {
		return bt_gatt_notify_cb(conn, &params);
	} else {
		return -EINVAL;
	}
}



const char error_bit_to_code[] = { 
 1,11,12,13,14,15,16,17, 18,19,20,21,22,23,24,25, 
26,27,28,29, 0, 0,49,47, 40,41,42,43,44,45,46,48 } ;



int sent_page( uint8_t page_id , uint8_t *p_param ){
	uint8_t data_ary[23] ; // BT package limit is 22 ! In here only use 22
	struct hmi_pass_device_parameter *p_pdp = NULL ;
	struct hmi_pass_system_info *p_psi = NULL ;
	struct hmi_pass_event_data *p_ped = NULL ;
	struct hmi_pass_normal_data *p_pnd = NULL ;
	struct hmi_by_pass_can_msg *p_pcm = NULL ;
	struct bt_conn *cur_conn = fw_get_current_connect();

	if(cur_conn == NULL)
        {
		return 0;
	}

        //if(g_ble_peripheral_state.bits.pairing_complete == 0)
        //{
        //        return 0;
        //}
        

	memset( data_ary , 0 , sizeof(data_ary) );

	switch( page_id ){
	case 1 : case 2 : case 3 :	p_pdp = (struct hmi_pass_device_parameter *)p_param ;	break ;
	case 4 :			p_psi = (struct hmi_pass_system_info *)p_param ;	break ;
	case 5 : case 6 :		p_ped = (struct hmi_pass_event_data *)p_param ;		break ;
	case 7 : case 8 : case 9 :
	case 10 : case 40 :		p_pnd = (struct hmi_pass_normal_data *)p_param ;	break ;
	case 31 : case 32 :             p_pcm = (struct hmi_by_pass_can_msg *)p_param ;		break ;
	default :
		if( (page_id >= 11) && (page_id <= 29) ){
			break ;
		}
		return -99 ; // no such command
	}

	data_ary[1] = page_id ;
	switch( page_id ){
	case 1 :
	case 2 :
	case 3 :
		data_ary[2] = ( p_pdp->manufacturer>>24 ) & 0xFF ;
		data_ary[3] = ( p_pdp->manufacturer>>16 ) & 0xFF ;
		data_ary[4] = ( p_pdp->manufacturer>>8 ) & 0xFF ;
		data_ary[5] = ( p_pdp->manufacturer & 0xFF ) ;
		data_ary[6] = ( p_pdp->manufacture_year - 2000 )  & 0xFF ;
		data_ary[7] = p_pdp->manufacture_month & 0xFF ;
		data_ary[8] = p_pdp->manufacture_day  & 0xFF ;
		data_ary[9] = ( p_pdp->serial_number>>24 ) & 0xFF ;
		data_ary[10] = ( p_pdp->serial_number>>16 ) & 0xFF ;
		data_ary[11] = ( p_pdp->serial_number>>8 ) & 0xFF ;
		data_ary[12] = ( p_pdp->serial_number & 0xFF ) ;
		data_ary[13] = ( p_pdp->model_number>>8 ) & 0xFF ;
		data_ary[14] = ( p_pdp->model_number & 0xFF ) ;
		data_ary[15] = ( p_pdp->sw_ver>>24 ) & 0xFF ;
		data_ary[16] = ( p_pdp->sw_ver>>16 ) & 0xFF ;
		data_ary[17] = ( p_pdp->sw_ver>>8 ) & 0xFF ;
		data_ary[18] = ( p_pdp->sw_ver & 0xFF ) ;
		data_ary[19] = ( p_pdp->hw_ver>>8 ) & 0xFF ;
		data_ary[20] = ( p_pdp->hw_ver & 0xFF ) ;
		break ;
	case 4 :
		data_ary[8] = (p_psi->battery_system_year - 2000)  & 0xFF ; // Plus 2000 to get A.D. year
		data_ary[9] = p_psi->battery_system_month ;
		data_ary[10] = p_psi->battery_system_day ;
		break ;
	case 6 :
		//data_ary[3] = ( p_ped->speed_limit>>8 ) & 0xFF ;
		//data_ary[4] = ( p_ped->speed_limit & 0xFF ) ;
                data_ary[10] = p_ped->CAN_TimeOut_error_code ;
                data_ary[11] = ( p_ped->controller_error_code>>8 ) & 0xFF ;
		data_ary[12] = ( p_ped->controller_error_code & 0xFF ) ;
		data_ary[13] = ( p_ped->motor_error_code>>24 ) & 0xFF ;
		data_ary[14] = ( p_ped->motor_error_code>>16 ) & 0xFF ;
		data_ary[15] = ( p_ped->motor_error_code>>8 ) & 0xFF ;
		data_ary[16] = ( p_ped->motor_error_code & 0xFF ) ;
		data_ary[17] = ( p_ped->battery_error_code>>8 ) & 0xFF ;
		data_ary[18] = ( p_ped->battery_error_code & 0xFF ) ;
		data_ary[19] = ( p_ped->battery2_error_code>>8 ) & 0xFF ;
		data_ary[20] = ( p_ped->battery2_error_code & 0xFF ) ;
		break ;
	case 7 :
		data_ary[2] = p_pnd->assist_mode ;
		data_ary[3] = ( p_pnd->driving_speed>>8 ) & 0xFF ;
		data_ary[4] = ( p_pnd->driving_speed & 0xFF ) ;
		//data_ary[5] = ( p_pnd->wheel_speed>>8 ) & 0xFF ;
		//data_ary[6] = ( p_pnd->wheel_speed & 0xFF ) ;
		data_ary[9] = ( p_pnd->remaining_distance>>8 ) & 0xFF ;
		data_ary[10] = ( p_pnd->remaining_distance & 0xFF ) ;
		data_ary[11] = ( p_pnd->trip_distance>>24 ) & 0xFF ;
		data_ary[12] = ( p_pnd->trip_distance>>16 ) & 0xFF ;
		data_ary[13] = ( p_pnd->trip_distance>>8 ) & 0xFF ;
		data_ary[14] = ( p_pnd->trip_distance & 0xFF ) ;
		data_ary[15] = ( p_pnd->odometer>>24 ) & 0xFF ;
		data_ary[16] = ( p_pnd->odometer>>16 ) & 0xFF ;
		data_ary[17] = ( p_pnd->odometer>>8 ) & 0xFF ;
		data_ary[18] = ( p_pnd->odometer & 0xFF ) ;
		break ;
	case 8 :
		data_ary[2] = p_pnd->Ebike_lock_status ;
                data_ary[3] = ( p_pnd->charge_count>>8 ) & 0xFF ;
		data_ary[4] = ( p_pnd->charge_count & 0xFF ) ;
		//data_ary[5] = ( p_pnd->torque_value>>8 ) & 0xFF ;
		//data_ary[6] = ( p_pnd->torque_value & 0xFF ) ;
                data_ary[7] = p_pnd->motor_status ;
		data_ary[11] = ( p_pnd->trip_distance>>24 ) & 0xFF ;
		data_ary[12] = ( p_pnd->trip_distance>>16 ) & 0xFF ;
		data_ary[13] = ( p_pnd->trip_distance>>8 ) & 0xFF ;
		data_ary[14] = ( p_pnd->trip_distance & 0xFF ) ;
		break ;
	case 9 :
		data_ary[2] = p_pnd->assist_mode ;
		data_ary[3] = ( p_pnd->driving_speed>>8 ) & 0xFF ;
		data_ary[4] = ( p_pnd->driving_speed & 0xFF ) ;
		//data_ary[5] = ( p_pnd->battery_remain_capacity>>8 ) & 0xFF ;
		//data_ary[6] = ( p_pnd->battery_remain_capacity & 0xFF ) ;
		//data_ary[7] = p_pnd->battery_temperature ;
		//data_ary[8] = ( p_pnd->battery_current>>8 ) & 0xFF ;
		//data_ary[9] = ( p_pnd->battery_current & 0xFF ) ;
		//data_ary[10] = ( p_pnd->battery_voltage>>8 ) & 0xFF ;
		//data_ary[11] = ( p_pnd->battery_voltage & 0xFF ) ;
		//data_ary[12] = p_pnd->motor_temperature ;
		//data_ary[13] = ( p_pnd->motor_current>>8 ) & 0xFF ;
		//data_ary[14] = ( p_pnd->motor_current & 0xFF ) ;
		//data_ary[15] = ( p_pnd->motor_rpm>>8 ) & 0xFF ;
		//data_ary[16] = ( p_pnd->motor_rpm & 0xFF ) ;
		//data_ary[17] = ( p_pnd->motor_power>>24 ) & 0xFF ;
		//data_ary[18] = ( p_pnd->motor_power>>16 ) & 0xFF ;
		//data_ary[19] = ( p_pnd->motor_power>>8 ) & 0xFF ;
		//data_ary[20] = ( p_pnd->motor_power & 0xFF ) ;
		break ;

	case 10 :
		data_ary[2] = p_pnd->assist_mode ;
		data_ary[3] = ( p_pnd->driving_speed>>8 ) & 0xFF ;
		data_ary[4] = ( p_pnd->driving_speed & 0xFF ) ;
		data_ary[5] = p_pnd->assist_mode ;
		data_ary[6] = p_pnd->mix_battery_level ; //   Unit : %
		//data_ary[7] = ( p_pnd->battery_full_capacity>>8 ) & 0xFF ;
		//data_ary[8] = ( p_pnd->battery_full_capacity & 0xFF ) ;
		//data_ary[9] = p_pnd->battery2_level ; //   Unit : %
		//data_ary[10] = ( p_pnd->battery_full_capacity>>8 ) & 0xFF ;
		//data_ary[11] = ( p_pnd->battery_full_capacity & 0xFF ) ;
                data_ary[14] = p_pnd->units_status ; //   0 is km, 1 is miles
		data_ary[15] = p_pnd->main_light_status ; //   0 is off, 1 is on
		break ;
		
	//case 31 : // CAN Directory
	//	data_ary[2] = ( p_pcm->can_msg_id>>24 ) & 0xFF ;
	//	data_ary[3] = ( p_pcm->can_msg_id>>16 ) & 0xFF ;
	//	data_ary[4] = ( p_pcm->can_msg_id>>8 ) & 0xFF ;
	//	data_ary[5] = ( p_pcm->can_msg_id & 0xFF ) ;
	//	data_ary[6] = ( p_pcm->can_data_len & 0xFF ) ;
	//	memcpy( &data_ary[7] , p_pcm->can_data , 8 ) ;
	//	break ;
          
        //case 32 : // log message from HMI in ï¿½ï¿½By Pass Modeï¿½ï¿½
	//	data_ary[2] = ( p_pcm->log_msg_id>>24 ) & 0xFF ;
	//	data_ary[3] = ( p_pcm->log_msg_id>>16 ) & 0xFF ;
	//	data_ary[4] = ( p_pcm->log_msg_id>>8 ) & 0xFF ;
	//	data_ary[5] = ( p_pcm->log_msg_id & 0xFF ) ;
	//	data_ary[6] = ( p_pcm->log_data_len & 0xFF ) ;
	//	memcpy( &data_ary[7] , p_pcm->log_data , 8 ) ;
	//	break ;

	
	default : // 11 ~ 28
		memcpy( &data_ary[2] , p_param , 16 );
		break ;
	}
	return bt_hmi_send( cur_conn , &data_ary[1] , 20 );
}




