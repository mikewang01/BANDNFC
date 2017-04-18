//
//  File: notification.h
//  
//  Description: the header contains all reminder information
//
//  Created on Jan 6, 2014
//
#ifndef __NOTIFIC_HEADER__
#define __NOTIFIC_HEADER__
#include "standard_types.h"

enum {
	NOTIFIC_SMART_PHONE_NEW,
	NOTIFIC_SMART_PHONE_DELETE,
	NOTIFIC_SMART_PHONE_STOP,
};

enum {
	NOTIFICATION_TYPE_REMINDER = 0,
	NOTIFICATION_TYPE_MESSAGE,
	NOTIFICATION_TYPE_IDLE_ALERT,
	NOTIFICATION_TYPE_NORMAL_HR_ALERT,
	NOTIFICATION_TYPE_10K_STEP,
	NOTIFICATION_TYPE_RUNNING_PACE_ALERT,	
	NOTIFICATION_TYPE_RUNNING_HR_ALERT,		
	NOTIFICATION_TYPE_NORMAL_ALARM_CLOCK,		
	NOTIFICATION_TYPE_SLEEP_ALARM_CLOCK,			
};

typedef enum {
	NOTIFIC_STATE_IDLE,
	NOTIFIC_STATE_SETUP_VIBRATION,
	NOTIFIC_STATE_ON,
	NOTIFIC_STATE_OFF,
	NOTIFIC_STATE_REPEAT,
	NOTIFIC_STATE_SECOND_REMINDER,
	NOTIFIC_STATE_STOPPED_EXTERNAL,
} NOTIFIC_STATE_TYPE;

typedef struct tagNOTIFIC_CTX {
	I32U ts;
	I8U hour;
	I8U minute;
	I8U b_valid;
	I8U vibrate_time;
	I8U second_reminder_time;
	I8U second_reminder_max;
	I8U first_reminder_max;
	I8U cat_id;
	I8U vibrate_on_time;
	NOTIFIC_STATE_TYPE state;
} NOTIFIC_CTX;


void NOTIFIC_state_machine(void);
void NOTIFIC_start_notifying(I8U notif_type,I8U cat_id);
void NOTIFIC_stop_notifying(void);
BOOLEAN NOTIFIC_is_idle(void);
void NOTIFIC_smart_phone_notify(I8U* data);
I8U NOTIFIC_get_message_total(void);
I8U NOTIFIC_get_app_name(I8U index, char *app_name);
I8U NOTIFIC_get_app_message_detail(I8U index, char *string);
#endif  // __NOTIFIC_HEADER__
