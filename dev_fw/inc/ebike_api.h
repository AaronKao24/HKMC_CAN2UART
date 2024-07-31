
//---------modification list for Ebike Potocol---------
#define EBIKE_MAX_LEN 20

union {	
  	unsigned char Matrix[EBIKE_MAX_LEN];
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char motor_manufacturer[4];
            unsigned char motor_manufacture_year;
            unsigned char motor_manufacture_month;
            unsigned char motor_manufacture_day;
            unsigned char motor_serial_number[4];
            unsigned char motor_model_number[2];
            unsigned char motor_sw_ver[4];
            unsigned char motor_hw_ver[2];
        }Page1;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char controller_manufacturer[4];
            unsigned char controller_manufacture_year;
            unsigned char controller_manufacture_month;
            unsigned char controller_manufacture_day;
            unsigned char controller_serial_number[4];
            unsigned char controller_model_number[2];
            unsigned char controller_sw_ver[4];
            unsigned char controller_hw_ver[2];
        }Page2;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char battery_manufacturer[4];
            unsigned char battery_manufacture_year;
            unsigned char battery_manufacture_month;
            unsigned char battery_manufacture_day;
            unsigned char battery_serial_number[4];
            unsigned char battery_model_number[2];
            unsigned char battery_sw_ver[4];
            unsigned char battery_hw_ver[2];
        }Page3;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char controller_system_year;
            unsigned char controller_system_month;
            unsigned char controller_system_day;
            unsigned char controller_system_hour;
            unsigned char controller_system_min;
            unsigned char controller_system_sec;
            unsigned char battery_system_year;
            unsigned char battery_system_month;
            unsigned char battery_system_day;
            unsigned char battery_system_hour;
            unsigned char battery_system_min;
            unsigned char battery_system_sec;
            unsigned char battery_full_capacity[2];
            unsigned char battery_health;
            unsigned char battery_status[4];
        }Page4;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char shut_down_timer[4];
            unsigned char wheel_size[2];
            unsigned char main_light_status;
            unsigned char sensitivity;
            unsigned char charge_count[2];
            unsigned char last_charge_distance[4];
            unsigned char last_charge_time[4];
            unsigned char assist_mode_num;
        }Page5;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char motor_status;
            unsigned char speed_limit[2];
            unsigned char controller_temperature;
            unsigned char dsp_temperature;
            unsigned char sensor_temperature;
            unsigned char overload_speed[2];
            unsigned char overload_temperature;
            unsigned char overload_current[2];
            unsigned char overload_voltage[2];
            unsigned char motor_error_code[2];
            unsigned char controller_error_code[2];
            unsigned char battery_error_code[2];
        }Page6;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char assist_mode;
            unsigned char driving_speed[2];
            unsigned char wheel_speed[2];
            unsigned char total_time[4];
            unsigned char trip_distance[4];
            unsigned char odometer[4];
            unsigned char drive_range[2];
        }Page7;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char assist_mode;
            unsigned char driving_speed[2];
            unsigned char torque_value[2];
            unsigned char total_time[4];
            unsigned char trip_distance[4];
            unsigned char power_of_pedaling[4];
            unsigned char cadence[2];
        }Page8;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char assist_mode;
            unsigned char driving_speed[2];
            unsigned char battery_remain_capacity[2];
            unsigned char battery_temperature;
            unsigned char battery_current[2];
            unsigned char battery_voltage[2];
            unsigned char motor_temperature;
            unsigned char motor_current[2];
            unsigned char motor_rpm[2];
            unsigned char motor_power[4];
        }Page9;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char assist_mode;
            unsigned char driving_speed[2];
            unsigned char assist_mode_num;
            unsigned char battery_level;
            unsigned char battery_full_capacity[2];
            unsigned char up_key_short_press;
            unsigned char up_key_long_press;
            unsigned char down_key_short_press;
            unsigned char down_key_long_press;
            unsigned char power_key_short_press;
            unsigned char power_key_long_press;
            unsigned char main_light_status;
            unsigned char sensitivity;
            unsigned char controller_error_code[2];
            unsigned char battery_error_code[2];
        }Page10;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char motor_serial_number_1[16];
            unsigned char reserved[3];
        }Page11;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char motor_serial_number_2[16];
            unsigned char reserved[3];
        }Page12;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char motor_model_number[16];
            unsigned char reserved[3];
        }Page13;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char motor_sw_ver[16];
            unsigned char reserved[3];
        }Page14;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char motor_hw_ver[16];
            unsigned char reserved[3];
        }Page15;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char controller_serial_number_1[16];
            unsigned char reserved[3];
        }Page16;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char controller_serial_number_2[16];
            unsigned char reserved[3];
        }Page17;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char controller_model_number[16];
            unsigned char reserved[3];
        }Page18;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char controller_sw_ver[16];
            unsigned char reserved[3];
        }Page19;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char controller_hw_ver[16];
            unsigned char reserved[3];
        }Page20;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char battery_serial_number_1[16];
            unsigned char reserved[3];
        }Page21;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char battery_serial_number_2[16];
            unsigned char reserved[3];
        }Page22;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char battery_model_number[16];
            unsigned char reserved[3];
        }Page23;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char battery_sw_ver[16];
            unsigned char reserved[3];
        }Page24;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char battery_hw_ver[16];
            unsigned char reserved[3];
        }Page25;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char motor_manufacturer[16];
            unsigned char reserved[3];
        }Page26;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char controller_manufacturer[16];
            unsigned char reserved[3];
        }Page27;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char battery_manufacturer[16];
            unsigned char reserved[3];
        }Page28;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char can_msg_id[4];
            unsigned char can_data_len;
            unsigned char can_data[8];
        }Page31;
        //-------------------------------------------------
        struct {
            unsigned char id;
            unsigned char assist_mode;
            unsigned char driving_speed[2];
            unsigned char error_collection[4];
            unsigned char torque_voltage[2];
            unsigned char cw;
            unsigned char ccw;
            unsigned char motor_voltage[2];
            unsigned char throttle_signal;
            unsigned char calorie;
            unsigned char last_charge_date_time[4];
        }Page40;
}Ebike_RxBleData;

union {	
  	unsigned char Matrix[EBIKE_MAX_LEN];
        struct {
            unsigned char op_code;
            unsigned char set_assist_mode;
        }OpCode1;
        struct {
            unsigned char op_code;
            unsigned char set_wheel_size[2];
        }OpCode2;
        struct {
            unsigned char op_code;
            unsigned char set_speed_limit[2];
        }OpCode3;
        struct {
            unsigned char op_code;
            unsigned char set_main_light;
            unsigned char set_turn_light;
        }OpCode7;
        struct {
            unsigned char op_code;
        }OpCode8;
        struct {
            unsigned char op_code;
            unsigned char set_shut_down_timer[4];
        }OpCode9;
        struct {
            unsigned char op_code;
            unsigned char magic_key[4];
        }OpCode10;
        struct {
            unsigned char op_code;
            unsigned char set_sensitivity;
        }OpCode11;
        struct {
            unsigned char op_code;
            unsigned char set_by_pass;
        }OpCode30;
        struct {
            unsigned char op_code;
            unsigned char agic_key[4];
        }OpCode31;
        struct {
            unsigned char op_code;
            unsigned char can_msg_id[4];
            unsigned char can_data_len;
            unsigned char can_data[8];
        }OpCode32;
}Ebike_TxBleData;
