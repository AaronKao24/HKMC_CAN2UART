
#include <stdio.h>
#include <stdlib.h>
#include <init.h>
#include <zephyr.h>
#include <device.h>
#include <hal/nrf_saadc.h>
#include <drivers/adc.h>

#include "nRF52840_PIN_DEF.h"
#include "fw_api.h"


#define ADC_DEVICE_NAME		DT_LABEL(DT_INST(0, nordic_nrf_saadc))
#define ADC_1ST_CHANNEL_ID	0
#define ADC_2ND_CHANNEL_ID	1
#define ADC_3RD_CHANNEL_ID	2
#define ADC_4TH_CHANNEL_ID	3
#define ADC_5TH_CHANNEL_ID	4


#define SAMPLE_BUFFER_SIZE  8
static short m_sample_buffer[SAMPLE_BUFFER_SIZE];

static const struct adc_channel_cfg m_1st_channel_cfg = { // cc1
	.gain	     = ADC_GAIN_1_6, // ADC_GAIN_1_5, // ADC_GAIN_1_6 ,
	.reference	=  ADC_REF_INTERNAL,
	.acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 10),
	.channel_id       = ADC_1ST_CHANNEL_ID,
	.input_positive   = NRF_SAADC_INPUT_AIN0,
};

static const struct adc_channel_cfg m_2nd_channel_cfg = { // cc2
	.gain	     = ADC_GAIN_1_6 ,
	.reference	= ADC_REF_INTERNAL,
	.acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 10),
	.channel_id       = ADC_2ND_CHANNEL_ID,
	.input_positive   = NRF_SAADC_INPUT_AIN1,
};


static const struct adc_channel_cfg m_3rd_channel_cfg = { // thermal
	.gain	     = ADC_GAIN_1_6, // ADC_GAIN_1_5, // ADC_GAIN_1_6 ,
	.reference	=  ADC_REF_INTERNAL,
	.acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 10),
	.channel_id       = ADC_3RD_CHANNEL_ID,
	.input_positive   = NRF_SAADC_INPUT_AIN7,
};

static const struct adc_channel_cfg m_4th_channel_cfg = { // thermal
	.gain	     = ADC_GAIN_1_6, // ADC_GAIN_1_5, // ADC_GAIN_1_6 ,
	.reference	=  ADC_REF_INTERNAL,
	.acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 10),
	.channel_id       = ADC_4TH_CHANNEL_ID,
	.input_positive   = NRF_SAADC_INPUT_AIN4,
};

static const struct adc_channel_cfg m_5th_channel_cfg = { // thermal
	.gain	     = ADC_GAIN_1_6, // ADC_GAIN_1_5, // ADC_GAIN_1_6 ,
	.reference	=  ADC_REF_INTERNAL,
	.acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 10),
	.channel_id       = ADC_5TH_CHANNEL_ID,
	.input_positive   = NRF_SAADC_INPUT_AIN6,
};

/*
static const struct adc_channel_cfg m_4rd_channel_cfg = { // thermal
	.gain	     = ADC_GAIN_1_6, // ADC_GAIN_1_5, // ADC_GAIN_1_6 ,
	.reference	=  ADC_REF_INTERNAL,
	.acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 10),
	.channel_id       = ADC_3RD_CHANNEL_ID,
	.input_positive   = NRF_SAADC_INPUT_AIN7,
};
*/
const struct adc_sequence sequence = {
	.channels    = BIT(ADC_1ST_CHANNEL_ID) | BIT(ADC_2ND_CHANNEL_ID) | BIT(ADC_3RD_CHANNEL_ID)| BIT(ADC_4TH_CHANNEL_ID)| BIT(ADC_5TH_CHANNEL_ID),
	.buffer      = m_sample_buffer,
	.buffer_size = sizeof(m_sample_buffer),
	.resolution  = 14,
};
static const struct device *adc_dev;

