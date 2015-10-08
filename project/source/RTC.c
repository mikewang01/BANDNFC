/***************************************************************************//**
 * @file RTC_MSP430.c
 * @brief Driver implementation for the MSP430 real-time clock.
 *
 *
 ******************************************************************************/


#include "standards.h"
#include "main.h"

static volatile BOOLEAN bWriteRTC = BOOLEAN_FALSE;
#ifndef _CLING_PC_SIMULATION_
static app_timer_id_t								m_rtc_timer_id; /**< 1 sec based timer. >**/
#endif

const I8U DAYS_IN_EACH_MONTH[] = {31,28,31,30,31,30,31,31,30,31,30,31};

// RTC is set to Calendar mode
EN_STATUSCODE RTC_Init(void)
{
#ifndef _CLING_PC_SIMULATION_
	uint32_t err_code;
	
	err_code = app_timer_create(&m_rtc_timer_id,
															APP_TIMER_MODE_REPEATED,
															RTC_timer_handler);

	APP_ERROR_CHECK(err_code);
#endif
  return STATUSCODE_SUCCESS;
}

EN_STATUSCODE RTC_Start(void)
{
#ifndef _CLING_PC_SIMULATION_
	RTC_config(SYSCLK_INTERVAL_20MS);
	
	N_SPRINTF("[RTS] reset tick count, start 20 ms sysclock");
#endif
  return STATUSCODE_SUCCESS;
}

//#define TIMEING_TEST

void RTC_config(I32U interval)
{
#ifndef _CLING_PC_SIMULATION_
	uint32_t err_code;

	/////
	I32U tick_now=0;
	I32U tick_diff=0;
	I32U tick_in_s;
#ifdef TIMEING_TEST
	I32U aaaa, bbbb;
#endif
	//////
	if (cling.time.sysclk_interval == interval) {
		// Re-fresh timer
		cling.time.sysclk_config_timestamp = CLK_get_system_time();
		return;
	}
#ifdef TIMEING_TEST
	aaaa = CLK_get_system_time();
#endif
	cling.time.sysclk_interval = interval;
	cling.time.sysclk_config_timestamp = CLK_get_system_time();

	// Update tick count on the first interrupt
	// so that we can sync up to system tick.
	
	// Before reset the tick count, we should accumualte the residual for correct timing
	// The residual is the lower 15 bits
	app_timer_cnt_get(&tick_now);
	app_timer_cnt_diff_compute(tick_now, cling.time.tick_count, &tick_diff);
	cling.time.rtc_tick_residual += tick_diff;
	cling.time.rtc_tick_residual += (cling.time.tick_count&0x7fff);
	
	// if residual tick is greater than 1 second, add it to system clock
	tick_in_s = (cling.time.rtc_tick_residual>>15);
	cling.time.system_clock_in_sec += tick_in_s;
	cling.time.rtc_tick_residual -= (tick_in_s << 15);
	
	// Stop the timer
	app_timer_stop(m_rtc_timer_id);

	// Start application with new timers
	err_code = app_timer_start(m_rtc_timer_id, interval, NULL);
	APP_ERROR_CHECK(err_code);
	
	// Add about 1 ms timer delay
	cling.time.rtc_tick_residual += 32;

	// Initialize the tick count
	app_timer_cnt_get(&cling.time.tick_count);
#ifdef TIMEING_TEST
	bbbb = CLK_get_system_time();
	Y_SPRINTF("[SYSCLK] config timer: %d, %d, %d, %d, %d", 
	interval, cling.time.tick_count, cling.time.rtc_tick_residual, aaaa, bbbb);
#endif
#endif
}

// 
// We might not need RTC stop as RTC runs all the time
//
void RTC_stop(void)
{
#ifndef _CLING_PC_SIMULATION_
	I32U t_curr = CLK_get_system_time();
	
	N_SPRINTF("[RTC] status: %d, %d, %d", cling.time.sysclk_interval, cling.time.sysclk_config_timestamp, t_curr);
	
	if (t_curr > (cling.time.sysclk_config_timestamp + SYSCLK_EXPIRATION)) {
		N_SPRINTF("[SYSCLK] RTC stop, %d @ %d ", t_curr, cling.time.sysclk_config_timestamp);

		if (!cling.system.b_powered_up) {
			// RTC configure to a much lower clock
			RTC_config(SYSCLK_INTERVAL_4000MS);
		} else {
			RTC_config(SYSCLK_INTERVAL_2000MS);		
		}
	}
#endif
}


