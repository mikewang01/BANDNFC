/***************************************************************************/
/**
 * File: notification.c
 * 
 * Description: notification processing
 *
 * Created on May 15, 2014
 *
 ******************************************************************************/

#include <stdio.h>
#include <string.h>


#include "main.h"

#define NOTIFIC_ON_TIME_IN_MS 100
#define NOTIFIC_OFF_TIME_IN_MS 400
#define NOTIFIC_VIBRATION_REPEAT_TIME 3

#define NOTIFIC_MULTI_REMINDER_LATENCY 2000

#define NOTIFIC_MULTI_REMINDER_TIME 15


void NOTIFIC_stop_notifying()
{
	Y_SPRINTF("[NOTIFIC] Stop notifying");
	cling.notific.state = NOTIFIC_STATE_IDLE;
	GPIO_vibrator_set(FALSE);
}

#define NOTIFIC_MULTI_REMINDER_INCOMING_CALL 15
#define NOTIFIC_MULTI_REMINDER_OTHERS         1
#define NOTIFIC_MULTI_REMINDER_IDLE_ALERT     3

void NOTIFIC_start_notifying(I8U cat_id)
{			
	// Reset vibration times
	cling.notific.vibrate_time = 0;
	// The maximum vibration time
	if (cat_id == ANCS_CATEGORY_ID_INCOMING_CALL)
		cling.notific.second_reminder_max = NOTIFIC_MULTI_REMINDER_INCOMING_CALL;
	else
		cling.notific.second_reminder_max = NOTIFIC_MULTI_REMINDER_OTHERS;
	cling.notific.cat_id = cat_id;
	cling.notific.state = NOTIFIC_STATE_SETUP_VIBRATION;
	
	// Also, turn on screen
	UI_turn_on_display(UI_STATE_IDLE, 3000);
	
}

void NOTIFIC_start_idle_alert()
{
	cling.notific.vibrate_time = 0;
	cling.notific.second_reminder_max = NOTIFIC_MULTI_REMINDER_IDLE_ALERT;
	cling.notific.state = NOTIFIC_STATE_SETUP_VIBRATION;
	//UI_turn_on_display(UI_STATE_IDLE, 3000);
	
	Y_SPRINTF("NOTIFIC - IDLE ALERT @ %d:%d", cling.time.local.hour, cling.time.local.minute);
}

void NOTIFIC_state_machine()
{
	I32U t_curr = CLK_get_system_time();
	
	if (cling.notific.state != NOTIFIC_STATE_IDLE) {
			SYSCLK_timer_start();
	}
	
	switch (cling.notific.state) {
		case NOTIFIC_STATE_IDLE:
		{
			break;
		}
		case NOTIFIC_STATE_SETUP_VIBRATION:
		{
			cling.notific.state = NOTIFIC_STATE_ON;
			cling.notific.second_reminder_time = 0;
			Y_SPRINTF("[NOTIFIC] second notif max: %d,", cling.notific.second_reminder_max);
			break;
		}
		case NOTIFIC_STATE_ON:
		{
			N_SPRINTF("[NOTIFIC] vibrator is ON, %d", t_curr);
			cling.notific.ts = t_curr;
			GPIO_vibrator_set(TRUE);
			cling.notific.state = NOTIFIC_STATE_OFF;
			break;
		}
		case NOTIFIC_STATE_OFF:
		{
			if (t_curr > (cling.notific.ts + NOTIFIC_ON_TIME_IN_MS)) {
				N_SPRINTF("[NOTIFIC] vibrator is OFF, %d", t_curr);
				GPIO_vibrator_set(FALSE);
				cling.notific.state = NOTIFIC_STATE_REPEAT;
				cling.notific.ts = t_curr;
				cling.notific.vibrate_time ++;
			}
			break;
		}
		case NOTIFIC_STATE_REPEAT:
		{
			if (t_curr > (cling.notific.ts + NOTIFIC_OFF_TIME_IN_MS)) {
				if (cling.notific.vibrate_time >= NOTIFIC_VIBRATION_REPEAT_TIME) {
					cling.notific.state = NOTIFIC_STATE_SECOND_REMINDER;
					cling.notific.second_reminder_time ++;
					cling.notific.ts = t_curr;
				} else {
					cling.notific.state = NOTIFIC_STATE_ON;
					Y_SPRINTF("[NOTIFIC] vibrator repeat, %d, %d", cling.notific.vibrate_time, t_curr);
				}
			}
			break;
		}
		case NOTIFIC_STATE_SECOND_REMINDER:
		{
			if (t_curr > (cling.notific.ts + NOTIFIC_MULTI_REMINDER_LATENCY)) {
				if (cling.notific.second_reminder_time >= cling.notific.second_reminder_max) {
					cling.notific.state = NOTIFIC_STATE_IDLE;
				} else {
					cling.notific.state = NOTIFIC_STATE_ON;
					// Reset vibration times
					cling.notific.vibrate_time = 0;
					Y_SPRINTF("[NOTIFIC] Notify second reminder - %d", cling.notific.second_reminder_time);
				}
			}
			break;
		}
		default:
			break;
	}
}

void NOTIFIC_smart_phone_notify(I8U mode, I8U id, I8U count)
{
	if (mode == NOTIFIC_SMART_PHONE_NEW) {
#ifdef _ENABLE_ANCS_
		cling.ancs.cat_count[id] = count;
#endif
		// Inform NOTIFIC state machine to notify user
		NOTIFIC_start_notifying(id);
		Y_SPRINTF("NOTIFIC - SMART PHONE @ %d, %d, %d", id, count, mode);
	} else if (mode == NOTIFIC_SMART_PHONE_DELETE) {
		NOTIFIC_stop_notifying();
#ifdef _ENABLE_ANCS_
		cling.ancs.cat_count[id] = 0;
#endif
	} else if (mode == NOTIFIC_SMART_PHONE_STOP) {
		NOTIFIC_stop_notifying();
	}
}

