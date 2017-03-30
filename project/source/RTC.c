/***************************************************************************//**
 * @file RTC_MSP430.c
 * @brief Driver implementation for the MSP430 real-time clock.
 *
 *
 ******************************************************************************/


#include "standards.h"
#include "main.h"


#define APP_TIMER_MAX_TIMERS                 4                                          /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE              4                                          /**< Size of timer operation queues. */

#ifndef _CLING_PC_SIMULATION_
APP_TIMER_DEF(m_rtc_timer_id); /**< 1 sec based timer. >**/
APP_TIMER_DEF(m_operation_timer_id); /**< 1 sec based timer. >**/
#endif

const I8U DAYS_IN_EACH_MONTH[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

I8U const month_leap_in_days[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
I8U const month_normal_in_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_PACE_MODEL_)		

#define EPOCH_BASE 946684800 // Epoch time at 2000/1/1 0:0:0 (time zone: 0)

// RTC IC I2C address: 0x51 (in 7-bit mode)
#define RTC_I2C_ADDR 0xA2 


static EN_STATUSCODE RTC_read_reg(I8U cmdID, I8U bytes, I8U *pRegVal)
{
#ifndef _CLING_PC_SIMULATION_

	I32U err_code;
	const nrf_drv_twi_t twi = NRF_DRV_TWI_INSTANCE(1);
	
	if (pRegVal==NULL) return STATUSCODE_NULL_POINTER;
	
	if (!cling.system.b_twi_1_ON) {
		GPIO_twi_enable(1);
	}
	
	err_code = nrf_drv_twi_tx(&twi, (RTC_I2C_ADDR>>1), &cmdID, 1, true);
	if (err_code == NRF_SUCCESS) {
		N_SPRINTF("RTC: Read TX PASS: ");
	} else {
		N_SPRINTF("RTC: Read TX FAIL - %d", err_code);
		APP_ERROR_CHECK(err_code);
		return STATUSCODE_FAILURE;
	}
	err_code = nrf_drv_twi_rx(&twi, (RTC_I2C_ADDR>>1), pRegVal, bytes, false);
	if (err_code == NRF_SUCCESS) {
		N_SPRINTF("RTC: Read RX PASS: ");
		return STATUSCODE_SUCCESS;
	} else {
		N_SPRINTF("RTC: Read RX FAIL: %d", err_code);
		APP_ERROR_CHECK(err_code);
		return STATUSCODE_FAILURE;
	}
#else
	return STATUSCODE_SUCCESS;
#endif
}

static BOOLEAN RTC_write_reg(I8U* reg_val, I8U number_of_bytes)
{
#ifndef _CLING_PC_SIMULATION_

	I32U err_code;
	const nrf_drv_twi_t twi = NRF_DRV_TWI_INSTANCE(1);
	
	if (!cling.system.b_twi_1_ON) {
		GPIO_twi_enable(1);
	}
	
	err_code = nrf_drv_twi_tx(&twi, (RTC_I2C_ADDR>>1), reg_val, number_of_bytes, false);
	if (err_code == NRF_SUCCESS) {
		//Y_SPRINTF("BATT: Write PASS: 0x%02x  0x%02x", cmdID, regVal);
		return STATUSCODE_SUCCESS;
	} else {
		//Y_SPRINTF("BATT: Write FAIL(%d): 0x%02x  0x%02x", err_code, cmdID, regVal);
		APP_ERROR_CHECK(err_code);
		return STATUSCODE_FAILURE;
	}
#else
	return STATUSCODE_SUCCESS;
#endif
}

void RTC_hw_config()
{
	I8U data[10];

	// capacitor selection: 12.5 pf	
	data[0] = 0x00;
	data[1] = 0x59;	
	RTC_write_reg(data, 2);	
	
	// Minute interrupt: enabled
	// CLKOUT frequency selection: 32768Hz	
	data[0] = 0x01;
	data[1] = 0x20;
	RTC_write_reg(data, 2);
}
 
uint8_t store_one_byte_to_rtc_ram(uint8_t content)
{
	I8U data[10];

	// capacitor selection: 12.5 pf	
	data[0] = 0x03;
	data[1] = content;	
	return RTC_write_reg(data, 2);	

}

uint8_t load_one_byte_from_rtc_ram(uint8_t *content)
{
	return RTC_read_reg(0x03, 1, content);
}

void RTC_calibrate_current_epoch()
{
	I32U epoch, t_diff;
	REALTIME_CTX rt;
	
	epoch = RTC_get_current_epoch(&rt);
	
	// Make sure we have RTC running correctly (no power-on reset)
	if (epoch > cling.time.time_since_1970) {
		cling.time.time_since_1970 = epoch;
	} else {
		
		t_diff = cling.time.time_since_1970 - epoch;
		
		// If Epoch from RTC chip is far different from software Epoch, then, go ahead to use software Epoch
		if (t_diff > 3600) 
			RTC_set_current_epoch(cling.time.time_since_1970);
	}
	
	RTC_get_local_clock(cling.time.time_since_1970, &cling.time.local);

	// Get current local minute
	cling.time.local_day = cling.time.local.day;
	cling.time.local_minute = cling.time.local.minute;
}

void RTC_minute_int_service()
{
	I8U b_pin = nrf_gpio_pin_read(GPIO_RTC_INT);	
	
	if (!b_pin) {
		Y_SPRINTF("RTC INT --");
		RTC_timer_handler(0);
		//cling.time.time_since_1970 = RTC_get_current_epoch();
		//RTC_get_local_clock(cling.time.time_since_1970, &cling.time.local);
	}
}

BOOLEAN RTC_get_current_rtc(REALTIME_CTX *p_rt)
{
	I8U time_ctx[8];
	I16U integer, fractional;
	I8U second, minute, hour, day, month, year;
	
	RTC_read_reg(0x04, 7, time_ctx);
	
	Y_SPRINTF("RTC READ(%d): %02x, %02x, %02x, %02x, %02x, %02x, %02x", cling.time.time_zone,
		time_ctx[6], time_ctx[5], time_ctx[4], time_ctx[3], time_ctx[2], time_ctx[1], time_ctx[0]);
	
	// Detect whether there is a power-on reset
	if ((time_ctx[6] == 0) && (time_ctx[5] == 1) && (time_ctx[3] == 1)) {
		return FALSE;
	}
	
	// Calculate Epoch
	
	// Second
	fractional = time_ctx[0] & 0x0f;
	integer = time_ctx[0] & 0x70;
	integer >>= 4;
	second = fractional + integer * 10;
	
	// Minute
	fractional = time_ctx[1] & 0x0f;
	integer = time_ctx[1] & 0x70;
	integer >>= 4;
	minute = fractional + integer * 10;
	
	// hour
	fractional = time_ctx[2] & 0x0f;
	integer = time_ctx[2] & 0x30;
	integer >>= 4;
	hour = fractional + integer * 10;
	
	// day
	fractional = time_ctx[3] & 0x0f;
	integer = time_ctx[3] & 0x30;
	integer >>= 4;
	day = fractional + integer * 10;
	
	// month
	fractional = time_ctx[5] & 0x0f;
	integer = time_ctx[5] & 0x10;
	integer >>= 4;
	month = fractional + integer * 10;
	
	// year
	fractional = time_ctx[6] & 0x0f;
	integer = time_ctx[6] & 0xf0;
	integer >>= 4;
	year = fractional + integer * 10;
	
	p_rt->second = second;
	p_rt->minute = minute;
	p_rt->hour = hour;
	p_rt->day = day;
	p_rt->month = month;
	p_rt->year = year;
	
	return TRUE;
}

I32U RTC_get_current_epoch(REALTIME_CTX *p_rt)
{
	I32U time_since_1970 = 0;
	I16U value;
	I8U i;
	I16S time_diff_in_minute = cling.time.time_zone;
	time_diff_in_minute *= TIMEZONE_DIFF_UNIT_IN_SECONDS;

	// Detect whether there is a power-on reset
	if (!RTC_get_current_rtc(p_rt)) {
		return time_since_1970;
	}
	
	// Accumulate epoch in year
	time_since_1970 = EPOCH_BASE; // Epoch time at 2000/1/1 0:0:0 (Time zone: 0)
	if (p_rt->year) {
		for (i = 0; i < p_rt->year; i++) {
			if (i & 0x03) {
				time_since_1970 += 31536000;
			} else {
				time_since_1970 += 31622400;
			}
		}
	}
	
	// Accumulate epoch in month
	value = 0;
	if (p_rt->year & 0x03) {
		for (i = 0; i < (p_rt->month-1); i++) {
			value += month_normal_in_days[i];
		}
	} else {
		for (i = 0; i < (p_rt->month-1); i++) {
			value += month_leap_in_days[i];
		}
	}
	time_since_1970 += value * 24 * 60 * 60;
	
	// Accumulate epoch in days, hour, minute, second
	time_since_1970 += (((p_rt->day-1)*24+p_rt->hour)*60+p_rt->minute)*60+p_rt->second;

	// Minute the time zone offset
	time_since_1970 -= time_diff_in_minute;
	
	N_SPRINTF("TIME EPOCH: %d(%d, %d, %d, %d, %d, %d)", time_since_1970, year, month, day, hour, minute, second);

	return time_since_1970;
}

void RTC_set_current_epoch(I32U time_since_1970)
{
	I8U time_reg[8];
	I8U integer, fractional, value;
	
	RTC_get_local_clock(time_since_1970, &cling.time.local);

	time_reg[0] = 0x04; // Address 0x04
	
	// Get second
	integer = cling.time.local.second/10;
	fractional = cling.time.local.second - integer * 10;
	time_reg[1] = fractional;
	integer <<= 4;
	time_reg[1] |= integer;
	
	// Get minute
	integer = cling.time.local.minute/10;
	fractional = cling.time.local.minute - integer * 10;
	time_reg[2] = fractional;
	integer <<= 4;
	time_reg[2] |= integer;
	
	// Get hour
	integer = cling.time.local.hour/10;
	fractional = cling.time.local.hour - integer * 10;
	time_reg[3] = fractional;
	integer <<= 4;
	time_reg[3] |= integer;
	
	// Get day
	integer = cling.time.local.day/10;
	fractional = cling.time.local.day - integer * 10;
	time_reg[4] = fractional;
	integer <<= 4;
	time_reg[4] |= integer;
	
	// Get week
	integer = cling.time.local.dow+1;
	if (integer == 7) {
		integer = 0;
	}	
	time_reg[5] = integer;
	
	// Get month
	integer = cling.time.local.month/10;
	fractional = cling.time.local.month - integer * 10;
	time_reg[6] = fractional;
	integer <<= 4;
	time_reg[6] |= integer;
	
	// Get year
	value = cling.time.local.year%100;
	integer = value / 10;
	fractional = value - integer * 10;
	time_reg[7] = fractional;
	integer <<= 4;
	time_reg[7] |= integer;
	
	RTC_write_reg(time_reg, 8);

	Y_SPRINTF("RTC WRITE: %02x, %02x, %02x, %02x, %02x, %02x, %02x", 
	time_reg[7], time_reg[6], time_reg[5], time_reg[4], time_reg[3], time_reg[2], time_reg[1]);

}
#endif

void OPERATION_timer_handler( void * p_context )
{
    N_SPRINTF("[RTC] OPERATION 50ms timer handler func");

    // Timer update
    cling.hr.sample_ready = TRUE;
}

void RTC_timer_handler( void * p_context )
{
	I32U tick_now;
	I32U tick_diff;
	I32U tick_in_s;

#ifndef _CLING_PC_SIMULATION_	
	if (cling.time.tick_count) {		
		app_timer_cnt_get(&tick_now);
		app_timer_cnt_diff_compute(tick_now, cling.time.tick_count, &tick_diff);
		tick_in_s = (tick_diff>>15);
		tick_diff = (tick_in_s << 15);
		cling.time.tick_count += tick_diff;
		cling.time.tick_count &= MAX_RTC_CNT;
	} else {	
		app_timer_cnt_get(&cling.time.tick_count);
		tick_in_s = (cling.time.tick_count>>15);
	}
#else
	tick_in_s = 1; // 1 second timer
#endif
	
	// update battery measuring timer
	cling.batt.level_update_timebase += tick_in_s;
	BATT_update_charging_time(tick_in_s);

	// update radio duty cycling
	cling.time.system_clock_in_sec += tick_in_s;
	cling.time.time_since_1970 += tick_in_s;
	
	// Set to 3 minutes duraton of screen lock
	if (cling.batt.state_switching_duration < 180) {
		cling.batt.state_switching_duration += tick_in_s;
	} else {
		cling.batt.b_toggle_lock = FALSE;
	}
	
	if (cling.batt.shut_down_time < BATTERY_SYSTEM_UNAUTH_POWER_DOWN) {
		cling.batt.shut_down_time += tick_in_s;
	}
#ifdef _ENABLE_TOUCH_
	// Accumulate skin touch time
	if (TOUCH_is_skin_touched()) {
		if (cling.touch.skin_touch_time_per_minute < 60) {
			cling.touch.skin_touch_time_per_minute += tick_in_s;
			N_SPRINTF("[RTC] skint touch time per min: %d", cling.touch.skin_touch_time_per_minute);
		}
	}	
#endif
#ifdef USING_VIRTUAL_ACTIVITY_SIM
	 if (OTA_if_enabled()) {
		 cling.ota.percent ++;
		 if (cling.ota.percent >= 100) {
			 cling.ota.percent = 100;
		 }
	 }
#endif

	// Indicates a second-based RTC interrupt
	RTC_get_local_clock(cling.time.time_since_1970, &cling.time.local);
	 
	// Check if we have minute passed by, or 
	if (cling.time.local.minute != cling.time.local_minute) {
		cling.time.local_minute_updated = TRUE;
		cling.time.local_minute = cling.time.local.minute;
		if (
			   (cling.user_data.idle_time_in_minutes>0) && 
				 (cling.time.local.hour>=cling.user_data.idle_time_start) && 
				 (cling.time.local.hour< cling.user_data.idle_time_end)
			 )
    {
			cling.user_data.idle_minutes_countdown --;
		}
		Y_SPRINTF("[RTC] min updated (%d)", cling.activity.day.walking);
		
		// Once a minute, refresh reminder
		REMINDER_set_sleep_reminder();

		// Once a minute, send a workout active message to App
		if (BTLE_is_connected()) {
			if (cling.activity.b_workout_active) {
				if ((cling.activity.workout_type == WORKOUT_RUN_OUTDOOR) || 
						(cling.activity.workout_type == WORKOUT_CYCLING_OUTDOOR))
				{
					CP_create_workout_rt_msg(cling.activity.workout_type);
				}
			}
		}
	}	
	
	if (cling.time.local.day != cling.time.local_day) {
		cling.time.local_day_updated = TRUE;
		cling.time.local_day = cling.time.local.day;
		
		// Reset reminder
		cling.reminder.state = REMINDER_STATE_CHECK_NEXT_REMINDER;
		
		Y_SPRINTF("[RTC] local day updated");
	}

	// Testing, assuming user sleeps around 22:00 at night
	if (cling.time.local_hour != cling.time.local.hour) {
		if (cling.time.local.hour == 12) {
			cling.time.local_noon_updated = TRUE;
		}
		cling.time.local_hour = cling.time.local.hour;
	}
}

// RTC is set to Calendar mode
EN_STATUSCODE RTC_Init(void)
{
#ifndef _CLING_PC_SIMULATION_
	uint32_t err_code;
	
	// Initialize timer module.
	APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);

	err_code = app_timer_create(&m_rtc_timer_id,
															APP_TIMER_MODE_REPEATED,
															RTC_timer_handler);

	APP_ERROR_CHECK(err_code);
	
	err_code = app_timer_create(&m_operation_timer_id,
															APP_TIMER_MODE_REPEATED,
															OPERATION_timer_handler);

	APP_ERROR_CHECK(err_code);
#endif
  return STATUSCODE_SUCCESS;
}