void RTC_get_delta_clock_forward(SYSTIME_CTX *delta, I8U offset)
{
	I32U epoch = cling.time.time_since_1970+offset*EPOCH_DAY_SECOND;
	I16S time_diff_in_minute = cling.time.time_zone;
	time_diff_in_minute *= TIMEZONE_DIFF_UNIT_IN_SECONDS;
	epoch += time_diff_in_minute;
	
	RTC_get_regular_time(epoch, delta);
}

void RTC_get_delta_clock_backward(SYSTIME_CTX *delta, I8U offset)
{
	I32U epoch = cling.time.time_since_1970-offset*EPOCH_DAY_SECOND;
	I16S time_diff_in_minute = cling.time.time_zone;
	time_diff_in_minute *= TIMEZONE_DIFF_UNIT_IN_SECONDS;
	epoch += time_diff_in_minute;

	RTC_get_regular_time(epoch, delta);
}

void RTC_get_local_clock(SYSTIME_CTX *local)
{
	I32U epoch = cling.time.time_since_1970;
	I32S time_diff_in_minute = cling.time.time_zone;
	time_diff_in_minute *= TIMEZONE_DIFF_UNIT_IN_SECONDS;
	epoch += time_diff_in_minute;
	
	RTC_get_regular_time(epoch, local);

}

void RTC_timer_handler( void * p_context )
{
	I32U tick_now;
	I32U tick_diff;
	I32U tick_in_s;

#ifndef _CLING_PC_SIMULATION_	
	app_timer_cnt_get(&tick_now);
	app_timer_cnt_diff_compute(tick_now, cling.time.tick_count, &tick_diff);
	tick_in_s = (tick_diff>>15);
	tick_diff = (tick_in_s << 15);
	cling.time.tick_count += tick_diff;
	cling.time.tick_count &= MAX_RTC_CNT;
	
#else
	tick_in_s = 1; // 1 second timer
#endif
	
	// Timer update
	cling.hr.sample_ready = TRUE;
	
	// update battery measuring timer
	cling.batt.level_update_timebase += tick_in_s;
	BATT_update_charging_time(tick_in_s);
	BATT_exit_charging_state(tick_in_s);

	// update radio duty cycling
	cling.time.system_clock_in_sec += tick_in_s;
	cling.time.time_since_1970 += tick_in_s;
	if (cling.batt.state_switching_duration < 128)
		cling.batt.state_switching_duration += tick_in_s;
	if (cling.batt.shut_down_time < BATTERY_SYSTEM_UNAUTH_POWER_DOWN) {
		cling.batt.shut_down_time += tick_in_s;
	}
	
#ifdef USING_VIRTUAL_ACTIVITY_SIM
	 if (OTA_if_enabled()) {
		 cling.ota.percent ++;
		 if (cling.ota.percent >= 100) {
			 cling.ota.percent = 100;
		 }
	 }
#endif

	// Indicates a second-based RTC interrupt
	RTC_get_local_clock(&cling.time.local);

	// Check if we have minute passed by, or 
	if (cling.time.local.minute != cling.time.local_minute) {
		cling.time.local_minute_updated = TRUE;
		cling.time.local_minute = cling.time.local.minute;
		
		if (
			   cling.user_data.idle_time_in_minutes>0 && 
				 cling.time.local.hour>=cling.user_data.idle_time_start && 
				 cling.time.local.hour< cling.user_data.idle_time_end
			 )
    {
			cling.user_data.idle_minutes_countdown --;
		}
		N_SPRINTF("[RTC] min updated (%d)", cling.activity.day.walking);
	}	
	
	if (cling.time.local.day != cling.time.local_day) {
		cling.time.local_day_updated = TRUE;
		cling.time.local_day = cling.time.local.day;
		
		// Reset reminder
		cling.reminder.state = REMINDER_STATE_CHECK_NEXT_REMINDER;
	}

	// Testing, assuming user sleeps around 22:00 at night
	if (cling.time.local_hour != cling.time.local.hour) {
		if (cling.time.local.hour == 12) {
			cling.time.local_noon_updated = TRUE;
		}
		cling.time.local_hour = cling.time.local.hour;
	}
}

I8U const month_leap_in_days[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
I8U const month_normal_in_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

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
	t->hour = dayclock/3600;
	t->minute = (dayclock - t->hour*3600)/60;
	t->second = dayclock % 60;
	t->dow = (dayno+3)%7;
	
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
/* @} */
