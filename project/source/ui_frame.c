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
	OLED_full_scree_show(cling.ui.p_oled_up);

	OLED_set_display(1);
}

/***************************************************************************/
/*********************** Horizontal display page ***************************/
/***************************************************************************/
static void _render_one_icon_8(I8U len, I8U *p_out, const I8U *p_in)
{
	I8U j;
	I8U *p_out_0 = p_out;
	
	// Render the left side
	for (j = 0; j < len; j++) {
			*p_out_0++ = (*p_in++);
	}
}

static void _render_one_icon_16(I8U len, I8U *p_out, const I8U *p_in)
{
	I8U j;
	I8U *p_out_0 = p_out;
	I8U *p_out_1 = p_out_0+128;
	
	// Render the left side
	for (j = 0; j < len; j++) {
			*p_out_0++ = (*p_in++);
			*p_out_1++ = (*p_in++);
	}
}

static void _render_one_icon_24(I8U len, I8U *p_out, const I8U *p_in)
{
	I8U j;
	I8U *p_out_0 = p_out;
	I8U *p_out_1 = p_out_0+128;
	I8U *p_out_2 = p_out_1+128;
	
	// Render the left side
	for (j = 0; j < len; j++) {
			*p_out_0++ = (*p_in++);
			*p_out_1++ = (*p_in++);
			*p_out_2++ = (*p_in++);
	}
}

static void _render_batt_and_ble(I8U *pin)
{	
  I8U *in, *p0;	
	I8U j;
	I8U curr_batt_level = cling.system.mcu_reg[REGISTER_MCU_BATTERY];
	I8U p_v = 0x1c;
	BOOLEAN b_ble_conn = FALSE;
 	BOOLEAN b_batt_charging = FALSE;
	
	in = pin;
	
	if (BTLE_is_connected())
		b_ble_conn = TRUE;

	if (BATT_is_charging()) 
		b_batt_charging = TRUE;
				
	if (b_ble_conn) {
		p0 = in + ICON8_BATT_CHARGING_LEN + 2;
		_render_one_icon_8(ICON8_SMALL_BLE_LEN, p0, asset_content+ICON8_SMALL_BLE_POS);
	} 

	// Render the right side (offset set to 60 for steps comment)
	if (b_batt_charging) {
		p0 = in;
		_render_one_icon_8(ICON8_BATT_CHARGING_LEN, p0, asset_content+ICON8_BATT_CHARGING_POS);
	}	else {
		p0 = in;
		_render_one_icon_8(ICON8_BATT_NOCHARGING_LEN, p0, asset_content+ICON8_BATT_NOCHARGING_POS);		
	}
	
	// Filling up the percentage
	curr_batt_level /= 11;
	
	if (curr_batt_level == 0)
		return;
	
	if (curr_batt_level > 9)
		curr_batt_level = 9;
	
	// Note: the battery button icon is 9 pixels of length
	p0 = in+2;
	for (j = 0; j < curr_batt_level; j++) {
		*p0++ |= p_v;
	}
}

static void _left_render_horizontal_batt_ble() 
{
  _render_batt_and_ble(cling.ui.p_oled_up);
}

static void _left_render_horizontal_steps()
{
	I8U *in = cling.ui.p_oled_up;

	_render_one_icon_16(ICON16_STEPS_LEN, in, asset_content+ICON16_STEPS_POS);
}

static void _left_render_horizontal_active_time()
{
	I8U *in = cling.ui.p_oled_up;
	
	_render_one_icon_16(ICON16_ACTIVE_TIME_LEN, in, asset_content+ICON16_ACTIVE_TIME_POS);
}

#ifdef _CLINGBAND_UV_MODEL_
static void _left_render_horizontal_uv_index()
{
	I8U *in = cling.ui.p_oled_up;
	
	_render_one_icon_16(ICON16_UV_INDEX_LEN, in, asset_content+ICON16_UV_INDEX_POS);
}
#endif

static void _left_render_horizontal_distance()
{
	I8U *in = cling.ui.p_oled_up;

	_render_one_icon_16(ICON16_DISTANCE_LEN, in, asset_content+ICON16_DISTANCE_POS);
}

static void _left_render_horizontal_calories()
{
	I8U *in = cling.ui.p_oled_up;

	_render_one_icon_16(ICON16_CALORIES_LEN, in, asset_content+ICON16_CALORIES_POS);
}

static void _left_render_horizontal_pm2p5()
{
	I8U *in = cling.ui.p_oled_up;
	I8U string[32];
	char **air_level;
	char *air_en[] = {"Good", "Moderate", "Unhealthy", "Poor", "hazardous"};
	char *air_s_cn[] = {"优质", "良好", "差 ", "很差", "极差"};
	char *air_t_cn[] = {"優質", "良好", "差 ", "很差", "极差"};
	
	_render_one_icon_16(ICON16_PM2P5_LEN, in, asset_content+ICON16_PM2P5_POS);
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {	
		air_level = air_en;
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		air_level = air_s_cn;
	} else {
		air_level = air_t_cn;
	}

	if (cling.pm2p5 == 0xffff) {
		// AQI value Not available
		sprintf((char *)string, "--");		
	} else if (cling.pm2p5 < 50) {
		sprintf((char *)string, "%s", air_level[0]);
	} else if (cling.pm2p5 < 150) {
		sprintf((char *)string, "%s", air_level[1]);
	} else if (cling.pm2p5 < 300) {
		sprintf((char *)string, "%s", air_level[2]);
	} else if (cling.pm2p5 < 500) {
		sprintf((char *)string, "%s", air_level[3]);
	} else {
		sprintf((char *)string, "%s", air_level[4]);
	}
	
	N_SPRINTF("AQI: %d, %s", cling.pm2p5, string);
	
	FONT_load_characters(cling.ui.p_oled_up+256, (char *)string, 16, 128, FALSE);
}

static void _left_render_horizontal_weather()
{
	WEATHER_CTX weather;
	I8U *in = cling.ui.p_oled_up;

	WEATHER_get_weather(0, &weather);
	_render_one_icon_16(asset_len[256+ICON16_WEATHER_IDX+weather.type], in, asset_content+asset_pos[256+ICON16_WEATHER_IDX+weather.type]);
}

static void _left_render_horizontal_heart_rate()
{
	I8U *in = cling.ui.p_oled_up;

	if (cling.ui.heart_rate_sec_blinking) {
		cling.ui.heart_rate_sec_blinking = FALSE;
		_render_one_icon_16(ICON16_HEART_RATE_LEN, in, asset_content+ICON16_HEART_RATE_POS);
	} else {
		cling.ui.heart_rate_sec_blinking = TRUE;
	}
}

#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)
static void _left_render_horizontal_skin_temp()
{
	I8U *in = cling.ui.p_oled_up;

	_render_one_icon_16(ICON16_SKIN_TEMP_LEN, in, asset_content+ICON16_SKIN_TEMP_POS);
}
#endif

