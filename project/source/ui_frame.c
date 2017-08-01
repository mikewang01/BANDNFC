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

const char press_ch[] = "长按";
const char press_th[] = "長按";
const	char *const button_hold_name[3] = {"HOLD", press_ch, press_th};	

const char km_ch[] = "公里";
const char km_th[] = "公裏";
const char ml_ch[] = "英里";
const char ml_th[] = "英裏";
const char *const distance_unit_name[3][2] = {{"KM", "ML"},{km_ch, ml_ch},{km_th, ml_th}};	

const char step_ch[] = "步数";
const char step_k_ch[] = "千步";
const char step_10k_ch[] = "万步";
const char step_10k_th[] = "萬步";
const char *const vertical_step_name[3][3] = {{"STEP", "K", "K"},
																							{step_ch, step_k_ch, step_10k_ch},
																							{step_ch, step_k_ch, step_10k_th}};
	
const char min_ch[] = "分钟";
const char min_th[] = "分鍾";
const char *const active_time_name[3] = {"MIN",min_ch,min_th};		
 
const char pace_ch[] = "速度";
const	char *const pace_name[3] = {"PACE", pace_ch, pace_ch};

const char avg_ch[] = "平均";
const	char *const avg_name[3] = {"AVG", avg_ch, avg_ch};		

const char hr_ch[] = "心率";
const	char *const heart_rate_name[3] = {"HR", hr_ch, hr_ch};	

const char cadence_ch[] = "步频";
const char cadence_th[] = "步頻";
const	char *const cadence_rate_name[3] = {"RATE", cadence_ch, cadence_th};	

const char spm_ch[] = "步分";
const	char *const cadence_spm_name[3] = {"SPM", spm_ch, spm_ch};		
	
const char stride_ch[] = "步幅";
const	char *const horizontal_stride_name[3] = {"STRIDE", stride_ch, stride_ch};
const	char *const vertical_stride_name[3] = {"STRD", stride_ch, stride_ch};

const char cm_ch[] = "厘米";
const char inch_ch[] = "英寸";
const char *const stride_unit_name[3][2] = {{"CM", "IN"},{cm_ch, inch_ch},{cm_ch, inch_ch}};		

const char calorie_name_ch[] = "热量";
const char calorie_name_th[] = "熱量";
const	char *const calories_name[3] = {"BURN", calorie_name_ch, calorie_name_th};		

const char calorie_ch[] = "大卡";
const char calorie_k_ch[] = "K";
const	char *const calories_unit_horizontal_name[3] = {"CALS", calorie_ch, calorie_ch};
const char *const calories_unit_vertical_name[3][2] = {{"CAL", "K"},{calorie_ch, calorie_k_ch},{calorie_ch, calorie_k_ch}};

const char time_ch[] = "耗时";
const char time_th[] = "耗時";
const	char *const running_time_name[3] = {"TIME", time_ch, time_th};

const char distance_ch[] = "里程";
const char distance_th[] = "裏程";
const char *const running_distance_name[3] = {"RUN", distance_ch, distance_th};	

const char start_ch[] = "准备";
const char end_ch[] = "结束";
const char end_th[] = "結束";
const	char *const stop_run_name[3] = {"STOP ", end_ch, end_th};	
const	char *const start_run_name[3] = {"NOW ", start_ch, start_ch};	
	
const char run_ch[] = "跑步";
const char workout_ch[] = "运动";
const char workout_th[] = "運動";
const char cycling_ch[] = "骑行";
const char cycling_th[] = "騎行";
const	char *const training_run_name[3] = {"RUN ", run_ch, run_ch};	
const	char *const gym_workout_run_name[3] = {"RUN ", workout_ch, workout_th};			
const	char *const cycling_run_name[3] = {"BIKE ", cycling_ch, cycling_th};	
	
const char *const ready_indicator_name[4] = {"3","2","1","GO"};

const char speed_ch[] = "速度";
const char *const run_speed_name[3] = {"SPED", speed_ch, speed_ch};		
const char *const speed_unit_name[2] = {"KM/H", "ML/H"};	

const char no_ble_ch[] = "没有蓝牙";
const char no_ble_th[] = "没有藍牙";
const char *const no_ble_horizontal_name[3] = {"NO BLE", no_ble_ch, no_ble_th};	

const char no_ch[] = "没有";
const char ble_ch[] = "蓝牙";
const char ble_th[] = "藍牙";
const char *const no_ble_vertical_name_1[3] = {"NO", no_ch, no_ch};		
const char *const no_ble_vertical_name_2[3] = {"BLE", ble_ch, ble_th};		

const char *const stopwatch_stop_name[3] = {"END", end_ch, end_th};

const	char *const run_start_name[3] = {"RUN ", run_ch, run_ch};	

const char analysis_ch[] = "记录";
const char analysis_th[] = "記錄";
const	char *const run_analysis_name[3] = {"MORE", analysis_ch, analysis_th};		

const char idle_ch[] = "该活动一下了";
const char idle_th[] = "該活動一下了";
const	char *const idle_alart_name[] = {"time for move", idle_ch, idle_th};
	
const char mon_ch[] = "周一";
const char mon_th[] = "周壹";
const char tue_ch[] = "周二";
const char wed_ch[] = "周三";
const char thu_ch[] = "周四";
const char fri_ch[] = "周五";
const char sat_ch[] = "周六";
const char sun_ch[] = "周日";
const char *const week_name[3][7] = {{"MON",  "TUE",  "WED",  "THU",  "FRI",  "SAT",  "SUN"},
	                                   {mon_ch, tue_ch, wed_ch, thu_ch, fri_ch, sat_ch, sun_ch},
	                                   {mon_th, tue_ch, wed_ch, thu_ch, fri_ch, sat_ch, sun_ch}};	

#ifdef _CLINGBAND_UV_MODEL_	
const char *const air_level_name[1][7] = {{"--","GOOD", "GOOD",     "POOR",      "POOR", "POOR", "POOR"}};																				 
#else																	 
const char good_ch[] = "优质";				 
const char good_th[] = "優質";
const char moderate_ch[] = "良好";
const char light_ch[] = "轻度";
const char light_th[] = "輕度";
const char bad_ch[] = "中度";
const char worse_ch[] = "重度";
const char hazardous_ch[] = "严重";
const char hazardous_th[] = "嚴重";
const char *const air_level_name[4][7] = {{"--","Good", "Moderate", "Unhealthy", "Poor", "Poor", "hazardous"},
	                                        {"--","GOOD", "GOOD",     "POOR",      "POOR", "POOR", "POOR"},
	                                        {"--", good_ch, moderate_ch, light_ch, bad_ch, worse_ch, hazardous_ch},
												                  {"--", good_th, moderate_ch, light_th, bad_ch, worse_ch, hazardous_th}};																			
#endif		
								
const char treadmill_ch[] = "室内跑步";
const char bike_ch[] = "单车";
const char bike_th[] = "單車";
const char stairs_ch[] = "爬楼梯机";
const char stairs_th[] = "爬楼梯機";
const char elliptical_ch[] = "跑椭圆机";
const char elliptical_th[] = "跑橢圓機";
const char row_ch[] = "划船";
const char row_th[] = "劃船";
const char aerobic_ch[] = "有氧训练";																					
const char other_ch[] = "其它";																					 
const char *const horizontal_workout_name[3][8] = {{"Treadmill", "Cycling", "Stairs",  "Elliptical", "Row",  "Aerobic", "Piloxing", "Others"},
	                                  {treadmill_ch, bike_ch, stairs_ch, elliptical_ch, row_ch, aerobic_ch, "Piloxing", other_ch},
	                                  {treadmill_ch, bike_th, stairs_th, elliptical_th, row_th, aerobic_ch, "Piloxing", other_ch}};
	
const char *const horizontal_ware_indicator[] = {"-,,,,,,,",
	                                             	 ",-,,,,,,", 
	                                               ",,-,,,,,", 
		                                             ",,,-,,,,", 
		                                             ",,,,-,,,", 
		                                             ",,,,,-,,", 
		                                             ",,,,,,-,", 
		                                             ",,,,,,,-"};

const char *const vertical_hr_wave_indicator[3] = {" - , ,", " , - ,", " , , -" };		

#ifdef _CLINGBAND_PACE_MODEL_	
const char conn_gps_ch[] = "连接手机 GPS";			
const char conn_gps_th[] = "連接手機 GPS";																					
const	char *const horizontal_conn_gps_name[] = {"Link GPS", conn_gps_ch, conn_gps_th};		
const char conn_to_ch[] = "连接超时";																						
const char conn_to_th[] = "連接超時";																					
const	char *const horizontal_conn_timeout_name[] = {"Link Timeout", conn_to_ch, conn_to_th};	
const char conn_fail_ch[] = "连接失败";																					
const char conn_fail_th[] = "連接失敗";																					
const	char *const horizontal_conn_fail_name[] = {"Link Fail", conn_fail_ch, conn_fail_th};		

const char phone_ch[] = "手机";																					
const	char *const vertical_phone_name[] = {"", phone_ch, phone_ch};	
const	char *const vertical_gps_name[] = {"GPS", "GPS", "GPS"};	
const char gps_to_ch[] = "超时";																					
const char gps_to_th[] = "超時";																										
const	char *const vertical_gps_timeout_name[] = {"T/O", gps_to_ch, gps_to_th};
const char gps_conn_ch[] = "连接";																					
const char gps_conn_th[] = "連接";																					
const	char *const vertical_gps_conn_name[] = {"Link", gps_conn_ch, gps_conn_th};	
const char gps_fail_ch[] = "失败";																					
const char gps_fail_th[] = "失敗";																					
const	char *const vertical_gps_fail_name[] = {"Fail", gps_fail_ch, gps_fail_th};		
#endif


static void _core_frame_display(I8U middle, BOOLEAN b_render);

