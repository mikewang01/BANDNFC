/***************************************************************************//**
 * File ui.h
 * 
 * Description: User Interface header
 *
 *
 ******************************************************************************/

#ifndef __UI_HEADER__
#define __UI_HEADER__

#define FRAME_DEPTH_STEP                              7
#define FRAME_DEPTH_DISTANCE                          7
#define FRAME_DEPTH_CALORIES                          7
#define FRAME_DEPTH_ACTIVE_TIME                       7

#define UI_STORE_NOTIFICATION_MAX_TIME_IN_MS          60000
#define UI_STORE_FRAME_MAX_TIME_IN_MS                 300000
#define UI_TOUCH_VIBRATION_ON_TIME_IN_MS              15
#define UI_LOOK_MESSAGE_MAX_REPEAT_TIME               1
#define UI_LOOK_ALARM_CLOCK_REMINDER_MAX_REPEAT_TIME  2
#define UI_LOOK_IDLE_ALERT_MAX_REPEAT_TIME            2
#define UI_LOOK_HEART_RATE_ALERT_MAX_REPEAT_TIME      2
#define UI_LOOK_STEP_10K_ALERT_MAX_REPEAT_TIME        2

/** @brief UI regular page filtering.
 *
 * @details 16 bits:
 * regular_page_display_1(1B) + regular_page_display_2(1B) in user_data->profile.
 * ------------|--------------|--------------|---------------|------------------
 * 1. UV Band  | 2. NFC Band  | 3. VOC Band  | 4. Cling Pace | 5. Lemon 2 pay       
 */
// 1. UV Band regular page filtering.
#ifdef _CLINGBAND_UV_MODEL_
#define UI_FRAME_ENABLE_STEP                       0x0001
#define UI_FRAME_ENABLE_DISTANCE                   0x0002
#define UI_FRAME_ENABLE_CALORIES                   0x0004
#define UI_FRAME_ENABLE_ACTIVE_TIME                0x0008
#define UI_FRAME_ENABLE_HEART_RATE                 0x0010
#define UI_FRAME_ENABLE_SKIN_TEMP                  0x0400
#define UI_FRAME_ENABLE_UV_INDEX                   0x0800
#define UI_FRAME_ENABLE_HOME                       0xFFFF
#endif

// 2. NFC Band regular page filtering.
#ifdef _CLINGBAND_NFC_MODEL_
#define UI_FRAME_ENABLE_STEP                       0x0001
#define UI_FRAME_ENABLE_DISTANCE                   0x0002
#define UI_FRAME_ENABLE_CALORIES                   0x0004
#define UI_FRAME_ENABLE_ACTIVE_TIME                0x0008
#define UI_FRAME_ENABLE_HEART_RATE                 0x0010
#define UI_FRAME_ENABLE_SKIN_TEMP                  0x0400
#define UI_FRAME_ENABLE_HOME                       0xFFFF
#endif

// 3. VOC Band regular page filtering.
#ifdef _CLINGBAND_VOC_MODEL_
#define UI_FRAME_ENABLE_STEP                       0x0001
#define UI_FRAME_ENABLE_DISTANCE                   0x0002
#define UI_FRAME_ENABLE_CALORIES                   0x0004
#define UI_FRAME_ENABLE_ACTIVE_TIME                0x0008
#define UI_FRAME_ENABLE_HEART_RATE                 0x0010
#define UI_FRAME_ENABLE_SKIN_TEMP                  0x0400
#define UI_FRAME_ENABLE_HOME                       0xFFFF
#endif

// 4. Cling Pace regular page filtering.
#ifdef _CLINGBAND_PACE_MODEL_
#define UI_FRAME_ENABLE_STEP                       0x0001
#define UI_FRAME_ENABLE_DISTANCE                   0x0002
#define UI_FRAME_ENABLE_CALORIES                   0x0004
#define UI_FRAME_ENABLE_ACTIVE_TIME                0x0008
#define UI_FRAME_ENABLE_HEART_RATE                 0x0010
#define UI_FRAME_ENABLE_WEATHER                    0x0020
#define UI_FRAME_ENABLE_PM2P5                      0x0040
#define UI_FRAME_ENABLE_START_RUN                  0x0100
#define UI_FRAME_ENABLE_RUN_ANALYSIS               0x0200
#define UI_FRAME_ENABLE_HOME                       0xFFFF
#endif

// 5. Lemon 2 pay regular page filtering.
#ifdef _CLINGBAND_2_PAY_MODEL_
#define UI_FRAME_ENABLE_STEP                       0x0001
#define UI_FRAME_ENABLE_DISTANCE                   0x0002
#define UI_FRAME_ENABLE_CALORIES                   0x0004
#define UI_FRAME_ENABLE_ACTIVE_TIME                0x0008
#define UI_FRAME_ENABLE_HEART_RATE                 0x0010
#define UI_FRAME_ENABLE_HOME                       0xFFFF
#endif

/** @brief  UI running analysis page filtering .
 *
 * @details 8 bits:
 * running_page_display(1B) in user_data->profile.   
 */
 // UV Band  +  NFC Band + VOC Band + Lemon 2 pay running analysis page filtering.
#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)	|| defined(_CLINGBAND_2_PAY_MODEL_)
#define UI_FRAME_ENABLE_RUNNING_DISTANCE           0x01
#define UI_FRAME_ENABLE_RUNNING_TIME               0x02
#define UI_FRAME_ENABLE_RUNNING_PACE               0x04
#define UI_FRAME_ENABLE_RUNNING_STRIDE             0x08
#define UI_FRAME_ENABLE_RUNNING_CADENCE            0x10
#define UI_FRAME_ENABLE_RUNNING_HEART_RATE         0x20
#define UI_FRAME_ENABLE_RUNNING_CALORIES           0x40
#define UI_FRAME_ENABLE_RUNNING_ALL                0xFF
#endif
 
