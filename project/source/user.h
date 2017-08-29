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

enum {
	DISPLAY_CALORIES_TYPE_ALL = 0,
	DISPLAY_CALORIES_TYPE_ACTIVE,
	DISPLAY_CALORIES_TYPE_IDLE,
};

#define APP_DISTANCE_NORMALIZATION  0x01
			
typedef struct tagUSER_PROFILE_CTX {
	I8U name[24]; // Limit user name length to 32 bytes
	I8U name_len;
	I16U weight_in_kg;
	I16U height_in_cm;
  I16U stride_in_cm;
	I16U stride_running_in_cm;
	I16U stride_treadmill_in_cm;
	BOOLEAN metric_distance;
	I8U sleep_dow;
	// bed & wake up time
	I8U wakeup_hh;
	I8U wakeup_mm;
	I8U bed_hh;
	I8U bed_mm;
	I8U regular_page_display_1;
	I8U regular_page_display_2;
	I8U training_alert;		
	I8U app_setting;
	I8U running_rate;
	I8U running_page_display;
	I8U touch_vibration;
	I8U display_active_calories_only;
	// Mileage limit for stigama tracking
	I8U mileage_limit;
	I8U age;
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
	// User dynamic data 
	I16U calories_factor;
	
	// gesture recognition
	BOOLEAN b_screen_wrist_flip;
	BOOLEAN b_screen_press_hold_1;
	BOOLEAN b_screen_press_hold_3;
	BOOLEAN b_running_alwayson;
	
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

void USER_device_specifics_init(void);
void USER_device_specifics_setup(I8U *data, BOOLEAN b_profile, BOOLEAN b_device);
void USER_state_machine(void);
void USER_setup_profile(I8U *data);
void USER_setup_device(I8U *data);
void USER_device_get_name(I8U *string);
void USER_default_setup(void);

#endif  // __USER_HEADER__
