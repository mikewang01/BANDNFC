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

#define NOTIFIC_MULTI_REMINDER_INCOMING_CALL 3
#define NOTIFIC_MULTI_REMINDER_OTHERS         1
#define NOTIFIC_MULTI_REMINDER_IDLE_ALERT     3

void NOTIFIC_start_notifying(I8U cat_id)
{			
	// Reset vibration times
	cling.notific.vibrate_time = 0;
	// The maximum vibration time
	if (cat_id == BLE_ANCS_CATEGORY_ID_INCOMING_CALL)
		cling.notific.second_reminder_max = NOTIFIC_MULTI_REMINDER_INCOMING_CALL;
	else
		cling.notific.second_reminder_max = NOTIFIC_MULTI_REMINDER_OTHERS;
	cling.notific.cat_id = cat_id;
	cling.notific.state = NOTIFIC_STATE_SETUP_VIBRATION;
	
	Y_SPRINTF("[NOTIFIC] start notification: %d", cat_id);
	
	// Also, turn on screen
	UI_turn_on_display(UI_STATE_NOTIFIC, 3000);
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


BOOLEAN NOTIFIC_is_new_message(void)
{
	
#ifdef _ENABLE_ANCS_		
	return cling.ancs.notf_updata;
#else
	return 0;
#endif
}


I8U NOTIFIC_get_message_total(void)
{

#ifdef _ENABLE_ANCS_
	return cling.ancs.message_total;
#else
	return 0;
#endif
}


I8U NOTIFIC_get_app_name(I8U index, char *app_name)
{
#ifdef _ENABLE_ANCS_		
	I8U  title_len;
	I32U addr_in;
	I32U tmpBuf[32];
	
	I8U  *pdata = (I8U *)tmpBuf;
	
	if (index == 0xff) {
		
	  addr_in=((cling.ancs.message_total-1)*256+SYSTEM_NOTIFICATION_SPACE_START);
	
	  FLASH_Read_App(addr_in, pdata, 128);	

	  title_len= pdata[0];
		
		if (title_len >= 127)
			title_len = 127;
		
		// Get the incoming message
	  memcpy(app_name,pdata+2,title_len);	
		
	  app_name[title_len] = 0;		

		Y_SPRINTF("[NOTIFIC] incoming msg len :%d",title_len );	
		
		Y_SPRINTF("[NOTIFIC] incoming msg :%s",app_name );		
		return title_len;
	} 
	addr_in=((index*256)+SYSTEM_NOTIFICATION_SPACE_START);

	FLASH_Read_App(addr_in, pdata, 128);	
	
	title_len =  pdata[0];
	
	// Get the incoming message
	memcpy(app_name,pdata+2,title_len);	
	
	app_name[title_len] = 0;		

	Y_SPRINTF("[NOTIFIC] get ancs title len  is :%d",title_len );	
	
	Y_SPRINTF("[NOTIFIC] get ancs title string  is :%s",app_name );		
	
	return title_len;
#endif
}

I8U NOTIFIC_get_app_message_detail(I8U index, char *string)
{
#ifdef _ENABLE_ANCS_	
	I8U  title_len;
	I8U  mes_len;	
	I8U  mes_offset;
	I32U addr_in;
	I32U tmpBuf_1[32];
	I32U tmpBuf_2[32];
	
	I8U  *pdata_1 = (I8U *)tmpBuf_1;
  I8U  *pdata_2 = (I8U *)tmpBuf_2;

	addr_in=((index*256)+SYSTEM_NOTIFICATION_SPACE_START);

	FLASH_Read_App(addr_in, pdata_1, 128);	

	addr_in+=128;
		
	FLASH_Read_App(addr_in, pdata_2, 128);	

  title_len = pdata_1[0];
  mes_len   = pdata_1[1];	
	
	mes_offset=(128-title_len-2);
	
	if(mes_len > 126){
	mes_len=126;
	}

	if(mes_len  <=  mes_offset) {
		
		// Get the  message string
	  memcpy(string,pdata_1+2+title_len,mes_len);	
	  string[mes_len] = 0;		
	}
	
	else{

		// Get the  message string
	  memcpy(string,pdata_1+2+2+title_len,mes_offset);	
		memcpy(&string[mes_offset],pdata_2,(mes_len-mes_offset));			
	  string[mes_len] = 0;		
	}

	Y_SPRINTF("[NOTIFIC] get ancs message len  is :%d",mes_len );	
	Y_SPRINTF("[NOTIFIC] get ancs message string  is :%s",string );			

	return mes_len;
	
#endif
}

//if message = "Incoming Call" .......
I8U NOTIFIC_get_callerID(char *string)
{
#ifdef _ENABLE_ANCS_		
	// ....
	I8U  title_len = 0;
	I32U addr_in;
	I32U tmpBuf[32];
	
	I8U  *pdata = (I8U *)tmpBuf;

	addr_in=((cling.ancs.message_total-1)*256+SYSTEM_NOTIFICATION_SPACE_START);
	
	FLASH_Read_App(addr_in, pdata, 128);	

	title_len= pdata[0];
	
	if (title_len >= 127)
		title_len = 127;

	memcpy(string, pdata+2, title_len);		
	string[title_len] = 0;

	Y_SPRINTF("[NOTIFIC] len is :%d, %02x, %02x, %02x, %02x",title_len, pdata[0], pdata[1], pdata[2], pdata[3]);		
	Y_SPRINTF("[NOTIFIC] get notf callerID string is :%s",string );
	
	return title_len;	
#else
	string[0] = 0;
	return 0;
#endif	
}

BOOLEAN NOTIFIC_is_user_viewing_message()
{
	// 1. see if device is receiving a notification message
	if (cling.ui.state == UI_STATE_NOTIFIC)
		return TRUE;
	
	// 1. see if device is in a active display state
	if (UI_is_idle()) {
		return FALSE;
	}
	
	// 3. see if UI frame is displaying notification message
	if ((cling.ui.frame_index >= UI_DISPLAY_SMART) && (cling.ui.frame_index <= UI_DISPLAY_SMART_END))
	{
		return TRUE;
	}

	return FALSE;
}
