/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef BT_HMI_H_
#define BT_HMI_H_

/**
 * @file
 * @defgroup bt_hmi Nordic UART (HMI) GATT Service
 * @{
 * @brief Nordic UART (HMI) GATT Service API.
 */

#include <zephyr/types.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief UUID of the HMI Service. **/
#define BT_UUID_HMI_VAL \
	BT_UUID_128_ENCODE(0x00010000, 0x452d, 0x4269, 0x6b65, 0x58706c6f7661)

/** @brief UUID of the TX Characteristic. **/
#define BT_UUID_HMI_TX_VAL \
	BT_UUID_128_ENCODE(0x00010001, 0x452d, 0x4269, 0x6b65, 0x58706c6f7661)

/** @brief UUID of the RX Characteristic. **/
#define BT_UUID_HMI_RX_VAL \
	BT_UUID_128_ENCODE(0x00010010, 0x452d, 0x4269, 0x6b65, 0x58706c6f7661)

#define BT_UUID_HMI_SERVICE   BT_UUID_DECLARE_128(BT_UUID_HMI_VAL)
#define BT_UUID_HMI_RX        BT_UUID_DECLARE_128(BT_UUID_HMI_RX_VAL)
#define BT_UUID_HMI_TX        BT_UUID_DECLARE_128(BT_UUID_HMI_TX_VAL)

/** @brief HMI send status. */
enum bt_hmi_send_status {
	/** Send notification enabled. */
	BT_HMI_SEND_STATUS_ENABLED,
	/** Send notification disabled. */
	BT_HMI_SEND_STATUS_DISABLED,
};

/** @brief Pointers to the callback functions for service events. */
struct bt_hmi_cb {
	/** @brief Data received callback.
	 *
	 * The data has been received as a write request on the HMI RX
	 * Characteristic.
	 *
	 * @param[in] conn  Pointer to connection object that has received data.
	 * @param[in] data  Received data.
	 * @param[in] len   Length of received data.
	 */
	void (*received)(struct bt_conn *conn,
			 const uint8_t *const data, uint16_t len);

	/** @brief Data sent callback.
	 *
	 * The data has been sent as a notification and written on the HMI TX
	 * Characteristic.
	 *
	 * @param[in] conn Pointer to connection object, or NULL if sent to all
	 *                 connected peers.
	 */
	void (*sent)(struct bt_conn *conn);

	/** @brief Send state callback.
	 *
	 * Indicate the CCCD descriptor status of the HMI TX characteristic.
	 *
	 * @param[in] status Send notification status.
	 */
	void (*send_enabled)(enum bt_hmi_send_status status);

};

/**@brief Initialize the service.
 *
 * @details This function registers a GATT service with two characteristics,
 *          TX and RX. A remote device that is connected to this service
 *          can send data to the RX Characteristic. When the remote enables
 *          notifications, it is notified when data is sent to the TX
 *          Characteristic.
 *
 * @param[in] callbacks  Struct with function pointers to callbacks for service
 *                       events. If no callbacks are needed, this parameter can
 *                       be NULL.
 *
 * @retval 0 If initialization is successful.
 *           Otherwise, a negative value is returned.
 */
int bt_hmi_init(void);


void set_have_hmi_event();

void FIT_HMI_ERROR_Get (uint16_t *err);


int sent_page( uint8_t page_id , uint8_t *p_param );

/**@brief Get maximum data length that can be used for @ref bt_hmi_send.
 *
 * @param[in] conn Pointer to connection Object.
 *
 * @return Maximum data length.
 */
static inline uint32_t bt_hmi_get_mtu(struct bt_conn *conn)
{
	/* According to 3.4.7.1 Handle Value Notification off the ATT protocol.
	 * Maximum supported notification is ATT_MTU - 3 */
	return bt_gatt_get_mtu(conn) - 3;
}



