/***************************************************************************//**
 * File: ui.c
 * 
 * Description: UI display frame, include horizontal and vertical.
 *
 ******************************************************************************/


#include "main.h"
#include "assets.h"
#include "font.h"
#include "ui_frame.h"

#pragma diag_suppress 870

static void _core_frame_display(I8U middle, BOOLEAN b_render);
static void _RENDER_NONE() { }

void UI_render_screen()
{	
	OLED_full_scree_show();

	OLED_set_display(1);
}

/***************************************************************************/
/*********************** Horizontal display page ***************************/
/***************************************************************************/
static void _render_one_icon_8(I8U icon_8_idx, I8U *p_out_0)
{
  I8U len, i;	
	const I8U *p_in;
	
	len = asset_len[icon_8_idx];
	p_in = asset_content+asset_pos[icon_8_idx];
	
	for (i = 0; i < len; i++) {
			*p_out_0++ = (*p_in++);
	}
}

static void _render_one_icon_16(I8U icon_16_idx, I16U offset)
{
  I8U len, i;	
	const I8U *p_in;
	I8U *p_out_0 = cling.ui.p_oled_up+offset;
	I8U *p_out_1 = p_out_0+128;

	len = asset_len[256+icon_16_idx];
	p_in = asset_content+asset_pos[256+icon_16_idx];
	
	// Render the left side
	for (i = 0; i < len; i++) {
			*p_out_0++ = (*p_in++);
			*p_out_1++ = (*p_in++);
	}
}

static void _render_one_icon_24(I8U icon_24_idx, I16U offset)
{
  I8U len, i;	
	const I8U *p_in;
	I8U *p_out_0 = cling.ui.p_oled_up+offset;
	I8U *p_out_1 = p_out_0+128;
	I8U *p_out_2 = p_out_1+128;
	
	len = asset_len[512+icon_24_idx];
	p_in = asset_content+asset_pos[512+icon_24_idx];
	
	for (i = 0; i < len; i++) {
			*p_out_0++ = (*p_in++);
			*p_out_1++ = (*p_in++);
			*p_out_2++ = (*p_in++);
	}	
}

static void _render_batt_ble_core(I8U *p_in)
{	
  I8U *p0;	
	I8U i;
	I8U curr_batt_level = cling.system.mcu_reg[REGISTER_MCU_BATTERY];
	I8U p_v = 0x1c;
  I16U offset = 0;
	
	if (BTLE_is_connected()) {
		offset += (ICON8_BATT_CHARGING_LEN + 2);
		_render_one_icon_8(ICON8_SMALL_BLE_IDX, p_in + offset);
	} 

	// Render the right side (offset set to 60 for steps comment)
	if (BATT_is_charging()) {
		_render_one_icon_8(ICON8_BATT_CHARGING_IDX, p_in);
	}	else {
		_render_one_icon_8(ICON8_BATT_NOCHARGING_IDX, p_in);		
	}
	
	// Filling up the percentage
	curr_batt_level /= 11;
	
	if (curr_batt_level == 0)
		return;
	
	if (curr_batt_level > 9)
		curr_batt_level = 9;
	
	// Note: the battery button icon is 9 pixels of length
	p0 = p_in+2;
	for (i = 0; i < curr_batt_level; i++) {
		*p0++ |= p_v;
	}
}

static void _left_render_horizontal_batt_ble()
{
  _render_batt_ble_core(cling.ui.p_oled_up);
}

static void _left_render_horizontal_16_icon()
{
  I8U horizontal_16_icon_idx = cling.ui.frm_render.horizontal_icon_16_idx;
	
	if (horizontal_16_icon_idx == ICON16_NONE)
	  return;
	
	_render_one_icon_16(horizontal_16_icon_idx, 0);
}

static void _left_render_horizontal_pm2p5()
{
	char *air_display[3][7] = {{"--","Good", "Moderate", "Unhealthy", "Poor", "Poor", "hazardous"},
	                       {"--","优质", "良好", "轻度", "中度", "重度", "严重"},
												 {"--","優質", "良好", "輕度", "中度", "重度", "嚴重"}};

	I8U level_idx;
	I8U language_type = cling.ui.language_type;
	WEATHER_CTX *w = &cling.weather;
												 
  //  First render pm2.5 icon.	
	_render_one_icon_16(ICON16_PM2P5_IDX, 0);
	
  if ((w->pm2p5_month == cling.time.local.month) && (w->pm2p5_day == cling.time.local.day)) {
		if (w->pm2p5_value == 0xffff) {
			// AQI value Not available
			level_idx = 0;
		} else if (!w->pm2p5_value) {
			// AQI value Not available
			level_idx = 0;	
		} else if (w->pm2p5_value < 50) {
			level_idx = 1;
		} else if (w->pm2p5_value < 100) {
			level_idx = 2;
		} else if (w->pm2p5_value < 150) {
			level_idx = 3;
		} else if (w->pm2p5_value < 250) {
			level_idx = 4;
		} else if (w->pm2p5_value < 350) {
			level_idx = 5;
		} else {
			level_idx = 6;
		}
  } else {
		// AQI value Not available
		level_idx = 0;		
	}
	
	FONT_load_characters(256, air_display[language_type][level_idx], 16, 128, FALSE);
}

static void _left_render_horizontal_weather()
{
	WEATHER_INFO_CTX weather_info;
	
	if (!WEATHER_get_weather_info(0, &weather_info)) {
	  weather_info.type = 0;
	}
	
	_render_one_icon_16(ICON16_WEATHER_IDX+weather_info.type, 0);
}

static void _left_render_horizontal_16_icon_blinking()
{
	if (cling.ui.icon_sec_blinking) 
		_left_render_horizontal_16_icon();
}

static void _left_render_horizontal_alarm_clock_reminder()
{
	if ((cling.reminder.alarm_type == SLEEP_ALARM_CLOCK) || (cling.reminder.alarm_type == WAKEUP_ALARM_CLOCK))
		_render_one_icon_16(ICON16_SLEEP_ALARM_CLOCK_IDX, 0);				
	else 
		_render_one_icon_16(ICON16_NORMAL_ALARM_CLOCK_IDX, 0);
}

static void _left_render_horizontal_idle_alert()
{
  _render_one_icon_24(ICON24_IDLE_ALERT_IDX, 0);	
}

static void _left_render_horizontal_training_ready()
{
	_render_one_icon_24(ICON24_RUNNING_DISTANCE_IDX, 128+10);
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _left_render_horizontal_cycling_outdoor_ready()
{
	_render_one_icon_24(ICON24_CYCLING_OUTDOOR_MODE_IDX, 128+10);
}
#endif

static void _middle_horizontal_alignment_center(I16U offset)
{
	I8U *p0, *p1, *p2;
	I8U *p4, *p5, *p6;
	I8U ptr;
	I16U i;

	p0 = cling.ui.p_oled_up+128;
	p1 = p0+128;
	p2 = p1+128;
	ptr = (128 - offset)>>1;

	// Update new offset and clean up the rest area
	offset = 128 - ptr;
	
	if (ptr > 0) {
		p0 += 127; p4 = p0 - ptr;
		p1 += 127; p5 = p1 - ptr;
		p2 += 127; p6 = p2 - ptr;
		for (i = 0; i < 128-ptr; i++) {
			*p0-- = *p4--;
			*p1-- = *p5--;
			*p2-- = *p6--;
		}
		for (; i < 128; i++) {
			*p0-- = 0;
			*p1-- = 0;
			*p2-- = 0;
		}
	}
}

static I16U _render_middle_horizontal_section_core(I8U *string, I8U b_24_size, I8U margin, I16U offset, I8U light_num)
{
	I16U i, j;
	I8U *p0, *p1, *p2;
	const I8U *pin;
	I8U char_len;
	I8U len;
	
	len = strlen((char *)string);
	
	for (i = 0; i < len; i++) {
		if (b_24_size == 24) {
	    p0 = cling.ui.p_oled_up+128+offset;					
		  p1 = p0+128;
		  p2 = p1+128;			
		  if (string[i] == ' ') {
			  // This is "space" character
			  char_len = 4;
		  } else {
				// Digits in large fonts
				if (i < light_num) { 
					// Using "light" font
					pin = asset_content+asset_pos[512+string[i]];
					char_len = asset_len[512+string[i]];
				} else {
					if ((string[i] >= '0') && (string[i] <= '9')) {
						pin = asset_content+asset_pos[512+string[i]+152];
						char_len = asset_len[512+string[i]+152];
					} else {
						// For punctuation, just show what it is
						pin = asset_content+asset_pos[512+string[i]];
						char_len = asset_len[512+string[i]];
					}
				}
				for (j = 0; j < char_len; j++) {
					*p0++ = (*pin++);
					*p1++ = (*pin++);
					*p2++ = (*pin++);
				}	
		  }				
		} else if (b_24_size == 16) {
	    p0 = cling.ui.p_oled_up+128+offset;					
			p1 = p0+128;
		  if (string[i] == ' ') {
			  // This is "space" character
			  char_len = 4;
		  } else {
				pin = asset_content+asset_pos[256+string[i]];
				char_len = asset_len[256+string[i]];
				for (j = 0; j < char_len; j++) {
					*p0++ = (*pin++);
					*p1++ = (*pin++);
				}	
		  }					
		} else if (b_24_size == 8) {
	    p0 = cling.ui.p_oled_up+128+offset;				
			pin = asset_content+asset_pos[string[i]];
			char_len = asset_len[string[i]];
			for (j = 0; j < char_len; j++) {
					*p0++ = (*pin++);
			}
		} else {
			
		}
		
		if (i != (len-1))
			offset += char_len + margin;
		else
			offset += char_len;
	}

	return offset;
}

static void _middle_render_horizontal_system_restart()
{
	I8U string1[128];
	I8U string2[16];
	I8U string3[16];
	I16U len;
	I16U major;
	I16U minor;
	
	USER_device_get_name(string1);
	FONT_load_characters(0, (char *)string1, 16, 128, TRUE);
	memset(cling.ui.p_oled_up+256, 0, 256);
	
	major = cling.system.mcu_reg[REGISTER_MCU_REVH]>>4;
	minor = cling.system.mcu_reg[REGISTER_MCU_REVH]&0x0f;
	minor <<= 8;
	minor |= cling.system.mcu_reg[REGISTER_MCU_REVL];

	len = sprintf((char *)string2, "VER:%d.%d", major, minor);
	FONT_load_characters(384+64, (char *)string2, 8, 128, FALSE);

	len = sprintf((char *)string3, "ID:");
	SYSTEM_get_ble_code(string3+len);	
	string3[len+4] = 0;	
	FONT_load_characters(384+12, (char *)string3, 8, 128, FALSE);
}

static void _middle_render_horizontal_clock()
{
	I8U string[32];
	I8U b_24_size = 24;		
	I8U margin = 3;
	I16U offset = 20;	

	if (cling.ui.icon_sec_blinking) {
		sprintf((char *)string, "%02d:%02d", cling.time.local.hour, cling.time.local.minute);
	} else {
		sprintf((char *)string, "%02d %02d", cling.time.local.hour, cling.time.local.minute);
	}
	
	_render_middle_horizontal_section_core(string, b_24_size, margin, offset, 3);
}

static void _middle_render_horizontal_ota()
{
  const I8U font_content[] = {0x3c,0x42,0x42,0xbc,0x60,0x18,0x06,0x00,0x60,0x18,0x06,0x3d,0x42,0x42,0x3c,0x00};/*%*/
	I8U string[32];
	I8U b_24_size = 16;			
  I8U string_len=0;
  I16U offset=0;
	I8U bar_len=0;
  I8U margin=2;
	I8U *p0,*p1;
	I8U i;

	string_len = sprintf((char *)string, "%d", cling.ota.percent);
	bar_len = cling.ota.percent;

  offset = 56 - string_len*4;
	offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);

	offset += 4;
	p0 = cling.ui.p_oled_up+128+offset;
	p1 = p0+128;
	memcpy(p0, font_content, 8);
	memcpy(p1, font_content+8, 8);
	
	p0 = cling.ui.p_oled_up+384+14;
	for (i = 0; i < bar_len; i++) {
		*p0++ = 0xf0;
	}
	for (; i < 100; i++) {
		*p0++ = 0x80;
	}
}

static void _middle_render_horizontal_system_charging()
{
	I16U perc = cling.system.mcu_reg[REGISTER_MCU_BATTERY];
	I16U i;
	I16U fill_len;
	I8U *p0, *p1;
	I16U offset = 0; // Pixel offet at top row
	I8U string[32];
  BOOLEAN b_batt_charging = FALSE;

	if (BATT_is_charging()) 
		b_batt_charging = TRUE;

	offset = 15;
		
	if (b_batt_charging) {
	  _render_one_icon_16(ICON16_BATT_CHARGING_IDX, 128+offset);
		offset += ICON16_BATT_CHARGING_LEN;
		offset += 5;
		_render_one_icon_16(ICON16_BATT_CHARGING_FLAG_IDX, 128+offset);
	} else {
	  _render_one_icon_16(ICON16_BATT_CHARGING_IDX, 128+offset);
	}
	
	// Filling up the percentage
	if (perc >= 100) {
		perc = 100;
		fill_len = 43;
	} else {
		fill_len = perc*43/100;
	}
	
	// Filling the percentage
	offset = 15;
	p0 = cling.ui.p_oled_up+128+offset+2;
	p1 = p0+128;
	for (i = 0; i < fill_len; i++) {
		*p0++ |= 0xfc;
		*p1++ |= 0x3f;
	}
	
	// show the percentage
	sprintf((char *)string, "%d%%", perc);
	FONT_load_characters(128+90, (char *)string, 16, 128, FALSE);
}

static void _middle_render_horizontal_ble_code()
{
	I8U string[32];
	SYSTEM_get_ble_code(string);
	string[4] = 0;

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_PACE_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)		
	I8U b_24_size = 24;		
	I8U margin = 3;	
	I16U offset = 33;		
	
	_render_middle_horizontal_section_core(string, b_24_size, margin, offset, 4);
#endif
	
#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)			
	I8U string1[16];
	
	string1[0] = string[0];
	string1[1] = ' ';		
	string1[2] = string[1];
	string1[3] = ' ';
	string1[4] = string[2];
	string1[5] = ' ';	
	string1[6] = string[3];
	string1[7] = 0;			
	FONT_load_characters(256, (char *)string1, 16, 128, TRUE);		
#endif
}

static void _middle_render_horizontal_linking()
{
	I16U offset = 0;
	
	if (cling.ui.linking_wave_index > 2)
		 cling.ui.linking_wave_index=0;

	for (I8U i=0;i<cling.ui.linking_wave_index;i++) {
		_render_one_icon_16(ICON16_AUTH_PROGRESS_LEFT_IDX, 128+offset);
		offset += ICON16_AUTH_PROGRESS_LEFT_LEN;	
		offset += 5;		
	}

	_render_one_icon_16(ICON16_AUTH_PROGRESS_MIDDLE_IDX, 128+offset);
	offset += ICON16_AUTH_PROGRESS_MIDDLE_LEN;		
	offset += 5;
	
	for (I8U i=0;i<cling.ui.linking_wave_index;i++) {
		_render_one_icon_16(ICON16_AUTH_PROGRESS_RIGHT_IDX, 128+offset);
		offset += ICON16_AUTH_PROGRESS_RIGHT_LEN;			
    offset += 5;			
	}

	_middle_horizontal_alignment_center(offset);
	
	cling.ui.linking_wave_index++;
}

static void _middle_render_horizontal_steps()
{
	I8U string[16];
	I8U b_24_size = 24;			
	I16U offset = 0;
	I8U margin = 3;
	I32U stat;
	I16U integer, fractional;

#ifdef _CLINGBAND_PACE_MODEL_	
	TRACKING_get_activity(0, TRACKING_STEPS, &stat);
#else	
	TRACKING_get_activity(cling.ui.vertical_index, TRACKING_STEPS, &stat);
#endif
	
	if (stat > 99999)
		stat = 99999;
	if (stat > 999) {
		integer = stat / 1000;
		fractional = stat - integer * 1000;
		sprintf((char *)string, "%d,%03d", integer, fractional);
	} else {
		sprintf((char *)string, "%d", stat);
	}

	offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
	// Shift all the display to the middle
	_middle_horizontal_alignment_center(offset);
}

static void _middle_render_horizontal_distance()
{
	const char *unit_distance_display[3][2] = {{"KM", "ML"},{"公里", "英里"},{"公裏", "英裏"}};	
	I8U string[32];
	I8U b_24_size = 24;				
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 3;
	I32U stat;
	I16U integer, fractional;
	I8U language_type = cling.ui.language_type;
	I8U metric = cling.user_data.profile.metric_distance;

#ifdef _CLINGBAND_PACE_MODEL_	
	TRACKING_get_activity(0, TRACKING_DISTANCE, &stat);
#else	
	TRACKING_get_activity(cling.ui.vertical_index, TRACKING_DISTANCE, &stat);
#endif
	
	// Convert to miles
	if (cling.user_data.profile.metric_distance) {
		stat *= 10;
		stat >>= 4;
	}
	
	if (stat > 99999)
		stat = 99999;
	integer = stat/1000;
	fractional = stat - integer * 1000;
	if (stat > 9999) {
		fractional /= 100;
		len = sprintf((char *)string, "%d.%d", integer, fractional);
	} else {
		fractional /= 10;
		len = sprintf((char *)string, "%d.%02d", integer, fractional);
	}

  if (len == 4) {
		offset = 25;
		margin = 3;
	} else {
	  offset = 20;
		margin = 1;
	}
	
	offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
	
	FONT_load_characters(256+offset+3, (char *)unit_distance_display[language_type][metric], 16, 128, FALSE);
}

static void _middle_render_horizontal_calories()
{
	I8U string[32];
	I8U b_24_size = 24;			
	I16U offset = 0;
	I8U margin = 3;
	I32U stat;
	I16U integer, fractional;

#ifdef _CLINGBAND_PACE_MODEL_	
	TRACKING_get_activity(0, TRACKING_CALORIES, &stat);
#else 
	TRACKING_get_activity(cling.ui.vertical_index, TRACKING_CALORIES, &stat);
#endif
	
	if (stat > 999) {
		integer = stat / 1000;
		fractional = stat - integer * 1000;
		sprintf((char *)string, "%d,%03d", integer, fractional);
	} else {
		sprintf((char *)string, "%d", stat);
	}
	
	offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
	// Shift all the display to the middle
	_middle_horizontal_alignment_center(offset);
}

static void _middle_render_horizontal_active_time()
{
	const char *active_time_name[] = {"MIN","分钟","分鍾"};		
	I8U string[32];
	I8U b_24_size = 24;			
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 3;
	I32U stat;
	I8U language_type = cling.ui.language_type;

#ifdef _CLINGBAND_PACE_MODEL_		
	TRACKING_get_activity(0, TRACKING_ACTIVE_TIME, &stat);
#else
	TRACKING_get_activity(cling.ui.vertical_index, TRACKING_ACTIVE_TIME, &stat);
#endif
	
	len = sprintf((char *)string, "%d", stat);

	if (len == 1) 
		offset = 40;
	else if (len == 2)
		offset = 35;
  else 
    offset = 30;

	offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);

	FONT_load_characters(256+offset+10, (char *)active_time_name[language_type], 16, 128, FALSE);
}

#ifdef _CLINGBAND_UV_MODEL_		
static void _middle_render_horizontal_uv_index()
{
	I8U string[32];
	I8U b_24_size = 24;			
	I16U offset = 0;
	I8U margin = 3;
  I8U integer;

	integer = cling.uv.max_UI_uv;
	sprintf((char *)string, "%d.%d", (integer/10), (integer%10));

	offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
	// Shift all the display to the middle
	_middle_horizontal_alignment_center(offset);
}
#endif

static I8U _render_middle_horizontal_hr_core(BOOLEAN b_training_mode)
{
	const char *heart_rate_wave_indicator[] = {
		"-,,,,,,",
		",-,,,,,", 
		",,-,,,,", 
		",,,-,,,", 
		",,,,-,,", 
		",,,,,-,", 
		",,,,,,-"};	
	I8U string[32];
	I8U b_24_size = 24;				
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 3;
	I8U hr_result = 0;
	
	if (cling.hr.b_closing_to_skin || cling.hr.b_start_detect_skin_touch) {
		if (cling.hr.heart_rate_ready) {
		  hr_result = PPG_minute_hr_calibrate();
			len = sprintf((char *)string, "%d", hr_result);			
		} else {
			if (cling.ui.heart_rate_wave_index > 6) {
				cling.ui.heart_rate_wave_index = 0;
			}
			len = sprintf((char *)string, "%s", heart_rate_wave_indicator[cling.ui.heart_rate_wave_index]);
			_render_middle_horizontal_section_core(string, 8, 5, 128+40, 0);
			cling.ui.heart_rate_wave_index ++;
			return 0;
		}
	} else {
		N_SPRINTF("[UI] Heart rate - not valid");
		len = 0;
		string[len++] = ICON24_NO_SKIN_TOUCH_IDX;		
		string[len++] = 0;				
	}

	if (b_training_mode)
	  offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, len);
	else 
		offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
	// Shift all the display to the middle
	_middle_horizontal_alignment_center(offset);		
	
	return hr_result;
}

static void _middle_render_horizontal_heart_rate()
{
  _render_middle_horizontal_hr_core(FALSE);
}

#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)	
static void _middle_render_horizontal_skin_temp()
{
	I8U string[32];
	I8U b_24_size = 24;				
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 3;
  I8U integer, fractional;

  integer = cling.therm.current_temperature/10;
	fractional = cling.therm.current_temperature - integer * 10;
	len = sprintf((char *)string, "%d.%d", integer, fractional);
	string[len++] = ICON24_CELCIUS_IDX;
	string[len++] = 0;
	
	offset = _render_middle_horizontal_section_core(string,b_24_size,  margin, offset, 0);
	// Shift all the display to the middle
	_middle_horizontal_alignment_center(offset);
}
#endif

static void _middle_render_horizontal_weather()
{
	I8U string[32];
	I8U b_24_size = 24;			
	I8U len = 0;
	I16U offset = 10;
	I8U margin = 5;
	WEATHER_INFO_CTX weather_info;

	if (WEATHER_get_weather_info(0, &weather_info)) {
		len = 0;
		len += sprintf((char *)string+len, "%d", weather_info.low_temperature);
		string[len++] = ICON24_WEATHER_RANGE_IDX;
		len += sprintf((char *)string+len, "%d", weather_info.high_temperature);
		string[len++] = ICON24_CELCIUS_IDX;
		string[len++] = 0;
	} else {
		len = 0;
		len += sprintf((char *)string+len, "15");
		string[len++] = ICON24_WEATHER_RANGE_IDX;
		len += sprintf((char *)string+len, "22");
		string[len++] = ICON24_CELCIUS_IDX;
		string[len++] = 0;
	}
	
	offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);

	// Shift all the display to the middle
	_middle_horizontal_alignment_center(offset);
}

#ifndef _CLINGBAND_PACE_MODEL_		
static void _middle_render_horizontal_message()
{
	I8U string[32];
	I8U b_24_size = 24;		
	I8U offset=0;
	I8U margin=3;
	
#ifdef _ENABLE_ANCS_				
	sprintf((char *)string, "%d", NOTIFIC_get_message_total());
#else	
	sprintf((char *)string, "0");
#endif	
	
	offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
	// Shift all the display to the middle
	_middle_horizontal_alignment_center(offset);
}

static void _middle_render_horizontal_app_notif()
{
	I8U len;
	I8U dis_len;
	I8U string[128];
	I16U offset = 0;
  BOOLEAN b_display_center = FALSE;
	
	len = NOTIFIC_get_app_name(cling.ui.app_notific_index, (char *)string);
	N_SPRINTF("[UI] app index: %d, %d, %s", cling.ui.app_notific_index, len, (char *)string);
	
	dis_len = FONT_get_string_display_len((char *)string);

	if (dis_len > 80) {
		offset = 22;
	} else {
		offset = 256;
		b_display_center = TRUE;	
	}
	
  FONT_load_characters(offset, (char *)string, 16, 80, b_display_center);		
		
	len = sprintf((char *)string, "%02d", cling.ui.app_notific_index);
	FONT_load_characters((128-len*8), (char *)string, 16, 128, FALSE);			
}
#endif

