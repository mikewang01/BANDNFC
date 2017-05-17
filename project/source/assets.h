/***************************************************************************//**
 * File: assets.h
 *
 * Created on Feb 28, 2014
 * 
 *****************************************************************************/

#include "standards.h"

#ifndef __ASSETS_HEADER__
#define __ASSETS_HEADER__

#include "assets\asset_len.h"
#include "assets\asset_pos.h"
#include "assets\asset_content.h"


/****************** 8 pixels high icon ********************/
#define ICON8_HEART_RATE_DYNAMICL_0_IDX    44
#define ICON8_HEART_RATE_DYNAMICL_0_LEN    asset_len[44]
#define ICON8_HEART_RATE_DYNAMICL_0_POS    asset_pos[44]
#define ICON8_HEART_RATE_DYNAMICL_1_IDX    45
#define ICON8_HEART_RATE_DYNAMICL_1_LEN    asset_len[45]
#define ICON8_HEART_RATE_DYNAMICL_1_POS    asset_pos[45]
#define ICON8_BATT_NOCHARGING_IDX          128
#define ICON8_BATT_NOCHARGING_LEN          asset_len[128]
#define ICON8_BATT_NOCHARGING_POS          asset_pos[128]
#define ICON8_BATT_CHARGING_IDX            129
#define ICON8_BATT_CHARGING_LEN            asset_len[129]
#define ICON8_BATT_CHARGING_POS            asset_pos[129]
#define ICON8_SMALL_BLE_IDX                130
#define ICON8_SMALL_BLE_LEN                asset_len[130]
#define ICON8_SMALL_BLE_POS                asset_pos[130]
#define ICON8_MORE_IDX                     147
#define ICON8_MORE_LEN                     asset_len[147]
#define ICON8_MORE_POS                     asset_pos[147]


