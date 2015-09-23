/***************************************************************************//**
 * File: sysclk.c
 *
 * Description: System clock manager
 *
 *
 * Created on Feb 08, 2014
 *
 * 
 *****************************************************************************/

#include "main.h"

#include "standards.h"

#define _DISABLE_SYSTEM_TIMER_

#ifndef _DISABLE_SYSTEM_TIMER_
static app_timer_id_t               m_sys_clk_timer_id;                        /**< system clock timer. */
#endif

#ifdef _CLING_PC_SIMULATION_
I32U sim_tick_acc = 0;
void SYSCLK_update_tick(void)
{
	sim_tick_acc += 20;
	if (sim_tick_acc > 1000)
		sim_tick_acc -= 1000;
}

#endif

/* Uses the 16-bit TimerA0 counter and the lower 16 bits of the system
 * clock to give a "fine-grained" time. */
I32U SYSCLK_GetFineTime(void)
{
	I32U tick_diff=0;
#ifndef _CLING_PC_SIMULATION_
	I32U tick_now;
	double system_clock_in_ms;

	// Get the elapsed time
	app_timer_cnt_get(&tick_now);
	app_timer_cnt_diff_compute(tick_now, cling.time.tick_count, &tick_diff);
	tick_diff += cling.time.rtc_tick_residual;
	system_clock_in_ms = cling.time.system_clock_in_sec*1000;
	system_clock_in_ms += (tick_diff/32.768);

	N_SPRINTF("[SYSCLK]: sec: %d, n: %d, o:%d", cling.time.system_clock_in_sec, tick_now, cling.time.tick_count);
#else
	tick_diff = sim_tick_acc;
#endif
	// 32758 ticks per second, about 32 tick per ms.
	return (I32U)system_clock_in_ms;
  //return (cling.time.system_clock_in_sec*1000+tick_diff);
}

void SYSCLK_timer_handler(void * p_context)
{
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
void SYSCLK_Init(void)
{
#ifndef _CLING_PC_SIMULATION_
	
	// Initialize timer module.
	APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, false);
#endif
}

EN_STATUSCODE SYSCLK_create_timer(void)
{
#ifndef _DISABLE_SYSTEM_TIMER_
	uint32_t err_code;
	// Create timers.
	err_code = app_timer_create(&m_sys_clk_timer_id,
															APP_TIMER_MODE_REPEATED,
															SYSCLK_timer_handler);
	APP_ERROR_CHECK(err_code);
#endif
	return STATUSCODE_SUCCESS;
}

void SYSCLK_timer_stop()
{
#ifndef _DISABLE_SYSTEM_TIMER_

	app_timer_stop(m_sys_clk_timer_id);
	
	N_SPRINTF("[SYSCLK] stop timer ...");
#endif
}

/**@brief Function for starting application timers.
 */
void SYSCLK_timer_start(void)
{
#ifndef _CLING_PC_SIMULATION_
	// Start application timers
	N_SPRINTF("[SYSCLK] RTC start 20ms, %d ", CLK_get_system_time());
	RTC_config(SYSCLK_INTERVAL_20MS);
#endif
}


/** @} */