void UI_render_screen()
{	
	OLED_full_screen_show();

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
#endif

static void _right_render_horizontal_small_clock()
{
	I8U string[16];
	I8U len;

	if (cling.ui.icon_sec_blinking) {
		len = sprintf((char *)string, "%d:%02d",cling.time.local.hour, cling.time.local.minute);
	} else {
		len = sprintf((char *)string, "%d %02d",cling.time.local.hour, cling.time.local.minute);		
	}
 
	FONT_load_characters(cling.ui.p_oled_up+(128-len*6), (char *)string, 8, 128, FALSE);
}

#ifdef _CLINGBAND_PACE_MODEL_
static void _right_render_horizontal_button_hold()
{
	I8U language_type = cling.ui.language_type;	
	I8U i;
	I8U *p0, *p1;
	
	FONT_load_characters(cling.ui.p_oled_up+128+96, (char *)button_hold_name[language_type], 16, 128, FALSE);	
	
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
	
	len = sprintf((char *)string, "%02d", cling.ui.notific.app_notific_index);
	FONT_load_characters(cling.ui.p_oled_up+(128-len*8), (char *)string, 16, 128, FALSE);	

	_right_render_horizontal_more();
}

static void _right_render_horizontal_reminder()
{
	I8U string[16];
	I8U len;

	len = sprintf((char *)string, "%02d", cling.ui.vertical_index);
	FONT_load_characters(cling.ui.p_oled_up+(128-len*8), (char *)string, 16, 128, FALSE);	

	_right_render_horizontal_more();
}
#endif

static void _right_render_horizontal_string_core(I8U *string1, I8U *string2)
{
	I16U string1_len = 0, string2_len = 0;
	I8U offset1 = 0, offset2 = 0;
	
	string1_len = FONT_get_string_display_len((char *)string1);
	if (string1_len > 128)
	  return;
	offset1 = 128 - string1_len;
	FONT_load_characters(cling.ui.p_oled_up+offset1, (char *)string1, 16, 128, FALSE);
	
	string2_len = FONT_get_string_display_len((char *)string2);
	if (string2_len > 128)
	  return;
	offset2 = 128 - string2_len;
	FONT_load_characters(cling.ui.p_oled_up+256 + offset2, (char *)string2, 16, 128, FALSE);
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
#endif

static void _right_render_horizontal_distance_core()
{
	I8U language_type = cling.ui.language_type;
	I8U metric = cling.user_data.profile.metric_distance;	

	_right_render_horizontal_string_core((I8U *)running_distance_name[language_type], (I8U *)distance_unit_name[language_type][metric]);		
}

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

static void _get_firmware_ver_core(I8U *string)
{
	I16U major;
	I16U minor;
	
	major = cling.system.mcu_reg[REGISTER_MCU_REVH]>>4;
	minor = cling.system.mcu_reg[REGISTER_MCU_REVH]&0x0f;
	minor <<= 8;
	minor |= cling.system.mcu_reg[REGISTER_MCU_REVL];
	
	sprintf((char *)string, "VER:%d.%d", major, minor);	
}

static void _frame_render_horizontal_home()
{
	I8U string[32];
	I8U len = 0;
	I8U b_24_size = 24;		
	I8U margin = 3;
	I16U offset = 20;	
	I8U language_type = cling.ui.language_type;	
	I8U dow_index = cling.time.local.dow;

  // 1. Render the middle	
	if (cling.ui.icon_sec_blinking) {
		sprintf((char *)string, "%02d:%02d", cling.time.local.hour, cling.time.local.minute);
	} else {
		sprintf((char *)string, "%02d %02d", cling.time.local.hour, cling.time.local.minute);
	}
	
	_render_middle_horizontal_section_core(string, b_24_size, margin, offset, 3);
	
 	if (BATT_is_charging()) {
		len = sprintf((char *)string, "%d %%", cling.system.mcu_reg[REGISTER_MCU_BATTERY]);		
	} else {
	  len = sprintf((char *)string, "%d/%02d", cling.time.local.month, cling.time.local.day);
	}

  // 2. Render the right	
	FONT_load_characters(cling.ui.p_oled_up+(128 -  len*6), (char *)string, 8, 128, FALSE);
	FONT_load_characters(cling.ui.p_oled_up+256+96, (char *)week_name[language_type][dow_index], 16, 128, FALSE);		
	
	// 3. Render the left
	_left_render_horizontal_batt_ble();
}

static void _frame_render_horizontal_system_restart()
{
	I8U string1[128];
	I8U string2[16];
	I8U string3[16];
	I16U len;

  // 1. Render the middle	
	USER_device_get_name(string1);
	FONT_load_characters(cling.ui.p_oled_up, (char *)string1, 16, 128, TRUE);
	
	// 2. Render the right	
	memset(cling.ui.p_oled_up+256, 0, 256);
	_get_firmware_ver_core(string2);
	FONT_load_characters(cling.ui.p_oled_up+384+64, (char *)string2, 8, 128, FALSE);

	// 3. Render the left
	len = sprintf((char *)string3, "ID:");
	SYSTEM_get_ble_code(string3+len);	
	string3[len+4] = 0;	
	FONT_load_characters(cling.ui.p_oled_up+384+12, (char *)string3, 8, 128, FALSE);
}

static void _frame_render_horizontal_ota()
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

	// 1. Render the middle
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

static void _frame_render_horizontal_linking()
{
	I16U offset = 0;

  // 1. Render the middle	
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
	
	// 2. Render the left
	_left_render_horizontal_batt_ble();
}

static void _frame_render_horizontal_unauthorized()
{
	I8U string[16];

	// 1. Render the middle
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
	FONT_load_characters(cling.ui.p_oled_up+256, (char *)string1, 16, 128, TRUE);		
#endif

	// 2. Render the right
  _get_firmware_ver_core(string);
	FONT_load_characters(cling.ui.p_oled_up+70, (char *)string, 8, 128, FALSE);
	
	// 3. Render the left
	_left_render_horizontal_batt_ble();
}

static void _frame_render_horizontal_charging()
{
	I16U perc = cling.system.mcu_reg[REGISTER_MCU_BATTERY];
	I16U i;
	I16U fill_len;
	I8U *p0, *p1;
	I16U offset = 0; // Pixel offet at top row
	I8U string[32];
  BOOLEAN b_batt_charging = FALSE;

	// 1. Render the middle
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

	// 2. Render the right	
	// show the percentage
	sprintf((char *)string, "%d%%", perc);
	FONT_load_characters(cling.ui.p_oled_up+128+90, (char *)string, 16, 128, FALSE);
}

static void _frame_render_horizontal_steps()
{
	I8U string[16];
	I8U b_24_size = 24;			
	I16U offset = 0;
	I8U margin = 3;
	I32U stat;
	I16U integer, fractional;

	// 1. Render the middle
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
	
	// 2. Render the right
#ifdef _CLINGBAND_PACE_MODEL_		
  _right_render_horizontal_small_clock();
#else	
	_right_render_horizontal_tracker();
#endif
	
	// 3. Render the left
	_render_one_icon_16(ICON16_STEPS_IDX, 0);
}

static void _frame_render_horizontal_distance()
{
	I8U string[32];
	I8U b_24_size = 24;				
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 3;
	I32U stat;
	I16U integer, fractional;
	I8U language_type = cling.ui.language_type;
	I8U metric = cling.user_data.profile.metric_distance;

	// 1. Render the middle
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
	FONT_load_characters(cling.ui.p_oled_up+256+offset+3, (char *)distance_unit_name[language_type][metric], 16, 128, FALSE);
	
	// 2. Render the right
#ifdef _CLINGBAND_PACE_MODEL_		
  _right_render_horizontal_small_clock();
#else	
	_right_render_horizontal_tracker();
#endif

	// 3. Render the left
	_render_one_icon_16(ICON16_DISTANCE_IDX, 0);	
}

static void _frame_render_horizontal_calories()
{
	I8U string[32];
	I8U b_24_size = 24;			
	I16U offset = 0;
	I8U margin = 3;
	I32U stat;
	I16U integer, fractional;

	// 1. Render the middle
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
	
	// 2. Render the right
#ifdef _CLINGBAND_PACE_MODEL_		
  _right_render_horizontal_small_clock();
#else	
	_right_render_horizontal_tracker();
#endif

	// 3. Render the left
	_render_one_icon_16(ICON16_CALORIES_IDX, 0);		
}

static void _frame_render_horizontal_active_time()
{
	I8U string[32];
	I8U b_24_size = 24;			
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 3;
	I32U stat;
	I8U language_type = cling.ui.language_type;

	// 1. Render the middle
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
	FONT_load_characters(cling.ui.p_oled_up+256+offset+10, (char *)active_time_name[language_type], 16, 128, FALSE);
	
	// 2. Render the right
#ifdef _CLINGBAND_PACE_MODEL_		
  _right_render_horizontal_small_clock();
#else	
	_right_render_horizontal_tracker();
#endif

	// 3. Render the left
	_render_one_icon_16(ICON16_ACTIVE_TIME_IDX, 0);			
}

#ifdef _CLINGBAND_UV_MODEL_		
static void _frame_render_horizontal_uv_idx()
{
	I8U string[32];
	I8U b_24_size = 24;			
	I16U offset = 0;
	I8U margin = 3;
  I8U integer;

	// 1. Render the middle
	integer = cling.uv.max_UI_uv;
	sprintf((char *)string, "%d.%d", (integer/10), (integer%10));

	offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
	// Shift all the display to the middle
	_middle_horizontal_alignment_center(offset);
	
	// 2. Render the right
#ifdef _CLINGBAND_PACE_MODEL_		
  _right_render_horizontal_small_clock();
#else	
	_right_render_horizontal_tracker();
#endif

	// 3. Render the left
	_render_one_icon_16(ICON16_UV_INDEX_IDX, 0);				
}
#endif

static void _middle_render_pm2p5_core()
{
	WEATHER_CTX *w = &cling.weather;
	
  if ((w->pm2p5_month == cling.time.local.month) && (w->pm2p5_day == cling.time.local.day)) {
		if (w->pm2p5_value == 0xffff) {
			// AQI value Not available
      w->pm2p5_value = 0;			
			w->pm2p5_level_idx = 0;
		} else if (!w->pm2p5_value) {
			// AQI value Not available
			w->pm2p5_level_idx = 0;	
		} else if (w->pm2p5_value < 50) {
			w->pm2p5_level_idx = 1;
		} else if (w->pm2p5_value < 100) {
			w->pm2p5_level_idx = 2;
		} else if (w->pm2p5_value < 150) {
			w->pm2p5_level_idx = 3;
		} else if (w->pm2p5_value < 250) {
			w->pm2p5_level_idx = 4;
		} else if (w->pm2p5_value < 350) {
			w->pm2p5_level_idx = 5;
		} else if (w->pm2p5_value < 999) {
			w->pm2p5_level_idx = 6;
		} else {
			w->pm2p5_level_idx = 6;
			w->pm2p5_value = 999;
		}
  } else {
    w->pm2p5_value = 0;
		w->pm2p5_level_idx = 0;
	}		
}

static void _frame_render_horizontal_pm2p5()
{
	I8U string[16];
	I8U b_24_size = 24;		
	I8U margin = 3;	
	I8U language_type = cling.ui.language_type;	
	WEATHER_CTX *w = &cling.weather;
	
	// 1. Render the middle	
  _middle_render_pm2p5_core();

#ifdef _CLINGBAND_UV_MODEL_	
		language_type = 0;
#else
	if (language_type != LANGUAGE_TYPE_ENGLISH)
		language_type++;
#endif	
	
	FONT_load_characters(cling.ui.p_oled_up+256, (char *)air_level_name[language_type][w->pm2p5_level_idx], 16, 128, FALSE);

  sprintf((char *)string, "%d", w->pm2p5_value);	
	_render_middle_horizontal_section_core(string, b_24_size, margin, 80, 0);
	
	// 2. Render the left
	_render_one_icon_16(ICON16_PM2P5_IDX, 0);			
}

static void _frame_render_horizontal_weather()
{
	I8U string[32];
	I8U b_24_size = 24;			
	I8U len = 0;
	I16U offset = 10;
	I8U margin = 5;
	WEATHER_INFO_CTX weather_info;

  // 1. Render the middle	and right
	if (!WEATHER_get_weather_info(0, &weather_info)) {
		weather_info.low_temperature = 15;
		weather_info.high_temperature = 22;
		weather_info.type = 0;
	}
	
	len = sprintf((char *)string, "%d", weather_info.low_temperature);
	string[len++] = ICON24_WEATHER_RANGE_IDX;
	len += sprintf((char *)string+len, "%d", weather_info.high_temperature);
	string[len++] = ICON24_CELCIUS_IDX;
	string[len++] = 0;
	
	offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
	// Shift all the display to the middle
	_middle_horizontal_alignment_center(offset);

	// 2. Render the left	
	_render_one_icon_16(ICON16_WEATHER_IDX+weather_info.type, 0);	
}

static void _render_horizontal_app_notif_core(I8U *string)
{
	I16U dis_len;
	I16U offset = 0;
  BOOLEAN b_display_center = FALSE;

	// bug fix: dis_len is a byte type, the length should not be greater than 255,
	// if it does go beyond 255, , dis_len becomes 0, that causes problem
	dis_len = FONT_get_string_display_len((char *)string);

	if (dis_len > 80) {
		offset = 22;
	} else {
		offset = 256;
		b_display_center = TRUE;	
	}

  FONT_load_characters(cling.ui.p_oled_up+offset, (char *)string, 16, 80, b_display_center);		
}

#ifndef _CLINGBAND_PACE_MODEL_		
static void _frame_render_horizontal_message()
{
	I8U string[32];
	I8U b_24_size = 24;		
	I16U offset=0;
	I8U margin=3;

  // 1. Render the middle		
#ifdef _ENABLE_ANCS_				
	sprintf((char *)string, "%d", NOTIFIC_get_message_total());
#else	
	sprintf((char *)string, "0");
#endif	
	
	offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
	// Shift all the display to the middle
	_middle_horizontal_alignment_center(offset);
	
	// 2. Render the right
  _right_render_horizontal_more();
	
	// 3. Render the left		
	_render_one_icon_16(ICON16_RETURN_IDX, 0);	
}

static void _frame_render_horizontal_app_notif()
{
	I8U len;
	I8U string[128];

  // 1. Render the middle		
	len = NOTIFIC_get_app_name(cling.ui.notific.app_notific_index, (char *)string);
	N_SPRINTF("[UI] app index: %d, %d, %s", cling.ui.notific.app_notific_index, len, (char *)string);
	
	_render_horizontal_app_notif_core(string);
	
	len = sprintf((char *)string, "%02d", cling.ui.notific.app_notific_index);
	FONT_load_characters(cling.ui.p_oled_up+(128-len*8), (char *)string, 16, 128, FALSE);		

	// 2. Render the right
  _right_render_horizontal_app_notif();
	
	// 3. Render the left		
	_render_one_icon_16(ICON16_RETURN_IDX, 0);		
}
#endif

static void _frame_render_horizontal_detail_notif()
{
  I8U string_pos = 0;
	I8U string[128];
  I8U msg_len = 0;

  // 1. Render the middle			
#ifdef _CLINGBAND_PACE_MODEL_			
	msg_len = NOTIFIC_get_app_message_detail(0, (char *)string);
	
	if (msg_len == 0) {
	  NOTIFIC_get_app_name(0, (char *)string);	
    cling.ui.notific.detail_idx = 0;			
	}	
#else 
	msg_len = NOTIFIC_get_app_message_detail(cling.ui.notific.app_notific_index, (char *)string);
	
	if (msg_len == 0) {
	  NOTIFIC_get_app_name(cling.ui.notific.app_notific_index, (char *)string);	
    cling.ui.notific.detail_idx = 0;		
	}		
#endif
	
	if (cling.ui.notific.detail_idx) {
		string_pos = cling.ui.notific.string_pos_buf[cling.ui.notific.detail_idx - 1];
	}
	
	if (string_pos <= 125)
	  FONT_load_characters(cling.ui.p_oled_up, (char *)string+string_pos, 16, UI_DETAIL_MESSAGE_DISPLAY_MAX_LEN, FALSE);		
	
	// 2. Render the right
  _right_render_horizontal_more();
	
	// 3. Render the left			
}

static void _render_horizontal_incoming_msg_core()
{
	I8U string[128];

	// 1. Render the middle			
	cling.ui.notific.app_notific_index = 0;	
	NOTIFIC_get_app_name(0, (char *)string);
	
	_render_horizontal_app_notif_core(string);
	
	// 2. Render the right
#ifdef _CLINGBAND_PACE_MODEL_	
	_right_render_horizontal_more();
#else	
	_right_render_horizontal_ok_top();
#endif		
}

static void _frame_render_horizontal_incoming_call()
{
	// 1. Render the middle and right			
	_render_horizontal_incoming_msg_core();
	
	// 2. Render the left			
	if (cling.ui.icon_sec_blinking) 
	  _render_one_icon_16(ICON16_INCOMING_CALL_IDX, 0);		
}

static void _frame_render_horizontal_incoming_msg()
{
	// 1. Render the middle and right			
	_render_horizontal_incoming_msg_core();

	// 2. Render the left			
	if (cling.ui.icon_sec_blinking) 
	  _render_one_icon_16(ICON16_MESSAGE_IDX, 0);		
}

static void _render_horizontal_reminder_core(BOOLEAN b_alarm_clock_reminder)
{
	I8U string[32];
	I8U b_24_size = 24;			
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 3;
  BOOLEAN b_invalid_alarm = TRUE;

  if (b_alarm_clock_reminder) {
		if ((cling.reminder.b_daily_alarm) || (cling.reminder.b_sleep_total)) {
		  b_invalid_alarm = FALSE;
		}			
	} else {
		if (cling.reminder.b_daily_alarm) {
      b_invalid_alarm = FALSE;
		}		
	}

 if ((cling.ui.notific.alarm_clock_hh >= 24) || (cling.ui.notific.alarm_clock_mm >= 60)) {
		b_invalid_alarm = TRUE;
	}
				
	if (b_invalid_alarm) {
		string[len++] = ICON24_NO_SKIN_TOUCH_IDX;		
		string[len++] = 0;
	} else {
	  len = sprintf((char *)string, "%02d:%02d", cling.ui.notific.alarm_clock_hh, cling.ui.notific.alarm_clock_mm);					
	}
	
	offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 2);
	// Shift all the display to the middle
	_middle_horizontal_alignment_center(offset);
}

static void _frame_render_horizontal_alarm_clock_reminder()
{
	// 1. Render the middle 			
  _render_horizontal_reminder_core(TRUE);

	// 2. Render the right	
	_right_render_horizontal_more();	
	
	// 3. Render the left
	if ((cling.reminder.alarm_type == SLEEP_ALARM_CLOCK) || (cling.reminder.alarm_type == WAKEUP_ALARM_CLOCK))
		_render_one_icon_16(ICON16_SLEEP_ALARM_CLOCK_IDX, 0);				
	else 
		_render_one_icon_16(ICON16_NORMAL_ALARM_CLOCK_IDX, 0);	
}

#ifndef _CLINGBAND_PACE_MODEL_	
static void _frame_render_horizontal_alarm_clock_detail()
{
	// 1. Render the middle 		
  _render_horizontal_reminder_core(FALSE);

	// 2. Render the right			
	_right_render_horizontal_reminder();
	
	// 3. Render the left
	_render_one_icon_16(ICON16_NORMAL_ALARM_CLOCK_IDX, 0);		
}
#endif

static void _frame_render_horizontal_idle_alert()
{
	I8U offset = 0;
	I8U language_type = cling.ui.language_type;	

	// 1. Render the middle 		
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) 
	  offset = 24;
	else 
		offset = 32;
	
	FONT_load_characters(cling.ui.p_oled_up+256+offset, (char *)idle_alart_name[language_type], 16, 128, FALSE);

	// 2. Render the right		
  _right_render_horizontal_small_clock();
	
	// 3. Render the left	
  _render_one_icon_24(ICON24_IDLE_ALERT_IDX, 0);		
}

static I8U _render_middle_horizontal_hr_core()
{
	I8U string[32];
	I8U b_24_size = 24;				
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 3;
	I8U hr_result = 0;
	
	if (cling.hr.b_closing_to_skin || cling.hr.b_start_detect_skin_touch) {
		if (cling.hr.heart_rate_ready) {
#ifdef _ENABLE_PPG_
			//hr_result = PPG_minute_hr_calibrate();
			update_and_push_hr();
			hr_result = cling.hr.m_curr_dispaly_HR;
#endif
			len = sprintf((char *)string, "%d", hr_result);			
		} else {
			if (cling.ui.heart_rate_wave_index > 7) {
				cling.ui.heart_rate_wave_index = 0;
			}
			len = sprintf((char *)string, "%s", horizontal_ware_indicator[cling.ui.heart_rate_wave_index]);
			_render_middle_horizontal_section_core(string, 8, 5, 128+35, 0);
			cling.ui.heart_rate_wave_index ++;
			return 0;
		}
	} else {
		N_SPRINTF("[UI] Heart rate - not valid");
		len = 0;
		string[len++] = ICON24_NO_SKIN_TOUCH_IDX;		
		string[len++] = 0;				
	}

	if (cling.activity.b_workout_active)	
	  offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, len);
	else 
		offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);

	// Shift all the display to the middle
	_middle_horizontal_alignment_center(offset);		
	
	return hr_result;
}

static void _frame_render_horizontal_heart_rate()
{
	// 1. Render the middle 			
  _render_middle_horizontal_hr_core();

	// 2. Render the right		
  _right_render_horizontal_small_clock();

	// 3. Render the left			
	if (cling.ui.icon_sec_blinking) 
	  _render_one_icon_16(ICON16_HEART_RATE_IDX, 0);			
}

#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)	
static void _frame_render_horizontal_sos_alert()
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