/****************** 16 pixels high icon *******************/
#define ICON16_NONE                        127
#define ICON16_NORMAL_ALARM_CLOCK_IDX      128
#define ICON16_NORMAL_ALARM_CLOCK_LEN      asset_len[256+128]
#define ICON16_NORMAL_ALARM_CLOCK_POS      asset_pos[256+128]
#define ICON16_BATT_CHARGING_IDX           129
#define ICON16_BATT_CHARGING_LEN           asset_len[256+129] 
#define ICON16_BATT_CHARGING_POS           asset_pos[256+129] 
#define ICON16_SLEEP_ALARM_CLOCK_IDX       130
#define ICON16_SLEEP_ALARM_CLOCK_LEN       asset_len[256+130] 
#define ICON16_SLEEP_ALARM_CLOCK_POS       asset_pos[256+130] 
#define ICON16_BATT_CHARGING_FLAG_IDX      131
#define ICON16_BATT_CHARGING_FLAG_LEN      asset_len[256+131] 
#define ICON16_BATT_CHARGING_FLAG_POS      asset_pos[256+131] 
#define ICON16_ACTIVE_TIME_IDX             131
#define ICON16_ACTIVE_TIME_LEN             asset_len[256+131]
#define ICON16_ACTIVE_TIME_POS             asset_pos[256+131]
#define ICON16_AUTH_PROGRESS_MIDDLE_IDX    132
#define ICON16_AUTH_PROGRESS_MIDDLE_LEN    asset_len[256+132] 
#define ICON16_AUTH_PROGRESS_MIDDLE_POS    asset_pos[256+132] 
#define ICON16_AUTH_PROGRESS_LEFT_IDX      134
#define ICON16_AUTH_PROGRESS_LEFT_LEN      asset_len[256+134] 
#define ICON16_AUTH_PROGRESS_LEFT_POS      asset_pos[256+134] 
#define ICON16_AUTH_PROGRESS_RIGHT_IDX     136
#define ICON16_AUTH_PROGRESS_RIGHT_LEN     asset_len[256+136] 
#define ICON16_AUTH_PROGRESS_RIGHT_POS     asset_pos[256+136] 
#define ICON16_MESSAGE_IDX                 137   
#define ICON16_MESSAGE_LEN                 asset_len[256+137]
#define ICON16_MESSAGE_POS                 asset_pos[256+137]
#define ICON16_RUNNING_STOP_IDX            138   
#define ICON16_RUNNING_STOP_LEN            asset_len[256+138]
#define ICON16_RUNNING_STOP_POS            asset_pos[256+138]
#define ICON16_CYCLING_OUTDOOR_MODE_IDX    139   
#define ICON16_CYCLING_OUTDOOR_MODE_LEN    asset_len[256+139]
#define ICON16_CYCLING_OUTDOOR_MODE_POS    asset_pos[256+139]
#define ICON24_SLEEP_ALARM_CLOCK_IDX       140
#define ICON24_SLEEP_ALARM_CLOCK_LEN       asset_len[512+140]
#define ICON24_SLEEP_ALARM_CLOCK_POS       asset_pos[512+140]
#define ICON16_SETTING_IDX                 140
#define ICON16_SETTING_LEN                 asset_len[256+140]
#define ICON16_SETTING_POS                 asset_pos[256+140]
#define ICON16_RETURN_IDX                  141
#define ICON16_RETURN_LEN                  asset_len[256+141]
#define ICON16_RETURN_POS                  asset_pos[256+141]
#define ICON16_OK_IDX                      142
#define ICON16_OK_LEN                      asset_len[256+142]
#define ICON16_OK_POS                      asset_pos[256+142]
#define ICON16_STOPWATCH_START_IDX         143
#define ICON16_STOPWATCH_START_LEN         asset_len[256+143]
#define ICON16_STOPWATCH_START_POS         asset_pos[256+143]
#define ICON16_STOPWATCH_STOP_IDX          144
#define ICON16_STOPWATCH_STOP_LEN          asset_len[256+144]
#define ICON16_STOPWATCH_STOP_POS          asset_pos[256+144]
#define ICON16_CYCLING_OUTDOOR_SPEED_IDX   146
#define ICON16_CYCLING_OUTDOOR_SPEED_LEN   asset_len[256+146]
#define ICON16_CYCLING_OUTDOOR_SPEED_POS   asset_pos[256+146]
#define ICON16_STOPWATCH_IDX               147
#define ICON16_STOPWATCH_LEN               asset_len[256+147]
#define ICON16_STOPWATCH_POS               asset_pos[256+147]
#define ICON16_UV_INDEX_IDX                148
#define ICON16_UV_INDEX_LEN                asset_len[256+148]
#define ICON16_UV_INDEX_POS                asset_pos[256+148]
#define ICON16_SKIN_TEMP_IDX               149
#define ICON16_SKIN_TEMP_LEN               asset_len[256+149]
#define ICON16_SKIN_TEMP_POS               asset_pos[256+149]
#define ICON16_HEART_RATE_IDX              150
#define ICON16_HEART_RATE_LEN              asset_len[256+150]
#define ICON16_HEART_RATE_POS              asset_pos[256+150]
#define ICON16_STEPS_IDX                   151
#define ICON16_STEPS_LEN                   asset_len[256+151]
#define ICON16_STEPS_POS                   asset_pos[256+151]
#define ICON16_DISTANCE_IDX                152
#define ICON16_DISTANCE_LEN                asset_len[256+152]
#define ICON16_DISTANCE_POS                asset_pos[256+152]
#define ICON16_CALORIES_IDX                153
#define ICON16_CALORIES_LEN                asset_len[256+153]
#define ICON16_CALORIES_POS                asset_pos[256+153]
#define ICON16_PM2P5_IDX                   154
#define ICON16_PM2P5_LEN                   asset_len[256+154]
#define ICON16_PM2P5_POS                   asset_pos[256+154]
#define ICON16_WEATHER_IDX                 156
#define ICON16_WEATHER_LEN                 asset_len[256+156]
#define ICON16_WEATHER_POS                 asset_pos[256+156]
#define ICON16_INCOMING_CALL_IDX           160
#define ICON16_INCOMING_CALL_LEN           asset_len[256+160]
#define ICON16_INCOMING_CALL_POS           asset_pos[256+160]
#define ICON16_RUNNING_DISTANCE_IDX        161
#define ICON16_RUNNING_DISTANCE_LEN        asset_len[256+161]
#define ICON16_RUNNING_DISTANCE_POS        asset_pos[256+161]
#define ICON16_RUNNING_TIME_IDX            162
#define ICON16_RUNNING_TIME_LEN            asset_len[256+162]
#define ICON16_RUNNING_TIME_POS            asset_pos[256+162]
#define ICON16_RUNNING_PACE_IDX            163
#define ICON16_RUNNING_PACE_LEN            asset_len[256+163]
#define ICON16_RUNNING_PACE_POS            asset_pos[256+163]
#define ICON16_RUNNING_STRIDE_IDX          164
#define ICON16_RUNNING_STRIDE_LEN          asset_len[256+164]
#define ICON16_RUNNING_STRIDE_POS          asset_pos[256+164]
#define ICON16_RUNNING_CADENCE_IDX         165
#define ICON16_RUNNING_CADENCE_LEN         asset_len[256+165]
#define ICON16_RUNNING_CADENCE_POS         asset_pos[256+165]
#define ICON16_RUNNING_HR_IDX              166
#define ICON16_RUNNING_HR_LEN              asset_len[256+166]
#define ICON16_RUNNING_HR_POS              asset_pos[256+166]
#define ICON16_RUNNING_CALORIES_IDX        167
#define ICON16_RUNNING_CALORIES_LEN        asset_len[256+167]
#define ICON16_RUNNING_CALORIES_POS        asset_pos[256+167]