#define Code_HMI_Communication_Error		1
#define Code_Battery_Cell_Over_Voltage		11
#define Code_Battery_Cell_Under_Voltage		12
#define Code_Battery_System Under_Voltage	13
#define Code_Battery_Short_ircuit			14
#define Code_Battery_Over_Discharging_Current	15
#define Code_Battery_Over_Charging_Current		16
#define Code_Battery_Cell_Over_Temperature_Charge	17
#define Code_Battery_Cell_Over_Temperature_Discharge	18
#define Code_Battery_FET_Over_Temperature_Charge	19
#define Code_Battery_FET_Over_Temperature_Discharge	20
#define Code_Battery_Cell_Under_Temperature_Charge	21
#define Code_Battery_Cell_Under_Temperature_Discharge	22
#define Code_Battery_Low_Capacity	23
#define Code_Battery_Fault	24
#define Code_Battery_Cell_NOT_balance	25
#define Code_Battery_Charge_FET_damaged	26
#define Code_Battery_Discharge_FET_damaged	27
#define Code_Battery_Fuse_Blown	28
#define Code_Battery_Others	29
#define Code_Ctrl_Sensor_Speed_Abnormal	49
#define Code_Ctrl_Voltage_Abnormal	47
#define Code_Ctrl_Current_Abnormal	40
#define Code_Ctrl_PAS_Throttle_Abmormal	41
#define Code_Ctrl_Motor_Phase_Lose	42
#define Code_Ctrl_Motor_Hall_Sensor_Abnormal	43
#define Code_Ctrl_Brake_Abnormal	44
#define Code_Ctrl_Over_Temperature	45
#define Code_Ctrl_Motor_Blocked_Rotor	46
#define Code_Ctrl_Others	48

#define Bit_HMI_Communication_Error		1
#define Bit_Battery_Cell_Over_Voltage		(1 << 1)
#define Bit_Battery_Cell_Under_Voltage		(1 << 2)
#define Bit_Battery_System Under_Voltage	(1 << 3)
#define Bit_Battery_Short_ircuit			(1 << 4)
#define Bit_Battery_Over_Discharging_Current	(1 << 5)
#define Bit_Battery_Over_Charging_Current		(1 << 6)
#define Bit_Battery_Cell_Over_Temperature_Charge	(1 << 7)
#define Bit_Battery_Cell_Over_Temperature_Discharge	(1 << 8)
#define Bit_Battery_FET_Over_Temperature_Charge	(1 << 9)
#define Bit_Battery_FET_Over_Temperature_Discharge	(1 << 10)
#define Bit_Battery_Cell_Under_Temperature_Charge	(1 << 11)
#define Bit_Battery_Cell_Under_Temperature_Discharge	(1 << 12)
#define Bit_Battery_Low_Capacity	(1 << 13)
#define Bit_Battery_Fault (1 << 14)
#define Bit_Battery_Cell_NOT_balance	(1 << 15)
#define Bit_Battery_Charge_FET_damaged	(1 << 16)
#define Bit_Battery_Discharge_FET_damaged	(1 << 17)
#define Bit_Battery_Fuse_Blown	(1 << 18)
#define Bit_Battery_Others	(1 << 19)
#define Bit_Ctrl_Sensor_Speed_Abnormal	(1 << 22)
#define Bit_Ctrl_Voltage_Abnormal	(1 << 23
#define Bit_Ctrl_Current_Abnormal	(1 << 24)
#define Bit_Ctrl_PAS_Throttle_Abmormal	(1 << 25)
#define Bit_Ctrl_Motor_Phase_Lose	(1 << 26)
#define Bit_Ctrl_Motor_Hall_Sensor_Abnormal	(1 << 27)
#define Bit_Ctrl_Brake_Abnormal	(1 << 28)
#define Bit_Ctrl_Over_Temperature	(1 << 29)
#define Bit_Ctrl_Motor_Blocked_Rotor	(1 << 30)
#define Bit_Ctrl_Others	(1 << 31)


