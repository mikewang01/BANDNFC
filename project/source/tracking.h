//
//  File: tracking.h
//  
//  Description: the header contains all activity info
//
//  Created on Jan 6, 2014
//
#ifndef __TRACKING_HEADER__
#define __TRACKING_HEADER__

#include "pedo_api.h"
#include "user.h"

#define STRIDE_LENGTH_NORM 100
#define INCHES_TO_MILE_CONVERSION ((I16U)63360) // 5280 * 12
#define INCHES_TO_KM_CONVERSION ((I16U)39370)
#define DISTANCE_ROUNDING_V 0 // 63360*0.05
#define CALORIE_NORM_FACTOR 60000 // 60 seconds x 1000 (the cadence calorie factor 1000)
#define ADJUSTED_STRIDE_LENGTH_DIFF 3 // 2.64 inches // 4.64 inches (30 - 25.36)

#define TRACKING_DAY_STAT_SIZE 24 // Curently, we have 20 bytes information about daily statistics
#define TRACKING_DAY_STAT_MAX  64 // 64 bytes space reserved for future extension.

#define MINUTE_STREAMING_FILE_SIZE 4096
#define MINUTE_STREAMING_FILE_ENTRIES 256

//open/close the function of wristFlip inversely to dark screen
//#define __YLF_WRIST_FLIP__

#define __YLF_STRIDE__

// Activity and Vital  type
enum {
	TRACKING_UNKNOWN,
	TRACKING_WALK,
	TRACKING_RUN,
	TRACKING_STAIRS,
	TRACKING_STEPS,
	TRACKING_ACTIVE_TIME,
	TRACKING_DISTANCE,
	TRACKING_CALORIES,
	TRACKING_TIME,
	TRACKING_SLEEP,
	TRACKING_HEART_RATE,
	TRACKING_SKIN_TEMPERATURE,
	TRACKING_SKIN_TOUCH,
	TRACKING_ACTIVITY
};

enum {
	POSITION_GPS_STATUS_READY = 0,
	POSITION_NEWORK_STATUS_READY,
	POSITION_NO_SERVICE
};
enum {
	WORKOUT_PLACE_NONE,
	WORKOUT_PLACE_INDOOR,
	WORKOUT_PLACE_OUTDOOR
};

enum {
	WORKOUT_NONE,
	WORKOUT_TREADMILL_INDOOR,  // Running indoor on treadmill
	WORKOUT_RUN_OUTDOOR,       // Running outdoor
	WORKOUT_CYCLING_INDOOR,    // Biking indoor
	WORKOUT_ELLIPTICAL_INDOOR,
	WORKOUT_STAIRS_INDOOR,
	WORKOUT_AEROBIC,
	WORKOUT_ROWING,
	WORKOUT_PILOXING,
	WORKOUT_OTHER,
	WORKOUT_CYCLING_OUTDOOR     // Cycling outdoor
};

typedef struct tagMINUTE_TRACKING_CTX {
	I32U epoch;
	I16S skin_temperature;
	I16U activity_count;
	I8U walking;
	I8U running;
	I8U calories;
	I8U distance;
	I8U sleep_state;
	I8U heart_rate;
	I8U skin_touch_pads;
	I8U uv_and_activity_type;
} MINUTE_TRACKING_CTX;

typedef struct tagMINUTE_DELTA_TRACKING_CTX {
	I8U sleep_state;
	I8U walking;
	I8U running;
	I8U distance;
	I8U calories;
	I16U activity_count;
} MINUTE_DELTA_TRACKING_CTX;

typedef struct tagDAY_TRACKING_CTX {
	I32U walking;
	I32U running;
	I32U distance;
	I32U calories;
	I32U active_time;
} DAY_TRACKING_CTX;

#define PACE_BUF_LENGTH 4

typedef struct tagRUNNING_TRACK_CTX {
	I32U steps;
	I32U distance;
	I32U calories;
	I32U accu_heart_rate;
	I16U time_min;
	I8U time_sec;
	I8U walk_per_60_second;
	I8U run_per_60_second;
	I16U last_10sec_distance;
	I16U last_d_buf[PACE_BUF_LENGTH];
	I16U last_t_buf[PACE_BUF_LENGTH];
	I8U pace_buf_idx;
	I8U last_10sec_pace_min;
	I8U last_10sec_pace_sec;
	I32U pace_calc_ts;
	I32U pace_visual_update_ts;
} RUNNING_TRACK_CTX;

