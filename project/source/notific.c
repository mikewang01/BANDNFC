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


#define NOTIFIC_ON_SHORT_TIME_IN_MS            40
#define NOTIFIC_ON_LONG_TIME_IN_MS             100
#define NOTIFIC_ON_VERY_LONG_TIME_IN_MS        140
#define NOTIFIC_OFF_TIME_IN_MS                 400
#define NOTIFIC_VIBRATION_REPEAT_TIME          3
#define NOTIFIC_VIBRATION_SHORT_TIME           2
#define NOTIFIC_VIBRATION_SINGLE_TIME          1

#define NOTIFIC_MULTI_REMINDER_LATENCY         2000
#define NOTIFIC_MULTI_REMINDER_TIME            15
#define NOTIFIC_MULTI_REMINDER_INCOMING_CALL   3
#define NOTIFIC_MULTI_REMINDER_OTHERS          1
#define NOTIFIC_MULTI_REMINDER_IDLE_ALERT      3
#define NOTIFIC_MULTI_REMINDER_HR              2
#define NOTIFIC_MULTI_REMINDER_10KS            3
#define NOTIFIC_MULTI_REMINDER_TRAINING_PACE   1
#define NOTIFIC_MULTI_REMINDER_TRAINING_HR     1

void NOTIFIC_stop_notifying()
{
	N_SPRINTF("[NOTIFIC] Stop notifying");
	cling.notific.state = NOTIFIC_STATE_IDLE;
	GPIO_vibrator_set(FALSE);
}

void NOTIFIC_start_notifying(I8U notif_type, I8U cat_id)
{		
	I8U notif_frame_index;
  BOOLEAN	b_valid_type = TRUE;
	
	// Do not notify user if unit is in sleep state
	if (SLEEP_is_sleep_state(SLP_STAT_SOUND) || SLEEP_is_sleep_state(SLP_STAT_LIGHT))
		return;

	// Reset vibration times
	cling.notific.vibrate_time = 0;
	
	switch (notif_type) {
		case NOTIFICATION_TYPE_MESSAGE: {
			// The maximum vibration time
			if (cat_id == BLE_ANCS_CATEGORY_ID_INCOMING_CALL) {
				cling.notific.second_reminder_max = NOTIFIC_MULTI_REMINDER_INCOMING_CALL;
				notif_frame_index = UI_DISPLAY_SMART_INCOMING_CALL;	
			} else {
				cling.notific.second_reminder_max = NOTIFIC_MULTI_REMINDER_OTHERS;
				notif_frame_index = UI_DISPLAY_SMART_INCOMING_MESSAGE;	
			}		
	    cling.notific.first_reminder_max = NOTIFIC_VIBRATION_SHORT_TIME;
	    cling.notific.vibrate_on_time = NOTIFIC_ON_SHORT_TIME_IN_MS;			
		  Y_SPRINTF("[NOTIFIC] start notification: %d", cat_id);
			break;
		}
		case NOTIFICATION_TYPE_IDLE_ALERT: {
			cling.notific.first_reminder_max = NOTIFIC_VIBRATION_SHORT_TIME;
	    cling.notific.second_reminder_max = NOTIFIC_MULTI_REMINDER_IDLE_ALERT;
	    cling.notific.vibrate_on_time = NOTIFIC_ON_SHORT_TIME_IN_MS;
	    notif_frame_index = UI_DISPLAY_SMART_IDLE_ALERT;
			Y_SPRINTF("NOTIFIC - IDLE ALERT @ %d:%d", cling.time.local.hour, cling.time.local.minute);
			break;
		}		
		case NOTIFICATION_TYPE_NORMAL_HR_ALERT: {
		  cling.notific.first_reminder_max = NOTIFIC_VIBRATION_REPEAT_TIME;
	    cling.notific.second_reminder_max = NOTIFIC_MULTI_REMINDER_HR;
	    cling.notific.vibrate_on_time = NOTIFIC_ON_SHORT_TIME_IN_MS;		
			notif_frame_index = UI_DISPLAY_SMART_HEART_RATE_ALERT;
			Y_SPRINTF("NOTIFIC - IDLE ALERT @ %d:%d", cling.time.local.hour, cling.time.local.minute);
			break;
		}
		case NOTIFICATION_TYPE_10K_STEP: {
	    cling.notific.first_reminder_max = NOTIFIC_VIBRATION_REPEAT_TIME;
	    cling.notific.second_reminder_max = NOTIFIC_MULTI_REMINDER_10KS;
	    cling.notific.vibrate_on_time = NOTIFIC_ON_VERY_LONG_TIME_IN_MS;
	    notif_frame_index = UI_DISPLAY_SMART_STEP_10K_ALERT;		
	    Y_SPRINTF("NOTIFIC - 10K step ALERT");
			break;
		}	
		case NOTIFICATION_TYPE_RUNNING_PACE_ALERT: {
			cling.notific.first_reminder_max = NOTIFIC_VIBRATION_SHORT_TIME;
			cling.notific.second_reminder_max = NOTIFIC_MULTI_REMINDER_TRAINING_PACE;
			cling.notific.vibrate_on_time = NOTIFIC_ON_VERY_LONG_TIME_IN_MS;
      if (cling.activity.workout_type == WORKOUT_RUN_OUTDOOR)
			  notif_frame_index = UI_DISPLAY_TRAINING_STAT_PACE;		
	    Y_SPRINTF("NOTIFIC - Running pace ALERT ");			
			break;
		}		
		case NOTIFICATION_TYPE_RUNNING_HR_ALERT: {
			cling.notific.first_reminder_max = NOTIFIC_VIBRATION_SHORT_TIME;
			cling.notific.second_reminder_max = NOTIFIC_MULTI_REMINDER_TRAINING_HR;
			cling.notific.vibrate_on_time = NOTIFIC_ON_VERY_LONG_TIME_IN_MS;			
      if (cling.activity.workout_type == WORKOUT_RUN_OUTDOOR)
			  notif_frame_index = UI_DISPLAY_TRAINING_STAT_HEART_RATE;		
	    Y_SPRINTF("NOTIFIC - Running heat rate ALERT ");					
			break;
		}
    default:
			b_valid_type = FALSE;
      break;			
	}
 
	if (!b_valid_type)
		return;
	
	// Start notific state machine.
	cling.notific.state = NOTIFIC_STATE_SETUP_VIBRATION;
	
	// Start ui notifying display.
  UI_start_notifying(notif_frame_index);
}