#if 0
static void _left_render_horizontal_setting()
{
	I8U *in = cling.ui.p_oled_up;

	_render_one_icon_16(ICON16_SETTING_LEN, in, asset_content+ICON16_SETTING_POS);
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_
static void _left_render_horizontal_return()
{
	I8U *in = cling.ui.p_oled_up;

	_render_one_icon_16(ICON16_RETURN_LEN, in, asset_content+ICON16_RETURN_POS);
}

static void _left_render_horizontal_stopwatch()
{
	I8U *in = cling.ui.p_oled_up;

	_render_one_icon_16(ICON16_STOPWATCH_LEN, in, asset_content+ICON16_STOPWATCH_POS);
}
#endif

static void _left_render_horizontal_reminder()
{
	I8U *in = cling.ui.p_oled_up;

	if ((cling.reminder.alarm_type == SLEEP_ALARM_CLOCK) || (cling.reminder.alarm_type == WAKEUP_ALARM_CLOCK))
		_render_one_icon_16(ICON16_SLEEP_ALARM_CLOCK_LEN, in, asset_content+ICON16_SLEEP_ALARM_CLOCK_POS);				
	else 
		_render_one_icon_16(ICON16_NORMAL_ALARM_CLOCK_LEN, in, asset_content+ICON16_NORMAL_ALARM_CLOCK_POS);
}

static void _left_render_horizontal_incoming_call()
{
	I8U *in = cling.ui.p_oled_up;

	_render_one_icon_16(ICON16_INCOMING_CALL_LEN, in, asset_content+ICON16_INCOMING_CALL_POS);
}

static void _left_render_horizontal_incoming_message()
{
	I8U *in = cling.ui.p_oled_up;

	_render_one_icon_16(ICON16_MESSAGE_LEN, in, asset_content+ICON16_MESSAGE_POS);
}

static void _left_render_horizontal_idle_alert()
{
	I8U *in = cling.ui.p_oled_up;

  _render_one_icon_24(ICON24_IDLE_ALERT_LEN, in, asset_content+ICON24_IDLE_ALERT_POS);	
}

static void _left_render_horizontal_running_distance_16()
{
	I8U *in = cling.ui.p_oled_up;
	
	_render_one_icon_16(ICON16_RUNNING_DISTANCE_LEN, in, asset_content+ICON16_RUNNING_DISTANCE_POS);
}

static void _left_render_horizontal_running_distance_24()
{
	I8U *in = cling.ui.p_oled_up;
	
	_render_one_icon_24(ICON24_RUNNING_DISTANCE_LEN, in, asset_content+ICON24_RUNNING_DISTANCE_POS);
}

static void _left_render_horizontal_running_time()
{
	I8U *in = cling.ui.p_oled_up;
	
	_render_one_icon_16(ICON16_RUNNING_TIME_LEN, in, asset_content+ICON16_RUNNING_TIME_POS);
}

static void _left_render_horizontal_running_pace()
{
	I8U *in = cling.ui.p_oled_up;
	
	_render_one_icon_16(ICON16_RUNNING_PACE_LEN, in, asset_content+ICON16_RUNNING_PACE_POS);
}

static void _left_render_horizontal_running_calories()
{
	I8U *in = cling.ui.p_oled_up;
	
	_render_one_icon_16(ICON16_RUNNING_CALORIES_LEN, in, asset_content+ICON16_RUNNING_CALORIES_POS);
}

static void _left_render_horizontal_running_hr()
{
	I8U *in = cling.ui.p_oled_up;
	
	_render_one_icon_16(ICON16_RUNNING_HR_LEN, in, asset_content+ICON16_RUNNING_HR_POS);
}

static void _left_render_horizontal_running_cadence()
{
	I8U *in = cling.ui.p_oled_up;
	
	_render_one_icon_16(ICON16_RUNNING_CADENCE_LEN, in, asset_content+ICON16_RUNNING_CADENCE_POS);
}

static void _left_render_horizontal_running_stride()
{
	I8U *in = cling.ui.p_oled_up;
	
	_render_one_icon_16(ICON16_RUNNING_STRIDE_LEN, in, asset_content+ICON16_RUNNING_STRIDE_POS);
}

static void _left_render_horizontal_training_ready()
{
	I8U *in = cling.ui.p_oled_up+20;
	
	_render_one_icon_24(ICON24_RUNNING_DISTANCE_LEN, in, asset_content+ICON24_RUNNING_DISTANCE_POS);
}

static void _left_render_horizontal_training_time()
{
	I8U *in = cling.ui.p_oled_up;
  I32U t_curr = CLK_get_system_time();	

	if (cling.ui.clock_sec_blinking) {
		cling.ui.clock_sec_blinking = FALSE;;
		_render_one_icon_16(ICON16_RUNNING_TIME_LEN, in, asset_content+ICON16_RUNNING_TIME_POS);
	} else {
		cling.ui.clock_sec_blinking = TRUE;
	}
}

static void _left_render_horizontal_training_distance()
{
	I8U *in = cling.ui.p_oled_up;
	
	if (cling.ui.clock_sec_blinking) {
		cling.ui.clock_sec_blinking = FALSE;
	  _render_one_icon_16(ICON16_RUNNING_DISTANCE_LEN, in, asset_content+ICON16_RUNNING_DISTANCE_POS);
	} else {
		cling.ui.clock_sec_blinking = TRUE;
	}
}

static void _left_render_horizontal_training_pace()
{
	I8U *in = cling.ui.p_oled_up;

	if (cling.ui.clock_sec_blinking) {
		cling.ui.clock_sec_blinking = FALSE;
	  _render_one_icon_16(ICON16_RUNNING_PACE_LEN, in, asset_content+ICON16_RUNNING_PACE_POS);
	} else {
		cling.ui.clock_sec_blinking = TRUE;
	}		
}

static void _left_render_horizontal_training_hr()
{
	I8U *in = cling.ui.p_oled_up;
	
	if (cling.ui.clock_sec_blinking) {
		cling.ui.clock_sec_blinking = FALSE;
	  _render_one_icon_16(ICON16_RUNNING_HR_LEN, in, asset_content+ICON16_RUNNING_HR_POS);
	} else {
		cling.ui.clock_sec_blinking = TRUE;
	}			
}

static void _left_render_horizontal_running_stop()
{
	I8U *in = cling.ui.p_oled_up;
	
	_render_one_icon_16(ICON16_RUNNING_STOP_LEN, in, asset_content+ICON16_RUNNING_STOP_POS);
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _left_render_horizontal_cycling_outdoor_16()
{
	I8U *in = cling.ui.p_oled_up;
	
	_render_one_icon_16(ICON16_CYCLING_OUTDOOR_MODE_LEN, in, asset_content+ICON16_CYCLING_OUTDOOR_MODE_POS);
}

static void _left_render_horizontal_cycling_outdoor_24()
{
	I8U *in = cling.ui.p_oled_up;
	
	_render_one_icon_24(ICON24_CYCLING_OUTDOOR_MODE_LEN, in, asset_content+ICON24_CYCLING_OUTDOOR_MODE_POS);
}

static void _left_render_horizontal_cycling_outdoor_ready()
{
	I8U *in = cling.ui.p_oled_up+20;
	
	_render_one_icon_24(ICON24_CYCLING_OUTDOOR_MODE_LEN, in, asset_content+ICON24_CYCLING_OUTDOOR_MODE_POS);
}

static void _left_render_horizontal_cycling_outdoor_speed()
{
	I8U *in = cling.ui.p_oled_up;
	
	_render_one_icon_16(ICON16_CYCLING_OUTDOOR_SPEED_LEN, in, asset_content+ICON16_CYCLING_OUTDOOR_SPEED_POS);
}
#endif

static I16U _middle_horizontal_alignment(I8U *p_oled_start, I16U offset, BOOLEAN b_unit_km, I8U alignment_type)
{
	I8U *p0, *p1, *p2;
	I8U *p4, *p5, *p6;
	I8U ptr;
	I16U i;

  if (alignment_type == ALIGNMENT_TYPE_CENTER) {
		p0 = p_oled_start;
		p1 = p0+128;
		p2 = p1+128;
		ptr = (128 - offset)>>1;

		// Update new offset and clean up the rest area
		offset = 128 - ptr;
		// Special process for distance
		if (b_unit_km) {
				if (ptr > 12) {
					offset -= 12;
					ptr -= 12;
			}
		}
	} else if (alignment_type == ALIGNMENT_TYPE_RIGHT) {
		p0 = cling.ui.p_oled_up+128;
	  p1 = p0+128;
	  p2 = p1+128;
	  ptr = 128 - offset;
	} else if (alignment_type == ALIGNMENT_TYPE_LEFT) {
		// ...
	}
	
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
	
	return offset;
}

static I16U _render_middle_stat_section(I8U len, I8U *string, I8U margin, I16U offset, BOOLEAN b_all_hold)
{
	I16U i, j;
	I8U *p0, *p1, *p2;
	const I8U *pin;
	I8U char_len;
		
	for (i = 0; i < len; i++) {
		p0 = cling.ui.p_oled_up+offset+128;
		p1 = p0+128;
		p2 = p1+128;
		
		if (string[i] == ' ') {
			// This is "space" character
			char_len = 4;
		} else if (b_all_hold) {
			pin = asset_content+asset_pos[512+string[i]];
			char_len = asset_len[512+string[i]];			
		} else {
			// Digits in large fonts
			if ((string[i] >= '0') && (string[i] <= '9')) {
				// Using "light" font for all other statistics except for clock
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
		
		if (i != (len-1))
			offset += char_len + margin;
		else
			offset += char_len;
	}

	return offset;
}

static I16U _render_middle_clock_section(I8U *p_oled_start, I8U len, I8U *string, I8U margin, I16U offset, I8U light_num)
{
	I16U i, j;
	I8U *p0, *p1, *p2;
	const I8U *pin;
	I8U char_len;
		
	for (i = 0; i < len; i++) {
		p0 = p_oled_start+offset;
		p1 = p0+128;
		p2 = p1+128;
	
		if (string[i] == 32) {
			// This is "space" character
			char_len = 4;
		} else {
				// Digits in large fonts
				if (i > light_num) { // Three characters: HH and a ':'
					// Using "light" font
					pin = asset_content+asset_pos[512+string[i]+152];
					char_len = asset_len[512+string[i]+152];
				} else {
					pin = asset_content+asset_pos[512+string[i]];
					char_len = asset_len[512+string[i]];
				}

				for (j = 0; j < char_len; j++) {
						*p0++ = (*pin++);
						*p1++ = (*pin++);
						*p2++ = (*pin++);
				}
		}
		if (i != (len-1))
			offset += char_len + margin;
		else
			offset += char_len;
	}

	return offset;
}

static I16U _render_middle_ble_code_section(I8U len, I8U *string, I8U margin, I16U offset)
{
	I16U i, j;
	I8U *p0, *p1, *p2;
	const I8U *pin;
	I8U char_len;
		
	for (i = 0; i < len; i++) {
		p0 = cling.ui.p_oled_up+offset+128;
		p1 = p0+128;
		p2 = p1+128;
	
		
		if (string[i] == 32) {
			// This is "space" character
			char_len = 4;
		} else {
				pin = asset_content+asset_pos[512+string[i]];
				char_len = asset_len[512+string[i]];

				for (j = 0; j < char_len; j++) {
						*p0++ = (*pin++);
						*p1++ = (*pin++);
						*p2++ = (*pin++);
				}
		}
		if (i != (len-1))
			offset += char_len + margin;
		else
			offset += char_len;
	}

	return offset;
}

static void _horizontal_display_dynamic(I8U *p_oled_in, I8U len, I8U *string)
{
	I8U *p, *pin;
	I8U *p6;
	I8U i, j, ptr, offset, char_len;

	// indicator
	p = p_oled_in;
	offset = 0;
	for (i = 0; i < len; i++) {
		pin = (I8U *)(asset_content+asset_pos[string[i]]);
		char_len = asset_len[string[i]];
		for (j = 0; j < char_len; j++) {
				*p++ = (*pin++);
		}
	
		offset += char_len;
		p += 5;
		if (i != (len -1))
			offset += 5;
	}

	// Center it in the middle
	ptr = (128 - offset)>>1;
	p = p_oled_in;
	if (ptr > 0) {
		p += 127; p6 = p - ptr;
		for (i = 0; i < 128-ptr; i++) {
			*p-- = *p6--;
		}
		for (; i < 128; i++) {
			*p-- = 0;
		}
	}
}

static void _middle_render_horizontal_system_restart()
{
	I8U string1[128];

#ifdef _CLINGBAND_PACE_MODEL_		
	I8U string2[16];
	I8U string3[16];
	I16U len;
	I16U major;
	I16U minor;

	USER_device_get_name(string1);
	FONT_load_characters(cling.ui.p_oled_up, (char *)string1, 16, 128, TRUE);
	
	memset(cling.ui.p_oled_up+256, 0, 256);
	
	major = cling.system.mcu_reg[REGISTER_MCU_REVH]>>4;
	minor = cling.system.mcu_reg[REGISTER_MCU_REVH]&0x0f;
	minor <<= 8;
	minor |= cling.system.mcu_reg[REGISTER_MCU_REVL];
	
	len = sprintf((char *)string2, "VER:%d.%d", major, minor);
	FONT_load_characters(cling.ui.p_oled_up+384+64, (char *)string2, 8, 128, FALSE);

	len = sprintf((char *)string3, "ID:");
	SYSTEM_get_ble_code(string3+len);	
	string3[len+4] = 0;	
	FONT_load_characters(cling.ui.p_oled_up+384+12, (char *)string3, 8, 128, FALSE);
#else 

	USER_device_get_name(string1);
	FONT_load_characters(cling.ui.p_oled_up+128, (char *)string1, 16, 128, TRUE);
#endif
}

#ifndef _CLINGBAND_PACE_MODEL_		
static void _middle_render_horizontal_setting()
{
	I8U string1[128];

	USER_device_get_name(string1);
	FONT_load_characters(cling.ui.p_oled_up, (char *)string1, 16, 128, TRUE);
	
	I8U string2[16];
	I8U string3[16];
	I16U len;
	I16U major;
	I16U minor;

	memset(cling.ui.p_oled_up+256, 0, 256);
	
	major = cling.system.mcu_reg[REGISTER_MCU_REVH]>>4;
	minor = cling.system.mcu_reg[REGISTER_MCU_REVH]&0x0f;
	minor <<= 8;
	minor |= cling.system.mcu_reg[REGISTER_MCU_REVL];
	
	len = sprintf((char *)string2, "VER:%d.%d", major, minor);
	FONT_load_characters(cling.ui.p_oled_up+384+64, (char *)string2, 8, 128, FALSE);

	len = sprintf((char *)string3, "ID:");
	SYSTEM_get_ble_code(string3+len);	
	string3[len+4] = 0;	
	FONT_load_characters(cling.ui.p_oled_up+384+12, (char *)string3, 8, 128, FALSE);
}
#endif

static void _middle_render_horizontal_clock()
{
	I8U string[32];
	I8U margin = 3;
	I16U offset = 20;	
	I8U len;

	if (cling.ui.clock_sec_blinking) {
		cling.ui.clock_sec_blinking = FALSE;
		len = sprintf((char *)string, "%02d:%02d", cling.time.local.hour, cling.time.local.minute);
	} else {
		cling.ui.clock_sec_blinking = TRUE;
		len = sprintf((char *)string, "%02d %02d", cling.time.local.hour, cling.time.local.minute);
	}
	
	_render_middle_clock_section(cling.ui.p_oled_up+128, len, string, margin, offset, 2);
}

static void _middle_render_horizontal_ota()
{
	I16U i;
	I8U string[32];
	I8U bar_len=0;
	I8U *p0;

	sprintf((char *)string, "%d %%", cling.ota.percent);
	bar_len = cling.ota.percent;
	FONT_load_ota_percent(cling.ui.p_oled_up+128,cling.ota.percent);

	p0 = cling.ui.p_oled_up+128+128+128+14;
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
	  _render_one_icon_16(ICON16_BATT_CHARGING_LEN, cling.ui.p_oled_up+128+offset, asset_content+ICON16_BATT_CHARGING_POS);
		offset += ICON16_BATT_CHARGING_LEN;
		offset += 5;
		_render_one_icon_16(ICON16_BATT_CHARGING_FLAG_LEN, cling.ui.p_oled_up+128+offset, asset_content+ICON16_BATT_CHARGING_FLAG_POS);
	} else {
	  _render_one_icon_16(ICON16_BATT_CHARGING_LEN, cling.ui.p_oled_up+128+offset, asset_content+ICON16_BATT_CHARGING_POS);
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
	FONT_load_characters(cling.ui.p_oled_up+128+90, (char *)string, 16, 128, FALSE);
}

static void _middle_render_horizontal_ble_code()
{
	I8U string[32];
	I8U len = 4;
	I16U offset = 0;
	I8U margin = 3;

	SYSTEM_get_ble_code(string);
	offset = _render_middle_ble_code_section(len, string, margin, offset);
	_middle_horizontal_alignment(cling.ui.p_oled_up+128, offset, FALSE, ALIGNMENT_TYPE_CENTER);
}

static void _middle_render_horizontal_linking()
{
	I16U offset = 0;
	
  cling.ui.linking_wave_index++;
	if (cling.ui.linking_wave_index > 3)
		 cling.ui.linking_wave_index=0;

	if (cling.ui.linking_wave_index == 3) {
	  _render_one_icon_16(ICON16_AUTH_PROGRESS_0_LEN, cling.ui.p_oled_up+128+offset, asset_content+ICON16_AUTH_PROGRESS_0_POS);
	}
	offset += ICON16_AUTH_PROGRESS_0_LEN;
	offset += 5;
	
	if (cling.ui.linking_wave_index >= 2) {	
	  _render_one_icon_16(ICON16_AUTH_PROGRESS_1_LEN, cling.ui.p_oled_up+128+offset, asset_content+ICON16_AUTH_PROGRESS_1_POS);
	}
	offset += ICON16_AUTH_PROGRESS_1_LEN;
	offset += 5;
	
	_render_one_icon_16(ICON16_AUTH_PROGRESS_2_LEN, cling.ui.p_oled_up+128+offset, asset_content+ICON16_AUTH_PROGRESS_2_POS);
	offset += ICON16_AUTH_PROGRESS_2_LEN;
	offset += 5;

	if (cling.ui.linking_wave_index >= 2) {
	  _render_one_icon_16(ICON16_AUTH_PROGRESS_3_LEN, cling.ui.p_oled_up+128+offset, asset_content+ICON16_AUTH_PROGRESS_3_POS);
	}
	offset += ICON16_AUTH_PROGRESS_3_LEN;
	offset += 5;

	if (cling.ui.linking_wave_index == 3) {
	  _render_one_icon_16(ICON16_AUTH_PROGRESS_4_LEN, cling.ui.p_oled_up+128+offset, asset_content+ICON16_AUTH_PROGRESS_4_POS);
	}
	offset += ICON16_AUTH_PROGRESS_4_LEN;

	_middle_horizontal_alignment(cling.ui.p_oled_up+128, offset, FALSE, ALIGNMENT_TYPE_CENTER);
}

static void _middle_render_horizontal_steps()
{
	I8U string[32];
	I8U len = 0;
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
		len = sprintf((char *)string, "%d,%03d", integer, fractional);
	} else {
		len = sprintf((char *)string, "%d", stat);
	}

	offset = _render_middle_stat_section(len, string, margin, offset, FALSE);
	// Shift all the display to the middle
	_middle_horizontal_alignment(cling.ui.p_oled_up+128, offset, FALSE, ALIGNMENT_TYPE_CENTER);
}

static void _middle_render_horizontal_distance()
{
	I8U string[32];
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 3;
	I32U stat;
	I16U integer, fractional;

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

	offset = _render_middle_stat_section(len, string, margin, offset, FALSE);
	// Shift all the display to the middle
	offset = 	_middle_horizontal_alignment(cling.ui.p_oled_up+128, offset, TRUE, ALIGNMENT_TYPE_CENTER);
	
	if (cling.user_data.profile.metric_distance) {
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {	
			sprintf((char *)string, "ML");
			FONT_load_characters(cling.ui.p_oled_up+256+offset+4, (char *)string, 16, 128, FALSE);
		}	else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {	
			sprintf((char *)string, "英里");
			FONT_load_characters(cling.ui.p_oled_up+256+offset+4, (char *)string, 16, 128, FALSE);
		} else {
			sprintf((char *)string, "英裏");
			FONT_load_characters(cling.ui.p_oled_up+256+offset+4, (char *)string, 16, 128, FALSE);
		}
	} else {
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {	
			sprintf((char *)string, "KM");
			FONT_load_characters(cling.ui.p_oled_up+256+offset+4, (char *)string, 16, 128, FALSE);
		}	else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {	
			sprintf((char *)string, "公里");
			FONT_load_characters(cling.ui.p_oled_up+256+offset+4, (char *)string, 16, 128, FALSE);
		} else {
			sprintf((char *)string, "公裏");
			FONT_load_characters(cling.ui.p_oled_up+256+offset+4, (char *)string, 16, 128, FALSE);
		}
	}
}

static void _middle_render_horizontal_calories()
{
	I8U string[32];
	I8U len = 0;
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
		len = sprintf((char *)string, "%d,%03d", integer, fractional);
	} else {
		len = sprintf((char *)string, "%d", stat);
	}
	
	offset = _render_middle_stat_section(len, string, margin, offset, FALSE);
	// Shift all the display to the middle
	_middle_horizontal_alignment(cling.ui.p_oled_up+128, offset, FALSE, ALIGNMENT_TYPE_CENTER);
}

static void _middle_render_horizontal_active_time()
{
	I8U string[32];
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 3;
	I32U stat;

#ifdef _CLINGBAND_PACE_MODEL_		
	TRACKING_get_activity(0, TRACKING_ACTIVE_TIME, &stat);
#else
	TRACKING_get_activity(cling.ui.vertical_index, TRACKING_ACTIVE_TIME, &stat);
#endif
	
	len = sprintf((char *)string, "%d", stat);

	offset = _render_middle_stat_section(len, string, margin, offset, FALSE);
	// Shift all the display to the middle
	offset = 	_middle_horizontal_alignment(cling.ui.p_oled_up+128, offset, TRUE, ALIGNMENT_TYPE_CENTER);
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {	
		sprintf((char *)string, "MIN");
		FONT_load_characters(cling.ui.p_oled_up+256+offset+10, (char *)string, 16, 128, FALSE);
	}	else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {	
		sprintf((char *)string, "分钟");
		FONT_load_characters(cling.ui.p_oled_up+256+offset+10, (char *)string, 16, 128, FALSE);
	} else {
		sprintf((char *)string, "分鍾");
		FONT_load_characters(cling.ui.p_oled_up+256+offset+10, (char *)string, 16, 128, FALSE);
	}
}

#ifdef _CLINGBAND_UV_MODEL_		
static void _middle_render_horizontal_uv_index()
{
	I8U string[32];
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 3;
  I8U integer;

	integer = cling.uv.max_UI_uv;
	len = sprintf((char *)string, "%d.%d", (integer/10), (integer%10));

	offset = _render_middle_stat_section(len, string, margin, offset, FALSE);
	// Shift all the display to the middle
	_middle_horizontal_alignment(cling.ui.p_oled_up+128, offset, FALSE, ALIGNMENT_TYPE_CENTER);
}
#endif

static I8U _render_middle_horizontal_hr_core(BOOLEAN b_training_mode)
{
	I8U string[32];
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 3;
	I8U hr_result = 0;
	BOOLEAN b_all_hold = FALSE;
	
	if (cling.hr.b_closing_to_skin || cling.hr.b_start_detect_skin_touch) {
		if (cling.hr.heart_rate_ready) {
		  hr_result = PPG_minute_hr_calibrate();
			len = sprintf((char *)string, "%d", hr_result);			
			if (b_training_mode) {
				b_all_hold = TRUE;
			}
		} else {
			if (cling.ui.heart_rate_wave_index > 5) {
				cling.ui.heart_rate_wave_index = 0;
			}
			switch (cling.ui.heart_rate_wave_index) {
				case 0:
					len = sprintf((char *)string, "-,,,,,");
					break;
				case 1:
					len = sprintf((char *)string, ",-,,,,");
					break;
				case 2:
					len = sprintf((char *)string, ",,-,,,");
					break;
				case 3:
					len = sprintf((char *)string, ",,,-,,");
					break;
				case 4:
					len = sprintf((char *)string, ",,,,-,");
					break;
				case 5:
					len = sprintf((char *)string, ",,,,,-");
					break;
				default:
					break;
			}
			
			_horizontal_display_dynamic(cling.ui.p_oled_up+256, len, string);
			cling.ui.heart_rate_wave_index ++;
			return 0;
		}
	} else {
		N_SPRINTF("[UI] Heart rate - not valid");
		len = 0;
		string[len++] = ICON24_NO_SKIN_TOUCH_IDX;			
	}

	offset = _render_middle_stat_section(len, string, margin, offset, b_all_hold);
	// Shift all the display to the middle
	_middle_horizontal_alignment(cling.ui.p_oled_up+128, offset, FALSE, ALIGNMENT_TYPE_CENTER);		
	
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
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 3;
  I8U integer, fractional;

  integer = cling.therm.current_temperature/10;
	fractional = cling.therm.current_temperature - integer * 10;
	len = sprintf((char *)string, "%d.%d", integer, fractional);
	string[len++] = ICON24_CELCIUS_IDX;
	
	offset = _render_middle_stat_section(len, string, margin, offset, FALSE);
	// Shift all the display to the middle
	_middle_horizontal_alignment(cling.ui.p_oled_up+128, offset, FALSE, ALIGNMENT_TYPE_CENTER);
}
#endif

static void _middle_render_horizontal_weather()
{
	I8U string[32];
	I8U len = 0;
	I16U offset = 10;
	I8U margin = 5;
	WEATHER_CTX weather;

	if (WEATHER_get_weather(0, &weather)) {
		len = 0;
		len += sprintf((char *)string+len, "%d", weather.low_temperature);
		string[len++] = ICON24_WEATHER_RANGE_IDX;
		len += sprintf((char *)string+len, "%d", weather.high_temperature);
		string[len++] = ICON24_CELCIUS_IDX;
	} else {
		len = 0;
		len += sprintf((char *)string+len, "15");
		string[len++] = ICON24_WEATHER_RANGE_IDX;
		len += sprintf((char *)string+len, "22");
		string[len++] = ICON24_CELCIUS_IDX;
	}
	
	offset = _render_middle_stat_section(len, string, margin, offset, FALSE);

	// Shift all the display to the middle
	_middle_horizontal_alignment(cling.ui.p_oled_up+128, offset, FALSE, ALIGNMENT_TYPE_CENTER);
}

#ifndef _CLINGBAND_PACE_MODEL_		
static void _middle_render_horizontal_message()
{
	I8U len;
	I8U string[32];
	I8U offset=0;
	I8U margin=3;
	
#ifdef _ENABLE_ANCS_				
	len = sprintf((char *)string, "%d", NOTIFIC_get_message_total());
#else	
	len = sprintf((char *)string, "0");
#endif	
	
	offset = _render_middle_stat_section(len, string, margin, offset, FALSE);
	// Shift all the display to the middle
	_middle_horizontal_alignment(cling.ui.p_oled_up+128, offset, FALSE, ALIGNMENT_TYPE_CENTER);
}

static void _middle_render_horizontal_app_notif()
{
	I8U len;
	I8U dis_len;
	I8U string[32];
	I8U offset=0;

	len = NOTIFIC_get_app_name(cling.ui.app_notific_index, (char *)string);
	N_SPRINTF("[UI] app index: %d, %d, %s", cling.ui.app_notific_index, len, (char *)string);
	dis_len = FONT_get_string_display_len((char *)string);
	if (dis_len > 104) {
		FONT_load_characters(cling.ui.p_oled_up+24, (char *)string, 16, 80, FALSE);
	} else if (dis_len > 80) {
		offset = 104 - dis_len;
		FONT_load_characters(cling.ui.p_oled_up+128+128+offset, (char *)string, 16, 104, FALSE);
	} else {
		FONT_load_characters(cling.ui.p_oled_up+128+128, (char *)string, 16, 80, TRUE);			
	}

	len = sprintf((char *)string, "%02d", cling.ui.app_notific_index);
	FONT_load_characters(cling.ui.p_oled_up+(128-len*8), (char *)string, 16, 128, FALSE);			
}
#endif

static void _middle_render_horizontal_pm2p5()
{
	I8U string[32];
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 3;

	if (cling.pm2p5 == 0xffff) {
		len = sprintf((char *)string, "N!A");
	} else {
		len = sprintf((char *)string, "%d", cling.pm2p5);
	}
		
	offset = 80;
	
	offset = _render_middle_stat_section(len, string, margin, offset, FALSE);
	_middle_horizontal_alignment(cling.ui.p_oled_up+128, offset, FALSE, ALIGNMENT_TYPE_CENTER);
}

static void _middle_render_horizontal_reminder()
{
	I8U string[32];
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 3;
  BOOLEAN b_invalid_alarm = FALSE;
	
	if ((cling.reminder.ui_alarm_on) || (cling.reminder.total) || (cling.reminder.b_sleep_total)){
		if (cling.ui.ui_alarm_hh >= 24 || cling.ui.ui_alarm_mm >= 60) {
			b_invalid_alarm = TRUE;
		} else {
			len = sprintf((char *)string, "%02d:%02d", cling.ui.ui_alarm_hh, cling.ui.ui_alarm_mm);				
		}
	} else {
		len = 0;
		string[len++] = ICON24_NO_SKIN_TOUCH_IDX;		
	}		

	if (b_invalid_alarm) {
		len = sprintf((char *)string, "--:--");
		return;
	} 
		
	offset = _render_middle_clock_section(cling.ui.p_oled_up+128, len, string, margin, offset, 2);
	// Shift all the display to the middle
	_middle_horizontal_alignment(cling.ui.p_oled_up+128, offset, FALSE, ALIGNMENT_TYPE_CENTER);
}

static void _middle_render_horizontal_incoming_call()
{
	I8U string[128];
	I16U dis_len=0;
	
	NOTIFIC_get_callerID((char *)string);
	dis_len = FONT_get_string_display_len((char *)string);
	
	if (dis_len > 88) {
		FONT_load_characters(cling.ui.p_oled_up+24, (char *)string, 16, 88, FALSE);
	} else {
		FONT_load_characters(cling.ui.p_oled_up+128+128, (char *)string, 16, 88, TRUE);			
	}
}

static void _middle_render_horizontal_incoming_message()
{
	I8U string[128];
	I16U dis_len=0;
	
	cling.ui.app_notific_index = 0;
	NOTIFIC_get_app_name(cling.ui.app_notific_index,(char *)string);	
	
	dis_len = FONT_get_string_display_len((char *)string);

	if (dis_len > 88) {
		FONT_load_characters(cling.ui.p_oled_up+24, (char *)string, 16, 88, FALSE);
	} else {
		FONT_load_characters(cling.ui.p_oled_up+128+128, (char *)string, 16, 88, TRUE);			
	}
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
	FONT_load_characters(cling.ui.p_oled_up, (char *)string+string_pos, 16, 112, FALSE);		
}

static void _middle_render_horizontal_idle_alert()
{
	I8U string[32];

	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {	
	  //sprintf((char *)string1, "time for a move");
		sprintf((char *)string, "time for move");		
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {	
		sprintf((char *)string, "该活动一下了");
	} else {
		sprintf((char *)string, "該活動一下了");
	}

	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {	
	  FONT_load_characters(cling.ui.p_oled_up+256+24, (char *)string, 16, 128, FALSE);
	} else {
		FONT_load_characters(cling.ui.p_oled_up+256+32, (char *)string, 16, 128, FALSE);
	}
}

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
static void _middle_render_horizontal_phone_finder()
{
	I16U offset = 0;

	_render_one_icon_24(ICON24_PHONE_FINDER_0_LEN, cling.ui.p_oled_up+128+offset, asset_content+ICON24_PHONE_FINDER_0_POS);
	offset += ICON24_PHONE_FINDER_0_LEN;
	offset += 12;
	
	_render_one_icon_24(ICON24_PHONE_FINDER_1_LEN, cling.ui.p_oled_up+128+offset, asset_content+ICON24_PHONE_FINDER_1_POS);
	offset += ICON24_PHONE_FINDER_1_LEN;
	offset += 12;
	
	_render_one_icon_24(ICON24_PHONE_FINDER_2_LEN, cling.ui.p_oled_up+128+offset, asset_content+ICON24_PHONE_FINDER_2_POS);
	offset += ICON24_PHONE_FINDER_2_LEN;

	_middle_horizontal_alignment(cling.ui.p_oled_up+128, offset, FALSE, ALIGNMENT_TYPE_CENTER);
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_			
static void _middle_render_horizontal_workout_mode_switch()
{
	I8U string1[32];
	I8U string2[32];
	I8U len=0, workout_idx=0, frame_index=0;
	
	char *workout_en[] =   {"Run",     "Cycle", "Stairs",  "Elliptical", "Row",  "Aerobic", "Piloxing", "Others"};
	char *workout_s_cn[] = {"跑步机 ", "单车",  "爬楼梯 ", "椭圆机 ",    "划船", "有氧操 ", "Piloxing", "其它"};
	char *workout_t_cn[] = {"跑步机 ", "單車",  "爬樓梯 ", "橢圓機 ",    "划船", "有氧操 ", "Piloxing", "其它"};
	char *workout_indicator[] = {
		"-,,,,,,,",
		",-,,,,,,", 
		",,-,,,,,", 
		",,,-,,,,", 
		",,,,-,,,", 
		",,,,,-,,", 
		",,,,,,-,", 
		",,,,,,,-"};
	
  frame_index = cling.ui.frame_index;
	if ((frame_index < UI_DISPLAY_WORKOUT)	|| (frame_index > UI_DISPLAY_WORKOUT_OTHERS))
		return;
	
	workout_idx = frame_index - UI_DISPLAY_WORKOUT_RUNNING;
		
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {	
		sprintf((char *)string1, "%s", workout_en[workout_idx]);
	}	else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {	
		sprintf((char *)string1, "%s", workout_s_cn[workout_idx]);
	} else {
		sprintf((char *)string1, "%s", workout_t_cn[workout_idx]);
	}

	FONT_load_characters(cling.ui.p_oled_up+128, (char *)string1, 16, 128, TRUE);
	
	len = sprintf((char *)string2, "%s", workout_indicator[workout_idx]);
	_horizontal_display_dynamic(cling.ui.p_oled_up+384, len, string2);
}
#endif

static void _horizontal_core_run_distance(I32U stat, BOOLEAN b_all_hold)
{
	I8U string[32];
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
	
	_render_middle_stat_section(len, string, margin, offset, b_all_hold);
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
	I8U string[32];
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {	
	  sprintf((char *)string, "RUN DATA");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		sprintf((char *)string, "训练分析");
	} else {
		sprintf((char *)string, "訓練分析");
	}
	
	FONT_load_characters(cling.ui.p_oled_up+128+24, (char *)string, 16, 128, FALSE);
}
#endif

static void _display_running_time_core(I8U *string, I8U len, I8U offset_in, BOOLEAN b_hour_hold, BOOLEAN b_all_hold)
{
	I8U *p0, *pin, *p1, *p2;
	I8U i, j, offset, char_len;
	I8U margin = 3;
	
	// Text line
	p0 = cling.ui.p_oled_up+128+offset_in;
	p1 = p0 + 128;
	p2 = p1 + 128;
	offset = 0;
	for (i = 0; i < len; i++) {

		if (string[i] == ' ') {
			// This is "space" character
			char_len = 4;
		} if (b_all_hold) {
			pin = (I8U *)(asset_content+asset_pos[512+string[i]]);
			char_len = asset_len[512+string[i]];				
		} else if ((b_hour_hold) && (i ==0)) {
			pin = (I8U *)(asset_content+asset_pos[512+string[i]]);
			char_len = asset_len[512+string[i]];			
		} else {
      if ((string[i] >= '0') && (string[i] <= '9')) {
			  pin = (I8U *)(asset_content+asset_pos[512+string[i]+152]);
			  char_len = asset_len[512+string[i]+152];
			} else {
			  pin = (I8U *)(asset_content+asset_pos[512+string[i]]);
			  char_len = asset_len[512+string[i]];				
			}
		}		
	
		for (j = 0; j < char_len; j++) {
			*p0++ = (*pin++);
			*p1++ = (*pin++);
			*p2++ = (*pin++);
		}			
		
		offset += char_len;
		
		p0 += margin;
		p1 += margin;;
		p2 += margin;
		if (i < len)
			offset += margin;
	}
}

static void _middle_render_horizontal_running_time()
{
	//_horizontal_core_run_time(cling.run_stat.time_min, cling.run_stat.time_sec, 0);
	I8U string[32];
	I8U hour=0, min=0, sec=0, len=0;

	min = cling.run_stat.time_min;
	sec = cling.run_stat.time_sec;
	
	hour = min/60;
	min = min - hour*60;
	
	// Render the time.
	if (hour > 9) hour = 9;
	len = sprintf((char *)string, "%d:%02d:%02d", hour, min, sec);
	_display_running_time_core(string, len, 33, TRUE, FALSE);
}

static void _middle_render_horizontal_running_pace()
{
	I8U string[32];
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 2;
	I32U pace = cling.run_stat.time_min; // Rounding 
	I32U min, sec;
	
	if (cling.run_stat.distance) {
		pace = cling.run_stat.time_min*60;
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
	len = sprintf((char *)string, "%d/%02d*", min, sec);
	_render_middle_stat_section(len, string, margin, offset, FALSE);
}

static void _middle_render_horizontal_running_cadence()
{
	I8U string[32];
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
	_render_middle_stat_section(len, string, margin, offset, FALSE);
}

static void _middle_render_horizontal_running_stride()
{
	I8U string[32];
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
	
	_render_middle_stat_section(len, string, margin, offset, FALSE);
}

static void _middle_render_horizontal_running_hr()
{
	I8U string[32];
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
	
	_render_middle_stat_section(len, string, margin, offset, FALSE);
}

static void _middle_render_horizontal_running_calories()
{
	I8U string[32];
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 2;
	I32U stat;

	stat = cling.run_stat.calories;
	
	len = sprintf((char *)string, "%d", stat);

	offset = ((80 - len*13 - (len-1)*2)/2) + 16;
	
	_render_middle_stat_section(len, string, margin, offset, FALSE);
}

#ifdef _CLINGBAND_PACE_MODEL_
static void _middle_render_horizontal_running_stop_analysis()
{
	I8U string[32];
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {	
	  sprintf((char *)string, "INFO END");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		sprintf((char *)string, "结束分析");
	} else {
		sprintf((char *)string, "結束分析");
	}
	
	FONT_load_characters(cling.ui.p_oled_up+128+24, (char *)string, 16, 128, FALSE);
}
#endif

static void _middle_render_horizontal_training_start_run()
{
	I8U string[32];
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {	
	  sprintf((char *)string, "RUN NOW");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		sprintf((char *)string, "开始跑步 ");
	} else {
		sprintf((char *)string, "開始跑步 ");
	}

	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {		
#ifdef _CLINGBAND_PACE_MODEL_		
	  FONT_load_characters(cling.ui.p_oled_up+128+32, (char *)string, 16, 128, FALSE);
#else
	  FONT_load_characters(cling.ui.p_oled_up+128+34, (char *)string, 16, 128, FALSE);
#endif	
	} else {
	  FONT_load_characters(cling.ui.p_oled_up+128+30, (char *)string, 16, 128, FALSE);		
	}
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _middle_render_horizontal_training_run_or_analysis()
{
	I8U i;
	I8U *p0, *p1;
	I8U *in = cling.ui.p_oled_up+128+40;
	
	memset(in, 0xff, 32);
	memset(in+128, 0xff, 32);
	
	FONT_load_characters(cling.ui.p_oled_up+128+40, "开始 ", 16, 128, FALSE);
 	FONT_load_characters(cling.ui.p_oled_up+128+96, "记录 ", 16, 128, FALSE);
	
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

static BOOLEAN _middle_render_horizontal_core_ready(I8U *pin, BOOLEAN b_center)
{
	I8U string[32];
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 0;
	I8U *in;
  I32U t_curr = CLK_get_system_time();
  BOOLEAN b_go_display = FALSE;
	BOOLEAN b_ready_finished = FALSE;
	
  cling.ui.touch_time_stamp = CLK_get_system_time();
	
	if (cling.ui.run_ready_index == 0) {
		if (t_curr > (cling.ui.running_time_stamp + 500)) {
			cling.ui.run_ready_index = 1;
		  cling.ui.running_time_stamp = t_curr;
		}
		len = sprintf((char *)string, "3");
	} else if(cling.ui.run_ready_index == 1) {
		if (t_curr > (cling.ui.running_time_stamp + 800)) {
			cling.ui.run_ready_index = 2;
		  cling.ui.running_time_stamp = t_curr;
		}
		len = sprintf((char *)string, "2");
	} else if(cling.ui.run_ready_index == 2) {
		if (t_curr > (cling.ui.running_time_stamp + 800)) {
			cling.ui.run_ready_index = 3;
		  cling.ui.running_time_stamp = t_curr;			
		}	
		len = sprintf((char *)string, "1");	
	} else if(cling.ui.run_ready_index == 3) {	
		if (t_curr > (cling.ui.running_time_stamp + 500)) {
			cling.ui.run_ready_index = 0;
			b_ready_finished = TRUE;
		}
		len = sprintf((char *)string, "G O");		
		b_go_display = TRUE;
	} else {
		cling.ui.run_ready_index = 0;	
	}

	if (b_center) {
	  offset = _render_middle_clock_section(cling.ui.p_oled_up, len, string, margin, offset, 3);
		// Shift all the display to the middle
	  _middle_horizontal_alignment(cling.ui.p_oled_up, offset, FALSE, ALIGNMENT_TYPE_CENTER);
	} else {
		in = pin;
		if (b_go_display) {
			in = cling.ui.p_oled_up + 65;
		}
		_render_middle_clock_section(in, len, string, margin, offset, 3);
	}
	
	return b_ready_finished;
}

static void _middle_render_horizontal_training_ready()
{
	BOOLEAN b_ready_finished = FALSE;
	
  b_ready_finished = _middle_render_horizontal_core_ready(cling.ui.p_oled_up+70, FALSE);
	
	if (b_ready_finished) {
		cling.ui.frame_index = UI_DISPLAY_TRAINING_STAT_TIME;
	  cling.ui.frame_next_idx = cling.ui.frame_index;
	}
}

static void _middle_render_horizontal_training_time()
{
	I8U string[32];
	I8U len;
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
	
	if (hour) {
	  if (hour > 9) 
		  hour = 9;
	  len = sprintf((char *)string, "%d:%02d:%02d", hour, min, sec);
	  _display_running_time_core(string, len, 33, TRUE, TRUE);
	} else {
	  len = sprintf((char *)string, "%d:%02d", min, sec);
	  _display_running_time_core(string, len, 40, FALSE, TRUE);		
	}
}

static void _middle_render_horizontal_training_pace()
{
	I8U string[32];
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

	_render_middle_stat_section(len, string, margin, offset, TRUE);
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
	I8U string[32];
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {	
	  sprintf((char *)string, "STOP RUN");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
#ifdef _CLINGBAND_PACE_MODEL_				
		sprintf((char *)string, "结束跑步");
#else 
		sprintf((char *)string, "结束运动");
#endif		
	} else {
#ifdef _CLINGBAND_PACE_MODEL_				
		sprintf((char *)string, "结束跑步");
#else 
		sprintf((char *)string, "結束運動");
#endif				
	}
	
	FONT_load_characters(cling.ui.p_oled_up+128+26, (char *)string, 16, 128, FALSE);
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _middle_render_horizontal_workout_ready()
{	
	BOOLEAN b_ready_finished = FALSE;
	
  b_ready_finished = _middle_render_horizontal_core_ready(cling.ui.p_oled_up+57, TRUE);
	
	if (b_ready_finished) {
		cling.ui.frame_index = UI_DISPLAY_WORKOUT_RUN_TIME;
	  cling.ui.frame_next_idx = cling.ui.frame_index;
	}
}

static void _middle_render_horizontal_cycling_outdoor_run_start()
{
	I8U string[32];
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {	
	  sprintf((char *)string, "RUN NOW");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {	
		sprintf((char *)string, "开始骑行 ");	
	} else {
		sprintf((char *)string, "開始騎行 ");			
	}
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {	
	  FONT_load_characters(cling.ui.p_oled_up+128+34, (char *)string, 16, 128, FALSE);
	} else {
		FONT_load_characters(cling.ui.p_oled_up+128+30, (char *)string, 16, 128, FALSE);
	}
}

static void _middle_render_horizontal_cycling_outdoor_ready()
{
	BOOLEAN b_ready_finished = FALSE;
	
  b_ready_finished = _middle_render_horizontal_core_ready(cling.ui.p_oled_up+70, FALSE);
	
	if (b_ready_finished) {
		cling.ui.frame_index = UI_DISPLAY_CYCLING_OUTDOOR_STAT_TIME;
	  cling.ui.frame_next_idx = cling.ui.frame_index;
	}
}

static void _middle_render_horizontal_cycling_outdoor_distance()
{
  BOOLEAN b_ble_connected = FALSE;
	I8U string[32];
	
	if (BTLE_is_connected())
		b_ble_connected = TRUE;
	
	if (b_ble_connected) {
		_horizontal_core_run_distance(cling.run_stat.distance, TRUE);
	} else {
		_render_one_icon_24(ICON24_NO_SKIN_TOUCH_LEN, cling.ui.p_oled_up+128+40, asset_content+ICON24_NO_SKIN_TOUCH_POS);
		sprintf((char *)string, "无蓝牙 ");	
		FONT_load_characters(cling.ui.p_oled_up+128+128+80, (char *)string, 16, 128, FALSE);
	}
}

static void _middle_render_horizontal_cycling_outdoor_speed()
{
  BOOLEAN b_ble_connected = FALSE;

	if (BTLE_is_connected())
		b_ble_connected = TRUE;
	
	if (b_ble_connected) {
		_horizontal_core_run_distance(17, TRUE);
	} else {
		_render_one_icon_24(ICON24_NO_SKIN_TOUCH_LEN, cling.ui.p_oled_up+128+40, asset_content+ICON24_NO_SKIN_TOUCH_POS);
	}
}

static void _middle_render_horizontal_cycling_outdoor_run_stop()
{
	I8U string[32];
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {	
	  sprintf((char *)string, "STOP RUN");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {	
		sprintf((char *)string, "结束骑行");
	} else {
		sprintf((char *)string, "結束騎行");			
	}
	
	FONT_load_characters(cling.ui.p_oled_up+128+28, (char *)string, 16, 128, FALSE);

}
#endif

#ifndef _CLINGBAND_PACE_MODEL_
static void _middle_render_horizontal_stopwatch_core(I8U *string, I8U len, I8U offset_in, BOOLEAN b_hour_hold)
{
	I8U *p0, *pin, *p1, *p2;
	I8U i, j, offset, char_len;
	I8U margin = 2;
	
	// Text line
	p0 = cling.ui.p_oled_up+128+offset_in;
	p1 = p0 + 128;
	p2 = p1 + 128;
	offset = 0;
	
	for (i = 0; i < len; i++) {
		if (string[i] == ' ') {
			// This is "space" character
			char_len = 4;
		} else if ((b_hour_hold) && (i ==0)) {
			pin = (I8U *)(asset_content+asset_pos[512+string[i]]);
			char_len = asset_len[512+string[i]];			
		} else {
			if ((!b_hour_hold) && (i >= (len - 2))) {
				pin = (I8U *)(asset_content+asset_pos[256+string[i]]);
			  char_len = asset_len[256+string[i]];	
			}	else {
				if ((string[i] >= '0') && (string[i] <= '9')) {
					pin = (I8U *)(asset_content+asset_pos[512+string[i]+152]);
					char_len = asset_len[512+string[i]+152];
				} else {
					pin = (I8U *)(asset_content+asset_pos[512+string[i]]);
					char_len = asset_len[512+string[i]];				
				}				
			}
		}		

    if ((!b_hour_hold) && (i >= (len - 2))) {		
			for (j = 0; j < char_len; j++) {
				*p0++ = 0;
				*p1++ = (*pin++);
				*p2++ = (*pin++);				
			}				
		} else {
			for (j = 0; j < char_len; j++) {
				*p0++ = (*pin++);
				*p1++ = (*pin++);
				*p2++ = (*pin++);
			}				
		}
		
		offset += char_len;
		
		p0 += margin;
		p1 += margin;;
		p2 += margin;
		if (i < len)
			offset += margin;
	}
}

static void _middle_render_horizontal_stopwatch_start()
{
	I8U string[32];
	I8U len = 0;
	I32U t_diff = 0;
	I8U hour = 0, min = 0, sec = 0, ms = 0;

  if (cling.ui.b_stopwatch_first_enter) {
	  cling.ui.stopwatch_time_stamp = CLK_get_system_time();		
		cling.ui.b_stopwatch_first_enter = FALSE;
	} else {
		t_diff = CLK_get_system_time();
		t_diff -= cling.ui.stopwatch_time_stamp;
		ms = t_diff % 1000;
		t_diff /= 1000;
		hour = t_diff / 3600;
		t_diff -= hour*3600;
		min = t_diff / 60;
		t_diff -= min * 60;
		sec = t_diff;
		
		ms /= 10;
	}
	
	if (hour) {
		// Render the time
		if (hour > 9) hour = 9;
		len = sprintf((char *)string, "%d:%02d:%02d", hour, min, sec);
		_middle_render_horizontal_stopwatch_core(string, len, 28, TRUE);
	} else {
	  // Render the time
		len = sprintf((char *)string, "%d:%02d.%02d", min, sec, ms);
		if (min > 9) {
	    _middle_render_horizontal_stopwatch_core(string, len, 23, FALSE);					
		} else {
	    _middle_render_horizontal_stopwatch_core(string, len, 33, FALSE);			
		}
	}
}

static void _middle_render_horizontal_stopwatch_stop()
{
	I8U string[32];
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {	
	  sprintf((char *)string, "END");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		sprintf((char *)string, "结束");
	} else {
		sprintf((char *)string, "結束");
	}
	
	FONT_load_characters(cling.ui.p_oled_up+128, (char *)string, 16, 128, TRUE);
}
#endif

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
static void _middle_render_horizontal_music_play()
{
	I16U offset = 10;

	_render_one_icon_24(ICON24_MUSIC_PLAY_LEN, cling.ui.p_oled_up+128+offset, asset_content+ICON24_MUSIC_PLAY_POS);
	offset += ICON24_MUSIC_PLAY_LEN;
	offset += 20;
	
	_render_one_icon_24(ICON24_MUSIC_MUTE_LEN, cling.ui.p_oled_up+128+offset, asset_content+ICON24_MUSIC_MUTE_POS);
	offset += ICON24_MUSIC_MUTE_LEN;
	offset += 20;
	
	_render_one_icon_16(ICON16_MUSIC_MORE_LEN, cling.ui.p_oled_up+128+128+offset, asset_content+ICON16_MUSIC_MORE_POS);
	offset += ICON16_MUSIC_MORE_LEN;

	_middle_horizontal_alignment(cling.ui.p_oled_up+128, offset, FALSE, ALIGNMENT_TYPE_CENTER);
}

static void _middle_render_horizontal_music_track()
{
	I16U offset = 10;

	_render_one_icon_24(ICON24_MUSIC_PREV_SONG_LEN, cling.ui.p_oled_up+128+offset, asset_content+ICON24_MUSIC_PREV_SONG_POS);
	offset += ICON24_MUSIC_PREV_SONG_LEN;
	offset += 20;
	
	_render_one_icon_24(ICON24_MUSIC_NEXT_SONG_LEN, cling.ui.p_oled_up+128+offset, asset_content+ICON24_MUSIC_NEXT_SONG_POS);
	offset += ICON24_MUSIC_NEXT_SONG_LEN;
	offset += 20;
	
	_render_one_icon_16(ICON16_MUSIC_MORE_LEN, cling.ui.p_oled_up+128+128+offset, asset_content+ICON16_MUSIC_MORE_POS);
	offset += ICON16_MUSIC_MORE_LEN;

	_middle_horizontal_alignment(cling.ui.p_oled_up+128, offset, FALSE, ALIGNMENT_TYPE_CENTER);
}

static void _middle_render_horizontal_music_volume()
{
	I16U offset = 10;

	_render_one_icon_24(ICON24_MUSIC_VOLUME_DOWN_LEN, cling.ui.p_oled_up+128+offset, asset_content+ICON24_MUSIC_VOLUME_DOWN_POS);
	offset += ICON24_MUSIC_VOLUME_DOWN_LEN;
	offset += 20;
	
	_render_one_icon_24(ICON24_MUSIC_VOLUME_UP_LEN, cling.ui.p_oled_up+128+offset, asset_content+ICON24_MUSIC_VOLUME_UP_POS);
	offset += ICON24_MUSIC_VOLUME_UP_LEN;
	offset += 20;
	
	_render_one_icon_16(ICON16_MUSIC_MORE_LEN, cling.ui.p_oled_up+128+128+offset, asset_content+ICON16_MUSIC_MORE_POS);
	offset += ICON16_MUSIC_MORE_LEN;

	_middle_horizontal_alignment(cling.ui.p_oled_up+128, offset, FALSE, ALIGNMENT_TYPE_CENTER);	
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_
static void _middle_render_horizontal_carousel_1()
{
	I8U *in = cling.ui.p_oled_up+128;
	I8U offset = 4;
	
  _render_one_icon_24(ICON24_RUNNING_MODE_LEN, in+offset, asset_content+ICON24_RUNNING_MODE_POS);	
	offset += ICON24_RUNNING_MODE_LEN;
	offset += 24;

  _render_one_icon_24(ICON24_CYCLING_OUTDOOR_MODE_LEN, in+offset, asset_content+ICON24_CYCLING_OUTDOOR_MODE_POS);	
	offset += ICON24_CYCLING_OUTDOOR_MODE_LEN;
	offset += 24;
	
  _render_one_icon_24(ICON24_WORKOUT_MODE_LEN, in+offset, asset_content+ICON24_WORKOUT_MODE_POS);	
}
#endif

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
static void _middle_render_horizontal_carousel_2()
{
	I8U *in = cling.ui.p_oled_up+128;
	I8U offset = 4;
	
  _render_one_icon_24(ICON24_MUSIC_LEN, in+offset, asset_content+ICON24_MUSIC_POS);	
	offset += ICON24_MUSIC_LEN;
	offset += 24;

  _render_one_icon_24(ICON24_STOPWATCH_LEN, in+offset, asset_content+ICON24_STOPWATCH_POS);	
	offset += ICON24_STOPWATCH_LEN;
	offset += 24;
	
  _render_one_icon_24(ICON24_MESSAGE_LEN, in+offset, asset_content+ICON24_MESSAGE_POS);	
}
#endif

#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	
static void _middle_render_horizontal_carousel_2()
{
	I8U *in = cling.ui.p_oled_up+128;
	I8U offset = 4;
	
  _render_one_icon_24(ICON24_MESSAGE_LEN, in+offset, asset_content+ICON24_MESSAGE_POS);	
	offset += ICON24_MESSAGE_LEN;
	offset += 24;

  _render_one_icon_24(ICON24_STOPWATCH_LEN, in+offset, asset_content+ICON24_STOPWATCH_POS);	
	offset += ICON24_STOPWATCH_LEN;
	offset += 24;
	
  _render_one_icon_24(ICON24_WEATHER_LEN, in+offset, asset_content+ICON24_WEATHER_POS);	
}
#endif

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
static void _middle_render_horizontal_carousel_3()
{
	I8U *in = cling.ui.p_oled_up+128;
	I8U offset = 4;
	
  _render_one_icon_24(ICON24_WEATHER_LEN, in+offset, asset_content+ICON24_WEATHER_POS);	
	offset += ICON24_WEATHER_LEN;
	offset += 24;

  _render_one_icon_24(ICON24_PM2P5_LEN, in+offset, asset_content+ICON24_PM2P5_POS);	
	offset += ICON24_PM2P5_LEN;
	offset += 24;
	
  _render_one_icon_24(ICON24_NORMAL_ALARM_CLOCK_LEN, in+offset, asset_content+ICON24_NORMAL_ALARM_CLOCK_POS);	
}
#endif

#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	
static void _middle_render_horizontal_carousel_3()
{
	I8U *in = cling.ui.p_oled_up+128;
	I8U offset = 4;
	
  _render_one_icon_24(ICON24_PM2P5_LEN, in+offset, asset_content+ICON24_PM2P5_POS);	
	offset += ICON24_PM2P5_LEN;
	offset += 24;

  _render_one_icon_24(ICON24_NORMAL_ALARM_CLOCK_LEN, in+offset, asset_content+ICON24_NORMAL_ALARM_CLOCK_POS);	
	offset += ICON24_NORMAL_ALARM_CLOCK_LEN;
	offset += 24;
	
  _render_one_icon_24(ICON24_SETTING_LEN, in+offset, asset_content+ICON24_SETTING_POS);	
}
#endif

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
static void _middle_render_horizontal_carousel_4()
{
	I8U *in = cling.ui.p_oled_up+128;
	I8U offset = 4;
	
  _render_one_icon_24(ICON24_BATT_POWER_LEN, in+offset, asset_content+ICON24_BATT_POWER_POS);	
	offset += ICON24_BATT_POWER_LEN;
	offset += 24;

  _render_one_icon_24(ICON24_PHONE_FINDER_LEN, in+offset, asset_content+ICON24_PHONE_FINDER_POS);	
	offset += ICON24_PHONE_FINDER_LEN;
	offset += 24;
	
  _render_one_icon_24(ICON24_SETTING_LEN, in+offset, asset_content+ICON24_SETTING_POS);	
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_
static void _right_render_horizontal_delta_data_backward()
{
  I8U string[16];
  I8U len;
	SYSTIME_CTX delta;

	RTC_get_delta_clock_backward(&delta, cling.ui.vertical_index);

	len = sprintf((char *)string, "%d/%02d", delta.month, delta.day);
	FONT_load_characters(cling.ui.p_oled_up+(128-len*6), (char *)string, 8, 128, FALSE);
}

static void _right_render_horizontal_ok()
{
	I8U *in = cling.ui.p_oled_up+110;

	_render_one_icon_16(ICON16_OK_LEN, in, asset_content+ICON16_OK_POS);
}
#endif

static void _right_render_horizontal_home()
{
	I8U string[32];
	I8U len;
 	BOOLEAN b_batt_charging = FALSE;

	char *week_en[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
	char *week_cn_s[] = {"周一", "周二", "周三", "周四", "周五", "周六", "周日"};
	char *week_cn_t[] = {"周壹", "周二", "周三", "周四", "周五", "周六", "周日"};

	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {	
		sprintf((char *)string, "%s", week_en[cling.time.local.dow]);
		FONT_load_characters(cling.ui.p_oled_up+256+104, (char *)string, 16, 128, FALSE);
	}	else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {	
		sprintf((char *)string, "%s", week_cn_s[cling.time.local.dow]);
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	} else {
		sprintf((char *)string, "%s", week_cn_t[cling.time.local.dow]);
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	}
	
	if (BATT_is_charging()) 
		b_batt_charging = TRUE;
 
 	if (b_batt_charging) {
	  len = sprintf((char *)string, "%d %%", cling.system.mcu_reg[REGISTER_MCU_BATTERY]);
	  FONT_load_characters(cling.ui.p_oled_up+(128-len*6), (char *)string, 8, 128, FALSE);
	} else {
	  len = sprintf((char *)string, "%d/%02d", cling.time.local.month, cling.time.local.day);
	  FONT_load_characters(cling.ui.p_oled_up+(128-len*6), (char *)string, 8, 128, FALSE);
	}
}

static void _right_render_horizontal_small_clock()
{
	I8U string[32];
	I8U len;

	if (cling.ui.clock_sec_blinking) {
		cling.ui.clock_sec_blinking = FALSE;
		len = sprintf((char *)string, "%d:%02d",cling.time.local.hour, cling.time.local.minute);
	} else {
		cling.ui.clock_sec_blinking = TRUE;
		len = sprintf((char *)string, "%d %02d",cling.time.local.hour, cling.time.local.minute);		
	}
 
	FONT_load_characters(cling.ui.p_oled_up+(128-len*6), (char *)string, 8, 128, FALSE);
}

#ifdef _CLINGBAND_PACE_MODEL_
static void _right_render_horizontal_button_hold()
{
	I8U string[32];
	I8U i;
	I8U *p0, *p1;
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
	  sprintf((char *)string, "HOLD");
  } else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {	
		sprintf((char *)string, "长按");
	} else {
	  sprintf((char *)string, "長按");
	}
	
	FONT_load_characters(cling.ui.p_oled_up+128+96, (char *)string, 16, 128, FALSE);
	
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
	I8U *in = cling.ui.p_oled_up+384+115;
	
  if (cling.ui.b_detail_page) {
	 	_render_one_icon_8(ICON8_MORE_LEN, in, asset_content+ICON8_MORE_POS);	
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
	I8U string[32];
	I8U len;
	
	len = sprintf((char *)string, "%02d", cling.ui.app_notific_index);
	FONT_load_characters(cling.ui.p_oled_up+(128-len*8), (char *)string, 16, 128, FALSE);	

	_right_render_horizontal_more();
}

static void _right_render_horizontal_reminder()
{
	I8U string[16];
	I8U len;

	if (cling.reminder.ui_alarm_on)
		return;
	
  if (!cling.ui.b_detail_page) 	
		return;
	
	len = sprintf((char *)string, "%02d", cling.ui.vertical_index);
	FONT_load_characters(cling.ui.p_oled_up+(128-len*8), (char *)string, 16, 128, FALSE);	

	_right_render_horizontal_more();
}
#endif

static void _right_render_horizontal_firmware_ver()
{
	I8U string[32];
	I16U len;
	I16U major;
	I16U minor;
	
	major = cling.system.mcu_reg[REGISTER_MCU_REVH]>>4;
	minor = cling.system.mcu_reg[REGISTER_MCU_REVH]&0x0f;
	minor <<= 8;
	minor |= cling.system.mcu_reg[REGISTER_MCU_REVL];
	
#ifdef _ENABLE_TOUCH_
	len = sprintf((char *)string, "VER:%d.%d(%d)", major, minor,cling.whoami.touch_ver[2]);
#else  
	len = sprintf((char *)string, "VER:%d.%d", major, minor);
#endif	

	FONT_load_characters(cling.ui.p_oled_up+(128-len*6), (char *)string, 8, 128, FALSE);
}

static void _right_render_horizontal_running_distance()
{
	I8U string[32];

	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		sprintf((char *)string, "RUN");
		FONT_load_characters(cling.ui.p_oled_up+104, (char *)string, 16, 128, FALSE);	
		if (cling.user_data.profile.metric_distance) {
			sprintf((char *)string, "ML");
		} else {
			sprintf((char *)string, "KM");
		}
		FONT_load_characters(cling.ui.p_oled_up+256+112, (char *)string, 16, 128, FALSE);
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		sprintf((char *)string, "路跑");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);
		if (cling.user_data.profile.metric_distance) {
			sprintf((char *)string, "英里");
		} else {
			sprintf((char *)string, "公里");
		}
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	} else {
		sprintf((char *)string, "路跑");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);
		if (cling.user_data.profile.metric_distance) {
			sprintf((char *)string, "英裏");
		} else {
			sprintf((char *)string, "公裏");
		}
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	}
}

static void _right_render_horizontal_running_pace()
{
	I8U string[32];

	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		sprintf((char *)string, "AVG");
		FONT_load_characters(cling.ui.p_oled_up+104, (char *)string, 16, 128, FALSE);	
		sprintf((char *)string, "PACE");
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		sprintf((char *)string, "平均");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);
		sprintf((char *)string, "配速 ");
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	} else {
		sprintf((char *)string, "平均");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);
		sprintf((char *)string, "配速 ");
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	}
}

static void _right_render_horizontal_running_calories()
{
	I8U string[32];

	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		sprintf((char *)string, "BURN");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);	
		sprintf((char *)string, "CALS");
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		sprintf((char *)string, "热量");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);
		sprintf((char *)string, "大卡");
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	} else {
		sprintf((char *)string, "熱量");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);
		sprintf((char *)string, "大卡");
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	}
}

static void _right_render_horizontal_running_cadence()
{
	I8U string[32];

	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		sprintf((char *)string, "RATE");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);	
		sprintf((char *)string, "SPM");
		FONT_load_characters(cling.ui.p_oled_up+256+104, (char *)string, 16, 128, FALSE);
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		sprintf((char *)string, "步频");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);
		sprintf((char *)string, "步分");
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	} else {
		sprintf((char *)string, "步頻");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);
		sprintf((char *)string, "步分");
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	}
}