typedef struct tagTRAINING_TRACK_CTX {
	I32U session_id;
	I32U distance;
	I32U speed;
	I32U time_start_in_ms;
	I32U calories;
	I8U  app_positon_service_status;
#ifndef _CLINGBAND_PACE_MODEL_
//#ifndef __YLF_CYCLING__
	BOOLEAN b_cycling_state;
	I32U cycling_curr_distance;
	I32U cycling_pre_distance;
	I32U cycling_pre_delta_distance;
#endif
#ifdef __YLF_STRIDE__
	I32U train_pre_distance;
	I32U train_curr_distance;
	I16U train_pre_walking;
	I16U train_curr_walking;
	I16U train_pre_running;
	I16U train_curr_running;
	I8U stride_running;//cm
	I8U stride_walking;//cm
	BOOLEAN b_first_walking_times;
	BOOLEAN b_first_running_times;
#endif
} TRAINING_TRACK_CTX;

typedef struct tagDAY_STREAMING_CTX {
	I32U steps;
	I32U distance;
	I16U calories_active;
	I16U calories_idle;
	I8U wake_up_time;
	I16U sleep_light;
	I16U sleep_sound;
	I16U sleep_rem;
	I8U heart_rate;
	I16S temperature;
} DAY_STREAMING_CTX;

typedef struct tagMINUTE_VITAL_CTX {
	I8U heart_rate;
	I8U skin_touch_pads;
	I16S skin_temperature;
} MINUTE_VITAL_CTX;

typedef enum {
	FACE_UNKNOWN,
	FACE_UP,
	FACE_DOWN,
	FACE_SIDE
} DEVICE_ORIENTATION_TYPE;

typedef struct tagTRACKING_CTX {

	// Daily activity info
	DAY_TRACKING_CTX day;
	
	// Minute granularity info
	DAY_TRACKING_CTX day_stored;

	// Sleep by noon
	I32U sleep_by_noon;
	I32U sleep_stored_by_noon;
	
	// motion time stamp
	I32U motion_ts;
	
	// Step time stamp
	I32U step_detect_t_sec;
	
	// Device tap timing
	I32U tap_ts_ms;
	
	// Overall length of activity for the day
	I32U tracking_flash_offset;
	I32U flash_block_1st_to_erase;
	I32U flash_block_2nd_to_erase;
	BOOLEAN b_pending_erase;
	
	// Wrist Flip Detection
	DEVICE_ORIENTATION_TYPE orientation[5];
	I8U face_up_index;
#ifdef __YLF_WRIST_FLIP__
	BOOLEAN b_screen_on_wrist_flip;
	BOOLEAN b_screen_off_wrist_flip;
#endif
	
	// workout
	I32U workout_time_stamp_start;
	I32U workout_Calories_start;
	I32U workout_Calories_acc;
	I32U workout_time_stamp_stop;
	I8U workout_type;
	I8U workout_place;
	BOOLEAN b_workout_active;
#ifndef __YLF_RUN_HR__
	I8U hr_sport_minutes;
#endif
#ifdef __YLF_ONDESK__
	BOOLEAN b_stay_on_desk;
	I32U t_stay_on_desk;
	I32U t_stay_on_desk_diff_ms;
#endif
} TRACKING_CTX;

BOOLEAN TRACKING_is_not_active(void);
void TRACKING_algorithms_proc(ACCELEROMETER_3D A);
void TRACKING_initialization(void);
void TRACKING_data_logging(void);

void TRACKING_exit_low_power_mode(BOOLEAN b_force);
void TRACKING_total_data_load_file(void);
void TRACKING_get_activity(I8U index, I8U mode, I32U *value);
I32U TRACKING_get_daily_total(DAY_TRACKING_CTX *day_total);
I32U TRACKING_get_sleep_by_noon(BOOLEAN b_previous_day);
void TRACKING_get_daily_streaming_sleep(DAY_STREAMING_CTX *day_streaming);
void TRACKING_get_daily_streaming_stat(DAY_STREAMING_CTX *day_streaming);
void TRACKING_enter_low_power_mode(void);
void TRACKING_get_cycling_stat(void);

#ifdef __YLF_STRIDE__
void GPS_distance_Init(void);
void update_stride_by_GPS_distance(void);
#endif
#endif  // __TRACKING_HEADER__