// Cling Pace running analysis page filtering.
#ifdef _CLINGBAND_PACE_MODEL_
#define UI_FRAME_ENABLE_RUNNING_DISTANCE           0x01
#define UI_FRAME_ENABLE_RUNNING_TIME               0x02
#define UI_FRAME_ENABLE_RUNNING_PACE               0x04
#define UI_FRAME_ENABLE_RUNNING_STRIDE             0x08
#define UI_FRAME_ENABLE_RUNNING_CADENCE            0x10
#define UI_FRAME_ENABLE_RUNNING_HEART_RATE         0x20
#define UI_FRAME_ENABLE_RUNNING_CALORIES           0x40
#define UI_FRAME_ENABLE_RUNNING_STOP_ANALYSIS      0x80
#define UI_FRAME_ENABLE_RUNNING_ALL                0xFF
#endif

/** @brief UI page display list.
 *
 * ------------|--------------|--------------|---------------|------------------
 * 1. UV Band  | 2. NFC Band  | 3. VOC Band  | 4. Cling Pace | 5. Lemon 2 pay       
 */
// 1. UV Band UI page display list.
#ifdef _CLINGBAND_UV_MODEL_
enum {
	// 0: Home
	UI_DISPLAY_HOME = 0, 
	// 1: System
	UI_DISPLAY_SYSTEM, 
	UI_DISPLAY_SYSTEM_RESTART = UI_DISPLAY_SYSTEM, //1		
	UI_DISPLAY_SYSTEM_OTA, //2	
	UI_DISPLAY_SYSTEM_LINKING, //3	
	UI_DISPLAY_SYSTEM_UNAUTHORIZED, //4		
	UI_DISPLAY_SYSTEM_BATT_POWER, //5	
	UI_DISPLAY_SYSTEM_END = UI_DISPLAY_SYSTEM_BATT_POWER,
	// 2: a set of the typical use cases
	UI_DISPLAY_TRACKER,
	UI_DISPLAY_TRACKER_STEP = UI_DISPLAY_TRACKER, //6		
	UI_DISPLAY_TRACKER_DISTANCE, //7
	UI_DISPLAY_TRACKER_CALORIES, //8 
	UI_DISPLAY_TRACKER_ACTIVE_TIME, //9	
	UI_DISPLAY_TRACKER_UV_IDX, //10		
	UI_DISPLAY_TRACKER_END = UI_DISPLAY_TRACKER_UV_IDX,
	// 3: a set of smart features
	UI_DISPLAY_SMART,
	UI_DISPLAY_SMART_PM2P5 = UI_DISPLAY_SMART, //11
	UI_DISPLAY_SMART_WEATHER, //12
  UI_DISPLAY_SMART_MESSAGE, //13
  UI_DISPLAY_SMART_APP_NOTIF, //14
	UI_DISPLAY_SMART_DETAIL_NOTIF, //15	
	UI_DISPLAY_SMART_INCOMING_CALL, //16
	UI_DISPLAY_SMART_INCOMING_MESSAGE, //17	
	UI_DISPLAY_SMART_ALARM_CLOCK_REMINDER, //18
	UI_DISPLAY_SMART_ALARM_CLOCK_DETAIL, //19		
	UI_DISPLAY_SMART_IDLE_ALERT, //20
	UI_DISPLAY_SMART_HEART_RATE_ALERT, //21		
	UI_DISPLAY_SMART_STEP_10K_ALERT, //22		
  UI_DISPLAY_SMART_SOS_ALERT,	//23
	UI_DISPLAY_SMART_END = UI_DISPLAY_SMART_SOS_ALERT,	
	// 4: a set of VITAL
	UI_DISPLAY_VITAL,
	UI_DISPLAY_VITAL_HEART_RATE = UI_DISPLAY_VITAL, //23	
	UI_DISPLAY_VITAL_SKIN_TEMP, //24
	UI_DISPLAY_VITAL_END = UI_DISPLAY_VITAL_SKIN_TEMP,
	// 5: Special back panel mode
	UI_DISPLAY_SETTING,
	UI_DISPLAY_SETTING_VER = UI_DISPLAY_SETTING, //25	
	UI_DISPLAY_SETTING_END = UI_DISPLAY_SETTING_VER,
	// 6: Stopwatch
	UI_DISPLAY_STOPWATCH,
	UI_DISPLAY_STOPWATCH_START = UI_DISPLAY_STOPWATCH,	//26	
	UI_DISPLAY_STOPWATCH_STOP, //27	                      
	UI_DISPLAY_STOPWATCH_END = UI_DISPLAY_STOPWATCH_STOP,
	// 7: Workout mode
	UI_DISPLAY_WORKOUT,
	UI_DISPLAY_WORKOUT_TREADMILL= UI_DISPLAY_WORKOUT, //28	
	UI_DISPLAY_WORKOUT_CYCLING, //29	
	UI_DISPLAY_WORKOUT_STAIRS, //30	
	UI_DISPLAY_WORKOUT_ELLIPTICAL, //31	
	UI_DISPLAY_WORKOUT_ROW, //32	
	UI_DISPLAY_WORKOUT_AEROBIC, //33	
	UI_DISPLAY_WORKOUT_PILOXING, //34	
	UI_DISPLAY_WORKOUT_OTHERS, //35	
	UI_DISPLAY_WORKOUT_RT_READY, //36		
	UI_DISPLAY_WORKOUT_RT_TIME,	//37		
	UI_DISPLAY_WORKOUT_RT_HEART_RATE, //38	
	UI_DISPLAY_WORKOUT_RT_CALORIES,	//39		
	UI_DISPLAY_WORKOUT_RT_END,	//40		
	UI_DISPLAY_WORKOUT_END = UI_DISPLAY_WORKOUT_RT_END,
	// 8: Running Statistics
	UI_DISPLAY_RUNNING_STATATISTICS,	
	UI_DISPLAY_RUNNING_STAT_DISTANCE = UI_DISPLAY_RUNNING_STATATISTICS, //41   
	UI_DISPLAY_RUNNING_STAT_TIME, //42
	UI_DISPLAY_RUNNING_STAT_PACE, //43
	UI_DISPLAY_RUNNING_STAT_STRIDE, //44
	UI_DISPLAY_RUNNING_STAT_CADENCE, //45
	UI_DISPLAY_RUNNING_STAT_HEART_RATE, //46
	UI_DISPLAY_RUNNING_STAT_CALORIES, //47
	UI_DISPLAY_RUNNING_STATATISTICS_END = UI_DISPLAY_RUNNING_STAT_CALORIES,
	// 9: Training Statistics
	UI_DISPLAY_TRAINING_STATATISTICS,
  UI_DISPLAY_TRAINING_STAT_START = UI_DISPLAY_TRAINING_STATATISTICS, //48    
 	UI_DISPLAY_TRAINING_STAT_START_OR_ANALYSIS, //49	
  UI_DISPLAY_TRAINING_STAT_READY, //50    
	UI_DISPLAY_TRAINING_STAT_TIME, //51	
	UI_DISPLAY_TRAINING_STAT_DISTANCE, //52
	UI_DISPLAY_TRAINING_STAT_PACE, //53	
	UI_DISPLAY_TRAINING_STAT_HEART_RATE, //54	
	UI_DISPLAY_TRAINING_STAT_RUN_STOP,	//55	
	UI_DISPLAY_TRAINING_STATATISTICS_END = UI_DISPLAY_TRAINING_STAT_RUN_STOP,
	// 10: Cycling Outdoor Statistics
	UI_DISPLAY_CYCLING_OUTDOOR_STATATISTICS,
  UI_DISPLAY_CYCLING_OUTDOOR_STAT_START = UI_DISPLAY_CYCLING_OUTDOOR_STATATISTICS, //56     	
  UI_DISPLAY_CYCLING_OUTDOOR_STAT_READY, //57  
	UI_DISPLAY_CYCLING_OUTDOOR_STAT_TIME, //58	
	UI_DISPLAY_CYCLING_OUTDOOR_STAT_DISTANCE, //59  
	UI_DISPLAY_CYCLING_OUTDOOR_STAT_SPEED, //60	
	UI_DISPLAY_CYCLING_OUTDOOR_STAT_HEART_RATE, //61	
	UI_DISPLAY_CYCLING_OUTDOOR_STAT_STOP,	//62	
	UI_DISPLAY_CYCLING_OUTDOOR_STATATISTICS_END = UI_DISPLAY_CYCLING_OUTDOOR_STAT_STOP,	
	// 11: CAROUSEL
	UI_DISPLAY_CAROUSEL,
	UI_DISPLAY_CAROUSEL_1 = UI_DISPLAY_CAROUSEL, // Running mode + Riding mode + Workout mode  //63	
	UI_DISPLAY_CAROUSEL_2, // Message + Stopwatch + Weather   //64	
	UI_DISPLAY_CAROUSEL_3, // PM2.5 + Alarm + Setting       //65	
	UI_DISPLAY_CAROUSEL_END = UI_DISPLAY_CAROUSEL_3,
	// 13: Remembered
	UI_DISPLAY_PREVIOUS,
	// The end
	UI_DISPLAY_MAXIMUM,
};
#endif

