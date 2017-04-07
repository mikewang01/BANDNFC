//
//  File: weather.h
//  
//  Description: the header contains all weather definition
//
//  Created on Jan 6, 2014
//
#ifndef __WEATHER_HEADER__
#define __WEATHER_HEADER__


#define MAX_WEATHER_DAYS  5

typedef struct tagWEATHER_INFO_CTX {
	I8U month;
	I8U day;
	I8U type;
	I8S low_temperature;
	I8S high_temperature;
} WEATHER_INFO_CTX;

enum {
	W_SUNNY,
	W_CLOUDY,
	W_RAINY,
	W_SNOWY
};

typedef struct tagWEATHER_CTX {

	WEATHER_INFO_CTX weather_info[MAX_WEATHER_DAYS];

	I8U  pm2p5_month;
	I8U  pm2p5_day;
	I16U pm2p5_value;
	
} WEATHER_CTX;

BOOLEAN WEATHER_get_weather_info(I8U index, WEATHER_INFO_CTX *wo);
void WEATHER_set_weather(I8U *data);

#endif  // __WEATHER_HEADER__