void NOTIFIC_state_machine()
{
	I32U t_curr = CLK_get_system_time();
	
	if (OTA_if_enabled()) {
		if (cling.notific.state != NOTIFIC_STATE_IDLE) {
			cling.notific.state = NOTIFIC_STATE_IDLE;
			GPIO_vibrator_set(FALSE);
		}
		return;
	} 
	
	if (cling.notific.state != NOTIFIC_STATE_IDLE) {
		RTC_start_operation_clk();
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
			N_SPRINTF("[NOTIFIC] second notif max: %d,", cling.notific.second_reminder_max);
			break;
		}
		case NOTIFIC_STATE_ON:
		{
			N_SPRINTF("[NOTIFIC] vibrator is ON, %d", t_curr);
			cling.notific.ts = t_curr;
			GPIO_vibrator_on_block(cling.notific.vibrate_on_time);
			cling.notific.state = NOTIFIC_STATE_OFF;
			break;
		}
		case NOTIFIC_STATE_OFF:
		{
			N_SPRINTF("[NOTIFIC] vibrator is OFF, %d", t_curr);
			GPIO_vibrator_set(FALSE);
			cling.notific.state = NOTIFIC_STATE_REPEAT;
			cling.notific.ts = t_curr;
			cling.notific.vibrate_time ++;
			break;
		}
		case NOTIFIC_STATE_REPEAT:
		{
			if (t_curr > (cling.notific.ts + NOTIFIC_OFF_TIME_IN_MS)) {
				if (cling.notific.vibrate_time >= cling.notific.first_reminder_max) {
					cling.notific.state = NOTIFIC_STATE_SECOND_REMINDER;
					cling.notific.second_reminder_time ++;
					cling.notific.ts = t_curr;
				} else {
					cling.notific.state = NOTIFIC_STATE_ON;
					N_SPRINTF("[NOTIFIC] vibrator repeat, %d, %d", cling.notific.vibrate_time, t_curr);
				}
			}
			break;
		}
		case NOTIFIC_STATE_SECOND_REMINDER:
		{
			if (t_curr > (cling.notific.ts + NOTIFIC_MULTI_REMINDER_LATENCY)) {
				if (cling.notific.second_reminder_time >= cling.notific.second_reminder_max) {
					cling.notific.state = NOTIFIC_STATE_IDLE;
					N_SPRINTF("[NOTIFIC] Notify second reminder over, idle - %d", cling.notific.second_reminder_time);
					GPIO_vibrator_set(FALSE);
				} else {
					cling.notific.state = NOTIFIC_STATE_ON;
					// Reset vibration times
					cling.notific.vibrate_time = 0;
					N_SPRINTF("[NOTIFIC] Notify second reminder - %d", cling.notific.second_reminder_time);
				}
			}
			break;
		}
		case NOTIFIC_STATE_STOPPED_EXTERNAL:
		{
			cling.notific.state = NOTIFIC_STATE_IDLE;
			GPIO_vibrator_set(FALSE);
			break;
		}
		default:
		{
			N_SPRINTF("[NOTIFIC] Notify wrong state - %d", cling.notific.state);
			cling.notific.state = NOTIFIC_STATE_IDLE;
			GPIO_vibrator_set(FALSE);
			break;
		}
	}
}
#ifdef _ENABLE_ANCS_
void NOTIFIC_smart_phone_notify(I8U* data)
{
#ifndef _CLING_PC_SIMULATION_	
	I8U mode = data[0];
	I8U id = data[1];
	I8U title_len, msg_len;
	
	title_len = data[2];
	msg_len = data[3];
  
	if ((title_len > ANCS_SUPPORT_MAX_TITLE_LEN) || (msg_len > ANCS_SUPPORT_MAX_MESSAGE_LEN))
		return;
	
	data[4+title_len+msg_len] = 0;

	if (mode == NOTIFIC_SMART_PHONE_NEW) {
#ifndef _CLINGBAND_PACE_MODEL_			
		 if(cling.ancs.message_total >= 16) {
				
			 FLASH_erase_App(SYSTEM_NOTIFICATION_SPACE_START);
			 
			 // Latency before writing notification message (Erasure latency: 50 ms)
			 BASE_delay_msec(50);
		 
			 N_SPRINTF("[ANCS] message is full, go erase the message space");

			 cling.ancs.message_total = 0;
		 }
		 
		cling.ancs.message_total++;		
#endif		
		Y_SPRINTF("\n[NOTIFIC] *** SMART PHONE @ %d, %d, %d, %d, %s\n", mode, id, title_len, msg_len, data+4);
	 
		// title len, message len, ....
#ifdef _CLINGBAND_PACE_MODEL_			 
		ANCS_store_one_message(data+2);
#else 
    ANCS_nflash_store_one_message(data+2);	 
#endif
		 
		// Inform NOTIFIC state machine to notify user
		NOTIFIC_start_notifying(NOTIFICATION_TYPE_MESSAGE, id);
		
	} else if (mode == NOTIFIC_SMART_PHONE_DELETE) {
		NOTIFIC_stop_notifying();
	} else if (mode == NOTIFIC_SMART_PHONE_STOP) {
		NOTIFIC_stop_notifying();
	}
#endif
}