struct hmi_pass_device_parameter {
	// page 1 2 3
	uint32_t manufacturer ;
	uint16_t manufacture_year ; // Plus 2000 to get A.D. year
	uint8_t manufacture_month ;
	uint8_t manufacture_day ;
	uint32_t serial_number ; 
	uint16_t model_number ;
	uint32_t sw_ver ; 
	uint16_t hw_ver ;
} ;

struct hmi_pass_system_info {
	// page 4
	uint16_t battery_system_year ; // Plus 2000 to get A.D. year
	uint8_t battery_system_month ;
	uint8_t battery_system_day ;
	//uint8_t battery_system_hour ;
	//uint8_t battery_system_min ;
	//uint8_t battery_system_sec ;
	//uint16_t battery_full_capacity ; // mAh
	//uint32_t battery_status ;
} ;

struct hmi_pass_event_data {
	// page 5
        uint8_t sensitivity ;
	uint16_t wheel_size ; 
	
	// page 6
        uint16_t speed_limit ; // 0.1 km/h
        uint8_t CAN_TimeOut_error_code ;
	uint16_t controller_error_code ; // Raw data by manufacturer
	uint32_t motor_error_code ; // Raw data by manufacturer
	uint16_t battery_error_code ; // Raw data by manufacturer
        uint16_t battery2_error_code ; // Raw data by manufacturer
        
	// OP 50
	uint8_t rtc_hour;
	uint8_t rtc_minute;
	uint8_t rtc_second;
} ;


struct hmi_pass_normal_data {
	// page 7
	uint8_t assist_mode ; // 0 is off, 1 ~ N
	uint16_t driving_speed ; // Unit : 1 km/h
	//uint16_t wheel_speed ; // Unit : rpm
        uint16_t  remaining_distance ; // Unit: Km
	uint32_t  trip_distance ; // (DST) Unit : m
	uint32_t odometer ; // (ODO) Unit : m

	// page 8
        uint8_t Ebike_lock_status ;//1:locked, 0:unlocked, 2:
	uint16_t charge_count ;
	uint8_t motor_status ; // 0 is off, 1 is on
	//uint16_t torque_value  ; // Unit : 0.1 Nm

	// page 9
	uint16_t battery_remain_capacity ; //  Unit : mAh
	uint8_t battery_temperature ; //  Unit : °C
	uint16_t battery_current ; //  Unit : mA
	uint16_t battery_voltage ; //  Unit : mV
	uint8_t motor_temperature ; //  Unit : °C
	uint16_t motor_current ; //  Unit : mA
	uint16_t motor_rpm ; //  Unit : rpm
	uint32_t motor_power ; //  Unit : Walt

	// page 10
	uint8_t   mix_battery_level ; //   Unit : %
	uint16_t  battery_full_capacity ; //   Unit : mAh
        uint8_t   battery2_level ; //   Unit : %
	uint16_t  battery2_full_capacity ; //   Unit : mAh
        uint8_t   units_status ; // 0 is km, 1 is miles
	uint8_t   main_light_status ; //   0 is off, 1 is on

	// page 40
	uint16_t torque_voltage ; // mV
} ;

struct hmi_by_pass_can_msg {
	// page 31
	uint32_t can_msg_id ; 
	uint8_t can_data_len ; 
	uint8_t can_data[8] ;
        
        // page 32
	uint32_t log_msg_id ; 
	uint8_t log_data_len ; 
	uint8_t log_data[8] ;
} ;

extern struct hmi_pass_normal_data m_normal_data ;
extern struct hmi_pass_event_data m_event_data ;
extern struct hmi_by_pass_can_msg m_can_buf ;

extern char hmi_sent_step_count ;
extern char hmi_event_step_count ;
extern char hmi_req_ascii_count ;

extern bool is_hmi_connect_flow_start ;
extern bool is_request_ascii_str_pages ;
extern bool is_have_hmi_event ;
extern bool is_do_normal_report_hmi ;
extern bool is_by_pass_mode ;
extern bool is_read_can_msg4pass ;
extern bool is_sent_to_can ;

#ifdef __cplusplus
}
#endif

/**
 *@}
 */

#endif /* BT_HMI_H_ */
