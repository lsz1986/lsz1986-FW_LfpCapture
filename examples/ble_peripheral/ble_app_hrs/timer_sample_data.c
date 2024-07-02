/*
 *	PIN4:		INPUT1:AIN0
 *	PIN7:		INPUT2:AIN3
 *	PIN40:		Battery ADC: AIN4
*/

#include "timer_sample_data.h"

#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"

#include "nrf_log.h"

	
	
#define RAW_ADC_BUFF_SIZE				6

typedef enum{
	SAMPLE_CHANNEL_HEART_RATE,
	SAMPLE_CHANNEL_BATTERY_CAPACITY,
	SAMPLE_CHANNEL_SIZE
}SAMPLE_CHANNEL_IDX;
#define SAMPLE_SUM_PER_FRAME (RAW_ADC_BUFF_SIZE/SAMPLE_CHANNEL_SIZE)


static nrf_saadc_value_t m_buffer_pool[2][RAW_ADC_BUFF_SIZE];

//static uint32_t          m_transducer_evt_counter;

static const nrf_drv_timer_t TIMER_HEART_RATE = NRF_DRV_TIMER_INSTANCE(3);
static const nrf_drv_timer_t TIMER_BATTERY = NRF_DRV_TIMER_INSTANCE(4);

static nrf_ppi_channel_t HEART_RATE_SAADC_TIMER_PPI_CHANNEL;
static nrf_ppi_channel_t BATTERY_SAADC_TIMER_PPI_CHANNEL;

static uint32_t heart_rate_timer_compare_event_addr, battery_timer_compare_event_addr;


uint16_t g_vBatteryCapacity = 0, g_vHeartRate = 0;


static void saadc_init(void);
static void saadc_sampling_event_init(void);
static void saadc_sampling_event_enable(void);
void task_sample_enable(void)
{
	saadc_init();
    saadc_sampling_event_init();
    saadc_sampling_event_enable();
}


static void saadc_transducer_callback(nrf_drv_saadc_evt_t const * p_event)
{
	if(NRF_DRV_SAADC_EVT_DONE != p_event->type)
	{
		return;
	}
	
	APP_ERROR_CHECK(nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, RAW_ADC_BUFF_SIZE));
  
//        NRF_LOG_INFO("[%d %d] channel %d  event: %d", battery_timer_compare_event_addr, heart_rate_timer_compare_event_addr,
//		p_event->data.limit.channel, (int)m_transducer_evt_counter);
	
	uint16_t vHeartRate = 0, vBatteryCapacity = 0;
	uint8_t idx = 0;
	for (idx = 0; idx < RAW_ADC_BUFF_SIZE; idx++)
	{
		switch(idx%SAMPLE_CHANNEL_SIZE){
		case SAMPLE_CHANNEL_HEART_RATE:
			vHeartRate += p_event->data.done.p_buffer[idx];
			break;
		case SAMPLE_CHANNEL_BATTERY_CAPACITY:
			vBatteryCapacity += p_event->data.done.p_buffer[idx];
			break;
		}
	}
	
	g_vHeartRate = vHeartRate / (double)SAMPLE_SUM_PER_FRAME;
	g_vBatteryCapacity = vBatteryCapacity/(double)SAMPLE_SUM_PER_FRAME;
	
	NRF_LOG_INFO("Heart Rate[%d] Battery Capacity[%d]", g_vHeartRate, g_vBatteryCapacity);
}

static  void saadc_init(void)
{
    nrf_saadc_channel_config_t heart_rate_channel_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN1);
    nrf_saadc_channel_config_t battery_channel_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN2);

    APP_ERROR_CHECK(nrf_drv_saadc_init(NULL, saadc_transducer_callback));

    APP_ERROR_CHECK(nrf_drv_saadc_channel_init(SAMPLE_CHANNEL_HEART_RATE, &heart_rate_channel_config));
    APP_ERROR_CHECK(nrf_drv_saadc_channel_init(SAMPLE_CHANNEL_BATTERY_CAPACITY, &battery_channel_config));

    APP_ERROR_CHECK(nrf_drv_saadc_buffer_convert(m_buffer_pool[0], RAW_ADC_BUFF_SIZE));
	APP_ERROR_CHECK(nrf_drv_saadc_buffer_convert(m_buffer_pool[1], RAW_ADC_BUFF_SIZE));
}

static  void saadc_sampling_event_init(void)
{
    uint32_t saadc_sample_task_addr;

    APP_ERROR_CHECK(nrf_drv_ppi_init());

    nrf_drv_timer_enable(&TIMER_HEART_RATE);
    nrf_drv_timer_enable(&TIMER_BATTERY);

    heart_rate_timer_compare_event_addr = nrf_drv_timer_compare_event_address_get(&TIMER_HEART_RATE, NRF_TIMER_CC_CHANNEL0);
    battery_timer_compare_event_addr = nrf_drv_timer_compare_event_address_get(&TIMER_BATTERY, NRF_TIMER_CC_CHANNEL0);
    
    saadc_sample_task_addr = nrf_drv_saadc_sample_task_get();
	
    /* Setup ppi channel so that timer compare event is triggering sample tasks in SAADC */
    APP_ERROR_CHECK(nrf_drv_ppi_channel_alloc(&HEART_RATE_SAADC_TIMER_PPI_CHANNEL));
    APP_ERROR_CHECK(nrf_drv_ppi_channel_alloc(&BATTERY_SAADC_TIMER_PPI_CHANNEL));

    APP_ERROR_CHECK(nrf_drv_ppi_channel_assign(HEART_RATE_SAADC_TIMER_PPI_CHANNEL, heart_rate_timer_compare_event_addr, saadc_sample_task_addr));
    APP_ERROR_CHECK(nrf_drv_ppi_channel_assign(BATTERY_SAADC_TIMER_PPI_CHANNEL, battery_timer_compare_event_addr, saadc_sample_task_addr));
}


static void saadc_sampling_event_enable(void)
{
    APP_ERROR_CHECK(nrf_drv_ppi_channel_enable(HEART_RATE_SAADC_TIMER_PPI_CHANNEL));
    APP_ERROR_CHECK(nrf_drv_ppi_channel_enable(BATTERY_SAADC_TIMER_PPI_CHANNEL));
}