static void _middle_render_horizontal_pm2p5()
{
	I8U string[32];
	I8U b_24_size = 24;		
	I16U offset = 0;
	I8U margin = 3;
	WEATHER_CTX *w = &cling.weather;
												 
  if ((w->pm2p5_month == cling.time.local.month) && (w->pm2p5_day == cling.time.local.day)) {	
		if (w->pm2p5_value == 0xffff) {
			sprintf((char *)string, "0");
		} else {
			sprintf((char *)string, "%d", w->pm2p5_value);
		}
  } else {
		sprintf((char *)string, "0");
	}
	
	offset = 80;
	
	offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
	_middle_horizontal_alignment_center(offset);
}

static void _middle_render_horizontal_reminder_core(BOOLEAN b_alarm_clock_reminder)
{
	I8U string[32];
	I8U b_24_size = 24;			
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 3;
  BOOLEAN b_invalid_alarm = TRUE;

  if (b_alarm_clock_reminder) {
		if ((cling.reminder.total) || (cling.reminder.b_sleep_total)) {
			if ((cling.ui.ui_alarm_hh < 24) && (cling.ui.ui_alarm_mm < 60)) {
				b_invalid_alarm = FALSE;
				len = sprintf((char *)string, "%02d:%02d", cling.ui.ui_alarm_hh, cling.ui.ui_alarm_mm);		
			}				
		}			
	} else {
		if (cling.reminder.total) {
			if ((cling.ui.ui_alarm_hh < 24) && (cling.ui.ui_alarm_mm < 60)) {
				b_invalid_alarm = FALSE;
				len = sprintf((char *)string, "%02d:%02d", cling.ui.ui_alarm_hh, cling.ui.ui_alarm_mm);				
			}
		}		
	}

	if (b_invalid_alarm) {
		len = 0;
		string[len++] = ICON24_NO_SKIN_TOUCH_IDX;		
		string[len++] = 0;
	} 
	
	offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 2);
	// Shift all the display to the middle
	_middle_horizontal_alignment_center(offset);
}

static void _middle_render_horizontal_alarm_clock_reminder()
{
  _middle_render_horizontal_reminder_core(TRUE);
}

#ifndef _CLINGBAND_PACE_MODEL_	
static void _middle_render_horizontal_alarm_clcok_detail()
{
  _middle_render_horizontal_reminder_core(FALSE);
}
#endif

static void _middle_render_horizontal_incoming_call_or_message()
{
	I8U string[128];
	I16U dis_len=0;
	I16U offset = 0;
  BOOLEAN b_display_center = FALSE;

#ifndef _CLINGBAND_PACE_MODEL_	
	cling.ui.app_notific_index = 0;	
#endif	
	NOTIFIC_get_app_name(0, (char *)string);
	dis_len = FONT_get_string_display_len((char *)string);

	if (dis_len > 80) {
		offset = 22;
	} else {
		offset = 256;
		b_display_center = TRUE;	
	}

	FONT_load_characters(offset, (char *)string, 16, 80, b_display_center);
}

static void _middle_render_horizontal_detail_notif()
{
  I8U string_pos = 0;
	I8U string[128];
  I8U msg_len = 0;
	
#ifdef _CLINGBAND_PACE_MODEL_			
	msg_len = NOTIFIC_get_app_message_detail(0, (char *)string);
	
	if (msg_len == 0) {
	  NOTIFIC_get_app_name(0, (char *)string);	
    cling.ui.notif_detail_index = 0;		
	}	
#else 
	msg_len = NOTIFIC_get_app_message_detail(cling.ui.app_notific_index, (char *)string);
	
	if (msg_len == 0) {
	  NOTIFIC_get_app_name(cling.ui.app_notific_index, (char *)string);	
    cling.ui.notif_detail_index = 0;		
	}		
#endif
	
	if (cling.ui.notif_detail_index) {
		string_pos = cling.ui.string_pos_buf[cling.ui.notif_detail_index - 1];
	}
	
	FONT_load_characters(0, (char *)string+string_pos, 16, 112, FALSE);		
}

static void _middle_render_horizontal_idle_alert()
{
	const	char *idle_alart_name[] = {"time for move", "该活动一下了", "該活動一下了"};
	I8U offset = 0;
	I8U language_type = cling.ui.language_type;	
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) 
	  offset = 24;
	else 
		offset = 32;
	
	FONT_load_characters(256+offset, (char *)idle_alart_name[language_type], 16, 128, FALSE);
}

#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)	
static void _middle_render_horizontal_sos_alert()
{
	I8U string[32];
	I8U b_24_size = 24;				
	I16U offset = 0;
	I8U margin = 5;

	sprintf((char *)string, "SOS");

	offset = _render_middle_horizontal_section_core(string, b_24_size,  margin, offset, 0);
	// Shift all the display to the middle
	_middle_horizontal_alignment_center(offset);
}
#endif

#ifdef _CLINGBAND_VOC_MODEL_ 
static void _middle_render_horizontal_phone_finder()
{
	I16U offset = 0;

	_render_one_icon_24(ICON24_PHONE_FINDER_0_IDX, 128+offset);
	offset += ICON24_PHONE_FINDER_0_LEN;
	offset += 12;
	
	_render_one_icon_24(ICON24_PHONE_FINDER_1_IDX, 128+offset);
	offset += ICON24_PHONE_FINDER_1_LEN;
	offset += 12;
	
	_render_one_icon_24(ICON24_PHONE_FINDER_2_IDX, 128+offset);
	offset += ICON24_PHONE_FINDER_2_LEN;

	_middle_horizontal_alignment_center(offset);
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_			
static void _middle_render_horizontal_workout_mode_switch()
{
	const char *workout_name[3][8] = {{"Treadmill", "Cycling", "Stairs",  "Elliptical", "Row",  "Aerobic", "Piloxing", "Others"},
	                                  {"跑步机 ",   "单车",    "爬楼梯 ", "椭圆机 ",    "划船", "有氧操 ", "Piloxing", "其它"},
	                                  {"跑步機 ",   "單車",    "爬樓梯 ", "橢圓機 ",    "劃船", "有氧操 ", "Piloxing", "其它"}};
	const char *workout_indicator[] = {
		"-,,,,,,,",
		",-,,,,,,", 
		",,-,,,,,", 
		",,,-,,,,", 
		",,,,-,,,", 
		",,,,,-,,", 
		",,,,,,-,", 
		",,,,,,,-"};
	I8U workout_idx=0, frame_index=0;	
	I8U language_type = cling.ui.language_type;
	
  frame_index = cling.ui.frame_index;
	if ((frame_index < UI_DISPLAY_WORKOUT)	|| (frame_index > UI_DISPLAY_WORKOUT_OTHERS))
		return;
	
	workout_idx = frame_index - UI_DISPLAY_WORKOUT_TREADMILL;
		
	FONT_load_characters(128, (char *)workout_name[language_type][workout_idx], 16, 128, TRUE);
	
	_render_middle_horizontal_section_core((I8U *)workout_indicator[workout_idx], 8, 7, 256+30, 0);
}
#endif

static void _horizontal_core_run_distance(I32U stat, BOOLEAN b_all_hold)
{
	I8U string[32];
	I8U b_24_size = 24;			
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 3;
	I16U integer, fractional;

	// Convert to miles
	if (cling.user_data.profile.metric_distance) {
		stat *= 10;
		stat >>= 4;
	}
	
	if (stat > 99999)
		stat = 99999;
	integer = stat/1000;
	fractional = stat - integer * 1000;
	if (stat > 9999) {
		fractional /= 100;
		len = sprintf((char *)string, "%d.%d", integer, fractional);
	} else {
		fractional /= 10;
		len = sprintf((char *)string, "%d.%02d", integer, fractional);
	}

	if (len == 4)
		offset = 30;
	else 
	  offset = 22;
	
	if (b_all_hold)
	  _render_middle_horizontal_section_core(string, b_24_size, margin, offset, len);
	else 
	  _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
}

static void _middle_render_horizontal_running_distance()
{
	_horizontal_core_run_distance(cling.run_stat.distance, FALSE);
}

static void _middle_render_horizontal_training_distance()
{
	_horizontal_core_run_distance(cling.train_stat.distance, TRUE);
}

#ifdef _CLINGBAND_PACE_MODEL_
static void _middle_render_horizontal_running_analysis()
{
	const char *running_analysis_name[] = {"RUN DATA", "当日跑步 ", "當日跑步 "};
	I8U language_type = cling.ui.language_type;
	
	FONT_load_characters(128+24, (char *)running_analysis_name[language_type], 16, 128, FALSE);
}
#endif

static void _middle_render_horizontal_running_time()
{
	I8U string[32];
	I8U b_24_size = 24;
	I8U margin = 3;
	I16U offset = 0;
	I8U hour=0, min=0, sec=0;

	min = cling.run_stat.time_min;
	sec = cling.run_stat.time_sec;
	
	hour = min/60;
	min = min - hour*60;
	
	// Render the hour
	if (hour > 9) hour = 9;	
	sprintf((char *)string, "%d:", hour);
	offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 1);
	// Render the minute and second
	sprintf((char *)string, "%02d:%02d", min, sec);
	offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
	_middle_horizontal_alignment_center(offset);
}

static void _middle_render_horizontal_running_pace()
{
	I8U string[32];
	I8U b_24_size = 24;			
	I16U offset = 0;
	I8U margin = 2;
	I32U pace = cling.run_stat.time_min; // Rounding 
	I32U min, sec;
	
	if (cling.run_stat.distance) {
		pace = cling.run_stat.time_sec+cling.run_stat.time_min*60;
		pace *= 100000;
		pace /= 60;
		pace /= cling.run_stat.distance;
	} else {
		pace = 0;
	}
	
	min = pace / 100;
	sec = pace - min * 100;
	sec *= 60;
	sec /= 100;
	
	if (min > 24) {
		min = 0;
		sec = 0;
	}
		
	if (min > 9)
		offset = 22;
	else
    offset = 29;
	
	// Render the pace
	sprintf((char *)string, "%d/%02d*", min, sec);
	_render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
}

static void _middle_render_horizontal_running_cadence()
{
	I8U string[32];
	I8U b_24_size = 24;		
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 2;
	I32U stat;

	if (cling.run_stat.time_min)
		stat = cling.run_stat.steps/cling.run_stat.time_min;
	else
		stat = 0;
	len = sprintf((char *)string, "%d", stat);
	
	offset = ((80 - len*13 - (len-1)*2)/2) + 16;
	_render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
}

static void _middle_render_horizontal_running_stride()
{
	I8U string[32];
	I8U b_24_size = 24;			
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 2;
	I32U stat;

	if (cling.run_stat.steps)
		stat = (cling.run_stat.distance*100)/cling.run_stat.steps;
	else
		stat = cling.user_data.profile.stride_running_in_cm;

	// Convert to in.
	if (cling.user_data.profile.metric_distance) {
		stat *= 100;
		stat /= 254;
	}
	
	len = sprintf((char *)string, "%d", stat);
	
	offset = ((80 - len*13 - (len-1)*2)/2) + 16;
	
	_render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
}

static void _middle_render_horizontal_running_hr()
{
	I8U string[32];
	I8U b_24_size = 24;				
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 2;
	I32U hr = cling.run_stat.accu_heart_rate;
	
	if (cling.run_stat.time_min) {
		hr /= cling.run_stat.time_min;
	} else {
		hr = 0;
	}

	len = sprintf((char *)string, "%d", hr);
	
	offset = ((80 - len*13 - (len-1)*2)/2) + 16;
	
	_render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
}

static void _render_horizontal_calories_core(I32U stat)
{
	I8U string[32];
	I8U b_24_size = 24;				
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 2;
	
	len = sprintf((char *)string, "%d", stat);

	offset = ((80 - len*13 - (len-1)*2)/2) + 16;
	
	_render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
}

static void _middle_render_horizontal_running_calories()
{
	_render_horizontal_calories_core(cling.run_stat.calories);
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _middle_render_horizontal_training_calories()
{
	_render_horizontal_calories_core(cling.train_stat.calories);	
}
#endif

#ifdef _CLINGBAND_PACE_MODEL_
static void _middle_render_horizontal_running_stop_analysis()
{
	const char *analysis_end_name[] = {"INFO END", "结束分析 ", "結束分析 "};
	I8U language_type = cling.ui.language_type;
	
	FONT_load_characters(128+24, (char *)analysis_end_name[language_type], 16, 128, FALSE);
}
#endif

static void _middle_render_horizontal_training_start_run()
{
	const char *run_start_name[] = {"RUN NOW", "开始跑步 ", "開始跑步 "};
	I8U language_type = cling.ui.language_type;
  I8U offset = 0;
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {		
#ifdef _CLINGBAND_PACE_MODEL_		
	  offset = 28;
#else
	  offset = 35;
#endif	
	} else {
#ifdef _CLINGBAND_PACE_MODEL_				
	  offset = 24;	
#else
	  offset = 31;	
#endif		
	}
	
	FONT_load_characters(128+offset, (char *)run_start_name[language_type], 16, 128, FALSE);
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _middle_render_horizontal_training_run_or_analysis()
{
	const	char *start_name[] = {"RUN ", "开始 ", "開始 "};	
	const	char *analysis_name[] = {"MORE", "记录 ", "記錄 "};		
	I8U language_type = cling.ui.language_type;		
	I8U i;
	I8U *p0, *p1;
	
	memset(cling.ui.p_oled_up+128+40, 0, 32);
	memset(cling.ui.p_oled_up+128+128+40, 0, 32);
	memset(cling.ui.p_oled_up+128+96, 0, 32);
	memset(cling.ui.p_oled_up+128+128+96, 0, 32);	
	
	FONT_load_characters(128+40, (char *)start_name[language_type], 16, 128, FALSE);
 	FONT_load_characters(128+96, (char *)analysis_name[language_type], 16, 128, FALSE);
	
  p0 = cling.ui.p_oled_up+128+40,
	p1 = p0+128;
	
	for (i=0;i<32;i++) {
	 *(p0+i) = ~(*(p0+i));
	 *(p1+i) = ~(*(p1+i));	
	}
	
  p0 = cling.ui.p_oled_up+128+96,
	p1 = p0+128;
	
	for (i=0;i<32;i++) {
	 *(p0+i) = ~(*(p0+i));
	 *(p1+i) = ~(*(p1+i));	
	}	
}
#endif

static BOOLEAN _middle_render_horizontal_run_ready_core()
{
	const char *ready_indicator_name[] = {"3","2","1","GO"};
	I8U string[32];
	I8U b_24_size = 24;			
	I8U len = 0;
	I16U offset = 60;
	I8U margin = 5;
  I32U t_curr = CLK_get_system_time();
	BOOLEAN b_ready_finished = FALSE;	
	
	if (t_curr  > (cling.ui.running_time_stamp + 700)) {
		cling.ui.running_time_stamp = t_curr;		
		cling.ui.run_ready_index++;
	}

	if (cling.ui.run_ready_index > 3) {
		cling.ui.run_ready_index = 3;
		b_ready_finished = TRUE;
	}
	
	if (cling.ui.run_ready_index == 3)
		offset = 53;
	
	len = sprintf((char *)string, "%s", ready_indicator_name[cling.ui.run_ready_index]);
		
  _render_middle_horizontal_section_core(string, b_24_size, margin, offset, len);

	return b_ready_finished;
}

static void _middle_render_horizontal_training_ready()
{
	BOOLEAN b_ready_finished = FALSE;
	
  b_ready_finished = _middle_render_horizontal_run_ready_core();
	
	if (b_ready_finished) {
		cling.ui.frame_index = UI_DISPLAY_TRAINING_STAT_TIME;
	  cling.ui.frame_next_idx = cling.ui.frame_index;
	}
}

static void _get_training_time_core(I8U *p_hour, I8U *p_min, I8U *p_sec)
{
	I32U curr_ts_ms = CLK_get_system_time();
	I32U diff_ts_ms, diff_ts_sec;
	I8U hour, min, sec;
	
	if (cling.ui.b_training_first_enter) {
		hour = 0;
		min = 0;
		sec = 0;
		cling.ui.b_training_first_enter = FALSE;
		cling.train_stat.time_start_in_ms = CLK_get_system_time();
	} else {
	  diff_ts_ms = curr_ts_ms - cling.train_stat.time_start_in_ms;
	  diff_ts_sec = diff_ts_ms/1000;
	  hour = diff_ts_sec/3600;
	  min = (diff_ts_sec - hour*3600)/60;
	  sec = (diff_ts_sec - hour*3600 - min*60);
	}
	
	*p_hour = hour;
	*p_min = min;
	*p_sec = sec;
}

static void _middle_render_horizontal_training_time()
{
	I8U string[32];
	I8U len = 0;
	I8U b_24_size = 24;
	I16U offset = 0;
	I8U margin = 3;
	I8U hour, min, sec;
	
	_get_training_time_core(&hour, &min, &sec);

	// Render the training time.
	if (hour > 9) hour = 9;		
	
	if (hour) {
	  len = sprintf((char *)string, "%d:%02d%02d", hour, min, sec);		
	} else {
	  len = sprintf((char *)string, "%d:%02d", min, sec);				
	}

	offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, len);	
	
	_middle_horizontal_alignment_center(offset);
}

static void _middle_render_horizontal_training_pace()
{
	I8U string[32];
	I8U b_24_size = 24;				
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 2;
	I32U min, sec;
	
	min = cling.run_stat.last_10sec_pace_min;
	sec = cling.run_stat.last_10sec_pace_sec;
	
  if (min > 9)	
		offset = 22;
	else 
		offset = 29;
	
	// Render the pace
	len = sprintf((char *)string, "%d/%02d*", min, sec);

	_render_middle_horizontal_section_core(string, b_24_size, margin, offset, len);
}

static void _middle_render_horizontal_training_hr()
{
	I32U hr_perc = 0;		
	I8U hr_result = 0;

  hr_result = _render_middle_horizontal_hr_core(TRUE);

	if (hr_result) {
		hr_perc = (hr_result * 100)/(220-cling.user_data.profile.age);	
		if (hr_perc > 98)
			hr_perc = 98;
  }
	
	cling.ui.training_hr = (I8U)hr_perc;
}

static void _middle_render_horizontal_training_run_stop()
{
	const char *run_stop_name[] = {"STOP RUN", "结束跑步 ", "结束跑步 "};

	I8U language_type = cling.ui.language_type;

	FONT_load_characters(128+31, (char *)run_stop_name[language_type], 16, 128, FALSE);
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _middle_render_horizontal_training_workout_stop()
{
	const char *run_stop_name[] = {"STOP NOW", "结束运动 ", "結束運動 "};

	I8U language_type = cling.ui.language_type;

	FONT_load_characters(128+31, (char *)run_stop_name[language_type], 16, 128, FALSE);
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_
static void _middle_render_horizontal_workout_ready()
{	
	BOOLEAN b_ready_finished = FALSE;
	
  b_ready_finished = _middle_render_horizontal_run_ready_core();
	
	if (b_ready_finished) {
		cling.ui.frame_index = UI_DISPLAY_WORKOUT_RT_TIME;
	  cling.ui.frame_next_idx = cling.ui.frame_index;
	}
}

static void _middle_render_horizontal_cycling_outdoor_start()
{
	const char *cycling_start_name[] = {"BIKE NOW", "开始骑行 ", "開始騎行 "};
	I8U language_type = cling.ui.language_type;

	FONT_load_characters(128+31, (char *)cycling_start_name[language_type], 16, 128, FALSE);
}

static void _middle_render_horizontal_cycling_outdoor_ready()
{
	BOOLEAN b_ready_finished = FALSE;
	
  b_ready_finished = _middle_render_horizontal_run_ready_core();
	
	if (b_ready_finished) {
		cling.ui.frame_index = UI_DISPLAY_CYCLING_OUTDOOR_STAT_TIME;
	  cling.ui.frame_next_idx = cling.ui.frame_index;
	}
}

static void _middle_render_horizontal_cycling_outdoor_distance()
{
	if (BTLE_is_connected()) {
		_horizontal_core_run_distance(cling.train_stat.distance, TRUE);
	} else {
		_render_one_icon_24(ICON24_NO_SKIN_TOUCH_IDX, 128+40);
	}
}

static void _middle_render_horizontal_cycling_outdoor_speed()
{
	if (BTLE_is_connected()) {
		_horizontal_core_run_distance(cling.train_stat.speed, TRUE);
	} else {
		_render_one_icon_24(ICON24_NO_SKIN_TOUCH_IDX, 128+40);
	}
}

static void _middle_render_horizontal_cycling_outdoor_stop()
{
	const char *cycling_stop_name[] = {"STOP BIKE", "结束骑行 ", "結束騎行 "};
	I8U language_type = cling.ui.language_type;

	FONT_load_characters(128+30, (char *)cycling_stop_name[language_type], 16, 128, FALSE);
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_
static void _middle_render_horizontal_stopwatch_start()
{
	I8U string[32];
	I8U b_24_size = 24;
	I8U margin = 3;	
	I8U offset = 0;
	I8U hour = 0, min = 0, sec = 0;
	I16U ms = 0;		
  I32U t_diff = 0;

  if (cling.ui.b_stopwatch_first_enter) {
  	cling.ui.stopwatch_time_stamp = CLK_get_system_time();				
		cling.ui.b_in_stopwatch_pause_mode = FALSE;
		cling.ui.b_stopwatch_first_enter = FALSE;
		cling.ui.stopwatch_t_stop_stamp = 0;
	} else {
	  if (!cling.ui.b_in_stopwatch_pause_mode)  {
      t_diff = CLK_get_system_time() - cling.ui.stopwatch_time_stamp;			
	    cling.ui.stopwatch_t_stop_stamp = t_diff;			
		} else {
	    t_diff = cling.ui.stopwatch_t_stop_stamp;
			cling.ui.stopwatch_time_stamp = CLK_get_system_time() - cling.ui.stopwatch_t_stop_stamp;
		}
	}

	ms = t_diff % 1000;
	t_diff /= 1000;
	hour = t_diff / 3600;
	t_diff -= hour*3600;
	min = t_diff / 60;
	t_diff -= min * 60;
	sec = t_diff;
	
	ms /= 10;
	
	// Render the time	
	if (hour) {
		// Render the hour	
		if (hour > 9) hour = 9;		
		sprintf((char *)string, "%d:", hour);
	  offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 1);
		// Render the minute				
		sprintf((char *)string, "%02d:", min);
	  offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);		
		// Render the second				 
		sprintf((char *)string, "%02d", sec);
	  offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);		
		
		_middle_horizontal_alignment_center(offset);
	} else {
		// Render the minute	
		sprintf((char *)string, "%d:", min);
	  offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
		// Render the second				
		sprintf((char *)string, "%02d.", sec);
	  offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);		
		// Render the millisecond
		b_24_size = 16;
		offset += 128;
		if (ms > 99) ms = 99;		
		sprintf((char *)string, "%02d", ms);
	  offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);		
		offset -= 128;
		_middle_horizontal_alignment_center(offset);
	}	
}

static void _middle_render_horizontal_stopwatch_stop()
{
	const char *stopwatch_stop_name[] = {"END", "结束 ", "結束 "};
	I8U language_type = cling.ui.language_type;

	FONT_load_characters(128, (char *)stopwatch_stop_name[language_type], 16, 128, TRUE);
}
#endif

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
static void _middle_render_horizontal_music_play()
{
	I16U offset = 10;

	_render_one_icon_24(ICON24_MUSIC_PLAY_IDX, 128+offset);
	offset += ICON24_MUSIC_PLAY_LEN;
	offset += 20;
	
	_render_one_icon_24(ICON24_MUSIC_MUTE_IDX, 128+offset);
	offset += ICON24_MUSIC_MUTE_LEN;
	offset += 20;
	
	_render_one_icon_16(ICON16_MUSIC_MORE_IDX, 128+128+offset);
	offset += ICON16_MUSIC_MORE_LEN;

	_middle_horizontal_alignment_center(offset);
}