static void _right_render_horizontal_running_stride()
{
	I8U string[32];

	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		sprintf((char *)string, "STRIDE");
		FONT_load_characters(cling.ui.p_oled_up+80, (char *)string, 16, 128, FALSE);	
		if (cling.user_data.profile.metric_distance) {
		  sprintf((char *)string, "IN");			
		} else {
		  sprintf((char *)string, "CM");
		}
		FONT_load_characters(cling.ui.p_oled_up+256+112, (char *)string, 16, 128, FALSE);
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		sprintf((char *)string, "步幅");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);
		if (cling.user_data.profile.metric_distance) {
		  sprintf((char *)string, "英寸");
		} else {			
		  sprintf((char *)string, "厘米");
		}
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	} else {
		sprintf((char *)string, "步幅");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);
		if (cling.user_data.profile.metric_distance) {
		  sprintf((char *)string, "英寸");
		} else {			
		  sprintf((char *)string, "厘米");
		}
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	}
}

static void _right_render_horizontal_running_hr()
{
	I8U string[32];

	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		sprintf((char *)string, "AVG");
		FONT_load_characters(cling.ui.p_oled_up+104, (char *)string, 16, 128, FALSE);	
		sprintf((char *)string, "BPM");
		FONT_load_characters(cling.ui.p_oled_up+256+104, (char *)string, 16, 128, FALSE);
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		sprintf((char *)string, "平均");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);
		sprintf((char *)string, "心率");
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	} else {
		sprintf((char *)string, "平均");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);
		sprintf((char *)string, "心率");
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	}
}

