/***************************************************************************/
/**
 * File: reminder.c
 * 
 * Description: user reminder processing
 *
 * Created on May 15, 2014
 *
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "main.h"


void REMINDER_setup(I8U *msg, BOOLEAN b_daily)
{
	I32U data[32];
	I8U *pdata = (I8U*)data;
	I8U *pmsg;
	I8U i, a, b, c, day;
	BOOLEAN b_init_alarm = FALSE;
	
	// Get current day
	day = 1 << cling.time.local.dow;

	FLASH_erase_App(SYSTEM_REMINDER_SPACE_START);
	BASE_delay_msec(50); // Latency before refreshing reminder space. (Erasure latency: 50 ms)
	if (b_daily) {
		// 3 bytes per one alarm clock
		cling.reminder.total = (msg[0] / 3);
	} else {
		// 2 bytes per one alarm clock
		cling.reminder.total = (msg[0] >> 1);
	}
	if (cling.reminder.total > 32)
		cling.reminder.total = 32;

	cling.reminder.ui_hh = 0xff;
	cling.reminder.ui_mm = 0xff;
	
	pmsg = msg + 1;
  if (cling.reminder.total > 0) {
		
		memset(pdata, 0, 128);
		
		// Copy all the alarm clock, and set it to everyday
		for (i = 0; i < cling.reminder.total; i++) {
			if (b_daily) 
				c = *pmsg++;
			else
				c = 0x7f;
			//*pdata++ = *pmsg++;
			//*pdata++ = *pmsg++;
			a = *pmsg++;
			b = *pmsg++;
			*pdata++ = c;
			*pdata++ = a;
			*pdata++ = b;

			if (c & day) {
				if (!b_init_alarm) {
					b_init_alarm = TRUE;
					cling.reminder.ui_hh = a;
					cling.reminder.ui_mm = b;
				}
			}
			Y_SPRINTF("[CP] reminder setup(%d, %d), %d:%d(%02x)", i, cling.reminder.total, a, b, c);
		}
		pdata = (I8U *)data;
		FLASH_Write_App(SYSTEM_REMINDER_SPACE_START, pdata, 128); // Maximum 32 entries
	}
	
	cling.reminder.state = REMINDER_STATE_CHECK_NEXT_REMINDER;			
}

void REMINDER_set_next_normal_reminder()
{
	I8U i, hour, minute, week, day;
	BOOLEAN b_found = FALSE;
	BOOLEAN b_first_alarm = FALSE;
	I32U buffer[32];
	I8U *data = (I8U *)buffer;
	
	cling.reminder.ui_hh = 0xff;
	cling.reminder.ui_mm = 0xff;
	cling.reminder.b_daily_alarm = FALSE;
	
	if (!LINK_is_authorized()) {
		cling.reminder.b_valid_alarm = FALSE;
		cling.reminder.hour = 0;
		cling.reminder.minute = 0;
		N_SPRINTF("[REMINDER] NOT authorized, set invalid hh/mm");
		return;
	}
	
	if (cling.user_data.b_reminder_off_weekends) {
		// Check if today is Saturday or Sunday
		if (cling.time.local.dow > 4) {
			cling.reminder.b_valid_alarm = FALSE;
			cling.reminder.hour = 0;
			cling.reminder.minute = 0;
			N_SPRINTF("[REMINDER] Weekend OFF mode, set invalid hh/mm");
			return;
		}
	}
	
	if (cling.reminder.total) {
		FLASH_Read_App(SYSTEM_REMINDER_SPACE_START, (I8U *)buffer, 128);
		
		// Get current day
		day = 1 << cling.time.local.dow;
		N_SPRINTF("[REMINDER] today is: 0x%02x, %d", day, cling.time.local.dow);

		for (i = 0; i < cling.reminder.total; i++) {
			week = *data++;
			hour = *data++;
			minute = *data++;
			
			N_SPRINTF("[REMINDER] current time: %d:%d, reminder: %d:%d, %02x", 
				cling.time.local.hour, cling.time.local.minute, hour, minute, week);

			if (hour >= 24) break;
			if (minute >= 60) break;
			
			if (week & day) {
				
				cling.reminder.b_daily_alarm = TRUE;
				
				if (!b_first_alarm) {
					b_first_alarm = TRUE;
					cling.reminder.ui_hh = hour;
					cling.reminder.ui_mm = minute;
					N_SPRINTF("[REMINDER] first reminder is set: %d:%d", hour, minute);

				}
				if (hour == cling.time.local.hour) {
					if (minute > cling.time.local.minute) {
						b_found = TRUE;				
						break;
					}
				} else if (hour > cling.time.local.hour) {
					b_found = TRUE;
					break;
				}
			}
		}
	}
	
	if (b_found) {
		cling.reminder.b_valid_alarm = TRUE;
		cling.reminder.hour = hour;
		cling.reminder.minute = minute;
		
		N_SPRINTF("[REMINDER] found reminder: %d:%d", cling.reminder.hour, cling.reminder.minute);
	} else {
		cling.reminder.b_valid_alarm = FALSE;
		cling.reminder.hour = 0;
		cling.reminder.minute = 0;

		N_SPRINTF("[REMINDER] No new reminder is found");
	}
	
	if (!b_first_alarm) {
		N_SPRINTF("[REMINDER] NOT reminder found for today, set invalid hh/mm");
  }

	N_SPRINTF("[REMINDER] Set next reminder - done");
}

static BOOLEAN _check_normal_reminder()
{
	if (!cling.reminder.b_valid_alarm)
		return FALSE;
	
	if ((cling.time.local.hour == cling.reminder.hour) && (cling.time.local.minute == cling.reminder.minute)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void REMINDER_set_sleep_reminder()
{
	I8U week, day;
	USER_PROFILE_CTX *p = &cling.user_data.profile;
	
	cling.reminder.b_sleep_total = FALSE;
	cling.reminder.b_sleep_valid = FALSE;
	cling.reminder.b_wakeup_valid = FALSE;

	if (p->sleep_dow & 0x80) {
		week = p->sleep_dow & 0x7f;
		day = 1 << cling.time.local.dow;
		
		if (week & day) {
			cling.reminder.b_sleep_valid = TRUE;
			cling.reminder.b_wakeup_valid = TRUE;
      cling.reminder.b_sleep_total = TRUE;
		}
	}
}

static BOOLEAN _check_sleep_reminder()
{	
	USER_PROFILE_CTX *p = &cling.user_data.profile;
	
	// Add sleep bed & wakeup time
	if (cling.reminder.b_sleep_valid) {
			if ((cling.time.local.hour == p->bed_hh) && 
					(cling.time.local.minute == p->bed_mm))
			{
				cling.reminder.alarm_type = SLEEP_ALARM_CLOCK;
				cling.reminder.b_sleep_valid = FALSE;
				cling.ui.notific.alarm_clock_hh = p->bed_hh;
				cling.ui.notific.alarm_clock_mm = p->bed_mm;		
				return TRUE;
			}
	}

	// Add wakeup wakeup time
	if (cling.reminder.b_wakeup_valid) {
			if ((cling.time.local.hour == p->wakeup_hh) && 
					(cling.time.local.minute == p->wakeup_mm))
			{
				cling.reminder.alarm_type = WAKEUP_ALARM_CLOCK;
				cling.reminder.b_wakeup_valid = FALSE;
				cling.ui.notific.alarm_clock_hh = p->wakeup_hh;
				cling.ui.notific.alarm_clock_mm = p->wakeup_mm;						
				return TRUE;
			}
	}
	
	return FALSE;
}

void REMINDER_state_machine()
{
	I32U t_curr = CLK_get_system_time();
	
	if (OTA_if_enabled())
		return;

	switch (cling.reminder.state) {
		case REMINDER_STATE_IDLE:
		{
			if (_check_sleep_reminder()) {
				// First setup alarm type
				N_SPRINTF("[REMINDER] Sleep reminder is hit @ %d:%d", cling.time.local.hour, cling.time.local.minute);
				cling.reminder.state = REMINDER_STATE_ON;
				cling.reminder.ts = t_curr;
				NOTIFIC_start_notifying(NOTIFICATION_TYPE_SLEEP_ALARM_CLOCK, 0);
			} else if (_check_normal_reminder()) {
				N_SPRINTF("[REMINDER] Work reminder is hit @ %d:%d", cling.time.local.hour, cling.time.local.minute);
				cling.reminder.b_valid_alarm = FALSE;
				cling.reminder.state = REMINDER_STATE_ON;
				cling.reminder.ts = t_curr;				
				cling.reminder.alarm_type = NORMAL_ALARM_CLOCK;
				cling.ui.notific.alarm_clock_hh = cling.reminder.hour;
				cling.ui.notific.alarm_clock_mm = cling.reminder.minute;				
				NOTIFIC_start_notifying(NOTIFICATION_TYPE_NORMAL_ALARM_CLOCK, 0);
			}
			break;
		}
		case REMINDER_STATE_ON:
		{					
			if (cling.notific.state == NOTIFIC_STATE_IDLE) {
				cling.reminder.state = REMINDER_STATE_CHECK_NEXT_REMINDER;
			} else if (t_curr > (cling.reminder.ts + 60000)) {
				cling.reminder.state = REMINDER_STATE_CHECK_NEXT_REMINDER;
			}				
		  break;
		}
		case REMINDER_STATE_CHECK_NEXT_REMINDER:
		{
		  REMINDER_set_next_normal_reminder();	
		  cling.reminder.state = REMINDER_STATE_IDLE;		
		}
		default:
		{
		  REMINDER_set_next_normal_reminder();				
			cling.reminder.state = REMINDER_STATE_IDLE;			
			break;		
		}
	}
}

I8U REMINDER_get_time_at_index(I8U index)
{
	I8U new_idx, hh, mm, i, week, day, total;
	I32U i32_buf[32];
	I8U *data;
	I8U alarm[64];
	I8U *palarm;
		
	// In case that no reminder is configure, go return
	cling.reminder.ui_hh = 0xff;
	cling.reminder.ui_mm = 0xff;
	if (cling.reminder.total == 0) {
		N_SPRINTF("[REMINDER] no reminder due to ZERO total");
		return 0;
	}
	
	// First we get Alarms for the day
	FLASH_Read_App(SYSTEM_REMINDER_SPACE_START, (I8U *)i32_buf, 128);
	
	total = 0;
	data = (I8U *)i32_buf;
	palarm = alarm;

	// Get correct day for alarm checkup
	day = 1 << cling.time.local.dow;

	for (i = 0; i < cling.reminder.total; i++) {
		week = *data++;
		hh = *data++;
		mm = *data++;
		if (week & day) {
			*palarm++ = hh;
			*palarm++ = mm;
			total++;
		}
	}
	
	if (total == 0) {
		N_SPRINTF("[REMINDER] No reminder found for today, set invalid hh/mm");
		return 0;
	}

	palarm = alarm;
	new_idx = index;
	N_SPRINTF("[REMINDER] maximum entry: %d, %d", total, new_idx);

	if (new_idx >= total) {
		N_SPRINTF("[REMINDER] reset entry: %d, %d", total, new_idx);
		new_idx = 0;
	}
	hh = alarm[new_idx<<1];
	mm = alarm[(new_idx<<1)+1];

	if (hh>=24 || mm>=60) {
		N_SPRINTF("[REMINDER] invalid alarm set up from flash, set invalid hh/mm");
		return 0;
	}

	cling.reminder.ui_hh = hh;
	cling.reminder.ui_mm = mm;
	
	cling.ui.notific.alarm_clock_hh = cling.reminder.ui_hh;
	cling.ui.notific.alarm_clock_mm = cling.reminder.ui_mm;				
	
	N_SPRINTF("[REMINDER] ui display: %d:%d", hh, mm);
		
	return new_idx;
}