// 2. NFC Band UI page display list.
#ifdef _CLINGBAND_NFC_MODEL_
enum {
	// 0: Home
	UI_DISPLAY_HOME = 0, 
	// 1: System
	UI_DISPLAY_SYSTEM, 
	UI_DISPLAY_SYSTEM_RESTART = UI_DISPLAY_SYSTEM, //1		
	UI_DISPLAY_SYSTEM_OTA, //2	
	UI_DISPLAY_SYSTEM_LINKING, //3	
	UI_DISPLAY_SYSTEM_UNAUTHORIZED, //4		
	UI_DISPLAY_SYSTEM_BATT_POWER, //5	
	UI_DISPLAY_SYSTEM_END = UI_DISPLAY_SYSTEM_BATT_POWER,
	// 2: a set of the typical use cases
	UI_DISPLAY_TRACKER,
	UI_DISPLAY_TRACKER_STEP = UI_DISPLAY_TRACKER, //6		
	UI_DISPLAY_TRACKER_DISTANCE, //7
	UI_DISPLAY_TRACKER_CALORIES, //8 
	UI_DISPLAY_TRACKER_ACTIVE_TIME, //9	
	UI_DISPLAY_TRACKER_END = UI_DISPLAY_TRACKER_ACTIVE_TIME,
	// 3: a set of smart features
	UI_DISPLAY_SMART,
	UI_DISPLAY_SMART_PM2P5 = UI_DISPLAY_SMART, //10
	UI_DISPLAY_SMART_WEATHER, //11
  UI_DISPLAY_SMART_MESSAGE, //12
  UI_DISPLAY_SMART_APP_NOTIF, //13
	UI_DISPLAY_SMART_DETAIL_NOTIF, //14	
	UI_DISPLAY_SMART_INCOMING_CALL, //15
	UI_DISPLAY_SMART_INCOMING_MESSAGE, //16	
	UI_DISPLAY_SMART_ALARM_CLOCK_REMINDER, //17
	UI_DISPLAY_SMART_ALARM_CLOCK_DETAIL, //18		
	UI_DISPLAY_SMART_IDLE_ALERT, //19
	UI_DISPLAY_SMART_HEART_RATE_ALERT, //20		
	UI_DISPLAY_SMART_STEP_10K_ALERT, //21			
  UI_DISPLAY_SMART_SOS_ALERT, //22		
	UI_DISPLAY_SMART_END = UI_DISPLAY_SMART_SOS_ALERT,	
	// 4: a set of VITAL
	UI_DISPLAY_VITAL,
	UI_DISPLAY_VITAL_HEART_RATE = UI_DISPLAY_VITAL, //22	
	UI_DISPLAY_VITAL_SKIN_TEMP, //23
	UI_DISPLAY_VITAL_END = UI_DISPLAY_VITAL_SKIN_TEMP,
	// 5: Special back panel mode
	UI_DISPLAY_SETTING,
	UI_DISPLAY_SETTING_VER = UI_DISPLAY_SETTING, //24	
	UI_DISPLAY_SETTING_END = UI_DISPLAY_SETTING_VER,
	// 6: Stopwatch
	UI_DISPLAY_STOPWATCH,
	UI_DISPLAY_STOPWATCH_START = UI_DISPLAY_STOPWATCH,	//25	
	UI_DISPLAY_STOPWATCH_STOP, //26	                      
	UI_DISPLAY_STOPWATCH_END = UI_DISPLAY_STOPWATCH_STOP,
	// 7: Workout mode
	UI_DISPLAY_WORKOUT,
	UI_DISPLAY_WORKOUT_TREADMILL= UI_DISPLAY_WORKOUT, //27	
	UI_DISPLAY_WORKOUT_CYCLING, //28	
	UI_DISPLAY_WORKOUT_STAIRS, //29	
	UI_DISPLAY_WORKOUT_ELLIPTICAL, //30	
	UI_DISPLAY_WORKOUT_ROW, //31	
	UI_DISPLAY_WORKOUT_AEROBIC, //32	
	UI_DISPLAY_WORKOUT_PILOXING, //33	
	UI_DISPLAY_WORKOUT_OTHERS, //34	
	UI_DISPLAY_WORKOUT_RT_READY, //35		
	UI_DISPLAY_WORKOUT_RT_TIME,	//36		
	UI_DISPLAY_WORKOUT_RT_HEART_RATE, //37	
	UI_DISPLAY_WORKOUT_RT_CALORIES,	//38		
	UI_DISPLAY_WORKOUT_RT_END,	//39		
	UI_DISPLAY_WORKOUT_END = UI_DISPLAY_WORKOUT_RT_END,
	// 8: Running Statistics
	UI_DISPLAY_RUNNING_STATATISTICS,	
	UI_DISPLAY_RUNNING_STAT_DISTANCE = UI_DISPLAY_RUNNING_STATATISTICS, //40   
	UI_DISPLAY_RUNNING_STAT_TIME, //41
	UI_DISPLAY_RUNNING_STAT_PACE, //42
	UI_DISPLAY_RUNNING_STAT_STRIDE, //43
	UI_DISPLAY_RUNNING_STAT_CADENCE, //44
	UI_DISPLAY_RUNNING_STAT_HEART_RATE, //45
	UI_DISPLAY_RUNNING_STAT_CALORIES, //46
	UI_DISPLAY_RUNNING_STATATISTICS_END = UI_DISPLAY_RUNNING_STAT_CALORIES,
	// 9: Training Statistics
	UI_DISPLAY_TRAINING_STATATISTICS,
  UI_DISPLAY_TRAINING_STAT_START = UI_DISPLAY_TRAINING_STATATISTICS, //47    
 	UI_DISPLAY_TRAINING_STAT_START_OR_ANALYSIS, //48	
  UI_DISPLAY_TRAINING_STAT_READY, //49    
	UI_DISPLAY_TRAINING_STAT_TIME, //50	
	UI_DISPLAY_TRAINING_STAT_DISTANCE, //51
	UI_DISPLAY_TRAINING_STAT_PACE, //52	
	UI_DISPLAY_TRAINING_STAT_HEART_RATE, //53	
	UI_DISPLAY_TRAINING_STAT_RUN_STOP,	//54	
	UI_DISPLAY_TRAINING_STATATISTICS_END = UI_DISPLAY_TRAINING_STAT_RUN_STOP,
	// 10: Cycling Outdoor Statistics
	UI_DISPLAY_CYCLING_OUTDOOR_STATATISTICS,
  UI_DISPLAY_CYCLING_OUTDOOR_STAT_START = UI_DISPLAY_CYCLING_OUTDOOR_STATATISTICS, //55     	
  UI_DISPLAY_CYCLING_OUTDOOR_STAT_READY, //56  
	UI_DISPLAY_CYCLING_OUTDOOR_STAT_TIME, //57	
	UI_DISPLAY_CYCLING_OUTDOOR_STAT_DISTANCE, //58  
	UI_DISPLAY_CYCLING_OUTDOOR_STAT_SPEED, //59	
	UI_DISPLAY_CYCLING_OUTDOOR_STAT_HEART_RATE, //60	
	UI_DISPLAY_CYCLING_OUTDOOR_STAT_STOP,	//61	
	UI_DISPLAY_CYCLING_OUTDOOR_STATATISTICS_END = UI_DISPLAY_CYCLING_OUTDOOR_STAT_STOP,	
	// 11: CAROUSEL
	UI_DISPLAY_CAROUSEL,
	UI_DISPLAY_CAROUSEL_1 = UI_DISPLAY_CAROUSEL, // Running mode + Riding mode + Workout mode  //62	
	UI_DISPLAY_CAROUSEL_2, // Message + Stopwatch + Weather   //63	
	UI_DISPLAY_CAROUSEL_3, // PM2.5 + Alarm + Setting       //64	
	UI_DISPLAY_CAROUSEL_END = UI_DISPLAY_CAROUSEL_3,
	// 13: Remembered
	UI_DISPLAY_PREVIOUS,
	// The end
	UI_DISPLAY_MAXIMUM,
};
#endif