#if 0
static void _right_render_horizontal_running_time()
{
	I8U string[128];
	
	if (cling.run_stat.time_min > 59)
		return;

	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		sprintf((char *)string, "TIME");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);	
		sprintf((char *)string, "SEC");
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		sprintf((char *)string, "耗时");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);
	} else {
		sprintf((char *)string, "耗時");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);
	}
}
#endif

static void _right_render_horizontal_training_pace()
{
	I8U string[32];

	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		sprintf((char *)string, "RT");
		FONT_load_characters(cling.ui.p_oled_up+104, (char *)string, 16, 128, FALSE);	
		sprintf((char *)string, "PACE");
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		sprintf((char *)string, "当前");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);
		sprintf((char *)string, "配速 ");
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	} else {
		sprintf((char *)string, "當前");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);
		sprintf((char *)string, "配速 ");
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	}
}

static void _right_render_horizontal_training_distance()
{
	I8U string[32];

	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		sprintf((char *)string, "RUN");
		FONT_load_characters(cling.ui.p_oled_up+104, (char *)string, 16, 128, FALSE);	
		if (cling.user_data.profile.metric_distance) {
			sprintf((char *)string, "ML");
		} else {
			sprintf((char *)string, "KM");
		}
		FONT_load_characters(cling.ui.p_oled_up+256+112, (char *)string, 16, 128, FALSE);
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		sprintf((char *)string, "里程");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);
		if (cling.user_data.profile.metric_distance) {
			sprintf((char *)string, "英里");
		} else {
			sprintf((char *)string, "公里");
		}
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	} else {
		sprintf((char *)string, "裏程");
		FONT_load_characters(cling.ui.p_oled_up+96, (char *)string, 16, 128, FALSE);
		if (cling.user_data.profile.metric_distance) {
			sprintf((char *)string, "英裏");
		} else {
			sprintf((char *)string, "公裏");
		}
		FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)string, 16, 128, FALSE);
	}
}

static void _right_render_horizontal_training_hr()
{
	I8U string[32];
	I8U len = 0;
	I8U hr_perc = 0;
	I8U *p0;
  I8U j;
	
	p0 = cling.ui.p_oled_up+384+100;
	if (cling.hr.heart_rate_ready)
    hr_perc = cling.ui.training_hr;

	len = sprintf((char *)string, "%d%%", hr_perc);
	FONT_load_characters(cling.ui.p_oled_up+128-(len*8), (char *)string, 16, 128, FALSE);
	
	if (hr_perc) {
		for (j = 0; j < 5; j++) {
			*p0++ = 0xff;
		}
		*p0++ = 0;	
		*p0++ = 0;
	} else {
		*p0++ = 0xff;
		for (j = 0; j < 4; j++) {
			*p0++ = 0x81;
		}
		*p0++ = 0xff;
		*p0++ = 0;		
	}
	
	if (hr_perc > 25) {
		for (j = 0; j < 5; j++) {
			*p0++ = 0xff;
		}
		*p0++ = 0;	
		*p0++ = 0;
	} else {
		*p0++ = 0xff;
		for (j = 0; j < 4; j++) {
			*p0++ = 0x81;
		}
		*p0++ = 0xff;
		*p0++ = 0;		
	} 

	if (hr_perc > 50) {
		for (j = 0; j < 5; j++) {
			*p0++ = 0xff;
		}
		*p0++ = 0;	
		*p0++ = 0;
	} else {
		*p0++ = 0xff;
		for (j = 0; j < 4; j++) {
			*p0++ = 0x81;
		}
		*p0++ = 0xff;
		*p0++ = 0x0;		
	} 

	if (hr_perc > 75) {
		for (j = 0; j < 5; j++) {
			*p0++ = 0xff;
		}
		*p0++ = 0;
		*p0++ = 0;
	} else {
		*p0++ = 0xff;
		for (j = 0; j < 4; j++) {
			*p0++ = 0x81;
		}
		*p0++ = 0xff;
		*p0++ = 0x0;		
	} 	
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _right_render_horizontal_run_ok()
{
	I8U i;
	I8U *p0, *p1;
	I8U *in = cling.ui.p_oled_up+128+104;
	
	memset(in, 0xff, 24);
	memset(in+128, 0xff, 24);
	
	FONT_load_characters(cling.ui.p_oled_up+128+108, "OK", 16, 128, FALSE);
 
  p0 = cling.ui.p_oled_up+128+108,
	p1 = p0+128;
	
	for (i=0;i<16;i++) {
	 *(p0+i) = ~(*(p0+i));
	 *(p1+i) = ~(*(p1+i));	
	}
}

static void _right_render_horizontal_run_go()
{
	I8U i;
	I8U *p0, *p1;
	I8U *in = cling.ui.p_oled_up+128+104;
	
	memset(in, 0xff, 24);
	memset(in+128, 0xff, 24);
	
	FONT_load_characters(cling.ui.p_oled_up+128+107, "G", 16, 128, FALSE);
 	FONT_load_characters(cling.ui.p_oled_up+128+117, "O", 16, 128, FALSE);
	
  p0 = cling.ui.p_oled_up+128+107,
	p1 = p0+128;
	
	for (i=0;i<8;i++) {
	 *(p0+i) = ~(*(p0+i));
	 *(p1+i) = ~(*(p1+i));	
	}
	
  p0 = cling.ui.p_oled_up+128+117,
	p1 = p0+128;
	
	for (i=0;i<8;i++) {
	 *(p0+i) = ~(*(p0+i));
	 *(p1+i) = ~(*(p1+i));	
	}	
}

static void _right_render_horizontal_cycling_outdoor_distance()
{
  BOOLEAN b_ble_connected = FALSE;
	
	if (BTLE_is_connected())
		b_ble_connected = TRUE;
	
	if (b_ble_connected) {
    _right_render_horizontal_training_distance();
	}
}

static void _right_render_horizontal_cycling_outdoor_speed()
{
	I8U string[32];
	I8U len;
  BOOLEAN b_ble_connected = FALSE;
	
	if (BTLE_is_connected())
		b_ble_connected = TRUE;
	
	if (b_ble_connected) {
	  len = sprintf((char *)string, "KM/H");
	  FONT_load_characters(cling.ui.p_oled_up+384+(128-len*6), (char *)string, 8, 128, FALSE);	
	} else {
		sprintf((char *)string, "无蓝牙 ");	
		FONT_load_characters(cling.ui.p_oled_up+128+128+80, (char *)string, 16, 128, FALSE);
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

static void _rotate_270_degree(I8U *in, I8U *out)
{
	I8U *in_data;
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

static void _render_vertical_character(I8U *string, I8U offset, I8U margin, I8U len, I8U b_24_size, BOOLEAN b_all_hold, BOOLEAN b_font_lib)
{
	I8U *p0, *p1, *p2;
	I8U char_len, i, j, line_len;
	I8U *buf1;
	I8U *buf2;
	I8U *buf3;
	I8U buf_fonts[256];
	const I8U *pin;
	
	buf1 = buf_fonts;
	buf2 = buf1 + 128;
	buf3 = buf2 + 32;
	
	// Render the hour
	memset(buf_fonts, 0, 256);
	p0 = buf1;
	p1 = buf2;
	p2 = buf3;
	if (b_font_lib) {
		line_len = FONT_load_characters(buf_fonts, (char *)string, 16, 128, FALSE);
		// Shift to the center
		_vertical_centerize(buf1, buf2, buf3, line_len);
		
		// do the rotation
		_rotate_270_degree(buf1, cling.ui.p_oled_up+384+offset);
		_rotate_270_degree(buf2, cling.ui.p_oled_up+384+offset+8);
		return;
	}
	line_len = 0;
	for (i = 0; i < len; i++) {
		// Digits in large fonts
		if (b_24_size == 24) {
			if (b_all_hold) {
				pin = asset_content+asset_pos[512+string[i]];
				char_len = asset_len[512+string[i]];				
			} else {
				if ((string[i] >= '0') && (string[i] <= '9')) {
					pin = asset_content+asset_pos[512+string[i]+152];
					char_len = asset_len[512+string[i]+152];
				} else {
					pin = asset_content+asset_pos[512+string[i]];
					char_len = asset_len[512+string[i]];
				}				
			}

			for (j = 0; j < char_len; j++) {
					*p0++ = (*pin++);
					*p1++ = (*pin++);
					*p2++ = (*pin++);
			}
		} else if (b_24_size == 16) {
			pin = asset_content+asset_pos[256+string[i]];
			char_len = asset_len[256+string[i]];
			for (j = 0; j < char_len; j++) {
					*p0++ = (*pin++);
					*p1++ = (*pin++);
					*p2++ = 0;
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
	_rotate_270_degree(buf1, cling.ui.p_oled_up+384+offset);
	if (b_24_size != 8)
		_rotate_270_degree(buf2, cling.ui.p_oled_up+384+offset+8);
	if (b_24_size == 24)
		_rotate_270_degree(buf3, cling.ui.p_oled_up+384+offset+16);
}

static void _render_vertical_icon(I8U icon_idx)
{
	I8U string[32];	
	I8U len = 0;	
	I8U	margin = 2;
	I8U b_24_size = 24;

	string[len++] = icon_idx;
	_render_vertical_character(string, 0, margin, len, b_24_size, FALSE, FALSE);
}

static void _top_render_vertical_batt_ble()
{
	I8U data_buf[128];
	I8U *in = cling.ui.p_oled_up;

	memset(data_buf, 0, 128);
	
	_render_batt_and_ble(data_buf);

	_rotate_270_degree(data_buf, in+384);
}

static void _top_render_vertical_steps()
{
	_render_vertical_icon(ICON24_STEPS_IDX);
}

static void _top_render_vertical_distance()
{
	_render_vertical_icon(ICON24_DISTANCE_IDX);
}

static void _top_render_vertical_calories()
{
	_render_vertical_icon(ICON24_CALORIES_IDX);
}

static void _top_render_vertical_pm2p5()
{
	_render_vertical_icon(ICON24_PM2P5_IDX);
}
		
static void _top_render_vertical_reminder()
{
	if ((cling.reminder.alarm_type == SLEEP_ALARM_CLOCK) || (cling.reminder.alarm_type == WAKEUP_ALARM_CLOCK))
    _render_vertical_icon(ICON24_SLEEP_ALARM_CLOCK_IDX);		
	else 
    _render_vertical_icon(ICON24_NORMAL_ALARM_CLOCK_IDX);		
}

static void _top_render_vertical_heart_rate()
{
	if (cling.ui.heart_rate_sec_blinking) {
		cling.ui.heart_rate_sec_blinking = FALSE;
		_render_vertical_icon(ICON24_HEART_RATE_IDX);
	} else {
		cling.ui.heart_rate_sec_blinking = TRUE;
	}
}

#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)	
static void _top_render_vertical_skin_temp()
{
	_render_vertical_icon(ICON24_SKIN_TEMP_IDX);
}
#endif

static void _top_render_vertical_weather()
{
	I8U string[32];	
  I8U len = 0;
	I8U margin = 2;;
	I8U b_24_size = 24;
	WEATHER_CTX weather;	

	WEATHER_get_weather(0, &weather);
	
	string[len++] = ICON24_WEATHER_IDX+weather.type;
	_render_vertical_character(string, 0, margin, len, b_24_size, FALSE, FALSE);	
}

static void _top_render_vertical_running_distance()
{
	_render_vertical_icon(ICON24_RUNNING_DISTANCE_IDX);
}

static void _top_render_vertical_running_time()
{
	I8U string[32];
	I8U len = 0;
	I8U margin = 0;	
	I8U b_24_size = 16;

	_render_vertical_icon(ICON24_RUNNING_TIME_IDX);
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "TIME");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "耗时");
	} else {
		len = sprintf((char *)string, "耗時");
	}
	
	_render_vertical_character(string, 28, margin, len, b_24_size, FALSE, TRUE);
}

static void _top_render_vertical_running_calories()
{
	_render_vertical_icon(ICON24_RUNNING_CALORIES_IDX);
}

static void _top_render_vertical_running_cadence()
{
	_render_vertical_icon(ICON24_RUNNING_CADENCE_IDX);
}

static void _top_render_vertical_running_stride()
{
	_render_vertical_icon(ICON24_RUNNING_STRIDE_IDX);
}

static void _top_render_vertical_running_pace()
{
	_render_vertical_icon(ICON24_RUNNING_PACE_IDX);
}

static void _top_render_vertical_running_hr()
{
	_render_vertical_icon(ICON24_RUNNING_HR_IDX);
}

static void _top_render_vertical_active_time()
{
	_render_vertical_icon(ICON24_ACTIVE_TIME_IDX);
}

#ifdef _CLINGBAND_UV_MODEL_
static void _top_render_vertical_uv_index()
{
	_render_vertical_icon(ICON24_UV_INDEX_IDX);
}
#endif

static void _top_render_vertical_training_time()
{
	I8U string[32];
	I8U len = 0;
	I8U margin = 2;	
	I8U b_24_size = 16;
  I32U t_curr = CLK_get_system_time();	

	_render_vertical_icon(ICON24_RUNNING_TIME_IDX);
	
	if (cling.ui.clock_sec_blinking) {
		cling.ui.clock_sec_blinking = FALSE;
	  if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		  len = sprintf((char *)string, "TIME");
	  } else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		  len = sprintf((char *)string, "耗时");
	  } else {
		  len = sprintf((char *)string, "耗時");
	  }
		_render_vertical_character(string, 28, margin, len, b_24_size, FALSE, TRUE);
  } else {
		cling.ui.clock_sec_blinking = TRUE;
	}
}

static void _top_render_vertical_running_stop()
{
	_render_vertical_icon(ICON24_RUNNING_STOP_IDX);
}

#ifndef _CLINGBAND_PACE_MODEL_		
static void _top_render_vertical_cycling_outdoor()
{
	_render_vertical_icon(ICON24_CYCLING_OUTDOOR_MODE_IDX);
}

static void _top_render_vertical_cycling_outdoor_speed()
{
	_render_vertical_icon(ICON24_CYCLING_OUTDOOR_SPEED_IDX);
}
#endif

static void _render_vertical_time(I8U *string, I8U offset, BOOLEAN b_bold, BOOLEAN b_center)
{
	I8U *p0, *p1, *p2;
	I8U char_len, i, j;
	I8U buf1[256];
	I8U *buf2, *buf3;
	const I8U *pin;
	I8U line_len=0;
	
	// Render the hour
	memset(buf1, 0, 256);

	buf2 = buf1 + 128;
	buf3 = buf2 + 32;	
	
	p0 = buf1;
	p1 = buf2;
	p2 = buf3;

	for (i = 0; i < 3; i++) {

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
			// Digits in large fonts
			if (b_bold) {
				pin = asset_content+asset_pos[512+string[i]];
				char_len = asset_len[512+string[i]];
			  line_len += char_len;	
			} else {
				if ((string[i] >= '0') && (string[i] <= '9')) {
					pin = asset_content+asset_pos[512+string[i]+152];
					char_len = asset_len[512+string[i]+152];
			    line_len += char_len;						
				} else {
					pin = asset_content+asset_pos[512+string[i]];
					char_len = asset_len[512+string[i]];
					line_len += char_len;	
				}
			}
		}
		
		for (j = 0; j < char_len; j++) {
			*p0++ = (*pin++);
			*p1++ = (*pin++);
			*p2++ = (*pin++);
		}					
		
		if (i != 2) {
		  p0 += 1;
		  p1 += 1;
		  p2 += 1;
		  line_len += 1;
		}
	}

	if (b_center)
    _vertical_centerize(buf1, buf2, buf3, line_len);	
	
	// do the rotation
	_rotate_270_degree(buf1, cling.ui.p_oled_up+384+offset);
	_rotate_270_degree(buf2, cling.ui.p_oled_up+384+offset+8);
	_rotate_270_degree(buf3, cling.ui.p_oled_up+384+offset+16);
}

static void _middle_render_vertical_clock()
{
	I8U string[32];

	// Render the hour
	sprintf((char *)string, " %02d", cling.time.local.hour);
	_render_vertical_time(string, 24, TRUE, FALSE);
	
	// Render the clock sign
	if (cling.ui.clock_sec_blinking) {
		cling.ui.clock_sec_blinking = FALSE;
		
		// Render the minute
		sprintf((char *)string, ":%02d", cling.time.local.minute);
	} else {
		cling.ui.clock_sec_blinking = TRUE;
		// Render the minute
		sprintf((char *)string, " %02d", cling.time.local.minute);
	}
	
	_render_vertical_time(string, 64, FALSE, FALSE);
}

static void _middle_render_vertical_steps()
{
	I8U string1[32];
	I8U string2[32];	
	I32U stat = 0;
	I8U len1=0, len2=0;
	I8U margin = 2;
	I8U b_24_size = 24;
	I32U v_10000 = 0, v_1000 = 0, v_100 = 0;

#ifdef _CLINGBAND_PACE_MODEL_	
	TRACKING_get_activity(0, TRACKING_STEPS, &stat);
#else  
	TRACKING_get_activity(cling.ui.vertical_index, TRACKING_STEPS, &stat);
#endif
	
	N_SPRINTF("UI: vertical step - %d", stat);
	
	if (stat > 99999) {
		v_10000 = 9;
		v_1000 = 9;
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			len1 = sprintf((char *)string1, "%d%d", v_10000, v_1000);
		} else {
			len1 = sprintf((char *)string1, "%d.%d", v_10000, v_1000);
		}
		margin = 1;
	} else if (stat > 9999) {
		v_10000 = stat / 10000;
		v_1000 = stat - (v_10000 * 10000);
		v_1000 /= 1000;
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			len1 = sprintf((char *)string1, "%d%d", v_10000, v_1000);
		} else {
			len1 = sprintf((char *)string1, "%d.%d", v_10000, v_1000);
		}
		margin = 1;
	} else if (stat > 999) {
		v_1000 = stat / 1000;
		v_100 = stat - (v_1000 * 1000);
		v_100 /= 100;
		len1 = sprintf((char *)string1, "%d.%d", v_1000, v_100);
		margin = 1;
	} else if (stat > 99) {
		len1 = sprintf((char *)string1, "%d", stat);
		b_24_size = 16;
	} else {
		len1 = sprintf((char *)string1, "%d", stat);
	}
	
	if (b_24_size == 24) 
		_render_vertical_character(string1, 42, margin, len1, b_24_size, FALSE, FALSE);
	else
		_render_vertical_character(string1, 46, margin, len1, b_24_size, FALSE, FALSE);

	b_24_size = 16;
	margin  = 0;
	
	if (stat > 9999) {
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			len1 = sprintf((char *)string1, "K");
			len2 = sprintf((char *)string2, "STEP");			
		} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
			len1 = sprintf((char *)string1, "万步");
		} else {
			len1 = sprintf((char *)string1, "萬步");
		}
	} else if (stat > 999) {
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			len1 = sprintf((char *)string1, "K");
			len2 = sprintf((char *)string2, "STEP");			
		} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
			len1 = sprintf((char *)string1, "千步");
		} else {
			len1 = sprintf((char *)string1, "千步");
		}
	} else {
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			len1 = sprintf((char *)string1, "STEP");
		} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
			len1 = sprintf((char *)string1, "步 ");
		} else {
			len1 = sprintf((char *)string1, "步 ");
		}
	}
	
	if (len2) {
	  _render_vertical_character(string1, 75, margin, len1, b_24_size, FALSE, TRUE);
		_render_vertical_character(string2, 95, margin, len2, b_24_size, FALSE, TRUE);
	} else {
		_render_vertical_character(string1, 80, margin, len1, b_24_size, FALSE, TRUE);
	}
}

static void _middle_render_vertical_distance()
{	
	I8U string[32];	
	I32U stat = 0;
	I8U len = 0;
	I8U margin = 2;
	I8U b_24_size = 24;
	I32U v_1000 = 0, v_100 = 0;

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
		len = sprintf((char *)string, "%d", v_1000);
	} else if (stat > 9999) {
		v_1000 = stat / 1000;
		len = sprintf((char *)string, "%d", v_1000);
	} else if (stat > 999) {
		v_1000 = stat / 1000;
		v_100 = stat - (v_1000 * 1000);
		v_100 /= 100;
		len = sprintf((char *)string, "%d.%d", v_1000, v_100);
		margin = 1;
	} else {
		v_100 = stat / 100;
		len = sprintf((char *)string, "0.%d", v_100);
		margin = 1;
	}
	
	_render_vertical_character(string, 42, margin, len, b_24_size, FALSE, FALSE);

  b_24_size = 16;
	margin = 0;
	
	if (cling.user_data.profile.metric_distance) {
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			len = sprintf((char *)string, "MILE");
		} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
			len = sprintf((char *)string, "英里");
		} else {
			len = sprintf((char *)string, "英裏");
		}
	} else {
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			len = sprintf((char *)string, "KM");
		} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
			len = sprintf((char *)string, "公里");
		} else {
			len = sprintf((char *)string, "公裏");
		}
	}
	
	_render_vertical_character(string, 80, margin, len, b_24_size, FALSE, TRUE);	
}

static void _middle_render_vertical_calories()
{
	I8U string1[32];
	I8U string2[32];	
	I32U stat = 0;
	I8U len1=0, len2=0;
	I8U margin = 2;
	I8U b_24_size = 24;
	I32U v_10000 = 0, v_1000 = 0, v_100 = 0;

#ifdef _CLINGBAND_PACE_MODEL_		
	TRACKING_get_activity(0, TRACKING_CALORIES, &stat);
#else
	TRACKING_get_activity(cling.ui.vertical_index, TRACKING_CALORIES, &stat);
#endif
	
	if (stat > 99999) {
		v_10000 = 9;
		v_1000 = 9;
		len1 = sprintf((char *)string1, "%d%d", v_10000, v_1000);
	} else if (stat > 9999) {
		v_10000 = stat / 10000;
		v_1000 = stat - (v_10000 * 10000);
		v_1000 /= 1000;
		len1 = sprintf((char *)string1, "%d%d", v_10000, v_1000);
	} else if (stat > 999) {
		v_1000 = stat / 1000;
		v_100 = stat - (v_1000 * 1000);
		v_100 /= 100;
		len1 = sprintf((char *)string1, "%d.%d", v_1000, v_100);
		margin = 1;
	} else if (stat > 99) {
		len1 = sprintf((char *)string1, "%d", stat);
		b_24_size = 16;
	} else {
		len1 = sprintf((char *)string1, "%d", stat);
	}
	
	if (b_24_size == 24) 
		_render_vertical_character(string1, 42, margin, len1, b_24_size, FALSE, FALSE);
	else
		_render_vertical_character(string1, 46, margin, len1, b_24_size, FALSE, FALSE);

	b_24_size = 16;
  margin = 0;
	
	if (stat > 999) {
	  if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			len1 = sprintf((char *)string1, "K");
			len2 = sprintf((char *)string2, "CAL");
		} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
			len1 = sprintf((char *)string1, "千 ");
			len2 = sprintf((char *)string2, "大卡");			
		} else {
			len1 = sprintf((char *)string1, "千 ");
			len2 = sprintf((char *)string2, "大卡");	
		}
	} else {
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			len1 = sprintf((char *)string1, "CAL");
		} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
			len1 = sprintf((char *)string1, "大卡");
		} else {
			len1 = sprintf((char *)string1, "大卡");
		}
	}
	
	if (len2) {
	  _render_vertical_character(string1, 75, margin, len1, b_24_size, FALSE, TRUE);
    _render_vertical_character(string2, 95, margin, len2, b_24_size, FALSE, TRUE);		
	} else {
	  _render_vertical_character(string1, 80, margin, len1, b_24_size, FALSE, TRUE);
	}
}

