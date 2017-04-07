/******************************************************************************
* Copyright 2013-2014 hicling Systems (MikeWang)
*
* FileName: rtc_hal.c
*
* Description: abstract layer between cling sdk and freertos.
*
* Modification history:
*     2016/7/22, v1.0 create this file mike.
*******************************************************************************/


#include "oop_hal.h"
#include "main.h"
#include "stdio.h"
#include "string.h"
#include "exti_hal.h"
#include "nrf_drv_gpiote.h"
/*********************************************************************
* MACROS
*/
#ifdef __cplusplus
extern "C" {
#endif

#define EVENTS_INT_TRIGGER_THREAHOLD  1
#define DEBOUNCE_TIME                 0//(ms)


/*********************************************************************
* TYPEDEFS
*/
struct exti_pin_mgr {
		uint8_t pin;
		bool    enable;
		uint8_t pull;
		uint8_t trigger_level;
		uint8_t hi_accuracy;
};



/*********************************************************************
* GLOBAL VARIABLES
*/

/*********************************************************************
* LOCAL VARIABLES
*/
static CLASS(HalExti)* p_instance = NULL;
static uint8_t  ints_suspended = false;
struct device_mode_event dev_mode = {
		.mode = DEVICE_NORMAL_MODE
};
#define MODULE_ACTIVE      1
#define MODULE_DEACTIVE    0
static  uint8_t current_state = MODULE_DEACTIVE;
/*********************************************************************
* LOCAL DECLARATION
*/
static const struct exti_pin_mgr wu_pins[]= {
		{.pin = GPIO_SENSOR_INT_1, .enable = true, .pull = GPIO_PIN_CNF_PULL_Disabled, .trigger_level = NRF_GPIOTE_POLARITY_LOTOHI, .hi_accuracy = false},/*cpresponded interrupt is disabled default which means  you gonna need call enbale function independly*/
		{.pin = GPIO_TOUCH_INT, .enable = true, .pull = GPIO_PIN_CNF_PULL_Pullup, .trigger_level = NRF_GPIOTE_POLARITY_HITOLO, .hi_accuracy = false},
		{.pin = GPIO_CHARGER_INT, .enable = true, .pull = GPIO_PIN_CNF_PULL_Pullup, .trigger_level = NRF_GPIOTE_POLARITY_HITOLO, .hi_accuracy = false},
		{.pin = GPIO_FM1280B_IRQ, .enable = true, .pull = GPIO_PIN_CNF_PULL_Disabled, .trigger_level = NRF_GPIOTE_POLARITY_LOTOHI, .hi_accuracy = false},
		{.pin = GPIO_RTC_INT, .enable = true, .pull = GPIO_PIN_CNF_PULL_Pullup, .trigger_level = NRF_GPIOTE_POLARITY_HITOLO, .hi_accuracy = false},
};

static const struct exti_pin_mgr suspend_pins[] = {
		//	{.pin = GPIO_TOUCH_INT},/*cpresponded interrupt is disabled default which means  you gonna need call enbale function independly*/
		//	{.pin = GPIO_RTC_INT},/*cpresponded interrupt is disabled default which means  you gonna need call enbale function independly*/
};

static void (*pin_int_callback)(uint8_t pin) = NULL;
CLASS(HalExti)* HalExti_get_instance(void);
static int reg_isr(CLASS(HalExti) *arg, void (*cb)(uint8_t));
static int HalExti_init(CLASS(HalExti) *arg);
static int disable_all(CLASS(HalExti) *arg);
static int enable_all(CLASS(HalExti) *arg);

static void hw_exti_init(void);
static int resume_ints(CLASS(HalExti) *arg);
static int suspend_ints(CLASS(HalExti) *arg);
static int check_process(CLASS(HalExti) *arg);
static void ad_exti_power_init(void);
int set_deep_sleep(CLASS(HalExti) *arg);
int set_low_power(CLASS(HalExti) *arg);
int set_active(CLASS(HalExti) *arg);
/******************************************************************************
* FunctionName :HalExti_get_instance
* Description  : get hal rtc instance
* Parameters   : write: write call back function
* Returns          : 0: sucess
										 -1: error
*******************************************************************************/
CLASS(HalExti)* HalExti_get_instance(void)
{
		static CLASS(HalExti) p;
		if (p_instance == NULL) {
				p_instance = &p;
				memset(&p, 0, sizeof(p));
				HalExti_init(p_instance);
				ad_exti_power_init();
		}
		return p_instance;
}

/******************************************************************************
* FunctionName :HalExti_init
* Description  : init hal rtc instance
* Parameters   : object pointer
* Returns          : 0: sucess
										 -1: error
*******************************************************************************/
static int HalExti_init(CLASS(HalExti) *arg)
{
		arg->isr_reg = reg_isr;
		arg->enable_all = enable_all;
		arg->disable_all = disable_all;
		arg->suspend_ints = suspend_ints;
		arg->resume_ints = resume_ints;
		arg->check_routine = check_process;
		arg->set_active = set_active;
		arg->set_low_power = set_low_power;
		arg->set_deep_sleep = set_deep_sleep;
		nrf_drv_gpiote_init();
		hw_exti_init();
		return 0;
}


void nrf_drv_gpiote_callback(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
		if(pin_int_callback != NULL) {
				pin_int_callback(pin);
				for(int i= 0; i< sizeof(wu_pins)/sizeof(wu_pins[0]); i++) {
						if(wu_pins[i].pin == pin) {
								continue;
						}
						if(nrf_gpio_pin_read(wu_pins[i].pin) == (wu_pins[i].trigger_level == NRF_GPIOTE_POLARITY_LOTOHI)) {
								pin_int_callback(wu_pins[i].pin);
						}
				}
		}
}

static void hw_exti_init()
{
		nrf_drv_gpiote_in_config_t  p_config ;

		for(int i= 0; i< sizeof(wu_pins)/sizeof(wu_pins[0]); i++) {
				if(wu_pins[i].enable == true) {
						p_config.hi_accuracy = wu_pins[i].hi_accuracy;
						p_config.is_watcher = false;
						p_config.pull = (nrf_gpio_pin_pull_t)wu_pins[i].pull;
						p_config.sense = (nrf_gpiote_polarity_t)wu_pins[i].trigger_level;
						nrf_drv_gpiote_in_init(wu_pins[i].pin, &p_config, nrf_drv_gpiote_callback);
				} else {
						nrf_gpio_cfg_input(wu_pins[i].pin, (nrf_gpio_pin_pull_t)wu_pins[i].pull);
				}
		}


}

static int check_process(CLASS(HalExti) *arg)
{
		if(pin_int_callback != NULL) {
				for(int i= 0; i< sizeof(wu_pins)/sizeof(wu_pins[0]); i++) {
						if(nrf_gpio_pin_read(wu_pins[i].pin) == (wu_pins[i].trigger_level == NRF_GPIOTE_POLARITY_LOTOHI)) {
								pin_int_callback(wu_pins[i].pin);
						}
				}
		}
		return 1;
}

int set_active(CLASS(HalExti) *arg)
{
			dev_mode.mode = DEVICE_NORMAL_MODE;
			return 0;
}
int set_low_power(CLASS(HalExti) *arg)
{
			dev_mode.mode = DEVICE_LOW_POWER_MODE;
			return 0;
}
int set_deep_sleep(CLASS(HalExti) *arg)
{
			dev_mode.mode = DEVICE_DEEP_SLEEP_MODE;
			return 0;
}

/******************************************************************************
* FunctionName : reg_isr
* Description  : register rtc callback
* Parameters   : write: write call back function
* Returns          : 0: sucess
										 -1: error
*******************************************************************************/

static int reg_isr(CLASS(HalExti) *arg, void (*cb)(uint8_t))
{
		if(cb != NULL) {
				pin_int_callback = cb;
		}
		return 1;
}

static int suspend_ints(CLASS(HalExti) *arg)
{

		ints_suspended = true;
		for(int j = 0; j < sizeof(suspend_pins)/sizeof(suspend_pins[0]); j++) {
				for(int i= 0; i< sizeof(wu_pins)/sizeof(wu_pins[0]); i++) {
						if(wu_pins[i].pin == suspend_pins[j].pin) {
								if(wu_pins[i].enable == true) {
										nrf_drv_gpiote_in_event_disable(wu_pins[i].pin);
										nrf_drv_gpiote_in_uninit(wu_pins[i].pin);
										nrf_gpio_cfg_input(wu_pins[i].pin, (nrf_gpio_pin_pull_t)wu_pins[i].pull);
								}
						}
				}
		}
		return 0;
}

static int resume_ints(CLASS(HalExti) *arg)
{

		nrf_drv_gpiote_in_config_t  p_config ;
		for(int j = 0; j < sizeof(suspend_pins)/sizeof(suspend_pins[0]); j++) {
				for(int i= 0; i< sizeof(wu_pins)/sizeof(wu_pins[0]); i++) {
						if(wu_pins[i].pin == suspend_pins[j].pin) {
								if(wu_pins[i].enable == true) {
										p_config.hi_accuracy = true;
										p_config.is_watcher = false;
										p_config.pull = (nrf_gpio_pin_pull_t)wu_pins[i].pull;
										p_config.sense = (nrf_gpiote_polarity_t)wu_pins[i].trigger_level;
										nrf_drv_gpiote_in_init(wu_pins[i].pin, &p_config, nrf_drv_gpiote_callback);
										nrf_drv_gpiote_in_event_enable(wu_pins[i].pin, true);
								} else {
										nrf_gpio_cfg_input(wu_pins[i].pin, (nrf_gpio_pin_pull_t)wu_pins[i].pull);
								}
						}
				}
		}
		ints_suspended = false;
		return 0;
}


static int enable_all(CLASS(HalExti) *arg)
{
		for(int i= 0; i< sizeof(wu_pins)/sizeof(wu_pins[0]); i++) {
				if(wu_pins[i].enable == true) {
						nrf_drv_gpiote_in_event_enable(wu_pins[i].pin, true);
				}
		}

		return 0;
}



static int disable_all(CLASS(HalExti) *arg)
{
		for(int i= 0; i< sizeof(wu_pins)/sizeof(wu_pins[0]); i++) {
				nrf_drv_gpiote_in_event_disable(wu_pins[i].pin);
		}
		return 0;
}


static bool ad_exti_prepare_for_sleep(void)
{
		if(current_state == MODULE_DEACTIVE) {
				return true;
		}
		N_SPRINTF("[EXTI] ad_exti_prepare_for_sleep");
		if(dev_mode.mode == DEVICE_NORMAL_MODE) {
				for(int i= 0; i< sizeof(wu_pins)/sizeof(wu_pins[0]); i++) {
						if(wu_pins[i].enable == true && wu_pins[i].hi_accuracy == true) {
								nrf_drv_gpiote_in_event_disable(wu_pins[i].pin);
								nrf_drv_gpiote_in_uninit(wu_pins[i].pin);
								nrf_gpio_cfg_input(wu_pins[i].pin, (nrf_gpio_pin_pull_t)wu_pins[i].pull);
						}
				}
		} else if(dev_mode.mode == DEVICE_LOW_POWER_MODE) {
				for(int i= 0; i< sizeof(wu_pins)/sizeof(wu_pins[0]); i++) {
						/*in low power mode we need to keep chager int open to track charging state*/
						if(wu_pins[i].pin == GPIO_CHARGER_INT) {
								continue;
						}
						if(wu_pins[i].enable == true) {
								nrf_drv_gpiote_in_event_disable(wu_pins[i].pin);
								nrf_drv_gpiote_in_uninit(wu_pins[i].pin);
								nrf_gpio_cfg_default(wu_pins[i].pin);
						}else {
								//nrf_gpio_cfg_input(wu_pins[i].pin, (nrf_gpio_pin_pull_t)wu_pins[i].pull);
								nrf_gpio_cfg_default(wu_pins[i].pin);
						}
				}
		} else if(dev_mode.mode == DEVICE_DEEP_SLEEP_MODE) {
				for(int i= 0; i< sizeof(wu_pins)/sizeof(wu_pins[0]); i++) {
						nrf_drv_gpiote_in_event_disable(wu_pins[i].pin);
						nrf_drv_gpiote_in_uninit(wu_pins[i].pin);
						nrf_gpio_cfg_default(wu_pins[i].pin);
				}

		}
		current_state = MODULE_DEACTIVE;
		return true;
}



static void ad_exti_wake_up_ind(bool arg)
{

		if(current_state == MODULE_ACTIVE) {
				return;
		}
		N_SPRINTF("[EXTI] ad_exti_wake_up_ind");
		if(dev_mode.mode == DEVICE_NORMAL_MODE) {
				nrf_drv_gpiote_in_config_t  p_config ;
				for(int i= 0; i< sizeof(wu_pins)/sizeof(wu_pins[0]); i++) {
						if(wu_pins[i].enable == true && wu_pins[i].hi_accuracy == true) {
								/*check if other resouce are using gpiote event*/
								if(ints_suspended == true) {
										int j = 0;
										for(; j < sizeof(suspend_pins)/sizeof(suspend_pins[0]); j++) {
												if(wu_pins[i].pin == suspend_pins[j].pin) {
														break;
												}
										}
										/*if this channel get occupied then skip it*/
										if(j < sizeof(suspend_pins)/sizeof(suspend_pins[0])) {
												nrf_gpio_cfg_input(wu_pins[i].pin, (nrf_gpio_pin_pull_t)wu_pins[i].pull);
												continue;
										}
								}
								p_config.hi_accuracy = true;
								p_config.is_watcher = false;
								p_config.pull = (nrf_gpio_pin_pull_t)wu_pins[i].pull;
								p_config.sense = (nrf_gpiote_polarity_t)wu_pins[i].trigger_level;
								nrf_drv_gpiote_in_init(wu_pins[i].pin, &p_config, nrf_drv_gpiote_callback);
								nrf_drv_gpiote_in_event_enable(wu_pins[i].pin, true);
						}  else if(wu_pins[i].enable == true && wu_pins[i].hi_accuracy == false) {
								nrf_drv_gpiote_in_event_enable(wu_pins[i].pin, true);
						} else {
								nrf_gpio_cfg_input(wu_pins[i].pin, (nrf_gpio_pin_pull_t)wu_pins[i].pull);
						}
				}
		} else if(dev_mode.mode == DEVICE_LOW_POWER_MODE) {
				/*we do'not need to enable any int line besides charge line*/
		} else if(dev_mode.mode == DEVICE_DEEP_SLEEP_MODE) {

		}
		current_state = MODULE_ACTIVE;
}

static void ad_exti_sleep_canceled(void)
{
		ad_exti_wake_up_ind(true);
}

static const adapter_call_backs_t ad_exti_pm_call_backs = {
		.ad_prepare_for_sleep = ad_exti_prepare_for_sleep,
		.ad_sleep_canceled = ad_exti_sleep_canceled,
		.ad_wake_up_ind = ad_exti_wake_up_ind,
		.ad_sleep_preparation_time = 0
};

static void ad_exti_power_init(void)
{
		pm_register_adapter(&ad_exti_pm_call_backs);
}

#ifdef __cplusplus
}
#endif
