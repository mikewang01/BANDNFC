//
//  File: reminder.h
//  
//  Description: the header contains all reminder information
//
//  Created on Jan 6, 2014
//
#ifndef __REMINDER_HEADER__
#define __REMINDER_HEADER__

typedef enum {
	REMINDER_STATE_IDLE,
  REMINDER_STATE_ON,	
  REMINDER_STATE_CHECK_NEXT_REMINDER,	
} REMINDER_STATE_TYPE;

typedef enum {
	NORMAL_ALARM_CLOCK = 0,		
  SLEEP_ALARM_CLOCK,
  WAKEUP_ALARM_CLOCK	
} ALARM_CLOCK_TYPE;

typedef struct tagREMINDER_CTX {
	I32U ts;
	I8U hour;
	I8U minute;
	I8U b_valid;
	I8U b_sleep_valid;
	I8U b_wakeup_valid;
	I8U ui_hh;
	I8U ui_mm;
	I8U total;
	I8U b_sleep_total;	
	I8U alarm_type;
	REMINDER_STATE_TYPE state;
} REMINDER_CTX;


void REMINDER_state_machine(void);
I8U REMINDER_get_time_at_index(I8U index);
void REMINDER_setup(I8U *msg, BOOLEAN b_daily);
void REMINDER_set_next_normal_reminder(void);
void REMINDER_set_sleep_reminder(void);

#endif  // __REMINDER_HEADER__