static void _middle_render_vertical_active_time()
{
	I8U string[32];	
	I32U stat = 0;
	I8U len = 0;
  I8U	margin = 2;
	I8U b_24_size = 24;

#ifdef _CLINGBAND_PACE_MODEL_			
	TRACKING_get_activity(0, TRACKING_ACTIVE_TIME, &stat);
#else
	TRACKING_get_activity(cling.ui.vertical_index, TRACKING_ACTIVE_TIME, &stat);
#endif
	
	N_SPRINTF("UI: vertical active time - %d", stat);
	if (stat > 99) {
		len = sprintf((char *)string, "%d", stat);
		b_24_size = 16;
	} else {
		len = sprintf((char *)string, "%d", stat);
	}
	
	if (b_24_size == 24) 
		_render_vertical_character(string, 42, margin, len, b_24_size, FALSE, FALSE);
	else
		_render_vertical_character(string, 46, margin, len, b_24_size, FALSE, FALSE);

	b_24_size = 16;
	margin = 0;
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "MIN");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "分钟");
	} else {
		len = sprintf((char *)string, "分鍾");
	}
	
	_render_vertical_character(string, 80, margin, len, b_24_size, FALSE, TRUE);
}

#ifdef _CLINGBAND_UV_MODEL_
static void _middle_render_vertical_uv_index()
{
	I8U string[32];	
	I8U len = 0;
  I8U	margin = 2;
	I8U b_24_size = 24;
	I8U integer = 0;
	
	integer = cling.uv.max_UI_uv;
	
	if (integer > 99) {
		len = sprintf((char *)string, "%d",(integer/10));
	} else {
		len = sprintf((char *)string, "%d.%d", (integer/10), (integer%10));
	  margin = 1;
	}
	
	_render_vertical_character(string, 50, margin, len, b_24_size, FALSE, FALSE);
}
#endif

static void _middle_render_vertical_training_run_start()
{
	I8U string[32];	
	I8U len = 0;
  I8U	margin = 0;	
	I8U b_24_size = 16;
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "RUN");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "开始 ");
	} else {
		len = sprintf((char *)string, "開始 ");
	}
	
	_render_vertical_character(string, 50, margin, len, b_24_size, FALSE, TRUE);
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "NOW");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "跑步");
	} else {
		len = sprintf((char *)string, "跑步");
	}
	
	_render_vertical_character(string, 70, margin, len, b_24_size, FALSE, TRUE);	
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _middle_render_vertical_training_run_or_analysis()
{
	I8U data_buf[256];
	I8U line_len;
	I8U i;
	I8U *p0, *p1;
	
	memset(data_buf, 0x00, 32);
	memset(data_buf+128, 0x00, 32);
	
	FONT_load_characters(data_buf, "开始 ", 16, 128, FALSE);

  p0 = data_buf,
	p1 = p0+128;
	
	for (i=0;i<32;i++) {
	 *(p0+i) = ~(*(p0+i));
	 *(p1+i) = ~(*(p1+i));	
	}
	
	line_len = 32;
	_vertical_centerize(data_buf, data_buf+128, data_buf+128+64, line_len);
	_rotate_270_degree(data_buf, cling.ui.p_oled_up+384+64);
	_rotate_270_degree(data_buf+128, cling.ui.p_oled_up+384+64+8);		
	
	memset(data_buf, 0x00, 32);
	memset(data_buf+128, 0x00, 32);
	
	FONT_load_characters(data_buf, "记录", 16, 128, FALSE);

  p0 = data_buf,
	p1 = p0+128;
	
	for (i=0;i<32;i++) {
	 *(p0+i) = ~(*(p0+i));
	 *(p1+i) = ~(*(p1+i));	
	}
	
	line_len = 32;
	_vertical_centerize(data_buf, data_buf+128, data_buf+128+64, line_len);
	_rotate_270_degree(data_buf, cling.ui.p_oled_up+384+112);
	_rotate_270_degree(data_buf+128, cling.ui.p_oled_up+384+112+8);			
}
#endif

#ifdef _CLINGBAND_PACE_MODEL_
static void _middle_render_vertical_running_analysis()
{
	I8U string[32];	
	I8U len = 0;
  I8U	margin = 0;	
	I8U b_24_size = 16;

	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "RUN");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "训练");
	} else {
		len = sprintf((char *)string, "訓練");
	}
	
	_render_vertical_character(string, 50, margin, len, b_24_size, FALSE, TRUE);
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "DATA");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "分析");
	} else {
		len = sprintf((char *)string, "分析");
	}
	
	_render_vertical_character(string, 70, margin, len, b_24_size, FALSE, TRUE);	
}
#endif

static void _middle_render_vertical_pm2p5()
{
	I8U string[32];	
	I32U stat = 0;
	I8U len = 0;
  I8U	margin = 2;
	I8U b_24_size = 24;
	char **air_level;
	char *air_en[] = {"GOOD", "GOOD", "POOR", "POOR", "POOR"};
	char *air_s_cn[] = {"优质", "良好", "差 ", "很差", "极差"};
	char *air_t_cn[] = {"優質", "良好", "差 ", "很差", "極差"};

  stat = cling.pm2p5;
	
	if (stat == 0xffff) {
		len = sprintf((char *)string, "NA");	
	} else if (stat > 999) {
		stat = 999;
		len = sprintf((char *)string, "%d", stat);
		b_24_size = 16;
	} else if (stat > 99) {
		len = sprintf((char *)string, "%d", stat);
		b_24_size = 16;
	} else {
		len = sprintf((char *)string, "%d", stat);
	}
	
	if (b_24_size == 24) 
		_render_vertical_character(string, 42, margin, len, b_24_size, FALSE, FALSE);
	else
		_render_vertical_character(string, 46, margin, len, b_24_size, FALSE, FALSE);
	
	b_24_size = 16;
	margin = 0;

	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {	
		air_level = air_en;
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		air_level = air_s_cn;
	} else {
		air_level = air_t_cn;
	}

	if (cling.pm2p5 == 0xffff) {
		len =	sprintf((char *)string, "--");
	} else if (cling.pm2p5 < 50) {
		len = sprintf((char *)string, "%s", air_level[0]);
	} else if (cling.pm2p5 < 150) {
		len = sprintf((char *)string, "%s", air_level[1]);
	} else if (cling.pm2p5 < 300) {
		len = sprintf((char *)string, "%s", air_level[2]);
	} else if (cling.pm2p5 < 500) {
		len = sprintf((char *)string, "%s", air_level[3]);
	} else {
		len = sprintf((char *)string, "%s", air_level[4]);
	}
	
	_render_vertical_character(string, 84, margin, len, b_24_size, FALSE, TRUE);
}

static I8U _render_middle_vertical_hr_core(I8U offset, BOOLEAN b_training_mode)
{
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
				_render_vertical_character(string, offset, margin, len, b_24_size, b_all_hold, FALSE);
			else
				_render_vertical_character(string, offset+5, margin, len, b_24_size, FALSE, FALSE);			
		} else {
			cling.ui.heart_rate_wave_index ++;
			if (cling.ui.heart_rate_wave_index > 2) {
				cling.ui.heart_rate_wave_index = 0;
			}
			switch (cling.ui.heart_rate_wave_index) {
				case 0:
					len = sprintf((char *)string, " - , ,");
					break;
				case 1:
					len = sprintf((char *)string, " , - ,");
					break;
				case 2:
					len = sprintf((char *)string, " , , -");
					break;
				default:
					break;
			}
			b_24_size = 8;
			margin = 3;
			_render_vertical_character(string, offset+5, margin, len, b_24_size, FALSE, FALSE);
			hr_result = 0;
			return hr_result;
		}
	} else {
		N_SPRINTF("[UI] Heart rate - not valid");
		b_24_size = 24;
		margin = 0;
		len = 0;
		string[len++] = ICON24_NO_SKIN_TOUCH_IDX;
		_render_vertical_character(string, offset, margin, len, b_24_size, FALSE, FALSE);
		hr_result = 0;
	}
	
	return hr_result;	
}

static void _middle_render_vertical_heart_rate()
{
	I8U string[32];	
	I8U len = 0;	
	I8U margin = 0;
	I8U b_24_size = 16;
	I8U hr_result = 0;

	hr_result = _render_middle_vertical_hr_core(50, FALSE);
	
	if (hr_result) {
		len = sprintf((char *)string, "BPM");
		_render_vertical_character(string, 80, margin, len, b_24_size, FALSE, TRUE);
	}
}

#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)	
static void _middle_render_vertical_skin_temp()
{
	I8U string[32];	
	I8U len = 0;
  I8U	margin = 2;
	I8U b_24_size = 24;
	I8U integer = 0;
	
  integer = cling.therm.current_temperature/10;
	
	// Temperature value
	len = sprintf((char *)string, "%d", integer);
	_render_vertical_character((I8U *)string, 42, margin, len, b_24_size, FALSE, FALSE);
	
	// - Skin temperature unit - 
	string[0] = ICON24_CELCIUS_IDX;
	len = 1;
	margin = 0;
	_render_vertical_character((I8U *)string, 80, margin, len, b_24_size, FALSE, FALSE);
}
#endif

static void _middle_render_vertical_weather()
{
	I8U string[32];
	I8U len = 0;
	I8U margin = 2;
	I8U b_24_size = 24;
	WEATHER_CTX weather;	

	WEATHER_get_weather(0, &weather);
	
	// - Temperature Range - 
	string[0] = ICON24_WEATHER_RANGE_IDX;
	len = 1;
	_render_vertical_character(string, 55, margin, len, b_24_size, FALSE, FALSE);
	
	// - Low temperature -
	len = sprintf((char *)string, "%d", weather.low_temperature);
	
	if (weather.low_temperature < 0)
		margin = 1;
	else
		margin = 2;
	_render_vertical_character(string, 32, margin, len, b_24_size, FALSE, FALSE);
		
	// - High temperature;
	if (weather.high_temperature < 0)
		margin = 1;
	else
		margin = 2;
	len = sprintf((char *)string, "%d", weather.high_temperature);
	_render_vertical_character(string, 74, margin, len, b_24_size, FALSE, FALSE);
	
	// - Weather temperature unit - 
	string[0] = ICON24_CELCIUS_IDX;
	len = 1;
	_render_vertical_character(string, 104, margin, len, b_24_size, FALSE, FALSE);
}

static void _middle_render_vertical_reminder()
{
	I8U string[32];	
	I8U len = 0;	
	I8U margin = 0;
	I8U b_24_size = 24;

  BOOLEAN b_invalid_alarm = FALSE;
	
  // Rendering clock ...
	if ((cling.reminder.ui_alarm_on) || (cling.reminder.total) || (cling.reminder.b_sleep_total)){
		if (cling.ui.ui_alarm_hh >= 24 || cling.ui.ui_alarm_mm >= 60) {
			b_invalid_alarm = TRUE;
		} else {	
			sprintf((char *)string, " %02d", cling.ui.ui_alarm_hh);
			_render_vertical_time(string, 40, TRUE, FALSE);
			// Render the minute
			sprintf((char *)string, ":%02d", cling.ui.ui_alarm_mm);
			_render_vertical_time(string, 74, FALSE, FALSE);
			N_SPRINTF("[UI] normal ui_hh: %d, ui_mm: %d", cling.ui.ui_alarm_hh, cling.ui.ui_alarm_mm);
		}
	} else {
		len = 0;
		string[len++] = ICON24_NO_SKIN_TOUCH_IDX;
		_render_vertical_character(string, 42, margin, len, b_24_size, FALSE, FALSE);		
	} 
	
	if (b_invalid_alarm) {
		len = sprintf((char *)string, " --");
		_render_vertical_time(string, 40, TRUE, FALSE);
		len = sprintf((char *)string, ":--");
		_render_vertical_time(string, 74, TRUE, FALSE);
	}
}

static void _vertical_core_run_distance(I32U stat)
{
	I8U string[32];	
	I8U len = 0;
	I8U margin = 0;
	I8U b_24_size = 16;
	I32U v_1000 = 0, v_100 = 0;

	// Convert to miles
	if (cling.user_data.profile.metric_distance) {
		stat *= 10;
		stat >>= 4;
	}
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "RUN");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "路跑");
	} else {
		len = sprintf((char *)string, "路跑");
	}
	_render_vertical_character(string, 28, margin, len, b_24_size, FALSE, TRUE);
	
	b_24_size = 24;
	margin = 2;
	
	if (stat > 99999) {
		v_1000 = 99;
		len = sprintf((char *)string, "%d", v_1000);
	} else if (stat > 9999) {
		v_1000 = stat / 1000;
		len = sprintf((char *)string, "%d", v_1000);
	} else if (stat > 999) {
		v_1000 = stat / 1000;
		v_100 = stat - (v_1000 * 1000);
		v_100 /= 100;
		len = sprintf((char *)string, "%d.%d", v_1000, v_100);
		margin = 1;
	} else {
		v_100 = stat / 100;
		len = sprintf((char *)string, "0.%d", v_100);
		margin = 1;
	}
	
	_render_vertical_character(string, 67, margin, len, b_24_size, FALSE, FALSE);

	b_24_size = 16;
	margin = 0;
	
	if (cling.user_data.profile.metric_distance) {
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			len = sprintf((char *)string, "MILE");
		} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
			len = sprintf((char *)string, "英里");
		} else {
			len = sprintf((char *)string, "英裏");
		}
	} else {
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			len = sprintf((char *)string, "KM");
		} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
			len = sprintf((char *)string, "公里");
		} else {
			len = sprintf((char *)string, "公裏");
		}
	}
	
	_render_vertical_character(string, 112, margin, len, b_24_size, FALSE, TRUE);	
}

static void _middle_render_vertical_running_distance()
{
	_vertical_core_run_distance(cling.run_stat.distance);
}

static void _middle_render_vertical_training_distance()
{
	I8U string[32];	
	I8U len = 0;
	I8U margin = 0;
	I8U b_24_size = 16;
	I32U v_1000 = 0, v_100 = 0;
	I32U stat = 0;

	stat = cling.train_stat.distance;

	if (cling.ui.clock_sec_blinking) {	
		cling.ui.clock_sec_blinking = FALSE;
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			len = sprintf((char *)string, "RUN");
		} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
			len = sprintf((char *)string, "里程");
		} else {
			len = sprintf((char *)string, "裏程");
		}
		_render_vertical_character(string, 28, margin, len, b_24_size, FALSE, TRUE);
	} else {
		cling.ui.clock_sec_blinking = TRUE;
	}

	// Convert to miles
	if (cling.user_data.profile.metric_distance) {
		stat *= 10;
		stat >>= 4;
	}
	
	b_24_size = 24;
	margin = 2;
	
	if (stat > 99999) {
		v_1000 = 99;
		len = sprintf((char *)string, "%d", v_1000);
	} else if (stat > 9999) {
		v_1000 = stat / 1000;
		len = sprintf((char *)string, "%d", v_1000);
	} else if (stat > 999) {
		v_1000 = stat / 1000;
		v_100 = stat - (v_1000 * 1000);
		v_100 /= 100;
		len = sprintf((char *)string, "%d.%d", v_1000, v_100);
		margin = 1;
	} else {
		v_100 = stat / 100;
		len = sprintf((char *)string, "0.%d", v_100);
		margin = 1;
	}
	
	_render_vertical_character(string, 64, margin, len, b_24_size, TRUE, FALSE);

	b_24_size = 16;
	margin = 0;
	
	if (cling.user_data.profile.metric_distance) {
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			len = sprintf((char *)string, "MILE");
		} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
			len = sprintf((char *)string, "英里");
		} else {
			len = sprintf((char *)string, "英裏");
		}
	} else {
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			len = sprintf((char *)string, "KM");
		} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
			len = sprintf((char *)string, "公里");
		} else {
			len = sprintf((char *)string, "公裏");
		}
	}
	
	_render_vertical_character(string, 112, margin, len, b_24_size, FALSE, TRUE);		
}

static void _middle_render_vertical_running_time()
{
	I8U string[32];
	I8U hour=0, min=0, sec=0;
	
	min = cling.run_stat.time_min;
	sec = cling.run_stat.time_sec;
	hour = min/60;
	min = min - hour*60;

	// Render the hour
	if (hour > 9) hour = 9;
	sprintf((char *)string, " +%d", hour);
	_render_vertical_time(string, 52, TRUE, FALSE);
		
	// Render the minute
	sprintf((char *)string, ":%02d", min);
	_render_vertical_time(string, 78, FALSE, FALSE);
	
	// Render the second
	sprintf((char *)string, ":%02d", sec);
	_render_vertical_time(string, 104, FALSE, FALSE);	
}

static void _middle_render_vertical_training_time()
{
	I8U string[32];
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

	if (hour) {
		// Render the hour
		if (hour > 9) hour = 9;
		sprintf((char *)string, " +%d", hour);
		_render_vertical_time(string, 52, TRUE, FALSE);
		// Render the minute
		sprintf((char *)string, ":%02d", min);
		_render_vertical_time(string, 78, TRUE, FALSE);
		// Render the second
		sprintf((char *)string, ":%02d", sec);
		_render_vertical_time(string, 104, TRUE, FALSE);
	} else {
	
		// Render the minute
		if (min > 9) {
			sprintf((char *)string, " %02d", min);
		} else {
			sprintf((char *)string, " +%d", min);
		}
		_render_vertical_time(string, 75, TRUE, FALSE);

		// Render the second
		sprintf((char *)string, ":%02d", sec);
		_render_vertical_time(string, 104, TRUE, FALSE);	
	}
}

static void _middle_render_vertical_running_pace()
{
	I8U string[32];
	I8U len = 0;
	I8U margin = 0;
	I8U b_24_size = 16;
	I32U pace = cling.run_stat.time_min; // Rounding 
	I32U min, sec;
	
	if (cling.run_stat.distance) {
		pace = cling.run_stat.time_min*60;
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
	_render_vertical_time(string, 76, FALSE, FALSE);

	// Render the minute
	sprintf((char *)string, "%02d*", sec);
	_render_vertical_time(string, 104, FALSE, FALSE);

	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "AVG");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "平均");
	} else {
		len = sprintf((char *)string, "平均");
	}
	
	_render_vertical_character(string, 28, margin, len, b_24_size, FALSE, TRUE);
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "PACE");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "配速 ");
	} else {
		len = sprintf((char *)string, "配速 ");
	}
	
	_render_vertical_character(string, 46, margin, len, b_24_size, FALSE, TRUE);
}

static void _middle_render_vertical_running_hr()
{
	I8U string[32];
	I8U len = 0;
	I8U margin = 0;
	I8U b_24_size = 16;
	I32U hr = cling.run_stat.accu_heart_rate;
	
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
	
	len = sprintf((char *)string, "%d", hr);
				
	if (b_24_size == 24) 
		_render_vertical_character(string, 86, margin, len, b_24_size, FALSE, FALSE);
	else
		_render_vertical_character(string, 90, margin, len, b_24_size, FALSE, FALSE);
		
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "AVG");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "平均");
	} else {
		len = sprintf((char *)string, "平均");
	}
	
	_render_vertical_character(string, 28, margin, len, b_24_size, FALSE, TRUE);
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "HR");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "心率");
	} else {
		len = sprintf((char *)string, "心率");
	}
	
	_render_vertical_character(string, 46, margin, len, b_24_size, FALSE, TRUE);
}

static void _middle_render_vertical_running_calories()
{
	I8U string1[32];
	I8U string2[32];
  I8U len1 = 0, len2 = 0;	
	I8U  margin = 0;
	I8U b_24_size = 16;
	I32U stat = 0;	
	I32U v_10000 = 0, v_1000 = 0, v_100 = 0;

	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len1 = sprintf((char *)string1, "BURN");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len1 = sprintf((char *)string1, "热量");
	} else {
		len1 = sprintf((char *)string1, "熱量");
	}
	
	_render_vertical_character(string1, 28, margin, len1, b_24_size, FALSE, TRUE);

  b_24_size = 24;	
	margin = 2;
	stat = cling.run_stat.calories;
	
	if (stat > 99999) {
		v_10000 = 9;
		v_1000 = 9;
		len1 = sprintf((char *)string1, "%d%d", v_10000, v_1000);
	} else if (stat > 9999) {
		v_10000 = stat / 10000;
		v_1000 = stat - (v_10000 * 10000);
		v_1000 /= 1000;
		len1 = sprintf((char *)string1, "%d%d", v_10000, v_1000);
	} else if (stat > 999) {
		v_1000 = stat / 1000;
		v_100 = stat - (v_1000 * 1000);
		v_100 /= 100;
		len1 = sprintf((char *)string1, "%d.%d", v_1000, v_100);
		margin = 1;
	} else if (stat > 99) {
		len1 = sprintf((char *)string1, "%d", stat);
		b_24_size = 16;
	} else {
		len1 = sprintf((char *)string1, "%d", stat);
	}
	
	if (b_24_size == 24) 
		_render_vertical_character(string1, 68, margin, len1, b_24_size, FALSE, FALSE);
	else
		_render_vertical_character(string1, 72, margin, len1, b_24_size, FALSE, FALSE);

	b_24_size = 16;
	margin = 0;
	
	if (stat > 999) {
		b_24_size = 16;
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			len1 = sprintf((char *)string1, "K");
			len2 = sprintf((char *)string1, "CAL");
		} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
			len1 = sprintf((char *)string1, "千 ");
			len2 = sprintf((char *)string2, "大卡");			
		} else {
			len1 = sprintf((char *)string1, "千 ");
			len2 = sprintf((char *)string2, "大卡");			
		}
	} else {		
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			len1 = sprintf((char *)string1, "CAL");
		} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
			len1 = sprintf((char *)string1, "大卡");
		} else {
			len1 = sprintf((char *)string1, "大卡");
		}
	}
	
	if (len2) {
	  _render_vertical_character(string1, 96, margin, len1, b_24_size, FALSE, TRUE);
	  _render_vertical_character(string2, 112, margin, len2, b_24_size, FALSE, TRUE);		
	} else {
	  _render_vertical_character(string1, 112, margin, len1, b_24_size, FALSE, TRUE);		
	}
}

static void _middle_render_vertical_running_cadence()
{
	I8U string[32];	
	I8U len = 0;
	I8U margin = 0;
	I8U b_24_size = 16;	
	I32U stat = 0;

	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "RATE");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "步频");
	} else {
		len = sprintf((char *)string, "步頻");
	}
	
	_render_vertical_character(string, 28, margin, len, b_24_size, FALSE, TRUE);
	
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
	
	len = sprintf((char *)string, "%d", stat);
				
	if (b_24_size == 24) 
		_render_vertical_character(string, 68, margin, len, b_24_size, FALSE, FALSE);
	else
		_render_vertical_character(string, 72, margin, len, b_24_size, FALSE, FALSE);
	
	b_24_size = 16;
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "SPM");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "步分");
	} else {
		len = sprintf((char *)string, "步分");
	}
	_render_vertical_character(string, 112, margin, len, b_24_size, FALSE, TRUE);
}