static void _middle_render_horizontal_music_track()
{
	I16U offset = 10;

	_render_one_icon_24(ICON24_MUSIC_PREV_SONG_IDX, 128+offset);
	offset += ICON24_MUSIC_PREV_SONG_LEN;
	offset += 20;
	
	_render_one_icon_24(ICON24_MUSIC_NEXT_SONG_IDX, 128+offset);
	offset += ICON24_MUSIC_NEXT_SONG_LEN;
	offset += 20;
	
	_render_one_icon_16(ICON16_MUSIC_MORE_IDX, 128+128+offset);
	offset += ICON16_MUSIC_MORE_LEN;

	_middle_horizontal_alignment_center(offset);
}

static void _middle_render_horizontal_music_volume()
{
	I16U offset = 10;

	_render_one_icon_24(ICON24_MUSIC_VOLUME_DOWN_IDX, 128+offset);
	offset += ICON24_MUSIC_VOLUME_DOWN_LEN;
	offset += 20;
	
	_render_one_icon_24(ICON24_MUSIC_VOLUME_UP_IDX, 128+offset);
	offset += ICON24_MUSIC_VOLUME_UP_LEN;
	offset += 20;
	
	_render_one_icon_16(ICON16_MUSIC_MORE_IDX, 128+128+offset);
	offset += ICON16_MUSIC_MORE_LEN;

	_middle_horizontal_alignment_center(offset);	
}
#endif

#ifdef _CLINGBAND_2_PAY_MODEL_
static void _middle_render_horizontal_balance_core(BOOLEAN b_bus_card_balance)
{
	const char *unit_balance_display[] = {"RMB", "元 ", "元 "};	
	I8U string[32];
	I8U len = 0;
	I8U b_24_size = 24;	
	I16U offset = 0;
	I8U margin = 3;
	I32U balance = 0;
	I32U integer, fractional;
	I8U language_type = cling.ui.language_type;

	if (b_bus_card_balance)
		balance = cling.ui.bus_card_balance;
	else
		balance = cling.ui.bank_card_balance;
	
	if (balance > 99999)
		b_24_size = 16;
	
	if (balance > 99999999)
		balance = 99999999;
	
	integer = balance/100;
	fractional = balance - integer * 100;
	
	// Render integer.		
	len = sprintf((char *)string, "%d", integer);
	if (len >= 3)
		margin = 2;
	
	if (len >= 6)
		margin = 1;

	if (b_24_size == 16)
		offset += 128;
	offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);

	if (b_24_size == 16)
		offset -= 128;
	b_24_size = 24;
	offset = _render_middle_horizontal_section_core((I8U *)".", b_24_size, margin, offset, 0);
	
	// Render fractional.	
	sprintf((char *)string, "%02d", fractional);	
	b_24_size = 16;	
	offset += 128;
	offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
	offset -= 128;
	
	_middle_horizontal_alignment_center(offset);
	
	if (language_type == LANGUAGE_TYPE_ENGLISH)
	  FONT_load_characters(256+104, (char *)unit_balance_display[language_type], 16, 128, FALSE);	
	else 
		FONT_load_characters(256+112, (char *)unit_balance_display[language_type], 16, 128, FALSE);	
}

static void _middle_render_horizontal_bus_card_balance_enquiry()
{
  _middle_render_horizontal_balance_core(TRUE);
}

static void _middle_render_horizontal_bank_card_balance_enquiry()
{
  _middle_render_horizontal_balance_core(FALSE);
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_
static void _middle_render_horizontal_carousel_core(I8U left_idx, I8U middle_idx, I8U right_idx)
{
  I8U offset = 128;
	
  _render_one_icon_24(left_idx, offset);	
	offset += 52;

  _render_one_icon_24(middle_idx, offset);	
	offset += 52;
	
  _render_one_icon_24(right_idx, offset);	
}

static void _middle_render_horizontal_carousel_1()
{
  _middle_render_horizontal_carousel_core(ICON24_RUNNING_MODE_IDX, ICON24_CYCLING_OUTDOOR_MODE_IDX, ICON24_WORKOUT_MODE_IDX);
}

static void _middle_render_horizontal_carousel_2()
{
#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)		
  _middle_render_horizontal_carousel_core(ICON24_MUSIC_IDX, ICON24_STOPWATCH_IDX, ICON24_MESSAGE_IDX);	
#endif	
	
#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	
  _middle_render_horizontal_carousel_core(ICON24_MESSAGE_IDX, ICON24_STOPWATCH_IDX, ICON24_WEATHER_IDX);	
#endif
}

static void _middle_render_horizontal_carousel_3()
{
#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	
  _middle_render_horizontal_carousel_core(ICON24_PM2P5_IDX, ICON24_NORMAL_ALARM_CLOCK_IDX, ICON24_SETTING_IDX);	
#endif
	
#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
  _middle_render_horizontal_carousel_core(ICON24_WEATHER_IDX, ICON24_PM2P5_IDX, ICON24_NORMAL_ALARM_CLOCK_IDX);	
#endif	
}

#ifdef _CLINGBAND_2_PAY_MODEL_
static void _middle_render_horizontal_carousel_4()
{
  I8U offset = 128;
	
  _render_one_icon_24(ICON24_BUS_CARD_IDX, offset);	
	offset += 48;

  _render_one_icon_24(ICON24_BANK_CARD_IDX, offset);	
	offset += 48;
	
  _render_one_icon_24(ICON24_SETTING_IDX, offset);			
}
#endif
#endif

static void _right_render_horizontal_string_core(const char *string1, const char *string2)
{
	I16U string1_len = 0, string2_len = 0;
	I8U offset1 = 0, offset2 = 0;
	
	string1_len = FONT_get_string_display_len((char *)string1);
	if (string1_len > 128)
	  return;
	offset1 = 128 - string1_len;
	FONT_load_characters(offset1, (char *)string1, 16, 128, FALSE);
	
	string2_len = FONT_get_string_display_len((char *)string2);
	if (string2_len > 128)
	  return;
	offset2 = 128 - string2_len;
	FONT_load_characters(256 + offset2, (char *)string2, 16, 128, FALSE);
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _right_render_horizontal_delta_data_backward()
{
  I8U string[16];
  I8U len;
	SYSTIME_CTX delta;

	RTC_get_delta_clock_backward(&delta, cling.ui.vertical_index);

	len = sprintf((char *)string, "%d/%02d", delta.month, delta.day);
	FONT_load_characters((128-len*6), (char *)string, 8, 128, FALSE);
}
#endif

static void _get_home_frame_stiring_core(I8U *string1, I8U *string2)
{

	const char *week_display[3][7] = {{"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"},
	                                 {"周一", "周二", "周三", "周四", "周五", "周六", "周日"},
	                                 {"周壹", "周二", "周三", "周四", "周五", "周六", "周日"}};
 	BOOLEAN b_batt_charging = FALSE;	
	I8U language_type = cling.ui.language_type;	
	I8U dow_index = cling.time.local.dow;

  if (dow_index >= 6)	
		dow_index = 6;
	
	if (BATT_is_charging()) 
		b_batt_charging = TRUE;
 
 	if (b_batt_charging) {
		if (cling.ui.clock_orientation == 1) {
		  sprintf((char *)string1, "%d %%", cling.system.mcu_reg[REGISTER_MCU_BATTERY]);		
		} else {
			sprintf((char *)string1, "%d%%", cling.system.mcu_reg[REGISTER_MCU_BATTERY]);	
		}
	} else {
	  sprintf((char *)string1, "%d/%02d", cling.time.local.month, cling.time.local.day);
	}
	
	sprintf((char *)string2, "%s", week_display[language_type][dow_index]);
}

static void _right_render_horizontal_home()
{
	I8U string1[16];
	I8U string2[16];					
 	
	_get_home_frame_stiring_core(string1, string2);

	FONT_load_characters((128 -  strlen((char *)string1)*6), (char *)string1, 8, 128, FALSE);
	
	FONT_load_characters(+256+96, (char *)string2, 16, 128, FALSE);	
}

static void _right_render_horizontal_small_clock()
{
	I8U string[16];
	I8U len;

	if (cling.ui.icon_sec_blinking) {
		len = sprintf((char *)string, "%d:%02d",cling.time.local.hour, cling.time.local.minute);
	} else {
		len = sprintf((char *)string, "%d %02d",cling.time.local.hour, cling.time.local.minute);		
	}
 
	FONT_load_characters((128-len*6), (char *)string, 8, 128, FALSE);
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _right_render_horizontal_stopwatch_start()
{
	if (cling.ui.b_in_stopwatch_pause_mode)
    _render_one_icon_16(ICON16_STOPWATCH_START_IDX, 110);
	else
    _render_one_icon_16(ICON16_STOPWATCH_STOP_IDX, 110);	
}
#endif

#ifdef _CLINGBAND_PACE_MODEL_
static void _right_render_horizontal_button_hold()
{
	const	char *hold_name[] = {"HOLD", "长按 ", "長按 "};
	I8U language_type = cling.ui.language_type;	
	I8U i;
	I8U *p0, *p1;
	
	FONT_load_characters(128+96, (char *)hold_name[language_type], 16, 128, FALSE);	
	
  p0 = cling.ui.p_oled_up+128+96,
	p1 = p0+128;

	for (i=0;i<32;i++) {
	 *(p0+i) = ~(*(p0+i));
	 *(p1+i) = ~(*(p1+i));	
	}
}
#endif

static void _right_render_horizontal_more()
{	
  if (cling.ui.b_detail_page) {
	 	_render_one_icon_8(ICON8_MORE_IDX, cling.ui.p_oled_up+384+115);	
  }
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _right_render_horizontal_tracker()
{
	if (!cling.ui.vertical_index) {
		_right_render_horizontal_small_clock();
  } else {
		_right_render_horizontal_delta_data_backward();
	}
	
	_right_render_horizontal_more();
}

static void _right_render_horizontal_app_notif()
{
	I8U string[16];
	I8U len;
	
	len = sprintf((char *)string, "%02d", cling.ui.app_notific_index);
	FONT_load_characters((128-len*8), (char *)string, 16, 128, FALSE);	

	_right_render_horizontal_more();
}

static void _right_render_horizontal_reminder()
{
	I8U string[16];
	I8U len;

	len = sprintf((char *)string, "%02d", cling.ui.vertical_index);
	FONT_load_characters((128-len*8), (char *)string, 16, 128, FALSE);	

	_right_render_horizontal_more();
}
#endif

static void _right_render_horizontal_firmware_ver()
{
	I8U string[16];
	I16U len;
	I16U major;
	I16U minor;
	
	major = cling.system.mcu_reg[REGISTER_MCU_REVH]>>4;
	minor = cling.system.mcu_reg[REGISTER_MCU_REVH]&0x0f;
	minor <<= 8;
	minor |= cling.system.mcu_reg[REGISTER_MCU_REVL];
	
//#ifdef _ENABLE_TOUCH_
//	len = sprintf((char *)string, "VER:%d.%d(%d)", major, minor,cling.whoami.touch_ver[2]);
//#else  
	len = sprintf((char *)string, "VER:%d.%d", major, minor);
//#endif	

	FONT_load_characters((128-len*6), (char *)string, 8, 128, FALSE);
}

static void _right_render_horizontal_running_pace()
{	
	const	char *run_name[] = {"RUN", "运动 ", "運動 "};	
	const	char *pace_name[] = {"PACE", "配速 ", "配速 "};
	I8U language_type = cling.ui.language_type;	
	
	_right_render_horizontal_string_core(run_name[language_type], pace_name[language_type]);
}

static void _right_render_horizontal_running_calories()
{
	const	char *cals_name[] = {"BURN", "热量", "熱量"};
	const	char *cals_unit_name[] = {"CALS", "大卡", "大卡"};
	I8U language_type = cling.ui.language_type;
	
	_right_render_horizontal_string_core(cals_name[language_type], cals_unit_name[language_type]);
}

static void _right_render_horizontal_running_cadence()
{
	const	char *rate_name[] = {"RATE", "步频", "步頻"};
	const	char *spm_name[] = {"SPM", "步分", "步分"};
	I8U language_type = cling.ui.language_type;	
	
	_right_render_horizontal_string_core(rate_name[language_type], spm_name[language_type]);
}

static void _right_render_horizontal_running_stride()
{
	const	char *stride_name[] = {"STRIDE", "步幅", "步幅"};
	const char *unit_stride_display[3][2] = {{"CM", "IN"},{"厘米", "英寸"},{"厘米", "英寸"}};	
	I8U language_type = cling.ui.language_type;	
	I8U metric = cling.user_data.profile.metric_distance;	

	_right_render_horizontal_string_core(stride_name[language_type], unit_stride_display[language_type][metric]);
}

static void _right_render_horizontal_running_hr()
{
	const	char *avg_name[] = {"AVG", "平均 ", "平均 "};		
	const	char *hate_rate_name[] = {"HR", "心率 ", "心率 "};	
	I8U language_type = cling.ui.language_type;

	_right_render_horizontal_string_core(avg_name[language_type], hate_rate_name[language_type]);
}

static void _right_render_horizontal_training_pace()
{
	const	char *pace_name[] = {"PACE", "配速 ", "配速 "};
	I8U language_type = cling.ui.language_type;

	_right_render_horizontal_string_core(pace_name[language_type], NULL);
}

static void _right_render_horizontal_run_distance_name()
{
	const char *unit_distance_display[3][2] = {{"KM", "ML"},{"公里", "英里"},{"公裏" ,"英裏"}};	
	I8U language_type = cling.ui.language_type;	
	I8U metric = cling.user_data.profile.metric_distance;	

	_right_render_horizontal_string_core(NULL, unit_distance_display[language_type][metric]);	
}
	
static void _right_render_horizontal_running_distance()
{
	const	char *distance_name[] = {"RUN", "里程", "裏程"};
	I8U language_type = cling.ui.language_type;	

	_right_render_horizontal_string_core(distance_name[language_type], NULL);	
	
	_right_render_horizontal_run_distance_name();
}

static void _right_render_horizontal_training_hr()
{
	I8U string[32];
	I8U len = 0;
	I8U hr_perc = 0;
	I8U *p0, *p1, *p2, *p3;
  I8U i;
	
	if (cling.hr.heart_rate_ready)
    hr_perc = cling.ui.training_hr;

	len = sprintf((char *)string, "%d%%", hr_perc);
	FONT_load_characters(128-(len*8), (char *)string, 16, 128, FALSE);
	
	p0 = cling.ui.p_oled_up+384+100;
	p1 = p0 + 7;
	p2 = p1 + 7;
	p3 = p2 + 7;
	
	*p0++ = 0xff;
	*p1++ = 0xff;
	*p2++ = 0xff;
	*p3++ = 0xff;
	for (i = 1; i < 4; i++) {
		if (!hr_perc) {
			*p0++ = 0x81;
			*p1++ = 0x81;
			*p2++ = 0x81;
			*p3++ = 0x81;			
		} else if ((hr_perc) && (hr_perc <= 25)) {
			*p0++ = 0xff;
			*p1++ = 0x81;
			*p2++ = 0x81;
			*p3++ = 0x81;
		} else if ((hr_perc > 25) && (hr_perc <= 50)) {
			*p0++ = 0xff;
			*p1++ = 0xff;
			*p2++ = 0x81;
			*p3++ = 0x81;
		} else if ((hr_perc > 50) && (hr_perc <= 75)) {
			*p0++ = 0xff;
			*p1++ = 0xff;
			*p2++ = 0xff;
			*p3++ = 0x81;
		} else {
			*p0++ = 0xff;
			*p1++ = 0xff;
			*p2++ = 0xff;
			*p3++ = 0xff;
		}
	}
	*p0++ = 0xff;
	*p1++ = 0xff;
	*p2++ = 0xff;
	*p3++ = 0xff;
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _right_render_horizontal_ok_middle()
{
	_render_one_icon_16(ICON16_OK_IDX , 128+110);
}

static void _right_render_horizontal_ok_top()
{
	_render_one_icon_16(ICON16_OK_IDX, 110);
}

static void _right_render_horizontal_cycling_outdoor_no_ble()
{
	const char *no_ble_name[] = {"NO BLE", "无蓝牙 ", "無藍牙 "};	
	I8U language_type = cling.ui.language_type;	
	
	_right_render_horizontal_string_core(NULL, no_ble_name[language_type]);	
}
	
static void _right_render_horizontal_cycling_outdoor_distance()
{
	if (BTLE_is_connected()) {
		_right_render_horizontal_run_distance_name();
	} else {
		_right_render_horizontal_cycling_outdoor_no_ble();
	}
}

static void _right_render_horizontal_cycling_outdoor_speed()
{
	const char *unit_speed_display[] = {"KM/H", "ML/H"};	
	I8U metric = cling.user_data.profile.metric_distance;	
	
	if (BTLE_is_connected()) {
	  FONT_load_characters(384+104, (char *)unit_speed_display[metric], 8, 128, FALSE);	
	} else {
		_right_render_horizontal_cycling_outdoor_no_ble();
	}
}
#endif

/***************************************************************************/
/************************** Vertical display page **************************/
/***************************************************************************/
static void _rotate_8_bytes_opposite_core(I8U *in_data, I8U *out_data)
{
	I8U i, j;
	for (j = 0; j < 8; j++) {
		for (i = 0; i < 8; i ++) {
			*(out_data-i) |= (((*in_data)<<i) & 0x80)>>j;
		}
		
		in_data ++;
	}
}

static void _rotate_270_degree(I8U *in, I16U offset)
{
	if (offset > (384 + 120))
		return;
		
	I8U *in_data;
	I8U *out = cling.ui.p_oled_up+offset;
	I8U *out_data;
	
	// first 8 bytes
	out_data = out+7;
	in_data = in;
	_rotate_8_bytes_opposite_core(in_data, out_data);
	
	// 2nd 8 bytes
	out_data = out +7 - 128;
	in_data = in+8;
	_rotate_8_bytes_opposite_core(in_data, out_data);

	// 3rd 8 bytes
	out_data = out + 7 - 256;
	in_data = in + 16;
	_rotate_8_bytes_opposite_core(in_data, out_data);

	// 4th 8 bytes
	out_data = out + 7 - 384;
	in_data = in + 24;
	_rotate_8_bytes_opposite_core(in_data, out_data);
}

static void _vertical_centerize(I8U *buf1, I8U *buf2, I8U *buf3, I8U len)
{
	I8U i;
	
	// Shift the line to the center
	len = 32 - len;
	len >>= 1;
	if (len > 0) {
		for (i = 0; i < (32-len); i++) {
			buf1[31-i] = buf1[31-i-len];
			buf2[31-i] = buf2[31-i-len];
			buf3[31-i] = buf3[31-i-len];
		}
		for (i = 0; i < len; i++) {
			buf1[len - 1 - i] = 0;
			buf2[len - 1 - i] = 0;
			buf3[len - 1 - i] = 0;
		}
	}
}	

static void _render_vertical_fonts_lib_character_core(I8U *string, I8U height, I8U offset)
{
	I8U buf_fonts[256];	
	I8U *p0, *p1, *p2;
	I8U line_len;
	p0 = buf_fonts;
	p1 = p0 + 128;
	p2 = p1 + 32;
	
	memset(buf_fonts, 0, 256);
	
	line_len = FONT_load_characters(offset, (char *)string, height, 128, FALSE);
	
	memcpy(buf_fonts, cling.ui.p_oled_up+offset, line_len);
	memset(cling.ui.p_oled_up+offset, 0, line_len);
	if (height == 16) {
	  memcpy(buf_fonts+128, cling.ui.p_oled_up+offset+128, line_len);
		memset(cling.ui.p_oled_up+offset+128, 0, line_len);	
	}

	// Shift to the center
	_vertical_centerize(p0, p1, p2, line_len);
	
	// do the rotation
	_rotate_270_degree(p0, 384+offset);
	if (height == 16)
	  _rotate_270_degree(p1, 384+offset+8);
}

static void _middle_render_vertical_character_core(I8U *string1, I8U offset1, I8U *string2, I8U offset2)
{
  _render_vertical_fonts_lib_character_core(string1, 16, offset1);
	
	_render_vertical_fonts_lib_character_core(string2, 16, offset2);
}

static void _render_vertical_local_character_core(I8U *string, I8U offset, I8U margin, I8U b_24_size, BOOLEAN b_all_hold)
{
	I8U *p0, *p1, *p2;
	I8U char_len, i, j, line_len;
	I8U *buf1;
	I8U *buf2;
	I8U *buf3;
	I8U buf_fonts[256];
	const I8U *pin;
  I8U	len;
	
	len = strlen((char *)string);
	
	buf1 = buf_fonts;
	buf2 = buf1 + 128;
	buf3 = buf2 + 32;
	
	memset(buf_fonts, 0, 256);
	
	p0 = buf1;
	p1 = buf2;
	p2 = buf3;

	line_len = 0;
	
	for (i = 0; i < len; i++) {
		// Digits in large fonts
		if (b_24_size == 24) {
			if (b_all_hold) {
        if (string[i] == ' ') {
				  p0 += 4;
				  p1 += 4;
				  p2 += 4;
				  char_len = 0;
				  line_len += 4;	
			  } else if (string[i] == '+') {
				  p0 += 13;
			  	p1 += 13;
				  p2 += 13;
				  char_len = 0;
				  line_len += 13;	
			  } else {				
				  pin = asset_content+asset_pos[512+string[i]];
				  char_len = asset_len[512+string[i]];	
				}					
			} else {
          if (string[i] == ' ') {
				  p0 += 4;
			  	p1 += 4;
				  p2 += 4;
				  char_len = 0;
				  line_len += 4;	
		  	} else if (string[i] == '+') {
				  p0 += 13;
				  p1 += 13;
				  p2 += 13;
				  char_len = 0;
				  line_len += 13;	
			  } else {				
				  if ((string[i] >= '0') && (string[i] <= '9')) {
					  pin = asset_content+asset_pos[512+string[i]+152];
					  char_len = asset_len[512+string[i]+152];
				  } else {
					  pin = asset_content+asset_pos[512+string[i]];
					  char_len = asset_len[512+string[i]];
				  }		
			  }				
			}

			for (j = 0; j < char_len; j++) {
					*p0++ = (*pin++);
					*p1++ = (*pin++);
					*p2++ = (*pin++);
			}
		} else if (b_24_size == 16) {
			if (string[i] == ' ') {
			  p0 += 4;
			  p1 += 4;
			  p2 += 4;
			  char_len = 0;
			  line_len += 4;	
		  }	else if (string[i] == '.') {
			  pin = asset_content+asset_pos[512+string[i]];
			  char_len = asset_len[512+string[i]];				
				for (j = 0; j < char_len; j++) {
					*p0++ = (*pin++);
					*p1++ = (*pin++);
					*p2++ = (*pin++);
			  }			
			} else {
			  pin = asset_content+asset_pos[256+string[i]];
			  char_len = asset_len[256+string[i]];
			  for (j = 0; j < char_len; j++) {
					*p0++ = (*pin++);
					*p1++ = (*pin++);
					*p2++ = 0;
			  }				
			}
		} else {
			pin = asset_content+asset_pos[string[i]];
			char_len = asset_len[string[i]];
			for (j = 0; j < char_len; j++) {
					*p0++ = (*pin++);
					*p1++ = 0;
					*p2++ = 0;
			}
		}
		line_len += char_len;
		p0 += margin;
		p1 += margin;
		p2 += margin;
		if (i != (len-1)) {
			line_len += margin;
		}
	}
	// Shift the line to the center
	_vertical_centerize(buf1, buf2, buf3, line_len);
	
	// do the rotation
	_rotate_270_degree(buf1, 384+offset);
	if (b_24_size != 8)
		_rotate_270_degree(buf2, 384+offset+8);
	if (b_24_size == 24)
		_rotate_270_degree(buf3, 384+offset+16);
}

static void _render_vertical_fonts_lib_invert_colors_core(I8U *string, I8U offset)
{
	I8U buf_fonts[256];	
	I8U *p0, *p1, *p2;
	I8U line_len;
	I8U i;

	p0 = buf_fonts;
	p1 = p0 + 128;
	p2 = p1 + 32;
	
	memset(buf_fonts, 0, 256);
	
	line_len = FONT_load_characters(offset, (char *)string, 16, 128, FALSE);
	
	memcpy(buf_fonts, cling.ui.p_oled_up+offset, line_len);
	memset(cling.ui.p_oled_up+offset, 0, line_len);
	memcpy(buf_fonts+128, cling.ui.p_oled_up+offset+128, line_len);
  memset(cling.ui.p_oled_up+offset+128, 0, line_len);	

	for (i=0;i<32;i++) {
		*(p0+i) = ~(*(p0+i));
		*(p1+i) = ~(*(p1+i));	
	}
	
	line_len = 32;

	// Shift to the center
	_vertical_centerize(p0, p1, p2, line_len);
	
	// do the rotation
	_rotate_270_degree(p0, 384+offset);
	_rotate_270_degree(p1, 384+offset+8);
}

static void _top_render_vertical_24_icon()
{
	I8U string[4];	
	I8U len = 0;	
	I8U	margin = 2;
	I8U b_24_size = 24;

	if (cling.ui.frm_render.vertical_icon_24_idx == ICON24_NONE)
		return;
	
	string[len++] = cling.ui.frm_render.vertical_icon_24_idx;
	string[len++] = 0;
	_render_vertical_local_character_core(string, 0, margin, b_24_size, FALSE);
}

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_PACE_MODEL_)	
static void _top_render_vertical_24_icon_core(I8U icon_24_idx)
{
	I8U string[16];	
	I8U len = 0;	
	I8U	margin = 2;
	I8U b_24_size = 24;

	string[len++] = icon_24_idx;
	string[len++] = 0;
	_render_vertical_local_character_core(string, 0, margin, b_24_size, FALSE);
}
#endif

static void _top_render_vertical_batt_ble()
{
	I8U data_buf[128];

	memset(data_buf, 0, 128);
	
  _render_batt_ble_core(data_buf);
	_rotate_270_degree(data_buf, 384);
}

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_PACE_MODEL_)	
static void _top_render_vertical_reminder()
{
	if ((cling.reminder.alarm_type == SLEEP_ALARM_CLOCK) || (cling.reminder.alarm_type == WAKEUP_ALARM_CLOCK))
    _top_render_vertical_24_icon_core(ICON24_SLEEP_ALARM_CLOCK_IDX);		
	else 
    _top_render_vertical_24_icon_core(ICON24_NORMAL_ALARM_CLOCK_IDX);		
}
#endif

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_PACE_MODEL_)		
static void _top_render_vertical_weather()
{
	I8U string[32];	
  I8U len = 0;
	I8U margin = 2;;
	I8U b_24_size = 24;
	WEATHER_INFO_CTX weather_info;	

	if (!WEATHER_get_weather_info(0, &weather_info)) {
    weather_info.type = 0;
	}
	
	string[len++] = ICON24_WEATHER_IDX+weather_info.type;
	string[len++] = 0;
	_render_vertical_local_character_core(string, 0, margin, b_24_size, FALSE);	
}
#endif

static void _top_render_vertical_24_icon_blinking()
{
	if (cling.ui.icon_sec_blinking) 
		_top_render_vertical_24_icon();
}

static void _middle_render_vertical_clock()
{
	I8U string[32];	
	I8U	margin = 1;
	I8U b_24_size = 24;

	// Render the hour
	sprintf((char *)string, " %02d", cling.time.local.hour);
	_render_vertical_local_character_core(string, 24, margin, b_24_size, TRUE);
	
	// Render the clock sign
	if (cling.ui.icon_sec_blinking) {
		// Render the minute
		sprintf((char *)string, ":%02d", cling.time.local.minute);
	} else {
		// Render the minute
		sprintf((char *)string, " %02d", cling.time.local.minute);
	}
	
	_render_vertical_local_character_core(string, 64, margin, b_24_size, FALSE);
}

static void _middle_render_vertical_steps()
{
	const char *step_name[3][3] = {{"STEP", "K", "K"},{"步 ", "千步", "万步"},{"步 ", "千步", "萬步"}};
	I8U string[32];
	I32U stat = 0;
	I8U margin = 2;
	I8U b_24_size = 24;
	I32U v_10000 = 0, v_1000 = 0, v_100 = 0;
  I8U language_type = cling.ui.language_type;	
	I8U step_unit_index = 0;
	
#ifdef _CLINGBAND_PACE_MODEL_	
	TRACKING_get_activity(0, TRACKING_STEPS, &stat);
#else  
	TRACKING_get_activity(cling.ui.vertical_index, TRACKING_STEPS, &stat);
#endif
	
	N_SPRINTF("UI: vertical step - %d (at %d day)", stat, cling.ui.vertical_index);
	
	if (stat > 99999) {
		v_10000 = 9;
		v_1000 = 9;
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			sprintf((char *)string, "%d%d", v_10000, v_1000);
		} else {
			sprintf((char *)string, "%d.%d", v_10000, v_1000);
		}
		margin = 1;
	} else if (stat > 9999) {
		v_10000 = stat / 10000;
		v_1000 = stat - (v_10000 * 10000);
		v_1000 /= 1000;
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			sprintf((char *)string, "%d%d", v_10000, v_1000);
		} else {
			sprintf((char *)string, "%d.%d", v_10000, v_1000);
		}
		margin = 1;
	} else if (stat > 999) {
		v_1000 = stat / 1000;
		v_100 = stat - (v_1000 * 1000);
		v_100 /= 100;
		sprintf((char *)string, "%d.%d", v_1000, v_100);
		margin = 1;
	} else if (stat > 99) {
		sprintf((char *)string, "%d", stat);
		b_24_size = 16;
	} else {
		sprintf((char *)string, "%d", stat);
	}
	
	if (b_24_size == 24) 
		_render_vertical_local_character_core(string, 42, margin, b_24_size, FALSE);
	else
		_render_vertical_local_character_core(string, 46, margin, b_24_size, FALSE);

	if (stat > 9999) {
		step_unit_index = 2;
	} else if (stat > 999) {
		step_unit_index = 1;
	} else {
		step_unit_index = 0;
	}
	
	if ((step_unit_index) && (language_type == LANGUAGE_TYPE_ENGLISH)) {
		_middle_render_vertical_character_core((I8U *)step_name[0][1], 75, (I8U *)step_name[0][0], 95);
	} else {
		_render_vertical_fonts_lib_character_core((I8U *)step_name[language_type][step_unit_index], 16, 80);
	}
}