EN_STATUSCODE RTC_Start(void)
{
#ifndef _CLING_PC_SIMULATION_

    app_timer_start(m_rtc_timer_id, SYSCLK_INTERVAL_2000MS, NULL);

    RTC_start_operation_clk();

    N_SPRINTF("[RTS] reset tick count, start 20 ms sysclock");
#endif
    return STATUSCODE_SUCCESS;
}

//
// We might not need RTC stop as RTC runs all the time
//
void RTC_stop_operation_clk(void)
{
#ifndef _CLING_PC_SIMULATION_
    I32U t_curr = CLK_get_system_time();

    N_SPRINTF("[RTC] status: %d, %d, %d", cling.time.sysclk_interval, cling.time.sysclk_config_timestamp, t_curr);

    if (t_curr > (cling.time.operation_clk_start_in_ms + OPERATION_CLK_EXPIRATION)) {

        if (cling.time.operation_clk_enabled) {
            N_SPRINTF("[SYSCLK] OPERATION clk stop, %d @ %d ", t_curr, cling.time.operation_clk_start_in_ms);
            cling.time.operation_clk_enabled = FALSE;
            app_timer_stop(m_operation_timer_id);
        }
    }
#endif
}

void RTC_start_operation_clk()
{
    I32U err_code;
#ifndef _CLING_PC_SIMULATION_
    if (!cling.time.operation_clk_enabled) {
        cling.time.operation_clk_start_in_ms = CLK_get_system_time();			
        N_SPRINTF("[SYSCLK] OPERATION clk start, %d ", cling.time.operation_clk_start_in_ms);
        cling.time.operation_clk_enabled = TRUE;
        err_code = app_timer_start(m_operation_timer_id, SYSCLK_INTERVAL_20MS, NULL);
        APP_ERROR_CHECK(err_code);
    }
#endif
}