// 3. VOC Band UI page display list.(To do ......)
#ifdef _CLINGBAND_VOC_MODEL_
enum {
	// 0: Home
	UI_DISPLAY_HOME = 0, 
	// 1: System
	UI_DISPLAY_SYSTEM, 
	UI_DISPLAY_SYSTEM_RESTART = UI_DISPLAY_SYSTEM, //1		
	UI_DISPLAY_SYSTEM_OTA, //2	
	UI_DISPLAY_SYSTEM_LINKING, //3	
	UI_DISPLAY_SYSTEM_UNAUTHORIZED, //4		
	UI_DISPLAY_SYSTEM_BATT_POWER, //5	
	UI_DISPLAY_SYSTEM_END = UI_DISPLAY_SYSTEM_BATT_POWER,
	// 2: a set of the typical use cases
	UI_DISPLAY_TRACKER,
	UI_DISPLAY_TRACKER_STEP = UI_DISPLAY_TRACKER, //6		
	UI_DISPLAY_TRACKER_DISTANCE, //7
	UI_DISPLAY_TRACKER_CALORIES, //8 
	UI_DISPLAY_TRACKER_ACTIVE_TIME, //9	
	UI_DISPLAY_TRACKER_END = UI_DISPLAY_TRACKER_UV_IDX,
	// 3: a set of smart features
	UI_DISPLAY_SMART,
	UI_DISPLAY_SMART_PM2P5 = UI_DISPLAY_SMART, //10
	UI_DISPLAY_SMART_WEATHER, //11
  UI_DISPLAY_SMART_MESSAGE, //12
  UI_DISPLAY_SMART_APP_NOTIF, //13
	UI_DISPLAY_SMART_DETAIL_NOTIF, //14	
	UI_DISPLAY_SMART_INCOMING_CALL, //15
	UI_DISPLAY_SMART_INCOMING_MESSAGE, //16	
	UI_DISPLAY_SMART_ALARM_CLOCK_REMINDER, //17
	UI_DISPLAY_SMART_ALARM_CLOCK_DETAIL, //18		
	UI_DISPLAY_SMART_IDLE_ALERT, //19
	UI_DISPLAY_SMART_HEART_RATE_ALERT, //20		
	UI_DISPLAY_SMART_STEP_10K_ALERT, //21		
  UI_DISPLAY_SMART_SOS_ALERT, //22		
	UI_DISPLAY_SMART_END = UI_DISPLAY_SMART_SOS_ALERT,	
	// 4: a set of VITAL
	UI_DISPLAY_VITAL,
	UI_DISPLAY_VITAL_HEART_RATE = UI_DISPLAY_VITAL, //21	
	UI_DISPLAY_VITAL_SKIN_TEMP, //22
	UI_DISPLAY_VITAL_END = UI_DISPLAY_VITAL_SKIN_TEMP,
	// 5: Special back panel mode
	UI_DISPLAY_SETTING,
	UI_DISPLAY_SETTING_VER = UI_DISPLAY_SETTING, //23	
	UI_DISPLAY_SETTING_END = UI_DISPLAY_SETTING_VER,
	// 6: Stopwatch
	UI_DISPLAY_STOPWATCH,
	UI_DISPLAY_STOPWATCH_START = UI_DISPLAY_STOPWATCH,	//24	
	UI_DISPLAY_STOPWATCH_STOP, //25	                      
	UI_DISPLAY_STOPWATCH_END = UI_DISPLAY_STOPWATCH_STOP,
	// 7: Workout mode
	UI_DISPLAY_WORKOUT,
	UI_DISPLAY_WORKOUT_TREADMILL= UI_DISPLAY_WORKOUT, //26	
	UI_DISPLAY_WORKOUT_CYCLING, //27	
	UI_DISPLAY_WORKOUT_STAIRS, //28	
	UI_DISPLAY_WORKOUT_ELLIPTICAL, //29	
	UI_DISPLAY_WORKOUT_ROW, //30	
	UI_DISPLAY_WORKOUT_AEROBIC, //31	
	UI_DISPLAY_WORKOUT_PILOXING, //32	
	UI_DISPLAY_WORKOUT_OTHERS, //33	
	UI_DISPLAY_WORKOUT_RT_READY, //34		
	UI_DISPLAY_WORKOUT_RT_TIME,	//35		
	UI_DISPLAY_WORKOUT_RT_HEART_RATE, //36	
	UI_DISPLAY_WORKOUT_RT_CALORIES,	//37		
	UI_DISPLAY_WORKOUT_RT_END,	//38		
	UI_DISPLAY_WORKOUT_END = UI_DISPLAY_WORKOUT_RT_END,
	// 8: Running Statistics
	UI_DISPLAY_RUNNING_STATATISTICS,	
	UI_DISPLAY_RUNNING_STAT_DISTANCE = UI_DISPLAY_RUNNING_STATATISTICS, //39   
	UI_DISPLAY_RUNNING_STAT_TIME, //40
	UI_DISPLAY_RUNNING_STAT_PACE, //41
	UI_DISPLAY_RUNNING_STAT_STRIDE, //42
	UI_DISPLAY_RUNNING_STAT_CADENCE, //43
	UI_DISPLAY_RUNNING_STAT_HEART_RATE, //44
	UI_DISPLAY_RUNNING_STAT_CALORIES, //45
	UI_DISPLAY_RUNNING_STATATISTICS_END = UI_DISPLAY_RUNNING_STAT_CALORIES,
	// 9: Training Statistics
	UI_DISPLAY_TRAINING_STATATISTICS,
  UI_DISPLAY_TRAINING_STAT_START = UI_DISPLAY_TRAINING_STATATISTICS, //46    
 	UI_DISPLAY_TRAINING_STAT_START_OR_ANALYSIS, //47	
  UI_DISPLAY_TRAINING_STAT_READY, //48    
	UI_DISPLAY_TRAINING_STAT_TIME, //49	
	UI_DISPLAY_TRAINING_STAT_DISTANCE, //50
	UI_DISPLAY_TRAINING_STAT_PACE, //51	
	UI_DISPLAY_TRAINING_STAT_HEART_RATE, //52	
	UI_DISPLAY_TRAINING_STAT_RUN_STOP,	//53	
	UI_DISPLAY_TRAINING_STATATISTICS_END = UI_DISPLAY_TRAINING_STAT_RUN_STOP,
	// 10: Cycling Outdoor Statistics
	UI_DISPLAY_CYCLING_OUTDOOR_STATATISTICS,
  UI_DISPLAY_CYCLING_OUTDOOR_STAT_START = UI_DISPLAY_CYCLING_OUTDOOR_STATATISTICS, //54     	
  UI_DISPLAY_CYCLING_OUTDOOR_STAT_READY, //55  
	UI_DISPLAY_CYCLING_OUTDOOR_STAT_TIME, //56	
	UI_DISPLAY_CYCLING_OUTDOOR_STAT_DISTANCE, //57  
	UI_DISPLAY_CYCLING_OUTDOOR_STAT_SPEED, //58	
	UI_DISPLAY_CYCLING_OUTDOOR_STAT_HEART_RATE, //59	
	UI_DISPLAY_CYCLING_OUTDOOR_STAT_STOP,	//60	
	UI_DISPLAY_CYCLING_OUTDOOR_STATATISTICS_END = UI_DISPLAY_CYCLING_OUTDOOR_STAT_STOP,	
	// 11: CAROUSEL
	UI_DISPLAY_CAROUSEL,
	UI_DISPLAY_CAROUSEL_1 = UI_DISPLAY_CAROUSEL, // Running mode + Riding mode + Workout mode  //61	
	UI_DISPLAY_CAROUSEL_2, // Message + Stopwatch + Weather   //62	
	UI_DISPLAY_CAROUSEL_3, // PM2.5 + Alarm + Setting       //63	
	UI_DISPLAY_CAROUSEL_END = UI_DISPLAY_CAROUSEL_3,
	// 13: Remembered
	UI_DISPLAY_PREVIOUS,
	// The end
	UI_DISPLAY_MAXIMUM,
};
#endif

