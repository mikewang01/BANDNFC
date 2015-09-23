/***************************************************************************//**
 * File: ui.c
 * 
 * Description: master UI controller
 *
 ******************************************************************************/

#include "main.h"
#include "assets.h"
#include "ui_matrix.h"

#define RENDERING_MAX_FRAMES     11

void UI_init()
{
	// Turn on OLED panel
	OLED_set_panel_on();

	// UI initial state
	UI_switch_state(UI_STATE_CLING_START, 2000);
	
	// UI prefer state
	cling.ui.prefer_state = UI_STATE_HOME;
	cling.ui.true_display = TRUE;
	cling.ui.fonts_cn = TRUE; // display Chinese characters
}

static void _set_animation(I8U mode, I8U dir)
{
		UI_ANIMATION_CTX *u = &cling.ui;

		u->state = UI_STATE_ANIMATING;
		u->animation_index = 0;
		u->direction = dir;
		u->frame_current_idx = u->frame_index;
		u->animation_mode = mode;
}

static void _update_vertical_index(UI_ANIMATION_CTX *u, BOOLEAN b_up)
{
	I8U index = u->frame_current_idx;
	I8U max_frame_num;
	
	if (index == UI_DISPLAY_HOME) {
		max_frame_num = FRAME_DEPTH_CLOCK;
	} else if (index == UI_DISPLAY_TRACKER_STEP) {
		max_frame_num = FRAME_DEPTH_STEP;
	} else if (index == UI_DISPLAY_TRACKER_SLEEP) {
		max_frame_num = FRAME_DEPTH_SLEEP; 
	} else if (index == UI_DISPLAY_TRACKER_DISTANCE) {
		max_frame_num = FRAME_DEPTH_DISTANCE;
	} else if (index == UI_DISPLAY_TRACKER_CALORIES) {
		max_frame_num = FRAME_DEPTH_CALORIES;
	} else if (index == UI_DISPLAY_SMART_REMINDER) {
//	max_frame_num = FRAME_DEPTH_REMINDER;
		max_frame_num = cling.reminder.total;
	} else if (index == UI_DISPLAY_SMART_WEATHER) {
		max_frame_num = FRAME_DEPTH_WEATHER;
	} else if (index == UI_DISPLAY_VITAL_HEART_RATE) {
		max_frame_num = FRAME_DEPTH_HEART_RATE;
	} else if (index == UI_DISPLAY_VITAL_SKIN_TEMP) {
		max_frame_num = FRAME_DEPTH_SKIN_TEMP;
	} else {
		max_frame_num = 0;
		
		return;
	}
	
	// Update vertical index
	if (b_up) {
		u->vertical_index++;
		
		if (u->vertical_index >= max_frame_num) {
			u->vertical_index = 0;
		}
		
	} else {
		if (u->vertical_index == 0) {
			u->vertical_index = max_frame_num - 1;
		} else {
			u->vertical_index --;
		}
	}
	
	// Read out reminder index from Flash
	if (index == UI_DISPLAY_SMART_REMINDER) {
			cling.ui.vertical_index = REMINDER_get_time_at_index(cling.ui.vertical_index);
	}
}

static void _update_finger_down_index(UI_ANIMATION_CTX *u)
{
	if (u->frame_current_idx == UI_DISPLAY_SMART_MESSAGE) {
		u->vertical_index++;
#ifndef _CLING_PC_SIMULATION_
#ifdef _ENABLE_ANCS_
		u->vertical_index = ANCS_get_next_new_notification(u->vertical_index);
#endif
#endif
		N_SPRINTF("[UI] ---- Notif idx: %d", u->vertical_index);
#ifdef _ENABLE_ANCS_
		if (u->vertical_index >= ANCS_CATEGORY_ID_MAX) {
			memset(cling.ancs.cat_count, 0, ANCS_CATEGORY_ID_MAX);
			u->frame_index = UI_DISPLAY_HOME;
			u->vertical_index = 0;
			N_SPRINTF("[UI] ---- Notif idx: FULL, frame_index: %d", u->frame_index);
		}
#endif
	} 
}

static I8U _ui_touch_sensing()
{
	UI_ANIMATION_CTX *u = &cling.ui;
	I8U gesture = TOUCH_get_gesture_panel();
	const I8U *p_matrix = NULL;
	I32U t_curr = CLK_get_system_time();
	
	if (gesture) {
		N_SPRINTF("[UI] ++++ gesture: %d", gesture);
	}
	
	// UI state switching based on valid gesture from Cypress IC
	switch (gesture) {
		case TOUCH_SWIPE_DOWN:
		{
			break;
		}
		case TOUCH_SWIPE_UP:
		{
			break;
		}
		case TOUCH_SWIPE_LEFT:
		{
			if (ui_gesture_constrain[u->frame_index] & UFG_SWIPE_HORIZONTAL) {
				_set_animation(ANIMATION_PANNING_OUT, TRANSITION_DIR_LEFT);
				p_matrix = ui_matrix_swipe_left;
				//u->vertical_index = 0; // Reset vertical index
				_update_vertical_index(u, FALSE);
				Y_SPRINTF("[UI] swipe left: %d", u->frame_index);
			}
			break;
		}
		case TOUCH_SWIPE_RIGHT:
		{
			if (ui_gesture_constrain[u->frame_index] & UFG_SWIPE_HORIZONTAL) {
				_set_animation(ANIMATION_PANNING_OUT, TRANSITION_DIR_RIGHT);
				p_matrix = ui_matrix_swipe_right;
					_update_vertical_index(u, TRUE);
//				u->vertical_index = 0; // Reset vertical index
				Y_SPRINTF("[UI] swipe right: %d", u->frame_index);
			}
			break;
		}
		case TOUCH_BUTTON_PRESS_HOLD:
		{
			_set_animation(ANIMATION_IRIS, TRANSITION_DIR_NONE);
			u->vertical_index = 0; // Reset vertical index
			p_matrix = NULL;
			u->frame_index = UI_DISPLAY_CAROUSEL_1;
			break;
		}
		case TOUCH_BUTTON_SINGLE:
		{
			_set_animation(ANIMATION_IRIS, TRANSITION_DIR_NONE);
			u->vertical_index = 0; // Reset vertical index
			p_matrix = ui_matrix_button;
			_update_finger_down_index(u);
			Y_SPRINTF("[UI] button single: %d", u->frame_index);
			break;
		}
		case TOUCH_FINGER_DOWN:
		{
			_set_animation(ANIMATION_IRIS, TRANSITION_DIR_NONE);
			p_matrix = ui_matrix_finger_down;
			_update_finger_down_index(u);
			N_SPRINTF("[UI] finger down: %d", u->frame_index);
			break;
		}
		case TOUCH_FINGER_UP:
		{
			_set_animation(ANIMATION_IRIS, TRANSITION_DIR_NONE);
			p_matrix = ui_matrix_finger_up;
			Y_SPRINTF("[UI] finger up: %d", u->frame_index);
			break;
		}
		case TOUCH_FINGER_LEFT:
		{
			_set_animation(ANIMATION_IRIS, TRANSITION_DIR_NONE);
			p_matrix = ui_matrix_finger_left;
			Y_SPRINTF("[UI] finger left: %d", u->frame_index);
			break;
		}
		case TOUCH_FINGER_RIGHT:
		{
			_set_animation(ANIMATION_IRIS, TRANSITION_DIR_NONE);
			p_matrix = ui_matrix_finger_right;
			N_SPRINTF("[UI] finger right, %d", u->frame_index);
			break;
		}
		case TOUCH_DOUBLE_TAP:
		{
			u->state = UI_STATE_APPEAR;
			u->frame_index = UI_DISPLAY_CAROUSEL_1;
			N_SPRINTF("[UI] double tapped: %d", u->frame_index);
			break;
		}
		default:
			break;
	}
	
	// Make sure that we get valid gesture prior to UI state switching.
	if (p_matrix == NULL) {
		u->frame_next_idx = u->frame_index;
	} else {

		if (gesture == TOUCH_FINGER_DOWN) {
			if (u->frame_current_idx == UI_DISPLAY_SMART_MESSAGE)
				u->frame_next_idx = u->frame_index;
			else {			
				u->frame_index = p_matrix[u->frame_index];
				u->frame_next_idx = p_matrix[u->frame_index];
			}

			Y_SPRINTF("[UI] finger down - frame index: %d, %d", u->frame_index, u->frame_next_idx);
		} else {
			u->frame_index = p_matrix[u->frame_index];
			u->frame_next_idx = p_matrix[u->frame_index];
			N_SPRINTF("[UI] OTHER - frame index: %d, %d", u->frame_index, u->frame_next_idx);
		}
		
		// Initialize smart reminder
		if (((gesture == TOUCH_FINGER_UP) && (u->frame_index == UI_DISPLAY_SMART_REMINDER)) || 
			  (cling.reminder.total>0)) 
		{
			cling.ui.vertical_index = REMINDER_get_time_at_index(0);
		}

	}
	
	return gesture;
}