#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)	
static void _frame_render_horizontal_skin_temp()
{
	I8U string[32];
	I8U b_24_size = 24;				
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 3;
  I8U integer, fractional;

	// 1. Render the middle 		
  integer = cling.therm.current_temperature/10;
	fractional = cling.therm.current_temperature - integer * 10;
	len = sprintf((char *)string, "%d.%d", integer, fractional);
	string[len++] = ICON24_CELCIUS_IDX;
	string[len++] = 0;
	
	offset = _render_middle_horizontal_section_core(string,b_24_size,  margin, offset, 0);
	// Shift all the display to the middle
	_middle_horizontal_alignment_center(offset);
	
	// 2. Render the right		
  _right_render_horizontal_small_clock();

	// 3. Render the left			
	if (cling.ui.icon_sec_blinking) 
	  _render_one_icon_16(ICON16_SKIN_TEMP_IDX, 0);			
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
static void _middle_render_stopwatch_core()
{
	I8U hour = 0, min = 0, sec = 0;
	I16U ms = 0;		
  I32U t_diff = 0;
  I32U t_curr_in_ms = CLK_get_system_time();	
	UI_STOPWATCH_CTX *stopwatch = &cling.ui.stopwatch;
	
	if (!stopwatch->t_start_stamp) {
		stopwatch->t_start_stamp = t_curr_in_ms;
	}
	
	if (!stopwatch->b_in_pause_mode) {
    t_diff = t_curr_in_ms - stopwatch->t_start_stamp;
	  stopwatch->t_stop_stamp = t_diff;			
  } else {
	  t_diff = stopwatch->t_stop_stamp;
	  stopwatch->t_start_stamp = t_curr_in_ms - stopwatch->t_stop_stamp;
	}
		
	ms = t_diff % 1000;
	t_diff /= 1000;
	hour = t_diff / 3600;
	t_diff -= hour*3600;
	min = t_diff / 60;
	t_diff -= min * 60;
	sec = t_diff;
	
	ms /= 10;
	
  if (ms > 99) ms = 99;
  if (hour > 9) hour = 9;
	
  stopwatch->hour = hour; 	
	stopwatch->min = min;
	stopwatch->sec = sec;
	stopwatch->ms = ms;
}

static void _frame_render_horizontal_stopwatch_start()
{
	I8U string[32];
	I8U b_24_size = 24;
	I8U margin = 3;	
	I16U offset = 0;
	UI_STOPWATCH_CTX *stopwatch = &cling.ui.stopwatch;	

	// 1. Render the middle 		
	_middle_render_stopwatch_core();
	
	if (stopwatch->hour) {
		// Render the time	
		sprintf((char *)string, "%d:%02d:%02d", stopwatch->hour, stopwatch->min, stopwatch->sec);
	  offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 1);
	} else {
		// Render the minute and second.	
		sprintf((char *)string, "%d:%02d.", stopwatch->min, stopwatch->sec);
	  offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
		// Render the millisecond
		b_24_size = 16;
		offset += 128;
		sprintf((char *)string, "%02d", stopwatch->ms);
	  offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);		
		offset -= 128;
	}

	_middle_horizontal_alignment_center(offset);	

	// 2. Render the right		
	if (cling.ui.stopwatch.b_in_pause_mode)
    _render_one_icon_16(ICON16_STOPWATCH_START_IDX, 110);
	else
    _render_one_icon_16(ICON16_STOPWATCH_STOP_IDX, 110);		
	
	// 3. Render the left			
  _render_one_icon_16(ICON16_STOPWATCH_IDX, 0);	
}

static void _frame_render_horizontal_stopwatch_stop()
{
	I8U language_type = cling.ui.language_type;

	// 1. Render the middle 			
	FONT_load_characters(cling.ui.p_oled_up+128, (char *)stopwatch_stop_name[language_type], 16, 128, TRUE);

	// 2. Render the right		
	_right_render_horizontal_ok_middle();
	
	// 3. Render the left			
  _render_one_icon_16(ICON16_STOPWATCH_IDX, 0);		
}
#endif

static BOOLEAN _middle_horizontal_running_ready_core()
{
	I8U b_24_size = 24;			
	I8U len = 1;
	I16U offset = 60;
	I8U margin = 5;
  I32U t_curr_in_ms = CLK_get_system_time();	
  UI_RUNNING_INFO_CTX *running_info = &cling.ui.running_info;	
	BOOLEAN b_ready_finished = FALSE;

	// First Update touch time stamp.
  cling.ui.touch_time_stamp = t_curr_in_ms;
	
	if (!running_info->t_ready_stamp)
		running_info->t_ready_stamp = t_curr_in_ms;
	
	if (t_curr_in_ms  > (running_info->t_ready_stamp + 800)) {
		running_info->t_ready_stamp = t_curr_in_ms;		
		running_info->ready_idx++;
	}

	if (running_info->ready_idx > 3) {
		running_info->ready_idx = 3;
		b_ready_finished = TRUE;
    cling.activity.b_workout_active = TRUE;			
	}
	
	if (running_info->ready_idx == 3){
		len = 2;
		offset = 53;
	}
	
  _render_middle_horizontal_section_core((I8U *)ready_indicator_name[running_info->ready_idx], b_24_size, margin, offset, len);	
	
	return b_ready_finished;	
}

static void _middle_render_horizontal_run_or_stop_core(I8U *string1, I8U *string2, I8U *string_out)
{
	I8U len = sprintf((char *)string_out, "%s", string1);
	
	sprintf((char *)&string_out[len], "%s", string2);	
}

#ifndef _CLINGBAND_PACE_MODEL_			
static void _frame_render_horizontal_workout_mode_switch()
{
	I8U workout_idx=0, frame_index=0;	
	I8U language_type = cling.ui.language_type;

	// 1. Render the middle 		
  frame_index = cling.ui.frame_index;
	if ((frame_index < UI_DISPLAY_WORKOUT)	|| (frame_index > UI_DISPLAY_WORKOUT_OTHERS))
		return;
	
	workout_idx = frame_index - UI_DISPLAY_WORKOUT_TREADMILL;
		
	FONT_load_characters(cling.ui.p_oled_up+128, (char *)horizontal_workout_name[language_type][workout_idx], 16, 128, TRUE);
	
	_render_middle_horizontal_section_core((I8U *)horizontal_ware_indicator[workout_idx], 8, 7, 256+30, 0);
	
	// 2. Render the right		
	_right_render_horizontal_ok_top();
	
	// 3. Render the left			
  _render_one_icon_16(ICON16_RETURN_IDX, 0);			
}

static void _frame_render_horizontal_workout_ready()
{	
	BOOLEAN b_ready_finished = FALSE;
	
	// 1. Render the middle 			
  b_ready_finished = _middle_horizontal_running_ready_core();
	
	if (b_ready_finished) {
		cling.ui.frame_index = UI_DISPLAY_WORKOUT_RT_TIME;
	  cling.ui.frame_next_idx = cling.ui.frame_index;
	}
}

static void _frame_render_horizontal_workout_stop()
{
	I8U string[32];
	I8U language_type = cling.ui.language_type;	

	// 1. Render the middle 		
	_middle_render_horizontal_run_or_stop_core((I8U *)stop_run_name[language_type], (I8U *)gym_workout_run_name[language_type], string);
	FONT_load_characters(cling.ui.p_oled_up+128, (char *)string, 16, 128, TRUE);
	
	// 2. Render the right
  _right_render_horizontal_ok_middle();
 	
	// 3. Render the left	
	if (cling.ui.icon_sec_blinking) 
	  _render_one_icon_16(ICON16_RUNNING_STOP_IDX, 0);			
}
#endif

static void _horizontal_core_run_distance(I32U stat)
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
	
	if (!cling.activity.b_workout_active)
		len = 0;
	
	_render_middle_horizontal_section_core(string, b_24_size, margin, offset, len);
}

static void _frame_render_horizontal_running_distance()
{
	// 1. Render the middle
	_horizontal_core_run_distance(cling.run_stat.distance);

	// 2. Render the right	
  _right_render_horizontal_distance_core();	
	
	// 3. Render the left
	_render_one_icon_16(ICON16_RUNNING_DISTANCE_IDX, 0);				
}

static void _frame_render_horizontal_running_time()
{
	I8U string[32];
	I8U b_24_size = 24;
	I8U margin = 3;
	I16U offset = 0;
	I8U hour=0, sec=0;
	I16U min = 0;

	// 1. Render the middle	
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
	
	// 2. Render the left
	_render_one_icon_16(ICON16_RUNNING_TIME_IDX, 0);					
}

static void _frame_render_horizontal_running_pace()
{
	I8U string[32];
	I8U b_24_size = 24;			
	I16U offset = 0;
	I8U margin = 2;
	I32U pace = cling.run_stat.time_min; // Rounding 
	I32U min, sec;
	I8U language_type = cling.ui.language_type;	

	// 1. Render the middle		
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

	if ((!min) && (!sec)) {
		offset = 39;
		// Render the pace
	  sprintf((char *)string, "--/ --*");	
	} else {
	  // Render the pace
	  sprintf((char *)string, "%d/%02d*", min, sec);
	}
	
	_render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
	
  // 2. Render the right	
	_right_render_horizontal_string_core((I8U *)gym_workout_run_name[language_type], (I8U *)pace_name[language_type]);	
	
	// 3. Render the left	
	_render_one_icon_16(ICON16_RUNNING_PACE_IDX, 0);				
}

static void _frame_render_horizontal_running_stride()
{
	I8U string[32];
	I8U b_24_size = 24;			
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 2;
	I32U stat;
	I8U language_type = cling.ui.language_type;	
	I8U metric = cling.user_data.profile.metric_distance;	

	// 1. Render the middle			
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
	
  // 2. Render the right		
	_right_render_horizontal_string_core((I8U *)horizontal_stride_name[language_type], (I8U *)stride_unit_name[language_type][metric]);	

	// 3. Render the left	
	_render_one_icon_16(ICON16_RUNNING_STRIDE_IDX, 0);			
}

static void _frame_render_horizontal_running_cadence()
{
	I8U string[32];
	I8U b_24_size = 24;		
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 2;
	I32U stat;
	I8U language_type = cling.ui.language_type;	
	
	// 1. Render the middle			
	if (cling.run_stat.time_min)
		stat = cling.run_stat.steps/cling.run_stat.time_min;
	else
		stat = 0;
	len = sprintf((char *)string, "%d", stat);
	
	offset = ((80 - len*13 - (len-1)*2)/2) + 16;
	_render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
	
  // 2. Render the right		
	_right_render_horizontal_string_core((I8U *)cadence_rate_name[language_type], (I8U *)cadence_spm_name[language_type]);	

	// 3. Render the left	
	_render_one_icon_16(ICON16_RUNNING_CADENCE_IDX, 0);	
}

static void _frame_render_horizontal_running_hr()
{
	I8U string[32];
	I8U b_24_size = 24;				
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 2;
	I32U hr = cling.run_stat.accu_heart_rate;
	I8U language_type = cling.ui.language_type;
	
	// 1. Render the middle				
	if (cling.run_stat.time_min) {
		hr /= cling.run_stat.time_min;
	} else {
		hr = 0;
	}

	len = sprintf((char *)string, "%d", hr);
	
	offset = ((80 - len*13 - (len-1)*2)/2) + 16;
	
	_render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
	
  // 2. Render the right		
	_right_render_horizontal_string_core((I8U *)avg_name[language_type], (I8U *)heart_rate_name[language_type]);	
	
	// 3. Render the left	
	_render_one_icon_16(ICON16_RUNNING_HR_IDX, 0);		
}

static void _render_horizontal_calories_core(I32U stat)
{
	I8U string[32];
	I8U b_24_size = 24;				
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 2;
	I8U language_type = cling.ui.language_type;
	
	len = sprintf((char *)string, "%d", stat);

	offset = ((80 - len*13 - (len-1)*2)/2) + 16;
	
	_render_middle_horizontal_section_core(string, b_24_size, margin, offset, 0);
	
	_right_render_horizontal_string_core((I8U *)calories_name[language_type], (I8U *)calories_unit_horizontal_name[language_type]);	
}

static void _frame_render_horizontal_running_calories()
{
	// 1. Render the middle and right	
	_render_horizontal_calories_core(cling.run_stat.calories);
	
	// 2. Render the left
	_render_one_icon_16(ICON16_RUNNING_CALORIES_IDX, 0);		
}

#ifdef _CLINGBAND_PACE_MODEL_
static void _frame_render_horizontal_running_analysis()
{
	const char *running_analysis_name[] = {"RUN DATA", "当日跑步 ", "當日跑步 "};
	I8U language_type = cling.ui.language_type;
	
	// 1. Render the middle
	FONT_load_characters(cling.ui.p_oled_up+128+24, (char *)running_analysis_name[language_type], 16, 128, FALSE);
	
	// 2. Render the right
	_right_render_horizontal_button_hold();
	
	// 3. Render the left
	_render_one_icon_16(ICON16_RUNNING_PACE_IDX, 0);	
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_
static void _frame_render_horizontal_training_calories()
{
	// 1. Render the middle and right
	_render_horizontal_calories_core(cling.train_stat.calories);	
	
	// 2. Render the left
	if (cling.ui.icon_sec_blinking) 
	  _render_one_icon_16(ICON16_RUNNING_CALORIES_IDX, 0);		
}
#endif

#ifdef _CLINGBAND_PACE_MODEL_
static void _frame_render_horizontal_running_stop_analysis()
{
	const char *analysis_end_name[] = {"INFO END", "结束分析 ", "結束分析 "};
	I8U language_type = cling.ui.language_type;

	// 1. Render the middle	
	FONT_load_characters(cling.ui.p_oled_up+128+24, (char *)analysis_end_name[language_type], 16, 128, FALSE);
	
	// 2. Render the right
	_right_render_horizontal_button_hold();
	
	// 3. Render the left
	_render_one_icon_16(ICON16_RUNNING_STOP_IDX, 0);		
}
#endif

#ifdef _CLINGBAND_PACE_MODEL_		
static void _frame_render_horizontal_connect_gps()
{
	I8U language_type = cling.ui.language_type;		

 	FONT_load_characters(cling.ui.p_oled_up+128, (char *)horizontal_conn_gps_name[language_type], 16, 128, TRUE);	
}

static void _frame_render_horizontal_connect_gps_timeout()
{
	I8U language_type = cling.ui.language_type;		

 	FONT_load_characters(cling.ui.p_oled_up+128, (char *)horizontal_conn_timeout_name[language_type], 16, 128, TRUE);	
	
  _render_one_icon_16(ICON16_GPS_CONN_TIMEOUT_IDX, 0);	
}

static void _frame_render_horizontal_connect_gps_fail()
{
	I8U language_type = cling.ui.language_type;		

 	FONT_load_characters(cling.ui.p_oled_up+128, (char *)horizontal_conn_fail_name[language_type], 16, 128, TRUE);	
	
	_render_one_icon_16(ICON16_BLE_IDX, 0);		
}
#endif

static void _frame_render_horizontal_training_start_run()
{
	I8U string[32];
  I8U offset = 0;	
  BOOLEAN b_center_display = TRUE;	
	I8U language_type = cling.ui.language_type;

  // 1. Render the middle			
#ifdef _CLINGBAND_PACE_MODEL_			
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH) {		
	  offset = 28;
	} else {		
	  offset = 24;	
	}
	
	b_center_display = FALSE;
#endif
	
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH)
	  _middle_render_horizontal_run_or_stop_core((I8U *)training_run_name[language_type], (I8U *)start_run_name[language_type], string);
	else
	  _middle_render_horizontal_run_or_stop_core((I8U *)start_run_name[language_type], (I8U *)training_run_name[language_type], string);

	FONT_load_characters(cling.ui.p_oled_up+128+offset, (char *)string, 16, 128, b_center_display);	
	
  // 2. Render the right			
#ifdef _CLINGBAND_PACE_MODEL_			
	_right_render_horizontal_button_hold();
#else
  _right_render_horizontal_ok_middle();
#endif	
	
  // 3. Render the left
	_render_one_icon_16(ICON16_RUNNING_DISTANCE_IDX, 0);				
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _frame_render_horizontal_training_run_or_analysis()
{
	I8U language_type = cling.ui.language_type;		
	I8U i;
	I8U *p0, *p1;
	I8U offset = 40;

  // 1. Render the middle				
  if (language_type	== LANGUAGE_TYPE_ENGLISH)
	  offset = 44;
	
	FONT_load_characters(cling.ui.p_oled_up+128+offset, (char *)run_start_name[language_type], 16, 128, FALSE);
	
  p0 = cling.ui.p_oled_up+128+40,
	p1 = p0+128;
	
	for (i=0;i<32;i++) {
	 *(p0+i) = ~(*(p0+i));
	 *(p1+i) = ~(*(p1+i));	
	}
	
  p0 = cling.ui.p_oled_up+128+96,
	p1 = p0+128;

	if (language_type	== LANGUAGE_TYPE_ENGLISH)
	  offset = 97;
	else
		offset = 96;
		
 	FONT_load_characters(cling.ui.p_oled_up+128+offset, (char *)run_analysis_name[language_type], 16, 128, FALSE);

	for (i=0;i<32;i++) {
		*(p0+i) = ~(*(p0+i));
		*(p1+i) = ~(*(p1+i));	
	}		
	
  // 2. Render the left	
	_render_one_icon_16(ICON16_RUNNING_DISTANCE_IDX, 0);			
}
#endif