// 4. Cling Pace UI page display list.
#ifdef _CLINGBAND_PACE_MODEL_
enum {
	// 0: Home
	UI_DISPLAY_HOME = 0, 
	// 1: System
	UI_DISPLAY_SYSTEM, 
	UI_DISPLAY_SYSTEM_RESTART = UI_DISPLAY_SYSTEM, //1		
	UI_DISPLAY_SYSTEM_OTA, //2	
	UI_DISPLAY_SYSTEM_LINKING, //3	
	UI_DISPLAY_SYSTEM_UNAUTHORIZED, //4		
	UI_DISPLAY_SYSTEM_BATT_POWER, //5	
	UI_DISPLAY_SYSTEM_END = UI_DISPLAY_SYSTEM_BATT_POWER,
	// 2: a set of the typical use cases
	UI_DISPLAY_TRACKER,
	UI_DISPLAY_TRACKER_STEP = UI_DISPLAY_TRACKER, //6		
	UI_DISPLAY_TRACKER_DISTANCE, //7
	UI_DISPLAY_TRACKER_CALORIES, //8 
	UI_DISPLAY_TRACKER_ACTIVE_TIME, //9	   
	UI_DISPLAY_TRACKER_END = UI_DISPLAY_TRACKER_ACTIVE_TIME,
	// 3: a set of smart features
	UI_DISPLAY_SMART,
	UI_DISPLAY_SMART_PM2P5 = UI_DISPLAY_SMART, //10
	UI_DISPLAY_SMART_WEATHER, //11
	UI_DISPLAY_SMART_DETAIL_NOTIF, //14	
	UI_DISPLAY_SMART_INCOMING_CALL, //12
	UI_DISPLAY_SMART_INCOMING_MESSAGE, //13
	UI_DISPLAY_SMART_ALARM_CLOCK_REMINDER, //15
	UI_DISPLAY_SMART_IDLE_ALERT, //16	
	UI_DISPLAY_SMART_HEART_RATE_ALERT, //17		
	UI_DISPLAY_SMART_STEP_10K_ALERT, //18			
	UI_DISPLAY_SMART_END = UI_DISPLAY_SMART_STEP_10K_ALERT,
	// 4: a set of VITAL
	UI_DISPLAY_VITAL,
	UI_DISPLAY_VITAL_HEART_RATE = UI_DISPLAY_VITAL, //19
	UI_DISPLAY_VITAL_END = UI_DISPLAY_VITAL_HEART_RATE,
	// 5: Running Statistics
	UI_DISPLAY_RUNNING_STATATISTICS,
  UI_DISPLAY_RUNNING_STAT_RUN_ANALYSIS = UI_DISPLAY_RUNNING_STATATISTICS, //20     	
	UI_DISPLAY_RUNNING_STAT_DISTANCE, //21
	UI_DISPLAY_RUNNING_STAT_TIME, //22
	UI_DISPLAY_RUNNING_STAT_PACE, //23
	UI_DISPLAY_RUNNING_STAT_STRIDE, //24
	UI_DISPLAY_RUNNING_STAT_CADENCE, //25
	UI_DISPLAY_RUNNING_STAT_HEART_RATE, //26
	UI_DISPLAY_RUNNING_STAT_CALORIES, //27
  UI_DISPLAY_RUNNING_STAT_STOP_ANALYSIS, //28
	UI_DISPLAY_RUNNING_STATATISTICS_END = UI_DISPLAY_RUNNING_STAT_STOP_ANALYSIS,
	// 6: training Statistics
	UI_DISPLAY_TRAINING_STATATISTICS,
  UI_DISPLAY_TRAINING_STAT_START = UI_DISPLAY_TRAINING_STATATISTICS, //29     	
  UI_DISPLAY_TRAINING_STAT_READY, //30    
	UI_DISPLAY_TRAINING_STAT_TIME, //31	
	UI_DISPLAY_TRAINING_STAT_DISTANCE, //32
	UI_DISPLAY_TRAINING_STAT_PACE, //33
	UI_DISPLAY_TRAINING_STAT_HEART_RATE, //34
	UI_DISPLAY_TRAINING_STAT_RUN_STOP, // 35	
	UI_DISPLAY_TRAINING_STATATISTICS_END = UI_DISPLAY_TRAINING_STAT_RUN_STOP,
	// 7: Remembered
	UI_DISPLAY_PREVIOUS,
	// The end
	UI_DISPLAY_MAXIMUM,
};
#endif