I8U NOTIFIC_get_message_total(void)
{
	if (cling.ancs.message_total > 16)
		cling.ancs.message_total = 16;
	
	return cling.ancs.message_total;
}
#endif

I8U NOTIFIC_get_app_name(I8U index, char *app_name)
{
	I8U  title_len;
#ifdef _ENABLE_ANCS_		
#ifdef _CLINGBAND_PACE_MODEL_
	I8U *pdata = cling.ancs.buf;
	
	title_len = pdata[0];
	if (title_len > ANCS_SUPPORT_MAX_TITLE_LEN)
		title_len = ANCS_SUPPORT_MAX_TITLE_LEN;
	
	// Get the incoming message
	memcpy(app_name, pdata+2 ,title_len);	
	
	app_name[title_len] = 0;		
#else	
	I32U addr_in;
	I32U tmpBuf[32];	
	I8U  *pdata = (I8U *)tmpBuf;
	
	if ((cling.ancs.message_total == 0) || (index > (cling.ancs.message_total-1))) {
		title_len = sprintf(app_name, "No message");
		return title_len;
	}

	// Get the latest notification first.
	index = cling.ancs.message_total - 1 - index;

	// Get message title
	addr_in=((index*256)+SYSTEM_NOTIFICATION_SPACE_START);

	FLASH_Read_App(addr_in, pdata, 128);	
	
	title_len =  pdata[0];
	
	if (title_len >= ANCS_SUPPORT_MAX_TITLE_LEN)
		title_len = ANCS_SUPPORT_MAX_TITLE_LEN;
	
	// Get the incoming message
	memcpy(app_name,pdata+2,title_len);	
	
	app_name[title_len] = 0;		

	N_SPRINTF("[NOTIFIC] get ancs title len :%d",title_len );	
	
	N_SPRINTF("[NOTIFIC] get ancs title string :%s",app_name );		
#endif	
	return title_len;
#else
	
	title_len = sprintf(app_name, "No message!");
	return title_len;
#endif
}