static void _frame_render_horizontal_training_ready()
{
	BOOLEAN b_ready_finished = FALSE;
	
  // 1. Render the middle			
  b_ready_finished = _middle_horizontal_running_ready_core();
	
	if (b_ready_finished) {
		cling.ui.frame_index = UI_DISPLAY_TRAINING_STAT_TIME;
	  cling.ui.frame_next_idx = cling.ui.frame_index;
	}
	
  // 2. Render the left		
	_render_one_icon_24(ICON24_RUNNING_DISTANCE_IDX, 128+10);
}

static void _get_training_time_core()
{
	I32U curr_ts_ms = CLK_get_system_time();
	I32U diff_ts_ms, diff_ts_sec;
	I8U hour=0, min=0, sec=0;
  UI_RUNNING_INFO_CTX *running_info = &cling.ui.running_info;
	
	if (!running_info->time_start_in_ms) {
		running_info->time_start_in_ms = curr_ts_ms;
		cling.train_stat.time_start_in_ms = curr_ts_ms;		
	} else {
	  diff_ts_ms = curr_ts_ms - cling.train_stat.time_start_in_ms;
	  diff_ts_sec = diff_ts_ms/1000;
	  hour = diff_ts_sec/3600;
	  min = (diff_ts_sec - hour*3600)/60;
	  sec = (diff_ts_sec - hour*3600 - min*60);
	}

	if (hour > 9) hour = 9;		
	
	running_info->hour = hour;
	running_info->min = min;
	running_info->sec = sec;
}

static void _frame_render_horizontal_training_time()
{
	I8U string[32];
	I8U len = 0;
	I8U b_24_size = 24;
	I16U offset = 0;
	I8U margin = 3;
  UI_RUNNING_INFO_CTX *running_info = &cling.ui.running_info;

  // 1. Render the middle	
	// Get display total training time; 
	_get_training_time_core();

	if (running_info->hour) {
	  len = sprintf((char *)string, "%d:%02d:%02d", running_info->hour, running_info->min, running_info->sec);		
	} else {
	  len = sprintf((char *)string, "%d:%02d", running_info->min, running_info->sec);				
	}

	offset = _render_middle_horizontal_section_core(string, b_24_size, margin, offset, len);	
	_middle_horizontal_alignment_center(offset);

  // 2. Render the left		
	if (cling.ui.icon_sec_blinking) 
	  _render_one_icon_16(ICON16_RUNNING_TIME_IDX, 0);			
	
	update_and_push_hr();	
}

static void _frame_render_horizontal_training_distance()
{
	// 1. Render the middle 		
	_horizontal_core_run_distance(cling.train_stat.distance);
	
	// 2. Render the right
  _right_render_horizontal_distance_core();
	
	// 3. Render the left
	if (cling.ui.icon_sec_blinking) 
	  _render_one_icon_16(ICON16_RUNNING_DISTANCE_IDX, 0);		
	
	update_and_push_hr();
}

static void _frame_render_horizontal_training_pace()
{
	I8U string[32];
	I8U b_24_size = 24;				
	I8U len = 0;
	I16U offset = 0;
	I8U margin = 2;
	I32U min, sec;
	I8U language_type = cling.ui.language_type;
	
	// 1. Render the middle 			
	min = cling.run_stat.last_10sec_pace_min;
	sec = cling.run_stat.last_10sec_pace_sec;
	
  if (min > 9)	
		offset = 22;
	else 
		offset = 29;
	
	if ((!min) && (!sec)) {
		offset = 39;
		// Render the pace
	  len = sprintf((char *)string, "--/ --*");	
	} else {
	  // Render the pace
	  len = sprintf((char *)string, "%d/%02d*", min, sec);		
	}
	
	_render_middle_horizontal_section_core(string, b_24_size, margin, offset, len);
	
	// 2. Render the right	
	_right_render_horizontal_string_core((I8U *)pace_name[language_type], NULL);
	
	// 3. Render the left
	if (cling.ui.icon_sec_blinking) 
	  _render_one_icon_16(ICON16_RUNNING_PACE_IDX, 0);	
	
	update_and_push_hr();
}

static void _frame_render_horizontal_training_hr()
{
	I8U string[32];
	I8U len = 0;
	I8U *p0, *p1, *p2, *p3;
  I8U i;	
	I32U hr_perc = 0;		
	I8U hr_result = 0;

  // 1. Render the middle		
  hr_result = _render_middle_horizontal_hr_core();
	
	if (hr_result) {
		hr_perc = (hr_result * 100)/(220-cling.user_data.profile.age);	
		if (hr_perc > 98)
			hr_perc = 98;
		
	  len = sprintf((char *)string, "%d%%", hr_perc);
	  FONT_load_characters(cling.ui.p_oled_up+128-(len*8), (char *)string, 16, 128, FALSE);		
  }

  // 2. Render the right			
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
	
  // 3. Render the left
	if (cling.ui.icon_sec_blinking) 
	  _render_one_icon_16(ICON16_RUNNING_HR_IDX, 0);			
}

