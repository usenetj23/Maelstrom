/**
 *
 * Triac Library
 * 
 * Keith Wakeham
 * 
 * 
 */


#include "nrf.h"
#include "app_timer.h"
#include "nrf_drv_gpiote.h"
#include "triac.h"
#include "nrf_log.h"


APP_TIMER_DEF(m_triac_timer_id); 
bool ZC_pulse = 1;
uint32_t offset = 150;
uint16_t p_avg = 0;

void timeout_handler(void * p_context)
{
    // nrf_drv_gpiote_out_set(BSP_LED_1);
    nrf_drv_gpiote_out_toggle(PIN_OUT);
    ret_code_t err_code;
    if (ZC_pulse == 1) {
        nrf_drv_gpiote_out_set(PIN_OUT);
        err_code = app_timer_start(m_triac_timer_id, 32, NULL);
        APP_ERROR_CHECK(err_code);
        ZC_pulse = 0;
    } else {
        nrf_drv_gpiote_out_clear(PIN_OUT);
    }
}

void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    ret_code_t err_code;
    // nrf_drv_gpiote_out_toggle(PIN_OUT);
    if (offset < 500)
    {
        err_code = app_timer_start(m_triac_timer_id, offset, NULL);
        APP_ERROR_CHECK(err_code);
        ZC_pulse = 1;
    }
    // NRF_LOG_INFO("Input");
    
}

/**
 * @brief Function for configuring: PIN_IN pin for input, PIN_OUT pin for output,
 * and configures GPIOTE to give an interrupt on pin change.
 */
void gpio_init(void)
{
    ret_code_t err_code;
    if(!nrf_drv_gpiote_is_init())
    {
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);
    }
    NRF_LOG_INFO("Init'd GPIOTE");
    // err_code = nrfx_gpiote_init();
    // APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);

    err_code = nrf_drv_gpiote_out_init(PIN_OUT, &out_config);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("Phase output setup");

    // err_code = nrf_drv_gpiote_out_init(BSP_LED_1, &out_config);
    // APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(PIN_ZC, &in_config, in_pin_handler);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("ZC input  setup");

    nrf_drv_gpiote_in_event_enable(PIN_ZC, true);

    // APP_TIMER_DEF(m_triac_timer_id); 
    err_code = app_timer_create(&m_triac_timer_id, APP_TIMER_MODE_SINGLE_SHOT, timeout_handler);
    NRF_LOG_INFO("Timer Create");
    APP_ERROR_CHECK(err_code);
}

void set_power(uint16_t bike_power){
    // if (bike_power > 200)
    // {
    //     offset = 60;
    // } else if (bike_power <= 200 && bike_power > 180)
    // {
    //     offset = 80;
    // } else if (bike_power <= 180  && bike_power > 160)
    // {
    //     offset = 100;
    // } else if (bike_power <= 160  && bike_power > 140)
    // {
    //     offset = 120;
    // } else if (bike_power < 140)
    // {
    //     offset = 140;
    // } 
    // NRF_LOG_INFO("HelloKeith");

    int16_t p_diff;
    p_diff = bike_power - p_avg;
    p_diff = p_diff/4;
    p_avg = p_avg + p_diff;

    if (p_avg > 200)
    {
        offset = 60;
    } else if (p_avg <= 200 && p_avg > 40) //(p_avg <= 200 && p_avg > 100) //This is for -0.8 and 220
    {
        offset = (p_avg*-0.5)+160;
    } else if (p_avg <= 40 && p_avg > 20)
    {
        offset = 140;
    } else if (p_avg <= 20)
    {
        offset = 500;
    } 
    NRF_LOG_INFO("offset:                  %u c", offset);
    NRF_LOG_INFO("p_avg:                  %u W", p_avg);
}

//150 = 4.683ms, 4.697ms
//75 = 2.454ms
//1800 = 59
//4000 = 586952301