void RTC_system_shutdown_timer()
{
#ifndef _CLING_PC_SIMULATION_
    app_timer_stop(m_rtc_timer_id);
    app_timer_start(m_rtc_timer_id, SYSCLK_INTERVAL_6000MS, NULL);
#endif
}


void RTC_get_delta_clock_forward(SYSTIME_CTX *delta, I8U offset)
{
    I32U epoch = cling.time.time_since_1970 + offset * EPOCH_DAY_SECOND;
    I16S time_diff_in_minute = cling.time.time_zone;
    time_diff_in_minute *= TIMEZONE_DIFF_UNIT_IN_SECONDS;
    epoch += time_diff_in_minute;

    RTC_get_regular_time(epoch, delta);
}

void RTC_get_delta_clock_backward(SYSTIME_CTX *delta, I8U offset)
{
    I32U epoch = cling.time.time_since_1970 - offset * EPOCH_DAY_SECOND;
    I16S time_diff_in_minute = cling.time.time_zone;
    time_diff_in_minute *= TIMEZONE_DIFF_UNIT_IN_SECONDS;
    epoch += time_diff_in_minute;

    RTC_get_regular_time(epoch, delta);
}

void RTC_get_local_clock(I32U epoch_start, SYSTIME_CTX *local)
{
    I32U epoch;
    I32S time_diff_in_minute = cling.time.time_zone;

    time_diff_in_minute *= TIMEZONE_DIFF_UNIT_IN_SECONDS;
    epoch = epoch_start + time_diff_in_minute;

    RTC_get_regular_time(epoch, local);

}