// 5. Lemon 2 pay UI page display list.
#ifdef _CLINGBAND_2_PAY_MODEL_
enum {
	// 0: Home
	UI_DISPLAY_HOME = 0, 
	// 1: System
	UI_DISPLAY_SYSTEM, 
	UI_DISPLAY_SYSTEM_RESTART = UI_DISPLAY_SYSTEM, //1		
	UI_DISPLAY_SYSTEM_OTA, //2	
	UI_DISPLAY_SYSTEM_LINKING, //3	
	UI_DISPLAY_SYSTEM_UNAUTHORIZED, //4		
	UI_DISPLAY_SYSTEM_BATT_POWER, //5	
	UI_DISPLAY_SYSTEM_END = UI_DISPLAY_SYSTEM_BATT_POWER, //6		
	// 2: a set of the typical use cases
	UI_DISPLAY_TRACKER,
	UI_DISPLAY_TRACKER_STEP = UI_DISPLAY_TRACKER, //7		
	UI_DISPLAY_TRACKER_DISTANCE, //8
	UI_DISPLAY_TRACKER_CALORIES, //9 
	UI_DISPLAY_TRACKER_ACTIVE_TIME, //10	   
	UI_DISPLAY_TRACKER_END = UI_DISPLAY_TRACKER_ACTIVE_TIME,
	// 3: a set of smart features
	UI_DISPLAY_SMART,
	UI_DISPLAY_SMART_PM2P5 = UI_DISPLAY_SMART, //11
	UI_DISPLAY_SMART_WEATHER, //12
  UI_DISPLAY_SMART_MESSAGE, //13
  UI_DISPLAY_SMART_APP_NOTIF, //14
	UI_DISPLAY_SMART_DETAIL_NOTIF, //15	
	UI_DISPLAY_SMART_INCOMING_CALL, //16
	UI_DISPLAY_SMART_INCOMING_MESSAGE, //17	
	UI_DISPLAY_SMART_ALARM_CLOCK_REMINDER, //18
	UI_DISPLAY_SMART_ALARM_CLOCK_DETAIL, //19		
	UI_DISPLAY_SMART_IDLE_ALERT, //20
	UI_DISPLAY_SMART_HEART_RATE_ALERT, //21		
	UI_DISPLAY_SMART_STEP_10K_ALERT, //22					
	UI_DISPLAY_SMART_END = UI_DISPLAY_SMART_STEP_10K_ALERT,	
	// 4: a set of VITAL		
	UI_DISPLAY_VITAL,
	UI_DISPLAY_VITAL_HEART_RATE = UI_DISPLAY_VITAL, //24	
	UI_DISPLAY_VITAL_END = UI_DISPLAY_VITAL_HEART_RATE,
	// 5: Special back panel mode
	UI_DISPLAY_SETTING,
	UI_DISPLAY_SETTING_VER = UI_DISPLAY_SETTING, //25	
	UI_DISPLAY_SETTING_END = UI_DISPLAY_SETTING_VER,
	// 6: Stopwatch
	UI_DISPLAY_STOPWATCH,
	UI_DISPLAY_STOPWATCH_START = UI_DISPLAY_STOPWATCH,	//26	
	UI_DISPLAY_STOPWATCH_STOP, //27	                      
	UI_DISPLAY_STOPWATCH_END = UI_DISPLAY_STOPWATCH_STOP,
	// 7: Workout mode
	UI_DISPLAY_WORKOUT,
	UI_DISPLAY_WORKOUT_TREADMILL= UI_DISPLAY_WORKOUT, //28	
	UI_DISPLAY_WORKOUT_CYCLING, //29	
	UI_DISPLAY_WORKOUT_STAIRS, //30	
	UI_DISPLAY_WORKOUT_ELLIPTICAL, //31	
	UI_DISPLAY_WORKOUT_ROW, //32	
	UI_DISPLAY_WORKOUT_AEROBIC, //33	
	UI_DISPLAY_WORKOUT_PILOXING, //34	
	UI_DISPLAY_WORKOUT_OTHERS, //35	
	UI_DISPLAY_WORKOUT_RT_READY, //36		
	UI_DISPLAY_WORKOUT_RT_TIME,	//37		
	UI_DISPLAY_WORKOUT_RT_HEART_RATE, //38	
	UI_DISPLAY_WORKOUT_RT_CALORIES,	//39		
	UI_DISPLAY_WORKOUT_RT_END,	//40		
	UI_DISPLAY_WORKOUT_END = UI_DISPLAY_WORKOUT_RT_END,
	// 8: Running Statistics
	UI_DISPLAY_RUNNING_STATATISTICS,	
	UI_DISPLAY_RUNNING_STAT_DISTANCE = UI_DISPLAY_RUNNING_STATATISTICS, //41   
	UI_DISPLAY_RUNNING_STAT_TIME, //42
	UI_DISPLAY_RUNNING_STAT_PACE, //43
	UI_DISPLAY_RUNNING_STAT_STRIDE, //44
	UI_DISPLAY_RUNNING_STAT_CADENCE, //45
	UI_DISPLAY_RUNNING_STAT_HEART_RATE, //46
	UI_DISPLAY_RUNNING_STAT_CALORIES, //47
	UI_DISPLAY_RUNNING_STATATISTICS_END = UI_DISPLAY_RUNNING_STAT_CALORIES,
	// 9: Training Statistics
	UI_DISPLAY_TRAINING_STATATISTICS,
  UI_DISPLAY_TRAINING_STAT_START = UI_DISPLAY_TRAINING_STATATISTICS, //48    
 	UI_DISPLAY_TRAINING_STAT_START_OR_ANALYSIS, //49	
  UI_DISPLAY_TRAINING_STAT_READY, //50    
	UI_DISPLAY_TRAINING_STAT_TIME, //51	
	UI_DISPLAY_TRAINING_STAT_DISTANCE, //52
	UI_DISPLAY_TRAINING_STAT_PACE, //53	
	UI_DISPLAY_TRAINING_STAT_HEART_RATE, //54	
	UI_DISPLAY_TRAINING_STAT_RUN_STOP,	//55	
	UI_DISPLAY_TRAINING_STATATISTICS_END = UI_DISPLAY_TRAINING_STAT_RUN_STOP,
	// 10: Cycling Outdoor Statistics
	UI_DISPLAY_CYCLING_OUTDOOR_STATATISTICS,
  UI_DISPLAY_CYCLING_OUTDOOR_STAT_START = UI_DISPLAY_CYCLING_OUTDOOR_STATATISTICS, //56     	
  UI_DISPLAY_CYCLING_OUTDOOR_STAT_READY, //57  
	UI_DISPLAY_CYCLING_OUTDOOR_STAT_TIME, //58	
	UI_DISPLAY_CYCLING_OUTDOOR_STAT_DISTANCE, //59  
	UI_DISPLAY_CYCLING_OUTDOOR_STAT_SPEED, //60	
	UI_DISPLAY_CYCLING_OUTDOOR_STAT_HEART_RATE, //61	
	UI_DISPLAY_CYCLING_OUTDOOR_STAT_STOP,	//62	
	UI_DISPLAY_CYCLING_OUTDOOR_STATATISTICS_END = UI_DISPLAY_CYCLING_OUTDOOR_STAT_STOP,	
	// 11: MUSIC
	UI_DISPLAY_MUSIC,
	UI_DISPLAY_MUSIC_PLAY = UI_DISPLAY_MUSIC, //63	
	UI_DISPLAY_MUSIC_VOLUME, //64	
	UI_DISPLAY_MUSIC_SONG, //65	
	UI_DISPLAY_MUSIC_END = UI_DISPLAY_MUSIC_SONG,
	// 12: Pay
	UI_DISPLAY_PAY,
	UI_DISPLAY_PAY_BUS_CARD_BALANCE_ENQUIRY = UI_DISPLAY_PAY, //63	
	UI_DISPLAY_PAY_BANK_CARD_BALANCE_ENQUIRY,
	UI_DISPLAY_PAY_END = UI_DISPLAY_PAY_BANK_CARD_BALANCE_ENQUIRY,	
	// 13: CAROUSEL
	UI_DISPLAY_CAROUSEL,
	UI_DISPLAY_CAROUSEL_1 = UI_DISPLAY_CAROUSEL, // Running mode + cycling outdoor mode + Workout indoor mode  //66	
	UI_DISPLAY_CAROUSEL_2, // Music + Stopwatch + Message    //67	
	UI_DISPLAY_CAROUSEL_3, // Weather + PM2.5 + Alarm        //68	
	UI_DISPLAY_CAROUSEL_4, // BATT + Phone_finder + Setting	 //69	
	UI_DISPLAY_CAROUSEL_END = UI_DISPLAY_CAROUSEL_4,
	// 13: Remembered
	UI_DISPLAY_PREVIOUS,
	// The end
	UI_DISPLAY_MAXIMUM,
};
#endif