static void _middle_render_vertical_distance()
{	
	const char *unit_distance_display[3][2] = {{"KM", "MILE"},{"公里", "英里"},{"公裏", "英裏"}};	
	I8U string[32];	
	I32U stat = 0;
	I8U margin = 2;
	I8U b_24_size = 24;
	I32U v_1000 = 0, v_100 = 0;
	I8U language_type = cling.ui.language_type;
	I8U metric = cling.user_data.profile.metric_distance;	
	
#ifdef _CLINGBAND_PACE_MODEL_		
	TRACKING_get_activity(0, TRACKING_DISTANCE, &stat);
#else 
	TRACKING_get_activity(cling.ui.vertical_index, TRACKING_DISTANCE, &stat);
#endif
	
	// Convert to miles
	if (cling.user_data.profile.metric_distance) {
		stat *= 10;
		stat >>= 4;
	}

	if (stat > 99999) {
		v_1000 = 99;
		sprintf((char *)string, "%d", v_1000);
	} else if (stat > 9999) {
		v_1000 = stat / 1000;
		sprintf((char *)string, "%d", v_1000);
	} else if (stat > 999) {
		v_1000 = stat / 1000;
		v_100 = stat - (v_1000 * 1000);
		v_100 /= 100;
		sprintf((char *)string, "%d.%d", v_1000, v_100);
		margin = 1;
	} else {
		v_100 = stat / 100;
		sprintf((char *)string, "0.%d", v_100);
		margin = 1;
	}
	
	_render_vertical_local_character_core(string, 42, margin, b_24_size, FALSE);

	_render_vertical_fonts_lib_character_core((I8U *)unit_distance_display[language_type][metric], 16, 80);	
}

static void _middle_render_vertical_calories()
{
	const char *unit_calories_display[3][2] = {{"CAL", "K"},{"大卡 ", "千 "},{"大卡 ", "千 "}};
	I8U string[32];
	I32U stat = 0;
	I8U margin = 2;
	I8U b_24_size = 24;
	I32U v_10000 = 0, v_1000 = 0, v_100 = 0;
  I8U language_type = cling.ui.language_type;	
	I8U calories_unit_index = 0;
	
#ifdef _CLINGBAND_PACE_MODEL_		
	TRACKING_get_activity(0, TRACKING_CALORIES, &stat);
#else
	TRACKING_get_activity(cling.ui.vertical_index, TRACKING_CALORIES, &stat);
#endif
	
	if (stat > 99999) {
		v_10000 = 9;
		v_1000 = 9;
		sprintf((char *)string, "%d%d", v_10000, v_1000);
	} else if (stat > 9999) {
		v_10000 = stat / 10000;
		v_1000 = stat - (v_10000 * 10000);
		v_1000 /= 1000;
		sprintf((char *)string, "%d%d", v_10000, v_1000);
	} else if (stat > 999) {
		v_1000 = stat / 1000;
		v_100 = stat - (v_1000 * 1000);
		v_100 /= 100;
		sprintf((char *)string, "%d.%d", v_1000, v_100);
		margin = 1;
	} else if (stat > 99) {
		sprintf((char *)string, "%d", stat);
		b_24_size = 16;
	} else {
		sprintf((char *)string, "%d", stat);
	}
	
	if (b_24_size == 24) 
		_render_vertical_local_character_core(string, 40, margin, b_24_size, FALSE);
	else
		_render_vertical_local_character_core(string, 46, margin, b_24_size, FALSE);

	if (stat > 999) 
		calories_unit_index = 1;
	else 
		calories_unit_index = 0;
	
	if (calories_unit_index) {
		_middle_render_vertical_character_core((I8U *)unit_calories_display[language_type][1], 70, (I8U *)unit_calories_display[language_type][0], 86);
	} else {
	  _render_vertical_fonts_lib_character_core((I8U *)unit_calories_display[language_type][0], 16, 75);
	}
}

static void _middle_render_vertical_active_time()
{
	const	char *minute_name[] = {"MIN", "分钟", "分鍾"};	
	I8U string[32];	
	I32U stat = 0;
  I8U	margin = 2;
	I8U b_24_size = 24;
	I8U language_type = cling.ui.language_type;	
	
#ifdef _CLINGBAND_PACE_MODEL_			
	TRACKING_get_activity(0, TRACKING_ACTIVE_TIME, &stat);
#else
	TRACKING_get_activity(cling.ui.vertical_index, TRACKING_ACTIVE_TIME, &stat);
#endif
	
	N_SPRINTF("UI: vertical active time - %d", stat);
	if (stat > 99) {
		sprintf((char *)string, "%d", stat);
		b_24_size = 16;
	} else {
		sprintf((char *)string, "%d", stat);
	}
	
	if (b_24_size == 24) 
		_render_vertical_local_character_core(string, 42, margin, b_24_size, FALSE);
	else
		_render_vertical_local_character_core(string, 46, margin, b_24_size, FALSE);

	_render_vertical_fonts_lib_character_core((I8U *)minute_name[language_type], 16, 80);
}

#ifdef _CLINGBAND_UV_MODEL_
static void _middle_render_vertical_uv_index()
{
	I8U string[32];	
  I8U	margin = 2;
	I8U b_24_size = 24;
	I8U integer = 0;
	
	integer = cling.uv.max_UI_uv;
	
	if (integer > 99) {
		sprintf((char *)string, "%d",(integer/10));
	} else {
		sprintf((char *)string, "%d.%d", (integer/10), (integer%10));
	  margin = 1;
	}
	
	_render_vertical_local_character_core(string, 50, margin, b_24_size, FALSE);
}
#endif

static void _middle_render_vertical_training_run_start()
{
	const	char *run_start_name_1[] = {"RUN", "开始 ", "開始 "};		
	const	char *run_start_name_2[] = {"NOW", "跑步 ", "跑步 "};			
	I8U language_type = cling.ui.language_type;		

	_middle_render_vertical_character_core((I8U *)run_start_name_1[language_type], 50, (I8U *)run_start_name_2[language_type], 70);	
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _middle_render_vertical_training_run_or_analysis()
{
	const	char *start_name[] = {"RUN ", "开始 ", "開始 "};	
	const	char *analysis_name[] = {"MORE", "记录 ", "記錄 "};		
	I8U language_type = cling.ui.language_type;	

	_render_vertical_fonts_lib_invert_colors_core((I8U *)start_name[language_type], 60);	
		
	_render_vertical_fonts_lib_invert_colors_core((I8U *)analysis_name[language_type], 100);	
}
#endif

#ifdef _CLINGBAND_PACE_MODEL_
static void _middle_render_vertical_running_analysis()
{
	const	char *running_analysis_name_1[] = {"RUN", "当日 ", "當日 "};	
	const	char *running_analysis_name_2[] = {"DATA", "跑步 ", "跑步 "};	
	I8U language_type = cling.ui.language_type;		

	_middle_render_vertical_character_core((I8U *)running_analysis_name_1[language_type], 50, (I8U *)running_analysis_name_2[language_type], 70);		
}
#endif

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_PACE_MODEL_)		
static void _middle_render_vertical_pm2p5()
{
	const char *air_display[3][7] = {{"--","GOOD", "GOOD", "POOR", "POOR", "POOR", "POOR"},
	                       {"--","优质", "良好", "轻度", "中度", "重度", "严重"},
												 {"--","優質", "良好", "輕度", "中度", "重度", "嚴重"}};

	I8U level_idx;						
	I8U string[32];	
  I8U	margin = 2;
	I8U b_24_size = 24;
	I8U language_type = cling.ui.language_type;			

	WEATHER_CTX *w = &cling.weather;
												 
  if ((w->pm2p5_month == cling.time.local.month) && (w->pm2p5_day == cling.time.local.day)) {	
		if (w->pm2p5_value == 0xffff) {
			sprintf((char *)string, "0");	
		} else if (!w->pm2p5_value) {
			sprintf((char *)string, "0");			
		} else if (w->pm2p5_value > 999) {
			w->pm2p5_value = 999;
			sprintf((char *)string, "%d", w->pm2p5_value);
			b_24_size = 16;
		} else if (w->pm2p5_value > 99) {
			sprintf((char *)string, "%d", w->pm2p5_value);
			b_24_size = 16;
		} else {
			sprintf((char *)string, "%d", w->pm2p5_value);
		}
	} else {
		sprintf((char *)string, "0");	
	}
	
	if (b_24_size == 24) 
		_render_vertical_local_character_core(string, 42, margin, b_24_size, FALSE);
	else
		_render_vertical_local_character_core(string, 46, margin, b_24_size, FALSE);

  if ((w->pm2p5_month == cling.time.local.month) && (w->pm2p5_day == cling.time.local.day)) {		
		if (w->pm2p5_value == 0xffff) {
			level_idx = 0;
		} else if (!w->pm2p5_value) {
			level_idx = 0;		
		} else if (w->pm2p5_value < 50) {
			level_idx = 1;
		} else if (w->pm2p5_value < 100) {
			level_idx = 2;
		} else if (w->pm2p5_value < 150) {
			level_idx = 3;
		} else if (w->pm2p5_value < 250) {
			level_idx = 4;
		} else if (w->pm2p5_value < 350) {
			level_idx = 5;
		} else {
			level_idx = 6;
		}
  } else {
		level_idx = 0;
	}
	
	_render_vertical_fonts_lib_character_core((I8U *)air_display[language_type][level_idx], 16, 84);
}
#endif

static I8U _render_middle_vertical_hr_core(I8U offset, BOOLEAN b_training_mode)
{
	const char *heart_rate_wave_indicator[] = {" - , ,", " , - ,", " , , -" };		
	I8U string[32];		
	I8U len = 0;	
	I8U margin = 2;
	I8U b_24_size = 24;
	I8U hr_result = 0;
  BOOLEAN b_all_hold = FALSE;
	
	// Second, render heart rate 
	if (cling.hr.b_closing_to_skin || cling.hr.b_start_detect_skin_touch) {
		if (cling.hr.heart_rate_ready) {
			if (b_training_mode)
				b_all_hold = TRUE;
			hr_result = PPG_minute_hr_calibrate();
			if (hr_result > 99) {
				b_24_size = 16; 
				margin = 1;
			} else {
				b_24_size = 24;
				margin = 2;
			}
			len = sprintf((char *)string, "%d", hr_result);		
			if (b_24_size == 24) 
				_render_vertical_local_character_core(string, offset, margin, b_24_size, b_all_hold);
			else
				_render_vertical_local_character_core(string, offset+5, margin, b_24_size, FALSE);			
		} else {
			cling.ui.heart_rate_wave_index ++;
			if (cling.ui.heart_rate_wave_index > 2) {
				cling.ui.heart_rate_wave_index = 0;
			}
			len = sprintf((char *)string, "%s", heart_rate_wave_indicator[cling.ui.heart_rate_wave_index]);
			b_24_size = 8;
			margin = 3;
			_render_vertical_local_character_core(string, offset+5, margin, b_24_size, FALSE);
			hr_result = 0;
			return hr_result;
		}
	} else {
		N_SPRINTF("[UI] Heart rate - not valid");
		b_24_size = 24;
		margin = 0;
		len = 0;
		string[len++] = ICON24_NO_SKIN_TOUCH_IDX;
		string[len++] = 0;
		_render_vertical_local_character_core(string, offset, margin, b_24_size, FALSE);
		hr_result = 0;
	}
	
	return hr_result;	
}

static void _middle_render_vertical_heart_rate()
{
	I8U string[32];	
	I8U margin = 0;
	I8U b_24_size = 16;
	I8U hr_result = 0;

	hr_result = _render_middle_vertical_hr_core(50, FALSE);
	
	if (hr_result) {
		sprintf((char *)string, "BPM");
		_render_vertical_local_character_core(string, 80, margin, b_24_size, FALSE);
	}
}

#ifdef _CLINGBAND_2_PAY_MODEL_
static void _middle_render_vertical_stopwatch_start()
{
	I8U string[32];	
	I8U margin = 1;
	I8U b_24_size = 24;	
	I32U t_diff = 0;
	I8U hour = 0, min = 0, sec = 0;
	I16U ms = 0;

  if (cling.ui.b_stopwatch_first_enter) {
  	cling.ui.stopwatch_time_stamp = CLK_get_system_time();				
		cling.ui.b_in_stopwatch_pause_mode = FALSE;
		cling.ui.b_stopwatch_first_enter = FALSE;
		cling.ui.stopwatch_t_stop_stamp = 0;
	} else {
	  if (!cling.ui.b_in_stopwatch_pause_mode)  {
      t_diff = CLK_get_system_time() - cling.ui.stopwatch_time_stamp;			
	    cling.ui.stopwatch_t_stop_stamp = t_diff;			
		} else {
	    t_diff = cling.ui.stopwatch_t_stop_stamp;
			cling.ui.stopwatch_time_stamp = CLK_get_system_time() - cling.ui.stopwatch_t_stop_stamp;
		}
	}

	ms = t_diff % 1000;
	t_diff /= 1000;
	hour = t_diff / 3600;
	t_diff -= hour*3600;
	min = t_diff / 60;
	t_diff -= min * 60;
	sec = t_diff;
	
	ms /= 10;	
	
	// Render the time	
	if (hour) {
	// Render the hour
		if (hour > 9) hour = 9;
	  sprintf((char *)string, " +%d", hour);
    _render_vertical_local_character_core(string, 28, margin, b_24_size, TRUE);		
		
	  // Render the minute
	  sprintf((char *)string, ":%02d", min);
    _render_vertical_local_character_core(string, 56, margin, b_24_size, FALSE);		

	  // Render the second
	  sprintf((char *)string, ":%02d", sec);
    _render_vertical_local_character_core(string, 84, margin, b_24_size, FALSE);			
	} else {
    if (min > 9) {
	    // Render the minute
	    sprintf((char *)string, " %02d", min);
      _render_vertical_local_character_core(string, 28, margin, b_24_size, FALSE);					
		} else {
	    // Render the minute
	    sprintf((char *)string, "+ %d", min);
      _render_vertical_local_character_core(string, 28, margin, b_24_size, FALSE);					
		}

		// Render the second
	  sprintf((char *)string, ":%02d", sec);
    _render_vertical_local_character_core(string, 56, margin, b_24_size, FALSE);			
		
		// Render the millisecond
	  sprintf((char *)string, "  .   ");		
		_render_vertical_local_character_core(string, 76, margin, b_24_size, FALSE);	
		if (ms > 99)
			ms = 99;
		b_24_size = 16;
	  sprintf((char *)string, "   %02d", ms);
    _render_vertical_local_character_core(string, 84, margin, b_24_size, FALSE);		
	}
}

static void _middle_render_vertical_stopwatch_stop()
{
	const char *stopwatch_stop_name[] = {"END", "结束 ", "結束 "};
	I8U language_type = cling.ui.language_type;

	_render_vertical_fonts_lib_character_core((I8U *)stopwatch_stop_name[language_type], 16, 55);
}
#endif

#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)	
static void _middle_render_vertical_skin_temp()
{
	I8U string[32];	
  I8U	margin = 2;
	I8U b_24_size = 24;
	I8U integer = 0;
	
  integer = cling.therm.current_temperature/10;
	
	// Temperature value
	sprintf((char *)string, "%d", integer);
	_render_vertical_local_character_core((I8U *)string, 42, margin, b_24_size, FALSE);
	
	// - Skin temperature unit - 
	string[0] = ICON24_CELCIUS_IDX;
  string[1] = 0;

	_render_vertical_local_character_core((I8U *)string, 80, margin, b_24_size, FALSE);
}
#endif

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_PACE_MODEL_)
static void _middle_render_vertical_weather()
{
	I8U string[32];
	I8U margin = 2;
	I8U b_24_size = 24;
	WEATHER_INFO_CTX weather_info;	

	if (!WEATHER_get_weather_info(0, &weather_info)) {
		weather_info.low_temperature = 15;
		weather_info.high_temperature = 22;
	}
	
	// - Temperature Range - 
	string[0] = ICON24_WEATHER_RANGE_IDX;
	string[1] = 0;
	_render_vertical_local_character_core(string, 55, margin, b_24_size, FALSE);
	
	// - Low temperature -
	sprintf((char *)string, "%d", weather_info.low_temperature);
	
	if (weather_info.low_temperature < 0)
		margin = 1;
	else
		margin = 2;
	_render_vertical_local_character_core(string, 32, margin, b_24_size, FALSE);
		
	// - High temperature;
	if (weather_info.high_temperature < 0)
		margin = 1;
	else
		margin = 2;
	sprintf((char *)string, "%d", weather_info.high_temperature);
	_render_vertical_local_character_core(string, 74, margin, b_24_size, FALSE);
	
	// - Weather temperature unit - 
	string[0] = ICON24_CELCIUS_IDX;
	string[1] = 0;
	_render_vertical_local_character_core(string, 104, margin, b_24_size, FALSE);
}
#endif

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_PACE_MODEL_)
static void _middle_render_vertical_reminder_core(BOOLEAN b_alarm_clock_reminder)
{
	I8U string[32];	
	I8U len = 0;	
	I8U margin = 1;
	I8U b_24_size = 24;
  BOOLEAN b_invalid_alarm = TRUE;
	
	if (b_alarm_clock_reminder) {
		if ((cling.reminder.total) || (cling.reminder.b_sleep_total)){
	    if ((cling.ui.ui_alarm_hh < 24) && (cling.ui.ui_alarm_mm < 60)) {
		    b_invalid_alarm = FALSE;
	    }
		} 		
	} else {
		if (cling.reminder.total){
	    if ((cling.ui.ui_alarm_hh < 24) && (cling.ui.ui_alarm_mm < 60)) {
		    b_invalid_alarm = FALSE;
	    }
		} 
	}
	
	if (b_invalid_alarm) {
		len = 0;
		string[len++] = ICON24_NO_SKIN_TOUCH_IDX;
		string[len++] = 0;
		_render_vertical_local_character_core(string, 50, margin, b_24_size, FALSE);		
	} else {
	  // Rendering clock ...
		sprintf((char *)string, " %02d", cling.ui.ui_alarm_hh);
		_render_vertical_local_character_core(string, 38, margin, b_24_size, TRUE);		
		
		// Render the minute
		sprintf((char *)string, ":%02d", cling.ui.ui_alarm_mm);
		_render_vertical_local_character_core(string, 65, margin, b_24_size, FALSE);			
		N_SPRINTF("[UI] normal ui_hh: %d, ui_mm: %d", cling.ui.ui_alarm_hh, cling.ui.ui_alarm_mm);		
	}
}

