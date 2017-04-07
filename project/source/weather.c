/***************************************************************************/
/**
 * File: weather.c
 * 
 * Description: weather information processing
 *
 * Created on May 15, 2014
 *
 ******************************************************************************/

#include <stdio.h>
#include <string.h>


#include "main.h"

void WEATHER_set_weather(I8U *data)
{
	WEATHER_CTX *w = &cling.weather;
	I8U *pPM2P5;

  memcpy(&w->weather_info[0], data, 5*sizeof(WEATHER_INFO_CTX));	
	
	N_SPRINTF("[WEATHR] %d, %d, %d, %d, %d", w->weather_info[0].month, w->weather_info[0].day, w->weather_info[0].type, w->weather_info[0].low_temperature, w->weather_info[0].high_temperature);
	N_SPRINTF("[WEATHR] %d, %d, %d, %d, %d", w->weather_info[1].month, w->weather_info[1].day, w->weather_info[1].type, w->weather_info[1].low_temperature, w->weather_info[1].high_temperature);
	N_SPRINTF("[WEATHR] %d, %d, %d, %d, %d", w->weather_info[2].month, w->weather_info[2].day, w->weather_info[2].type, w->weather_info[2].low_temperature, w->weather_info[2].high_temperature);
	N_SPRINTF("[WEATHR] %d, %d, %d, %d, %d", w->weather_info[3].month, w->weather_info[3].day, w->weather_info[3].type, w->weather_info[3].low_temperature, w->weather_info[3].high_temperature);
	N_SPRINTF("[WEATHR] %d, %d, %d, %d, %d", w->weather_info[4].month, w->weather_info[4].day, w->weather_info[4].type, w->weather_info[4].low_temperature, w->weather_info[4].high_temperature);

	// Load PM2.5 read
	pPM2P5 = data + 5*sizeof(WEATHER_INFO_CTX);
	
	w->pm2p5_value = pPM2P5[0];
	w->pm2p5_value <<= 8;
	w->pm2p5_value |= pPM2P5[1];
	
	// Update pm2.5 time
  w->pm2p5_month = cling.time.local.month;
	w->pm2p5_day = cling.time.local.day;
}

BOOLEAN WEATHER_get_weather_info(I8U index, WEATHER_INFO_CTX *wo)
{
	WEATHER_CTX *w = &cling.weather;

#ifdef _CLINGBAND_PACE_MODEL_
	if ((w->weather_info[0].day == cling.time.local.day) && (w->weather_info[0].month == cling.time.local.month)) {
  	memcpy(wo, &w->weather_info[0], sizeof(WEATHER_INFO_CTX));
	  return TRUE;
	} else {
		return FALSE;
	}
#else
	I8U i;
	BOOLEAN b_available = FALSE;
	SYSTIME_CTX new_time;
	
	RTC_get_delta_clock_forward(&new_time, index);
	
	for (i = 0; i < MAX_WEATHER_DAYS; i++) {
		if ((w->weather_info[i].day == new_time.day) && (w->weather_info[i].month == new_time.month)) {
			memcpy(wo, &w->weather_info[i], sizeof(WEATHER_INFO_CTX));
			b_available = TRUE;
			break;
		}
	}
	
	if (b_available) return TRUE;
	
	// if no weather available, go return the first one
	RTC_get_delta_clock_forward(&new_time, 0);
	
	if ((w->weather_info[0].day == new_time.day) && (w->weather_info[0].month == new_time.month)) {
		memcpy(wo, &w->weather_info[0], sizeof(WEATHER_INFO_CTX));
		return TRUE;
	} else {
		wo->high_temperature = 0;
		wo->low_temperature = 0;
		wo->type = 0;
		return FALSE;
	}
#endif
}