static void _frame_render_horizontal_training_run_stop()
{
	I8U string[32];
  I8U offset = 31;	
	I8U language_type = cling.ui.language_type;	
	
  // 1. Render the middle			
	_middle_render_horizontal_run_or_stop_core((I8U *)stop_run_name[language_type], (I8U *)training_run_name[language_type], string);

#ifdef _CLINGBAND_PACE_MODEL_				
	  offset = 24;	
#endif	
	
	FONT_load_characters(cling.ui.p_oled_up+128+offset, (char *)string, 16, 128, FALSE);
	
  // 2. Render the right		
#ifdef _CLINGBAND_PACE_MODEL_			
  _right_render_horizontal_button_hold();
#else
	_right_render_horizontal_ok_middle();
#endif
	
  // 3. Render the left
	if (cling.ui.icon_sec_blinking) 
	  _render_one_icon_16(ICON16_RUNNING_STOP_IDX, 0);			
	
	update_and_push_hr();
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _frame_render_horizontal_cycling_outdoor_start()
{
	I8U string[32];
	I8U language_type = cling.ui.language_type;

#ifndef _CLINGBAND_PACE_MODEL_
	cling.train_stat.b_cycling_state = FALSE;
#endif
	
  // 1. Render the middle				
	if (cling.ui.language_type == LANGUAGE_TYPE_ENGLISH)
	  _middle_render_horizontal_run_or_stop_core((I8U *)cycling_run_name[language_type], (I8U *)start_run_name[language_type], string);
	else
	  _middle_render_horizontal_run_or_stop_core((I8U *)start_run_name[language_type], (I8U *)cycling_run_name[language_type], string);

	FONT_load_characters(cling.ui.p_oled_up+128, (char *)string, 16, 128, TRUE);
	
  // 2. Render the right		
	_right_render_horizontal_ok_middle();
	
  // 3. Render the left
	_render_one_icon_16(ICON16_CYCLING_OUTDOOR_MODE_IDX, 0);		
}

static void _frame_render_horizontal_cycling_outdoor_ready()
{
	BOOLEAN b_ready_finished = FALSE;
	
  // 1. Render the middle				
  b_ready_finished = _middle_horizontal_running_ready_core();
	
	if (b_ready_finished) {
		cling.ui.frame_index = UI_DISPLAY_CYCLING_OUTDOOR_STAT_TIME;
	  cling.ui.frame_next_idx = cling.ui.frame_index;
	}
	
  // 2. Render the left	
	_render_one_icon_24(ICON24_CYCLING_OUTDOOR_MODE_IDX, 128+10);	
}

static void _frame_render_horizontal_cycling_outdoor_distance()
{
	I8U language_type = cling.ui.language_type;

  // 1. Render the middle	and right	
	if (BTLE_is_connected()) {
		_horizontal_core_run_distance(cling.train_stat.distance);
		_right_render_horizontal_distance_core();	
	} else {
		_render_one_icon_24(ICON24_NO_SKIN_TOUCH_IDX, 128+40);
		_right_render_horizontal_string_core(NULL, (I8U *)no_ble_horizontal_name[language_type]);
	}
	
  // 2. Render the left
	if (cling.ui.icon_sec_blinking) 
	  _render_one_icon_16(ICON16_CYCLING_OUTDOOR_MODE_IDX, 0);				
}

static void _frame_render_horizontal_cycling_outdoor_speed()
{
	I8U language_type = cling.ui.language_type;
	I8U metric = cling.user_data.profile.metric_distance;	
	
  // 1. Render the middle	and right		
	if (BTLE_is_connected()) {
		_horizontal_core_run_distance(cling.train_stat.speed);
		FONT_load_characters(cling.ui.p_oled_up+384+104, (char *)speed_unit_name[metric], 8, 128, FALSE);	
	} else {
		_render_one_icon_24(ICON24_NO_SKIN_TOUCH_IDX, 128+40);
		_right_render_horizontal_string_core(NULL, (I8U *)no_ble_horizontal_name[language_type]);
	}
	
  // 2. Render the left
	if (cling.ui.icon_sec_blinking) 
	  _render_one_icon_16(ICON16_CYCLING_OUTDOOR_SPEED_IDX, 0);			
}

static void _frame_render_horizontal_cycling_outdoor_stop()
{
	I8U string[32];
	I8U language_type = cling.ui.language_type;
	
  // 1. Render the middle		
	_middle_render_horizontal_run_or_stop_core((I8U *)stop_run_name[language_type], (I8U *)cycling_run_name[language_type], string);
	
	FONT_load_characters(cling.ui.p_oled_up+128, (char *)string, 16, 128, TRUE);
	
  // 2. Render the right			
	_right_render_horizontal_ok_middle();
	
  // 3. Render the left			
	if (cling.ui.icon_sec_blinking) 
	  _render_one_icon_16(ICON16_RUNNING_STOP_IDX, 0);		
}
#endif

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
static void _frame_render_horizontal_music_play()
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

static void _frame_render_horizontal_music_track()
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

static void _frame_render_horizontal_music_volume()
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
static void _render_horizontal_balance_core(BOOLEAN b_bus_card_balance)
{
	UI_BALANCE_CTX *balance = &cling.ui.balance;
	
	const char *unit_balance_display[] = {"RMB", "元 ", "元 "};	
	I8U string[32];
	I8U len = 0;
	I8U b_24_size = 24;	
	I16U offset = 0;
	I8U margin = 3;
	I32U balance_display = 0;
	I32U integer, fractional;
	I8U language_type = cling.ui.language_type;

	if (b_bus_card_balance)
		balance_display = balance->bus_card_balance;
	else
		balance_display = balance->bank_card_balance;
	
	if (balance_display > 99999)
		b_24_size = 16;
	
	if (balance_display > 99999999)
		balance_display = 99999999;
	
	integer = balance_display/100;
	fractional = balance_display - integer * 100;
	
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
	  FONT_load_characters(cling.ui.p_oled_up+256+104, (char *)unit_balance_display[language_type], 16, 128, FALSE);	
	else 
		FONT_load_characters(cling.ui.p_oled_up+256+112, (char *)unit_balance_display[language_type], 16, 128, FALSE);	
}

static void _frame_render_horizontal_bus_card_balance_enquiry()
{
	// 1. Render the middle
  _render_horizontal_balance_core(TRUE);
	
	// 2. Render the left	
  _render_one_icon_16(ICON16_BUS_CARD_IDX, 0);		
}

static void _frame_render_horizontal_bank_card_balance_enquiry()
{
	// 1. Render the middle	
  _render_horizontal_balance_core(FALSE);
	
	// 2. Render the left		
  _render_one_icon_16(ICON16_BANK_CARD_IDX, 0);		
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_
static void _render_horizontal_carousel_core(I8U left_idx, I8U middle_idx, I8U right_idx)
{
  _render_one_icon_24(left_idx, 128);	

  _render_one_icon_24(middle_idx, 128+52);	
	
  _render_one_icon_24(right_idx, 128+104);	
}

static void _frame_render_horizontal_carousel_1()
{
  _render_horizontal_carousel_core(ICON24_RUNNING_MODE_IDX, ICON24_CYCLING_OUTDOOR_MODE_IDX, ICON24_WORKOUT_MODE_IDX);
}

static void _frame_render_horizontal_carousel_2()
{
#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)		
  _render_horizontal_carousel_core(ICON24_MUSIC_IDX, ICON24_STOPWATCH_IDX, ICON24_MESSAGE_IDX);	
#endif	
	
#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	
  _render_horizontal_carousel_core(ICON24_MESSAGE_IDX, ICON24_STOPWATCH_IDX, ICON24_WEATHER_IDX);	
#endif
}

static void _frame_render_horizontal_carousel_3()
{
#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	
  _render_horizontal_carousel_core(ICON24_PM2P5_IDX, ICON24_NORMAL_ALARM_CLOCK_IDX, ICON24_SETTING_IDX);	
#endif
	
#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
  _render_horizontal_carousel_core(ICON24_WEATHER_IDX, ICON24_PM2P5_IDX, ICON24_NORMAL_ALARM_CLOCK_IDX);	
#endif	
}

#ifdef _CLINGBAND_2_PAY_MODEL_
static void _frame_render_horizontal_carousel_4()
{
  _render_one_icon_24(ICON24_BUS_CARD_IDX, 128);	

  _render_one_icon_24(ICON24_BANK_CARD_IDX, 128+48);	
	
  _render_one_icon_24(ICON24_SETTING_IDX, 128+96);			
}
#endif
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
	I8U *in_data;
	I8U *out = cling.ui.p_oled_up+offset;
	I8U *out_data;

	if (offset > (384 + 120))
		return;
		
	
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
	
	line_len = FONT_load_characters(buf_fonts, (char *)string, height, 128, FALSE);
	
	if (line_len > 32)
		line_len = 32;
	
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
			if (string[i] == ' ') {
				p0 += 4;
				p1 += 4;
				p2 += 4;
				char_len = 4;
			} else if (string[i] == '+') {
				p0 += 13;
				p1 += 13;
				p2 += 13;
				char_len = 13;
			} else {				
				if (b_all_hold) {
				  pin = asset_content+asset_pos[512+string[i]];
				  char_len = asset_len[512+string[i]];	
				}	else {				
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
			}
		} else if (b_24_size == 16) {
			if (string[i] == ' ') {
			  p0 += 4;
			  p1 += 4;
			  p2 += 4;
			  char_len = 4;
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
	
	if (line_len > 32)
		line_len = 32;
	
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

  if (!memcmp(string ,"RUN ", 4))
	  line_len = FONT_load_characters(buf_fonts+4, (char *)string, 16, 128, FALSE);
	else if (!memcmp(string ,"MORE", 4)) 
	  line_len = FONT_load_characters(buf_fonts+1, (char *)string, 16, 128, FALSE);			
	else 
	  line_len = FONT_load_characters(buf_fonts, (char *)string, 16, 128, FALSE);		

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

static void _render_vertical_icon_core(I8U icon_24_idx, I8U b_24_size, I8U offset)
{
	I8U string[4];	
	I8U	margin = 2;

	string[0] = icon_24_idx;
	string[1] = 0;
	_render_vertical_local_character_core(string, offset, margin, b_24_size, FALSE);
}

static void _top_render_vertical_batt_ble()
{
	I8U data_buf[128];

	memset(data_buf, 0, 128);
	
  _render_batt_ble_core(data_buf);
	_rotate_270_degree(data_buf, 384);
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
  if (cling.ui.b_detail_page) {
		_render_vertical_icon_core(ICON8_MORE_IDX, 8, 120);
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

static void _bottom_render_vertical_alarm_clock_detail()
{
	I8U string[16];
	
	sprintf((char *)string, "%02d", cling.ui.vertical_index);
	_render_vertical_fonts_lib_character_core(string, 16, 100);

	_bottom_render_vertical_more();
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_
static void _bottom_render_vertical_ok()
{
  _render_vertical_icon_core(ICON16_OK_IDX, 16, 110);	
}
#endif

#ifdef _CLINGBAND_PACE_MODEL_
static void _bottom_render_vertical_button_hold()
{
	I8U language_type = cling.ui.language_type;	
	
  _render_vertical_fonts_lib_invert_colors_core((I8U *)button_hold_name[language_type], 112);
}
#endif

static void _frame_render_vertical_home()
{
	I8U string[32];	
	I8U	margin = 1;
	I8U b_24_size = 24;
	I8U language_type = cling.ui.language_type;	
	I8U dow_index = cling.time.local.dow;	
	
	// 1. Render the middle
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
	
 	if (BATT_is_charging()) {
		sprintf((char *)string, "%d %%", cling.system.mcu_reg[REGISTER_MCU_BATTERY]);		
	} else {
	  sprintf((char *)string, "%d/%02d", cling.time.local.month, cling.time.local.day);
	}		
 	
  _render_vertical_fonts_lib_character_core((I8U *)week_name[language_type][dow_index], 16, 96);
	
	_render_vertical_fonts_lib_character_core(string, 8, 120);	
	
	// 2. Render the top
	_top_render_vertical_batt_ble();
}

static void _frame_render_vertical_steps()
{
	I8U string[32];
	I32U stat = 0;
	I8U margin = 2;
	I8U b_24_size = 24;
	I32U v_10000 = 0, v_1000 = 0, v_100 = 0;
  I8U language_type = cling.ui.language_type;	
	I8U step_unit_index = 0;
	
	// 1. Render the middle
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
		_middle_render_vertical_character_core((I8U *)vertical_step_name[0][1], 75, (I8U *)vertical_step_name[0][0], 95);
	} else {
		_render_vertical_fonts_lib_character_core((I8U *)vertical_step_name[language_type][step_unit_index], 16, 80);
	}
	
	// 2. Render the bottom	
#ifdef _CLINGBAND_PACE_MODEL_		
  _bottom_render_vertical_small_clock();
#else	
	_bottom_render_vertical_tracker();
#endif
	
	// 3. Render the top		
	_render_vertical_icon_core(ICON24_STEPS_IDX, 24, 0);	
}

static void _frame_render_vertical_distance()
{	
	I8U string[32];	
	I32U stat = 0;
	I8U margin = 2;
	I8U b_24_size = 24;
	I32U v_1000 = 0, v_100 = 0;
	I8U language_type = cling.ui.language_type;
	I8U metric = cling.user_data.profile.metric_distance;	
	
	// 1. Render the middle	
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

	_render_vertical_fonts_lib_character_core((I8U *)distance_unit_name[language_type][metric], 16, 80);	
	
	// 2. Render the bottom	
#ifdef _CLINGBAND_PACE_MODEL_		
  _bottom_render_vertical_small_clock();
#else	
	_bottom_render_vertical_tracker();
#endif
	
	// 3. Render the top		
	_render_vertical_icon_core(ICON24_DISTANCE_IDX, 24, 0);		
}

static void _frame_render_vertical_calories()
{
	I8U string[32];
	I32U stat = 0;
	I8U margin = 2;
	I8U b_24_size = 24;
	I32U v_10000 = 0, v_1000 = 0, v_100 = 0;
  I8U language_type = cling.ui.language_type;	
	I8U calories_unit_index = 0;
	
	// 1. Render the middle	
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
		_middle_render_vertical_character_core((I8U *)calories_unit_vertical_name[language_type][1], 70, (I8U *)calories_unit_vertical_name[language_type][0], 86);
	} else {
	  _render_vertical_fonts_lib_character_core((I8U *)calories_unit_vertical_name[language_type][0], 16, 75);
	}
	
	// 2. Render the bottom	
#ifdef _CLINGBAND_PACE_MODEL_		
  _bottom_render_vertical_small_clock();
#else	
	_bottom_render_vertical_tracker();
#endif
	
	// 3. Render the top		
	_render_vertical_icon_core(ICON24_CALORIES_IDX, 24, 0);			
}

static void _frame_render_vertical_active_time()
{
	I8U string[32];	
	I32U stat = 0;
  I8U	margin = 2;
	I8U b_24_size = 24;
	I8U language_type = cling.ui.language_type;	
	
	// 1. Render the middle		
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

	_render_vertical_fonts_lib_character_core((I8U *)active_time_name[language_type], 16, 80);
	
	// 2. Render the bottom	
#ifdef _CLINGBAND_PACE_MODEL_		
  _bottom_render_vertical_small_clock();
#else	
	_bottom_render_vertical_tracker();
#endif
	
	// 3. Render the top		
	_render_vertical_icon_core(ICON24_ACTIVE_TIME_IDX, 24, 0);				
}

#ifdef _CLINGBAND_UV_MODEL_
static void _frame_render_vertical_uv_index()
{
	I8U string[32];	
  I8U	margin = 2;
	I8U b_24_size = 24;
	I8U integer = 0;
	
	// 1. Render the middle		
	integer = cling.uv.max_UI_uv;
	
	if (integer > 99) {
		sprintf((char *)string, "%d",(integer/10));
	} else {
		sprintf((char *)string, "%d.%d", (integer/10), (integer%10));
	  margin = 1;
	}
	
	_render_vertical_local_character_core(string, 50, margin, b_24_size, FALSE);
	
	// 2. Render the bottom	
#ifdef _CLINGBAND_PACE_MODEL_		
  _bottom_render_vertical_small_clock();
#else	
	_bottom_render_vertical_tracker();
#endif
	
	// 3. Render the top		
	_render_vertical_icon_core(ICON24_UV_INDEX_IDX, 24, 0);					
}
#endif

static void _frame_render_vertical_pm2p5()
{					
	I8U string[32];	
  I8U	margin = 1;
	I8U b_24_size = 24;
	I8U language_type = cling.ui.language_type;			
	WEATHER_CTX *w = &cling.weather;
	
	// 1. Render the middle			
  _middle_render_pm2p5_core();

#ifdef _CLINGBAND_UV_MODEL_	
		language_type = 0;
#else
		language_type++;
#endif	
	
	if (w->pm2p5_value > 99)
		b_24_size = 16;
	else
		margin = 2;

  sprintf((char *)string, "%d", w->pm2p5_value);
	
	if (b_24_size == 24) 
		_render_vertical_local_character_core(string, 42, margin, b_24_size, FALSE);
	else
		_render_vertical_local_character_core(string, 46, margin, b_24_size, FALSE);
	
	_render_vertical_fonts_lib_character_core((I8U *)air_level_name[language_type][w->pm2p5_level_idx], 16, 84);
	
	// 2. Render the bottom	
  _bottom_render_vertical_small_clock();	
	
	// 3. Render the top		
	_render_vertical_icon_core(ICON24_PM2P5_IDX, 24, 0);			
}

static void _frame_render_vertical_weather()
{
	I8U string[32];
	I8U margin = 2;
	I8U b_24_size = 24;
	WEATHER_INFO_CTX weather_info;	

	if (!WEATHER_get_weather_info(0, &weather_info)) {
		weather_info.low_temperature = 15;
		weather_info.high_temperature = 22;
		weather_info.type = 0;
	}

	// 1. Render the top			
	_render_vertical_icon_core(ICON24_WEATHER_IDX+weather_info.type, 24, 0);

	// 2. Render the middle		
	// - Temperature Range - 
	_render_vertical_icon_core(ICON24_WEATHER_RANGE_IDX, 24, 55);

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
	
	// 3. Render the bottom		
	// - Weather temperature unit - 
	_render_vertical_icon_core(ICON24_CELCIUS_IDX, 24, 104);
}

static void _render_vertical_reminder_core(BOOLEAN b_alarm_clock_reminder)
{
	I8U string[32];	
  BOOLEAN b_invalid_alarm = TRUE;
	
  if (b_alarm_clock_reminder) {
		if ((cling.reminder.b_daily_alarm) || (cling.reminder.b_sleep_total)) {
		  b_invalid_alarm = FALSE;
		}			
	} else {
		if (cling.reminder.b_daily_alarm) {
      b_invalid_alarm = FALSE;
		}		
	}

	if ((cling.ui.notific.alarm_clock_hh >= 24) || (cling.ui.notific.alarm_clock_mm >= 60)) {
		b_invalid_alarm = TRUE;
	}
	
	if (b_invalid_alarm) {
		_render_vertical_icon_core(ICON24_NO_SKIN_TOUCH_IDX, 24, 50);
	} else {
	  // Rendering clock ...
		sprintf((char *)string, " %02d", cling.ui.notific.alarm_clock_hh);
		_render_vertical_local_character_core(string, 35, 1, 24, TRUE);		
		
		// Render the minute
		sprintf((char *)string, ":%02d", cling.ui.notific.alarm_clock_mm);
		_render_vertical_local_character_core(string, 65, 1, 24, FALSE);			
	}
}

static void _frame_render_vertical_alarm_clock_reminder()
{
  // 1. Render the middle			
  _render_vertical_reminder_core(TRUE);
	
  // 2. Render the bottom			
	_bottom_render_vertical_more();
	
  // 3. Render the top			
	if ((cling.reminder.alarm_type == SLEEP_ALARM_CLOCK) || (cling.reminder.alarm_type == WAKEUP_ALARM_CLOCK)) 
    _render_vertical_icon_core(ICON24_SLEEP_ALARM_CLOCK_IDX, 24, 0);
	else 
    _render_vertical_icon_core(ICON24_NORMAL_ALARM_CLOCK_IDX, 24, 0);
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _frame_render_vertical_alarm_clock_detail()
{
  // 1. Render the middle			
  _render_vertical_reminder_core(FALSE);
	
  // 2. Render the bottom				
	_bottom_render_vertical_alarm_clock_detail();
	
  // 3. Render the top			
	_render_vertical_icon_core(ICON24_NORMAL_ALARM_CLOCK_IDX, 24, 0);	
}
#endif

static I8U _render_vertical_hr_core(I8U offset)
{
	I8U string[32];		
	I8U margin = 2;
	I8U b_24_size = 24;
	I8U hr_result = 0;
	
	// Second, render heart rate 
	if (cling.hr.b_closing_to_skin || cling.hr.b_start_detect_skin_touch) {
		if (cling.hr.heart_rate_ready) {
#ifdef _ENABLE_PPG_
			//hr_result = PPG_minute_hr_calibrate();
			update_and_push_hr();
			hr_result = cling.hr.m_curr_dispaly_HR;
#endif
			if (hr_result > 99) {
				b_24_size = 16; 
				margin = 1;
			} 
			sprintf((char *)string, "%d", hr_result);		
			if ((b_24_size == 24) && (cling.activity.b_workout_active)) {
				_render_vertical_local_character_core(string, offset, margin, b_24_size, TRUE);
			}	else {
				_render_vertical_local_character_core(string, offset, margin, b_24_size, FALSE);		
			}				
		} else {
			cling.ui.heart_rate_wave_index ++;
			if (cling.ui.heart_rate_wave_index > 2) {
				cling.ui.heart_rate_wave_index = 0;
			}
			sprintf((char *)string, "%s", vertical_hr_wave_indicator[cling.ui.heart_rate_wave_index]);
			b_24_size = 8;
			margin = 3;
			_render_vertical_local_character_core(string, offset+5, margin, b_24_size, FALSE);
			hr_result = 0;
			return hr_result;
		}
	} else {
		_render_vertical_icon_core(ICON24_NO_SKIN_TOUCH_IDX, 24, 50);
	}
	
	return hr_result;	
}

static void _frame_render_vertical_heart_rate()
{
  // 1. Render the middle			
	_render_vertical_hr_core(50);
	
  // 2. Render the bottom			
  _bottom_render_vertical_small_clock();
	
  // 3. Render the top			
	if (cling.ui.icon_sec_blinking)
	  _render_vertical_icon_core(ICON24_HEART_RATE_IDX, 24, 0);		
}

#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)	
static void _frame_render_vertical_skin_temp()
{
	I8U string[32];	
  I8U	margin = 2;
	I8U b_24_size = 24;
	I8U integer = 0;
	
  // 1. Render the middle				
  integer = cling.therm.current_temperature/10;
	
	// Temperature value
	sprintf((char *)string, "%d", integer);
	_render_vertical_local_character_core((I8U *)string, 42, margin, b_24_size, FALSE);
	
	// - Skin temperature unit - 
	_render_vertical_icon_core(ICON24_CELCIUS_IDX, 24, 80);
	
  // 2. Render the bottom			
  _bottom_render_vertical_small_clock();
	
  // 3. Render the top			
	if (cling.ui.icon_sec_blinking)
	  _render_vertical_icon_core(ICON24_SKIN_TEMP_IDX, 24, 0);			
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_
static void _frame_render_vertical_stopwatch_start()
{
	I8U string[32];	
	I8U margin = 1;
	I8U b_24_size = 24;	
	UI_STOPWATCH_CTX *stopwatch = &cling.ui.stopwatch;	
	
  // 1. Render the middle			
	// Get stopwatch display time.
	_middle_render_stopwatch_core();
	
	// Render the time	
	if (stopwatch->hour) {
	// Render the hour
	  sprintf((char *)string, " +%d", stopwatch->hour);
    _render_vertical_local_character_core(string, 28, margin, b_24_size, TRUE);		
		
	  // Render the minute
	  sprintf((char *)string, ":%02d", stopwatch->min);
    _render_vertical_local_character_core(string, 56, margin, b_24_size, FALSE);		

	  // Render the second
	  sprintf((char *)string, ":%02d", stopwatch->sec);
    _render_vertical_local_character_core(string, 84, margin, b_24_size, FALSE);			
	} else {
	  // Render the minute		
    if (stopwatch->min > 9) 
	    sprintf((char *)string, " %02d", stopwatch->min);
		else 
	    sprintf((char *)string, "+ %d", stopwatch->min);		
		
		_render_vertical_local_character_core(string, 28, margin, b_24_size, FALSE);	
		
		// Render the second
	  sprintf((char *)string, ":%02d", stopwatch->sec);
    _render_vertical_local_character_core(string, 56, margin, b_24_size, FALSE);			
		
		// Render the millisecond
	  sprintf((char *)string, "  .   ");		
		_render_vertical_local_character_core(string, 76, margin, b_24_size, FALSE);	
		b_24_size = 16;
	  sprintf((char *)string, "   %02d", stopwatch->ms);
    _render_vertical_local_character_core(string, 84, margin, b_24_size, FALSE);		
	}
	
  // 2. Render the bottom				
	if (stopwatch->b_in_pause_mode)
		_render_vertical_icon_core(ICON16_STOPWATCH_START_IDX, 16, 110);
	else
		_render_vertical_icon_core(ICON16_STOPWATCH_STOP_IDX, 16, 110);
	
  // 3. Render the top			
	_render_vertical_icon_core(ICON24_STOPWATCH_IDX, 24, 0);				
}

static void _frame_render_vertical_stopwatch_stop()
{
	I8U language_type = cling.ui.language_type;

  // 1. Render the middle			
	_render_vertical_fonts_lib_character_core((I8U *)stopwatch_stop_name[language_type], 16, 55);
	
  // 2. Render the bottom			
  _bottom_render_vertical_ok();
	
  // 3. Render the top			
	_render_vertical_icon_core(ICON24_STOPWATCH_IDX, 24, 0);			
}
#endif

static void _vertical_core_run_distance(I32U stat, BOOLEAN b_all_hold, I8U *string1, I8U *string2)
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

  if (b_all_hold) {	
	  if (cling.ui.icon_sec_blinking)	{	
	    _render_vertical_fonts_lib_character_core(string1, 16, 28);
		}
	} else {
	  _render_vertical_fonts_lib_character_core(string1, 16, 28);		
	}
	
	_render_vertical_fonts_lib_character_core(string2, 16, 112);	
}

static void _frame_render_vertical_running_distance()
{
	I8U language_type = cling.ui.language_type;		
	I8U metric = cling.user_data.profile.metric_distance;	
	
	// 1. Render the middle and bottom
	_vertical_core_run_distance(cling.run_stat.distance, FALSE, (I8U *)running_distance_name[language_type], (I8U *)distance_unit_name[language_type][metric]);

	// 2. Render the top
	_render_vertical_icon_core(ICON24_RUNNING_DISTANCE_IDX, 24, 0);			
	
}

static void _frame_render_vertical_training_distance()
{
	I8U language_type = cling.ui.language_type;		
	I8U metric = cling.user_data.profile.metric_distance;	

	// 1. Render the middle and bottom	
	_vertical_core_run_distance(cling.train_stat.distance, TRUE, (I8U *)running_distance_name[language_type], (I8U *)distance_unit_name[language_type][metric]);
	
	// 2. Render the top
	_render_vertical_icon_core(ICON24_RUNNING_DISTANCE_IDX, 24, 0);			
	
	update_and_push_hr();
}

static void _frame_render_vertical_running_time()
{
	I8U string[32];	
	I8U margin = 1;
	I8U b_24_size = 24;	
	I8U hour=0, sec=0;	
	I16U min = 0;
	I8U language_type = cling.ui.language_type;

	// 1. Render the middle and bottom		
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
	
	// 2. Render the top
	_render_vertical_icon_core(ICON24_RUNNING_TIME_IDX, 24, 0);			
}

static void _frame_render_vertical_training_time()
{
	I8U string[32];	
	I8U margin = 1;
	I8U b_24_size = 24;	
	I8U language_type = cling.ui.language_type;
  UI_RUNNING_INFO_CTX *running_info = &cling.ui.running_info;
	
	// 1. Render the middle and bottom
	if (cling.ui.icon_sec_blinking) {
		_render_vertical_fonts_lib_character_core((I8U *)running_time_name[language_type], 16, 28);
	} 
	
	// Get display total training time; 
	_get_training_time_core();

	if (running_info->hour) {
		// Render the hour
		sprintf((char *)string, " +%d", running_info->hour);
    _render_vertical_local_character_core(string, 52, margin, b_24_size, TRUE);				
		// Render the minute
		sprintf((char *)string, ":%02d", running_info->min);
    _render_vertical_local_character_core(string, 78, margin, b_24_size, TRUE);				
		// Render the second
		sprintf((char *)string, ":%02d", running_info->sec);
    _render_vertical_local_character_core(string, 104, margin, b_24_size, TRUE);				
	} else {
		// Render the minute
		if (running_info->min > 9) {
			sprintf((char *)string, " %02d", running_info->min);
		} else {
			sprintf((char *)string, " +%d", running_info->min);
		}
    _render_vertical_local_character_core(string, 75, margin, b_24_size, TRUE);		
		
		// Render the second
		sprintf((char *)string, ":%02d", running_info->sec);
    _render_vertical_local_character_core(string, 104, margin, b_24_size, TRUE);				
	}
	
  // 2. Render the top			
	_render_vertical_icon_core(ICON24_RUNNING_TIME_IDX, 24, 0);				
	
	update_and_push_hr();
}

static void _frame_render_vertical_running_pace()
{
	I8U string[32];	
	I8U margin = 1;
	I8U b_24_size = 24;	
	I32U pace = cling.run_stat.time_min; // Rounding 
	I32U min, sec;
	I8U language_type = cling.ui.language_type;	

	// 1. Render the middle and bottom
	_middle_render_vertical_character_core((I8U *)gym_workout_run_name[language_type], 28, (I8U *)pace_name[language_type], 46);		
	
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
	
	// Render the minute
	if (min > 9)
		sprintf((char *)string, "%02d/", min);
	else
		sprintf((char *)string, "+%d/", min);
	
	if ((!min) && (!sec)) {
		margin = 0;
    sprintf((char *)string, "+  - /");
	} 

	_render_vertical_local_character_core(string, 76, margin, b_24_size, FALSE);		

	if ((!min) && (!sec)) {
		margin = 0;
	  // Render the second
	  sprintf((char *)string, "   - - *");		
	} else {
		margin = 1;
	  // Render the second
	  sprintf((char *)string, "%02d*", sec);				
	}
	
	_render_vertical_local_character_core(string, 104, margin, b_24_size, FALSE);		
	
	// 2. Render the top
	_render_vertical_icon_core(ICON24_RUNNING_PACE_IDX, 24, 0);				
}

static void _frame_render_vertical_running_hr()
{
	I8U string[32];
	I8U margin = 0;
	I8U b_24_size = 16;
	I32U hr = cling.run_stat.accu_heart_rate;
	I8U language_type = cling.ui.language_type;	
	
	// 1. Render the middle and bottom	
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

	_middle_render_vertical_character_core((I8U *)avg_name[language_type], 28, (I8U *)heart_rate_name[language_type], 46);		
	
	// 2. Render the top
	_render_vertical_icon_core(ICON24_RUNNING_HR_IDX, 24, 0);				
}

static void _render_vertical_run_calories_core(I32U run_calories, BOOLEAN icon_blinking)
{
	I8U string[32];
	I8U margin = 0;
	I8U b_24_size = 16;
	I32U stat = 0;	
	I32U v_10000 = 0, v_1000 = 0, v_100 = 0;
	I8U language_type = cling.ui.language_type;	
	I8U calories_unit_index = 0;
	
	// 1. Render the middle	
	if (icon_blinking) {
		if (cling.ui.icon_sec_blinking) {
	    _render_vertical_fonts_lib_character_core((I8U *)calories_name[language_type], 16, 28);
		}
	} else {
	  _render_vertical_fonts_lib_character_core((I8U *)calories_name[language_type], 16, 28);		
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

	// 2. Render the bottom			
	if (stat > 999) {
    calories_unit_index = 1;
	} else {		
    calories_unit_index = 0;
	}
	
	if (calories_unit_index) {
		_middle_render_vertical_character_core((I8U *)calories_unit_vertical_name[language_type][1], 96, (I8U *)calories_unit_vertical_name[language_type][0], 112);		
	} else {
	  _render_vertical_fonts_lib_character_core((I8U *)calories_unit_vertical_name[language_type][0], 16, 112);		
	}
	
	// 3. Render the top		
  _render_vertical_icon_core(ICON24_RUNNING_CALORIES_IDX, 24, 0);		
}

static void _frame_render_vertical_running_calories()
{
  _render_vertical_run_calories_core(cling.run_stat.calories, FALSE);	
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _frame_render_vertical_training_calories()
{
  _render_vertical_run_calories_core(cling.train_stat.calories, TRUE);	
}
#endif

static void _frame_render_vertical_running_cadence()
{		
	I8U string[32];	
	I8U margin = 0;
	I8U b_24_size = 16;	
	I32U stat = 0;
	I8U language_type = cling.ui.language_type;		
	
	// 1. Render the middle	and bootom	
	_render_vertical_fonts_lib_character_core((I8U *)cadence_rate_name[language_type], 16, 28);
	
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
	_render_vertical_fonts_lib_character_core((I8U *)cadence_spm_name[language_type], 16, 112);
	
	// 2. Render the top		
  _render_vertical_icon_core(ICON24_RUNNING_CADENCE_IDX, 24, 0);			
}

static void _frame_render_vertical_running_stride()
{
	I8U string[32];	
	I8U margin = 0;
	I8U b_24_size = 16;	
	I32U stat = 0;
	I8U language_type = cling.ui.language_type;			
	I8U metric = cling.user_data.profile.metric_distance;
	
	// 1. Render the middle	and bootom		
	_render_vertical_fonts_lib_character_core((I8U *)vertical_stride_name[language_type], 16, 28);
	
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
	
	_render_vertical_fonts_lib_character_core((I8U *)stride_unit_name[language_type][metric], 16, 112);
	
	// 2. Render the top		
  _render_vertical_icon_core(ICON24_RUNNING_STRIDE_IDX, 24, 0);				
}

static void _middle_render_vertical_start_or_stop_run_core(I8U *string1, I8U *string2)
{
	I8U language_type = cling.ui.language_type;	
	
	if (language_type == LANGUAGE_TYPE_ENGLISH)
	  _middle_render_vertical_character_core(string2, 50, string1, 70);	
	else
	  _middle_render_vertical_character_core(string1, 50, string2, 70);			
}

static void _frame_render_vertical_training_run_start()
{	
	I8U language_type = cling.ui.language_type;		

	// 1. Render the middle		
	_middle_render_vertical_start_or_stop_run_core((I8U *)start_run_name[language_type], (I8U *)training_run_name[language_type]);

	// 2. Render the bottom	
#ifdef _CLINGBAND_PACE_MODEL_	
  _bottom_render_vertical_button_hold();
#else	
  _bottom_render_vertical_ok();
#endif
	
	// 3. Render the top	
  _render_vertical_icon_core(ICON24_RUNNING_DISTANCE_IDX, 24, 0);			
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _frame_render_vertical_cycling_outdoor_start()
{
	I8U language_type = cling.ui.language_type;		

	// 1. Render the middle	
	_middle_render_vertical_start_or_stop_run_core((I8U *)start_run_name[language_type], (I8U *)cycling_run_name[language_type]);

	// 2. Render the bottom
  _bottom_render_vertical_ok();

	// 3. Render the top		
  _render_vertical_icon_core(ICON24_CYCLING_OUTDOOR_MODE_IDX, 24, 0);			
	
	cling.train_stat.b_cycling_state = FALSE;
}
#endif

static void _render_vertical_running_stop_buttom_top()
{
	// 2. Render the bottom
#ifdef _CLINGBAND_PACE_MODEL_	
  _bottom_render_vertical_button_hold();
#else	
  _bottom_render_vertical_ok();
#endif
	
	// 3. Render the top		
	if (cling.ui.icon_sec_blinking)
    _render_vertical_icon_core(ICON24_RUNNING_STOP_IDX, 24, 0);			
}

static void _frame_render_vertical_training_run_stop()
{
	I8U language_type = cling.ui.language_type;		

	// 1. Render the middle
	_middle_render_vertical_start_or_stop_run_core((I8U *)stop_run_name[language_type], (I8U *)training_run_name[language_type]);

	// 2. Render the bottom and top	
  _render_vertical_running_stop_buttom_top();
	
	update_and_push_hr();
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _frame_render_vertical_training_workout_stop()
{
	I8U language_type = cling.ui.language_type;		

	// 1. Render the middle
	_middle_render_vertical_start_or_stop_run_core((I8U *)stop_run_name[language_type], (I8U *)gym_workout_run_name[language_type]);	

	// 2. Render the bottom and top	
  _render_vertical_running_stop_buttom_top();
}

static void _frame_render_vertical_cycling_outdoor_stop()
{
	I8U language_type = cling.ui.language_type;		

	// 1. Render the middle	
	_middle_render_vertical_start_or_stop_run_core((I8U *)stop_run_name[language_type], (I8U *)cycling_run_name[language_type]);		

	// 2. Render the bottom and top	
  _render_vertical_running_stop_buttom_top();	
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_
static void _frame_render_vertical_training_run_or_analysis()
{
	I8U language_type = cling.ui.language_type;	

	// 1. Render the middle and bottom		
	_render_vertical_fonts_lib_invert_colors_core((I8U *)run_start_name[language_type], 60);	
		
	_render_vertical_fonts_lib_invert_colors_core((I8U *)run_analysis_name[language_type], 100);	
	
	// 2. Render the top	
  _render_vertical_icon_core(ICON24_RUNNING_DISTANCE_IDX, 24, 0);				
}
#endif

#ifdef _CLINGBAND_PACE_MODEL_
static void _frame_render_vertical_running_analysis()
{
	const	char *const running_analysis_name_1[] = {"RUN", "当日 ", "當日 "};	
	const	char *const running_analysis_name_2[] = {"DATA", "跑步 ", "跑步 "};	
	I8U language_type = cling.ui.language_type;		

	// 1. Render the middle.
	_middle_render_vertical_character_core((I8U *)running_analysis_name_1[language_type], 50, (I8U *)running_analysis_name_2[language_type], 70);		
	
	// 2. Render the bottom.
	_bottom_render_vertical_button_hold();
	
	// 3. Render the top	
  _render_vertical_icon_core(ICON24_RUNNING_PACE_IDX, 24, 0);					
}
#endif

#ifdef _CLINGBAND_PACE_MODEL_
static void _frame_render_vertical_running_stop_analysis()
{
	const	char *const stop_analysis_name_1[] = {"INFO", "结束 ", "結束 "};				
	const	char *const stop_analysis_name_2[] = {"END", "分析 ", "分析 "};				
	I8U language_type = cling.ui.language_type;			

	// 1. Render the middle.	
	_middle_render_vertical_character_core((I8U *)stop_analysis_name_1[language_type], 50, (I8U *)stop_analysis_name_2[language_type], 70);		
	
	// 2. Render the bottom.
	_bottom_render_vertical_button_hold();
	
	// 3. Render the top	
  _render_vertical_icon_core(ICON24_RUNNING_STOP_IDX, 24, 0);				
}
#endif

#ifdef _CLINGBAND_PACE_MODEL_		
static void _frame_render_vertical_connect_gps()
{
	I8U language_type = cling.ui.language_type;		

	_render_vertical_fonts_lib_character_core((I8U *)vertical_gps_conn_name[language_type], 16, 20);
	_render_vertical_fonts_lib_character_core((I8U *)vertical_phone_name[language_type], 16, 40);
	_render_vertical_fonts_lib_character_core((I8U *)vertical_gps_name[language_type], 16, 75);
}

static void _frame_render_vertical_connect_gps_timeout()
{
	I8U language_type = cling.ui.language_type;		

	// 1. Render the middle	
	_render_vertical_fonts_lib_character_core((I8U *)vertical_gps_conn_name[language_type], 16, 55);
	_render_vertical_fonts_lib_character_core((I8U *)vertical_gps_timeout_name[language_type], 16, 75);
	
	// 2. Render the top	
  _render_vertical_icon_core(ICON24_GPS_CONN_TIMEOUT_IDX, 24, 0);			
}

static void _frame_render_vertical_connect_gps_fail()
{
	I8U language_type = cling.ui.language_type;		

	// 1. Render the middle
	_render_vertical_fonts_lib_character_core((I8U *)vertical_gps_conn_name[language_type], 16, 55);
	_render_vertical_fonts_lib_character_core((I8U *)vertical_gps_fail_name[language_type], 16, 75);
	
	// 2. Render the top	
  _render_vertical_icon_core(ICON24_BLE_IDX, 24, 0);		
}
#endif

static BOOLEAN _middle_vertical_running_ready_core()
{
	I8U margin = 4;
	I8U b_24_size = 24;	
  I32U t_curr_in_ms = CLK_get_system_time();
  BOOLEAN b_ready_finished = FALSE;	
  UI_RUNNING_INFO_CTX	*running_info = &cling.ui.running_info;

	// First Update touch time stamp.
  cling.ui.touch_time_stamp = t_curr_in_ms;
	
	if (!running_info->t_ready_stamp)
		running_info->t_ready_stamp = t_curr_in_ms;
	
	if (t_curr_in_ms  > (running_info->t_ready_stamp + 800)) {
		running_info->t_ready_stamp = t_curr_in_ms;		
		running_info->ready_idx++;
	}
	
	if (running_info->ready_idx > 3) {
		running_info->ready_idx = 3;
		b_ready_finished = TRUE;
    cling.activity.b_workout_active = TRUE;		
	}

	_render_vertical_local_character_core((I8U *)ready_indicator_name[running_info->ready_idx], 60, margin, b_24_size, TRUE);		
	
	return b_ready_finished;	
}

static void _frame_render_vertical_training_ready()
{
  BOOLEAN b_ready_finished = FALSE;	
	
	// 1. Render the middle and bottom		
	b_ready_finished = _middle_vertical_running_ready_core();
	
	if (b_ready_finished) {
		cling.ui.frame_index = UI_DISPLAY_TRAINING_STAT_TIME;
		cling.ui.frame_next_idx = cling.ui.frame_index;	
	}
	
	// 2. Render the top	
  _render_vertical_icon_core(ICON24_RUNNING_DISTANCE_IDX, 24, 0);				
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _frame_render_vertical_cycling_outdoor_ready()
{	
  BOOLEAN b_ready_finished = FALSE;	
	
	// 1. Render the middle		
	b_ready_finished = _middle_vertical_running_ready_core();
	
	if (b_ready_finished) {	
		cling.ui.frame_index = UI_DISPLAY_CYCLING_OUTDOOR_STAT_TIME;
		cling.ui.frame_next_idx = cling.ui.frame_index;
	}
	
	// 2. Render the top			
  _render_vertical_icon_core(ICON24_CYCLING_OUTDOOR_MODE_IDX, 24, 0);			
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_
static void _frame_render_vertical_workout_ready()
{
  BOOLEAN b_ready_finished = FALSE;	
	
	b_ready_finished = _middle_vertical_running_ready_core();
	
	if (b_ready_finished) {
		cling.ui.frame_index = UI_DISPLAY_WORKOUT_RT_TIME;
		cling.ui.frame_next_idx = cling.ui.frame_index;
	}
}
#endif

static void _frame_render_vertical_training_pace()
{
	I8U string[32];	
	I8U margin = 1;
	I8U b_24_size = 24;		
	I32U min, sec;
	I8U language_type = cling.ui.language_type;		
	
	// 1. Render the middle and bottom
	min = cling.run_stat.last_10sec_pace_min;
	sec = cling.run_stat.last_10sec_pace_sec;

	if ((!min) && (!sec)) {
		margin = 0;
    sprintf((char *)string, "+  - /");
	} else {
	  if (min > 9)
		  sprintf((char *)string, "%02d/", min);
	  else
		  sprintf((char *)string, "+%d/", min);
	}
	
	_render_vertical_local_character_core(string, 76, margin, b_24_size, TRUE);	

	// Render the seconds
	if ((!min) && (!sec)) {
		margin = 0;
	  // Render the minute
	  sprintf((char *)string, "   - - *");		
	} else {
		margin = 1;
	  // Render the minute
	  sprintf((char *)string, "%02d*", sec);				
	}
	
	_render_vertical_local_character_core(string, 104, margin, b_24_size, TRUE);	
	
	if (cling.ui.icon_sec_blinking) {	
		_render_vertical_fonts_lib_character_core((I8U *)pace_name[language_type], 16, 28);
	}
	
	// 2. Render the top	
  _render_vertical_icon_core(ICON24_RUNNING_PACE_IDX, 24, 0);				
	
	update_and_push_hr();
}

static void _frame_render_vertical_training_hr()
{
	I8U string[32];	
	I8U hr_result = 0;
	I8U i;		
	I32U hr_perc = 0;
	I8U *p0, *p1, *p2, *p3;
	I8U language_type = cling.ui.language_type;		
	
	// 1. Render the middle
	if (cling.ui.icon_sec_blinking) {		
		_render_vertical_fonts_lib_character_core((I8U *)heart_rate_name[language_type], 16, 28);
	}
	
	hr_result = _render_vertical_hr_core(60);
	
  if (hr_result) {
	  hr_perc = (hr_result * 100)/(220-cling.user_data.profile.age);	
	  if (hr_perc > 98)
		  hr_perc = 98;
		
	  sprintf((char *)string, "%d%%", hr_perc);
	  _render_vertical_fonts_lib_character_core(string, 16, 94);
  }
	
	// 2. Render the bottom	
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
	
	// 3. Render the top		
  _render_vertical_icon_core(ICON24_RUNNING_HR_IDX, 24, 0);			
}

#ifndef _CLINGBAND_PACE_MODEL_
static void _middle_render_vertical_no_ble_core()
{
	I8U language_type = cling.ui.language_type;		

	_render_vertical_icon_core(ICON24_NO_SKIN_TOUCH_IDX, 24, 55);

  _middle_render_vertical_character_core((I8U *)no_ble_vertical_name_1[language_type], 92, (I8U *)no_ble_vertical_name_2[language_type], 112);					
}

static void _frame_render_vertical_cycling_outdoor_distance()
{
	I8U language_type = cling.ui.language_type;		
	I8U metric = cling.user_data.profile.metric_distance;	
	
	// 1. Render the middle	
	if (BTLE_is_connected()) {
		_vertical_core_run_distance(cling.train_stat.distance, TRUE, (I8U *)cycling_run_name[language_type], (I8U *)distance_unit_name[language_type][metric]);
	} else {
    _middle_render_vertical_no_ble_core();					
	}
	
	// 2. Render the top
  _render_vertical_icon_core(ICON24_CYCLING_OUTDOOR_MODE_IDX, 24, 0);	
}

static void _frame_render_vertical_cycling_outdoor_speed()
{
	I8U language_type = cling.ui.language_type;		
	I8U metric = cling.user_data.profile.metric_distance;	
	
	// 1. Render the middle		
	if (BTLE_is_connected()) {
		_vertical_core_run_distance(cling.train_stat.speed, TRUE, (I8U *)run_speed_name[language_type], NULL);
		_render_vertical_fonts_lib_character_core((I8U *)speed_unit_name[metric], 8, 120);
	} else {
    _middle_render_vertical_no_ble_core();							
	}
	
	// 2. Render the top	
  _render_vertical_icon_core(ICON24_CYCLING_OUTDOOR_SPEED_IDX, 24, 0);		
}
#endif

#ifndef _CLINGBAND_PACE_MODEL_
static void _render_vertical_carousel_core(I8U top_idx, I8U middle_idx, I8U bottom_idx)
{
  _render_vertical_icon_core(top_idx, 24, 4);

  _render_vertical_icon_core(middle_idx, 24, 4+48);

  _render_vertical_icon_core(bottom_idx, 24, 4+96);
}

static void _frame_render_vertical_carousel_1()
{
  _render_vertical_carousel_core(ICON24_RUNNING_MODE_IDX, ICON24_CYCLING_OUTDOOR_MODE_IDX, ICON24_WORKOUT_MODE_IDX);	
}
#endif

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
static void _frame_render_vertical_carousel_2()
{
  _render_vertical_carousel_core(ICON24_MUSIC_IDX, ICON24_STOPWATCH_IDX, ICON24_MESSAGE_IDX);	
}
#endif

#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	
static void _frame_render_vertical_carousel_2()
{
  _render_vertical_carousel_core(ICON24_MESSAGE_IDX, ICON24_STOPWATCH_IDX, ICON24_WEATHER_IDX);	
}
#endif

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
static void _frame_render_vertical_carousel_3()
{
  _render_vertical_carousel_core(ICON24_WEATHER_IDX, ICON24_PM2P5_IDX, ICON24_NORMAL_ALARM_CLOCK_IDX);	
}
#endif

#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	
static void _frame_render_vertical_carousel_3()
{
  _render_vertical_carousel_core(ICON24_PM2P5_IDX, ICON24_NORMAL_ALARM_CLOCK_IDX, ICON24_SETTING_IDX);		
}
#endif

#ifdef _CLINGBAND_2_PAY_MODEL_
static void _frame_render_vertical_carousel_4()
{
  _render_vertical_icon_core(ICON24_BUS_CARD_IDX, 24, 0);	

  _render_vertical_icon_core(ICON24_BANK_CARD_IDX, 24, 48);	

  _render_vertical_icon_core(ICON24_SETTING_IDX, 24, 4+96);	
}
#endif

void UI_frame_display_appear(I8U frame_index, BOOLEAN b_render)
{
	N_SPRINTF("[UI] frame appear: %d, %d", frame_index, u->frame_cached_index);

	if (frame_index >= UI_DISPLAY_PREVIOUS)
		return;
	
	_core_frame_display(frame_index, b_render);
}

typedef struct tagUI_RENDER_CTX {
  void (*frame_render_display)();
} UI_RENDER_CTX;

/*******************************************************************************************/
/********************* horizontal frame display ********************************************/
/*******************************************************************************************/
const UI_RENDER_CTX horizontal_ui_render[] = {
  // 0: Home
	{_frame_render_horizontal_home},                     /*UI_DISPLAY_HOME_CLOCK*/
	
	// 1: System	
	{_frame_render_horizontal_system_restart},           /*UI_DISPLAY_SYSTEM_RESTART*/
	{_frame_render_horizontal_ota},                      /*UI_DISPLAY_SYSTEM_OTA*/
	{_frame_render_horizontal_linking},                  /*UI_DISPLAY_SYSTEM_LINKING*/
	{_frame_render_horizontal_unauthorized},             /*UI_DISPLAY_SYSTEM_UNAUTHORIZED*/
	{_frame_render_horizontal_charging},                 /*UI_DISPLAY_SYSTEM_BATT_POWER*/
	
	// 2: a set of the typical use cases	
	{_frame_render_horizontal_steps},                    /*UI_DISPLAY_TRACKER_STEP*/
	{_frame_render_horizontal_distance},                 /*UI_DISPLAY_TRACKER_DISTANCE*/
	{_frame_render_horizontal_calories},                 /*UI_DISPLAY_TRACKER_CALORIES*/
	{_frame_render_horizontal_active_time},              /*UI_DISPLAY_TRACKER_ACTIVE_TIME*/
#ifdef _CLINGBAND_UV_MODEL_		
	{_frame_render_horizontal_uv_idx},                   /*UI_DISPLAY_TRACKER_UV_IDX*/
#endif
	
	// 3: a set of smart features	
	{_frame_render_horizontal_pm2p5},                    /*UI_DISPLAY_SMART_PM2P5*/
  {_frame_render_horizontal_weather}, 	               /*UI_DISPLAY_SMART_WEATHER*/
#ifndef _CLINGBAND_PACE_MODEL_		
	{_frame_render_horizontal_message},                  /*UI_DISPLAY_SMART_MESSAGE*/
	{_frame_render_horizontal_app_notif},                /*UI_DISPLAY_SMART_APP_NOTIF*/
#endif	
	{_frame_render_horizontal_detail_notif},             /*UI_DISPLAY_SMART_DETAIL_NOTIF*/
	{_frame_render_horizontal_incoming_call},            /*UI_DISPLAY_SMART_INCOMING_CALL*/
	{_frame_render_horizontal_incoming_msg},             /*UI_DISPLAY_SMART_INCOMING_MESSAGE*/	
	{_frame_render_horizontal_alarm_clock_reminder},     /*UI_DISPLAY_SMART_ALARM_CLOCK_REMINDER*/	
#ifndef _CLINGBAND_PACE_MODEL_			
	{_frame_render_horizontal_alarm_clock_detail},       /*UI_DISPLAY_SMART_ALARM_CLOCK_DETAIL*/	
#endif	
  {_frame_render_horizontal_idle_alert},               /*UI_DISPLAY_SMART_IDLE_ALERT*/	
	{_frame_render_horizontal_heart_rate},               /*UI_DISPLAY_SMART_HEART_RATE_ALERT*/	
	{_frame_render_horizontal_steps},                    /*UI_DISPLAY_SMART_STEP_10K_ALERT*/	           
#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)	
  {_frame_render_horizontal_sos_alert},                /*UI_DISPLAY_SMART_SOS_ALERT*/		
#endif	
	
	// 4: a set of VITAL	
	{_frame_render_horizontal_heart_rate},               /*UI_DISPLAY_VITAL_HEART_RATE*/	
#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)	
  {_frame_render_horizontal_skin_temp},                /*UI_DISPLAY_VITAL_SKIN_TEMP*/	
#endif
	
#ifndef _CLINGBAND_PACE_MODEL_		
	{_frame_render_horizontal_system_restart},           /*UI_DISPLAY_SETTING_VER*/	
  {_frame_render_horizontal_stopwatch_start},	         /*UI_DISPLAY_STOPWATCH_START*/	
  {_frame_render_horizontal_stopwatch_stop},	         /*UI_DISPLAY_STOPWATCH_STOP*/		
	{_frame_render_horizontal_workout_mode_switch},      /*UI_DISPLAY_WORKOUT_TREADMILL*/	
	{_frame_render_horizontal_workout_mode_switch},      /*UI_DISPLAY_WORKOUT_CYCLING*/	
	{_frame_render_horizontal_workout_mode_switch},      /*UI_DISPLAY_WORKOUT_STAIRS*/	
	{_frame_render_horizontal_workout_mode_switch},      /*UI_DISPLAY_WORKOUT_ELLIPTICAL*/	
	{_frame_render_horizontal_workout_mode_switch},      /*UI_DISPLAY_WORKOUT_ROW*/	
	{_frame_render_horizontal_workout_mode_switch},      /*UI_DISPLAY_WORKOUT_AEROBIC*/	
	{_frame_render_horizontal_workout_mode_switch},      /*UI_DISPLAY_WORKOUT_PILOXING*/		
	{_frame_render_horizontal_workout_mode_switch},      /*UI_DISPLAY_WORKOUT_OTHERS*/		
	{_frame_render_horizontal_workout_ready},            /*UI_DISPLAY_WORKOUT_RT_READY*/			
	{_frame_render_horizontal_training_time},            /*UI_DISPLAY_WORKOUT_RT_TIME*/			
	{_frame_render_horizontal_training_hr},              /*UI_DISPLAY_WORKOUT_RT_HEART_RATE*/			
	{_frame_render_horizontal_training_calories},        /*UI_DISPLAY_WORKOUT_RT_CALORIES*/			
	{_frame_render_horizontal_workout_stop},             /*UI_DISPLAY_WORKOUT_RT_END*/			
#endif	
	
	// 5: Running Statistics	
#ifdef _CLINGBAND_PACE_MODEL_			
	{_frame_render_horizontal_running_analysis},         /*UI_DISPLAY_RUNNING_STAT_RUN_ANALYSIS*/		
#endif	
	{_frame_render_horizontal_running_distance},         /*UI_DISPLAY_RUNNING_STAT_DISTANCE*/				
	{_frame_render_horizontal_running_time},             /*UI_DISPLAY_RUNNING_STAT_TIME*/			
	{_frame_render_horizontal_running_pace},             /*UI_DISPLAY_RUNNING_STAT_PACE*/			
	{_frame_render_horizontal_running_stride},           /*UI_DISPLAY_RUNNING_STAT_STRIDE*/			
	{_frame_render_horizontal_running_cadence},          /*UI_DISPLAY_RUNNING_STAT_CADENCE*/		
	{_frame_render_horizontal_running_hr},               /*UI_DISPLAY_RUNNING_STAT_HEART_RATE*/			
	{_frame_render_horizontal_running_calories},         /*UI_DISPLAY_RUNNING_STAT_CALORIES*/				
#ifdef _CLINGBAND_PACE_MODEL_		
	{_frame_render_horizontal_running_stop_analysis},    /*UI_DISPLAY_RUNNING_STAT_STOP_ANALYSIS*/		
#endif	
	
	// 6: training Statistics	
	{_frame_render_horizontal_training_start_run},       /*UI_DISPLAY_TRAINING_STAT_START*/			
#ifndef _CLINGBAND_PACE_MODEL_		
	{_frame_render_horizontal_training_run_or_analysis}, /*UI_DISPLAY_TRAINING_STAT_START_OR_ANALYSIS*/		
#endif
#ifdef _CLINGBAND_PACE_MODEL_		
	{_frame_render_horizontal_connect_gps},              /*UI_DISPLAY_TRAINING_STAT_CONNECT_GPS*/			
	{_frame_render_horizontal_connect_gps_timeout},      /*UI_DISPLAY_TRAINING_STAT_CONNECT_GPS_TIMEOUT*/			
	{_frame_render_horizontal_connect_gps_fail},         /*UI_DISPLAY_TRAINING_STAT_CONNECT_GPS_FAIL*/	
#endif	
	{_frame_render_horizontal_training_ready},           /*UI_DISPLAY_TRAINING_STAT_READY*/			
	{_frame_render_horizontal_training_time},            /*UI_DISPLAY_TRAINING_STAT_TIME*/	
	{_frame_render_horizontal_training_distance},        /*UI_DISPLAY_TRAINING_STAT_DISTANCE*/		
	{_frame_render_horizontal_training_pace},            /*UI_DISPLAY_TRAINING_STAT_PACE*/			
	{_frame_render_horizontal_training_hr},              /*UI_DISPLAY_TRAINING_STAT_HEART_RATE*/	
	{_frame_render_horizontal_training_run_stop},        /*UI_DISPLAY_TRAINING_STAT_RUN_STOP*/	

	// 7: Cycling Outdoor Statistics	
#ifndef _CLINGBAND_PACE_MODEL_		
	{_frame_render_horizontal_cycling_outdoor_start},    /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_START*/		
	{_frame_render_horizontal_cycling_outdoor_ready},    /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_READY*/		
	{_frame_render_horizontal_training_time},            /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_TIME*/			
  {_frame_render_horizontal_cycling_outdoor_distance}, /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_DISTANCE*/			
  {_frame_render_horizontal_cycling_outdoor_speed},    /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_SPEED*/	
	{_frame_render_horizontal_training_hr},              /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_HEART_RATE*/	
	{_frame_render_horizontal_cycling_outdoor_stop},     /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_STOP*/	

	// 8: MUSIC	
#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
	{_frame_render_horizontal_music_play},               /*UI_DISPLAY_MUSIC_PLAY*/	
	{_frame_render_horizontal_music_track},              /*UI_DISPLAY_MUSIC_SONG*/		
	{_frame_render_horizontal_music_volume},             /*UI_DISPLAY_MUSIC_VOLUME*/		
#endif	

		// 9: Pay	
#ifdef _CLINGBAND_2_PAY_MODEL_		
	{_frame_render_horizontal_bus_card_balance_enquiry}, /*UI_DISPLAY_PAY_BUS_CARD_BALANCE_ENQUIRY*/		
	{_frame_render_horizontal_bank_card_balance_enquiry},/*UI_DISPLAY_PAY_BANK_CARD_BALANCE_ENQUIRY*/			
#endif	
	
  // 10: CAROUSEL		
	{_frame_render_horizontal_carousel_1},               /*UI_DISPLAY_CAROUSEL_1*/	
	{_frame_render_horizontal_carousel_2},               /*UI_DISPLAY_CAROUSEL_2*/	
	{_frame_render_horizontal_carousel_3},               /*UI_DISPLAY_CAROUSEL_3*/	
#endif	
#ifdef _CLINGBAND_2_PAY_MODEL_		
	{_frame_render_horizontal_carousel_4},               /*UI_DISPLAY_CAROUSEL_4*/		
#endif	
}; 


/***********************************************************************************************/
/************************* vertical frame display **********************************************/
/***********************************************************************************************/
const UI_RENDER_CTX vertical_ui_render[] = {
  // 0: Home	
	{_frame_render_vertical_home},                       /*UI_DISPLAY_HOME_CLOCK*/
	
	// 1: System		
	{_frame_render_horizontal_system_restart},           /*UI_DISPLAY_SYSTEM_RESTART*/
	{_frame_render_horizontal_ota},                      /*UI_DISPLAY_SYSTEM_OTA*/
	{_frame_render_horizontal_linking},                  /*UI_DISPLAY_SYSTEM_LINKING*/
	{_frame_render_horizontal_unauthorized},             /*UI_DISPLAY_SYSTEM_UNAUTHORIZED*/
	{_frame_render_horizontal_charging},                 /*UI_DISPLAY_SYSTEM_BATT_POWER*/
	
	// 2: a set of the typical use cases		
	{_frame_render_vertical_steps},                      /*UI_DISPLAY_TRACKER_STEP*/
	{_frame_render_vertical_distance},                   /*UI_DISPLAY_TRACKER_DISTANCE*/
	{_frame_render_vertical_calories},                   /*UI_DISPLAY_TRACKER_CALORIES*/
	{_frame_render_vertical_active_time},                /*UI_DISPLAY_TRACKER_ACTIVE_TIME*/
#ifdef _CLINGBAND_UV_MODEL_		
	{_frame_render_vertical_uv_index},                   /*UI_DISPLAY_TRACKER_UV_IDX*/
#endif
	
	// 3: a set of smart features		
	{_frame_render_vertical_pm2p5},                      /*UI_DISPLAY_SMART_PM2P5*/
  {_frame_render_vertical_weather}, 	                 /*UI_DISPLAY_SMART_WEATHER*/
#ifndef _CLINGBAND_PACE_MODEL_		
	{_frame_render_horizontal_message},                  /*UI_DISPLAY_SMART_MESSAGE*/
	{_frame_render_horizontal_app_notif},                /*UI_DISPLAY_SMART_APP_NOTIF*/
#endif	
	{_frame_render_horizontal_detail_notif},             /*UI_DISPLAY_SMART_DETAIL_NOTIF*/
	{_frame_render_horizontal_incoming_call},            /*UI_DISPLAY_SMART_INCOMING_CALL*/
	{_frame_render_horizontal_incoming_msg},             /*UI_DISPLAY_SMART_INCOMING_MESSAGE*/	
	{_frame_render_vertical_alarm_clock_reminder},       /*UI_DISPLAY_SMART_ALARM_CLOCK_REMINDER*/
#ifndef _CLINGBAND_PACE_MODEL_			
	{_frame_render_vertical_alarm_clock_detail},         /*UI_DISPLAY_SMART_ALARM_CLOCK_DETAIL*/	
#endif	
  {_frame_render_horizontal_idle_alert},               /*UI_DISPLAY_SMART_IDLE_ALERT*/	
	{_frame_render_vertical_heart_rate},                 /*UI_DISPLAY_SMART_HEART_RATE_ALERT*/	
	{_frame_render_vertical_steps},                      /*UI_DISPLAY_SMART_STEP_10K_ALERT*/	           
#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)	
  {_frame_render_horizontal_sos_alert},                /*UI_DISPLAY_SMART_SOS_ALERT*/		
#endif	
	
	// 4: a set of VITAL	
	{_frame_render_vertical_heart_rate},                 /*UI_DISPLAY_VITAL_HEART_RATE*/	
#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)	
  {_frame_render_vertical_skin_temp},                  /*UI_DISPLAY_VITAL_SKIN_TEMP*/	
#endif
	
#ifndef _CLINGBAND_PACE_MODEL_		
	{_frame_render_horizontal_system_restart},           /*UI_DISPLAY_SETTING_VER*/	
  {_frame_render_vertical_stopwatch_start},	           /*UI_DISPLAY_STOPWATCH_START*/	
  {_frame_render_vertical_stopwatch_stop},	           /*UI_DISPLAY_STOPWATCH_STOP*/		
	{_frame_render_horizontal_workout_mode_switch},      /*UI_DISPLAY_WORKOUT_TREADMILL*/	
	{_frame_render_horizontal_workout_mode_switch},      /*UI_DISPLAY_WORKOUT_CYCLING*/	
	{_frame_render_horizontal_workout_mode_switch},      /*UI_DISPLAY_WORKOUT_STAIRS*/	
	{_frame_render_horizontal_workout_mode_switch},      /*UI_DISPLAY_WORKOUT_ELLIPTICAL*/	
	{_frame_render_horizontal_workout_mode_switch},      /*UI_DISPLAY_WORKOUT_ROW*/	
	{_frame_render_horizontal_workout_mode_switch},      /*UI_DISPLAY_WORKOUT_AEROBIC*/	
	{_frame_render_horizontal_workout_mode_switch},      /*UI_DISPLAY_WORKOUT_PILOXING*/		
	{_frame_render_horizontal_workout_mode_switch},      /*UI_DISPLAY_WORKOUT_OTHERS*/		
	{_frame_render_vertical_workout_ready},              /*UI_DISPLAY_WORKOUT_RT_READY*/			
	{_frame_render_vertical_training_time},              /*UI_DISPLAY_WORKOUT_RT_TIME*/			
	{_frame_render_vertical_training_hr},                /*UI_DISPLAY_WORKOUT_RT_HEART_RATE*/			
	{_frame_render_vertical_training_calories},          /*UI_DISPLAY_WORKOUT_RT_CALORIES*/			
	{_frame_render_vertical_training_workout_stop},      /*UI_DISPLAY_WORKOUT_RT_END*/			
#endif	
	
	// 5: Running Statistics	
#ifdef _CLINGBAND_PACE_MODEL_			
	{_frame_render_vertical_running_analysis},           /*UI_DISPLAY_RUNNING_STAT_RUN_ANALYSIS*/			
#endif	
	{_frame_render_vertical_running_distance},           /*UI_DISPLAY_RUNNING_STAT_DISTANCE*/				
	{_frame_render_vertical_running_time},               /*UI_DISPLAY_RUNNING_STAT_TIME*/			
	{_frame_render_vertical_running_pace},               /*UI_DISPLAY_RUNNING_STAT_PACE*/			
	{_frame_render_vertical_running_stride},             /*UI_DISPLAY_RUNNING_STAT_STRIDE*/			
	{_frame_render_vertical_running_cadence},            /*UI_DISPLAY_RUNNING_STAT_CADENCE*/		
	{_frame_render_vertical_running_hr},                 /*UI_DISPLAY_RUNNING_STAT_HEART_RATE*/			
	{_frame_render_vertical_running_calories},           /*UI_DISPLAY_RUNNING_STAT_CALORIES*/				
#ifdef _CLINGBAND_PACE_MODEL_		
	{_frame_render_vertical_running_stop_analysis},      /*UI_DISPLAY_RUNNING_STAT_STOP_ANALYSIS*/		
#endif	
	
	// 6: training Statistics		
	{_frame_render_vertical_training_run_start},         /*UI_DISPLAY_TRAINING_STAT_START*/			
#ifndef _CLINGBAND_PACE_MODEL_		
	{_frame_render_vertical_training_run_or_analysis},   /*UI_DISPLAY_TRAINING_STAT_START_OR_ANALYSIS*/	
#endif	
#ifdef _CLINGBAND_PACE_MODEL_			
	{_frame_render_vertical_connect_gps},                /*UI_DISPLAY_TRAINING_STAT_CONNECT_GPS*/			
	{_frame_render_vertical_connect_gps_timeout},        /*UI_DISPLAY_TRAINING_STAT_CONNECT_GPS_TIMEOUT*/			
	{_frame_render_vertical_connect_gps_fail},           /*UI_DISPLAY_TRAINING_STAT_CONNECT_GPS_FAIL*/	
#endif	
	{_frame_render_vertical_training_ready},             /*UI_DISPLAY_TRAINING_STAT_READY*/			
	{_frame_render_vertical_training_time},              /*UI_DISPLAY_TRAINING_STAT_TIME*/	
	{_frame_render_vertical_training_distance},          /*UI_DISPLAY_TRAINING_STAT_DISTANCE*/		
	{_frame_render_vertical_training_pace},              /*UI_DISPLAY_TRAINING_STAT_PACE*/			
	{_frame_render_vertical_training_hr},                /*UI_DISPLAY_TRAINING_STAT_HEART_RATE*/	
	{_frame_render_vertical_training_run_stop},          /*UI_DISPLAY_TRAINING_STAT_RUN_STOP*/	

	// 7: Cycling Outdoor Statistics		
#ifndef _CLINGBAND_PACE_MODEL_		
	{_frame_render_vertical_cycling_outdoor_start},      /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_START*/		
	{_frame_render_vertical_cycling_outdoor_ready},      /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_READY*/		
	{_frame_render_vertical_training_time},              /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_TIME*/			
  {_frame_render_vertical_cycling_outdoor_distance},   /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_DISTANCE*/			
  {_frame_render_vertical_cycling_outdoor_speed},      /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_SPEED*/	
	{_frame_render_vertical_training_hr},                /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_HEART_RATE*/	
	{_frame_render_vertical_cycling_outdoor_stop},       /*UI_DISPLAY_CYCLING_OUTDOOR_STAT_STOP*/	

	// 8: MUSIC	
#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
	{_frame_render_horizontal_music_play},               /*UI_DISPLAY_MUSIC_PLAY*/	
	{_frame_render_horizontal_music_track},              /*UI_DISPLAY_MUSIC_SONG*/		
	{_frame_render_horizontal_music_volume},             /*UI_DISPLAY_MUSIC_VOLUME*/		
#endif	

		// 9: Pay	
#ifdef _CLINGBAND_2_PAY_MODEL_		
	{_frame_render_horizontal_bus_card_balance_enquiry}, /*UI_DISPLAY_PAY_BUS_CARD_BALANCE_ENQUIRY*/		
	{_frame_render_horizontal_bank_card_balance_enquiry},/*UI_DISPLAY_PAY_BANK_CARD_BALANCE_ENQUIRY*/			
#endif	

  // 10: CAROUSEL	
	{_frame_render_vertical_carousel_1},                 /*UI_DISPLAY_CAROUSEL_1*/	
	{_frame_render_vertical_carousel_2},                 /*UI_DISPLAY_CAROUSEL_2*/	
	{_frame_render_vertical_carousel_3},                 /*UI_DISPLAY_CAROUSEL_3*/	
#endif	
#ifdef _CLINGBAND_2_PAY_MODEL_	
	{_frame_render_vertical_carousel_4},                 /*UI_DISPLAY_CAROUSEL_4*/		
#endif	
}; 


static void _core_frame_display(I8U frame_index, BOOLEAN b_render)
{	
	const UI_RENDER_CTX *ui_render;

#ifdef _CLINGBAND_PACE_MODEL_		
  I32U t_curr = CLK_get_system_time();
	I32U t_delay_offset = 500;
  UI_PACE_PRIVATE_CTX *p = &cling.ui.pace_private;
	
	if (cling.ui.frame_index == UI_DISPLAY_TRAINING_STAT_CONNECT_GPS) {
		// Update touch time stamp.
		cling.ui.touch_time_stamp = t_curr;
	  if (t_curr > (p->conn_gps_t_stamp + 1000 + t_delay_offset)) {
		  if (!BTLE_is_connected()) {
				// Change UI frame page to "Connect GPS fail" page.
			  cling.ui.frame_index = UI_DISPLAY_TRAINING_STAT_CONNECT_GPS_FAIL;
				// Update connect gps time stamp.
			  p->conn_gps_t_stamp = t_curr;
				Y_SPRINTF("[UI] CONN gps frame index to fail page");
		  } 
			
			if (t_curr < (p->conn_gps_t_stamp + 4000 + t_delay_offset)) {
				if (cling.train_stat.app_positon_service_status == POSITION_GPS_STATUS_READY) {
			    // Connect GPS successed, and change UI frame page to "Training state ready" page.
          cling.ui.frame_index = UI_DISPLAY_TRAINING_STAT_READY;				
				  Y_SPRINTF("[UI] CONN gps frame index to training ready page (conn successed)");
				}
			} else {
				// Change UI frame page to "Connect GPS timeout" page.
				cling.ui.frame_index = UI_DISPLAY_TRAINING_STAT_CONNECT_GPS_TIMEOUT;
				// Update connect gps time stamp.
				p->conn_gps_t_stamp = t_curr;			
        Y_SPRINTF("[UI] CONN gps frame index to timeout page");				
			}
	  }	
	}

	if (cling.ui.frame_index == UI_DISPLAY_TRAINING_STAT_CONNECT_GPS_TIMEOUT) {
		// Update touch time stamp.
		cling.ui.touch_time_stamp = t_curr;		
		if (t_curr > p->conn_gps_t_stamp + 1000) {
			// Change UI frame page to "Training state ready" page.
      cling.ui.frame_index = UI_DISPLAY_TRAINING_STAT_READY;
			Y_SPRINTF("[UI] CONN gps frame index to training ready page 1");
		}			
	}
	
	if (cling.ui.frame_index == UI_DISPLAY_TRAINING_STAT_CONNECT_GPS_FAIL) {
		// Update touch time stamp.
		cling.ui.touch_time_stamp = t_curr;		
		if (t_curr > p->conn_gps_t_stamp  + 1000) {
			// Change UI frame page to "Training state ready" page.
      cling.ui.frame_index = UI_DISPLAY_TRAINING_STAT_READY;
			Y_SPRINTF("[UI] CONN gps frame index to training ready page 2");
		}			
	}
#endif	
#ifdef _CLING_PC_SIMULATION_
	if (cling.ui.false_det) {
		Y_SPRINTF("[UI] Wrong!!! - memory overflow");
	}
#endif
	
	// If we see language type out of range, something is very wrong!!
	if (cling.ui.language_type > LANGUAGE_TYPE_TRADITIONAL_CHINESE)	{
	  cling.ui.language_type = LANGUAGE_TYPE_SIMPLIFIED_CHINESE;
	  Y_SPRINTF("[UI] Wrong!!! - language is modified");
	}
	
	if (cling.user_data.profile.metric_distance)
		cling.user_data.profile.metric_distance = 1;

	// Dow should not change
	if (cling.time.local.dow >= 6)
		cling.time.local.dow = 6;
	
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
	
	ui_render[frame_index].frame_render_display();
	
	if (b_render) {
		// Finally, we render the frame
		UI_render_screen();
	}
#ifdef _CLING_PC_SIMULATION_
	Y_SPRINTF("[UI] Core display frame: %d @ %d", frame_index, CLK_get_system_time());
#endif
}