static void _middle_render_vertical_alarm_clock_reminder()
{
  _middle_render_vertical_reminder_core(TRUE);
}
#endif

#ifdef _CLINGBAND_2_PAY_MODEL_
static void _middle_render_vertical_alarm_clock_detail()
{
  _middle_render_vertical_reminder_core(FALSE);
}
#endif

static void _vertical_core_run_distance(I32U stat, BOOLEAN b_all_hold)
{
	I8U string[32];	
	I8U margin = 2;
	I8U b_24_size = 24;
	I32U v_1000 = 0, v_100 = 0;
	I8U metric = cling.user_data.profile.metric_distance;
	
	// Convert to miles
	if (metric) {
		stat *= 10;
		stat >>= 4;
	}

	if (stat > 99999) {
		v_1000 = 99;
		sprintf((char *)string, "%d", v_1000);
	} else if (stat > 9999) {
		v_1000 = stat / 1000;
		sprintf((char *)string, "%d", v_1000);
	} else if (stat > 999) {
		v_1000 = stat / 1000;
		v_100 = stat - (v_1000 * 1000);
		v_100 /= 100;
		sprintf((char *)string, "%d.%d", v_1000, v_100);
		margin = 1;
	} else {
		v_100 = stat / 100;
		sprintf((char *)string, "0.%d", v_100);
		margin = 1;
	}
	
	_render_vertical_local_character_core(string, 67, margin, b_24_size, b_all_hold);
}

static void _middle_render_vertical_running_distance()
{
	const char *run_distance_name[] = {"RUN", "里程", "裏程" };	
	I8U language_type = cling.ui.language_type;		

	_render_vertical_fonts_lib_character_core((I8U *)run_distance_name[language_type], 16, 28);
	
	_vertical_core_run_distance(cling.run_stat.distance, FALSE);
}

static void _middle_render_vertical_training_distance()
{
	const char *run_distance_name[] = {"RUN", "里程", "裏程" };	
	I8U language_type = cling.ui.language_type;		

	if (cling.ui.icon_sec_blinking)
	  _render_vertical_fonts_lib_character_core((I8U *)run_distance_name[language_type], 16, 28);
  
	_vertical_core_run_distance(cling.train_stat.distance, TRUE);	
}

static void _middle_render_vertical_running_time()
{
	const	char *running_time_name[3] = {"TIME", "耗时", "耗時"};
	I8U string[32];	
	I8U margin = 1;
	I8U b_24_size = 24;	
	I8U hour=0, min=0, sec=0;	
	I8U language_type = cling.ui.language_type;

	_render_vertical_fonts_lib_character_core((I8U *)running_time_name[language_type], 16, 28);
	
	min = cling.run_stat.time_min;
	sec = cling.run_stat.time_sec;
	hour = min/60;
	min = min - hour*60;

	// Render the hour
	if (hour > 9) hour = 9;
	sprintf((char *)string, " +%d", hour);
  _render_vertical_local_character_core(string, 52, margin, b_24_size, TRUE);			
	
	// Render the minute
	sprintf((char *)string, ":%02d", min);
  _render_vertical_local_character_core(string, 78, margin, b_24_size, FALSE);		
	
	// Render the second
	sprintf((char *)string, ":%02d", sec);
  _render_vertical_local_character_core(string, 104, margin, b_24_size, FALSE);		
}

static void _middle_render_vertical_training_time()
{
 	const	char *running_time_name[3] = {"TIME", "耗时", "耗時"};
	I8U string[32];	
	I8U margin = 1;
	I8U b_24_size = 24;	
	I8U hour, min, sec;	
	I8U language_type = cling.ui.language_type;

	if (cling.ui.icon_sec_blinking) {
		_render_vertical_fonts_lib_character_core((I8U *)running_time_name[language_type], 16, 28);
	} 
	
	_get_training_time_core(&hour, &min, &sec);
	
	if (hour) {
		// Render the hour
		if (hour > 9) hour = 9;
		sprintf((char *)string, " +%d", hour);
    _render_vertical_local_character_core(string, 52, margin, b_24_size, TRUE);				
		// Render the minute
		sprintf((char *)string, ":%02d", min);
    _render_vertical_local_character_core(string, 78, margin, b_24_size, TRUE);				
		// Render the second
		sprintf((char *)string, ":%02d", sec);
    _render_vertical_local_character_core(string, 104, margin, b_24_size, TRUE);				
	} else {
		// Render the minute
		if (min > 9) {
			sprintf((char *)string, " %02d", min);
		} else {
			sprintf((char *)string, " +%d", min);
		}
    _render_vertical_local_character_core(string, 75, margin, b_24_size, TRUE);		
		
		// Render the second
		sprintf((char *)string, ":%02d", sec);
    _render_vertical_local_character_core(string, 104, margin, b_24_size, TRUE);				
	}
}

static void _middle_render_vertical_running_pace()
{
	const	char *run_name[] = {"RUN", "运动 ", "運動 "};	
	const	char *pace_name[] = {"PACE", "配速 ", "配速 "};		
	I8U string[32];	
	I8U margin = 1;
	I8U b_24_size = 24;	
	I32U pace = cling.run_stat.time_min; // Rounding 
	I32U min, sec;
	I8U language_type = cling.ui.language_type;	

	_middle_render_vertical_character_core((I8U *)run_name[language_type], 28, (I8U *)pace_name[language_type], 46);		
	
	if (cling.run_stat.distance) {
		pace = cling.run_stat.time_sec+cling.run_stat.time_min*60;
		pace *= 100000;
		pace /= 60;
		pace /= cling.run_stat.distance;
	} else {
		pace = 0;
	}
	
	min = pace / 100;
	sec = pace - min * 100;
	sec *= 60;
	sec /= 100;
	
	if (min > 24) {
		min = 0;
		sec = 0;
	}
	
	// Render the hour
	if (min > 9)
		sprintf((char *)string, "%02d/", min);
	else
		sprintf((char *)string, "+%d/", min);
	_render_vertical_local_character_core(string, 76, margin, b_24_size, FALSE);		

	// Render the minute
	sprintf((char *)string, "%02d*", sec);
	_render_vertical_local_character_core(string, 104, margin, b_24_size, FALSE);		
}

static void _middle_render_vertical_running_hr()
{
	const	char *avg_name[] = {"AVG", "平均 ", "平均 "};
	const	char *hate_rate_name[] = {"HR", "心率 ", "心率 "};	
	I8U string[32];
	I8U margin = 0;
	I8U b_24_size = 16;
	I32U hr = cling.run_stat.accu_heart_rate;
	I8U language_type = cling.ui.language_type;	
	
	if (cling.run_stat.time_min) {
		hr /= cling.run_stat.time_min;
	} else {
		hr = 0;
	}
	
	// Render heart rate 
	if (hr > 99) {
		b_24_size = 16; 
		margin = 1;
	} else {
		margin = 2;
		b_24_size = 24;
	}
	
	sprintf((char *)string, "%d", hr);
				
	if (b_24_size == 24) 
		_render_vertical_local_character_core(string, 86, margin, b_24_size, FALSE);
	else
		_render_vertical_local_character_core(string, 90, margin, b_24_size, FALSE);

	_middle_render_vertical_character_core((I8U *)avg_name[language_type], 28, (I8U *)hate_rate_name[language_type], 46);		
}

static void _middle_vertical_run_calories_core(I32U run_calories, BOOLEAN icon_blinking)
{
	const	char *running_calories_name[] = {"BURN", "热量 ", "熱量 "};		
	const char *unit_calories_display[3][2] = {{"CAL", "K"},{"大卡 ", "千 "},{"大卡 ", "千 "}};
	I8U string[32];
	I8U margin = 0;
	I8U b_24_size = 16;
	I32U stat = 0;	
	I32U v_10000 = 0, v_1000 = 0, v_100 = 0;
	I8U language_type = cling.ui.language_type;	
	I8U calories_unit_index = 0;
	
	if (icon_blinking) {
		if (cling.ui.icon_sec_blinking) {
	    _render_vertical_fonts_lib_character_core((I8U *)running_calories_name[language_type], 16, 28);
		}
	} else {
	  _render_vertical_fonts_lib_character_core((I8U *)running_calories_name[language_type], 16, 28);		
	}

  b_24_size = 24;	
	margin = 2;
	stat = run_calories;
	
	if (stat > 99999) {
		v_10000 = 9;
		v_1000 = 9;
		sprintf((char *)string, "%d%d", v_10000, v_1000);
	} else if (stat > 9999) {
		v_10000 = stat / 10000;
		v_1000 = stat - (v_10000 * 10000);
		v_1000 /= 1000;
		sprintf((char *)string, "%d%d", v_10000, v_1000);
	} else if (stat > 999) {
		v_1000 = stat / 1000;
		v_100 = stat - (v_1000 * 1000);
		v_100 /= 100;
		sprintf((char *)string, "%d.%d", v_1000, v_100);
		margin = 1;
	} else if (stat > 99) {
		sprintf((char *)string, "%d", stat);
		b_24_size = 16;
	} else {
		sprintf((char *)string, "%d", stat);
	}
	
	if (b_24_size == 24) 
		_render_vertical_local_character_core(string, 68, margin, b_24_size, FALSE);
	else
		_render_vertical_local_character_core(string, 72, margin, b_24_size, FALSE);

	if (stat > 999) {
    calories_unit_index = 1;
	} else {		
    calories_unit_index = 0;
	}
	
	if (calories_unit_index) {
		_middle_render_vertical_character_core((I8U *)unit_calories_display[language_type][1], 96, (I8U *)unit_calories_display[language_type][0], 112);		
	} else {
	  _render_vertical_fonts_lib_character_core((I8U *)unit_calories_display[language_type][0], 16, 112);		
	}
}