void RTC_get_regular_time(I32U epoch, SYSTIME_CTX *t)
{
    I8U i;
    I32U s;
    I32U tick = epoch;
    I32U dayno = tick / EPOCH_DAY_SECOND;
    I32U dayclock = tick - dayno * EPOCH_DAY_SECOND;

    t->year = 1970;
    t->month = 1;
    t->day = 1;
    t->hour = dayclock / 3600;
    t->minute = (dayclock - t->hour * 3600) / 60;
    t->second = dayclock % 60;
    t->dow = (dayno + 3) % 7;

    // Give a life span of 200 years
    s = 31556926;
    for (i = 0; i < 200; i++) {
        // First figure out the year
        if ((t->year & 0x03) == 0) {
            s = 366;
        } else {
            s = 365;
        }
        if (dayno < s) {
            break;
        }
        t->year++;
        dayno -= s;
    }

    // get the month
    if ((t->year & 0x03) == 0) {
        for (i = 0; i < 12; i++) {
            if (dayno < month_leap_in_days[i]) {
                break;
            }
            t->month ++;
            dayno -= month_leap_in_days[i];
        }
    } else {
        for (i = 0; i < 12; i++) {
            if (dayno < month_normal_in_days[i]) {
                break;
            }
            t->month ++;
            dayno -= month_normal_in_days[i];
        }
    }

    // get the day
    t->day += dayno;

}

I32U RTC_get_epoch_day_start(I32U past_days)
{
    I32U current_epoch = cling.time.time_since_1970;
    I32U offset = cling.time.local.second;

    offset += cling.time.local.minute * 60;
    offset += cling.time.local.hour * 3600;

    offset += past_days * EPOCH_DAY_SECOND;

    current_epoch -= offset;

    return current_epoch;
}

I32U RTC_get_last_minute_epoch()
{
    I32U current_epoch = cling.time.time_since_1970;
    I32U offset = cling.time.local.second;

    offset += 60;
    current_epoch -= offset;

    return current_epoch;
}

/* @} */