static void _middle_render_vertical_running_stride()
{
	I8U string[32];	
	I8U len = 0;
	I8U margin = 0;
	I8U b_24_size = 16;	
	I32U stat = 0;
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "STR.");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "步幅");
	} else {
		len = sprintf((char *)string, "步幅");
	}
	
	_render_vertical_character(string, 28, margin, len, b_24_size, FALSE, TRUE);
	
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
	
	len = sprintf((char *)string, "%d", stat);
				
	if (b_24_size == 24) 
		_render_vertical_character(string, 68, margin, len, b_24_size, FALSE, FALSE);
	else
		_render_vertical_character(string, 72, margin, len, b_24_size, FALSE, FALSE);
	
	b_24_size = 16;
	margin = 0;
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		if (cling.user_data.profile.metric_distance) {
			len = sprintf((char *)string, "IN");
		} else {
		  len = sprintf((char *)string, "CM");
		}
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		if (cling.user_data.profile.metric_distance) {
		  len = sprintf((char *)string, "英寸");			
		} else {
		  len = sprintf((char *)string, "厘米");			
		}
	} else {
		if (cling.user_data.profile.metric_distance) {
		  len = sprintf((char *)string, "英寸");			
		} else {
		  len = sprintf((char *)string, "厘米");			
		}
	}
	
	_render_vertical_character(string, 112, margin, len, b_24_size, FALSE, TRUE);
}

#ifdef _CLINGBAND_PACE_MODEL_
static void _middle_render_vertical_running_stop_analysis()
{
	I8U string[32];	
	I8U len = 0;
	I8U margin = 0;
	I8U b_24_size = 16;	
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "INFO");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "结束");
	} else {
		len = sprintf((char *)string, "結束");
	}
	
	_render_vertical_character(string, 50, margin, len, b_24_size, FALSE, TRUE);
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "END");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "分析");
	} else {
		len = sprintf((char *)string, "分析");
	}
	
	_render_vertical_character(string, 70, margin, len, b_24_size, FALSE, TRUE);
}
#endif

static BOOLEAN _middle_render_vertical_core_ready()
{
	I8U string[32];	
  I32U t_curr = CLK_get_system_time();
  BOOLEAN b_ready_finished = FALSE;
	
	if (cling.ui.run_ready_index == 0) {
		if (t_curr > (cling.ui.running_time_stamp + 500)) {
			cling.ui.run_ready_index = 1;
		  cling.ui.running_time_stamp = t_curr;
			Y_SPRINTF("[UI] Ready go 3 :%d :%d",t_curr, cling.ui.running_time_stamp)
		}
		sprintf((char *)string, "3");
	} else if(cling.ui.run_ready_index == 1) {
		if (t_curr > (cling.ui.running_time_stamp + 800)) {
			cling.ui.run_ready_index = 2;
		  cling.ui.running_time_stamp = t_curr;	
			Y_SPRINTF("[UI] Ready go 2 :%d :%d",t_curr, cling.ui.running_time_stamp)			
		}
		sprintf((char *)string, "2");
	} else if(cling.ui.run_ready_index == 2) {
		if (t_curr > (cling.ui.running_time_stamp + 800)) {
			cling.ui.run_ready_index = 3;
		  cling.ui.running_time_stamp = t_curr;	
			Y_SPRINTF("[UI] Ready go 1 :%d :%d",t_curr, cling.ui.running_time_stamp)			
		}	
		sprintf((char *)string, "1");
	} else if(cling.ui.run_ready_index == 3) {	
		if (t_curr > (cling.ui.running_time_stamp + 500)) {
			cling.ui.run_ready_index = 0;
      b_ready_finished = TRUE;
			Y_SPRINTF("[UI] Ready go start :%d :%d",t_curr, cling.ui.running_time_stamp)			
		}
		sprintf((char *)string, "G O");		
	} else {
		cling.ui.run_ready_index = 0;
	}

	_render_vertical_time(string, 60, TRUE, TRUE);
	
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
	I8U string[32];	
	I8U len = 0;
	I8U margin = 2;
	I8U b_24_size = 24;	
	I32U min, sec;
	
	min = cling.run_stat.last_10sec_pace_min;
	sec = cling.run_stat.last_10sec_pace_sec;
	
	if (min > 9)
		sprintf((char *)string, "%02d/", min);
	else
		sprintf((char *)string, "+%d/", min);
	_render_vertical_time(string, 76, TRUE, FALSE);

	// Render the minute
	sprintf((char *)string, "%02d*", sec);
	_render_vertical_time(string, 104, TRUE, FALSE);

	b_24_size = 16;
	margin = 0;
	
	if (cling.ui.clock_sec_blinking) {	
		cling.ui.clock_sec_blinking = FALSE;	
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			len = sprintf((char *)string, "PACE");
		} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
			len = sprintf((char *)string, "配速 ");
		} else {
			len = sprintf((char *)string, "配速 ");
		}
		_render_vertical_character(string, 28, margin, len, b_24_size, FALSE, TRUE);
	} else {
		cling.ui.clock_sec_blinking = TRUE;			
	}
}

static void _middle_render_vertical_training_hr()
{
	I8U string[32];	
	I8U len = 0;		
	I8U margin = 0;
	I8U b_24_size = 16;
	I8U hr_result = 0;
	I8U i;		
	I32U hr_perc = 0;
	I8U *p0, *p1, *p2, *p3;
	
	if (cling.ui.clock_sec_blinking) {	
		cling.ui.clock_sec_blinking = FALSE;		
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			len = sprintf((char *)string, "HR");
		} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
			len = sprintf((char *)string, "心率");
		} else {
			len = sprintf((char *)string, "心率");
		}
		_render_vertical_character(string, 28, margin, len, b_24_size, FALSE, TRUE);
	} else {
		cling.ui.clock_sec_blinking = TRUE;	
	}

	hr_result = _render_middle_vertical_hr_core(60, TRUE);
	
  if (hr_result) {
	  hr_perc = (hr_result * 100)/(220-cling.user_data.profile.age);	
	  if (hr_perc > 98)
		  hr_perc = 98;
		
		b_24_size = 16;
		margin = 2;
	  len = sprintf((char *)string, "%d%%", hr_perc);
	  _render_vertical_character(string, 94, margin, len, b_24_size, FALSE, TRUE);
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
	I8U string[32];	
	I8U len = 0;		
	I8U margin = 0;
	I8U b_24_size = 16;
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "RUN");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "结束");
	} else {
		len = sprintf((char *)string, "結束");
	}
	
	_render_vertical_character(string, 50, margin, len, b_24_size, FALSE, TRUE);
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "STOP");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
#ifdef _CLINGBAND_PACE_MODEL_		
		len = sprintf((char *)string, "跑步");
#else 		
		len = sprintf((char *)string, "运动");
#endif		
	} else {
#ifdef _CLINGBAND_PACE_MODEL_		
		len = sprintf((char *)string, "跑步");
#else 		
		len = sprintf((char *)string, "運動");
#endif	
	}
	
	_render_vertical_character(string, 70, margin, len, b_24_size, FALSE, TRUE);
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _middle_render_vertical_cycling_outdoor_run_start()
{
	I8U string[32];	
	I8U len = 0;		
	I8U margin = 0;
	I8U b_24_size = 16;
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "RUN");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "开始 ");
	} else {
		len = sprintf((char *)string, "開始 ");
	}
	
	_render_vertical_character(string, 50, margin, len, b_24_size, FALSE, TRUE);
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "NOW");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "骑行");
	} else {
		len = sprintf((char *)string, "騎行");
	}
	
	_render_vertical_character(string, 70, margin, len, b_24_size, FALSE, TRUE);	
}

static void _middle_render_vertical_cycling_outdoor_run_stop()
{
	I8U string[32];	
	I8U len = 0;		
	I8U margin = 0;
	I8U b_24_size = 16;
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "RUN");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "结束");
	} else {
		len = sprintf((char *)string, "結束");
	}
	_render_vertical_character(string, 50, margin, len, b_24_size, FALSE, TRUE);
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		len = sprintf((char *)string, "STOP");
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		len = sprintf((char *)string, "骑行");
	} else {
		len = sprintf((char *)string, "騎行");
	}
	
	_render_vertical_character(string, 70, margin, len, b_24_size, FALSE, TRUE);
}

static void _middle_render_vertical_cycling_outdoor_distance()
{
	I8U string[32];	
	I8U len = 0;		
	I8U margin = 2;
	I8U b_24_size = 24;
  BOOLEAN b_ble_connected = FALSE;

	if (BTLE_is_connected())
		b_ble_connected = TRUE;
	
	if (b_ble_connected) {
		_vertical_core_run_distance(cling.run_stat.distance);
	} else {
		len = 0;
		string[len++] = ICON24_NO_SKIN_TOUCH_IDX;
		_render_vertical_character(string, 42, margin, len, b_24_size, FALSE, FALSE);				
	}
}

static void _middle_render_vertical_cycling_outdoor_speed()
{
	I8U string[32];	
	I8U len = 0;		
	I8U margin = 2;
	I8U b_24_size = 24;
  BOOLEAN b_ble_connected = FALSE;

	if (BTLE_is_connected())
		b_ble_connected = TRUE;
	
	if (b_ble_connected) {
		_vertical_core_run_distance(17);
	} else {
		len = 0;
		string[len++] = ICON24_NO_SKIN_TOUCH_IDX;
		_render_vertical_character(string, 42, margin, len, b_24_size, FALSE, FALSE);				
	}
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_
static void _middle_render_vertical_carousel_1()
{
	I8U string[32];	
	I8U len = 0;		
	I8U margin = 0;
	I8U b_24_size = 24;
	
	string[0] = ICON24_RUNNING_MODE_IDX;
	len = 1;
	_render_vertical_character(string, 4, margin, len, b_24_size, FALSE, FALSE);
	
	string[0] = ICON24_CYCLING_OUTDOOR_MODE_IDX;
	len = 1;
	_render_vertical_character(string, (4+48), margin, len, b_24_size, FALSE, FALSE);
	
	string[0] = ICON24_WORKOUT_MODE_IDX;
	len = 1;
	_render_vertical_character(string, (4+96), margin, len, b_24_size, FALSE, FALSE);
}
#endif

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
static void _middle_render_vertical_carousel_2()
{
	I8U string[32];	
	I8U len = 0;		
	I8U margin = 0;
	I8U b_24_size = 24;
	
	string[0] = ICON24_MUSIC_IDX;
	len = 1;
	_render_vertical_character(string, 4, margin, len, b_24_size, FALSE, FALSE);
	
	string[0] = ICON24_STOPWATCH_IDX;
	len = 1;
	_render_vertical_character(string, (4+48), margin, len, b_24_size, FALSE, FALSE);
	
	string[0] = ICON24_MESSAGE_IDX;
	len = 1;
	_render_vertical_character(string, (4+96), margin, len, b_24_size, FALSE, FALSE);
}
#endif

#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	
static void _middle_render_vertical_carousel_2()
{
	I8U string[32];	
	I8U len = 0;		
	I8U margin = 0;
	I8U b_24_size = 24;
	
	string[0] = ICON24_MESSAGE_IDX;
	len = 1;
	_render_vertical_character(string, 4, margin, len, b_24_size, FALSE, FALSE);
	
	string[0] = ICON24_STOPWATCH_IDX;
	len = 1;
	_render_vertical_character(string, (4+48), margin, len, b_24_size, FALSE, FALSE);
	
	string[0] = ICON24_WEATHER_IDX;
	len = 1;
	_render_vertical_character(string, (4+96), margin, len, b_24_size, FALSE, FALSE);
}
#endif

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
static void _middle_render_vertical_carousel_3()
{
	I8U string[32];	
	I8U len = 0;		
	I8U margin = 0;
	I8U b_24_size = 24;
	
	string[0] = ICON24_WEATHER_IDX;
	len = 1;
	_render_vertical_character(string, 4, margin, len, b_24_size, FALSE, FALSE);
	
	string[0] = ICON24_PM2P5_IDX;
	len = 1;
	_render_vertical_character(string, (4+48), margin, len, b_24_size, FALSE, FALSE);
	
	string[0] = ICON24_NORMAL_ALARM_CLOCK_IDX;
	len = 1;
	_render_vertical_character(string, (4+96), margin, len, b_24_size, FALSE, FALSE);
}
#endif

#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	
static void _middle_render_vertical_carousel_3()
{
	I8U string[32];	
	I8U len = 0;		
	I8U margin = 0;
	I8U b_24_size = 24;
	
	string[0] = ICON24_PM2P5_IDX;
	len = 1;
	_render_vertical_character(string, 4, margin, len, b_24_size, FALSE, FALSE);
	
	string[0] = ICON24_NORMAL_ALARM_CLOCK_IDX;
	len = 1;
	_render_vertical_character(string, (4+48), margin, len, b_24_size, FALSE, FALSE);
	
	string[0] = ICON24_SETTING_IDX;
	len = 1;
	_render_vertical_character(string, (4+96), margin, len, b_24_size, FALSE, FALSE);
}
#endif

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
static void _middle_render_vertical_carousel_4()
{
	I8U string[32];	
	I8U len = 0;		
	I8U margin = 0;
	I8U b_24_size = 24;
	
	string[0] = ICON24_BATT_POWER_IDX;
	len = 1;
	_render_vertical_character(string, 4, margin, len, b_24_size, FALSE, FALSE);
	
	string[0] = ICON24_PHONE_FINDER_IDX;
	len = 1;
	_render_vertical_character(string, (4+48), margin, len, b_24_size, FALSE, FALSE);
	
	string[0] = ICON24_SETTING_IDX;
	len = 1;
	_render_vertical_character(string, (4+96), margin, len, b_24_size, FALSE, FALSE);
}
#endif

static void _bottom_render_vertical_home()
{
	I8U data_buf[256];
	I8U string[32];
	I8U line_len;
	BOOLEAN b_batt_charging = FALSE;
	char *week_en[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
	char *week_cn_s[] = {"周一", "周二", "周三", "周四", "周五", "周六", "周日"};
	char *week_cn_t[] = {"周壹", "周二", "周三", "周四", "周五", "周六", "周日"};
	
	if (BATT_is_charging()) 
		b_batt_charging = TRUE;
	
	memset(data_buf, 0, 256);
	
	if (b_batt_charging) {
	  sprintf((char *)string, "%d%%", cling.system.mcu_reg[REGISTER_MCU_BATTERY]);	
	} else {
		sprintf((char *)string, "%d/%02d",cling.time.local.month, cling.time.local.day);	
	}
	
	line_len = FONT_load_characters(data_buf, (char *)string, 8, 128, FALSE);
	_vertical_centerize(data_buf, data_buf+32, data_buf+64, line_len);
	_rotate_270_degree(data_buf, cling.ui.p_oled_up+384+120);
	
	memset(data_buf, 0, 256);	

	// Render the dow	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
		sprintf((char *)string, "%s", week_en[cling.time.local.dow]);
		line_len = FONT_load_characters(data_buf, (char *)string, 16, 128, FALSE);
	} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
		sprintf((char *)string, "%s", week_cn_s[cling.time.local.dow]);
		line_len = FONT_load_characters(data_buf, (char *)string, 16, 128, FALSE);
	} else {
		sprintf((char *)string, "%s", week_cn_t[cling.time.local.dow]);
		line_len = FONT_load_characters(data_buf, (char *)string, 16, 128, FALSE);
	}
	
	_vertical_centerize(data_buf, data_buf+128, data_buf+128+64, line_len);
	_rotate_270_degree(data_buf, cling.ui.p_oled_up+384+96+4);
	_rotate_270_degree(data_buf+128, cling.ui.p_oled_up+384+104+4);	
}

static void _render_vertical_small_clock(I8U offset)
{
	I8U data_buf[128];
	I8U string[32];
	I8U line_len;
	
	memset(data_buf, 0, 128);
	
	if (cling.ui.clock_sec_blinking) {
		cling.ui.clock_sec_blinking = FALSE;
		sprintf((char *)string, "%d:%02d",cling.time.local.hour, cling.time.local.minute);
	} else {
		cling.ui.clock_sec_blinking = TRUE;
		sprintf((char *)string, "%d %02d",cling.time.local.hour, cling.time.local.minute);		
	}
	
	line_len = FONT_load_characters(data_buf, (char *)string, 8, 128, FALSE);
	_vertical_centerize(data_buf, data_buf+32, data_buf+64, line_len);
	_rotate_270_degree(data_buf, cling.ui.p_oled_up+384+offset);
}