static void _middle_render_vertical_running_calories()
{
  _middle_vertical_run_calories_core(cling.run_stat.calories, FALSE);	
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _middle_render_vertical_training_calories()
{
  _middle_vertical_run_calories_core(cling.train_stat.calories, TRUE);	
}
#endif

static void _middle_render_vertical_running_cadence()
{
	const	char *rate_name[] = {"RATE", "步频 ", "步頻 "};			
  const	char *spm_name[] = {"SPM", "步分 ", "步分 "};			
	I8U string[32];	
	I8U margin = 0;
	I8U b_24_size = 16;	
	I32U stat = 0;
	I8U language_type = cling.ui.language_type;		
	
	_render_vertical_fonts_lib_character_core((I8U *)rate_name[language_type], 16, 28);
	
	if (cling.run_stat.time_min)
		stat = cling.run_stat.steps/cling.run_stat.time_min;
	else
		stat = 0;
	
	// Render heart rate 
	if (stat > 99) {
		b_24_size = 16; 
		margin = 1;
	} else {
		margin = 2;
		b_24_size = 24;
	}
	
	sprintf((char *)string, "%d", stat);
				
	if (b_24_size == 24) 
		_render_vertical_local_character_core(string, 68, margin, b_24_size, FALSE);
	else
		_render_vertical_local_character_core(string, 72, margin, b_24_size, FALSE);
	
	b_24_size = 16;
  margin = 0;
	_render_vertical_fonts_lib_character_core((I8U *)spm_name[language_type], 16, 112);
}

static void _middle_render_vertical_running_stride()
{
	const	char *stride_name[] = {"STR.", "步幅 ", "步幅 "};		
	const char *unit_stride_display[3][2] = {{"CM", "IN"},{"厘米", "英寸"},{"厘米", "英寸"}};		
	I8U string[32];	
	I8U margin = 0;
	I8U b_24_size = 16;	
	I32U stat = 0;
	I8U language_type = cling.ui.language_type;			
	I8U metric = cling.user_data.profile.metric_distance;
	
	_render_vertical_fonts_lib_character_core((I8U *)stride_name[language_type], 16, 28);
	
	if (cling.run_stat.steps)
		stat = (cling.run_stat.distance*100)/cling.run_stat.steps;
	else
		stat = cling.user_data.profile.stride_running_in_cm;

	// Convert to in.
	if (cling.user_data.profile.metric_distance) {
		stat *= 100;
		stat /= 254;
	}
	
	// Render heart rate 
	if (stat > 99) {
		b_24_size = 16; 
		margin = 1;
	} else {
		margin = 2;
		b_24_size = 24;
	}
	
	sprintf((char *)string, "%d", stat);
				
	if (b_24_size == 24) 
		_render_vertical_local_character_core(string, 68, margin, b_24_size, FALSE);
	else
		_render_vertical_local_character_core(string, 72, margin, b_24_size, FALSE);
	
	_render_vertical_fonts_lib_character_core((I8U *)unit_stride_display[language_type][metric], 16, 112);
}

#ifdef _CLINGBAND_PACE_MODEL_
static void _middle_render_vertical_running_stop_analysis()
{
	const	char *stop_analysis_name_1[] = {"INFO", "结束 ", "結束 "};				
	const	char *stop_analysis_name_2[] = {"END", "分析 ", "分析 "};				
	I8U language_type = cling.ui.language_type;			

	_middle_render_vertical_character_core((I8U *)stop_analysis_name_1[language_type], 50, (I8U *)stop_analysis_name_2[language_type], 70);		
}
#endif

static BOOLEAN _middle_render_vertical_core_ready()
{
	const char *ready_indicator_name[] = {"3","2","1","GO"};
	I8U string[32];	
	I8U margin = 4;
	I8U b_24_size = 24;	
  I32U t_curr = CLK_get_system_time();
  BOOLEAN b_ready_finished = FALSE;	
	
	if (t_curr  > (cling.ui.running_time_stamp + 700)) {
		cling.ui.running_time_stamp = t_curr;		
		cling.ui.run_ready_index++;
	}
	
	if (cling.ui.run_ready_index > 3) {
		cling.ui.run_ready_index = 3;
		b_ready_finished = TRUE;
	}

	sprintf((char *)string, "%s", ready_indicator_name[cling.ui.run_ready_index]);
	
	_render_vertical_local_character_core(string, 60, margin, b_24_size, TRUE);		
	return b_ready_finished;	
}

static void _middle_render_vertical_training_ready()
{
  BOOLEAN b_ready_finished = FALSE;	
	
	b_ready_finished = _middle_render_vertical_core_ready();
	
	if (b_ready_finished) {
		cling.ui.frame_index = UI_DISPLAY_TRAINING_STAT_TIME;
		cling.ui.frame_next_idx = cling.ui.frame_index;	
	}
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _middle_render_vertical_cycling_outdoor_ready()
{
  BOOLEAN b_ready_finished = FALSE;	
	
	b_ready_finished = _middle_render_vertical_core_ready();
	
	if (b_ready_finished) {
		cling.ui.frame_index = UI_DISPLAY_CYCLING_OUTDOOR_STAT_TIME;
		cling.ui.frame_next_idx = cling.ui.frame_index;	
	}
}
#endif

static void _middle_render_vertical_training_pace()
{
	const	char *pace_name[] = {"PACE", "配速 ", "配速 "};			
	I8U string[32];	
	I8U margin = 1;
	I8U b_24_size = 24;		
	I32U min, sec;
	I8U language_type = cling.ui.language_type;		
	
	min = cling.run_stat.last_10sec_pace_min;
	sec = cling.run_stat.last_10sec_pace_sec;
	
	if (min > 9)
		sprintf((char *)string, "%02d/", min);
	else
		sprintf((char *)string, "+%d/", min);
	
	_render_vertical_local_character_core(string, 76, margin, b_24_size, TRUE);	

	// Render the minute
	sprintf((char *)string, "%02d*", sec);
	_render_vertical_local_character_core(string, 104, margin, b_24_size, TRUE);	
	
	if (cling.ui.icon_sec_blinking) {	
		_render_vertical_fonts_lib_character_core((I8U *)pace_name[language_type], 16, 28);
	}
}

static void _middle_render_vertical_training_hr()
{
	const	char *hart_rate_name[] = {"HR", "心率 ", "心率 "};		
	I8U string[32];	
	I8U hr_result = 0;
	I8U i;		
	I32U hr_perc = 0;
	I8U *p0, *p1, *p2, *p3;
	I8U language_type = cling.ui.language_type;		
	
	if (cling.ui.icon_sec_blinking) {		
		_render_vertical_fonts_lib_character_core((I8U *)hart_rate_name[language_type], 16, 28);
	} 

	hr_result = _render_middle_vertical_hr_core(60, TRUE);
	
  if (hr_result) {
	  hr_perc = (hr_result * 100)/(220-cling.user_data.profile.age);	
	  if (hr_perc > 98)
		  hr_perc = 98;
		
	  sprintf((char *)string, "%d%%", hr_perc);
	  _render_vertical_fonts_lib_character_core(string, 16, 94);
  }
	
	cling.ui.training_hr = (I8U)hr_perc;
	
	p0 = cling.ui.p_oled_up+112;
	p1 = p0 + 128;
	p2 = p1 + 128;
	p3 = p2 + 128;
	
	*p0++ = 0x7e;
	*p1++ = 0x7e;
	*p2++ = 0x7e;
	*p3++ = 0x7e;
	for (i = 1; i < 15; i++) {
		if (!hr_perc) {
			*p0++ = 0x42;
			*p1++ = 0x42;
			*p2++ = 0x42;
			*p3++ = 0x42;			
		} else if ((hr_perc) && (hr_perc <= 25)) {
			*p0++ = 0x42;
			*p1++ = 0x42;
			*p2++ = 0x42;
			*p3++ = 0x7e;
		} else if ((hr_perc > 25) && (hr_perc <= 50)) {
			*p0++ = 0x42;
			*p1++ = 0x42;
			*p2++ = 0x7e;
			*p3++ = 0x7e;
		} else if ((hr_perc > 50) && (hr_perc <= 75)) {
			*p0++ = 0x42;
			*p1++ = 0x7e;
			*p2++ = 0x7e;
			*p3++ = 0x7e;
		} else {
			*p0++ = 0x7e;
			*p1++ = 0x7e;
			*p2++ = 0x7e;
			*p3++ = 0x7e;
		}
	}
	*p0++ = 0x7e;
	*p1++ = 0x7e;
	*p2++ = 0x7e;
	*p3++ = 0x7e;
}

static void _middle_render_vertical_training_run_stop()
{
	const	char *run_stop_name_1[] = {"RUN", "结束 ", "結束 "};	
	const	char *run_stop_name_2[] = {"STOP", "跑步 ", "跑步 "};		

	I8U language_type = cling.ui.language_type;		

	_middle_render_vertical_character_core((I8U *)run_stop_name_1[language_type],50, (I8U *)run_stop_name_2[language_type], 70);
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _middle_render_vertical_training_workout_stop()
{
	const	char *run_stop_name_1[] = {"RUN", "结束 ", "結束 "};	
	const	char *run_stop_name_2[] = {"STOP", "运动 ", "運動 "};		

	I8U language_type = cling.ui.language_type;		

	_middle_render_vertical_character_core((I8U *)run_stop_name_1[language_type],50, (I8U *)run_stop_name_2[language_type], 70);
}

static void _middle_render_vertical_cycling_outdoor_start()
{
	const	char *cycling_start_name_1[] = {"BIKE", "开始 ", "開始 "};	
	const	char *cycling_start_name_2[] = {"NOW", "骑行 ", "騎行 "};			
	I8U language_type = cling.ui.language_type;		
	
	_middle_render_vertical_character_core((I8U *)cycling_start_name_1[language_type], 50, (I8U *)cycling_start_name_2[language_type], 70);	
}

static void _middle_render_vertical_cycling_outdoor_stop()
{
	const	char *cycling_stop_name_1[] = {"BIKE", "结束", "結束"};	
	const	char *cycling_stop_name_2[] = {"STOP", "骑行", "騎行"};		
	I8U language_type = cling.ui.language_type;		
	
	_middle_render_vertical_character_core((I8U *)cycling_stop_name_1[language_type], 50, (I8U *)cycling_stop_name_2[language_type], 70);		
}

static void _middle_render_vertical_cycling_outdoor_distance()
{
	const char *run_cycling_name[] = {"BIKE", "骑行", "騎行" };		
	I8U string[32];	
	I8U len = 0;		
	I8U margin = 2;
	I8U b_24_size = 24;
 	I8U language_type = cling.ui.language_type;	

	if (cling.ui.icon_sec_blinking) {		
		_render_vertical_fonts_lib_character_core((I8U *)run_cycling_name[language_type], 16, 28);
	}
	
	if (BTLE_is_connected()) {
		_vertical_core_run_distance(cling.train_stat.distance, TRUE);
	} else {
		len = 0;
		string[len++] = ICON24_NO_SKIN_TOUCH_IDX;
		string[len++] = 0;
		_render_vertical_local_character_core(string, 55, margin, b_24_size, FALSE);				
	}
}

static void _middle_render_vertical_cycling_outdoor_speed()
{
	const char *run_speed_name[] = {"SPED", "速度", "速度" };		
	I8U string[32];	
	I8U len = 0;		
	I8U margin = 2;
	I8U b_24_size = 24;
	I8U language_type = cling.ui.language_type;		

	if (cling.ui.icon_sec_blinking) {		
	  _render_vertical_fonts_lib_character_core((I8U *)run_speed_name[language_type], 16, 28);
	}

	if (BTLE_is_connected()) {
		_vertical_core_run_distance(cling.train_stat.speed, TRUE);
	} else {
		len = 0;
		string[len++] = ICON24_NO_SKIN_TOUCH_IDX;
		string[len++] = 0;
		_render_vertical_local_character_core(string, 55, margin, b_24_size, FALSE);				
	}
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_
static void _middle_render_vertical_carousel_core(I8U top_idx, I8U middle_idx, I8U bottom_idx)
{
	I8U string[32];		
	I8U margin = 0;
	I8U b_24_size = 24;
	
	string[0] = top_idx;
	string[1] = 0;
	_render_vertical_local_character_core(string, 4, margin, b_24_size, FALSE);
	
	string[0] = middle_idx;
	string[1] = 0;
	_render_vertical_local_character_core(string, (4+48), margin, b_24_size, FALSE);
	
	string[0] = bottom_idx;
	string[1] = 0;
	_render_vertical_local_character_core(string, (4+96), margin, b_24_size, FALSE);
}

static void _middle_render_vertical_carousel_1()
{
  _middle_render_vertical_carousel_core(ICON24_RUNNING_MODE_IDX, ICON24_CYCLING_OUTDOOR_MODE_IDX, ICON24_WORKOUT_MODE_IDX);	
}
#endif

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
static void _middle_render_vertical_carousel_2()
{
  _middle_render_vertical_carousel_core(ICON24_MUSIC_IDX, ICON24_STOPWATCH_IDX, ICON24_MESSAGE_IDX);	
}
#endif

#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	
static void _middle_render_vertical_carousel_2()
{
  _middle_render_vertical_carousel_core(ICON24_MESSAGE_IDX, ICON24_STOPWATCH_IDX, ICON24_WEATHER_IDX);	
}
#endif

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
static void _middle_render_vertical_carousel_3()
{
  _middle_render_vertical_carousel_core(ICON24_WEATHER_IDX, ICON24_PM2P5_IDX, ICON24_NORMAL_ALARM_CLOCK_IDX);	
}
#endif

#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	
static void _middle_render_vertical_carousel_3()
{
  _middle_render_vertical_carousel_core(ICON24_PM2P5_IDX, ICON24_NORMAL_ALARM_CLOCK_IDX, ICON24_SETTING_IDX);		
}
#endif

#ifdef _CLINGBAND_2_PAY_MODEL_
static void _middle_render_vertical_carousel_4()
{
	I8U string[32];		
	I8U margin = 0;
	I8U b_24_size = 24;
	
	string[0] = ICON24_BUS_CARD_IDX;
	string[1] = 0;
	_render_vertical_local_character_core(string, 0, margin, b_24_size, FALSE);
	
	string[0] = ICON24_BANK_CARD_IDX;
	string[1] = 0;
	_render_vertical_local_character_core(string, 48, margin, b_24_size, FALSE);
	
	string[0] = ICON24_SETTING_IDX;
	string[1] = 0;
	_render_vertical_local_character_core(string, (4+96), margin, b_24_size, FALSE);
}
#endif

static void _bottom_render_vertical_home()
{
	I8U string1[32];
	I8U string2[32];					
 	
	_get_home_frame_stiring_core(string1, string2);
	
  _render_vertical_fonts_lib_character_core(string2, 16, 96);
	
	_render_vertical_fonts_lib_character_core(string1, 8, 120);	
}

static void _bottom_render_vertical_small_clock()
{
	I8U string[32];

	if (cling.ui.icon_sec_blinking) {
		sprintf((char *)string, "%d:%02d",cling.time.local.hour, cling.time.local.minute);
	} else {
		sprintf((char *)string, "%d %02d",cling.time.local.hour, cling.time.local.minute);		
	}
	
  _render_vertical_fonts_lib_character_core(string, 8, 120);
}

static void _bottom_render_vertical_more()
{
	I8U string[16];
  I8U b_24_size = 8;
	I8U margin = 0;
	
  if (cling.ui.b_detail_page) {
	  string[0] = ICON8_MORE_IDX;
	  string[1] = 0;
	  _render_vertical_local_character_core(string, 120, margin, b_24_size, FALSE); 	
  }
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _bottom_render_vertical_tracker()
{
	I8U string[32];
	SYSTIME_CTX delta;

	if (cling.ui.b_detail_page) {	
		RTC_get_delta_clock_backward(&delta, cling.ui.vertical_index);	
		sprintf((char *)string, "%d/%02d", delta.month, delta.day);
		_render_vertical_fonts_lib_character_core(string, 8, 108);		
	} else {
	  _bottom_render_vertical_small_clock();
	}
	
  _bottom_render_vertical_more();	
}
#endif

#ifdef _CLINGBAND_2_PAY_MODEL_
static void _bottom_render_vertical_alarm_clock_detail()
{
	I8U string[16];
  I8U b_24_size = 8;
	I8U margin = 0;
	
	sprintf((char *)string, "%02d", cling.ui.vertical_index);
	_render_vertical_fonts_lib_character_core(string, 16, 96);

  if (cling.ui.b_detail_page) {
	  string[0] = ICON8_MORE_IDX;
	  string[1] = 0;
	  _render_vertical_local_character_core(string, 120, margin, b_24_size, FALSE); 	
  }
}

static void _bottom_render_vertical_stopwatch_start() 
{
 	I8U string[16];		
	I8U margin = 0;
	I8U b_24_size = 16;

	if (cling.ui.b_in_stopwatch_pause_mode)
		string[0] = ICON16_STOPWATCH_START_IDX;
	else
		string[0] = ICON16_STOPWATCH_STOP_IDX;

	string[1] = 0;
	_render_vertical_local_character_core(string, 110, margin, b_24_size, FALSE); 
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_
static void _bottom_render_vertical_ok()
{
	I8U string[16];		
	I8U margin = 0;
	I8U b_24_size = 16;
	
	string[0] = ICON16_OK_IDX;
	string[1] = 0;
	_render_vertical_local_character_core(string, 110, margin, b_24_size, FALSE);
}
#endif

#ifdef _CLINGBAND_PACE_MODEL_
static void _bottom_render_vertical_button_hold()
{
	const	char *button_hold_name[] = {"HOLD", "长按", "長按"};	
	I8U language_type = cling.ui.language_type;	
	
  _render_vertical_fonts_lib_invert_colors_core((I8U *)button_hold_name[language_type], 112);
}
#endif

static void _bottom_render_vertical_run_distance_uint()
{
	const char *unit_distance_display[3][2] = {{"KM", "MILE"},{"公里", "英里"},{"公裏", "英裏"}};	
	I8U language_type = cling.ui.language_type;		
	I8U metric = cling.user_data.profile.metric_distance;
	
	_render_vertical_fonts_lib_character_core((I8U *)unit_distance_display[language_type][metric], 16, 112);		
}
	
#ifndef _CLINGBAND_PACE_MODEL_
static void _bottom_render_vertical_cycling_outdoor_no_ble()
{
	const char *no_ble_name_1[] = {"NO", "无 ", "無 "};		
	const char *no_ble_name_2[] = {"BLE", "蓝牙 ", "藍牙 "};		
 	I8U language_type = cling.ui.language_type;	
	
  _middle_render_vertical_character_core((I8U *)no_ble_name_1[language_type], 92, (I8U *)no_ble_name_2[language_type], 112);			
}

static void _bottom_render_vertical_cycling_outdoor_distance()
{
	if (BTLE_is_connected()) {
		_bottom_render_vertical_run_distance_uint();
	} else {
    _bottom_render_vertical_cycling_outdoor_no_ble();
	}
}

static void _bottom_render_vertical_cycling_outdoor_speed()
{ 
	const char *unit_speed_display[] = {"KM/H", "ML/H"};	
	I8U metric = cling.user_data.profile.metric_distance;	
	
  if (BTLE_is_connected()) {	
	  _render_vertical_fonts_lib_character_core((I8U *)unit_speed_display[metric], 8, 120);
	} else {
		_bottom_render_vertical_cycling_outdoor_no_ble();	
	}
}
#endif

void UI_frame_display_appear(I8U frame_index, BOOLEAN b_render)
{
	N_SPRINTF("[UI] frame appear: %d, %d", frame_index, u->frame_cached_index);

	if (frame_index >= UI_DISPLAY_PREVIOUS)
		return;
	
	_core_frame_display(frame_index, b_render);
}

#ifdef _CLINGBAND_PACE_MODEL_
const I8U ui_matrix_horizontal_icon_16_idx[] = {
  ICON16_NONE,                      /*UI_DISPLAY_HOME*/	
  ICON16_NONE,                      /*UI_DISPLAY_SYSTEM_RESTART*/
  ICON16_NONE,                      /*UI_DISPLAY_SYSTEM_OTA*/
  ICON16_NONE,                      /*UI_DISPLAY_SYSTEM_LINKING*/
  ICON16_NONE,                      /*UI_DISPLAY_SYSTEM_UNAUTHORIZED*/
  ICON16_NONE,                      /*UI_DISPLAY_SYSTEM_BATT_POWER*/
	ICON16_STEPS_IDX,                 /*UI_DISPLAY_STEPS*/
  ICON16_DISTANCE_IDX,              /*UI_DISPLAY_DISTANCE*/
	ICON16_CALORIES_IDX,              /*UI_DISPLAY_CALORIES*/
	ICON16_ACTIVE_TIME_IDX,           /*UI_DISPLAY_ACTIVE_TIME*/
  ICON16_PM2P5_IDX,                 /*UI_DISPLAY_PM2P5*/
  ICON16_NONE,                      /*UI_DISPLAY_WEATHER*/
  ICON16_NONE,                      /*UI_DISPLAY_DETAIL_NOTIF*/	
  ICON16_INCOMING_CALL_IDX,         /*UI_DISPLAY_INCOMING_CALL*/
  ICON16_MESSAGE_IDX,               /*UI_DISPLAY_INCOMING_MESSAGE*/
  ICON16_NONE,                      /*UI_DISPLAY_ALARM_CLOCK_REMINDER*/
  ICON16_NONE,                      /*UI_DISPLAY_IDLE_ALERT*/
	ICON16_HEART_RATE_IDX,            /*UI_DISPLAY_HEART_RATE_ALERT*/
	ICON16_STEPS_IDX,                 /*UI_DISPLAY_STEP_10K_ALERT*/
	ICON16_HEART_RATE_IDX,            /*UI_DISPLAY_HEART_RATE*/                          
  ICON16_RUNNING_PACE_IDX,          /*UI_DISPLAY_RUNNING_ANALYSIS*/
  ICON16_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_RUNNING_DISTANCE*/
	ICON16_RUNNING_TIME_IDX,          /*UI_DISPLAY_RUNNING_TIME*/
	ICON16_RUNNING_PACE_IDX,          /*UI_DISPLAY_RUNNING_PACE*/
	ICON16_RUNNING_STRIDE_IDX,        /*UI_DISPLAY_RUNNING_STRIDE*/	
	ICON16_RUNNING_CADENCE_IDX,       /*UI_DISPLAY_RUNNING_CADENCE*/	
	ICON16_RUNNING_HR_IDX,            /*UI_DISPLAY_RUNNING_HEART_RATE*/
	ICON16_RUNNING_CALORIES_IDX,      /*UI_DISPLAY_RUNNING_CALORIES*/
	ICON16_RUNNING_STOP_IDX,          /*UI_DISPLAY_RUNNING_STOP_ANALYSIS*/
	ICON16_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_RUN_START*/
	ICON16_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_READY*/
	ICON16_RUNNING_TIME_IDX,          /*UI_DISPLAY_TRAINING_TIME*/
	ICON16_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_DISTANCE*/
	ICON16_RUNNING_PACE_IDX,          /*UI_DISPLAY_TRAINING_PACE*/
	ICON16_RUNNING_HR_IDX,            /*UI_DISPLAY_TRAINING_HEART_RATE*/
	ICON16_RUNNING_STOP_IDX,          /*UI_DISPLAY_TRAINING_RUN_STOP*/
  ICON16_NONE,                      /*UI_DISPLAY_PREVIOUS*/			
};
#endif

#ifdef _CLINGBAND_UV_MODEL_
const I8U ui_matrix_horizontal_icon_16_idx[] = {
  ICON16_NONE,                      /*UI_DISPLAY_HOME*/	
  ICON16_NONE,                      /*UI_DISPLAY_SYSTEM_RESTART*/
  ICON16_NONE,                      /*UI_DISPLAY_SYSTEM_OTA*/
  ICON16_NONE,                      /*UI_DISPLAY_SYSTEM_LINKING*/
  ICON16_NONE,                      /*UI_DISPLAY_SYSTEM_UNAUTHORIZED*/
  ICON16_NONE,                      /*UI_DISPLAY_SYSTEM_BATT_POWER*/
	ICON16_STEPS_IDX,                 /*UI_DISPLAY_STEPS*/
  ICON16_DISTANCE_IDX,              /*UI_DISPLAY_DISTANCE*/
	ICON16_CALORIES_IDX,              /*UI_DISPLAY_CALORIES*/
	ICON16_ACTIVE_TIME_IDX,           /*UI_DISPLAY_ACTIVE_TIME*/
	ICON16_UV_INDEX_IDX,              /*UI_DISPLAY_UV_IDX*/	
  ICON16_PM2P5_IDX,                 /*UI_DISPLAY_PM2P5*/
  ICON16_NONE,                      /*UI_DISPLAY_WEATHER*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_MESSAGE*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_APP_NOTIF*/
  ICON16_NONE,                      /*UI_DISPLAY_DETAIL_NOTIF*/	
  ICON16_INCOMING_CALL_IDX,         /*UI_DISPLAY_INCOMING_CALL*/
  ICON16_MESSAGE_IDX,               /*UI_DISPLAY_INCOMING_MESSAGE*/
  ICON16_NONE,                      /*UI_DISPLAY_ALARM_CLOCK_REMINDER*/
  ICON16_NORMAL_ALARM_CLOCK_IDX,    /*UI_DISPLAY_ALARM_CLOCK_DETAIL*/
  ICON16_NONE,                      /*UI_DISPLAY_IDLE_ALERT*/
	ICON16_HEART_RATE_IDX,            /*UI_DISPLAY_HEART_RATE_ALERT*/
	ICON16_STEPS_IDX,                 /*UI_DISPLAY_STEP_10K_ALERT*/
	ICON16_NONE,                      /*UI_DISPLAY_SMART_SOS_ALERT*/		
	ICON16_HEART_RATE_IDX,            /*UI_DISPLAY_HEART_RATE*/
	ICON16_SKIN_TEMP_IDX,             /*UI_DISPLAY_SKIN_TEMP*/
  ICON16_SETTING_IDX,               /*UI_DISPLAY_SETTING*/
  ICON16_STOPWATCH_IDX,             /*UI_DISPLAY_STOPWATCH_START*/
  ICON16_STOPWATCH_IDX,             /*UI_DISPLAY_STOPWATCH_STOP*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_TREADMILL*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_CYCLING*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_STAIRS*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_ELLIPTICAL*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_ROW*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_AEROBIC*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_PILOXING*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_OTHERS*/
  ICON16_NONE,                      /*UI_DISPLAY_WORKOUT_RT_READY*/
  ICON16_RUNNING_TIME_IDX,          /*UI_DISPLAY_WORKOUT_RT_TIME*/
  ICON16_RUNNING_HR_IDX,            /*UI_DISPLAY_WORKOUT_RT_HEART_RATE*/
  ICON16_RUNNING_CALORIES_IDX,      /*UI_DISPLAY_WORKOUT_RT_CALORIES*/
  ICON16_RUNNING_STOP_IDX,          /*UI_DISPLAY_WORKOUT_RT_END*/
  ICON16_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_RUNNING_DISTANCE*/
	ICON16_RUNNING_TIME_IDX,          /*UI_DISPLAY_RUNNING_TIME*/
	ICON16_RUNNING_PACE_IDX,          /*UI_DISPLAY_RUNNING_PACE*/
	ICON16_RUNNING_STRIDE_IDX,        /*UI_DISPLAY_RUNNING_STRIDE*/	
	ICON16_RUNNING_CADENCE_IDX,       /*UI_DISPLAY_RUNNING_CADENCE*/	
	ICON16_RUNNING_HR_IDX,            /*UI_DISPLAY_RUNNING_HEART_RATE*/
	ICON16_RUNNING_CALORIES_IDX,      /*UI_DISPLAY_RUNNING_CALORIES*/
	ICON16_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_RUN_START*/
	ICON16_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_RUN_OR_ANALYSIS*/
	ICON16_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_READY*/
	ICON16_RUNNING_TIME_IDX,          /*UI_DISPLAY_TRAINING_TIME*/
	ICON16_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_DISTANCE*/
	ICON16_RUNNING_PACE_IDX,          /*UI_DISPLAY_TRAINING_PACE*/
	ICON16_RUNNING_HR_IDX,            /*UI_DISPLAY_TRAINING_HEART_RATE*/
	ICON16_RUNNING_STOP_IDX,          /*UI_DISPLAY_TRAINING_RUN_STOP*/
	ICON16_CYCLING_OUTDOOR_MODE_IDX,  /*UI_DISPLAY_CYCLING_OUTDOOR_RUN_START*/
	ICON16_CYCLING_OUTDOOR_MODE_IDX,  /*UI_DISPLAY_CYCLING_OUTDOOR_READY*/
	ICON16_RUNNING_TIME_IDX,          /*UI_DISPLAY_CYCLING_OUTDOOR_TIME*/
	ICON16_CYCLING_OUTDOOR_MODE_IDX,  /*UI_DISPLAY_CYCLING_OUTDOOR_DISTANCE*/
	ICON16_CYCLING_OUTDOOR_SPEED_IDX, /*UI_DISPLAY_CYCLING_OUTDOOR_SPEED*/
  ICON16_RUNNING_HR_IDX,            /*UI_DISPLAY_CYCLING_OUTDOOR_HEART_RATE*/
	ICON16_RUNNING_STOP_IDX,          /*UI_DISPLAY_CYCLING_OUTDOOR_STATATISTICS_END*/
  ICON16_NONE,                      /*UI_DISPLAY_CAROUSEL_1*/
  ICON16_NONE,                      /*UI_DISPLAY_CAROUSEL_2*/
  ICON16_NONE,                      /*UI_DISPLAY_CAROUSEL_3*/
  ICON16_NONE,                      /*UI_DISPLAY_PREVIOUS*/			
};
#endif

#ifdef _CLINGBAND_NFC_MODEL_
const I8U ui_matrix_horizontal_icon_16_idx[] = {
  ICON16_NONE,                      /*UI_DISPLAY_HOME*/	
  ICON16_NONE,                      /*UI_DISPLAY_SYSTEM_RESTART*/
  ICON16_NONE,                      /*UI_DISPLAY_SYSTEM_OTA*/
  ICON16_NONE,                      /*UI_DISPLAY_SYSTEM_LINKING*/
  ICON16_NONE,                      /*UI_DISPLAY_SYSTEM_UNAUTHORIZED*/
  ICON16_NONE,                      /*UI_DISPLAY_SYSTEM_BATT_POWER*/
	ICON16_STEPS_IDX,                 /*UI_DISPLAY_STEPS*/
  ICON16_DISTANCE_IDX,              /*UI_DISPLAY_DISTANCE*/
	ICON16_CALORIES_IDX,              /*UI_DISPLAY_CALORIES*/
	ICON16_ACTIVE_TIME_IDX,           /*UI_DISPLAY_ACTIVE_TIME*/
  ICON16_PM2P5_IDX,                 /*UI_DISPLAY_PM2P5*/
  ICON16_NONE,                      /*UI_DISPLAY_WEATHER*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_MESSAGE*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_APP_NOTIF*/
  ICON16_NONE,                      /*UI_DISPLAY_DETAIL_NOTIF*/	
  ICON16_INCOMING_CALL_IDX,         /*UI_DISPLAY_INCOMING_CALL*/
  ICON16_MESSAGE_IDX,               /*UI_DISPLAY_INCOMING_MESSAGE*/
  ICON16_NONE,                      /*UI_DISPLAY_ALARM_CLOCK_REMINDER*/
  ICON16_NORMAL_ALARM_CLOCK_IDX,    /*UI_DISPLAY_ALARM_CLOCK_DETAIL*/
  ICON16_NONE,                      /*UI_DISPLAY_IDLE_ALERT*/
	ICON16_HEART_RATE_IDX,            /*UI_DISPLAY_HEART_RATE_ALERT*/
	ICON16_STEPS_IDX,                 /*UI_DISPLAY_STEP_10K_ALERT*/
	ICON16_NONE,                      /*UI_DISPLAY_SMART_SOS_ALERT*/	
	ICON16_HEART_RATE_IDX,            /*UI_DISPLAY_HEART_RATE*/
	ICON16_SKIN_TEMP_IDX,             /*UI_DISPLAY_SKIN_TEMP*/
  ICON16_SETTING_IDX,               /*UI_DISPLAY_SETTING*/
  ICON16_STOPWATCH_IDX,             /*UI_DISPLAY_STOPWATCH_START*/
  ICON16_STOPWATCH_IDX,             /*UI_DISPLAY_STOPWATCH_STOP*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_TREADMILL*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_CYCLING*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_STAIRS*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_ELLIPTICAL*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_ROW*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_AEROBIC*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_PILOXING*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_OTHERS*/
  ICON16_NONE,                      /*UI_DISPLAY_WORKOUT_RT_READY*/
  ICON16_RUNNING_TIME_IDX,          /*UI_DISPLAY_WORKOUT_RT_TIME*/
  ICON16_RUNNING_HR_IDX,            /*UI_DISPLAY_WORKOUT_RT_HEART_RATE*/
  ICON16_RUNNING_CALORIES_IDX,      /*UI_DISPLAY_WORKOUT_RT_CALORIES*/
  ICON16_RUNNING_STOP_IDX,          /*UI_DISPLAY_WORKOUT_RT_END*/
  ICON16_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_RUNNING_DISTANCE*/
	ICON16_RUNNING_TIME_IDX,          /*UI_DISPLAY_RUNNING_TIME*/
	ICON16_RUNNING_PACE_IDX,          /*UI_DISPLAY_RUNNING_PACE*/
	ICON16_RUNNING_STRIDE_IDX,        /*UI_DISPLAY_RUNNING_STRIDE*/	
	ICON16_RUNNING_CADENCE_IDX,       /*UI_DISPLAY_RUNNING_CADENCE*/	
	ICON16_RUNNING_HR_IDX,            /*UI_DISPLAY_RUNNING_HEART_RATE*/
	ICON16_RUNNING_CALORIES_IDX,      /*UI_DISPLAY_RUNNING_CALORIES*/
	ICON16_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_RUN_START*/
	ICON16_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_RUN_OR_ANALYSIS*/
	ICON16_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_READY*/
	ICON16_RUNNING_TIME_IDX,          /*UI_DISPLAY_TRAINING_TIME*/
	ICON16_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_DISTANCE*/
	ICON16_RUNNING_PACE_IDX,          /*UI_DISPLAY_TRAINING_PACE*/
	ICON16_RUNNING_HR_IDX,            /*UI_DISPLAY_TRAINING_HEART_RATE*/
	ICON16_RUNNING_STOP_IDX,          /*UI_DISPLAY_TRAINING_RUN_STOP*/
	ICON16_CYCLING_OUTDOOR_MODE_IDX,  /*UI_DISPLAY_CYCLING_OUTDOOR_RUN_START*/
	ICON16_CYCLING_OUTDOOR_MODE_IDX,  /*UI_DISPLAY_CYCLING_OUTDOOR_READY*/
	ICON16_RUNNING_TIME_IDX,          /*UI_DISPLAY_CYCLING_OUTDOOR_TIME*/
	ICON16_CYCLING_OUTDOOR_MODE_IDX,  /*UI_DISPLAY_CYCLING_OUTDOOR_DISTANCE*/
	ICON16_CYCLING_OUTDOOR_SPEED_IDX, /*UI_DISPLAY_CYCLING_OUTDOOR_SPEED*/
  ICON16_RUNNING_HR_IDX,            /*UI_DISPLAY_CYCLING_OUTDOOR_HEART_RATE*/
	ICON16_RUNNING_STOP_IDX,          /*UI_DISPLAY_CYCLING_OUTDOOR_STATATISTICS_END*/
  ICON16_NONE,                      /*UI_DISPLAY_CAROUSEL_1*/
  ICON16_NONE,                      /*UI_DISPLAY_CAROUSEL_2*/
  ICON16_NONE,                      /*UI_DISPLAY_CAROUSEL_3*/
  ICON16_NONE,                      /*UI_DISPLAY_PREVIOUS*/			
};
#endif

#ifdef _CLINGBAND_2_PAY_MODEL_
const I8U ui_matrix_horizontal_icon_16_idx[] = {
  ICON16_NONE,                      /*UI_DISPLAY_HOME*/	
  ICON16_NONE,                      /*UI_DISPLAY_SYSTEM_RESTART*/
  ICON16_NONE,                      /*UI_DISPLAY_SYSTEM_OTA*/
  ICON16_NONE,                      /*UI_DISPLAY_SYSTEM_LINKING*/
  ICON16_NONE,                      /*UI_DISPLAY_SYSTEM_UNAUTHORIZED*/
  ICON16_NONE,                      /*UI_DISPLAY_SYSTEM_BATT_POWER*/
	ICON16_STEPS_IDX,                 /*UI_DISPLAY_STEPS*/
  ICON16_DISTANCE_IDX,              /*UI_DISPLAY_DISTANCE*/
	ICON16_CALORIES_IDX,              /*UI_DISPLAY_CALORIES*/
	ICON16_ACTIVE_TIME_IDX,           /*UI_DISPLAY_ACTIVE_TIME*/
  ICON16_PM2P5_IDX,                 /*UI_DISPLAY_PM2P5*/
  ICON16_NONE,                      /*UI_DISPLAY_WEATHER*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_MESSAGE*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_APP_NOTIF*/
  ICON16_NONE,                      /*UI_DISPLAY_DETAIL_NOTIF*/	
  ICON16_INCOMING_CALL_IDX,         /*UI_DISPLAY_INCOMING_CALL*/
  ICON16_MESSAGE_IDX,               /*UI_DISPLAY_INCOMING_MESSAGE*/
  ICON16_NONE,                      /*UI_DISPLAY_ALARM_CLOCK_REMINDER*/
  ICON16_NORMAL_ALARM_CLOCK_IDX,    /*UI_DISPLAY_ALARM_CLOCK_DETAIL*/
  ICON16_NONE,                      /*UI_DISPLAY_IDLE_ALERT*/
	ICON16_HEART_RATE_IDX,            /*UI_DISPLAY_HEART_RATE_ALERT*/
	ICON16_STEPS_IDX,                 /*UI_DISPLAY_STEP_10K_ALERT*/
	ICON16_HEART_RATE_IDX,            /*UI_DISPLAY_HEART_RATE*/
  ICON16_SETTING_IDX,               /*UI_DISPLAY_SETTING*/
  ICON16_STOPWATCH_IDX,             /*UI_DISPLAY_STOPWATCH_START*/
  ICON16_STOPWATCH_IDX,             /*UI_DISPLAY_STOPWATCH_STOP*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_TREADMILL*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_CYCLING*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_STAIRS*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_ELLIPTICAL*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_ROW*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_AEROBIC*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_PILOXING*/
  ICON16_RETURN_IDX,                /*UI_DISPLAY_WORKOUT_OTHERS*/
  ICON16_NONE,                      /*UI_DISPLAY_WORKOUT_RT_READY*/
  ICON16_RUNNING_TIME_IDX,          /*UI_DISPLAY_WORKOUT_RT_TIME*/
  ICON16_RUNNING_HR_IDX,            /*UI_DISPLAY_WORKOUT_RT_HEART_RATE*/
  ICON16_RUNNING_CALORIES_IDX,      /*UI_DISPLAY_WORKOUT_RT_CALORIES*/
  ICON16_RUNNING_STOP_IDX,          /*UI_DISPLAY_WORKOUT_RT_END*/
  ICON16_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_RUNNING_DISTANCE*/
	ICON16_RUNNING_TIME_IDX,          /*UI_DISPLAY_RUNNING_TIME*/
	ICON16_RUNNING_PACE_IDX,          /*UI_DISPLAY_RUNNING_PACE*/
	ICON16_RUNNING_STRIDE_IDX,        /*UI_DISPLAY_RUNNING_STRIDE*/	
	ICON16_RUNNING_CADENCE_IDX,       /*UI_DISPLAY_RUNNING_CADENCE*/	
	ICON16_RUNNING_HR_IDX,            /*UI_DISPLAY_RUNNING_HEART_RATE*/
	ICON16_RUNNING_CALORIES_IDX,      /*UI_DISPLAY_RUNNING_CALORIES*/
	ICON16_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_RUN_START*/
	ICON16_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_RUN_OR_ANALYSIS*/
	ICON16_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_READY*/
	ICON16_RUNNING_TIME_IDX,          /*UI_DISPLAY_TRAINING_TIME*/
	ICON16_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_DISTANCE*/
	ICON16_RUNNING_PACE_IDX,          /*UI_DISPLAY_TRAINING_PACE*/
	ICON16_RUNNING_HR_IDX,            /*UI_DISPLAY_TRAINING_HEART_RATE*/
	ICON16_RUNNING_STOP_IDX,          /*UI_DISPLAY_TRAINING_RUN_STOP*/
	ICON16_CYCLING_OUTDOOR_MODE_IDX,  /*UI_DISPLAY_CYCLING_OUTDOOR_RUN_START*/
	ICON16_CYCLING_OUTDOOR_MODE_IDX,  /*UI_DISPLAY_CYCLING_OUTDOOR_READY*/
	ICON16_RUNNING_TIME_IDX,          /*UI_DISPLAY_CYCLING_OUTDOOR_TIME*/
	ICON16_CYCLING_OUTDOOR_MODE_IDX,  /*UI_DISPLAY_CYCLING_OUTDOOR_DISTANCE*/
	ICON16_CYCLING_OUTDOOR_SPEED_IDX, /*UI_DISPLAY_CYCLING_OUTDOOR_SPEED*/
  ICON16_RUNNING_HR_IDX,            /*UI_DISPLAY_CYCLING_OUTDOOR_HEART_RATE*/
	ICON16_RUNNING_STOP_IDX,          /*UI_DISPLAY_CYCLING_OUTDOOR_STATATISTICS_END*/
  ICON16_NONE,                      /*UI_DISPLAY_MUSIC_PLAY_PAUSE*/
  ICON16_NONE,                      /*UI_DISPLAY_MUSIC_TRACK*/
  ICON16_NONE,                      /*UI_DISPLAY_MUSIC_VOLUME*/
	ICON16_BUS_CARD_IDX,              /*UI_DISPLAY_PAY_BUS_CARD_BALANCE_ENQUIRY*/	
  ICON16_BANK_CARD_IDX,             /*UI_DISPLAY_PAY_BANK_CARD_BALANCE_ENQUIRY*/		
  ICON16_NONE,                      /*UI_DISPLAY_CAROUSEL_1*/
  ICON16_NONE,                      /*UI_DISPLAY_CAROUSEL_2*/
  ICON16_NONE,                      /*UI_DISPLAY_CAROUSEL_3*/
  ICON16_NONE,                      /*UI_DISPLAY_CAROUSEL_4*/
  ICON16_NONE,                      /*UI_DISPLAY_PREVIOUS*/		
};
#endif

#ifdef _CLINGBAND_PACE_MODEL_
const I8U ui_matrix_vertical_icon_24_idx[] = {
  ICON24_NONE,                      /*UI_DISPLAY_HOME*/	
  ICON24_NONE,                      /*UI_DISPLAY_SYSTEM_RESTART*/
  ICON24_NONE,                      /*UI_DISPLAY_SYSTEM_OTA*/
  ICON24_NONE,                      /*UI_DISPLAY_SYSTEM_LINKING*/
  ICON24_NONE,                      /*UI_DISPLAY_SYSTEM_UNAUTHORIZED*/
  ICON24_NONE,                      /*UI_DISPLAY_SYSTEM_BATT_POWER*/
	ICON24_STEPS_IDX,                 /*UI_DISPLAY_STEPS*/
  ICON24_DISTANCE_IDX,              /*UI_DISPLAY_DISTANCE*/
	ICON24_CALORIES_IDX,              /*UI_DISPLAY_CALORIES*/
	ICON24_ACTIVE_TIME_IDX,           /*UI_DISPLAY_ACTIVE_TIME*/
  ICON24_PM2P5_IDX,                 /*UI_DISPLAY_PM2P5*/
  ICON24_NONE,                      /*UI_DISPLAY_WEATHER*/
  ICON24_NONE,                      /*UI_DISPLAY_DETAIL_NOTIF*/	
  ICON24_NONE,                      /*UI_DISPLAY_INCOMING_CALL*/
  ICON24_NONE,                      /*UI_DISPLAY_INCOMING_MESSAGE*/
  ICON24_NONE,                      /*UI_DISPLAY_ALARM_CLOCK_REMINDER*/
  ICON24_NONE,                      /*UI_DISPLAY_IDLE_ALERT*/
	ICON24_HEART_RATE_IDX,            /*UI_DISPLAY_HEART_RATE_ALERT*/
	ICON24_STEPS_IDX,                 /*UI_DISPLAY_STEP_10K_ALERT*/
	ICON24_HEART_RATE_IDX,            /*UI_DISPLAY_HEART_RATE*/
  ICON24_RUNNING_PACE_IDX,          /*UI_DISPLAY_RUNNING_ANALYSIS*/
  ICON24_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_RUNNING_DISTANCE*/
	ICON24_RUNNING_TIME_IDX,          /*UI_DISPLAY_RUNNING_TIME*/
	ICON24_RUNNING_PACE_IDX,          /*UI_DISPLAY_RUNNING_PACE*/
	ICON24_RUNNING_STRIDE_IDX,        /*UI_DISPLAY_RUNNING_STRIDE*/
	ICON24_RUNNING_CADENCE_IDX,       /*UI_DISPLAY_RUNNING_CADENCE*/	
	ICON24_RUNNING_HR_IDX,            /*UI_DISPLAY_RUNNING_HEART_RATE*/
	ICON24_RUNNING_CALORIES_IDX,      /*UI_DISPLAY_RUNNING_CALORIES*/
	ICON24_RUNNING_STOP_IDX,          /*UI_DISPLAY_RUNNING_STOP_ANALYSIS*/
	ICON24_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_RUN_START*/
	ICON24_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_READY*/
	ICON24_RUNNING_TIME_IDX,          /*UI_DISPLAY_TRAINING_TIME*/
	ICON24_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_DISTANCE*/
	ICON24_RUNNING_PACE_IDX,          /*UI_DISPLAY_TRAINING_PACE*/
	ICON24_RUNNING_HR_IDX,            /*UI_DISPLAY_TRAINING_HEART_RATE*/
	ICON24_RUNNING_STOP_IDX,          /*UI_DISPLAY_TRAINING_RUN_STOP*/
  ICON24_NONE,                      /*UI_DISPLAY_PREVIOUS*/		
};
#endif

#ifdef _CLINGBAND_UV_MODEL_
const I8U ui_matrix_vertical_icon_24_idx[] = {
  ICON24_NONE,                      /*UI_DISPLAY_HOME*/	
  ICON24_NONE,                      /*UI_DISPLAY_SYSTEM_RESTART*/
  ICON24_NONE,                      /*UI_DISPLAY_SYSTEM_OTA*/
  ICON24_NONE,                      /*UI_DISPLAY_SYSTEM_LINKING*/
  ICON24_NONE,                      /*UI_DISPLAY_SYSTEM_UNAUTHORIZED*/
  ICON24_NONE,                      /*UI_DISPLAY_SYSTEM_BATT_POWER*/
	ICON24_STEPS_IDX,                 /*UI_DISPLAY_STEPS*/
  ICON24_DISTANCE_IDX,              /*UI_DISPLAY_DISTANCE*/
	ICON24_CALORIES_IDX,              /*UI_DISPLAY_CALORIES*/
	ICON24_ACTIVE_TIME_IDX,           /*UI_DISPLAY_ACTIVE_TIME*/
	ICON24_UV_INDEX_IDX,              /*UI_DISPLAY_UV_IDX*/		
  ICON24_NONE,                      /*UI_DISPLAY_PM2P5*/
  ICON24_NONE,                      /*UI_DISPLAY_WEATHER*/
  ICON24_NONE,                      /*UI_DISPLAY_MESSAGE*/
  ICON24_NONE,                      /*UI_DISPLAY_APP_NOTIF*/
  ICON24_NONE,                      /*UI_DISPLAY_DETAIL_NOTIF*/	
  ICON24_NONE,                      /*UI_DISPLAY_INCOMING_CALL*/
  ICON24_NONE,                      /*UI_DISPLAY_INCOMING_MESSAGE*/
  ICON24_NONE,                      /*UI_DISPLAY_ALARM_CLOCK_REMINDER*/
  ICON24_NONE,                      /*UI_DISPLAY_ALARM_CLOCK_DETAIL*/
  ICON24_NONE,                      /*UI_DISPLAY_IDLE_ALERT*/
	ICON24_HEART_RATE_IDX,            /*UI_DISPLAY_HEART_RATE_ALERT*/
	ICON24_STEPS_IDX,                 /*UI_DISPLAY_STEP_10K_ALERT*/
	ICON24_NONE,                      /*UI_DISPLAY_SMART_SOS_ALERT*/	
	ICON24_HEART_RATE_IDX,            /*UI_DISPLAY_HEART_RATE*/
	ICON24_SKIN_TEMP_IDX,             /*UI_DISPLAY_SKIN_TEMP*/
  ICON24_NONE,                      /*UI_DISPLAY_SETTING*/
  ICON24_NONE,                      /*UI_DISPLAY_STOPWATCH_START*/
  ICON24_NONE,                      /*UI_DISPLAY_STOPWATCH_STOP*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_TREADMILL*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_CYCLING*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_STAIRS*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_ELLIPTICAL*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_ROW*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_AEROBIC*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_PILOXING*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_OTHERS*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_RT_READY*/
  ICON24_RUNNING_TIME_IDX,          /*UI_DISPLAY_WORKOUT_RT_TIME*/
  ICON24_RUNNING_HR_IDX,            /*UI_DISPLAY_WORKOUT_RT_HEART_RATE*/
  ICON24_RUNNING_CALORIES_IDX,      /*UI_DISPLAY_WORKOUT_RT_CALORIES*/
  ICON24_RUNNING_STOP_IDX,          /*UI_DISPLAY_WORKOUT_RT_END*/
  ICON24_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_RUNNING_DISTANCE*/
	ICON24_RUNNING_TIME_IDX,          /*UI_DISPLAY_RUNNING_TIME*/
	ICON24_RUNNING_PACE_IDX,          /*UI_DISPLAY_RUNNING_PACE*/
	ICON24_RUNNING_STRIDE_IDX,        /*UI_DISPLAY_RUNNING_STRIDE*/	
	ICON24_RUNNING_CADENCE_IDX,       /*UI_DISPLAY_RUNNING_CADENCE*/	
	ICON24_RUNNING_HR_IDX,            /*UI_DISPLAY_RUNNING_HEART_RATE*/
	ICON24_RUNNING_CALORIES_IDX,      /*UI_DISPLAY_RUNNING_CALORIES*/
	ICON24_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_RUN_START*/
	ICON24_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_RUN_OR_ANALYSIS*/
	ICON24_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_READY*/
	ICON24_RUNNING_TIME_IDX,          /*UI_DISPLAY_TRAINING_TIME*/
	ICON24_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_DISTANCE*/
	ICON24_RUNNING_PACE_IDX,          /*UI_DISPLAY_TRAINING_PACE*/
	ICON24_RUNNING_HR_IDX,            /*UI_DISPLAY_TRAINING_HEART_RATE*/
	ICON24_RUNNING_STOP_IDX,          /*UI_DISPLAY_TRAINING_RUN_STOP*/
	ICON24_CYCLING_OUTDOOR_MODE_IDX,  /*UI_DISPLAY_CYCLING_OUTDOOR_RUN_START*/
	ICON24_CYCLING_OUTDOOR_MODE_IDX,  /*UI_DISPLAY_CYCLING_OUTDOOR_READY*/
	ICON24_RUNNING_TIME_IDX,          /*UI_DISPLAY_CYCLING_OUTDOOR_TIME*/
	ICON24_CYCLING_OUTDOOR_MODE_IDX,  /*UI_DISPLAY_CYCLING_OUTDOOR_DISTANCE*/
	ICON24_CYCLING_OUTDOOR_SPEED_IDX, /*UI_DISPLAY_CYCLING_OUTDOOR_SPEED*/
  ICON24_RUNNING_HR_IDX,            /*UI_DISPLAY_CYCLING_OUTDOOR_HEART_RATE*/
	ICON24_RUNNING_STOP_IDX,          /*UI_DISPLAY_CYCLING_OUTDOOR_STATATISTICS_END*/
  ICON24_NONE,                      /*UI_DISPLAY_CAROUSEL_1*/
  ICON24_NONE,                      /*UI_DISPLAY_CAROUSEL_2*/
  ICON24_NONE,                      /*UI_DISPLAY_CAROUSEL_3*/
  ICON24_NONE,                      /*UI_DISPLAY_PREVIOUS*/		
};
#endif

#ifdef _CLINGBAND_NFC_MODEL_
const I8U ui_matrix_vertical_icon_24_idx[] = {
  ICON24_NONE,                      /*UI_DISPLAY_HOME*/	
  ICON24_NONE,                      /*UI_DISPLAY_SYSTEM_RESTART*/
  ICON24_NONE,                      /*UI_DISPLAY_SYSTEM_OTA*/
  ICON24_NONE,                      /*UI_DISPLAY_SYSTEM_LINKING*/
  ICON24_NONE,                      /*UI_DISPLAY_SYSTEM_UNAUTHORIZED*/
  ICON24_NONE,                      /*UI_DISPLAY_SYSTEM_BATT_POWER*/
	ICON24_STEPS_IDX,                 /*UI_DISPLAY_STEPS*/
  ICON24_DISTANCE_IDX,              /*UI_DISPLAY_DISTANCE*/
	ICON24_CALORIES_IDX,              /*UI_DISPLAY_CALORIES*/
	ICON24_ACTIVE_TIME_IDX,           /*UI_DISPLAY_ACTIVE_TIME*/
  ICON24_NONE,                      /*UI_DISPLAY_PM2P5*/
  ICON24_NONE,                      /*UI_DISPLAY_WEATHER*/
  ICON24_NONE,                      /*UI_DISPLAY_MESSAGE*/
  ICON24_NONE,                      /*UI_DISPLAY_APP_NOTIF*/
  ICON24_NONE,                      /*UI_DISPLAY_DETAIL_NOTIF*/	
  ICON24_NONE,                      /*UI_DISPLAY_INCOMING_CALL*/
  ICON24_NONE,                      /*UI_DISPLAY_INCOMING_MESSAGE*/
  ICON24_NONE,                      /*UI_DISPLAY_ALARM_CLOCK_REMINDER*/
  ICON24_NONE,                      /*UI_DISPLAY_ALARM_CLOCK_DETAIL*/
  ICON24_NONE,                      /*UI_DISPLAY_IDLE_ALERT*/
	ICON24_HEART_RATE_IDX,            /*UI_DISPLAY_HEART_RATE_ALERT*/
	ICON24_STEPS_IDX,                 /*UI_DISPLAY_STEP_10K_ALERT*/
	ICON24_NONE,                      /*UI_DISPLAY_SMART_SOS_ALERT*/
	ICON24_HEART_RATE_IDX,            /*UI_DISPLAY_HEART_RATE*/
	ICON24_SKIN_TEMP_IDX,             /*UI_DISPLAY_SKIN_TEMP*/
  ICON24_NONE,                      /*UI_DISPLAY_SETTING*/
  ICON24_NONE,                      /*UI_DISPLAY_STOPWATCH_START*/
  ICON24_NONE,                      /*UI_DISPLAY_STOPWATCH_STOP*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_TREADMILL*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_CYCLING*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_STAIRS*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_ELLIPTICAL*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_ROW*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_AEROBIC*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_PILOXING*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_OTHERS*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_RT_READY*/
  ICON24_RUNNING_TIME_IDX,          /*UI_DISPLAY_WORKOUT_RT_TIME*/
  ICON24_RUNNING_HR_IDX,            /*UI_DISPLAY_WORKOUT_RT_HEART_RATE*/
  ICON24_RUNNING_CALORIES_IDX,      /*UI_DISPLAY_WORKOUT_RT_CALORIES*/
  ICON24_RUNNING_STOP_IDX,          /*UI_DISPLAY_WORKOUT_RT_END*/
  ICON24_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_RUNNING_DISTANCE*/
	ICON24_RUNNING_TIME_IDX,          /*UI_DISPLAY_RUNNING_TIME*/
	ICON24_RUNNING_PACE_IDX,          /*UI_DISPLAY_RUNNING_PACE*/
	ICON24_RUNNING_STRIDE_IDX,        /*UI_DISPLAY_RUNNING_STRIDE*/	
	ICON24_RUNNING_CADENCE_IDX,       /*UI_DISPLAY_RUNNING_CADENCE*/	
	ICON24_RUNNING_HR_IDX,            /*UI_DISPLAY_RUNNING_HEART_RATE*/
	ICON24_RUNNING_CALORIES_IDX,      /*UI_DISPLAY_RUNNING_CALORIES*/
	ICON24_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_RUN_START*/
	ICON24_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_RUN_OR_ANALYSIS*/
	ICON24_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_READY*/
	ICON24_RUNNING_TIME_IDX,          /*UI_DISPLAY_TRAINING_TIME*/
	ICON24_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_DISTANCE*/
	ICON24_RUNNING_PACE_IDX,          /*UI_DISPLAY_TRAINING_PACE*/
	ICON24_RUNNING_HR_IDX,            /*UI_DISPLAY_TRAINING_HEART_RATE*/
	ICON24_RUNNING_STOP_IDX,          /*UI_DISPLAY_TRAINING_RUN_STOP*/
	ICON24_CYCLING_OUTDOOR_MODE_IDX,  /*UI_DISPLAY_CYCLING_OUTDOOR_RUN_START*/
	ICON24_CYCLING_OUTDOOR_MODE_IDX,  /*UI_DISPLAY_CYCLING_OUTDOOR_READY*/
	ICON24_RUNNING_TIME_IDX,          /*UI_DISPLAY_CYCLING_OUTDOOR_TIME*/
	ICON24_CYCLING_OUTDOOR_MODE_IDX,  /*UI_DISPLAY_CYCLING_OUTDOOR_DISTANCE*/
	ICON24_CYCLING_OUTDOOR_SPEED_IDX, /*UI_DISPLAY_CYCLING_OUTDOOR_SPEED*/
  ICON24_RUNNING_HR_IDX,            /*UI_DISPLAY_CYCLING_OUTDOOR_HEART_RATE*/
	ICON24_RUNNING_STOP_IDX,          /*UI_DISPLAY_CYCLING_OUTDOOR_STATATISTICS_END*/
  ICON24_NONE,                      /*UI_DISPLAY_CAROUSEL_1*/
  ICON24_NONE,                      /*UI_DISPLAY_CAROUSEL_2*/
  ICON24_NONE,                      /*UI_DISPLAY_CAROUSEL_3*/
  ICON24_NONE,                      /*UI_DISPLAY_PREVIOUS*/		
};
#endif

#ifdef _CLINGBAND_2_PAY_MODEL_
const I8U ui_matrix_vertical_icon_24_idx[] = {
  ICON24_NONE,                      /*UI_DISPLAY_HOME*/	
  ICON24_NONE,                      /*UI_DISPLAY_SYSTEM_RESTART*/
  ICON24_NONE,                      /*UI_DISPLAY_SYSTEM_OTA*/
  ICON24_NONE,                      /*UI_DISPLAY_SYSTEM_LINKING*/
  ICON24_NONE,                      /*UI_DISPLAY_SYSTEM_UNAUTHORIZED*/
  ICON24_NONE,                      /*UI_DISPLAY_SYSTEM_BATT_POWER*/
	ICON24_STEPS_IDX,                 /*UI_DISPLAY_STEPS*/
  ICON24_DISTANCE_IDX,              /*UI_DISPLAY_DISTANCE*/
	ICON24_CALORIES_IDX,              /*UI_DISPLAY_CALORIES*/
	ICON24_ACTIVE_TIME_IDX,           /*UI_DISPLAY_ACTIVE_TIME*/
  ICON24_PM2P5_IDX,                 /*UI_DISPLAY_PM2P5*/
  ICON24_NONE,                      /*UI_DISPLAY_WEATHER*/
  ICON24_NONE,                      /*UI_DISPLAY_MESSAGE*/
  ICON24_NONE,                      /*UI_DISPLAY_APP_NOTIF*/
  ICON24_NONE,                      /*UI_DISPLAY_DETAIL_NOTIF*/	
  ICON24_NONE,                      /*UI_DISPLAY_INCOMING_CALL*/
  ICON24_NONE,                      /*UI_DISPLAY_INCOMING_MESSAGE*/
  ICON24_NONE,                      /*UI_DISPLAY_ALARM_CLOCK_REMINDER*/
  ICON24_NORMAL_ALARM_CLOCK_IDX,    /*UI_DISPLAY_ALARM_CLOCK_DETAIL*/
  ICON24_NONE,                      /*UI_DISPLAY_IDLE_ALERT*/
	ICON24_HEART_RATE_IDX,            /*UI_DISPLAY_HEART_RATE_ALERT*/
	ICON24_STEPS_IDX,                 /*UI_DISPLAY_STEP_10K_ALERT*/
	ICON24_HEART_RATE_IDX,            /*UI_DISPLAY_HEART_RATE*/
  ICON24_NONE,                      /*UI_DISPLAY_SETTING*/
  ICON24_STOPWATCH_IDX,             /*UI_DISPLAY_STOPWATCH_START*/
  ICON24_STOPWATCH_IDX,             /*UI_DISPLAY_STOPWATCH_STOP*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_TREADMILL*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_CYCLING*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_STAIRS*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_ELLIPTICAL*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_ROW*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_AEROBIC*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_PILOXING*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_OTHERS*/
  ICON24_NONE,                      /*UI_DISPLAY_WORKOUT_RT_READY*/
  ICON24_RUNNING_TIME_IDX,          /*UI_DISPLAY_WORKOUT_RT_TIME*/
  ICON24_RUNNING_HR_IDX,            /*UI_DISPLAY_WORKOUT_RT_HEART_RATE*/
  ICON24_RUNNING_CALORIES_IDX,      /*UI_DISPLAY_WORKOUT_RT_CALORIES*/
  ICON24_RUNNING_STOP_IDX,          /*UI_DISPLAY_WORKOUT_RT_END*/
  ICON24_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_RUNNING_DISTANCE*/
	ICON24_RUNNING_TIME_IDX,          /*UI_DISPLAY_RUNNING_TIME*/
	ICON24_RUNNING_PACE_IDX,          /*UI_DISPLAY_RUNNING_PACE*/
	ICON24_RUNNING_STRIDE_IDX,        /*UI_DISPLAY_RUNNING_STRIDE*/	
	ICON24_RUNNING_CADENCE_IDX,       /*UI_DISPLAY_RUNNING_CADENCE*/	
	ICON24_RUNNING_HR_IDX,            /*UI_DISPLAY_RUNNING_HEART_RATE*/
	ICON24_RUNNING_CALORIES_IDX,      /*UI_DISPLAY_RUNNING_CALORIES*/
	ICON24_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_RUN_START*/
	ICON24_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_RUN_OR_ANALYSIS*/
	ICON24_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_READY*/
	ICON24_RUNNING_TIME_IDX,          /*UI_DISPLAY_TRAINING_TIME*/
	ICON24_RUNNING_DISTANCE_IDX,      /*UI_DISPLAY_TRAINING_DISTANCE*/
	ICON24_RUNNING_PACE_IDX,          /*UI_DISPLAY_TRAINING_PACE*/
	ICON24_RUNNING_HR_IDX,            /*UI_DISPLAY_TRAINING_HEART_RATE*/
	ICON24_RUNNING_STOP_IDX,          /*UI_DISPLAY_TRAINING_RUN_STOP*/
	ICON24_CYCLING_OUTDOOR_MODE_IDX,  /*UI_DISPLAY_CYCLING_OUTDOOR_RUN_START*/
	ICON24_CYCLING_OUTDOOR_MODE_IDX,  /*UI_DISPLAY_CYCLING_OUTDOOR_READY*/
	ICON24_RUNNING_TIME_IDX,          /*UI_DISPLAY_CYCLING_OUTDOOR_TIME*/
	ICON24_CYCLING_OUTDOOR_MODE_IDX,  /*UI_DISPLAY_CYCLING_OUTDOOR_DISTANCE*/
	ICON24_CYCLING_OUTDOOR_SPEED_IDX, /*UI_DISPLAY_CYCLING_OUTDOOR_SPEED*/
  ICON24_RUNNING_HR_IDX,            /*UI_DISPLAY_CYCLING_OUTDOOR_HEART_RATE*/
	ICON24_RUNNING_STOP_IDX,          /*UI_DISPLAY_CYCLING_OUTDOOR_RUN_STOP*/
  ICON24_NONE,                      /*UI_DISPLAY_MUSIC_PLAY_PAUSE*/
  ICON24_NONE,                      /*UI_DISPLAY_MUSIC_TRACK*/
  ICON24_NONE,                      /*UI_DISPLAY_MUSIC_VOLUME*/
  ICON24_NONE,                      /*UI_DISPLAY_PAY_BUS_CARD_BALANCE_ENQUIRY*/	
  ICON24_NONE,                      /*UI_DISPLAY_PAY_BANK_CARD_BALANCE_ENQUIRY*/		
  ICON24_NONE,                      /*UI_DISPLAY_CAROUSEL_1*/
  ICON24_NONE,                      /*UI_DISPLAY_CAROUSEL_2*/
  ICON24_NONE,                      /*UI_DISPLAY_CAROUSEL_3*/
  ICON24_NONE,                      /*UI_DISPLAY_CAROUSEL_4*/
  ICON24_NONE,                      /*UI_DISPLAY_PREVIOUS*/	
};
#endif

/***********************************************************************************************************************************************************************/
/********************************************************** frame display **********************************************************************************************/
/***********************************************************************************************************************************************************************/
const UI_RENDER_CTX horizontal_ui_render[] = {
  {_left_render_horizontal_batt_ble,              _middle_render_horizontal_clock,                     _right_render_horizontal_home},                    /*UI_DISPLAY_HOME_CLOCK*/
  {_RENDER_NONE,                                  _middle_render_horizontal_system_restart,            _RENDER_NONE},	                                    /*UI_DISPLAY_PACE_LOGO*/
  {_RENDER_NONE,                                  _middle_render_horizontal_ota,                       _RENDER_NONE},                                     /*UI_DISPLAY_OTA*/	
  {_left_render_horizontal_batt_ble,              _middle_render_horizontal_linking,                   _RENDER_NONE},                                     /*UI_DISPLAY_LINKING*/	
  {_left_render_horizontal_batt_ble,              _middle_render_horizontal_ble_code,                  _right_render_horizontal_firmware_ver},            /*UI_DISPLAY_UNAUTHORIZED*/
  {_RENDER_NONE,                                  _middle_render_horizontal_system_charging,           _RENDER_NONE},                                     /*UI_DISPLAY_BATT_POWER*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_steps,                     _right_render_horizontal_tracker},                 /*UI_DISPLAY_STEPS*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_distance,                  _right_render_horizontal_tracker},                 /*UI_DISPLAY_DISTANCE*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_calories,                  _right_render_horizontal_tracker},                 /*UI_DISPLAY_CALORIES*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_active_time,               _right_render_horizontal_tracker},                 /*UI_DISPLAY_ACTIVE_TIME*/
  {_left_render_horizontal_pm2p5,                 _middle_render_horizontal_pm2p5,                     _RENDER_NONE},                                     /*UI_DISPLAY_PM2P5*/
  {_left_render_horizontal_weather,               _middle_render_horizontal_weather,                   _RENDER_NONE},                                     /*UI_DISPLAY_WEATHER*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_message,                   _right_render_horizontal_more},                    /*UI_DISPLAY_MESSAGE*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_app_notif,                 _right_render_horizontal_app_notif},               /*UI_DISPLAY_APP_NOTIF*/
  {_RENDER_NONE,                                  _middle_render_horizontal_detail_notif,              _right_render_horizontal_more},                    /*UI_DISPLAY_DETAIL_NOTIF*/	
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_incoming_call_or_message,  _right_render_horizontal_ok_top},                  /*UI_DISPLAY_INCOMING_CALL*/
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_incoming_call_or_message,  _right_render_horizontal_ok_top},                  /*UI_DISPLAY_INCOMING_MESSAGE*/
  {_left_render_horizontal_alarm_clock_reminder,  _middle_render_horizontal_alarm_clock_reminder,      _right_render_horizontal_more},                    /*UI_DISPLAY_ALARM_CLOCK_REMINDER*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_alarm_clcok_detail,        _right_render_horizontal_reminder},                /*UI_DISPLAY_ALARM_CLOCK_DETAIL*/
  {_left_render_horizontal_idle_alert,            _middle_render_horizontal_idle_alert,                _RENDER_NONE},                                     /*UI_DISPLAY_IDLE_ALERT*/
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_heart_rate,                _right_render_horizontal_small_clock},             /*UI_DISPLAY_HEART_RATE_ALERT*/
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_steps,                     _right_render_horizontal_small_clock},             /*UI_DISPLAY_STEP_10K_ALERT*/
  {_RENDER_NONE,                                  _middle_render_horizontal_sos_alert,                 _RENDER_NONE},                                     /*UI_DISPLAY_SMART_SOS_ALERT*/	
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_heart_rate,                _right_render_horizontal_small_clock},             /*UI_DISPLAY_HEART_RATE*/
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_skin_temp,                 _right_render_horizontal_small_clock},             /*UI_DISPLAY_SKIN_TEMP*/
  {_RENDER_NONE,                                  _middle_render_horizontal_system_restart,            _RENDER_NONE},                                     /*UI_DISPLAY_SETTING*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_stopwatch_start,           _right_render_horizontal_stopwatch_start},         /*UI_DISPLAY_STOPWATCH_START*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_stopwatch_stop,            _right_render_horizontal_ok_middle},               /*UI_DISPLAY_STOPWATCH_STOP*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok_top},                  /*UI_DISPLAY_WORKOUT_TREADMILL*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok_top},                  /*UI_DISPLAY_WORKOUT_CYCLING*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok_top},                  /*UI_DISPLAY_WORKOUT_STAIRS*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok_top},                  /*UI_DISPLAY_WORKOUT_ELLIPTICAL*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok_top},                  /*UI_DISPLAY_WORKOUT_ROW*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok_top},                  /*UI_DISPLAY_WORKOUT_AEROBIC*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok_top},                  /*UI_DISPLAY_WORKOUT_PILOXING*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok_top},                  /*UI_DISPLAY_WORKOUT_OTHERS*/
  {_RENDER_NONE,                                  _middle_render_horizontal_workout_ready,             _RENDER_NONE},                                     /*UI_DISPLAY_WORKOUT_RT_READY*/
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_training_time,             _RENDER_NONE},                                     /*UI_DISPLAY_WORKOUT_RT_TIME*/
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_training_hr,               _right_render_horizontal_training_hr},             /*UI_DISPLAY_WORKOUT_RT_HEART_RATE*/
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_training_calories,         _right_render_horizontal_running_calories},        /*UI_DISPLAY_WORKOUT_RT_CALORIES*/
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_training_workout_stop,     _right_render_horizontal_ok_middle},               /*UI_DISPLAY_WORKOUT_RT_END*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_running_distance,          _right_render_horizontal_running_distance},        /*UI_DISPLAY_RUNNING_DISTANCE*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_running_time,              _RENDER_NONE},                                     /*UI_DISPLAY_RUNNING_TIME*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_running_pace,              _right_render_horizontal_running_pace},            /*UI_DISPLAY_RUNNING_PACE*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_running_stride,            _right_render_horizontal_running_stride},          /*UI_DISPLAY_RUNNING_STRIDE*/	
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_running_cadence,           _right_render_horizontal_running_cadence},         /*UI_DISPLAY_RUNNING_CADENCE*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_running_hr,                _right_render_horizontal_running_hr},              /*UI_DISPLAY_RUNNING_HEART_RATE*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_running_calories,          _right_render_horizontal_running_calories},        /*UI_DISPLAY_RUNNING_CALORIES*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_training_start_run,        _right_render_horizontal_ok_middle},               /*UI_DISPLAY_TRAINING_RUN_START*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_training_run_or_analysis,  _RENDER_NONE},                                     /*UI_DISPLAY_TRAINING_RUN_OR_ANALYSIS*/
  {_left_render_horizontal_training_ready,        _middle_render_horizontal_training_ready,            _RENDER_NONE},                                     /*UI_DISPLAY_TRAINING_READY*/ 
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_training_time,             _RENDER_NONE},                                     /*UI_DISPLAY_TRAINING_TIME*/
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_training_distance,         _right_render_horizontal_running_distance},        /*UI_DISPLAY_TRAINING_DISTANCE*/
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_training_pace,             _right_render_horizontal_training_pace},           /*UI_DISPLAY_TRAINING_PACE*/
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_training_hr,               _right_render_horizontal_training_hr},             /*UI_DISPLAY_TRAINING_HEART_RATE*/
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_training_run_stop,         _right_render_horizontal_ok_middle},               /*UI_DISPLAY_TRAINING_RUN_STOP*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_cycling_outdoor_start,     _right_render_horizontal_ok_middle},               /*UI_DISPLAY_CYCLING_OUTDOOR_RUN_START*/
  {_left_render_horizontal_cycling_outdoor_ready, _middle_render_horizontal_cycling_outdoor_ready,     _RENDER_NONE},                                     /*UI_DISPLAY_CYCLING_OUTDOOR_READY*/
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_training_time,             _RENDER_NONE},                                     /*UI_DISPLAY_CYCLING_OUTDOOR_TIME*/
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_cycling_outdoor_distance,  _right_render_horizontal_cycling_outdoor_distance},/*UI_DISPLAY_CYCLING_OUTDOOR_DISTANCE*/
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_cycling_outdoor_speed,     _right_render_horizontal_cycling_outdoor_speed},   /*UI_DISPLAY_CYCLING_OUTDOOR_SPEED*/
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_training_hr,               _right_render_horizontal_training_hr},             /*UI_DISPLAY_CYCLING_OUTDOOR_HEART_RATE*/
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_cycling_outdoor_stop,      _right_render_horizontal_ok_middle},               /*UI_DISPLAY_CYCLING_OUTDOOR_STATATISTICS_END*/
  {_RENDER_NONE,                                  _middle_render_horizontal_carousel_1,                _RENDER_NONE},                                     /*UI_DISPLAY_CAROUSEL_1*/
  {_RENDER_NONE,                                  _middle_render_horizontal_carousel_2,                _RENDER_NONE},                                     /*UI_DISPLAY_CAROUSEL_2*/
  {_RENDER_NONE,                                  _middle_render_horizontal_carousel_3,                _RENDER_NONE},                                     /*UI_DISPLAY_CAROUSEL_3*/
};

