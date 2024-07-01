#include "task_sample_hrs.h"


#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"

#include "nrf_log.h"


#define SAMPLES_HRS_IN_BUFFER 			5

static nrf_drv_timer_t m_hrs_timer = NRF_DRV_TIMER_INSTANCE(3);
static nrf_saadc_value_t     m_buffer_hrs_pool[2][SAMPLES_HRS_IN_BUFFER];
static nrf_ppi_channel_t     m_ppi_hrs_channel;


static void timer_hrs_handler(nrf_timer_event_t event_type, void * p_context)
{

}

void saadc_hrs_sampling_event_init(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    err_code = nrf_drv_timer_init(&m_hrs_timer, &timer_cfg, timer_hrs_handler);
    APP_ERROR_CHECK(err_code);

    /* setup m_hrs_timer for compare event every 50ms */
    uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_hrs_timer, 50);
    nrf_drv_timer_extended_compare(&m_hrs_timer,
                                   NRF_TIMER_CC_CHANNEL0,
                                   ticks,
                                   NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
                                   false);
    nrf_drv_timer_enable(&m_hrs_timer);

    const uint32_t timer_compare_hrs_event_addr = nrf_drv_timer_compare_event_address_get(&m_hrs_timer,
                                                                                NRF_TIMER_CC_CHANNEL0);
    const uint32_t saadc_hrs_sample_task_addr   = nrf_drv_saadc_sample_task_get();

    /* setup ppi channel so that timer compare event is triggering sample task in SAADC */
    err_code = nrf_drv_ppi_channel_alloc(&m_ppi_hrs_channel);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_ppi_channel_assign(m_ppi_hrs_channel,
                                          timer_compare_hrs_event_addr,
                                          saadc_hrs_sample_task_addr);
    APP_ERROR_CHECK(err_code);
}


void saadc_hrs_sampling_event_enable(void)
{
    ret_code_t err_code = nrf_drv_ppi_channel_enable(m_ppi_hrs_channel);

    APP_ERROR_CHECK(err_code);
}


static void saadc_hrs_callback(nrf_drv_saadc_evt_t const * p_event)
{
	if(NRF_DRV_SAADC_EVT_DONE != p_event->type)
	{
		return;
	}
	
	ret_code_t err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_HRS_IN_BUFFER);
	APP_ERROR_CHECK(err_code);
	
	int i = 0;
	for (i = 0; i < SAMPLES_HRS_IN_BUFFER; i++)
	{
		NRF_LOG_INFO("hrs %d", p_event->data.done.p_buffer[i]);
		
//		val = p_event->data.done.p_buffer[i] * 3.6 /1024;	
//			NRF_LOG_INFO(" %d mV", val*1000);
	}
	
}


void saadc_hrs_init(void)
{
    ret_code_t err_code;
    nrf_saadc_channel_config_t channel_config =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN4);

    err_code = nrf_drv_saadc_init(NULL, saadc_hrs_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_hrs_pool[0], SAMPLES_HRS_IN_BUFFER);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_hrs_pool[1], SAMPLES_HRS_IN_BUFFER);
    APP_ERROR_CHECK(err_code);
}