I8U NOTIFIC_get_app_message_detail(I8U index, char *string)
{
	I8U  title_len;
#ifdef _ENABLE_ANCS_	
	I8U  msg_len;	
#ifdef _CLINGBAND_PACE_MODEL_
	I8U  *pdata = cling.ancs.buf;

	title_len = pdata[0];
	if (title_len > ANCS_SUPPORT_MAX_TITLE_LEN)
		title_len = ANCS_SUPPORT_MAX_TITLE_LEN;
	
	msg_len =  pdata[1];
	if(msg_len >= ANCS_SUPPORT_MAX_MESSAGE_REALITY_LEN)
	  msg_len = ANCS_SUPPORT_MAX_MESSAGE_REALITY_LEN;
	
	memcpy(string, pdata + title_len +2, msg_len);
	string[msg_len] = 0;
#else	
	I8U  msg_offset;
	I32U addr_in;
	I32U tmpBuf_1[32];
	I32U tmpBuf_2[32];
	I8U  *pdata_1 = (I8U *)tmpBuf_1;
  I8U  *pdata_2 = (I8U *)tmpBuf_2;
	
	if ((cling.ancs.message_total == 0) || (index > (cling.ancs.message_total-1))){
		title_len = sprintf(string, "No message!");
		return title_len;
	}
	
	// Get the latest notification first.
	index = cling.ancs.message_total - 1 - index;

	// First, read the first 128 bytes
	addr_in=((index*256)+SYSTEM_NOTIFICATION_SPACE_START);

	FLASH_Read_App(addr_in, pdata_1, 128);	

  title_len = pdata_1[0];
  msg_len   = pdata_1[1];	
	
	msg_offset = 1+1+title_len;
	
	//Currently only support 128 byte	
	if(msg_len >= ANCS_SUPPORT_MAX_MESSAGE_REALITY_LEN)
		msg_len = ANCS_SUPPORT_MAX_MESSAGE_REALITY_LEN;
	
	// if the overall length of message and title is less than 128 bytes
	if (msg_len <= (128-msg_offset)) {
		memcpy(string, pdata_1+msg_offset, msg_len);
		string[msg_len] = 0;
	} else {
		addr_in+=128;
		FLASH_Read_App(addr_in, pdata_2, 128);	
		
		// if the overall length of message and title is greater than 128 bytes
		memcpy(string, pdata_1+msg_offset, 128-msg_offset);
		memcpy(string+(128-msg_offset), pdata_2, msg_len-(128-msg_offset));
		string[msg_len] = 0;
	}
#endif	
	return msg_len;
#else

	title_len = sprintf(string, "No message!");
	return title_len;
#endif
}