const UI_RENDER_CTX vertical_ui_render[] = {
  {_top_render_vertical_batt_ble,                 _middle_render_vertical_clock,                       _bottom_render_vertical_home},                     /*UI_DISPLAY_HOME_CLOCK*/
  {_RENDER_NONE,                                  _middle_render_horizontal_system_restart,            _RENDER_NONE},	                                    /*UI_DISPLAY_PACE_LOGO*/
  {_RENDER_NONE,                                  _middle_render_horizontal_ota,                       _RENDER_NONE},                                     /*UI_DISPLAY_OTA*/	
  {_left_render_horizontal_batt_ble,              _middle_render_horizontal_linking,                   _RENDER_NONE},                                     /*UI_DISPLAY_LINKING*/	
  {_left_render_horizontal_batt_ble,              _middle_render_horizontal_ble_code,                  _right_render_horizontal_firmware_ver},            /*UI_DISPLAY_UNAUTHORIZED*/
  {_RENDER_NONE,                                  _middle_render_horizontal_system_charging,           _RENDER_NONE},                                     /*UI_DISPLAY_BATT_POWER*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_steps,                       _bottom_render_vertical_tracker},                  /*UI_DISPLAY_STEPS*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_distance,                    _bottom_render_vertical_tracker},                  /*UI_DISPLAY_DISTANCE*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_calories,                    _bottom_render_vertical_tracker},                  /*UI_DISPLAY_CALORIES*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_active_time,                 _bottom_render_vertical_tracker},                  /*UI_DISPLAY_ACTIVE_TIME*/
  {_left_render_horizontal_pm2p5,                 _middle_render_horizontal_pm2p5,                     _RENDER_NONE},                                     /*UI_DISPLAY_PM2P5*/
  {_left_render_horizontal_weather,               _middle_render_horizontal_weather,                   _RENDER_NONE},                                     /*UI_DISPLAY_WEATHER*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_message,                   _right_render_horizontal_more},                    /*UI_DISPLAY_MESSAGE*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_app_notif,                 _right_render_horizontal_app_notif},               /*UI_DISPLAY_APP_NOTIF*/
  {_RENDER_NONE,                                  _middle_render_horizontal_detail_notif,              _right_render_horizontal_more},                    /*UI_DISPLAY_DETAIL_NOTIF*/	
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_incoming_call_or_message,  _right_render_horizontal_ok_top},                  /*UI_DISPLAY_INCOMING_CALL*/
  {_left_render_horizontal_16_icon_blinking,      _middle_render_horizontal_incoming_call_or_message,  _right_render_horizontal_ok_top},                  /*UI_DISPLAY_INCOMING_MESSAGE*/
  {_left_render_horizontal_alarm_clock_reminder,  _middle_render_horizontal_alarm_clock_reminder,      _bottom_render_vertical_more},                     /*UI_DISPLAY_ALARM_CLOCK_REMINDER*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_alarm_clcok_detail,        _right_render_horizontal_reminder},                /*UI_DISPLAY_ALARM_CLOCK_DETAIL*/
  {_left_render_horizontal_idle_alert,            _middle_render_horizontal_idle_alert,                _right_render_horizontal_small_clock},             /*UI_DISPLAY_IDLE_ALERT*/
  {_top_render_vertical_24_icon_blinking,         _middle_render_vertical_heart_rate,                  _bottom_render_vertical_small_clock},              /*UI_DISPLAY_HEART_RATE_ALERT*/
  {_top_render_vertical_24_icon_blinking,         _middle_render_vertical_steps,                       _bottom_render_vertical_small_clock},              /*UI_DISPLAY_STEP_10K_ALERT*/
  {_RENDER_NONE,                                  _middle_render_horizontal_sos_alert,                 _RENDER_NONE},                                     /*UI_DISPLAY_SMART_SOS_ALERT*/	
  {_top_render_vertical_24_icon_blinking,         _middle_render_vertical_heart_rate,                  _bottom_render_vertical_small_clock},              /*UI_DISPLAY_HEART_RATE*/
  {_top_render_vertical_24_icon_blinking,         _middle_render_vertical_skin_temp,                   _bottom_render_vertical_small_clock},              /*UI_DISPLAY_SKIN_TEMP*/
  {_RENDER_NONE,                                  _middle_render_horizontal_system_restart,            _RENDER_NONE},                                     /*UI_DISPLAY_SETTING*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_stopwatch_start,           _right_render_horizontal_stopwatch_start},         /*UI_DISPLAY_STOPWATCH_START*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_stopwatch_stop,            _right_render_horizontal_ok_middle},               /*UI_DISPLAY_STOPWATCH_STOP*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok_top},                  /*UI_DISPLAY_WORKOUT_TREADMILL*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok_top},                  /*UI_DISPLAY_WORKOUT_CYCLING*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok_top},                  /*UI_DISPLAY_WORKOUT_STAIRS*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok_top},                  /*UI_DISPLAY_WORKOUT_ELLIPTICAL*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok_top},                  /*UI_DISPLAY_WORKOUT_ROW*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok_top},                  /*UI_DISPLAY_WORKOUT_AEROBIC*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok_top},                  /*UI_DISPLAY_WORKOUT_PILOXING*/
  {_left_render_horizontal_16_icon,               _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok_top},                  /*UI_DISPLAY_WORKOUT_OTHERS*/
  {_RENDER_NONE,                                  _middle_render_horizontal_workout_ready,             _RENDER_NONE},                                     /*UI_DISPLAY_WORKOUT_RT_READY*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_training_time,               _RENDER_NONE},                                     /*UI_DISPLAY_WORKOUT_RTTIME*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_training_hr,                 _RENDER_NONE},                                     /*UI_DISPLAY_WORKOUT_RT_HEART_RATE*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_training_calories,           _RENDER_NONE},                                     /*UI_DISPLAY_WORKOUT_RT_CALORIES*/
  {_top_render_vertical_24_icon_blinking,         _middle_render_vertical_training_workout_stop,       _bottom_render_vertical_ok},                       /*UI_DISPLAY_WORKOUT_RT_END*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_running_distance,            _bottom_render_vertical_run_distance_uint},        /*UI_DISPLAY_RUNNING_DISTANCE*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_running_time,                _RENDER_NONE},                                     /*UI_DISPLAY_RUNNING_TIME*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_running_pace,                _RENDER_NONE},                                     /*UI_DISPLAY_RUNNING_PACE*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_running_stride,              _RENDER_NONE},                                     /*UI_DISPLAY_RUNNING_STRIDE*/	
  {_top_render_vertical_24_icon,                  _middle_render_vertical_running_cadence,             _RENDER_NONE},                                     /*UI_DISPLAY_RUNNING_CADENCE*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_running_hr,                  _RENDER_NONE},                                     /*UI_DISPLAY_RUNNING_HEART_RATE*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_running_calories,            _RENDER_NONE},                                     /*UI_DISPLAY_RUNNING_CALORIES*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_training_run_start,          _bottom_render_vertical_ok},                       /*UI_DISPLAY_TRAINING_RUN_START*/ 
  {_top_render_vertical_24_icon,                  _middle_render_vertical_training_run_or_analysis,    _RENDER_NONE},                                     /*UI_DISPLAY_TRAINING_RUN_OR_ANALYSIS*/ 
  {_top_render_vertical_24_icon,                  _middle_render_vertical_training_ready,              _RENDER_NONE},                                     /*UI_DISPLAY_TRAINING_READY*/ 
  {_top_render_vertical_24_icon,                  _middle_render_vertical_training_time,               _RENDER_NONE},                                     /*UI_DISPLAY_TRAINING_TIME*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_training_distance,           _bottom_render_vertical_run_distance_uint},        /*UI_DISPLAY_TRAINING_DISTANCE*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_training_pace,               _RENDER_NONE},                                     /*UI_DISPLAY_TRAINING_PACE*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_training_hr,                 _RENDER_NONE},                                     /*UI_DISPLAY_TRAINING_HEART_RATE*/
  {_top_render_vertical_24_icon_blinking,         _middle_render_vertical_training_run_stop,           _bottom_render_vertical_ok},                       /*UI_DISPLAY_TRAINING_RUN_STOP*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_cycling_outdoor_start,       _bottom_render_vertical_ok},                       /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_START*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_cycling_outdoor_ready,       _RENDER_NONE},                                     /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_READY*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_training_time,               _RENDER_NONE},                                     /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_TIME*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_cycling_outdoor_distance,    _bottom_render_vertical_cycling_outdoor_distance}, /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_DISTANCE*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_cycling_outdoor_speed,       _bottom_render_vertical_cycling_outdoor_speed},    /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_SPEED*/
  {_top_render_vertical_24_icon,                  _middle_render_vertical_training_hr,                 _RENDER_NONE},                                     /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_HEART_RATE*/
  {_top_render_vertical_24_icon_blinking,         _middle_render_vertical_cycling_outdoor_stop,        _bottom_render_vertical_ok},                       /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_STOP*/
  {_RENDER_NONE,                                  _middle_render_vertical_carousel_1,                  _RENDER_NONE},                                     /*UI_DISPLAY_CAROUSEL_1*/
  {_RENDER_NONE,                                  _middle_render_vertical_carousel_2,                  _RENDER_NONE},                                     /*UI_DISPLAY_CAROUSEL_2*/
  {_RENDER_NONE,                                  _middle_render_vertical_carousel_3,                  _RENDER_NONE},                                     /*UI_DISPLAY_CAROUSEL_3*/	
};

