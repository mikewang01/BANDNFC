/***************************************************************************/
/**
 * File: user.c
 * 
 * Description: USER related data processing functions
 *
 * Created on Feb 21, 2014
 *
 ******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "pedo.h"

static BOOLEAN _is_idle_alert_allowed()
{	
	if (OTA_if_enabled()) {
		return FALSE;
	}
	
	if (!cling.hr.b_closing_to_skin) {
		return FALSE;
	}
	
	return TRUE;
}


void USER_default_setup()
{
	I8U data[128];
	I32U value;
	I8U *pdata;
	
	FLASH_erase_App(SYSTEM_USER_DEVICE_SPACE_START);

	memset(data, 0, 128);
		
	// ------------------ Profile Space (the first 64 bytes) ---------------------
	pdata = data;
	value = 170; // height
	*pdata++ = value & 0xff;
	*pdata++ = (value>>8)&0xff; // 
	
	value = 70; // weight
	*pdata++ = value & 0xff;
	*pdata++ = (value>>8)&0xff; // 
	
	value = 75; // walking stride
	*pdata++ = value & 0xff;
	*pdata++ = (value>>8)&0xff; // 
	
	value = 105; // running stride
	*pdata++ = value & 0xff;
	*pdata++ = (value>>8)&0xff; // 

	value = 0; // Metric unit for distance
	*pdata++ = value;
	
	value = 0; // name length
	*pdata++ = value;
	
	value = 0; // sleep dow
	*pdata++ = value;
	
	value = 22; // bed time in hh
	*pdata++ = value;
	
	value = 45; // bed time in mm
	*pdata++ = value;
	
	value = 6; // wakeup time in hh
	*pdata++ = value;
	
	value = 0; // wakeup time in mm
	*pdata++ = value;
	
	value = 0x1f; // page display
	*pdata++ = value;
	
	value = 1; // touch key vibration
	*pdata++ = value;
	
	value = 2; // the mileage limit
	*pdata++ = value;

	value = FALSE; // the SEX
	*pdata++ = value;
	
	value = 100; // treadmill stride
	*pdata++ = value & 0xff;
	*pdata++ = (value>>8)&0xff; // 

	value = 90; // Max heart rate
	*pdata++ = value;
	
	// ------------------ Device Space (the second 64 bytes) ---------------------
	pdata = data+64;

	value = 900000;
	*pdata++ = (value>>24)&0xff; // day interval
	*pdata++ = (value>>16)&0xff; // day interval
	*pdata++ = (value>>8)&0xff; // day interval
	*pdata++ = (value&0xff); // day interval
	
	value = 1800000; // 30 minutes
	*pdata++ = (value>>24)&0xff; // night interval
	*pdata++ = (value>>16)&0xff; // night interval
	*pdata++ = (value>>8)&0xff; // night interval
	*pdata++ = (value&0xff); // night interval

	value = 600000;
	*pdata++ = (value>>24)&0xff; // skin temp day interval
	*pdata++ = (value>>16)&0xff; // skin temp day interval
	*pdata++ = (value>>8)&0xff; // skin temp day interval
	*pdata++ = (value&0xff); // skin temp day interval
	
	value = 600000;
	*pdata++ = (value>>24)&0xff; // skin temp night interval
	*pdata++ = (value>>16)&0xff; // skin temp night interval
	*pdata++ = (value>>8)&0xff; // skin temp night interval
	*pdata++ = (value&0xff); // skin temp night interval

	// gesture recognition
	value = 0;
	*pdata++ = value; // b_screen_wrist_flip;
	value = 1;
	*pdata++ = value; // b_screen_press_hold_1;
	value = 0;
	*pdata++ = value; // b_screen_press_hold_3;
	value = TRUE;
	*pdata++ = value; // b_running_alwayson;
	
	value = 0;
	*pdata++ = value; // b_navigation_tapping;
	value = 0;
	*pdata++ = value; // b_navigation_wrist_shake;

	value = 0;
	*pdata++ = value; // idle_time_in_minutes;
	*pdata++ = value; // Idle alert - start time
	*pdata++ = value; // Idle alert - end time
	
	value = 5;
	*pdata++ = value; // screen_on_general;
	value = 25;
	*pdata++ = value; // screen_on_heart_rate;
	
	value = SLEEP_SENSITIVE_LOW;
	*pdata++ = value; // cling.sleep.m_sensitive_mode;
	
	value = 0;
	*pdata++ = value; // b_reminder_off_weekends;
	
	value = PEDO_SENSITIVITY_HIGH;
	*pdata++ = value; //m_pedo_sensitivity;
	
	// Latency before writing notification message (Erasure latency: 50 ms)
	BASE_delay_msec(25);

	FLASH_Write_App(SYSTEM_USER_DEVICE_SPACE_START, data, 128);
}

void USER_setup_profile(I8U *data)
{
	USER_PROFILE_CTX *p = &cling.user_data.profile;
	I16U user_info;
	I8U *pdata = data;
	
	user_info = *pdata++; // height
	user_info |= (*pdata++)<<8; // 
	p->height_in_cm = user_info; // in center meters
	
	user_info = *pdata++; // weight
	user_info |= (*pdata++)<<8; // 
	p->weight_in_kg = user_info; // in KG
	
	user_info = *pdata++; // stride for walking
	user_info |= (*pdata++)<<8; // 
	p->stride_in_cm = user_info; // In center meters
	
	user_info = *pdata++; // stride for running
	user_info |= (*pdata++)<<8; // 
	p->stride_running_in_cm = user_info;
	
	// If it cannot pass sanity check, go return
	if ((p->height_in_cm > 250) ||
		  (p->weight_in_kg > 250) || 
	    (p->stride_in_cm > 500) || 
	    (p->stride_running_in_cm > 1000))
	{
		p->height_in_cm = 170;
		p->weight_in_kg = 70;
		p->stride_in_cm = 75;
		p->stride_running_in_cm = 105;
		p->stride_treadmill_in_cm = 100;
		p->metric_distance = 0;
		p->name_len = 0;
		cling.ui.clock_orientation = 0;
		p->sleep_dow = 0;
		p->regular_page_display_1 = 0xff;
		p->regular_page_display_2 = 0xff;
		p->running_page_display = 0xff;
		p->touch_vibration = FALSE;
		p->mileage_limit = 3;
		p->sex = FALSE;
		p->age = 17;
		p->max_hr_alert = 90;
		p->running_rate = 172;
		return;
	}
	
	Y_SPRINTF("[USER] PROFILE1: %d, %d, %d, %d", p->height_in_cm, p->weight_in_kg, p->stride_in_cm, p->stride_running_in_cm);
	
	p->metric_distance = *pdata++; // Metric unit for distance
	
	p->name_len = *pdata++; // the length of user name
	
	memset(p->name, 0, 24);
	memcpy(p->name, pdata, p->name_len); // user name
	pdata += p->name_len;

	Y_SPRINTF("[USER] PROFILE2: %d, %d, %s", p->metric_distance, p->name_len, p->name);

	cling.ui.clock_orientation = *pdata++; // Clock face orientation
	
	// Wake up and bed time
	p->sleep_dow = *pdata++;
	p->bed_hh = *pdata++;
	p->bed_mm = *pdata++;
	p->wakeup_hh = *pdata++;
	p->wakeup_mm = *pdata++;
	Y_SPRINTF("[USER] PROFILE3: %d, %d, %d, %d, %d, %d", cling.ui.clock_orientation, 
		p->sleep_dow,
		p->bed_hh,
		p->bed_mm,
		p->wakeup_hh,
		p->wakeup_mm);

	p->regular_page_display_1 = *pdata++; // Regular page display options
	
	p->touch_vibration = *pdata++; // touch key vibration
	
	p->mileage_limit = *pdata++; // the mileage limit
	
	p->running_page_display = *pdata++;  // Running page display options
	
	p->age = *pdata++;  // Running page display options
	
	Y_SPRINTF("[USER] PROFILE4: %02x, %02x, %02x, %02x, (age: %d)", 
		p->regular_page_display_1, p->touch_vibration, p->mileage_limit, p->running_page_display, p->age);

	p->sex = *pdata++;
	user_info = *pdata++; // stride for running
	user_info |= (*pdata++)<<8; // 
	p->stride_treadmill_in_cm = user_info;
	p->max_hr_alert = *pdata++;
	Y_SPRINTF("[USER] PROFILE5: %02x, %02x, %02x, ", 
		p->sex, p->stride_treadmill_in_cm, p->max_hr_alert);
		
	// Add more page settings for regular home pages
	p->regular_page_display_2 = *pdata++;
	Y_SPRINTF("[USER] PROFILE regular page display 2 :%02x",p->regular_page_display_2);
	
	// Training alert (Pace / HR alarm zone)
	p->training_alert = *pdata++;
	
	// App setting
	p->app_setting = *pdata++;
	Y_SPRINTF("[USER] PROFILE range&setting: %02x :%02x", p->training_alert, p->app_setting);
	
	// Running Rate
	p->running_rate = *pdata++;
	Y_SPRINTF("[USER] Running rate: %d", p->running_rate);
	// If running rate is less than 150, which is unrealistic number, set to 172
	if (p->running_rate < 150) 
		p->running_rate = 172;
}

#if 0
static __INLINE I32U _get_dword_value(I8U *pdata)
{
	I32U value;
	
	value = pdata[0]; // day interval
	value <<= 8; // 
	value |= pdata[1]; // 
	value <<= 8; // 
	value |= pdata[2]; // 
	value <<= 8; // 
	value |= pdata[3]; // 
	
	return value;
}
#endif

void USER_setup_device(I8U *data)
{
	USER_DATA *u = &cling.user_data;
	I8U *pdata = data;
//	I32U value;

//	value = _get_dword_value(pdata); // day interval
	pdata += 4;
	// 5 minutes is the minimum
#if 1 // Not used
	u->ppg_day_interval = 900000;
#endif

//	value = _get_dword_value(pdata); // night interval
	pdata += 4;
	// 5 minutes is the minimum
#if 1 // not used
	u->ppg_night_interval = 1800000;
#endif

//	value = _get_dword_value(pdata); // night interval
	pdata += 4;
	// 5 minutes is the minimum
#if 1 // Not used
	u->skin_temp_day_interval = 600000;
#endif

//	value = _get_dword_value(pdata); // night interval
	pdata += 4;
	// 5 minutes is the minimum
#if 1 // Not used
	u->skin_temp_night_interval = 600000;
#endif
	// gesture recognition 
	u->b_screen_wrist_flip = *pdata++;
	if (u->b_screen_wrist_flip == 0xff) {
		u->b_screen_wrist_flip = FALSE; // Default OFF
	}
	u->b_screen_press_hold_1 = *pdata++;
	if (u->b_screen_press_hold_1 == 0xff) {
		u->b_screen_press_hold_1 = TRUE; // Default ON
	}
	u->b_screen_press_hold_3 = *pdata++;
	if (u->b_screen_press_hold_3 == 0xff) {
		u->b_screen_press_hold_3 = FALSE; // Default OFF
	}
	u->b_running_alwayson = *pdata++;
	u->b_navigation_tapping = *pdata++;
	if (u->b_navigation_tapping == 0xff) {
		u->b_navigation_tapping = FALSE;
	}
	u->b_navigation_wrist_shake = *pdata++;
	u->b_navigation_wrist_shake = FALSE;// NOT used

	Y_SPRINTF("[USER] DEVICE1: %d,%d,%d,%d", 
		u->ppg_day_interval,
		u->ppg_night_interval,
		u->skin_temp_day_interval,
		u->skin_temp_night_interval);
	Y_SPRINTF("[USER] DEVICE2: %d,%d,%d,%d,%d,%d",
		u->b_screen_wrist_flip,
		u->b_screen_press_hold_1,
		u->b_screen_press_hold_3,
		u->b_running_alwayson,
		u->b_navigation_tapping,
		u->b_navigation_wrist_shake);
	
	u->idle_time_in_minutes = *pdata++;
	if (u->idle_time_in_minutes == 0xff) {
		u->idle_time_in_minutes = 0;
	}
	u->idle_time_start = *pdata++; // Idle alert - start time
	if (u->idle_time_start == 0xff) {
		u->idle_time_start = 0;
	}
	u->idle_time_end = *pdata++; // Idle alert - end time
	if (u->idle_time_end == 0xff) {
		u->idle_time_end = 0;
	}
	
	// Do not reset idle alert state machine during data sync
	// u->idle_state = IDLE_ALERT_STATE_IDLE;

	Y_SPRINTF("[USER] DEVICE3: %d, %d, %d", u->idle_time_in_minutes, u->idle_time_start, u->idle_time_end);

	u->screen_on_general = *pdata++;
	if (u->screen_on_general == 0xff) {
		u->screen_on_general = 5;
	}
	u->screen_on_heart_rate = *pdata++;
	if (u->screen_on_heart_rate == 0xff) {
		u->screen_on_heart_rate = 25;
	}
	cling.sleep.m_sensitive_mode = (SLEEP_SENSITIVE_MODE)(*pdata++);
	if (cling.sleep.m_sensitive_mode == (SLEEP_SENSITIVE_MODE)0xff) {
		cling.sleep.m_sensitive_mode = SLEEP_SENSITIVE_LOW;
	}
	u->m_pedo_sensitivity = *pdata++;
	if (u->m_pedo_sensitivity > PEDO_SENSITIVITY_LOW) {
		u->m_pedo_sensitivity = PEDO_SENSITIVITY_HIGH;
	}
	u->b_reminder_off_weekends = *pdata++;
	
	Y_SPRINTF("[USER] DEVICE4: %d,%d,%d,%d,%d\n", 
		u->screen_on_general, 
		u->screen_on_heart_rate, 
		cling.sleep.m_sensitive_mode, 
		u->b_reminder_off_weekends, 
		u->m_pedo_sensitivity);
}

void USER_state_machine()
{
	USER_DATA *u = &cling.user_data;
	
	switch (u->idle_state) {
		case IDLE_ALERT_STATE_IDLE:
			if (u->idle_time_in_minutes > 0) {
				if (cling.time.local.hour>=cling.user_data.idle_time_start) {
					if (cling.time.local.hour < cling.user_data.idle_time_end) {
						u->idle_minutes_countdown = u->idle_time_in_minutes;
						u->idle_step_countdown = u->idle_time_in_minutes<<2;
						u->idle_state = IDLE_ALERT_STATE_COUNT_DOWN;
						N_SPRINTF("[USER] reset idle alert: %d, %d", u->idle_minutes_countdown, u->idle_step_countdown);
					}
				}
			}
			break;

		case IDLE_ALERT_STATE_COUNT_DOWN:
			if (u->idle_minutes_countdown ==0) {
				if (u->idle_time_in_minutes == 0) {
					u->idle_state = IDLE_ALERT_STATE_IDLE;
				} else if (cling.time.local.hour < cling.user_data.idle_time_start) {
					u->idle_state = IDLE_ALERT_STATE_IDLE;
				} else if (cling.time.local.hour >= cling.user_data.idle_time_end) {
					u->idle_state = IDLE_ALERT_STATE_IDLE;
				} else {
					if (_is_idle_alert_allowed()) {
						u->idle_state = IDLE_ALERT_STATE_NOTIFY;
						N_SPRINTF("[USER] start idle alert");
					}
				}
	    }
			break;

		case IDLE_ALERT_STATE_NOTIFY:
			NOTIFIC_start_idle_alert();
			u->idle_state = IDLE_ALERT_STATE_IDLE;
			break;
			
		default:
			u->idle_state = IDLE_ALERT_STATE_IDLE;
			break;
  }
}

void USER_device_specifics_init()
{
	I8U buf[128];
	I8U *pbuf;
	
	pbuf = (I8U *)buf;
	FLASH_Read_App(SYSTEM_USER_DEVICE_SPACE_START, pbuf, 128);

	// Setup profile detail
	USER_setup_profile(pbuf);
	
	pbuf = (I8U *)buf + 64;
	
	// Setup device operation
	USER_setup_device(pbuf);
}

void USER_device_specifics_setup(I8U *data, BOOLEAN b_profile, BOOLEAN b_device)
{
	I8U buf[128];
	I8U *pbuf;
	I8U *p_profile_buf;
	I8U *p_device_buf;
	
	if (b_profile || b_device) {
		pbuf = (I8U *)buf;
		FLASH_Read_App(SYSTEM_USER_DEVICE_SPACE_START, pbuf, 128);
		FLASH_erase_App(SYSTEM_USER_DEVICE_SPACE_START);
	} else {
		return;
	}
	
	if (b_profile) {
		p_profile_buf = buf;
		memcpy(p_profile_buf, data, 64);
		USER_setup_profile(p_profile_buf);
	}
	
	if (b_device) {
		p_device_buf = buf+64;
		memcpy(p_device_buf, data, 64);
		USER_setup_device(p_device_buf);
	}
	
	BASE_delay_msec(10);
	
	// First 
	FLASH_Write_App(SYSTEM_USER_DEVICE_SPACE_START, buf, 128);
}

void USER_device_get_name(I8U *string)
{
	if (cling.user_data.profile.name_len) {
		sprintf((char *)string, "%s", cling.user_data.profile.name);
	} else {
#ifdef _CLINGBAND_PACE_MODEL_		
		sprintf((char *)string, "PACE Fitness");
#endif	
#ifdef _CLINGBAND_2_PAY_MODEL_		
		sprintf((char *)string, "Clingband 2");
#endif				
#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)		
		sprintf((char *)string, "Cling Fitness");
#endif		
	}
}