#define THERMAL_BUFFER_SIZE   8
uint16_t ary_thermal[THERMAL_BUFFER_SIZE] ;
#define REMOTE1_BUFFER_SIZE   8
uint16_t ary_remote1[REMOTE1_BUFFER_SIZE] ;
#define REMOTE2_BUFFER_SIZE   8
uint16_t ary_remote2[REMOTE2_BUFFER_SIZE] ;
int thermal_read_idx ,remote1_read_idx, remote2_read_idx;
int32_t m_cc1 , m_cc2 ;
/**
* @fn fw_init_adc
* @brief initialize ADC with nRF52840 IC.
*/
int fw_init_adc(void)
{
	int ret, i ;
	if (adc_dev != NULL) {
		return -2 ;
	}
	adc_dev = device_get_binding(ADC_DEVICE_NAME);
	
	if (adc_dev == NULL) {
		return -1 ;
	}
	ret = adc_channel_setup(adc_dev, &m_1st_channel_cfg);	
	ret |= adc_channel_setup(adc_dev, &m_2nd_channel_cfg);
	ret |= adc_channel_setup(adc_dev, &m_3rd_channel_cfg);
	ret |= adc_channel_setup(adc_dev, &m_4th_channel_cfg);
	ret |= adc_channel_setup(adc_dev, &m_5th_channel_cfg);
	for (i = 0; i < THERMAL_BUFFER_SIZE; i++) {
		ary_thermal[i] = 0 ;
	}
	for (i = 0; i < REMOTE1_BUFFER_SIZE; i++) {
		ary_remote1[i] = 0 ;
	}
	for (i = 0; i < REMOTE2_BUFFER_SIZE; i++) {
		ary_remote2[i] = 0 ;
	}
	thermal_read_idx = 0 ;
	remote1_read_idx = 0 ;
	remote2_read_idx = 0;
	m_cc1 = 0 ;
	m_cc2 = 0 ;


	return ret ;
}

int flash_adc(void)
{
	int i , read = 0 ;
	
	if (adc_dev == NULL)
		goto EXECPTION_FETCH_ERROR ;
	for( i = 0 ; i < 12 ; i++ ) {
		if ((read = adc_read(adc_dev, &sequence)) != 0)
			goto EXECPTION_FETCH_ERROR ;
		m_cc1 = m_sample_buffer[0];
		m_cc2 = m_sample_buffer[1];
		ary_thermal[thermal_read_idx] = m_sample_buffer[2];
		ary_remote1[remote1_read_idx] = m_sample_buffer[3];
		ary_remote2[remote2_read_idx] = m_sample_buffer[4];
	}
	thermal_read_idx = ( thermal_read_idx + 1 ) % THERMAL_BUFFER_SIZE ;
	remote1_read_idx = ( remote1_read_idx + 1 ) % REMOTE1_BUFFER_SIZE ;
	remote2_read_idx = ( remote2_read_idx + 1 ) % REMOTE2_BUFFER_SIZE ;

	return 0 ;
EXECPTION_FETCH_ERROR :
	printf("Sensor value fetch error %d\n", read);
	return -1 ;
}

/**
* @fn get_thermal_adc
* @brief ���o�q���O�W���ū׷P��
*/
int get_thermal_adc()
{
	int i ;
	int32_t n_tmp = 0 ;
	for( i = 0 ; i < THERMAL_BUFFER_SIZE ; i++ ) {
		n_tmp += ary_thermal[i] ;
	}
	return ((n_tmp + 4) >> 3) ;
}

/**
* @fn get_remote1_adc
* @brief ���o�q���O�W���ū׷P��
*/
int get_remote1_adc()
{
	int i ;
	int32_t n_tmp = 0 ;
	for( i = 0 ; i < REMOTE1_BUFFER_SIZE ; i++ ) {
		n_tmp += ary_remote1[i] ;
	}
	return ((n_tmp + 4) >> 3) ;
}

/**
* @fn get_remote2_adc
* @brief ���o�q���O�W���ū׷P��
*/
int get_remote2_adc()
{
	int i ;
	int32_t n_tmp = 0 ;
	for( i = 0 ; i < REMOTE2_BUFFER_SIZE ; i++ ) {
		n_tmp += ary_remote2[i] ;
	}
	return ((n_tmp + 4) >> 3) ;
}

/**
* @fn get_usb_cc_adc
* @brief Get the voltage of the CC pin on the USB
* CC1 / CC2 (Configuration Channel) :
* After the Sink terminal is powered on, it will detect whether the voltage of CC1 and CC2 is greater than its local ground voltage. 
* If it is at a higher voltage, it means that it is pulled up by the Rp in the Source terminal, so the direction of the plug can be judged.
*/
int get_usb_cc_adc()
{
	if( m_cc1 > m_cc2 )	return m_cc1 ;
	return m_cc2 ;
}
int get_usb_cc1_adc()
{
	return m_cc1 ;
}
int get_usb_cc2_adc()
{
	return m_cc2 ;
}