static void _render_one_icon(I8U len, I8U *p_out, const I8U *p_in)
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

static I8U _fill_local_clock(char *string)
{
	I8U len;
		
	if (cling.ui.clock_sec_blinking) {
		cling.ui.clock_sec_blinking = FALSE;
		len = sprintf((char *)string, "%02d:%02d", cling.time.local.hour, cling.time.local.minute);
	} else {
		cling.ui.clock_sec_blinking = TRUE;
		len = sprintf((char *)string, "%02d %02d", cling.time.local.hour, cling.time.local.minute);
	}
	
	return len;
}


static I16U _render_middle_section_large(I8U len, I8U *string, I8U margin)
{
	I16U i, j;
	I8U *p0, *p1, *p2;
	const I8U *pin;
	I8U char_len;
	I16U offset=0;
	I16U extra_margin;
		
	for (i = 0; i < len; i++) {
		p0 = cling.ui.p_oled_up+offset+128;
		p1 = p0+128;
		p2 = p1+128;
		if (string[i] == 32) {
			char_len = 6;
		} else {
				// Digits in large fonts
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

static void _display_dynamic(I8U *pIn, I8U len2, I8U *string2)
{
	I8U *p, *pin;
	I8U *p6;
	I8U i, j, ptr, offset2, char_len;

	// indicator
	p = pIn;
	offset2 = 0;
	for (i = 0; i < len2; i++) {
		pin = (I8U *)(asset_content+asset_pos[string2[i]]);
		char_len = asset_len[string2[i]];
		for (j = 0; j < char_len; j++) {
				*p++ = (*pin++);
		}
	
		offset2 += char_len;
		if (i < len2) {
			offset2 += 3;
		}
		
		p += 5;
		
	}

	// Center it in the middle
	ptr = (128 - offset2)>>1;
	p = pIn;
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

static void _middle_row_render(I8U mode, BOOLEAN b_center)
{
	I16U i;
	I16U offset = 0; // Pixel offet at top row
	I8U *p0, *p1, *p2;
	I8U *p4, *p5, *p6;
	I8U string[64];
	I8U len=0, ptr, margin;
	WEATHER_CTX weather;
	I32U stat;
	I16U integer, fractional;

	// Clean up top row
	memset(cling.ui.p_oled_up, 0, 512);
	
	// Set up margin between characters
	margin = 3;
	
	// Render the left side 
	if (mode == UI_MIDDLE_MODE_CLOCK) {
		len = _fill_local_clock((char *)string);
	} else if (mode == UI_MIDDLE_MODE_STEPS) {
		TRACKING_get_activity(cling.ui.vertical_index, TRACKING_STEPS, &stat);
		len = sprintf((char *)string, "%d", stat);
	} else if (mode == UI_MIDDLE_MODE_DISTANCE) {
		TRACKING_get_activity(cling.ui.vertical_index, TRACKING_DISTANCE, &stat);
		integer = stat/1000;
		fractional = stat - integer * 1000;
		fractional /= 10;
		len = sprintf((char *)string, "%d.%02d", integer, fractional);
	} else if (mode == UI_MIDDLE_MODE_CALORIES) {
		TRACKING_get_activity(cling.ui.vertical_index, TRACKING_CALORIES, &stat);
		len = sprintf((char *)string, "%d", stat);
	} else if (mode == UI_MIDDLE_MODE_CALL) {
		len = sprintf((char *)string, "18221665107");
		FONT_load_characters(cling.ui.p_oled_up+128+40, (char *)string, 16, FALSE);
	} else if (mode == UI_MIDDLE_MODE_OTA) {
		#if 0
		len = sprintf((char *)string, "%d %%", cling.ota.percent);
		b_small_fonts = TRUE;
		bar_len = cling.ota.percent;
		if (bar_len >= 96)
			bar_len = 96;
		#endif
	} else if (mode == UI_MIDDLE_MODE_UV_IDX) {
		len = sprintf((char *)string, "5.4");
	} else if (mode == UI_MIDDLE_MODE_SLEEP) {
		TRACKING_get_activity(cling.ui.vertical_index, TRACKING_SLEEP, &stat);
		// Note: sleep is in seconds, so normalize it to minutes first
		stat /= 60; // Get sleep in minutes, and display in format: HH:MM
		integer = stat/60;
		fractional = stat - integer * 60;
		len = sprintf((char *)string, "%d", integer);
		string[len++]=ICON_MIDDLE_SLEEP_H;
		len += sprintf((char *)(string+len), "%02d", fractional);
		string[len++] = ICON_MIDDLE_SLEEP_M;
		string[len] = 0;
	} else if (mode == UI_MIDDLE_MODE_MESSAGE) {
		// In case iOS just delete a message
		len += sprintf((char *)string+len, "10");
		N_SPRINTF("[UI] smart message hit +++++");
	} else if (mode == UI_MIDDLE_MODE_PHONE_FINDER) {
		string[0] = ICON32_PHONE_FINDER_1;
		string[1] = ' ';
		string[2] = ICON32_PHONE_FINDER_2;
		string[3] = ' ';
		string[4] = ICON32_PHONE_FINDER_3;
		len = 5;
	} else if (mode == UI_MIDDLE_MODE_REMINDER) {
		
		if (cling.reminder.total>0) {
			
			if (cling.reminder.ui_hh == 0xff || cling.reminder.ui_mm == 0xff || cling.reminder.ui_hh >= 24 || cling.reminder.ui_mm >= 60) {
				len = sprintf((char *)string, "0:00");
			} else {
				len = sprintf((char *)string, "%d:%02d", cling.reminder.ui_hh, cling.reminder.ui_mm);
			}
			
		} else {
				len = sprintf((char *)string, "0:00");
		}
		
	} else if (mode == UI_MIDDLE_MODE_HEART_RATE) {
		if (cling.ui.vertical_index == 0) {
			if (TOUCH_is_skin_touched()) {
				
				len = 0;
				cling.hr.heart_rate_ready = 0;
				if (cling.hr.heart_rate_ready) {
					len = sprintf((char *)string, "%d", cling.hr.current_rate);
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
					
					_display_dynamic(cling.ui.p_oled_up+128+128, len, string);
					cling.ui.heart_rate_wave_index ++;
					
					return;
				}
			} else {
				len = 0;
				string[len++] = ICON_MIDDLE_NO_SKIN_TOUCH;
			}
		} else if (cling.ui.vertical_index == 1) {
			len = 0;
			string[len++] = ICON_MIDDLE_HIGH_TEMP_IDX;
			len += sprintf((char *)string+len, "%d", cling.hr.high_rate);
		} else {
			len = 0;
			string[len++] = ICON_MIDDLE_LOW_TEMP_IDX;
			len += sprintf((char *)string+len, "%d", cling.hr.low_rate);
		}
	} else if (mode == UI_MIDDLE_MODE_SKIN_TEMP) {
		if (cling.ui.vertical_index == 0) {
				integer = cling.therm.current_temperature/10;
				fractional = cling.therm.current_temperature - integer * 10;
				len = sprintf((char *)string, "%d.%d", integer, fractional);
				string[len++] = ICON_MIDDLE_CELCIUS_IDX;
		} else if (cling.ui.vertical_index == 1) {
			len = 0;
			string[len++] = ICON_MIDDLE_HIGH_TEMP_IDX;
			integer = cling.therm.high_temperature/10;
			fractional = cling.therm.high_temperature - integer * 10;
			len += sprintf((char *)string+len, "%d.%d", integer, fractional);
		} else {
			len = 0;
			string[len++] = ICON_MIDDLE_LOW_TEMP_IDX;
			integer = cling.therm.low_temperature/10;
			fractional = cling.therm.low_temperature - integer * 10;
			len += sprintf((char *)string+len, "%d.%d", integer, fractional);
		}
	} else if (mode == UI_MIDDLE_MODE_WEATHER) {
		#if 1

		if (cling.ui.vertical_index & 0x01) {
			len = 0;
			string[len++] = ICON_MIDDLE_LOW_TEMP_IDX;
			len += sprintf((char *)string+len, "19");
			string[len++] = ICON_MIDDLE_CELCIUS_IDX;
		} else {
			len = 0;
			string[len++] = ICON_MIDDLE_HIGH_TEMP_IDX;
			len += sprintf((char *)string+len, "25");
			string[len++] = ICON_MIDDLE_CELCIUS_IDX;
		}
		#else
		

		if (WEATHER_get_weather(cling.ui.vertical_index, &weather)) {
			len = 0;
			string[len++] = ICON_MIDDLE_HIGH_TEMP_IDX;
			len += sprintf((char *)string+len, "%d", weather.low_temperature);
			//string[len++] = ICON_MIDDLE_LOW_TEMP_IDX;
			//len += sprintf((char *)string+len, "%d", weather.high_temperature);
		} else {
			len = 0;
			string[len++] = ICON_MIDDLE_NO_SKIN_TOUCH;
		}
		#endif
		
	} else if (mode == UI_MIDDLE_MODE_BLE_CODE) {
		SYSTEM_get_ble_code(string);
		len = 4;
		b_center = TRUE;
	} else if (mode == UI_MIDDLE_MODE_LINKING) {
		
		string[0] = ICON32_PHONE_FINDER_1;
		string[1] = ' ';
		string[2] = ICON32_PHONE_FINDER_2;
		string[3] = ' ';
		string[4] = ICON32_PHONE_FINDER_4;
		len = 5;
		b_center = TRUE;
	}

	if (len > 0) {
		offset = _render_middle_section_large(len, string, margin);
	}
	
	// Shift all the display to the middle
	if (b_center) {
		p0 = cling.ui.p_oled_up+128;
		p1 = p0+128;
		p2 = p1+128;
		ptr = (128 - offset)>>1;
		
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
}

static I8U _render_top_sec(I8U *string, I8U len, I8U offset)
{
	I8U i, j, margin = 1;
	const I8U *pin;
	I8U *p0;
	
	for (i = 0; i < len; i++) {
		p0 = cling.ui.p_oled_up+offset; // Add extra 10 pixels to make it center
		if (string[i] == ' ') {
			offset += 4;
		} else {
			pin = asset_content+asset_pos[string[i]];
			for (j = 0; j < asset_len[string[i]]; j++) {
					*p0++ = (*pin++);
			}
			if (i != (len-1))
				offset += asset_len[string[i]] + margin;
			else
				offset += asset_len[string[i]];
		}
	}
	
	return offset;
}

static void _render_firmware_ver()
{
	I8U string[64];
	I16U len;
	I16U major;
	I16U minor;
	
	major = cling.system.mcu_reg[REGISTER_MCU_REVH]>>4;
	minor = cling.system.mcu_reg[REGISTER_MCU_REVH]&0x0f;
	minor <<= 8;
	minor |= cling.system.mcu_reg[REGISTER_MCU_REVL];
	
	len = sprintf((char *)string, "VER:%d.%d", major, minor);
	FONT_load_characters(cling.ui.p_oled_up+(128-len*6), (char *)string, 8, FALSE);
}

static void _render_2_indicator(I8U len1, const I8U *in1, I8U offset1, I8U len2, const I8U *in2, I8U offset2)
{
	I8U *p0, *pin;
	I8U j;

	// Render the left side (offset set to 60 for steps comment)
	p0 = cling.ui.p_oled_up+offset1+128;
	pin = (I8U *)in1;
	for (j = 0; j < len1; j++) {
			*p0++ = (*pin++);
	}
	
	// Render the left side (offset set to 60 for steps comment)
	p0 = cling.ui.p_oled_up+offset2+128+128+128;
	pin = (I8U *)in2;
	for (j = 0; j < len2; j++) {
			*p0++ = (*pin++);
	}
}

static void _render_indicator(I8U len1, const I8U *in1, I8U offset)
{
	I8U *p0, *pin, *p1;
	I8U j;

	// Render the left side (offset set to 60 for steps comment)
	p0 = cling.ui.p_oled_up+offset;
	p1 = p0 + 128;
	pin = (I8U *)in1;
	for (j = 0; j < len1; j++) {
			*p0++ = (*pin++);
			*p1++ = (*pin++);
	}
}


static void _left_icon_render(I8U mode)
{
	I16U i;
	I16U offset = 0; // Pixel offet at top row
	I8U string[64];
	I8U len=0;
	WEATHER_CTX weather;

	// Render the left side 
	if (mode == UI_TOP_MODE_NONE) {
	} else if (mode == UI_TOP_MODE_STEPS) {
			// Render the left side
		_render_one_icon(ICON_TOP_STEPS_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_STEPS);
	} else if (mode == 	UI_TOP_MODE_DISTANCE) {
		_render_one_icon(ICON_TOP_DISTANCE_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_DISTANCE);
	} else if (mode == UI_TOP_MODE_CALORIES) {
		_render_one_icon(ICON_TOP_CALORIES_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_CALORIES);
	} else if (mode == UI_TOP_MODE_SLEEP) {
		_render_one_icon(ICON_TOP_SLEEP_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_SLEEP);
	} else if (mode == UI_TOP_MODE_UV_IDX) {
		_render_one_icon(ICON_TOP_UV_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_UV);
	} else if (mode == UI_TOP_MODE_CALL) {
		_render_one_icon(ICON_TOP_CALLS_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_CALLS);
	} else if (mode == UI_TOP_MODE_MESSAGE) {
		if (cling.ui.clock_sec_blinking) {
			cling.ui.clock_sec_blinking = FALSE;
			_render_one_icon(ICON_TOP_MESSAGE_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_MESSAGE);
		} else {
			cling.ui.clock_sec_blinking = TRUE;
			// Empty icon to have blinking effects
		}

	} else if (mode == UI_TOP_MODE_REMINDER) {
		if (cling.reminder.state == REMINDER_STATE_SECOND_REMINDER) {
				if (cling.ui.clock_sec_blinking) {
					cling.ui.clock_sec_blinking = FALSE;
					_render_one_icon(ICON_TOP_REMINDER_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_REMINDER);
				} else {
					cling.ui.clock_sec_blinking = TRUE;
					// Empty icon to have blinking effects
				}
		} else {
		
				// Render top row heart rate icon
				_render_one_icon(ICON_TOP_REMINDER_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_REMINDER);
		}
	} else if (mode == UI_TOP_MODE_HEART_RATE) {
		if ((cling.ui.vertical_index == 0) && PPG_is_skin_touched()) {
				if (cling.ui.clock_sec_blinking) {
					cling.ui.clock_sec_blinking = FALSE;
					_render_one_icon(ICON_TOP_HEART_RATE_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_HEART_RATE);
				} else {
					cling.ui.clock_sec_blinking = TRUE;
					// Empty icon to have blinking effects
				}
		} else {
			// Render top row heart rate icon
			_render_one_icon(ICON_TOP_HEART_RATE_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_HEART_RATE);
		}
	} else if (mode == UI_TOP_MODE_SKIN_TEMP) {
		Y_SPRINTF("[UI] display skin temp (2)");
		_render_one_icon(ICON_TOP_SKIN_TEMP_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_SKIN_TEMP);
	} else if (mode == UI_TOP_MODE_WEATHER) {
		WEATHER_get_weather(cling.ui.vertical_index, &weather);
		_render_one_icon(asset_len[ICON_WEATHER_IDX+weather.type], cling.ui.p_oled_up+offset, asset_content+asset_pos[ICON_WEATHER_IDX+weather.type]);
	} else if (mode == UI_TOP_MODE_RETURN) {
		_render_one_icon(ICON_TOP_RETURN_LEN, cling.ui.p_oled_up, asset_content+ICON_TOP_RETURN);
	} else if (mode == UI_TOP_MODE_WORKOUT_START) {
		_render_2_indicator(ICON_TOP_WORKOUT_START_ICON_LEN, asset_content+ICON_TOP_WORKOUT_START_ICON, 5,
												ICON_TOP_WORKOUT_START_TEXT_LEN, asset_content+ICON_TOP_WORKOUT_START_TEXT, 0);
	} else if (mode == UI_TOP_MODE_WORKOUT_STOP) {
		_render_2_indicator(ICON_TOP_WORKOUT_STOP_ICON_LEN, asset_content+ICON_TOP_WORKOUT_STOP_ICON, 5,
												ICON_TOP_WORKOUT_STOP_TEXT_LEN, asset_content+ICON_TOP_WORKOUT_STOP_TEXT, 0);
	}

	
	if (len > 0) {
		for (i = 0; i < len; i++) {
			// Render the left side
			_render_one_icon(asset_len[string[i]], cling.ui.p_oled_up+offset, asset_content+asset_pos[string[i]]);
			
			offset += asset_len[string[i]] + 1;
		}
	}

}

static void _left_system_render(BOOLEAN b_charging, BOOLEAN b_ble_conn)
{	
	I8U *p0, *pin;
	I8U i, j, idx;
	I8U curr_batt_level = cling.system.mcu_reg[REGISTER_MCU_BATTERY];
	I8U perc[8] = {12, 24, 36, 48, 60, 72, 84, 96};
	I8U p_v = 0x1c;

	if (b_ble_conn) {
		p0 = cling.ui.p_oled_up + ICON_BOTTOM_IND_CHARGING_LEN + 2;
		pin = (I8U *)(asset_content+ICON_TOP_SMALL_BLE);
		for (j = 0; j < ICON_TOP_SMALL_BLE_LEN; j++) {
				*p0++ = (*pin++);
		}
	}
	
	// Render the right side (offset set to 60 for steps comment)
	p0 = cling.ui.p_oled_up;
	
	if (b_charging) {
		pin = (I8U *)(asset_content+ICON_BOTTOM_IND_CHARGING);
		for (j = 0; j < ICON_BOTTOM_IND_CHARGING_LEN; j++) {
				*p0++ = (*pin++);
		}
	} else {
		pin = (I8U *)(asset_content+ICON_BOTTOM_IND_NO_CHARGING);
		for (j = 0; j < ICON_BOTTOM_IND_NO_CHARGING_LEN; j++) {
				*p0++ = (*pin++);
		}
	}
	
	// Filling up the percentage
	curr_batt_level /= 11;
	
	if (curr_batt_level == 0)
		return;
	
	if (curr_batt_level > 9)
		curr_batt_level = 9;
	
	// Note: the battery button icon is 9 pixels of length
	p0 = cling.ui.p_oled_up+2;
	for (j = 0; j < curr_batt_level; j++) {
		*p0++ |= p_v;
	}
}

static void _render_calendar(SYSTIME_CTX time)
{
	I8U string[64];
	I8U len, offset=95;
	char *week[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};

	if (cling.ui.fonts_cn) {
		string[0] = ICON_DOW_IDX+time.dow;
		
		len = 1;
		
		len += sprintf((char *)(string+1), " %d",time.day);
		
		_render_top_sec(string, len, offset);
		
		len = sprintf((char *)string, " %d", time.day);
		FONT_load_characters(cling.ui.p_oled_up+(128-len*6), (char *)string, 8, FALSE);
	} else {
		len = sprintf((char *)string, "%s %d", week[time.dow], time.day);
		FONT_load_characters(cling.ui.p_oled_up+(128-len*6), (char *)string, 8, FALSE);
	}
	
}

static void _render_clock(SYSTIME_CTX time)
{
	I8U string[64];
	I8U len;

	len = sprintf((char *)string, "%d:%02d",time.hour, time.minute);
	FONT_load_characters(cling.ui.p_oled_up+(128-len*6), (char *)string, 8, FALSE);
}

static void _right_row_render(I8U mode)
{	
	I8U string[64];
	I8U len;
	SYSTIME_CTX delta;

	// Render the right side
	if (mode == UI_BOTTOM_MODE_NONE) {
	} else if (mode == UI_BOTTOM_MODE_2DIGITS_INDEX) {

		len = sprintf((char *)string, "%02d", cling.ui.vertical_index);
		FONT_load_characters(cling.ui.p_oled_up+(128-len*6), (char *)string, 8, FALSE);

	} else if (mode == UI_BOTTOM_MODE_CLOCK) {
		
		_render_clock(cling.time.local);
		
	} else if (mode == UI_BOTTOM_MODE_CALENDAR) {
		
		_render_calendar(cling.time.local);
		
	} else if (mode == UI_BOTTOM_MODE_DELTA_DATE_BACKWARD) {
		
		RTC_get_delta_clock_backward(&delta, cling.ui.vertical_index);
		_render_calendar(delta);
		
	} else if (mode == UI_BOTTOM_MODE_MAX) {
		_render_indicator(ICON_BOTTOM_IND_MAX_LEN, asset_content+ICON_BOTTOM_IND_MAX, 123);

	} else if (mode == UI_BOTTOM_MODE_FIRMWARE_VER) {
		_render_firmware_ver();
	} else if (mode == UI_BOTTOM_MODE_CHARGING_PERCENTAGE) {
		len = sprintf((char *)string, "%d %%", cling.system.mcu_reg[REGISTER_MCU_BATTERY]);
		FONT_load_characters(cling.ui.p_oled_up+(128-len*6), (char *)string, 8, FALSE);
	}
}

static void _render_screen()
{
		OLED_full_scree_show(cling.ui.p_oled_up);

		OLED_set_display(1);
}

static I8U _render_carousel_icon_single(I16U x, I8U char_len, I8U index)
{
	I8U j;
	I8U *p0, *p1, *p2;
	const I8U *pin;
	
	if (!index)
		return 0;

	// We might add some margin here
	p0 = cling.ui.p_oled_up+128+x;
	p1 = p0 + 128;
	p2 = p1 + 128;

	pin = asset_content+asset_pos[512+index];
	for (j = 0; j < char_len; j++) {
			*p0++ = (*pin++);
			*p1++ = (*pin++);
			*p2++ = (*pin++);
	}
	
	return char_len;
}

static void _core_display_carousel(I8U left, I8U top, I8U right, BOOLEAN b_render)
{
	I8U start_offset;
	I8U char_len;
	
	// Clean up top row
	memset(cling.ui.p_oled_up, 0, 512);
	
	// Render the left icon
	char_len = asset_len[512+left];
	start_offset = 0;
	_render_carousel_icon_single(start_offset, char_len, left);
	
	char_len = asset_len[512+top];
	start_offset = 42-char_len;
	start_offset >>= 1;
	start_offset += 42;
	_render_carousel_icon_single(start_offset, char_len, top);
	
	char_len = asset_len[512+right];
	start_offset = 128-char_len;
	_render_carousel_icon_single(start_offset, char_len, right);
	
	if (b_render) {
		// Finally, we render the frame
		_render_screen();
	}
}

static void _core_home_display_horizontal(I8U middle, I8U bottom, BOOLEAN b_render)
{
	BOOLEAN b_charging = FALSE;
	BOOLEAN b_conn = FALSE;
	
	// Main info
	_middle_row_render(middle, TRUE);

	// Info on the right
	if (BATT_is_charging())
		b_charging = TRUE;
	if (BTLE_is_connected())
		b_conn = TRUE;

	// Charging icon & BLE connection
	_left_system_render(b_charging, b_conn);
		
	// Info on the left
	_right_row_render(bottom);

	if (b_render) {
		// Finally, we render the frame
		_render_screen();
	}
}

static void _core_display_horizontal(I8U top, I8U middle, I8U bottom, BOOLEAN b_render)
{	
	// Main info
	_middle_row_render(middle, TRUE);

	// Info on the left
	_left_icon_render(top);
	
	_right_row_render(bottom);

	if (b_render) {
		// Finally, we render the frame
		_render_screen();
	}
}


static void _display_unauthorized_home()
{
	_core_home_display_horizontal(UI_MIDDLE_MODE_BLE_CODE, UI_BOTTOM_MODE_FIRMWARE_VER, TRUE);
}

static void _display_authenticating()
{
	_core_home_display_horizontal(UI_MIDDLE_MODE_LINKING, UI_BOTTOM_MODE_NONE, TRUE);
}

static void _display_tracking_stats(UI_ANIMATION_CTX *u, BOOLEAN b_render, I8U top, I8U middle, I8U bottom)
{
		N_SPRINTF("[UI] tracking frame: %d", u->vertical_index);
	if (u->vertical_index == 0) {
		_core_display_horizontal(top, middle, bottom, b_render);
	} else  {
		_core_display_horizontal(top, middle, UI_BOTTOM_MODE_DELTA_DATE_BACKWARD, b_render);
	}
}

static void _display_frame_tracker(BOOLEAN b_render)
{
	UI_ANIMATION_CTX *u = &cling.ui;
	
	switch (u->frame_index) {
		case UI_DISPLAY_TRACKER_STEP:
			_display_tracking_stats(u, b_render, UI_TOP_MODE_STEPS, UI_MIDDLE_MODE_STEPS, UI_BOTTOM_MODE_CLOCK);
			break;
		case UI_DISPLAY_TRACKER_DISTANCE:
			_display_tracking_stats(u, b_render, UI_TOP_MODE_DISTANCE, UI_MIDDLE_MODE_DISTANCE, UI_BOTTOM_MODE_CLOCK);
			break;
		case UI_DISPLAY_TRACKER_CALORIES:
			_display_tracking_stats(u, b_render, UI_TOP_MODE_CALORIES, UI_MIDDLE_MODE_CALORIES, UI_BOTTOM_MODE_CLOCK);
			break;
		case UI_DISPLAY_TRACKER_SLEEP:
			_display_tracking_stats(u, b_render, UI_TOP_MODE_SLEEP, UI_MIDDLE_MODE_SLEEP, UI_BOTTOM_MODE_CLOCK);
			break;
		case UI_DISPLAY_TRACKER_UV_IDX:
			Y_SPRINTF("[UI] uv index (1)");
			_display_tracking_stats(u, b_render, UI_TOP_MODE_UV_IDX, UI_MIDDLE_MODE_UV_IDX, UI_BOTTOM_MODE_CLOCK);
			break;
		default:
			break;
	}
}

static void _display_weather(UI_ANIMATION_CTX *u, BOOLEAN b_render)
{
		_core_display_horizontal(UI_TOP_MODE_WEATHER, UI_MIDDLE_MODE_WEATHER, UI_BOTTOM_MODE_CLOCK, b_render);
}

static void _display_frame_workout(BOOLEAN b_render)
{
	UI_ANIMATION_CTX *u = &cling.ui;
	I8U string1[32];
	I8U string2[32];
	I8U len2=0, len1=0;
	
	memset(cling.ui.p_oled_up, 0, 512);
	
	switch (u->frame_index) {
		case UI_DISPLAY_WORKOUT_WALKING:
		{
			if (cling.ui.fonts_cn) {
				len1 = sprintf((char *)string1, "??");
			} else {
				//display_one_Chinese_characters("zhou
				len1 = sprintf((char *)string1, "Walk");
			}
			len2 = sprintf((char *)string2, "-,,,,,,,");
			break;
		}
		case UI_DISPLAY_WORKOUT_RUNNING:
		{
			if (cling.ui.fonts_cn) {
				len1 = sprintf((char *)string1, "??");
			} else {
				//display_one_Chinese_characters("zhou
				len1 = sprintf((char *)string1, "Run");
			}
			len2 = sprintf((char *)string2, ",-,,,,,,");
			break;
		}
		case UI_DISPLAY_WORKOUT_ROW:
		{
			if (cling.ui.fonts_cn) {
				len1 = sprintf((char *)string1, "??");
			} else {
				//display_one_Chinese_characters("zhou
				len1 = sprintf((char *)string1, "Row");
			}
			len2 = sprintf((char *)string2, ",,-,,,,,");
			break;
		}
		case UI_DISPLAY_WORKOUT_ELLIPTICAL:
		{
			if (cling.ui.fonts_cn) {
				len1 = sprintf((char *)string1, "???");
			} else {
				//display_one_Chinese_characters("zhou
				len1 = sprintf((char *)string1, "Elliptical");
			}
			len2 = sprintf((char *)string2, ",,,-,,,,");
			break;
		}
		case UI_DISPLAY_WORKOUT_STAIRS:
		{
			if (cling.ui.fonts_cn) {
				len1 = sprintf((char *)string1, "???");
			} else {
				//display_one_Chinese_characters("zhou
				len1 = sprintf((char *)string1, "Stairs");
			}
			len2 = sprintf((char *)string2, ",,,,-,,,");
			break;
		}
		case UI_DISPLAY_WORKOUT_CYCLING:
		{
			if (cling.ui.fonts_cn) {
				len1 = sprintf((char *)string1, "??");
			} else {
				//display_one_Chinese_characters("zhou
				len1 = sprintf((char *)string1, "Cycle");
			}
			len2 = sprintf((char *)string2, ",,,,,-,,");
			break;
		}
		case UI_DISPLAY_WORKOUT_SWIMING:
		{
			if (cling.ui.fonts_cn) {
				len1 = sprintf((char *)string1, "???");
			} else {
				//display_one_Chinese_characters("zhou
				len1 = sprintf((char *)string1, "Aerobic");
			}
			len2 = sprintf((char *)string2, ",,,,,,-,");
			break;
		}
		case UI_DISPLAY_WORKOUT_OTHERS:
		{
			if (cling.ui.fonts_cn) {
				len1 = sprintf((char *)string1, "??");
			} else {
				//display_one_Chinese_characters("zhou
				len1 = sprintf((char *)string1, "Others");
			}
			len2 = sprintf((char *)string2, ",,,,,,,-");
			break;
		}
		case UI_DISPLAY_WORKOUT_OUTDOOR:
		{
			if (cling.ui.fonts_cn) {
				len1 = sprintf((char *)string1, "??");
			} else {
				//display_one_Chinese_characters("zhou
				len1 = sprintf((char *)string1, "Outdoor");
			}
			len2 = sprintf((char *)string2, "-,");
			break;
		}
		case UI_DISPLAY_WORKOUT_INDOOR:
		{
			if (cling.ui.fonts_cn) {
				len1 = sprintf((char *)string1, "??");
			} else {
				//display_one_Chinese_characters("zhou
				len1 = sprintf((char *)string1, "Indoor");
			}
			len2 = sprintf((char *)string2, ",-");
			break;
		}
		default:
			break;
	}
	
	FONT_load_characters(cling.ui.p_oled_up+128, (char *)string1, 16, TRUE);
	
	_display_dynamic(cling.ui.p_oled_up+128*3, len2, string2);
	
	_left_icon_render(UI_TOP_MODE_RETURN);
	
	if (b_render) {

		// Finally, we render the frame
		_render_screen();
	}
}

static void _display_stopwatch_core(I8U *string1, I8U len1, I8U *string2, I8U len2, I8U mode, BOOLEAN b_center)
{
	I8U *p0, *pin, *p1, *p2;
	I8U *p4, *p5, *p6;
	I8U i, j, ptr, offset1, char_len;

	// Reset frame buffer
	memset(cling.ui.p_oled_up, 0, 512);
	
	// Text line
	p0 = cling.ui.p_oled_up;
	p1 = p0 + 128;
	p2 = p1 + 128;
	offset1 = 0;
	for (i = 0; i < len1; i++) {
		pin = (I8U *)(asset_content+asset_pos[512+string1[i]]);
		char_len = asset_len[512+string1[i]];
		for (j = 0; j < char_len; j++) {
				*p0++ = (*pin++);
				*p1++ = (*pin++);
				*p2++ = (*pin++);
		}
		offset1 += char_len;
		p0 ++;
		p1 ++;
		p2 ++;
		if (i < len1)
			offset1 ++;
	}

	p0 = cling.ui.p_oled_up;
	p1 = p0+128;
	p2 = p1+128;
	if (b_center) {
		
		ptr = (128 - offset1)>>1;
		
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
	} else {
		ptr = 128 - offset1;
		
		if (ptr > 0) {
			p0 += 127; p4 = p0 - ptr;
			p1 += 127; p5 = p1 - ptr;
			p2 += 127; p6 = p2 - ptr;
			for (i = 0; i < offset1; i++) {
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
	
	if (len2 > 0) {
		// indicator
		p2 = cling.ui.p_oled_up+128+128+128+60;
		for (i = 0; i < len2; i++) {
			pin = (I8U *)(asset_content+asset_pos[string2[i]]);
			char_len = asset_len[string2[i]];
			for (j = 0; j < char_len; j++) {
					*p2++ = (*pin++);
			}
			p2 += 5;			
		}
	}
	
	_left_icon_render(mode);
}

static void _display_frame_setting(BOOLEAN b_render)
{
	_core_display_horizontal(UI_TOP_MODE_RETURN, UI_MIDDLE_MODE_BLE_CODE, UI_BOTTOM_MODE_FIRMWARE_VER, b_render);
}

static void _display_frame_stopwatch(BOOLEAN b_render)
{
	UI_ANIMATION_CTX *u = &cling.ui;
	I8U string1[32];
	I8U string2[32];
	I8U len1=0, len2 = 0;
	BOOLEAN b_center = TRUE;
	
	memset(cling.ui.p_oled_up, 0, 512);
	
	switch (u->frame_index) {
		case UI_DISPLAY_STOPWATCH_START:
		{
			len1 = sprintf((char *)string1, "0:00.00");
			len2 = 0;
			b_center = FALSE;
			_display_stopwatch_core(string1, len1, string2, len2, UI_TOP_MODE_WORKOUT_START, b_center);
			break;
		} 
		case UI_DISPLAY_STOPWATCH_STOP:
		{
			len1 = sprintf((char *)string1, "2:32.40");
			len2 = sprintf((char *)string2, "-,,");
			b_center = FALSE;
			_display_stopwatch_core(string1, len1, string2, len2, UI_TOP_MODE_WORKOUT_STOP, b_center);
			break;
		} 
		case UI_DISPLAY_STOPWATCH_RESULT:
		{
			len1 = sprintf((char *)string1, "8:50.39");
			len2 = 0;
			b_center = FALSE;
			_display_stopwatch_core(string1, len1, string2, len2, UI_TOP_MODE_RETURN, b_center);
			break;
		}
		case UI_DISPLAY_STOPWATCH_HEARTRATE:
		{
			len1 = sprintf((char *)string1, "113");
			len2 = sprintf((char *)string2, ",-,");
			_display_stopwatch_core(string1, len1, string2, len2, UI_TOP_MODE_HEART_RATE, b_center);
			break;
		}
		case UI_DISPLAY_STOPWATCH_CALORIES:
		{
			len1 = sprintf((char *)string1, "243");
			len2 = sprintf((char *)string2, ",,-");
			_display_stopwatch_core(string1, len1, string2, len2, UI_TOP_MODE_CALORIES, b_center);
			break;
		}
		default:
			break;
	}
	
	if (b_render) {

		// Finally, we render the frame
		_render_screen();
	}
}

static void _display_frame_smart(BOOLEAN b_render)
{
	UI_ANIMATION_CTX *u = &cling.ui;
	
	switch (u->frame_index) {
		case UI_DISPLAY_SMART_CALL:
			_core_display_horizontal(UI_TOP_MODE_CALL, UI_MIDDLE_MODE_CALL, UI_BOTTOM_MODE_CLOCK, b_render);
			break;
		case UI_DISPLAY_SMART_WEATHER:
			_display_weather(u, b_render);
			break;
		case UI_DISPLAY_SMART_MESSAGE:
			_core_display_horizontal(UI_TOP_MODE_MESSAGE, UI_MIDDLE_MODE_MESSAGE, UI_BOTTOM_MODE_CLOCK, b_render);
			break;
		case UI_DISPLAY_SMART_REMINDER:
			_core_display_horizontal(UI_TOP_MODE_REMINDER, UI_MIDDLE_MODE_REMINDER, UI_BOTTOM_MODE_2DIGITS_INDEX, b_render);
			break;
		case UI_DISPLAY_SMART_PHONE_FINDER:
			_core_display_horizontal(UI_TOP_MODE_NONE, UI_MIDDLE_MODE_PHONE_FINDER, UI_BOTTOM_MODE_NONE, b_render);
			break;
		default:
			break;
	}
}

static void _display_heart_rate(UI_ANIMATION_CTX *u, BOOLEAN b_render)
{
	//if (u->vertical_index == 0) {
	if (1) {
		// Current heart rate
		_core_display_horizontal(UI_TOP_MODE_HEART_RATE, UI_MIDDLE_MODE_HEART_RATE, UI_BOTTOM_MODE_CLOCK, b_render);
		#if 0
	} else if (u->vertical_index == 1) {
		// Highest heart rate in the day
		_core_display_horizontal(UI_TOP_MODE_HEART_RATE, UI_MIDDLE_MODE_HEART_RATE, UI_BOTTOM_MODE_CLOCK, b_render);
	} else if (u->vertical_index == 2) {
		// Lowest herat rate in the day
		_core_display_horizontal(UI_TOP_MODE_HEART_RATE, UI_MIDDLE_MODE_HEART_RATE, UI_BOTTOM_MODE_CLOCK, b_render);
		#endif
	}
}

static void _display_skin_temperature(UI_ANIMATION_CTX *u, BOOLEAN b_render)
{
	if (u->vertical_index == 0) {
		N_SPRINTF("[UI] display skin temp (1)");
		// Current skin temperature
		_core_display_horizontal(UI_TOP_MODE_SKIN_TEMP, UI_MIDDLE_MODE_SKIN_TEMP, UI_BOTTOM_MODE_CLOCK, b_render);
	} else if (u->vertical_index == 1) {
		// Highest skin temperature in the day
		_core_display_horizontal(UI_TOP_MODE_SKIN_TEMP, UI_MIDDLE_MODE_SKIN_TEMP, UI_BOTTOM_MODE_CLOCK, b_render);
	} else if (u->vertical_index == 2) {
		// Lowest skin temperature in the day
		_core_display_horizontal(UI_TOP_MODE_SKIN_TEMP, UI_MIDDLE_MODE_SKIN_TEMP, UI_BOTTOM_MODE_CLOCK, b_render);
	}
}

static void _display_frame_vital(BOOLEAN b_render)
{
	UI_ANIMATION_CTX *u = &cling.ui;
	
	switch (u->frame_index) {
		case UI_DISPLAY_VITAL_SKIN_TEMP:
			N_SPRINTF("[UI] vital: skin temp");
			_display_skin_temperature(u, b_render);
			break;
		case UI_DISPLAY_VITAL_HEART_RATE:
			N_SPRINTF("[UI] vital: heart rate");
			_display_heart_rate(u, b_render);
			break;
		default:
			break;
	}
}

static void _display_frame_carousel(BOOLEAN b_render)
{
	UI_ANIMATION_CTX *u = &cling.ui;
	
	switch (u->frame_index) {
		case UI_DISPLAY_CAROUSEL_1:
			_core_display_carousel(ICON32_HEART_RATE_IDX, ICON32_SKIN_TEMP_IDX, ICON32_SLEEP_IDX, b_render);
			break;
		case UI_DISPLAY_CAROUSEL_2:
			_core_display_carousel(ICON32_STEP_IDX, ICON32_CALORIES_IDX, ICON32_DISTANCE_IDX, b_render);
			break;
		case UI_DISPLAY_CAROUSEL_3:
			_core_display_carousel(ICON32_UV_IDX, ICON32_TIME_IDX, ICON32_WEATHER_IDX, b_render);
			break;
		case UI_DISPLAY_CAROUSEL_4:
			_core_display_carousel(ICON32_WORKOUT_IDX, ICON32_STOPWATCH_IDX, ICON32_MESSAGE_IDX, b_render);
			break;
		case UI_DISPLAY_CAROUSEL_5:
			_core_display_carousel(ICON32_SETTING_IDX, ICON32_PHONEFIND_IDX, ICON32_NONE_CAROUSEL, b_render);
			break;
		default:
			break;
	}
}

static void _display_frame_ota(BOOLEAN b_render)
{
		_core_display_horizontal(UI_TOP_MODE_NONE, UI_MIDDLE_MODE_OTA, UI_BOTTOM_MODE_CLOCK, b_render);
}

static void _display_frame_home(UI_ANIMATION_CTX *u, BOOLEAN b_render)
{
		if (BATT_is_charging()) {
			_core_home_display_horizontal(UI_MIDDLE_MODE_CLOCK, UI_BOTTOM_MODE_CHARGING_PERCENTAGE, b_render);
		} else {
			_core_home_display_horizontal(UI_MIDDLE_MODE_CLOCK, UI_BOTTOM_MODE_CALENDAR, b_render);
		}
}

static void _display_frame_appear(I8U index, BOOLEAN b_render)
{
	UI_ANIMATION_CTX *u = &cling.ui;
	
	if (index == UI_DISPLAY_HOME) {
		_display_frame_home(u, b_render);
		u->frame_cached_index = index;
	} else if ((index >= UI_DISPLAY_TRACKER) && (index <= UI_DISPLAY_TRACKER_END)) {
		_display_frame_tracker(b_render);
		u->frame_cached_index = index;
	} else if ((index >= UI_DISPLAY_SMART) && (index <= UI_DISPLAY_SMART_END)) {
		_display_frame_smart(b_render);
		u->frame_cached_index = index;
	} else if ((index >= UI_DISPLAY_VITAL) && (index <= UI_DISPLAY_VITAL_END)) {
		_display_frame_vital(b_render);
		u->frame_cached_index = index;
	} else if ((index >= UI_DISPLAY_WORKOUT) && (index <= UI_DISPLAY_WORKOUT_END)) {
		_display_frame_workout(b_render);
		u->frame_cached_index = index;
	} else if ((index >= UI_DISPLAY_STOPWATCH) && (index <= UI_DISPLAY_STOPWATCH_END)) {
		_display_frame_stopwatch(b_render);
		u->frame_cached_index = index;
	} else if ((index >= UI_DISPLAY_SETTING) && (index <= UI_DISPLAY_SETTING_END)) {
		_display_frame_setting(b_render);
		u->frame_cached_index = index;
	} else if ((index >= UI_DISPLAY_CAROUSEL) && (index <= UI_DISPLAY_CAROUSEL_END)) {
		_display_frame_carousel(b_render);
	} else if (index == UI_DISPLAY_OTA) {
		_display_frame_ota(b_render);
	} else {
	}
}

static void _render_logo()
{
	I16U i;
	I16U strt_offset;
	I8U *p0, *p1, *p2, *p3;
	const I8U *pin;

	memset(cling.ui.p_oled_up, 0, 512);
	strt_offset = (128-ICON32_LOGO_LEN)>>1;
	p0 = cling.ui.p_oled_up+strt_offset;
	p1 = p0+128;
	p2 = p1+128;
	p3 = p2+128;
	pin = asset_content+ICON32_LOGO;
	for (i = 0; i < ICON32_LOGO_LEN; i++) {
			*p0++ = (*pin++);
			*p1++ = (*pin++);
			*p2++ = (*pin++);
			*p3++ = (*pin++);
	}

	// Finally, we render the frame
	OLED_full_scree_show(cling.ui.p_oled_up);

	OLED_set_display(1);
	
}

static void _display_charging(I16U perc)
{
	I16U i, j;
	I16U fill_len;
	I8U *p0, *p1;
	I16U offset = 0; // Pixel offet at top row
	I8U string[64];
	I8U len=0;
		const I8U *pin;

	// Render the icon the first
	memset(cling.ui.p_oled_up, 0, 512);
	if (BATT_is_charging()) {
		string[len++] = ICON_MIDDLE_CHARGING_BATT_IDX;
		string[len++] = ICON_MIDDLE_CHARGING_ICON_IDX;
	} else {
		string[len++] = ICON_MIDDLE_CHARGING_BATT_IDX;
	}
	
	// Rendering the icon
	offset = 15;
	for (i = 0; i < len; i++) {
		p0 = cling.ui.p_oled_up+offset+128;
		p1 = p0+128;
		pin = asset_content+asset_pos[256+string[i]];
		for (j = 0; j < asset_len[256+string[i]]; j++) {
				*p0++ = (*pin++);
				*p1++ = (*pin++);
		}
		
		if (i != (len-1))
			offset += asset_len[256+string[i]] + 5;
		else
			offset += asset_len[256+string[i]];
	}
	
	// Shift up by 6 pixels
	//_frame_buffer_vertical_adj(-6);
	
	// Filling up the percentage
	//perc = cling.system.mcu_reg[REGISTER_MCU_BATTERY];
	if (perc >= 100) {
		perc = 100;
		fill_len = 43;
	} else {
		fill_len = perc*43/100;
	}
	
	// Filling the percentage
	offset = 15;
	p0 = cling.ui.p_oled_up+2+128+offset;
	p1 = p0+128;
	for (i = 0; i < fill_len; i++) {
		*p0++ |= 0xfc;
		*p1++ |= 0x3f;
	}
	
	// show the percentage
	len = sprintf((char *)string, "%d%%", perc);
	FONT_load_characters(cling.ui.p_oled_up+128+90, (char *)string, 16, FALSE);
	
	// Finally, we render the frame
	OLED_full_scree_show(cling.ui.p_oled_up);

	OLED_set_display(1);
	
}

void UI_switch_state(I8U state, I32U interval)
{
	UI_ANIMATION_CTX *u = &cling.ui;
	
	u->state_init = TRUE;
	u->state = state;
	u->frame_interval = interval;
	u->display_to_base = CLK_get_system_time();
	
	N_SPRINTF("[UI] state switching: %d, %d", state, u->display_to_base);
}

BOOLEAN UI_is_idle()
{
	return (cling.ui.state == UI_STATE_IDLE);
}



#define OVERALL_PANNING_FRAMES 10

const I8U panning_table[] = {
	16, 16, 16, 16, 16, 16, 16, 16, 0, 0, 10
};

static void _frame_buffer_move_forward(I8U offset, BOOLEAN b_carousel)
{
	I8U *p0;
	I8U len = 128 - offset;
	int i;

		p0 = cling.ui.p_oled_up;
	
	for (i = 0; i < 4; i++) {
		memcpy(p0, p0+offset, len); p0 += 128;
	}
	
		p0 = cling.ui.p_oled_up+len;
	
	for (i = 0; i < 4; i++) {
		memset(p0, 0, offset); p0 += 128;
	}
}

static void _frame_buffer_move_backward(I8U offset, BOOLEAN b_carousel)
{
	I8U *p0;
	I8U len = 128 - offset;
	int i;

	p0 = cling.ui.p_oled_up;

	for (i = 0; i < 4; i++) {
		memcpy(p0+offset, p0, len); p0 += 128;
	}
	
	p0 = cling.ui.p_oled_up;
	
	for (i = 0; i < 4; i++) {
		memset(p0, 0, offset); p0 += 128;
	}
}

const I8U iris_tab[4][12] = {
 {128,0,0,
	48,32,48,
	48,32,48,
	128,0,0},
 {128,0,0,
	32,64,32,
	32,64,32,
	128,0,0},
 {16,96,16,
	16,96,16,
	16,96,16,
	16,96,16},
 {8,112,8,
	8,112,8,
	8,112,8,
	8,112,8},
};

static void _iris_frame_core(I8U offset)
{
	I8U *p0;
	I8U i;
	I8U len, index;
	const I8U *p_tab;
	
	p0 = cling.ui.p_oled_up;

	if (offset == 1) {
		memset(p0, 0, 512);
		return;
	} else if (offset == 5) {
		return;
	}
	
	p_tab = iris_tab[offset-2];
	for (i = 0; i < 4; i++) {
		len = *p_tab++; // reset  
		index = len;
		memset(p0,0,len);
		
		index += *p_tab++;  // Keep value
		
		len = *p_tab++; // reset
		memset(p0+index,0,len); //
		p0 += 128;
	}
	
}

static BOOLEAN _frame_panning(I8U dir, BOOLEAN b_out, BOOLEAN b_carousel)
{
	UI_ANIMATION_CTX *u = &cling.ui;
	I8U shift_pos = 0;
	I8U i;
	
	// Update animation index
	u->animation_index ++;
	
	if (u->animation_index >= panning_table[OVERALL_PANNING_FRAMES]) {
		u->animation_index = 0;
		return TRUE;
	}

	// Shift screen
	if (b_out) {
		// Panning out current frame
		_display_frame_appear(u->frame_current_idx, FALSE);
		
		for (i = 0; i < u->animation_index; i++) {
			shift_pos += panning_table[i];
		}
		
		if (dir == TRANSITION_DIR_LEFT) {
			_frame_buffer_move_forward(shift_pos, b_carousel);
		} else {
			_frame_buffer_move_backward(shift_pos, b_carousel);
		}
	} else {
		// Panning in next frame
		_display_frame_appear(u->frame_next_idx, FALSE);
		shift_pos = 128;
		for (i = 0; i < u->animation_index; i++) {
			shift_pos -= panning_table[i];
		}
		
		if (dir == TRANSITION_DIR_LEFT) {
			_frame_buffer_move_backward(shift_pos, b_carousel);
		} else {
			_frame_buffer_move_forward(shift_pos, b_carousel);
		}
	}
	
	// update frame buffer and render it
	_render_screen();
	
	return FALSE;
}

static BOOLEAN _frame_iris()
{
	UI_ANIMATION_CTX *u = &cling.ui;
	
	// Update animation index
	u->animation_index ++;
	
	if (u->animation_index >= 6) {
		u->animation_index = 0;
		return TRUE;
	}

	// Iris  in next frame
	_display_frame_appear(u->frame_next_idx, FALSE);
	
	//
	_iris_frame_core(u->animation_index);
		
	
	// update frame buffer and render it
	_render_screen();
	
	return FALSE;
}

static void _frame_animating()
{
	UI_ANIMATION_CTX *u = &cling.ui;
	BOOLEAN b_carousel = FALSE;
	
	if ((u->frame_index >= UI_DISPLAY_CAROUSEL) && (u->frame_index <= UI_DISPLAY_CAROUSEL_END)) {
		b_carousel = TRUE;
	}

	switch (u->animation_mode) {
		case ANIMATION_PANNING_OUT:
			if (_frame_panning(u->direction, TRUE, b_carousel)) {
				u->animation_mode = ANIMATION_PANNING_IN;
			}
			break;
		case ANIMATION_PANNING_IN:
			if (_frame_panning(u->direction, FALSE, b_carousel)) {
				u->state = UI_STATE_APPEAR;
			}
			break;
		case ANIMATION_TILTING_OUT:
			break;
		case ANIMATION_TILTING_IN:
			break;
		case ANIMATION_IRIS:
			if (_frame_iris()) {
				u->state = UI_STATE_APPEAR;
			}
			break;
		default:
			break;
	}
}
#if 0
static I8U uistate;
static BOOLEAN uiactive;
#endif
extern I32U sim_started;

void UI_state_machine()
{
	UI_ANIMATION_CTX *u = &cling.ui;
	I32U t_curr = CLK_get_system_time();
	I32U t_diff, t_threshold;
	#if 0
	if (uiactive != u->display_active) {
		uiactive = u->display_active;
		N_SPRINTF("[UI] new active state: %d @ %d", u->state, CLK_get_system_time());
	}
	#endif
	if (!u->display_active)
		return;
	#if 0
	if (uistate != u->state) {
		uistate = u->state;
		Y_SPRINTF("[UI] new state: %d @ %d", u->state, CLK_get_system_time());
	}
	#endif
	switch (u->state) {
		case UI_STATE_IDLE:
			break;
		case UI_STATE_CLING_START:
			if (u->state_init) {
				u->state_init = FALSE;
				_render_logo();
			} else if (t_curr > u->display_to_base + u->frame_interval) {
				UI_switch_state(UI_STATE_HOME, 1000);
			}
			break;
		case UI_STATE_CLOCK_GLANCE:
		{
			if (u->state_init) {
				u->state_init = FALSE;
				if (BATT_is_charging() || BATT_is_low_battery()) {
					Y_SPRINTF("[UI] LOW BATTERY: %d", cling.system.mcu_reg[REGISTER_MCU_BATTERY]);
					UI_switch_state(UI_STATE_LOW_POWER, 1000);
				} else {
					_display_frame_home(u, TRUE);
				}
				N_SPRINTF("[UI] clock glance");
			} else if ((t_curr > u->display_to_base + 1000) || TOUCH_new_gesture()) {
				N_SPRINTF("[UI] switch to sensing mode, %d, %d", t_curr, u->display_to_base);
				UI_switch_state(UI_STATE_TOUCH_SENSING, 2000);
				u->touch_time_stamp = t_curr;
			}
			break;
		}
		case UI_STATE_HOME:
			if (u->state_init) {
				u->state_init = FALSE;
				if (LINK_is_authorized()) {

					if (BATT_is_low_battery())
					{
						N_SPRINTF("[UI] LOW BATTERY: %d", cling.system.mcu_reg[REGISTER_MCU_BATTERY]);
						u->state = UI_STATE_LOW_POWER;
					} else {
						UI_switch_state(UI_STATE_APPEAR, 1000);
					}
					// Let's give UI screen about 4 more seconds once we have clock shown up
					// Just to be nice!
					u->touch_time_stamp = t_curr;
				} else {
					u->prefer_state = UI_STATE_AUTHORIZATION;
					
					UI_switch_state(UI_STATE_AUTHORIZATION, 1000);
				}
			}
			break;
		case UI_STATE_AUTHORIZATION:
			if (u->state_init) {
				u->state_init = FALSE;
				_display_unauthorized_home();
			} else if (t_curr > u->display_to_base + u->frame_interval) {
				u->display_to_base = t_curr;
				Y_SPRINTF("[UI] display to at authorization: %d", u->display_to_base);
				u->frame_interval = 1000;
				if (LINK_is_authorizing()) {
					t_diff = t_curr - cling.link.link_ts;
					if (t_diff < 60000) {
						_display_authenticating();
					} else {
						cling.link.b_authorizing = FALSE;
					}
				} else {
					if (OTA_if_enabled()) {
						u->frame_index = UI_DISPLAY_OTA;
						UI_switch_state(UI_STATE_APPEAR, 1000);
					} else {
						UI_switch_state(UI_STATE_HOME, 0);
					}
				}
			}
			break;
		case UI_STATE_REMINDER:
		{
			if (u->state_init) {
				u->state_init = FALSE;
				u->frame_index = UI_DISPLAY_SMART_REMINDER;
				_display_frame_smart(TRUE);
			} else {
				if (t_curr > u->display_to_base + u->frame_interval) {
					if (LINK_is_authorized()) {
						u->frame_index = UI_DISPLAY_PREVIOUS;
						UI_switch_state(UI_STATE_TOUCH_SENSING, 4000);
					} else {
						UI_switch_state(UI_STATE_HOME, 4000);
					}
				}
			}
			break;
		}
		case UI_STATE_NOTIFIC:
		{
			if (u->state_init) {
				u->state_init = FALSE;
				u->frame_index = UI_DISPLAY_SMART_MESSAGE;
				_display_frame_smart(TRUE);
			} else {
				if (t_curr > u->display_to_base + u->frame_interval) {
					if (LINK_is_authorized()) {
						UI_switch_state(UI_STATE_TOUCH_SENSING, 4000);
					} else {
						UI_switch_state(UI_STATE_HOME, 4000);
					}
				}
			}
			break;
		}
		case UI_STATE_LOW_POWER:
			if (u->state_init) {
				Y_SPRINTF("[UI] low power shown");
				u->state_init = FALSE;
				_display_charging(cling.system.mcu_reg[REGISTER_MCU_BATTERY]);
				u->touch_time_stamp = t_curr;
			}
			else {
				if (t_curr > (u->touch_time_stamp+2000)) {
					UI_switch_state(UI_STATE_APPEAR, 1000);
				}
			}
			break;
		case UI_STATE_CHARGING:
			if (u->state_init) {
				u->state_init = FALSE;
				UI_switch_state(UI_STATE_HOME, 0);
			}
			else {
				if (t_curr > u->display_to_base + u->frame_interval) {
					
					//if (LINK_is_authorized())
						//UI_switch_state(u->prefer_state, 4000);
					//else
						UI_switch_state(UI_STATE_HOME, 4000);
				}
			}
			break;
		case UI_STATE_TOUCH_SENSING:
			// Don't do anything if system is updating firmware
			if (OTA_if_enabled()) {
				
				// Skip all the gestures
				u->frame_index = UI_DISPLAY_OTA;
				UI_switch_state(UI_STATE_APPEAR, 1000);
#if 0
				if (cling.system.mcu_reg[REGISTER_MCU_BATTERY] > 90) 
#endif
				{
					u->touch_time_stamp = t_curr;
				}
#ifndef USING_VIRTUAL_ACTIVITY_SIM
			} else if (!LINK_is_authorized()) {
				// Go to home page if device is unauthorized.
				UI_switch_state(UI_STATE_HOME, 4000);
#endif
			} else if (LINK_is_authorizing()) {
					u->prefer_state = UI_STATE_AUTHORIZATION;
					
					UI_switch_state(UI_STATE_AUTHORIZATION, 1000);
			} else if (_ui_touch_sensing())  {
				N_SPRINTF("[UI] new gesture --- %d", t_curr);
				u->touch_time_stamp = t_curr;
			} else {
				// Otherwise, blinking every 800 ms.
				if (t_curr > u->display_to_base + u->frame_interval) {
					UI_switch_state(UI_STATE_APPEAR, 400);
				}
			}
			
#ifndef USING_VIRTUAL_ACTIVITY_SIM
			
			// 4 seconds screen dark expiration
			// for heart rate measuring, double the Screen ON time.
			if (u->true_display)
				t_threshold = cling.user_data.screen_on_general; // in second
			else
				t_threshold = 2;
			t_threshold *= 1000; // second -> milli-second
			if (u->frame_index == UI_DISPLAY_VITAL_HEART_RATE) {
				if (TOUCH_is_skin_touched()) {
					t_threshold = cling.user_data.screen_on_heart_rate; // in second
					t_threshold *= 1000; // second -> milli-second
				}
			}
			
			// If we don't see any gesture in 4 seconds, dark out screen
			if (t_curr > (u->touch_time_stamp+t_threshold)) {
				N_SPRINTF("[UI] gesture monitor time out - %d", t_threshold);
				if (BATT_is_charging()) {
					if (t_curr > (u->touch_time_stamp+10000)) {
						u->state = UI_STATE_DARK;
						TRACKING_enter_low_power_mode();
					}
				} else {
					u->state = UI_STATE_DARK;
				}
			}
#endif
			break;
		case UI_STATE_ANIMATING:
			// perform frame animation before a final apperance
			_frame_animating();
			break;
		case UI_STATE_APPEAR:
			#if 1
			sim_started = 1;
		#endif
			if (u->state_init) {
				N_SPRINTF("[UI] time: %d, index: %d", t_curr, u->frame_index);
				u->state_init = FALSE;
				if (u->frame_index == UI_DISPLAY_PREVIOUS) {
					u->frame_index = u->frame_cached_index;
					Y_SPRINTF("[UI] load cached index: %d", u->frame_index);
				}
				// Display this frame.
				_display_frame_appear(u->frame_index, TRUE);
			} else {
				UI_switch_state(UI_STATE_TOUCH_SENSING, 500);
			}
			break;
		case UI_STATE_DARK:
			if (t_curr > (u->display_to_base+u->frame_interval)) {
				// Turn off OLED panel
				OLED_set_panel_off();
				u->state = UI_STATE_IDLE;
				u->vertical_index = 0;
				u->true_display = FALSE;
				
				// Remember the last screen so that 
#if 0
				if ((u->frame_cached_index == UI_DISPLAY_VITAL_HEART_RATE)||
						(u->frame_cached_index == UI_DISPLAY_VITAL_SKIN_TEMP) ||
						(u->frame_cached_index == UI_DISPLAY_TRACKER_SLEEP))
#else
				if (u->frame_cached_index == UI_DISPLAY_VITAL_HEART_RATE)
#endif
				{
					u->frame_index = UI_DISPLAY_CAROUSEL_2;
				} 
#if 0
				else if ((u->frame_cached_index == UI_DISPLAY_TRACKER_STEP) ||
						(u->frame_cached_index == UI_DISPLAY_TRACKER_CALORIES) ||
						(u->frame_cached_index == UI_DISPLAY_TRACKER_DISTANCE))
				{
					u->frame_index = UI_DISPLAY_CAROUSEL_3;
				} 
#endif
				else {
					u->frame_index = UI_DISPLAY_PREVIOUS;
				}
				if (BATT_is_charging()) {
					u->frame_index = UI_DISPLAY_HOME;
				}
				
				// Set Touch IC power mode accordingly
				if (cling.user_data.b_screen_press_hold_1) {
					cling.touch.power_new_mode = TOUCH_POWER_MIDIAN_200MS;
				} else {
					cling.touch.power_new_mode = TOUCH_POWER_LOW_2000MS;
				}		
				cling.touch.state = TOUCH_STATE_MODE_SET;
				cling.lps.b_touch_deep_sleep_mode = FALSE;
			}
			break;
		default:
			break;
	}
}

void UI_turn_on_display(UI_ANIMATION_STATE state, I32U time_offset)
{
	// Start 20 ms timer for screen rendering
	SYSCLK_timer_start();
	cling.touch.power_new_mode = TOUCH_POWER_HIGH_20MS;
	
	// Turn on OLED panel
	OLED_set_panel_on();
	
	// touch time update
	cling.ui.touch_time_stamp = CLK_get_system_time()-time_offset;
	cling.touch.state = TOUCH_STATE_MODE_SET;
	cling.ui.true_display = TRUE;

	if (state != UI_STATE_IDLE) {
		UI_switch_state(state, 0);
	} else {
		if (UI_is_idle()) {
			// UI initial state, A glance of current time
			UI_switch_state(UI_STATE_CLOCK_GLANCE, 0);
		}
	}
}
#if 0
void _loading_ch_fonts()
{
	I8U row, height, len;
	I8U *ptr;
	row = 1;
	height = 16; // 8;
	len = FONT_load_characters(ptr, "xxxx", height);
}
#endif