static void _bottom_render_vertical_small_clock()
{
  _render_vertical_small_clock(120);
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _bottom_render_vertical_more()
{
	I8U *in = cling.ui.p_oled_up+128+128+120;

	_render_one_icon_8(ICON8_MORE_LEN, in, asset_content+ICON8_MORE_POS);
}

static void _bottom_render_vertical_delta_data_backward()
{
	I8U data_buf[128];
	I8U string[32];
	I8U line_len;
	SYSTIME_CTX delta;
	
	memset(data_buf, 0, 128);

	RTC_get_delta_clock_backward(&delta, cling.ui.vertical_index);	
	sprintf((char *)string, "%d/%02d", delta.month, delta.day);
	line_len = FONT_load_characters(data_buf, (char *)string, 8, 128, FALSE);
	
	_vertical_centerize(data_buf, data_buf+32, data_buf+64, line_len);
	_rotate_270_degree(data_buf, cling.ui.p_oled_up+384+112);
}

static void _bottom_render_vertical_tracker()
{
	if (!cling.ui.vertical_index) {
		if (cling.ui.b_detail_page) {
			_render_vertical_small_clock(112);
		} else {
			_render_vertical_small_clock(120);
		}
	} else {
		_bottom_render_vertical_delta_data_backward();
	}
	
	if (cling.ui.b_detail_page) {
		_bottom_render_vertical_more();	
	}	
}

static void _bottom_render_vertiacl_reminder()
{
	I8U buf1[256];
  I8U *buf2, *buf3;
	I8U string[32];
	I8U line_len;

	memset(buf1, 0, 256);

	if (cling.reminder.ui_alarm_on)
		return;

	if (!cling.ui.b_detail_page) 
    return;		
	
	sprintf((char *)string, "%02d", cling.ui.vertical_index);
	line_len = FONT_load_characters(buf1, (char *)string, 16, 128, FALSE);	

	buf2 = buf1 + 128;
	buf3 = buf2 + 32;
	
	_vertical_centerize(buf1, buf2, buf3, line_len);
	_rotate_270_degree(buf1, cling.ui.p_oled_up+384+101);
	_rotate_270_degree(buf2, cling.ui.p_oled_up+384+101+8);
	
	_bottom_render_vertical_more();	
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_
static void _bottom_render_vertical_run_go()
{
	I8U data_buf[256];
	I8U line_len;
	I8U i;
	I8U *p0, *p1;
	
	memset(data_buf, 0x00, 32);
	memset(data_buf+128, 0x00, 32);
	
	FONT_load_characters(data_buf+7, "G", 16, 128, FALSE);
	FONT_load_characters(data_buf+18, "O", 16, 128, FALSE);
	
  p0 = data_buf,
	p1 = p0+128;
	
	for (i=0;i<32;i++) {
	 *(p0+i) = ~(*(p0+i));
	 *(p1+i) = ~(*(p1+i));	
	}
	
	line_len = 32;
	_vertical_centerize(data_buf, data_buf+128, data_buf+128+64, line_len);
	_rotate_270_degree(data_buf, cling.ui.p_oled_up+384+112);
	_rotate_270_degree(data_buf+128, cling.ui.p_oled_up+384+112+8);	
}

static void _bottom_render_vertical_run_ok()
{
	I8U data_buf[256];
	I8U line_len;
	I8U i;
	I8U *p0, *p1;
	
	memset(data_buf, 0x00, 32);
	memset(data_buf+128, 0x00, 32);
	
	FONT_load_characters(data_buf+6, "O", 16, 128, FALSE);
	FONT_load_characters(data_buf+16, "K", 16, 128, FALSE);
	
  p0 = data_buf,
	p1 = p0+128;
	
	for (i=0;i<32;i++) {
	 *(p0+i) = ~(*(p0+i));
	 *(p1+i) = ~(*(p1+i));	
	}
	
	line_len = 32;
	_vertical_centerize(data_buf, data_buf+128, data_buf+128+64, line_len);
	_rotate_270_degree(data_buf, cling.ui.p_oled_up+384+112);
	_rotate_270_degree(data_buf+128, cling.ui.p_oled_up+384+112+8);	
}
#endif

#ifdef _CLINGBAND_PACE_MODEL_
static void _bottom_render_vertical_button_hold()
{
	I8U string[32];
	I8U data_buf[256];
	I8U line_len;
	I8U i;
	I8U *p0, *p1;
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
	  sprintf((char *)string, "HOLD");
  } else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {	
		sprintf((char *)string, "长按");
	} else {
	  sprintf((char *)string, "長按");	
	}

	line_len = FONT_load_characters(data_buf, (char *)string, 16, 128, FALSE);

  p0 = data_buf,
	p1 = p0+128;
	
	for (i=0;i<32;i++) {
	 *(p0+i) = ~(*(p0+i));
	 *(p1+i) = ~(*(p1+i));	
	}
	
	_vertical_centerize(data_buf, data_buf+128, data_buf+128+64, line_len);
	_rotate_270_degree(data_buf, cling.ui.p_oled_up+384+112);
	_rotate_270_degree(data_buf+128, cling.ui.p_oled_up+384+112+8);			
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_
static void _bottom_render_vertical_cycling_outdoor_distance()
{
	I8U string1[32];
	I8U string2[32];	
	I8U len1 = 0, len2 = 0;
	I8U margin = 0;
	I8U b_24_size = 16;
  BOOLEAN b_ble_connected = FALSE;
 	
	if (BTLE_is_connected())
		b_ble_connected = TRUE;
	
	if (b_ble_connected) {
		if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {
			len1 = sprintf((char *)string1, "KM");	
		} else if (cling.ui.language_type == LANGUAGE_TYPE_SIMPLIFIED_CHINESE) {
			len1 = sprintf((char *)string1, "公里");
		} else {
			len1 = sprintf((char *)string1, "公裏");
		}
	} else {
		len1 = sprintf((char *)string1, "无 ");
		len2 = sprintf((char *)string2, "蓝牙");			
	} 
		
	if (len2) {
	  _render_vertical_character(string1, 92,  margin, len1, b_24_size, FALSE, TRUE);
		_render_vertical_character(string2, 112, margin, len2, b_24_size, FALSE, TRUE);
	} else {
		_render_vertical_character(string1, 112, margin, len1, b_24_size, FALSE, TRUE);
	}
}

static void _bottom_render_vertical_cycling_outdoor_speed()
{
	I8U data_buf[256];
	I8U string[32];
	I8U line_len;
  BOOLEAN b_ble_connected = FALSE;
	
	if (BTLE_is_connected())
		b_ble_connected = TRUE;
	
	memset(data_buf, 0, 256);

  if (b_ble_connected) {	
		sprintf((char *)string, "KM/H");
		line_len = FONT_load_characters(data_buf, (char *)string, 8, 128, FALSE);
		_vertical_centerize(data_buf, data_buf+32, data_buf+64, line_len);
		_rotate_270_degree(data_buf, cling.ui.p_oled_up+384+120);	
	} else {
		sprintf((char *)string, "无蓝牙 ");	
    line_len = FONT_load_characters(data_buf, (char *)string, 16, 128, FALSE);		
	  _vertical_centerize(data_buf, data_buf+128, data_buf+128+64, line_len);
	  _rotate_270_degree(data_buf, cling.ui.p_oled_up+384+112);
	  _rotate_270_degree(data_buf+128, cling.ui.p_oled_up+384+112+8);				
	}
}
#endif

static void _frame_display_home(BOOLEAN b_render)
{
	_core_frame_display(UI_FRAME_PAGE_HOME_CLOCK, b_render);
}

static void _frame_display_system(I8U index, BOOLEAN b_render)
{
	switch (index) {
		case UI_DISPLAY_SYSTEM_RESTART:
			_core_frame_display(UI_FRAME_PAGE_PACE_LOGO, b_render);
			break;
		case UI_DISPLAY_SYSTEM_OTA:
			_core_frame_display(UI_FRAME_PAGE_OTA, b_render);
			break;
		case UI_DISPLAY_SYSTEM_UNAUTHORIZED:
      _core_frame_display(UI_FRAME_PAGE_UNAUTHORIZED, b_render);
			break;		
		case UI_DISPLAY_SYSTEM_LINKING:
			_core_frame_display(UI_FRAME_PAGE_LINKING, b_render);
			break;
		case UI_DISPLAY_SYSTEM_BATT_POWER:		
			_core_frame_display(UI_FRAME_PAGE_BATT_POWER, b_render);
			break;	
#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)			
		case UI_DISPLAY_SYSTEM_BATT_POWER_2:		
			_core_frame_display(UI_FRAME_PAGE_BATT_POWER_2, b_render);
			break;	
#endif		
		default:
			break;
	}
}

static void _frame_display_tracker(I8U index, BOOLEAN b_render)
{
	switch (index) {
		case UI_DISPLAY_TRACKER_STEP:
			_core_frame_display(UI_FRAME_PAGE_STEPS, b_render);
			break;
		case UI_DISPLAY_TRACKER_DISTANCE:
			_core_frame_display(UI_FRAME_PAGE_DISTANCE, b_render);
			break;
		case UI_DISPLAY_TRACKER_CALORIES:
			_core_frame_display(UI_FRAME_PAGE_CALORIES, b_render);
			break;
		case UI_DISPLAY_TRACKER_ACTIVE_TIME:
			_core_frame_display(UI_FRAME_PAGE_ACTIVE_TIME, b_render);
			break;	
#ifdef _CLINGBAND_UV_MODEL_		
		case UI_DISPLAY_TRACKER_UV_IDX:
			_core_frame_display(UI_FRAME_PAGE_UV_IDX, b_render);
			break;			
#endif		
		default:
			break;
	}
}

static void _frame_display_smart(I8U index, BOOLEAN b_render)
{
	switch (index) {
		case UI_DISPLAY_SMART_WEATHER:
		  _core_frame_display(UI_FRAME_PAGE_WEATHER, b_render);
			break;
		case UI_DISPLAY_SMART_ALARM_CLOCK_REMINDER:
			_core_frame_display(UI_FRAME_PAGE_ALARM_CLOCK_REMINDER, b_render);
			break;
#ifndef _CLINGBAND_PACE_MODEL_		
		case UI_DISPLAY_SMART_ALARM_CLOCK_DETAIL:
			_core_frame_display(UI_FRAME_PAGE_ALARM_CLOCK_DETAIL, b_render);
			break;		
#endif		
		case UI_DISPLAY_SMART_IDLE_ALERT:
			_core_frame_display(UI_FRAME_PAGE_IDLE_ALERT, b_render);
			break;		
		case UI_DISPLAY_SMART_PM2P5:
			_core_frame_display(UI_FRAME_PAGE_PM2P5, b_render);
		  break;
		case UI_DISPLAY_SMART_HEART_RATE_ALERT:
			_core_frame_display(UI_FRAME_PAGE_HEART_RATE_ALERT, b_render);
		  break;
		case UI_DISPLAY_SMART_STEP_10K_ALERT:
			_core_frame_display(UI_FRAME_PAGE_STEP_10K_ALERT, b_render);
		  break;				
		case UI_DISPLAY_SMART_INCOMING_CALL:
			_core_frame_display(UI_FRAME_PAGE_INCOMING_CALL, b_render);
			break;
		case UI_DISPLAY_SMART_INCOMING_MESSAGE:
			_core_frame_display(UI_FRAME_PAGE_INCOMING_MESSAGE,  b_render);
			break;
		case UI_DISPLAY_SMART_DETAIL_NOTIF:
			_core_frame_display(UI_FRAME_PAGE_DETAIL_NOTIF,  b_render);
			break;	
#ifndef _CLINGBAND_PACE_MODEL_		
		case UI_DISPLAY_SMART_MESSAGE:
		  _core_frame_display(UI_FRAME_PAGE_MESSAGE, b_render);
		  break;		
		case UI_DISPLAY_SMART_APP_NOTIF:
		  _core_frame_display(UI_FRAME_PAGE_APP_NOTIF, b_render);
		  break;
#endif		
#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)			
		case UI_DISPLAY_SMART_PHONE_FINDER:
			_core_frame_display(UI_FRAME_PAGE_PHONE_FINDER, b_render);
		  break;			
#endif		
		default:
			break;
	}
}

static void _frame_display_vital(I8U index, BOOLEAN b_render)
{	
	switch (index) {
		case UI_DISPLAY_VITAL_HEART_RATE:
			_core_frame_display(UI_FRAME_PAGE_HEART_RATE, b_render);
			break;
#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)	
		case UI_DISPLAY_VITAL_SKIN_TEMP:
			_core_frame_display(UI_FRAME_PAGE_SKIN_TEMP, b_render);
			break;		
#endif		
		default:
			break;
	}
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _display_frame_setting(I8U index, BOOLEAN b_render)
{
	switch (index) {
		case UI_DISPLAY_SETTING_VER:
	    _core_frame_display(UI_FRAME_PAGE_SETTING, b_render);
			break;
		default:
			break;
	}
}

static void _display_frame_stopwatch(I8U index, BOOLEAN b_render)
{
	switch (index) {
		case UI_DISPLAY_STOPWATCH_START:
			_core_frame_display(UI_FRAME_PAGE_STOPWATCH_START, b_render);
			break;	
		case UI_DISPLAY_STOPWATCH_STOP:
			_core_frame_display(UI_FRAME_PAGE_STOPWATCH_STOP, b_render);
			break;
		default:				
			break;
	}
}

static void _display_frame_workout(I8U index, BOOLEAN b_render)
{
	switch (index) {
		case UI_DISPLAY_WORKOUT_RUNNING:
			_core_frame_display(UI_FRAME_PAGE_WORKOUT_RUNNING, b_render);
			break;		
		case UI_DISPLAY_WORKOUT_CYCLING:
			_core_frame_display(UI_FRAME_PAGE_WORKOUT_CYCLING, b_render);
			break;			
		case UI_DISPLAY_WORKOUT_STAIRS:
			_core_frame_display(UI_FRAME_PAGE_WORKOUT_STAIRS, b_render);
			break;			
		case UI_DISPLAY_WORKOUT_ELLIPTICAL:
			_core_frame_display(UI_FRAME_PAGE_WORKOUT_ELLIPTICAL, b_render);
			break;		
		case UI_DISPLAY_WORKOUT_ROW:
			_core_frame_display(UI_FRAME_PAGE_WORKOUT_ROW, b_render);
			break;		
		case UI_DISPLAY_WORKOUT_AEROBIC:
			_core_frame_display(UI_FRAME_PAGE_WORKOUT_AEROBIC, b_render);
			break;			
		case UI_DISPLAY_WORKOUT_PILOXING:
			_core_frame_display(UI_FRAME_PAGE_WORKOUT_PILOXING, b_render);
			break;			
		case UI_DISPLAY_WORKOUT_OTHERS:
			_core_frame_display(UI_FRAME_PAGE_WORKOUT_OTHERS, b_render);
			break;			
		case UI_DISPLAY_WORKOUT_RUN_READY:
			_core_frame_display(UI_FRAME_PAGE_WORKOUT_RUN_READY, b_render);
			break;			
		case UI_DISPLAY_WORKOUT_RUN_TIME:
			_core_frame_display(UI_FRAME_PAGE_WORKOUT_RUN_TIME, b_render);
			break;	
		case UI_DISPLAY_WORKOUT_RUN_HEART_RATE:
			_core_frame_display(UI_FRAME_PAGE_WORKOUT_RUN_HEART_RATE, b_render);
			break;
		case UI_DISPLAY_WORKOUT_RUN_CALORIES:
			_core_frame_display(UI_FRAME_PAGE_WORKOUT_RUN_CALORIES, b_render);
			break;
		case UI_DISPLAY_WORKOUT_RUN_END:
			_core_frame_display(UI_FRAME_PAGE_WORKOUT_RUN_END, b_render);
			break;		
		default:
			break;		
	}
}
#endif

static void _frame_display_running_stats(I8U index, BOOLEAN b_render)
{	
	switch (index) {
#ifdef _CLINGBAND_PACE_MODEL_		
		case UI_DISPLAY_RUNNING_STAT_RUN_ANALYSIS:
			_core_frame_display(UI_FRAME_PAGE_RUNNING_ANALYSIS, b_render);
			break;		
#endif		
		case UI_DISPLAY_RUNNING_STAT_DISTANCE:
			_core_frame_display(UI_FRAME_PAGE_RUNNING_DISTANCE, b_render);
			break;
		case UI_DISPLAY_RUNNING_STAT_TIME:
			_core_frame_display(UI_FRAME_PAGE_RUNNING_TIME, b_render);
			break;
		case UI_DISPLAY_RUNNING_STAT_PACE:
			_core_frame_display(UI_FRAME_PAGE_RUNNING_PACE, b_render);
			break;
		case UI_DISPLAY_RUNNING_STAT_STRIDE:
			_core_frame_display(UI_FRAME_PAGE_RUNNING_STRIDE, b_render);
			break;
		case UI_DISPLAY_RUNNING_STAT_CADENCE:
			_core_frame_display(UI_FRAME_PAGE_RUNNING_CADENCE, b_render);
			break;
		case UI_DISPLAY_RUNNING_STAT_HEART_RATE:
			_core_frame_display(UI_FRAME_PAGE_RUNNING_HEART_RATE, b_render);
			break;
		case UI_DISPLAY_RUNNING_STAT_CALORIES:
			_core_frame_display(UI_FRAME_PAGE_RUNNING_CALORIES, b_render);
			break;
#ifdef _CLINGBAND_PACE_MODEL_			
		case UI_DISPLAY_RUNNING_STAT_STOP_ANALYSIS:
			_core_frame_display(UI_FRAME_PAGE_RUNNING_STOP_ANALYSIS, b_render);
			break;		
#endif		
		default:
			break;
	}
}

static void _frame_display_training_stats(I8U index, BOOLEAN b_render)
{	
	switch (index) {
		case UI_DISPLAY_TRAINING_STAT_RUN_START:
			_core_frame_display(UI_FRAME_PAGE_TRAINING_RUN_START, b_render);
			break;		
#ifndef _CLINGBAND_PACE_MODEL_				
		case UI_DISPLAY_TRAINING_STAT_RUN_OR_ANALYSIS:
			_core_frame_display(UI_FRAME_PAGE_TRAINING_RUN_OR_ANALYSIS, b_render);
			break;		
#endif		
		case UI_DISPLAY_TRAINING_STAT_READY:
			_core_frame_display(UI_FRAME_PAGE_TRAINING_READY, b_render);
			break;		
		case UI_DISPLAY_TRAINING_STAT_TIME:
			_core_frame_display(UI_FRAME_PAGE_TRAINING_TIME, b_render);
			break;		
		case UI_DISPLAY_TRAINING_STAT_DISTANCE:
			_core_frame_display(UI_FRAME_PAGE_TRAINING_DISTANCE, b_render);
			break;
		case UI_DISPLAY_TRAINING_STAT_PACE:
			_core_frame_display(UI_FRAME_PAGE_TRAINING_PACE, b_render);
			break;
		case UI_DISPLAY_TRAINING_STAT_HEART_RATE:
			_core_frame_display(UI_FRAME_PAGE_TRAINING_HEART_RATE, b_render);
			break;	
		case UI_DISPLAY_TRAINING_STAT_RUN_STOP:
			_core_frame_display(UI_FRAME_PAGE_TRAINING_RUN_STOP, b_render);
			break;				
		default:
			break;
	}
}

#ifndef _CLINGBAND_PACE_MODEL_		
static void _display_frame_cycling_outdoor_stats(I8U index, BOOLEAN b_render)
{
	switch (index) {
		case UI_DISPLAY_CYCLING_OUTDOOR_STAT_RUN_START:
			_core_frame_display(UI_FRAME_PAGE_CYCLING_OUTDOOR_RUN_START, b_render);
			break;				
		case UI_DISPLAY_CYCLING_OUTDOOR_STAT_READY:
			_core_frame_display(UI_FRAME_PAGE_CYCLING_OUTDOOR_READY, b_render);
			break;		
		case UI_DISPLAY_CYCLING_OUTDOOR_STAT_TIME:
			_core_frame_display(UI_FRAME_PAGE_CYCLING_OUTDOOR_TIME, b_render);
			break;		
		case UI_DISPLAY_CYCLING_OUTDOOR_STAT_DISTANCE:
			_core_frame_display(UI_FRAME_PAGE_CYCLING_OUTDOOR_DISTANCE, b_render);
			break;
		case UI_DISPLAY_CYCLING_OUTDOOR_STAT_SPEED:
			_core_frame_display(UI_FRAME_PAGE_CYCLING_OUTDOOR_SPEED, b_render);
			break;
		case UI_DISPLAY_CYCLING_OUTDOOR_STAT_HEART_RATE:
			_core_frame_display(UI_FRAME_PAGE_CYCLING_OUTDOOR_HEART_RATE, b_render);
			break;	
		case UI_DISPLAY_CYCLING_OUTDOOR_STAT_RUN_STOP:
			_core_frame_display(UI_FRAME_PAGE_CYCLING_OUTDOOR_RUN_STOP, b_render);
			break;				
		default:
			break;
	}
}
#endif

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)		
static void _display_frame_music(I8U index, BOOLEAN b_render)
{
	switch (index) {
		case UI_DISPLAY_MUSIC_PLAY:
			_core_frame_display(UI_FRAME_PAGE_MUSIC_PLAY_PAUSE, b_render);
			break;
		case UI_DISPLAY_MUSIC_VOLUME:
			_core_frame_display(UI_FRAME_PAGE_MUSIC_VOLUME, b_render);
			break;
		case UI_DISPLAY_MUSIC_SONG:
			_core_frame_display(UI_FRAME_PAGE_MUSIC_TRACK, b_render);
			break;
		default:
			break;
	}
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_		
static void _display_frame_carousel(I8U index, BOOLEAN b_render)
{	
	switch (index) {
		case UI_DISPLAY_CAROUSEL_1:
			_core_frame_display(UI_FRAME_PAGE_CAROUSEL_1, b_render);
			break;
		case UI_DISPLAY_CAROUSEL_2:
			_core_frame_display(UI_FRAME_PAGE_CAROUSEL_2, b_render);
			break;
		case UI_DISPLAY_CAROUSEL_3:
			_core_frame_display(UI_FRAME_PAGE_CAROUSEL_3, b_render);
			break;
#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)			
		case UI_DISPLAY_CAROUSEL_4:
			_core_frame_display(UI_FRAME_PAGE_CAROUSEL_4, b_render);
			break;		
#endif		
		default:
			break;
	}
}
#endif

void UI_frame_display_appear(I8U index, BOOLEAN b_render)
{
	N_SPRINTF("[UI] frame appear: %d, %d", index, u->frame_cached_index);

	if (index == UI_DISPLAY_HOME) {
		_frame_display_home(b_render);
	} else if ((index >= UI_DISPLAY_SYSTEM) && (index <= UI_DISPLAY_SYSTEM_END)) {
	  _frame_display_system(index, b_render);
	} else if ((index >= UI_DISPLAY_TRACKER) && (index <= UI_DISPLAY_TRACKER_END)) {
		_frame_display_tracker(index, b_render);
	} else if ((index >= UI_DISPLAY_SMART) && (index <= UI_DISPLAY_SMART_END)) {
		_frame_display_smart(index, b_render);
	} else if ((index >= UI_DISPLAY_VITAL) && (index <= UI_DISPLAY_VITAL_END)) {
		_frame_display_vital(index, b_render);
	} 
#ifndef _CLINGBAND_PACE_MODEL_	
	 else if ((index >= UI_DISPLAY_SETTING) && (index <= UI_DISPLAY_SETTING_END)) {
		_display_frame_setting(index, b_render);
	} else if ((index >= UI_DISPLAY_STOPWATCH) && (index <= UI_DISPLAY_STOPWATCH_END)) {
		_display_frame_stopwatch(index, b_render);
	} else if ((index >= UI_DISPLAY_WORKOUT) && (index <= UI_DISPLAY_WORKOUT_END)) {
		_display_frame_workout(index, b_render);
	}
#endif
	 else if ((index >= UI_DISPLAY_RUNNING_STATATISTICS) && (index <= UI_DISPLAY_RUNNING_STATATISTICS_END)) {
		_frame_display_running_stats(index, b_render);
	} else if ((index >= UI_DISPLAY_TRAINING_STATATISTICS) && (index <= UI_DISPLAY_TRAINING_STATATISTICS_END)) {
		_frame_display_training_stats(index, b_render);
	} 
#ifndef _CLINGBAND_PACE_MODEL_		
	  else if ((index >= UI_DISPLAY_CYCLING_OUTDOOR_STATATISTICS) && (index <= UI_DISPLAY_CYCLING_OUTDOOR_STATATISTICS_END)) {
		_display_frame_cycling_outdoor_stats(index, b_render);
	} 
#endif		
#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)		
	  else if ((index >= UI_DISPLAY_MUSIC) && (index <= UI_DISPLAY_MUSIC_END)) {
		_display_frame_music(index, b_render);
	} 
#endif		
#ifndef _CLINGBAND_PACE_MODEL_		
	  else if ((index >= UI_DISPLAY_CAROUSEL) && (index <= UI_DISPLAY_CAROUSEL_END)) {
		_display_frame_carousel(index, b_render);
	} 
#endif	
	  else {
	}
}
/*****************************************************************************************************/
/************************************ frame display **************************************************/
/*****************************************************************************************************/
const UI_RENDER_CTX horizontal_ui_render[] = {
  {"NONE",                     _RENDER_NONE,                                  _RENDER_NONE,                                        _RENDER_NONE},                                     /*UI_FRAME_PAGE_NONE*/
  {"HOME_CLOCK",               _left_render_horizontal_batt_ble,              _middle_render_horizontal_clock,                     _right_render_horizontal_home},                    /*UI_FRAME_PAGE_HOME_CLOCK*/
  {"SYATEM_RESTART_LOGO",      _RENDER_NONE,                                  _middle_render_horizontal_system_restart,            _RENDER_NONE},	                                    /*UI_FRAME_PAGE_PACE_LOGO*/
  {"SYATEM_AUTH_UNAUTHORIZED", _left_render_horizontal_batt_ble,              _middle_render_horizontal_ble_code,                  _right_render_horizontal_firmware_ver},            /*UI_FRAME_PAGE_UNAUTHORIZED*/
  {"SYATEM_AUTH_LINKING",      _left_render_horizontal_batt_ble,              _middle_render_horizontal_linking,                   _RENDER_NONE},                                     /*UI_FRAME_PAGE_LINKING*/
  {"SYATEM_OTA",               _RENDER_NONE,                                  _middle_render_horizontal_ota,                       _RENDER_NONE},                                     /*UI_FRAME_PAGE_OTA*/
  {"SYSTEM_BATT_POWER_1",      _RENDER_NONE,                                  _middle_render_horizontal_system_charging,           _RENDER_NONE},                                     /*UI_FRAME_PAGE_BATT_POWER*/
  {"TRACKER_STEP",             _left_render_horizontal_steps,                 _middle_render_horizontal_steps,                     _right_render_horizontal_tracker},                 /*UI_FRAME_PAGE_STEPS*/
  {"TRACKER_DISTANCE",         _left_render_horizontal_distance,              _middle_render_horizontal_distance,                  _right_render_horizontal_tracker},                 /*UI_FRAME_PAGE_DISTANCE*/
  {"TRACKER_CALORIES",         _left_render_horizontal_calories,              _middle_render_horizontal_calories,                  _right_render_horizontal_tracker},                 /*UI_FRAME_PAGE_CALORIES*/
  {"TRACKER_ACTIVE_TIME",      _left_render_horizontal_active_time,           _middle_render_horizontal_active_time,               _right_render_horizontal_tracker},                 /*UI_FRAME_PAGE_ACTIVE_TIME*/
  {"SMART_PM2P5",              _left_render_horizontal_pm2p5,                 _middle_render_horizontal_pm2p5,                     _RENDER_NONE},                                     /*UI_FRAME_PAGE_PM2P5*/
  {"SMART_WEATHER",            _left_render_horizontal_weather,               _middle_render_horizontal_weather,                   _RENDER_NONE},                                     /*UI_FRAME_PAGE_WEATHER*/
  {"SMART_MESSAGE",            _left_render_horizontal_return,                _middle_render_horizontal_message,                   _right_render_horizontal_more},                    /*UI_FRAME_PAGE_MESSAGE*/
  {"SMART_APP_NOTIF",          _left_render_horizontal_return,                _middle_render_horizontal_app_notif,                 _right_render_horizontal_app_notif},               /*UI_FRAME_PAGE_APP_NOTIF*/
  {"SMART_INCOMING_CALL",      _left_render_horizontal_incoming_call,         _middle_render_horizontal_incoming_call,             _right_render_horizontal_ok},                      /*UI_FRAME_PAGE_INCOMING_CALL*/
  {"SMART_INCOMING_MESSAGE",   _left_render_horizontal_incoming_message,      _middle_render_horizontal_incoming_message,          _right_render_horizontal_ok},                      /*UI_FRAME_PAGE_INCOMING_MESSAGE*/
  {"SMART_DETAIL_NOTIF",       _RENDER_NONE,                                  _middle_render_horizontal_detail_notif,              _right_render_horizontal_more},                    /*UI_FRAME_PAGE_DETAIL_NOTIF*/
  {"SMART_ALARM_REMINDER",     _left_render_horizontal_reminder,              _middle_render_horizontal_reminder,                  _right_render_horizontal_reminder},                /*UI_FRAME_PAGE_ALARM_CLOCK_REMINDER*/
  {"SMART_ALARM_DETAIL",       _left_render_horizontal_reminder,              _middle_render_horizontal_reminder,                  _right_render_horizontal_reminder},                /*UI_FRAME_PAGE_ALARM_CLOCK_DETAIL*/
  {"SMART_IDLE_ALERT",         _left_render_horizontal_idle_alert,            _middle_render_horizontal_idle_alert,                _RENDER_NONE},                                     /*UI_FRAME_PAGE_IDLE_ALERT*/
  {"SMART_HEART_RATE_ALERT",   _left_render_horizontal_heart_rate,            _middle_render_horizontal_heart_rate,                _right_render_horizontal_small_clock},             /*UI_FRAME_PAGE_HEART_RATE_ALERT*/
  {"SMART_STEP_10K_ALERT",     _left_render_horizontal_steps,                 _middle_render_horizontal_steps,                     _right_render_horizontal_small_clock},             /*UI_FRAME_PAGE_STEP_10K_ALERT*/
  {"TRACKER_HEART_RATE",       _left_render_horizontal_heart_rate,            _middle_render_horizontal_heart_rate,                _right_render_horizontal_small_clock},             /*UI_FRAME_PAGE_HEART_RATE*/
  {"TRACKER_SKIN_TEMP",        _left_render_horizontal_skin_temp,             _middle_render_horizontal_skin_temp,                 _right_render_horizontal_small_clock},             /*UI_FRAME_PAGE_SKIN_TEMP*/
  {"SETTING",                  _RENDER_NONE,                                  _middle_render_horizontal_setting,                   _RENDER_NONE},                                     /*UI_FRAME_PAGE_SETTING*/
  {"STOPWATCH_START",          _left_render_horizontal_stopwatch,             _middle_render_horizontal_stopwatch_start,           _RENDER_NONE},                                     /*UI_FRAME_PAGE_STOPWATCH_START*/
  {"STOPWATCH_STOP",           _left_render_horizontal_stopwatch,             _middle_render_horizontal_stopwatch_stop,            _right_render_horizontal_run_ok},                  /*UI_FRAME_PAGE_STOPWATCH_STOP*/
  {"WORKOUT_RUNNING_MODE",     _left_render_horizontal_return,                _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok},                      /*UI_FRAME_PAGE_WORKOUT_RUNNING*/
  {"WORKOUT_CYCLING_MODE",     _left_render_horizontal_return,                _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok},                      /*UI_FRAME_PAGE_WORKOUT_CYCLING*/
  {"WORKOUT_STAIRS_MODE",      _left_render_horizontal_return,                _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok},                      /*UI_FRAME_PAGE_WORKOUT_STAIRS*/
  {"WORKOUT_ELLIPTICAL_MODE",  _left_render_horizontal_return,                _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok},                      /*UI_FRAME_PAGE_WORKOUT_ELLIPTICAL*/
  {"WORKOUT_ROW_MODE",         _left_render_horizontal_return,                _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok},                      /*UI_FRAME_PAGE_WORKOUT_ROW*/
  {"WORKOUT_AEROBIC_MODE",     _left_render_horizontal_return,                _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok},                      /*UI_FRAME_PAGE_WORKOUT_AEROBIC*/
  {"WORKOUT_PILOXING_MODE",    _left_render_horizontal_return,                _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok},                      /*UI_FRAME_PAGE_WORKOUT_PILOXING*/
  {"WORKOUT_OTHERS_MODE",      _left_render_horizontal_return,                _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok},                      /*UI_FRAME_PAGE_WORKOUT_OTHERS*/
  {"WORKOUT_RUN_READY",        _RENDER_NONE,                                  _middle_render_horizontal_workout_ready,             _RENDER_NONE},                                     /*UI_FRAME_PAGE_WORKOUT_RUN_READY*/
  {"WORKOUT_RUN_TIME",         _left_render_horizontal_training_time,         _middle_render_horizontal_training_time,             _RENDER_NONE},                                     /*UI_FRAME_PAGE_WORKOUT_RUN_TIME*/
  {"WORKOUT_RUN_HEART_RATE",   _left_render_horizontal_training_hr,           _middle_render_horizontal_training_hr,               _right_render_horizontal_training_hr},             /*UI_FRAME_PAGE_WORKOUT_RUN_HEART_RATE*/
  {"WORKOUT_RUN_CALORIES",     _left_render_horizontal_running_calories,      _middle_render_horizontal_running_calories,          _right_render_horizontal_running_calories},        /*UI_FRAME_PAGE_WORKOUT_RUN_CALORIES*/
  {"WORKOUT_RUN_END",          _left_render_horizontal_running_stop,          _middle_render_horizontal_training_run_stop,         _right_render_horizontal_run_ok},                  /*UI_FRAME_PAGE_WORKOUT_RUN_END*/
  {"RUN_DISTANCE",             _left_render_horizontal_running_distance_16,   _middle_render_horizontal_running_distance,          _right_render_horizontal_running_distance},        /*UI_FRAME_PAGE_RUNNING_DISTANCE*/
  {"RUN_TIME",                 _left_render_horizontal_running_time,          _middle_render_horizontal_running_time,              _RENDER_NONE},                                     /*UI_FRAME_PAGE_RUNNING_TIME*/
  {"RUN_PACE",                 _left_render_horizontal_running_pace,          _middle_render_horizontal_running_pace,              _right_render_horizontal_running_pace},            /*UI_FRAME_PAGE_RUNNING_PACE*/
  {"RUN_HR",                   _left_render_horizontal_running_hr,            _middle_render_horizontal_running_hr,                _right_render_horizontal_running_hr},              /*UI_FRAME_PAGE_RUNNING_HEART_RATE*/
  {"RUN_CAL",                  _left_render_horizontal_running_calories,      _middle_render_horizontal_running_calories,          _right_render_horizontal_running_calories},        /*UI_FRAME_PAGE_RUNNING_CALORIES*/
  {"RUN_CADENCE",              _left_render_horizontal_running_cadence,       _middle_render_horizontal_running_cadence,           _right_render_horizontal_running_cadence},         /*UI_FRAME_PAGE_RUNNING_CADENCE*/
  {"RUN_STRIDE",               _left_render_horizontal_running_stride,        _middle_render_horizontal_running_stride,            _right_render_horizontal_running_stride},          /*UI_FRAME_PAGE_RUNNING_STRIDE*/
  {"TRAIN_RUN_START",          _left_render_horizontal_running_distance_24,   _middle_render_horizontal_training_start_run,        _right_render_horizontal_run_go},                  /*UI_FRAME_PAGE_TRAINING_RUN_START*/
  {"TRAIN_RUN_OR_ANALYSIS",    _left_render_horizontal_running_distance_24,   _middle_render_horizontal_training_run_or_analysis,  _RENDER_NONE},                                     /*UI_FRAME_PAGE_TRAINING_RUN_OR_ANALYSIS*/
  {"TRAIN_READY",              _left_render_horizontal_training_ready,        _middle_render_horizontal_training_ready,            _RENDER_NONE},                                     /*UI_FRAME_PAGE_TRAINING_READY*/ 
  {"TRAIN_TIME",               _left_render_horizontal_training_time,         _middle_render_horizontal_training_time,             _RENDER_NONE},                                     /*UI_FRAME_PAGE_TRAINING_TIME*/
  {"TRAIN_DISTANCE",           _left_render_horizontal_training_distance,     _middle_render_horizontal_training_distance,         _right_render_horizontal_training_distance},       /*UI_FRAME_PAGE_TRAINING_DISTANCE*/
  {"TRAIN_PACE",               _left_render_horizontal_training_pace,         _middle_render_horizontal_training_pace,             _right_render_horizontal_training_pace},           /*UI_FRAME_PAGE_TRAINING_PACE*/
  {"TRAIN_HR",                 _left_render_horizontal_training_hr,           _middle_render_horizontal_training_hr,               _right_render_horizontal_training_hr},             /*UI_FRAME_PAGE_TRAINING_HEART_RATE*/
  {"TRAIN_RUN_STOP",           _left_render_horizontal_running_stop,          _middle_render_horizontal_training_run_stop,         _right_render_horizontal_run_ok},                  /*UI_FRAME_PAGE_TRAINING_RUN_STOP*/
  {"CYCLING_OUTDOOR_RUN_START",_left_render_horizontal_cycling_outdoor_24,    _middle_render_horizontal_cycling_outdoor_run_start, _right_render_horizontal_run_go},                  /*UI_FRAME_PAGE_CYCLING_OUTDOOR_RUN_START*/
  {"CYCLING_OUTDOOR_READY",    _left_render_horizontal_cycling_outdoor_ready, _middle_render_horizontal_cycling_outdoor_ready,     _RENDER_NONE},                                     /*UI_FRAME_PAGE_CYCLING_OUTDOOR_READY*/
  {"CYCLING_OUTDOOR_TIME",     _left_render_horizontal_training_time,         _middle_render_horizontal_training_time,             _RENDER_NONE},                                     /*UI_FRAME_PAGE_CYCLING_OUTDOOR_TIME*/
  {"CYCLING_OUTDOOR_DISTANCE", _left_render_horizontal_cycling_outdoor_16,    _middle_render_horizontal_cycling_outdoor_distance,  _right_render_horizontal_cycling_outdoor_distance},/*UI_FRAME_PAGE_CYCLING_OUTDOOR_DISTANCE*/
  {"CYCLING_OUTDOOR_SPEED",    _left_render_horizontal_cycling_outdoor_speed, _middle_render_horizontal_cycling_outdoor_speed,     _right_render_horizontal_cycling_outdoor_speed},   /*UI_FRAME_PAGE_CYCLING_OUTDOOR_SPEED*/
  {"CYCLING_OUTDOOR_HR",       _left_render_horizontal_training_hr,           _middle_render_horizontal_training_hr,               _right_render_horizontal_training_hr},             /*UI_FRAME_PAGE_CYCLING_OUTDOOR_HEART_RATE*/
  {"CYCLING_OUTDOOR_RUN_STOP", _left_render_horizontal_running_stop,          _middle_render_horizontal_cycling_outdoor_run_stop,  _right_render_horizontal_run_ok},                  /*UI_FRAME_PAGE_CYCLING_OUTDOOR_RUN_STOP*/
  {"CAROUSEL_1",               _RENDER_NONE,                                  _middle_render_horizontal_carousel_1,                _RENDER_NONE},                                     /*UI_FRAME_PAGE_CAROUSEL_1*/
  {"CAROUSEL_2",               _RENDER_NONE,                                  _middle_render_horizontal_carousel_2,                _RENDER_NONE},                                     /*UI_FRAME_PAGE_CAROUSEL_2*/
  {"CAROUSEL_3",               _RENDER_NONE,                                  _middle_render_horizontal_carousel_3,                _RENDER_NONE},                                     /*UI_FRAME_PAGE_CAROUSEL_3*/
};

const UI_RENDER_CTX vertical_ui_render[] = {
  {"NONE",                     _RENDER_NONE,                                  _RENDER_NONE,                                        _RENDER_NONE},                                     /*UI_FRAME_PAGE_NONE*/
  {"HOME_CLOCK",               _top_render_vertical_batt_ble,                 _middle_render_vertical_clock,                       _bottom_render_vertical_home},                     /*UI_FRAME_PAGE_HOME_CLOCK*/
  {"SYATEM_RESTART_LOGO",      _RENDER_NONE,                                  _middle_render_horizontal_system_restart,            _RENDER_NONE},		                                  /*UI_FRAME_PAGE_PACE_LOGO*/
  {"SYATEM_AUTH_UNAUTHORIZED", _left_render_horizontal_batt_ble,              _middle_render_horizontal_ble_code,                  _right_render_horizontal_firmware_ver},            /*UI_FRAME_PAGE_UNAUTHORIZED*/
  {"SYATEM_AUTH_LINKING",      _left_render_horizontal_batt_ble,              _middle_render_horizontal_linking,                   _RENDER_NONE},                                     /*UI_FRAME_PAGE_LINKING*/
  {"SYATEM_OTA",               _RENDER_NONE,                                  _middle_render_horizontal_ota,                       _RENDER_NONE},                                     /*UI_FRAME_PAGE_OTA*/
  {"SYSTEM_BATT_POWER_1",      _RENDER_NONE,                                  _middle_render_horizontal_system_charging,           _RENDER_NONE},                                     /*UI_FRAME_PAGE_BATT_POWER*/
  {"TRACKER_STEP",             _top_render_vertical_steps,                    _middle_render_vertical_steps,                       _bottom_render_vertical_tracker},                  /*UI_FRAME_PAGE_STEPS*/
  {"TRACKER_DISTANCE",         _top_render_vertical_distance,                 _middle_render_vertical_distance,                    _bottom_render_vertical_tracker},                  /*UI_FRAME_PAGE_DISTANCE*/
  {"TRACKER_CALORIES",         _top_render_vertical_calories,                 _middle_render_vertical_calories,                    _bottom_render_vertical_tracker},                  /*UI_FRAME_PAGE_CALORIES*/
  {"TRACKER_ACTIVE_TIME",      _top_render_vertical_active_time,              _middle_render_vertical_active_time,                 _bottom_render_vertical_tracker},                  /*UI_FRAME_PAGE_ACTIVE_TIME*/
  {"SMART_PM2P5",              _top_render_vertical_pm2p5,                    _middle_render_vertical_pm2p5,                       _bottom_render_vertical_small_clock},              /*UI_FRAME_PAGE_PM2P5*/
  {"SMART_WEATHER",            _top_render_vertical_weather,                  _middle_render_vertical_weather,                     _RENDER_NONE},                                     /*UI_FRAME_PAGE_WEATHER*/
  {"SMART_MESSAGE",            _left_render_horizontal_return,                _middle_render_horizontal_message,                   _right_render_horizontal_more},                    /*UI_FRAME_PAGE_MESSAGE*/
  {"SMART_APP_NOTIF",          _left_render_horizontal_return,                _middle_render_horizontal_app_notif,                 _right_render_horizontal_app_notif},               /*UI_FRAME_PAGE_APP_NOTIF*/
  {"SMART_INCOMING_CALL",      _left_render_horizontal_incoming_call,         _middle_render_horizontal_incoming_call,             _right_render_horizontal_ok},                      /*UI_FRAME_PAGE_INCOMING_CALL*/
  {"SMART_INCOMING_MESSAGE",   _left_render_horizontal_incoming_message,      _middle_render_horizontal_incoming_message,          _right_render_horizontal_ok},                      /*UI_FRAME_PAGE_INCOMING_MESSAGE*/
  {"SMART_DETAIL_NOTIF",       _RENDER_NONE,                                  _middle_render_horizontal_detail_notif,              _right_render_horizontal_more},                    /*UI_FRAME_PAGE_DETAIL_NOTIF*/
  {"SMART_ALARM_REMINDER",     _top_render_vertical_reminder,                 _middle_render_vertical_reminder,                    _bottom_render_vertiacl_reminder},                 /*UI_FRAME_PAGE_ALARM_CLOCK_REMINDER*/
  {"SMART_ALARM_DETAIL",       _top_render_vertical_reminder,                 _middle_render_vertical_reminder,                    _bottom_render_vertiacl_reminder},                 /*UI_FRAME_PAGE_ALARM_CLOCK_DETAIL*/
  {"SMART_IDLE_ALERT",         _left_render_horizontal_idle_alert,            _middle_render_horizontal_idle_alert,                _right_render_horizontal_small_clock},             /*UI_FRAME_PAGE_IDLE_ALERT*/
  {"SMART_HEART_RATE_ALERT",   _top_render_vertical_heart_rate,               _middle_render_vertical_heart_rate,                  _bottom_render_vertical_small_clock},              /*UI_FRAME_PAGE_HEART_RATE_ALERT*/
  {"SMART_STEP_10K_ALERT",     _top_render_vertical_steps,                    _middle_render_vertical_steps,                       _bottom_render_vertical_small_clock},              /*UI_FRAME_PAGE_STEP_10K_ALERT*/
  {"TRACKER_HEART_RATE",       _top_render_vertical_heart_rate,               _middle_render_vertical_heart_rate,                  _bottom_render_vertical_small_clock},              /*UI_FRAME_PAGE_HEART_RATE*/
  {"TRACKER_SKIN_TEMP",        _top_render_vertical_skin_temp,                _middle_render_vertical_skin_temp,                   _bottom_render_vertical_small_clock},              /*UI_FRAME_PAGE_SKIN_TEMP*/
  {"SETTING",                  _RENDER_NONE,                                  _middle_render_horizontal_setting,                   _RENDER_NONE},                                     /*UI_FRAME_PAGE_SETTING*/
  {"STOPWATCH_START",          _left_render_horizontal_stopwatch,             _middle_render_horizontal_stopwatch_start,           _RENDER_NONE},                                     /*UI_FRAME_PAGE_STOPWATCH_START*/
  {"STOPWATCH_STOP",           _left_render_horizontal_stopwatch,             _middle_render_horizontal_stopwatch_stop,            _right_render_horizontal_run_ok},                  /*UI_FRAME_PAGE_STOPWATCH_STOP*/
  {"WORKOUT_RUNNING_MODE",     _left_render_horizontal_return,                _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok},                      /*UI_FRAME_PAGE_WORKOUT_RUNNING*/
  {"WORKOUT_CYCLING_MODE",     _left_render_horizontal_return,                _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok},                      /*UI_FRAME_PAGE_WORKOUT_CYCLING*/
  {"WORKOUT_STAIRS_MODE",      _left_render_horizontal_return,                _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok},                      /*UI_FRAME_PAGE_WORKOUT_STAIRS*/
  {"WORKOUT_ELLIPTICAL_MODE",  _left_render_horizontal_return,                _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok},                      /*UI_FRAME_PAGE_WORKOUT_ELLIPTICAL*/
  {"WORKOUT_ROW_MODE",         _left_render_horizontal_return,                _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok},                      /*UI_FRAME_PAGE_WORKOUT_ROW*/
  {"WORKOUT_AEROBIC_MODE",     _left_render_horizontal_return,                _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok},                      /*UI_FRAME_PAGE_WORKOUT_AEROBIC*/
  {"WORKOUT_PILOXING_MODE",    _left_render_horizontal_return,                _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok},                      /*UI_FRAME_PAGE_WORKOUT_PILOXING*/
  {"WORKOUT_OTHERS_MODE",      _left_render_horizontal_return,                _middle_render_horizontal_workout_mode_switch,       _right_render_horizontal_ok},                      /*UI_FRAME_PAGE_WORKOUT_OTHERS*/
  {"WORKOUT_RUN_READY",        _RENDER_NONE,                                  _middle_render_horizontal_workout_ready,             _RENDER_NONE},                                     /*UI_FRAME_PAGE_WORKOUT_RUN_READY*/
  {"WORKOUT_RUN_TIME",         _left_render_horizontal_training_time,         _middle_render_horizontal_training_time,             _RENDER_NONE},                                     /*UI_FRAME_PAGE_WORKOUT_RUN_TIME*/
  {"WORKOUT_RUN_HEART_RATE",   _left_render_horizontal_training_hr,           _middle_render_horizontal_training_hr,               _RENDER_NONE},                                     /*UI_FRAME_PAGE_WORKOUT_RUN_HEART_RATE*/
  {"WORKOUT_RUN_CALORIES",     _left_render_horizontal_running_calories,      _middle_render_horizontal_running_calories,          _right_render_horizontal_running_calories},        /*UI_FRAME_PAGE_WORKOUT_RUN_CALORIES*/
  {"WORKOUT_RUN_END",          _left_render_horizontal_running_stop,          _middle_render_horizontal_training_run_stop,         _right_render_horizontal_run_ok},                  /*UI_FRAME_PAGE_WORKOUT_RUN_END*/
  {"RUN_DISTANCE",             _top_render_vertical_running_distance,         _middle_render_vertical_running_distance,            _RENDER_NONE},                                     /*UI_FRAME_PAGE_RUNNING_DISTANCE*/
  {"RUN_TIME",                 _top_render_vertical_running_time,             _middle_render_vertical_running_time,                _RENDER_NONE},                                     /*UI_FRAME_PAGE_RUNNING_TIME*/
  {"RUN_PACE",                 _top_render_vertical_running_pace,             _middle_render_vertical_running_pace,                _RENDER_NONE},                                     /*UI_FRAME_PAGE_RUNNING_PACE*/
  {"RUN_HR",                   _top_render_vertical_running_hr,               _middle_render_vertical_running_hr,                  _RENDER_NONE},                                     /*UI_FRAME_PAGE_RUNNING_HEART_RATE*/
  {"RUN_CAL",                  _top_render_vertical_running_calories,         _middle_render_vertical_running_calories,            _RENDER_NONE},                                     /*UI_FRAME_PAGE_RUNNING_CALORIES*/
  {"RUN_CADENCE",              _top_render_vertical_running_cadence,          _middle_render_vertical_running_cadence,             _RENDER_NONE},                                     /*UI_FRAME_PAGE_RUNNING_CADENCE*/
  {"RUN_STRIDE",               _top_render_vertical_running_stride,           _middle_render_vertical_running_stride,              _RENDER_NONE},                                     /*UI_FRAME_PAGE_RUNNING_STRIDE*/
  {"TRAIN_RUN_START",          _top_render_vertical_running_distance,         _middle_render_vertical_training_run_start,          _bottom_render_vertical_run_go},                   /*UI_FRAME_PAGE_TRAINING_RUN_START*/ 
  {"TRAIN_RUN_OR_ANALYSIS",    _top_render_vertical_running_distance,         _middle_render_vertical_training_run_or_analysis,    _RENDER_NONE},                                     /*UI_FRAME_PAGE_TRAINING_RUN_OR_ANALYSIS*/ 
  {"TRAIN_READY",              _top_render_vertical_running_distance,         _middle_render_vertical_training_ready,              _RENDER_NONE},                                     /*UI_FRAME_PAGE_TRAINING_READY*/ 
  {"TRAIN_TIME",               _top_render_vertical_training_time,            _middle_render_vertical_training_time,               _RENDER_NONE},                                     /*UI_FRAME_PAGE_TRAINING_TIME*/
  {"TRAIN_DISTANCE",           _top_render_vertical_running_distance,         _middle_render_vertical_training_distance,           _RENDER_NONE},                                     /*UI_FRAME_PAGE_TRAINING_DISTANCE*/
  {"TRAIN_PACE",               _top_render_vertical_running_pace,             _middle_render_vertical_training_pace,               _RENDER_NONE},                                     /*UI_FRAME_PAGE_TRAINING_PACE*/
  {"TRAIN_HR",                 _top_render_vertical_running_hr,               _middle_render_vertical_training_hr,                 _RENDER_NONE},                                     /*UI_FRAME_PAGE_TRAINING_HEART_RATE*/
  {"TRAIN_RUN_STOP",           _top_render_vertical_running_stop,             _middle_render_vertical_training_run_stop,           _bottom_render_vertical_run_ok},                   /*UI_FRAME_PAGE_TRAINING_RUN_STOP*/
  {"CYCLING_OUTDOOR_RUN_START",_top_render_vertical_cycling_outdoor,          _middle_render_vertical_cycling_outdoor_run_start,   _bottom_render_vertical_run_go},                   /*UI_FRAME_PAGE_CYCLING_OUTDOOR_RUN_START*/
  {"CYCLING_OUTDOOR_READY",    _top_render_vertical_cycling_outdoor,          _middle_render_vertical_cycling_outdoor_ready,       _RENDER_NONE},                                     /*UI_FRAME_PAGE_CYCLING_OUTDOOR_READY*/
  {"CYCLING_OUTDOOR_TIME",     _top_render_vertical_training_time,            _middle_render_vertical_training_time,               _RENDER_NONE},                                     /*UI_FRAME_PAGE_CYCLING_OUTDOOR_TIME*/
  {"CYCLING_OUTDOOR_DISTANCE", _top_render_vertical_cycling_outdoor,          _middle_render_vertical_cycling_outdoor_distance,    _bottom_render_vertical_cycling_outdoor_distance}, /*UI_FRAME_PAGE_CYCLING_OUTDOOR_DISTANCE*/
  {"CYCLING_OUTDOOR_SPEED",    _top_render_vertical_cycling_outdoor_speed,    _middle_render_vertical_cycling_outdoor_speed,       _bottom_render_vertical_cycling_outdoor_speed},    /*UI_FRAME_PAGE_CYCLING_OUTDOOR_SPEED*/
  {"CYCLING_OUTDOOR_HR",       _top_render_vertical_running_hr,               _middle_render_vertical_training_hr,                 _RENDER_NONE},                                     /*UI_FRAME_PAGE_CYCLING_OUTDOOR_HEART_RATE*/
  {"CYCLING_OUTDOOR_RUN_STOP", _top_render_vertical_running_stop,             _middle_render_vertical_cycling_outdoor_run_stop,    _bottom_render_vertical_run_ok},                   /*UI_FRAME_PAGE_CYCLING_OUTDOOR_RUN_STOP*/
  {"CAROUSEL_1",               _RENDER_NONE,                                  _middle_render_vertical_carousel_1,                  _RENDER_NONE},                                     /*UI_FRAME_PAGE_CAROUSEL_1*/
  {"CAROUSEL_2",               _RENDER_NONE,                                  _middle_render_vertical_carousel_2,                  _RENDER_NONE},                                     /*UI_FRAME_PAGE_CAROUSEL_2*/
  {"CAROUSEL_3",               _RENDER_NONE,                                  _middle_render_vertical_carousel_3,                  _RENDER_NONE},                                     /*UI_FRAME_PAGE_CAROUSEL_3*/	
};


static void _core_frame_display(I8U middle, BOOLEAN b_render)
{	
	const UI_RENDER_CTX *ui_render;

	// Set correct screen orientation
	if (cling.ui.clock_orientation == 1) {
		ui_render = horizontal_ui_render;
	} else {
		ui_render = vertical_ui_render;
	}

	// Clean up all screen.
	memset(cling.ui.p_oled_up, 0, 512);

	ui_render[middle].middle_row_render();
	ui_render[middle].top_row_render();	
	ui_render[middle].bottom_row_render();
	
	if (b_render) {
		// Finally, we render the frame
		UI_render_screen();
	}
}