static void _core_frame_display(I8U frame_index, BOOLEAN b_render)
{	
	const UI_RENDER_CTX *ui_render;

	if (cling.ui.language_type >= LANGUAGE_TYPE_TRADITIONAL_CHINESE)	
	  cling.ui.language_type = LANGUAGE_TYPE_TRADITIONAL_CHINESE;
	
	if (cling.user_data.profile.metric_distance)
		cling.user_data.profile.metric_distance = 1;
	
	if (cling.ui.icon_sec_blinking) {
		cling.ui.icon_sec_blinking = FALSE;
	} else {
		cling.ui.icon_sec_blinking = TRUE;
	}
	
	// Set correct screen orientation
	if (cling.ui.clock_orientation == 1) {
		ui_render = horizontal_ui_render;  		
	} else {
		ui_render = vertical_ui_render;
	}

	// Clean up all screen.
	memset(cling.ui.p_oled_up, 0, 512);
	
	// Get top render icon index
	cling.ui.frm_render.horizontal_icon_16_idx = ui_matrix_horizontal_icon_16_idx[frame_index];		
	cling.ui.frm_render.vertical_icon_24_idx = ui_matrix_vertical_icon_24_idx[frame_index];		

	ui_render[frame_index].middle_row_render();
	ui_render[frame_index].bottom_row_render();	
	ui_render[frame_index].top_row_render();	

	if (b_render) {
		// Finally, we render the frame
		UI_render_screen();
	}
}