/****************** 24 pixels high icon *******************/
#define ICON24_NONE                        125
#define ICON24_WEATHER_RANGE_IDX           126
#define ICON24_WEATHER_RANGE_LEN           asset_len[512+126]
#define ICON24_WEATHER_RANGE_POS           asset_pos[512+126]
#define ICON24_UV_INDEX_IDX                127
#define ICON24_UV_INDEX_LEN                asset_len[512+127]
#define ICON24_UV_INDEX_POS                asset_pos[512+127]
#define ICON24_SKIN_TEMP_IDX               128
#define ICON24_SKIN_TEMP_LEN               asset_len[512+128]
#define ICON24_SKIN_TEMP_POS               asset_pos[512+128]
#define ICON24_ACTIVE_TIME_IDX             129
#define ICON24_ACTIVE_TIME_LEN             asset_len[512+129]
#define ICON24_ACTIVE_TIME_POS             asset_pos[512+129]
#define ICON24_HEART_RATE_IDX              130
#define ICON24_HEART_RATE_LEN              asset_len[512+130]
#define ICON24_HEART_RATE_POS              asset_pos[512+130]
#define ICON24_STEPS_IDX                   131
#define ICON24_STEPS_LEN                   asset_len[512+131]
#define ICON24_STEPS_POS                   asset_pos[512+131]
#define ICON24_CALORIES_IDX                132
#define ICON24_CALORIES_LEN                asset_len[512+132]
#define ICON24_CALORIES_POS                asset_pos[512+132]
#define ICON24_DISTANCE_IDX                133
#define ICON24_DISTANCE_LEN                asset_len[512+133]
#define ICON24_DISTANCE_POS                asset_pos[512+133]
#define ICON24_PM2P5_IDX                   134
#define ICON24_PM2P5_LEN                   asset_len[512+134]
#define ICON24_PM2P5_POS                   asset_pos[512+134]
#define ICON24_WEATHER_IDX                 135
#define ICON24_WEATHER_LEN                 asset_len[512+135]
#define ICON24_WEATHER_POS                 asset_pos[512+135]
#define ICON24_NORMAL_ALARM_CLOCK_IDX      139
#define ICON24_NORMAL_ALARM_CLOCK_LEN      asset_len[512+139]
#define ICON24_NORMAL_ALARM_CLOCK_POS      asset_pos[512+139]
#define ICON24_SLEEP_ALARM_CLOCK_IDX       140
#define ICON24_SLEEP_ALARM_CLOCK_LEN       asset_len[512+140]
#define ICON24_SLEEP_ALARM_CLOCK_POS       asset_pos[512+140]
#define ICON24_SETTING_IDX                 141
#define ICON24_SETTING_LEN                 asset_len[512+141]
#define ICON24_SETTING_POS                 asset_pos[512+141]
#define ICON24_IDLE_ALERT_IDX              142
#define ICON24_IDLE_ALERT_LEN              asset_len[512+142]
#define ICON24_IDLE_ALERT_POS              asset_pos[512+142]
#define ICON24_CELCIUS_IDX                 143
#define ICON24_CELCIUS_LEN                 asset_len[512+143]
#define ICON24_CELCIUS_POS                 asset_pos[512+143]
#define ICON24_MESSAGE_IDX                 144
#define ICON24_MESSAGE_LEN                 asset_len[512+144]
#define ICON24_MESSAGE_POS                 asset_pos[512+144]
#define ICON24_WORKOUT_MODE_IDX            145
#define ICON24_WORKOUT_MODE_LEN            asset_len[512+145]
#define ICON24_WORKOUT_MODE_POS            asset_pos[512+145]
#define ICON24_RUNNING_MODE_IDX            147
#define ICON24_RUNNING_MODE_LEN            asset_len[512+147]
#define ICON24_RUNNING_MODE_POS            asset_pos[512+147]
#define ICON24_RUNNING_STOP_IDX            149		
#define ICON24_RUNNING_STOP_LEN            asset_len[512+149]
#define ICON24_RUNNING_STOP_POS            asset_pos[512+149]
#define ICON24_CYCLING_OUTDOOR_MODE_IDX    151		
#define ICON24_CYCLING_OUTDOOR_MODE_LEN    asset_len[512+151]
#define ICON24_CYCLING_OUTDOOR_MODE_POS    asset_pos[512+151]
#define ICON24_NO_SKIN_TOUCH_IDX           152		
#define ICON24_NO_SKIN_TOUCH_LEN           asset_len[512+152]
#define ICON24_NO_SKIN_TOUCH_POS           asset_pos[512+152]
#define ICON24_CYCLING_OUTDOOR_SPEED_IDX   153		
#define ICON24_CYCLING_OUTDOOR_SPEED_LEN   asset_len[512+153]
#define ICON24_CYCLING_OUTDOOR_SPEED_POS   asset_pos[512+153]
#define ICON24_STOPWATCH_IDX               157		
#define ICON24_STOPWATCH_LEN               asset_len[512+157]
#define ICON24_STOPWATCH_POS               asset_pos[512+157]
#define ICON24_RUNNING_TIME_IDX            160		
#define ICON24_RUNNING_TIME_LEN            asset_len[512+160]
#define ICON24_RUNNING_TIME_POS            asset_pos[512+160]
#define ICON24_RUNNING_PACE_IDX            161
#define ICON24_RUNNING_PACE_LEN            asset_len[512+161]
#define ICON24_RUNNING_PACE_POS            asset_pos[512+161]
#define ICON24_RUNNING_DISTANCE_IDX        162
#define ICON24_RUNNING_DISTANCE_LEN        asset_len[512+162]
#define ICON24_RUNNING_DISTANCE_POS        asset_pos[512+162]
#define ICON24_RUNNING_CALORIES_IDX        163
#define ICON24_RUNNING_CALORIES_LEN        asset_len[512+163]
#define ICON24_RUNNING_CALORIES_POS        asset_pos[512+163]
#define ICON24_RUNNING_HR_IDX              164
#define ICON24_RUNNING_HR_LEN              asset_len[512+164]
#define ICON24_RUNNING_HR_POS              asset_pos[512+164]
#define ICON24_RUNNING_STRIDE_IDX          165
#define ICON24_RUNNING_STRIDE_LEN          asset_len[512+165]
#define ICON24_RUNNING_STRIDE_POS          asset_pos[512+165]
#define ICON24_RUNNING_CADENCE_IDX         166
#define ICON24_RUNNING_CADENCE_LEN         asset_len[512+166]
#define ICON24_RUNNING_CADENCE_POS         asset_pos[512+166]

#endif /* __ASSETS_HEADER__ */

/** @} */