typedef struct tagFRAME_RENDERING_CTX {
	
	I8U vertical_icon_24_idx;
	
	I8U horizontal_icon_16_idx;
} FRAME_RENDERING_CTX;

typedef struct tagUI_ANIMATION_CTX {
	// All the dword variables
	I32U display_to_base;        // Display timeout base
	I32U frame_interval;         // Frame interval, i.e., the time interval that a particular frame stays on this frame
	I32U touch_time_stamp;       // Touch event time stamp
  I32U running_time_stamp;     // Running record time stamp.
	I32U page_store_time_stamp;  // Page store time stamp.
	I32U stopwatch_time_stamp;   // Stopwatch record time stamp. 	
	I32U stopwatch_t_stop_stamp; // Stopwatch record stop time stamp. 	
	
	// All the frame buffer related
	I8U p_oled_up[512];
	
	// State machine
	I8U state;
	
	BOOLEAN b_first_light_up_from_dark;
	BOOLEAN b_in_running_alarm_page;
	
	// Animation
	I8U animation_mode;
	I8U direction;
	I8U animation_index;
	
	// Page frame index switching
	I8U frame_index;
	I8U frame_cached_index;
	I8U frame_prev_idx;
	I8U frame_next_idx;	
	
	// Vertical switching
	I8U vertical_index;

	// Icon flashing
	BOOLEAN icon_sec_blinking;
	I8U linking_wave_index;
	I8U heart_rate_wave_index;
		
	// Fonts type
	BOOLEAN language_type;
	
	// Clock orientation
	I8U clock_orientation;

	// Notification
	BOOLEAN b_detail_page;	    // Detail page.
	BOOLEAN b_notif_need_store;
	BOOLEAN b_in_incoming_detail_page;
	I8U notif_repeat_look_time; // Notification repeat look time.	
	I8U notif_detail_index;     // Incoming message detail index.
  I8U string_pos_buf[5];	    // Incoming message detail index buff.	
	I8U app_notific_index;	    // App notific index

	// Running
	I8U run_ready_index;    // Read go index.
	BOOLEAN b_training_first_enter;
	
	// Alarm
	I8U ui_alarm_hh;
	I8U ui_alarm_mm;	
	
	// Training pace and hr alert
	I8U training_hr;

	// Stopwatch flag
	BOOLEAN b_stopwatch_first_enter;
	BOOLEAN b_in_stopwatch_mode;
	BOOLEAN b_in_stopwatch_pause_mode;

	// Rendering context
	FRAME_RENDERING_CTX frm_render;

	I32U bus_card_balance;
  I32U bank_card_balance;
} UI_ANIMATION_CTX;

typedef enum {
	UI_STATE_IDLE = 0,
	UI_STATE_CLING_START,	
  UI_STATE_HOME,	
	UI_STATE_CHARGING_GLANCE,
	UI_STATE_AUTHORIZATION,
	UI_STATE_FIRMWARE_OTA,
	UI_STATE_TOUCH_SENSING,
	UI_STATE_APPEAR,
	UI_STATE_ANIMATING,
	UI_STATE_DARK,
} UI_ANIMATION_STATE;

void UI_init(void);

void UI_start_notifying(I8U frame_index);

void UI_switch_state(I8U state, I32U interval);

BOOLEAN UI_turn_on_display(UI_ANIMATION_STATE state);

void UI_state_machine(void);
#endif
