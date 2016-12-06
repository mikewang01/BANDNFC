//
//  File: user.h
//  
//  Description: USER data process header
//
//  Created on Feb 6, 2014
//
#ifndef __USER_HEADER__
#define __USER_HEADER__

enum {
	IDLE_ALERT_STATE_IDLE,
	IDLE_ALERT_STATE_COUNT_DOWN,
	IDLE_ALERT_STATE_NOTIFY,
};

enum {
	SEX_MALE,
	SEX_FEMALE
};
			
typedef struct tagUSER_PROFILE_CTX {
	I8U name[24]; // Limit user name length to 32 bytes
	I8U name_len;
	I16U weight_in_kg;
	I16U height_in_cm;
  I16U stride_in_cm;
	BOOLEAN metric_distance;
	I16U stride_running_in_cm;
	I16U stride_treadmill_in_cm;
	I8U sleep_dow;
	// bed & wake up time
	I8U wakeup_hh;
	I8U wakeup_mm;
	I8U bed_hh;
	I8U bed_mm;
	I8U regular_page_display;
	I8U running_page_display;
	I8U touch_vibration;
	// Mileage limit for stigama tracking
	I8U mileage_limit;
	I8U age;
	I8U clock_face;
	I8U sex;
	I8U max_hr_alert;
	
} USER_PROFILE_CTX;

typedef struct tagUSER_DATA {
	
	// System measurement setting
	I32U ppg_day_interval;
	I32U ppg_night_interval;
	I32U skin_temp_day_interval;
	I32U skin_temp_night_interval;
	
	// User profile
	USER_PROFILE_CTX profile;
	
	// device length
	I8U setting_len;
	I8U profile_len;
	// User dynamic data 
	I16U calories_factor;
	
	// gesture recognition
	BOOLEAN b_screen_wrist_flip;
	BOOLEAN b_screen_press_hold_1;
	BOOLEAN b_screen_press_hold_3;
	BOOLEAN b_screen_tapping;
	
	BOOLEAN b_navigation_tapping;
	BOOLEAN b_navigation_wrist_shake;
	
	// Idle alert
	I8U idle_state;
	I8U idle_time_in_minutes;
	I8U idle_minutes_countdown;
	I16U idle_step_countdown;
	I8U idle_time_start;
	I8U idle_time_end;
	
	// Screen ON time
	I8U screen_on_general;
	I8U screen_on_heart_rate;
	
	// Reminder OFF during weekends
	BOOLEAN b_reminder_off_weekends;
	
	// Pedometer sensitivity mode
	I8U m_pedo_sensitivity;

} USER_DATA;

void USER_data_init(void);
void USER_profile_update(void);
void USER_setup_device(I8U *data, I8U setting_length);
void USER_state_machine(void);
void USER_store_device_param(I8U *data);
void USER_setup_profile(I8U *data, I8U len);

#endif  // __USER_HEADER__
