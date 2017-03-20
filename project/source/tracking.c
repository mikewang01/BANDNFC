/***************************************************************************/
/**
 * File: tracking.c
 * 
 * Description: Activity tracking related processing
 *
 * Created on Jan 3, 2014
 *
 ******************************************************************************/

#include <stdio.h>
#include <string.h>


#include "main.h"
#include "fs_memcfg.h"
#include "btle.h"

#define TRACKING_SPRINTF N_SPRINTF

void TRACKING_enter_low_power_mode()
{
#ifdef _USE_HW_MOTION_DETECTION_
	LIS3DH_inertial_wake_up_init();
#endif
	
	// Upon seeing stationary, we clear the counter
	cling.lps.b_low_power_mode = TRUE;
	cling.lps.ts = CLK_get_system_time();
	cling.lps.int_count = 0;
#ifdef _ENABLE_TOUCH_
	if (UI_is_idle()) {
		// Set Touch IC power mode accordingly
		TOUCH_power_set(TOUCH_POWER_DEEP_SLEEP);
	}
#endif
	TRACKING_SPRINTF("[ACTIVITY] ENTER -- low power mode (stop timer) -- ");
}

void TRACKING_exit_low_power_mode(BOOLEAN b_force)
{
	I32U t_curr = CLK_get_system_time();

	// if user forces low power mode exit, reset accelerometer and pedometer altogether
	if (!b_force) {
		if (!cling.lps.b_low_power_mode) 
			return;
	
#ifdef _USE_HW_MOTION_DETECTION_
		// We probably don't need this process as the PORT sensing H/W bug is fixed.
		// Buf leave it for now ... need time for a verification
		cling.lps.int_count ++;
		
		if (cling.lps.int_count < 2) {
			N_SPRINTF("[ACTIVITY] low power counter: %d", cling.lps.int_count);

			return;
		}
#endif
	}
	
	// Reset motion detection algorithm
	cling.lps.b_low_power_mode = FALSE;
	
	TRACKING_SPRINTF("[ACTIVITY] EXIT -- low power mode (force: %d)-- ", b_force);
#ifdef _ENABLE_TOUCH_
	// Set Touch IC power mode accordingly
	TOUCH_power_set(TOUCH_POWER_HIGH_20MS);
#endif
	// Turn on display if device is in a charging state
	if (BATT_is_charging()) {
		if (!NOTIFIC_is_user_viewing_message()) {
			UI_turn_on_display(UI_STATE_TOUCH_SENSING, 0);
		}
	}
#ifndef _CLING_PC_SIMULATION_

	// Start advertising
	if (BTLE_is_idle()) {
		BTLE_execute_adv(FALSE);
	}
#endif
#ifdef _USE_HW_MOTION_DETECTION_
	LIS3DH_normal_FIFO();
#endif
	TRACKING_initialization();
	
	// No motion for 10 minutes, set pedometer to a low sensitivity state.
	if (t_curr > (cling.lps.ts + 300000)) {
		PEDO_set_step_detection_sensitivity(FALSE);
	}
	
	cling.lps.ts = t_curr;
#ifdef _CLINGBAND_UV_MODEL_
	// Performance UV index measure if device is un-volunteerly waked up
	if (!b_force) {
		cling.uv.b_perform_measure = TRUE;
	}
#endif
}

static I32U steps_time_stamp[8];

static I8U _get_stride_length(BOOLEAN b_running)
{
	// Calculate stride factor based on the pace
	// From a research paper, linear regression is used to model the 
	// relationship between stride length and step frequency (Cadence)
	//    -- Step frequency (in seconds):   1.3 - 2.2  ( steps per second )
	//    -- Stride length (in feet):       1.5 - 2.7 feet, (18 inches - 32.4 inches)
	//
	// According to this relationship, the stride length is about 30 inches if one walks at 2 steps per second
	//
	// Equation:  (p - 1.3)/(2.2-1.3) = (stride - 18) / (32.4 - 18)
	//            stride *= (stride_length / 30)
	//
	// 1 feet = 12 inch
	// 1 inch = 2.54 cm
	//
	// so, the basic equation to get stride length is:
	//
	// s = 40.64*p-7.112
	//
	// Calibrated with GPS, x1.08
	//
	// s = 43.89*p - 7.68
	//
	// where s is the stride length, and p is the pace (steps per second)
	double pace,stride, ratio, ref_rate;
	I8U i;
	for (i = 1; i < 8; i++) {
		steps_time_stamp[i-1] = steps_time_stamp[i];
	}
	steps_time_stamp[7] = CLK_get_system_time();
	
	// Calculate pace
	if (b_running) {
		// Running pace is calculated based on real-time steps
		if (steps_time_stamp[0] > 0) {
			// Average time for past 7 steps
			pace = 7000.0;
			pace /= (double)(steps_time_stamp[7] - steps_time_stamp[0]);
			TRACKING_SPRINTF("[TRACKING] running pace: %d, %d, %d", (I8U)(pace*10), steps_time_stamp[7], steps_time_stamp[0]);
			
			if (pace > 3.8) 
				pace = 3.8;
			if (pace < 2)
				pace = 2;
		} else {
			pace = 2;
			TRACKING_SPRINTF("[TRACKING] running fixed pace: %d", (I8U)(pace*10));
		}
		// Get reference rate
		ref_rate = cling.user_data.profile.running_rate;
		ref_rate /= 60;
		
		stride = cling.user_data.profile.stride_running_in_cm;
		if (cling.activity.b_workout_active) {
			if (cling.activity.workout_type == WORKOUT_TREADMILL_INDOOR) {
				stride = cling.user_data.profile.stride_treadmill_in_cm;
			} 
		}
				
		if (pace > ref_rate) {
			stride += (pace-ref_rate)*37.6;
		} else {
			stride -= (ref_rate-pace)*37.6;
		}
		TRACKING_SPRINTF("[TRACKING] running stride: %f, pace: %f", stride, pace);
	} else {
		// Walking pace is calculated based on the steps we took in last minute
		pace = (double)(cling.run_stat.walk_per_60_second+cling.run_stat.run_per_60_second);
		pace /= 60.0;
		if (pace > 3.5) 
			pace = 3.5;
		if (pace < 1.88)
			pace = 1.88;
		TRACKING_SPRINTF("[TRACKING] walking  pace: %d", (I8U)(pace*10));
		stride = 43.89*pace - 7.68;
		// Get user defined stride length and perform a calibration
		ratio = cling.user_data.profile.stride_in_cm;
	
		ratio /= 75; // Standard 75 cm (30 inches) per step for walking
		if (ratio > 2)
			ratio = 2;
		else if (ratio < 0.5)
			ratio = 0.5;
		TRACKING_SPRINTF("[TRACKING] walking stride: %f, ratio: %d", stride, (I16U)(ratio*100));
		// Normalization
		stride *= ratio;
	}
	
	
	N_SPRINTF("[TRACKING] adjusted stride: %f", stride);

	// Do normalization to 16 m
	stride *= 16;
	stride /= 100;
	stride += 0.5;
	
	return (I8U)stride;
}

void TRACKING_algorithms_proc(ACCELEROMETER_3D A)
{
	I16U pdm_stat;
	I32U curr_time = CLK_get_system_time();
	I8U act_motion;
	I8U distance_per_step;
//	REALTIME_CTX rt;
	
	// Go skip algorithms processing if device is not authorized
	if (!LINK_is_authorized())
		return;

	N_SPRINTF("[ACT] %08x, %08x, %08x, %04x, %04x, %04x,", A.x, A.y, A.z, xyz->x, xyz->y, xyz->z);
	N_SPRINTF("[ACT] %d, %d, %d, %d, %d, %d,", A.x, A.y, A.z, xyz->x, xyz->y, xyz->z);

	// Enable the interactive mode for now
	pdm_stat = PEDO_main(A);
	
	if (!pdm_stat) {

		return;
	}
	
	// If any change, or meaningful activity detected from pedometer
	act_motion = PEDO_get_motion_type();
	
	N_SPRINTF("[TRACKING] motion type: %d", act_motion);

	// Exit low power mode if any type of activity detected after a low power stationary
	if (act_motion == MOTION_STATIONARY) {
		TRACKING_enter_low_power_mode();
	} else if (act_motion == MOTION_UNKNOWN) {
		if (PDM_STEP_DETECTED == (pdm_stat & PDM_STEP_DETECTED)) {
			N_SPRINTF("[ACTIVITY] --- Motion unknown ---");
		}
	} else if (PDM_STEP_DETECTED == (pdm_stat & PDM_STEP_DETECTED)) {
		// For any position motion detected, we need to remeasure heart rate	
		if (act_motion == MOTION_WALKING) {
			//
			// Simple conversion:
			//
			// 1 Mile = 5280 feet
			// 1 feet = 12 inches   ==> 1 Mile = 63360;
			//
			// The distance (in inches) is normalized by 100, i.e.,
			// mileage = distance / (63360 * 100)
			// 
			cling.activity.day.walking ++;
			distance_per_step = _get_stride_length(FALSE); // 0.75 meters
			cling.activity.day.distance += distance_per_step;
			if (cling.activity.b_workout_active) {
				cling.run_stat.last_10sec_distance += distance_per_step;
			}
			cling.activity.step_detect_t_sec = cling.time.system_clock_in_sec;
			BATT_charging_update_steps(1);
			if (cling.user_data.idle_step_countdown > 0) {
				cling.user_data.idle_step_countdown --;
			} else {
				cling.user_data.idle_state = IDLE_ALERT_STATE_IDLE;
				N_SPRINTF("[USER] reset idle alert due to walking");
			}
		} else if (act_motion == MOTION_RUNNING) {
			cling.activity.day.running ++;
			
			if (cling.activity.b_workout_active) {
		
				if (UI_is_idle() && cling.user_data.b_running_alwayson) {
					UI_turn_on_display(UI_STATE_TOUCH_SENSING, 40);
				}
				// Turn on PPG
				if (cling.hr.state == PPG_STAT_DUTY_OFF) {
					PPG_closing_to_skin_detect_init();
				}
				
				// Update running sec
				{
					I32U t_diff;
					t_diff = CLK_get_system_time() - cling.train_stat.time_start_in_ms;
					t_diff /= 1000;
					t_diff %= 60;
					cling.run_stat.time_sec = t_diff;
				}
			}
			
			distance_per_step = _get_stride_length(TRUE); // 1.41 meters
			cling.activity.day.distance += distance_per_step;
			if (cling.activity.b_workout_active) {
				cling.run_stat.last_10sec_distance += distance_per_step;
			}
			cling.activity.step_detect_t_sec = cling.time.system_clock_in_sec;
			BATT_charging_update_steps(1);
			if (cling.user_data.idle_step_countdown > 0) {
				cling.user_data.idle_step_countdown --;
			} else {
				cling.user_data.idle_state = IDLE_ALERT_STATE_IDLE;
				N_SPRINTF("[USER] reset idle alert due to running");
			}
		}
	}
}

void TRACKING_initialization()
{
	N_SPRINTF("[ACTIVITY] initialize pedometer! ");
	
	PEDO_create();
}

static void _day_stat_reset()
{
	// Make sure all the accumulator gets updated first
	cling.activity.workout_Calories_acc += cling.activity.day.calories - cling.activity.workout_Calories_start;
	cling.activity.workout_Calories_start = 0;
	
	// Reset activity count for the Day rollover
	cling.activity.day.walking = 0;
	cling.activity.day.running = 0;
	cling.activity.day.calories = 0;
	cling.activity.day.distance = 0;
	cling.activity.day.active_time = 0;

	// Reset stored activity data buffer that is used for minute-based activity calculation
	cling.activity.day_stored.walking = 0;
	cling.activity.day_stored.running = 0;
	cling.activity.day_stored.distance = 0;
	cling.activity.day_stored.calories = 0;
	cling.activity.day_stored.active_time = 0;
	
	// Reset running statistics as well
	cling.run_stat.calories = 0;
	cling.run_stat.distance = 0;
	cling.run_stat.steps = 0;
	cling.run_stat.time_min = 0;
	cling.run_stat.time_sec = 0;
	cling.run_stat.accu_heart_rate = 0;
}

void TRACKING_total_data_load_file()
{
	CLING_FILE f;

	if (TRUE == FILE_if_exists((I8U *)"activity_init_total.skd")) {
		// file exists, so check
		f.fc = FILE_fopen((I8U *)"activity_init_total.skd", FILE_IO_READ);
		// size is right.  read the file into the buffer.
		FILE_fread(f.fc, (I8U *)(&cling.activity.day), 28);
		FILE_fclose(f.fc);

		FILE_delete((I8U *)"activity_init_total.skd");
	} else {
			// Reset daily statistics
			_day_stat_reset();
	}
}

static void _minute_data_flush_file(I32U flash_offset)
{
	CLING_FILE f;
	I32U dw_buf[4];
	I8U name_buf[128];
	I8U *pbuf = (I8U *)dw_buf;
	I32U epoch_head;
	BOOLEAN b_valid;
	I16U len = 0;
	I32U pos, epoch_valid;
	MINUTE_TRACKING_CTX *pminute = (MINUTE_TRACKING_CTX *)dw_buf;

	// Check if the whole 4 KB block has been erased
	pos = flash_offset;
	FLASH_Read_App(pos, pbuf, 16);
	
	if (pminute->epoch == 0xffffffff) {
		N_SPRINTF("[TRACKING] empty sector, no need to erase");
		return;
	}
	// If the space is used, check if there is any valid entry
	b_valid = FALSE;

	// check if there is any valid entry
	len = FLASH_ERASE_BLK_SIZE;
	while (len > 0) {
		FLASH_Read_App(pos, pbuf, 16);
		epoch_head = pminute->epoch & 0x80000000;
		pos += 16;
		len -= 16;
		if (epoch_head) {
			b_valid = TRUE;
			break;
		}
	}
	
	// If all the entries have been uploaded, go ahead to erase this block
	if (b_valid) {
			
		epoch_valid = pminute->epoch & 0x7fffffff;
		
		if (epoch_valid == 0x7fffffff) {
			// Erase this block and return
			FLASH_erase_App(flash_offset);
			return;
		}

		// generate file name
		sprintf((char *)name_buf, "epoch_%d.skd", epoch_valid);

		// Open this file for write
		f.type = FILE_TYPE_ACTIVITY;
		f.fc = FILE_fopen(name_buf, FILE_IO_WRITE);
							
		if (f.fc) {
			
			// Write all the valid entry to the file
		
			// The first entry
			FILE_fwrite(f.fc, pbuf, 16);

			while (len > 0) {
				FLASH_Read_App(pos, pbuf, 16);
				pos += 16;
				len -= 16;
				FILE_fwrite(f.fc, pbuf, 16);
			}
		}

		FILE_fclose(f.fc);
	}
	
	N_SPRINTF("[TRACKING] file %s generated, erasing at %d", name_buf, flash_offset);

	// Finally, we should erase this block
	FLASH_erase_App(flash_offset);
}

static I8U _get_calories_per_minute(I8U type)
{
	I8U cal_per_minute;
	switch (type) {
		case WORKOUT_TREADMILL_INDOOR:
			cal_per_minute = 108;
			break;
		case WORKOUT_RUN_OUTDOOR:
			cal_per_minute = 208;
			break;
		case WORKOUT_CYCLING_INDOOR:
			cal_per_minute = 158;
			break;
		case WORKOUT_ELLIPTICAL_INDOOR:
			cal_per_minute = 118;
			break;
		case WORKOUT_STAIRS_INDOOR:
			cal_per_minute = 121;
			break;
		case WORKOUT_AEROBIC:
			cal_per_minute = 168;
			break;
		case WORKOUT_ROWING:
			cal_per_minute = 148;
			break;
		case WORKOUT_PILOXING:
			cal_per_minute = 218;
			break;
		case WORKOUT_OTHER: 
			cal_per_minute = 110;
			break;
		default:
			cal_per_minute = 110;
	}
	
	return cal_per_minute;
}

static void	_get_activity_diff(MINUTE_DELTA_TRACKING_CTX *diff, BOOLEAN b_minute_update)
{
	TRACKING_CTX *a = &cling.activity;
	I8U calories_diff;

#ifdef _ACTIVITY_SIM_BASED_ON_EPOCH_
	I16U denom_stats;
	
		diff->walking = (I8U)SIM_get_current_activity(TRACKING_WALK);
		diff->running = (I8U)SIM_get_current_activity(TRACKING_RUN);
		diff->distance = (I8U)SIM_get_current_activity(TRACKING_DISTANCE);
		diff->calories = (I8U)SIM_get_current_activity(TRACKING_CALORIES);
		diff->sleep_state = (I8U)SIM_get_current_activity(TRACKING_SLEEP);
		diff->activity_count = (I16U)SIM_get_current_activity(TRACKING_ACTIVITY);
		
		if (b_minute_update) {
			// Update stored total
			a->day.walking += diff->walking;
			a->day.running += diff->running;
			denom_stats = diff->distance;
			denom_stats <<= 4;
			a->day.distance += denom_stats;
			denom_stats = diff->calories;
			denom_stats <<= 4;
			a->day.calories += denom_stats;
			if ((diff->sleep_state == SLP_STAT_LIGHT) ||
				  (diff->sleep_state == SLP_STAT_SOUND) ||
			    (diff->sleep_state == SLP_STAT_REM))
			{
				diff->activity_count = 0;
			} else {
				diff->activity_count = 50;
			}
		}
		return;
#endif
		// Update the activity minute granularity
		diff->walking = a->day.walking - a->day_stored.walking;
		diff->running = a->day.running - a->day_stored.running;
		// in 2 meters, normalized by 32
		// Theoretical maximum distance in 1 minute is 255*2 = 510 meters
		// Pace is about 1'57"
		//
		diff->distance = (a->day.distance - a->day_stored.distance) >> 5; 
		diff->sleep_state = cling.sleep.state;
		
		if ((diff->walking+diff->running) >= 40) {
			a->day.active_time++;
		}

		N_SPRINTF("[tracking] diff: %d, %d, %d", diff->walking, a->day.walking, a->day_stored.walking);

		if (diff->running > 0) {
			if (!cling.activity.b_workout_active) {
				if (diff->running > 50) {
					calories_diff = 208; // 12.5 for running
				} else if (diff->walking > diff->running) {
					calories_diff = 158;
				} else {
					calories_diff = 108;
				}
			} else {
				calories_diff = _get_calories_per_minute(cling.activity.workout_type);
				if (diff->walking+diff->running < 30) {
					calories_diff >>= 1;
				}
			}
		}
		else if (diff->walking > 0) {
			if (!cling.activity.b_workout_active) {
				if (diff->walking > 50) {
					calories_diff = 108;  // 6.2 for walking
				} else if (diff->walking > 20) {
					calories_diff = 58;
				} else {
					calories_diff = 33;
				}
			} else {
				calories_diff = _get_calories_per_minute(cling.activity.workout_type);
				if (diff->walking+diff->running < 30) {
					calories_diff >>= 1;
				}
			}
		}
		else {
			calories_diff = 18; // 1.1 for rest
		}

		if (b_minute_update) {
			a->day.calories += calories_diff;

			if ((diff->sleep_state == SLP_STAT_LIGHT) ||
				(diff->sleep_state == SLP_STAT_SOUND) ||
				(diff->sleep_state == SLP_STAT_REM))
			{
				a->sleep_by_noon += 60;
			}
  		diff->activity_count = cling.sleep.m_activity_per_min;
		}
		diff->calories = (a->day.calories - a->day_stored.calories) >> 4; // calories, normalized by 16
		
		N_SPRINTF("+++ sleep_state: %d", cling.sleep.state);
}

static void	_get_vital_minute(MINUTE_VITAL_CTX *vital)
{
	I32U t_curr_sec;
	
	t_curr_sec = cling.time.system_clock_in_sec - cling.activity.step_detect_t_sec;

		N_SPRINTF("[TRACKING] touch pads: %d, time: %d", vital->skin_touch_pads, touch_time);
		
		vital->skin_temperature = cling.therm.current_temperature;
		vital->skin_touch_pads = 1;

	if (!cling.hr.b_closing_to_skin) {
		// if sensor is detached from skin, set both vital signals to 0
			vital->heart_rate = 0;
			vital->skin_temperature = 0;
			return;
	} 
	
	vital->heart_rate = cling.hr.current_rate;
	
	if (t_curr_sec > 300) {
		// If no steps for over 5 minutes (300 seconds), filter out the heart rate that is greater than 95
		if (vital->heart_rate >= 95) {
			vital->heart_rate = 95 - (cling.hr.current_rate >> 3);

			N_SPRINTF("[TRACKING] heart rate: %d, original: %d", vital->heart_rate, cling.hr.current_rate);

		}
	}
}

void TRACKING_get_whole_minute_delta(MINUTE_TRACKING_CTX *pminute, MINUTE_DELTA_TRACKING_CTX *diff)
{
	MINUTE_VITAL_CTX vital;
	I32U adj;
	I8U hr_diff;
		
	// Get activity difference
	_get_activity_diff(diff, TRUE);
	
	// Get vital signal
	_get_vital_minute(&vital);
	
	// Get activity minute granularity
	// Note here, we have minute offset to UTC time.
	pminute->epoch = RTC_get_last_minute_epoch(); // Backwards 60 seconds as all the activties happen in the past minute
	pminute->skin_temperature = vital.skin_temperature;
	pminute->walking = diff->walking;
	pminute->running = diff->running;
	pminute->calories = diff->calories;
	pminute->distance = diff->distance; // Note here, distance unit is per 2 meters (/2m)
	pminute->sleep_state = diff->sleep_state;
	pminute->heart_rate = vital.heart_rate;
	pminute->skin_touch_pads = vital.skin_touch_pads;
	pminute->activity_count = diff->activity_count;
	
	// Set a invalid activity count
	if (BATT_is_charging()) {
		pminute->activity_count = 199; // Set a large number to indicate device is currently charging
	}
	
	// Get the maximum UV in past minute as part of minute data
#ifdef _CLINGBAND_UV_MODEL_
	{
		I8U uv_integer;
		uv_integer = (UV_get_max_index_per_minute()+5)/10;
		pminute->uv_and_activity_type = uv_integer & 0x0f;
	}
#else
	pminute->uv_and_activity_type = 0;
#endif

	if (cling.activity.b_workout_active) {
		pminute->uv_and_activity_type |= cling.activity.workout_type<<4;
	}
	
	N_SPRINTF("MINUTE UPDATE: %08x, %04x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %04x, %02x",
		pminute->epoch, pminute->skin_temperature, pminute->walking, pminute->running, pminute->calories, 
		pminute->distance, pminute->sleep_state, pminute->heart_rate, pminute->skin_touch_pads, pminute->activity_count,
		pminute->uv_and_activity_type);
	N_SPRINTF("MINUTE UV ACT UPDATE: %d, %d, %d, ", pminute->uv_and_activity_type, uv_integer, cling.activity.workout_type);

	// Check whether user gets out of IDLE state
	if ((pminute->running + pminute->walking) > 60) {
		cling.user_data.idle_state = IDLE_ALERT_STATE_IDLE;
	}
	
	adj = 0;
	// Adjust heart rate in case of intense activity
	if (cling.hr.b_closing_to_skin) {	
			if (pminute->running > 150) {
				adj = pminute->running-150;
				if (adj > 45) {
					pminute->heart_rate = 145+(adj>>1);
				} else {
					pminute->heart_rate = 129+adj;
				}
			} else if (pminute->running > 80) {
				adj = pminute->running - 80;
				if (adj > 45) adj >>= 1;
				pminute->heart_rate = 115+adj;
			} else if ((pminute->running + pminute->walking) > 120) {
				adj = pminute->running + pminute->walking - 120;
				if (adj > 35) adj >>= 1;
				pminute->heart_rate = 108+adj;
			} else if ((pminute->running + pminute->walking) > 100) {
				pminute->heart_rate = 102+(pminute->running + pminute->walking - 100);
			} else if ((pminute->running + pminute->walking) > 80) {
				pminute->heart_rate = 96+(pminute->running + pminute->walking - 80);
			} else if ((pminute->running + pminute->walking) > 60) {
				pminute->heart_rate = 83+(pminute->running + pminute->walking - 60);
			}
	}
	
	if ((pminute->running + pminute->walking) > 4) {
		SLEEP_wake_up_by_force(TRUE);
	}
	
	// For compatibility, we set activity count flag
	pminute->activity_count |= 0x8000;
	
	// Refresh minute-based HR
	if (pminute->heart_rate > 40) {		
		if (pminute->heart_rate > cling.hr.minute_rate) {
			cling.hr.minute_rate = pminute->heart_rate;
		} else {
			hr_diff = cling.hr.minute_rate - pminute->heart_rate;
			
			if (hr_diff > 30) {
				cling.hr.minute_rate = pminute->heart_rate + (hr_diff>>1);
			} else {
				cling.hr.minute_rate = pminute->heart_rate;
			}
		}
	}
	
	N_SPRINTF("MINUTE UPDATE-2: %08x, %04x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %04x, %02x",
		pminute->epoch, pminute->skin_temperature, pminute->walking, pminute->running, pminute->calories, 
		pminute->distance, pminute->sleep_state, pminute->heart_rate, pminute->skin_touch_pads, pminute->activity_count,
		pminute->uv_and_activity_type);
}

/*----------------------------------------------------------------------------------
*  Function:	_check_training_pace_and_hr_alert(I16U minute_distance)
*
*  Description: Check pace and hate rate when user in running mode.
*
*----------------------------------------------------------------------------------*/
void _check_training_pace_and_hr_alert(I16U minute_distance)
{
	UI_ANIMATION_CTX *u = &cling.ui;	
	I16U pace_range_up = 0;
	I8U hr_range_down = 0;
	BOOLEAN b_pace_range_alert = FALSE;
	BOOLEAN b_hr_range_alert = FALSE;
	I32U input_pace_min, hr, hr_perc;
	
  if (!cling.activity.b_workout_active)
		return;
	
	if (cling.user_data.profile.training_alert & 0x80) {
		if (minute_distance) {
			// Get Pace in minute
			input_pace_min = 100000;
			input_pace_min /= minute_distance;
			input_pace_min /= 100;
		} else {
			input_pace_min = 0;
		}

	  pace_range_up = (cling.user_data.profile.training_alert & 0x7f);

		if (input_pace_min > 2)
			b_pace_range_alert = TRUE;
		else
			b_pace_range_alert = FALSE;
	} else {
		hr = PPG_minute_hr_calibrate();

		hr_perc = (hr * 100);
		hr_perc /= (220-cling.user_data.profile.age);	
		if (hr_perc > 98)
			hr_perc = 98;
		
	  hr_range_down = cling.user_data.profile.max_hr_alert;
    b_hr_range_alert = TRUE;
	}	

	if (b_pace_range_alert) {
		if (input_pace_min < pace_range_up) {
			if (!u->b_training_alert) {
				if (cling.time.system_clock_in_sec > cling.hr.alert_ts + 300) {
					cling.hr.alert_ts = cling.time.system_clock_in_sec;

					u->b_training_alert = TRUE;
					NOTIFIC_start_notifying(NOTIFICATION_TYPE_RUNNING_PACE_ALERT, 0);			
				}
			}
		} else {
			u->b_training_alert = FALSE;
		}
  } 
	
	if (b_hr_range_alert) {
		if (hr_perc >= hr_range_down) {
			if (!u->b_training_alert) {
				if (cling.time.system_clock_in_sec > cling.hr.alert_ts + 300) {
					cling.hr.alert_ts = cling.time.system_clock_in_sec;
					
					u->b_training_alert = TRUE;
					NOTIFIC_start_notifying(NOTIFICATION_TYPE_RUNNING_HR_ALERT, 0);					
				}
			}
		} else {
			u->b_training_alert = FALSE;
		}		
	}
}

void _update_minute_base(MINUTE_TRACKING_CTX *minute)
{
	TRACKING_CTX *a = &cling.activity;
	I32U denormalized_stat;
	I16U denormalized_distance;
	RUNNING_TRACK_CTX *r = &cling.run_stat;
	TRAINING_TRACK_CTX *t = &cling.train_stat;

	// Update stored total
	a->day_stored.walking += minute->walking;
	a->day_stored.running += minute->running;
	denormalized_stat = minute->distance;
	denormalized_stat <<= 5;
	a->day_stored.distance += denormalized_stat;
	denormalized_stat = minute->calories;
	denormalized_stat <<= 4;
	a->day_stored.calories += denormalized_stat;

	// Distance in meter
	denormalized_distance = minute->distance;
	denormalized_distance <<= 1;
	
	// Update running statistics
	if (cling.activity.b_workout_active) {
		// Update training stats
		t->distance += denormalized_distance;
		t->calories += minute->calories;
			
		if (minute->running > 99) {
			r->time_min ++;
			r->accu_heart_rate += minute->heart_rate;
			r->time_sec = 0;
			r->calories += minute->calories;
			r->steps += minute->walking + minute->running;
			r->distance += denormalized_distance;
			// Turn on PPG
			if (cling.hr.state == PPG_STAT_DUTY_OFF) {
				PPG_closing_to_skin_detect_init();
			}
		}
	}
	
	// Update speed
	r->walk_per_60_second = minute->walking;
	r->run_per_60_second = minute->running;
	r->last_minute_distance = denormalized_distance;;
	
	// If pace is falling ... start alerting
	_check_training_pace_and_hr_alert(r->last_minute_distance);
}

static void _logging_per_minute()
{
	MINUTE_DELTA_TRACKING_CTX diff;
	TRACKING_CTX *a = &cling.activity;
	I32U tracking_minute[4];
	MINUTE_TRACKING_CTX minute;
	I16U max_heart_rate;

	// Backup critical system information
	SYSTEM_backup_critical();

	// Update the activity minute granularity
	TRACKING_get_whole_minute_delta(&minute, &diff);

	max_heart_rate = 165;
	if (cling.user_data.profile.age > 18) {
		if (cling.user_data.profile.sex == SEX_MALE) {
			max_heart_rate = 220 - cling.user_data.profile.age;
		} else {
			max_heart_rate = 226 - cling.user_data.profile.age;
		}
		max_heart_rate *= cling.user_data.profile.max_hr_alert;
		max_heart_rate /= 100;
		
		TRACKING_SPRINTF("[TRACKING] Max heart rate: %d", max_heart_rate);
	}

	if (!cling.activity.b_workout_active) {
		// alert user if heart rate is approaching the limit
		if ((minute.heart_rate > max_heart_rate) && (diff.running > 172)) {
			if (cling.time.system_clock_in_sec > cling.hr.alert_ts + 300) {
				cling.hr.alert_ts = cling.time.system_clock_in_sec;
				cling.hr.b_closing_to_skin = TRUE;
				cling.hr.heart_rate_ready = TRUE;
				N_SPRINTF("[TRACKING] HR alerting ...");
				NOTIFIC_start_notifying(NOTIFICATION_TYPE_NORMAL_HR_ALERT, 0);
			}
		}
	}
	
	// alert user if steps go beyond 10 K steps
	if ((cling.activity.day.walking + cling.activity.day.running) >= 10000) {
		if ((cling.activity.day_stored.walking+cling.activity.day_stored.running) < 10000) {
			NOTIFIC_start_notifying(NOTIFICATION_TYPE_10K_STEP, 0);
		}
	}
	
	N_SPRINTF("[LOGGING1] %08x %08x %08x %08x", tracking_minute[0], tracking_minute[1], tracking_minute[2], tracking_minute[3]);

	// sleep minute process
	SLEEP_minute_proc();
	
	N_SPRINTF("[LOGGING2] %08x %08x %08x %08x", tracking_minute[0], tracking_minute[1], tracking_minute[2], tracking_minute[3]);
	N_SPRINTF("-- logging ---: %d, %d", a->tracking_flash_offset, minute.epoch);

	minute.epoch |= 0x80000000;  // Set un-read flag
	memcpy(tracking_minute, &minute, 16);

	// Update the differential of activities
	_update_minute_base(&minute);

	// Put it into NOR scrach pad
	FLASH_Write_App(a->tracking_flash_offset + SYSTEM_TRACKING_SPACE_START, (I8U *)tracking_minute, 16);

	// Update the buffer length
	a->tracking_flash_offset += 16;
	
	// Erase next block we now just switch to a new space
	if ((a->tracking_flash_offset & 0x0fff) == 0) {
		a->b_pending_erase = TRUE;
		a->flash_block_1st_to_erase = a->tracking_flash_offset;
		a->flash_block_2nd_to_erase = a->tracking_flash_offset + FLASH_ERASE_BLK_SIZE;
		N_SPRINTF("[TRACKING] need to erase next block - %d", a->tracking_flash_offset);
	}

	// Make sure next storing block does not go beyond boundary
	if (a->tracking_flash_offset >= SYSTEM_TRACKING_SPACE_SIZE) {
		a->tracking_flash_offset = 0;
		N_SPRINTF("-- tracking offset (wrapped around) ---: %d", a->tracking_flash_offset);
	}
	if (a->flash_block_1st_to_erase >= SYSTEM_TRACKING_SPACE_SIZE) {
		a->flash_block_1st_to_erase = 0;
	}

	// Make ure next erase block does not go beyond boundary, if so, substruct its offset.
	if (a->flash_block_2nd_to_erase >= SYSTEM_TRACKING_SPACE_SIZE) {
		a->flash_block_2nd_to_erase -= SYSTEM_TRACKING_SPACE_SIZE;
	}
	
	// do not flush any data during OTA
	if (OTA_if_enabled()) return;

	if (!a->b_pending_erase) return;

	if (SYSTEM_get_mutex(MUTEX_MCU_LOCK_VALUE)) {
		// Make sure next 2 blocks are empty (The 1st block erasure is redundent, but it is necessary, in the case of 
		// the erasure was interrupted by OTA or Mutex), we need to make sure block is fully erased before writing 
		// data to it
		//
		_minute_data_flush_file(a->flash_block_1st_to_erase + SYSTEM_TRACKING_SPACE_START);
		// 
		// Flush to a file
		_minute_data_flush_file(a->flash_block_2nd_to_erase + SYSTEM_TRACKING_SPACE_START);
		a->b_pending_erase = FALSE;
	
		SYSTEM_release_mutex(MUTEX_MCU_LOCK_VALUE);
		N_SPRINTF("[TRACKING] Erase two blocks: %d, %d", a->flash_block_1st_to_erase, a->flash_block_2nd_to_erase);
	}

}


static void _logging_midnight_local()
{
	I32U dw_buf[16];
	I32U buf2[16];
	I8U *pbuf = (I8U *)buf2;
	I16U pos=0;
	SYSTIME_CTX previous_day;

	N_SPRINTF("[ACTIVITY] !!!! midnight local flush !!!!");

	RTC_get_delta_clock_backward(&previous_day, 1);
	
	// Get the time (format: yyyymmdd)
	dw_buf[0] = previous_day.year;
	dw_buf[0] <<= 8;
	dw_buf[0] |= previous_day.month;
	dw_buf[0] <<= 8;
	dw_buf[0] |= previous_day.day;
	
	// Get other information
	dw_buf[1] = cling.activity.sleep_stored_by_noon;
	dw_buf[2] = cling.activity.day_stored.walking+cling.activity.day_stored.running;
	dw_buf[3] = cling.activity.day_stored.distance>>4;
	dw_buf[4] = cling.activity.day_stored.calories>>4;
	dw_buf[5] = cling.activity.day_stored.active_time;

	// Put it into DAYSTAT space
	pbuf = (I8U *)buf2;
	for (pos = 0; pos < SYSTEM_DAYSTAT_SPACE_SIZE; pos += TRACKING_DAY_STAT_MAX) {
		FLASH_Read_App(SYSTEM_DAYSTAT_SPACE_START+pos, pbuf, TRACKING_DAY_STAT_SIZE);
		
		if (pbuf[0] == 0xff) {
			FLASH_Write_App(SYSTEM_DAYSTAT_SPACE_START+pos, (I8U *)dw_buf, TRACKING_DAY_STAT_SIZE);
			break;
		} else if (dw_buf[0] == buf2[0]) {
			// If everything has already been backed up, igonre the storing request.
			break;
		}
	}

	// if we have filled up the space, clear it up
	if (pos >= SYSTEM_DAYSTAT_SPACE_SIZE) {
		// 1. clean up the scratch pad
		FLASH_erase_App(SYSTEM_SCRATCH_SPACE_START);

		// Add delay before write data (Erasure latency: 50 ms)
		BASE_delay_msec(50);	
		
		// 2. copy the last 32 entries to the scratch pad
		for (pos = 0; pos < (SYSTEM_DAYSTAT_SPACE_SIZE >> 1); pos += TRACKING_DAY_STAT_MAX) {
			FLASH_Read_App(SYSTEM_DAYSTAT_SPACE_START+pos+(SYSTEM_DAYSTAT_SPACE_SIZE>>1), pbuf, TRACKING_DAY_STAT_SIZE);
			FLASH_Write_App(SYSTEM_SCRATCH_SPACE_START + pos, pbuf, TRACKING_DAY_STAT_SIZE);
		}
		
		// 3. clean up the DAYSTAT space
		FLASH_erase_App(SYSTEM_DAYSTAT_SPACE_START);

		// Add delay before write data (Erasure latency: 50 ms)
		BASE_delay_msec(50);	
		
		// 4. copy back the last 32 entries to DAYSTAT space
		for (pos = 0; pos < (SYSTEM_DAYSTAT_SPACE_SIZE >> 1); pos += TRACKING_DAY_STAT_MAX) {
			FLASH_Read_App(SYSTEM_SCRATCH_SPACE_START+pos, pbuf, TRACKING_DAY_STAT_SIZE);
			FLASH_Write_App(SYSTEM_DAYSTAT_SPACE_START+pos, pbuf, TRACKING_DAY_STAT_SIZE);
		}
		
		// 5. write today's data
		FLASH_Write_App(SYSTEM_DAYSTAT_SPACE_START+pos, (I8U *)dw_buf, TRACKING_DAY_STAT_SIZE);
	}
	
	// Now, reset everything, and start a new day
	_day_stat_reset();	
}

static void _update_running_pace()
{
	I32U t_curr, t_diff, min, sec;
//	I32U t_backup;
	I32U pace, sum_d, sum_t;
	I16U d, t;
	I8U i;
	
	t_curr = CLK_get_system_time();
	t_diff = t_curr - cling.run_stat.pace_calc_ts;
	
	// Calculate pace on a basis of 10 seconds
	if (t_diff < 10000)
		return;
	
	// Take the average as a low pass filter
	if (cling.run_stat.pace_buf_idx >= PACE_BUF_LENGTH) {
		cling.run_stat.pace_buf_idx = 0;
	}
	if (cling.run_stat.last_10sec_distance) {
		cling.run_stat.last_d_buf[cling.run_stat.pace_buf_idx] = cling.run_stat.last_10sec_distance>>4;
		cling.run_stat.last_t_buf[cling.run_stat.pace_buf_idx] = t_diff;
	} else {
		cling.run_stat.last_d_buf[cling.run_stat.pace_buf_idx] = 0xffff;
		cling.run_stat.last_t_buf[cling.run_stat.pace_buf_idx] = 0xffff;
	}
	cling.run_stat.pace_buf_idx ++;
	sum_d = 0;
	sum_t = 0;
	for (i = 0; i < PACE_BUF_LENGTH; i++) {
		d = cling.run_stat.last_d_buf[i];
		t = cling.run_stat.last_t_buf[i];
		if (d && t) {
			if ((d != 0xffff) && (t != 0xffff)) {
				sum_d += d;
				sum_t += t;
			}
		}
	}
	
	pace = sum_t * 100;
	pace /= 60;
	pace /= sum_d;
	
	min = pace / 100;
	sec = pace - min * 100;
	sec *= 60;
	sec /= 100;
	
	// The slowest pace is 24'00"
	if (min >= 24) {
		min = 0;
		sec = 0;
	}
	
	cling.run_stat.last_10sec_pace_min = min;
	cling.run_stat.last_10sec_pace_sec = sec;
	
	TRACKING_SPRINTF("TRACKING: pace %d'%d\"(%d, %d)", min, sec, cling.run_stat.last_10sec_pace_min, cling.run_stat.last_10sec_pace_sec);
	
	cling.run_stat.pace_calc_ts = t_curr;
	cling.run_stat.last_10sec_distance = 0;
}

void TRACKING_data_logging()
{
	// Activity update only for a device that is authenticated
	if (!LINK_is_authorized()) {
		cling.time.local_minute_updated = FALSE;
		cling.time.local_day_updated = FALSE;
		cling.time.local_noon_updated = FALSE;
		return;
	}
	
	// On the minute update basis
	if (cling.time.local_minute_updated) {
		_logging_per_minute();

		cling.time.local_minute_updated = FALSE;
		N_SPRINTF("[TRACKING] time: %d", cling.time.time_since_1970);
	}
	
	// Check whether activity total should be flushed into flash
	if (cling.time.local_day_updated) {
		_logging_midnight_local();
	
		cling.time.local_day_updated = FALSE;
	}

	// Check if time just turn NOON
	if (cling.time.local_noon_updated) {
		cling.time.local_noon_updated = FALSE;
		N_SPRINTF("slepp by noon: %d", cling.activity.sleep_by_noon);
		cling.activity.sleep_by_noon = 0;
		cling.activity.sleep_stored_by_noon = TRACKING_get_sleep_by_noon(TRUE);

		// Remove sleep initialization as it interrupt sleep
//		SLEEP_init();
	}
	
	if (cling.activity.b_workout_active) 
	{
		_update_running_pace();
	}
}

// Check the system low power mode, whether it enters a NON-active mode
BOOLEAN TRACKING_is_not_active()
{
	return cling.lps.b_low_power_mode;
}

void TRACKING_get_sleep_statistics(I8U index, I32U *value)
{
	I32U buf[16];
	I8U *pbuf;
	I32U pos;
	I8U month, day;
	SYSTIME_CTX delta;
	BOOLEAN b_available = FALSE;
	
	if (cling.time.local.hour >= 12) {
		index --;
	}
	
	if (index == 0)  {
		// Just return whatever the value that we stored
		*value = cling.activity.sleep_stored_by_noon;
		return;
	}
	
	RTC_get_delta_clock_backward(&delta, index);
	pbuf = (I8U *)buf;
	for (pos = 0; pos < SYSTEM_DAYSTAT_SPACE_SIZE; pos+=64) {
		FLASH_Read_App(SYSTEM_DAYSTAT_SPACE_START+pos, pbuf, 64);
		
		if (pbuf[0] == 0xff) {
			break;
		} else {
			day = buf[0] & 0xff;
			month = (buf[0]>>8) & 0xff;
			if ((delta.month == month) && (delta.day == day)) {
				b_available = TRUE;
				break;
			}
		}
	}
	N_SPRINTF("[TRACKING] delta: %d, %d, index: %d", delta.month, delta.day, index);
		
	if (b_available) {
		*value = buf[1];
	} else {
		*value = 0;
	}
}

void TRACKING_get_activity(I8U index, I8U mode, I32U *value)
{
	I32U buf[16];
	I8U *pbuf;
	I32U pos;
	I8U month, day;
	SYSTIME_CTX delta;
	TRACKING_CTX *t = &cling.activity;
	BOOLEAN b_available = FALSE;
	
	N_SPRINTF("Tracking: %d, %d, %d, %d, %d", cling.activity.day.sleep, cling.activity.day.walking, cling.activity.day.running, cling.activity.day.calories>>4, cling.activity.day.distance>>4);

	if (index == 0) {
		switch (mode) {
			case TRACKING_STEPS:
				*value = t->day.walking + t->day.running;
				break;
			case TRACKING_CALORIES:
				*value = (t->day.calories >> 4);
				break;
			case TRACKING_SLEEP:
				*value = t->sleep_by_noon; //t->day.sleep;
				break;
			case TRACKING_DISTANCE:
				*value = (t->day.distance >> 4);
				break;
			case TRACKING_ACTIVE_TIME:
				*value = t->day.active_time;
			default:
				break;
		}
		
		N_SPRINTF("[TRACKING] index: %d, mode: %d, value: %d", index, mode, value);
		
		b_available = TRUE;

	} else {
		
		if (mode == TRACKING_SLEEP) {
			TRACKING_get_sleep_statistics(index, value);
			return;
		}
		RTC_get_delta_clock_backward(&delta, index);
		pbuf = (I8U *)buf;
		for (pos = 0; pos < SYSTEM_DAYSTAT_SPACE_SIZE; pos+=64) {
			FLASH_Read_App(SYSTEM_DAYSTAT_SPACE_START+pos, pbuf, 64);
			
			if (pbuf[0] == 0xff) {
				break;
			} else {
				day = buf[0] & 0xff;
				month = (buf[0]>>8) & 0xff;
				if ((delta.month == month) && (delta.day == day)) {
					b_available = TRUE;
					break;
				}
			}
		}
		N_SPRINTF("[TRACKING] delta: %d, %d, index: %d", delta.month, delta.day, index);
			
		if (b_available) {
			switch (mode) {
				case TRACKING_STEPS:
					*value = buf[2];
					N_SPRINTF("[TRACKING] %x, %x,%x, %x,", pbuf[8], pbuf[9], pbuf[10], pbuf[11]);
					break;
				case TRACKING_CALORIES:
					*value = buf[4];
					N_SPRINTF("[TRACKING] %x, %x,%x, %x,", pbuf[16], pbuf[17], pbuf[18], pbuf[19]);
					break;
				case TRACKING_SLEEP:
					*value = buf[1];
					N_SPRINTF("[TRACKING] %x, %x,%x, %x,", pbuf[4], pbuf[5], pbuf[6], pbuf[7]);
					break;
				case TRACKING_DISTANCE:
					*value = buf[3];
					N_SPRINTF("[TRACKING] %x, %x,%x, %x,", pbuf[12], pbuf[13], pbuf[14], pbuf[16]);
					break;
				case TRACKING_ACTIVE_TIME:
					*value = buf[5];
					N_SPRINTF("[TRACKING] %x, %x,%x, %x,", pbuf[12], pbuf[13], pbuf[14], pbuf[16]);
					break;
				default:
					break;
			}
		} else {
			*value = 0;
		}
		
		N_SPRINTF("[TRACKING] index: %d, mode: %d, value: %d, pos: %d", index, mode, value, pos);

	}
	
}

void TRACKING_get_daily_streaming_sleep(DAY_STREAMING_CTX *day_streaming)
{
	I32U epoch_start = RTC_get_epoch_day_start(0);
	I32U offset = 0;
	I32U dw_buf[4];
	MINUTE_TRACKING_CTX *minute = (MINUTE_TRACKING_CTX *)dw_buf;
	I8U *pbuf = (I8U *)dw_buf;
	I32U previous_sleep_state;
	I8U sleep_active_state;
	/*	
	if (cling.time.local.hour >= 12) {
		epoch_start += (EPOCH_DAY_SECOND>>1);
	} else 
	*/
	
	// Change to total sleep hour in a duration of 36 hours
	{
		epoch_start -= (EPOCH_DAY_SECOND>>1);
	}

	// other extra variables
	previous_sleep_state = SLP_STAT_AWAKE;
	
	N_SPRINTF("[TRACKING] streaming epoch start: %d", epoch_start);

	day_streaming->sleep_light = 0;
	day_streaming->sleep_sound = 0;
	day_streaming->sleep_rem = 0;
	day_streaming->wake_up_time = 0;
	
	while (offset < SYSTEM_TRACKING_SPACE_SIZE) {
		FLASH_Read_App(offset + SYSTEM_TRACKING_SPACE_START, pbuf, 16);
		
		if (pbuf[0] == 0xff) {
			
			offset += 16;
      continue;
		}
		offset += 16;

		minute->epoch &= 0x7fffffff;
		if (minute->epoch >= epoch_start) {

			// For some reason (surely we need to figure out the root cause),
			// if epoch appears invalid, go look for next valid entry
			if (minute->epoch == 0x7fffffff)
				continue;
			
			sleep_active_state = minute->sleep_state  & 0x07;
			// 5. Sleep times + sleep duration
			if (sleep_active_state == SLP_STAT_LIGHT) {
				day_streaming->sleep_light += 60;
				if ((previous_sleep_state == SLP_STAT_SOUND) || (previous_sleep_state == SLP_STAT_REM)) {
					day_streaming->wake_up_time ++;
				}
			} else if (sleep_active_state == SLP_STAT_SOUND) {
				day_streaming->sleep_sound += 60;
				if ((previous_sleep_state == SLP_STAT_REM) || (previous_sleep_state == SLP_STAT_LIGHT)) {
					day_streaming->wake_up_time ++;
				}
			} else if (sleep_active_state == SLP_STAT_REM) {
				day_streaming->sleep_rem += 60;
				if ((previous_sleep_state == SLP_STAT_SOUND) || (previous_sleep_state == SLP_STAT_LIGHT)) {
					day_streaming->wake_up_time ++;
				}
			}
			
			previous_sleep_state = minute->sleep_state;
		}
	}
}

void TRACKING_get_daily_streaming_stat(DAY_STREAMING_CTX *day_streaming)
{
	I32U epoch_start = RTC_get_epoch_day_start(0);
	I32U offset = 0;
	I32U dw_buf[4];
	I16U denormalized_distance;
	BOOLEAN b_init_offset = FALSE;
	MINUTE_TRACKING_CTX *minute = (MINUTE_TRACKING_CTX *)dw_buf;
	I8U *pbuf = (I8U *)dw_buf;
	I32U steps_in_a_minute, overall_temperature, overall_heart_rate, wearing_minutes;
	
	// other extra variables
	steps_in_a_minute = 0;
	overall_temperature = 0;
	overall_heart_rate = 0;
	wearing_minutes = 0;
	
	N_SPRINTF("[TRACKING] streaming epoch start: %d", epoch_start);

	day_streaming->steps = 0;
	day_streaming->distance = 0;
	day_streaming->calories_active = 0;
	day_streaming->calories_idle = 0;
	day_streaming->temperature = 0;
	day_streaming->heart_rate = 0;
	
	while (offset < SYSTEM_TRACKING_SPACE_SIZE) {
		FLASH_Read_App(offset + SYSTEM_TRACKING_SPACE_START, pbuf, 16);
		
		if (pbuf[0] == 0xff) {
			
			if (!b_init_offset) {
				b_init_offset = TRUE;
			}
			offset += 16;
		  continue;
		}
		offset += 16;

		minute->epoch &= 0x7fffffff;
		if (minute->epoch >= epoch_start) {
			
			// For some reason (surely we need to figure out the root cause),
			// if epoch appears invalid, go look for next valid entry
			if (minute->epoch == 0x7fffffff)
				continue;
			
			steps_in_a_minute = minute->walking+minute->running;
			
			// 1. Accumulating steps
			day_streaming->steps += steps_in_a_minute;
			
			// 2. Accumulating distance
			denormalized_distance = minute->distance;
			denormalized_distance <<= 1;
			day_streaming->distance += denormalized_distance;

			// 3. Accumulating calories (for exercise and idel)
			if (steps_in_a_minute > 0) {
				day_streaming->calories_active += minute->calories;
			} else {
				day_streaming->calories_idle += minute->calories;
			}
			
			// 4. Wearing minutes, and average heart_rate & temperature
			if ((minute->skin_touch_pads & 0x07)> 0) {
				wearing_minutes ++;
				overall_heart_rate += minute->heart_rate;
				overall_temperature += minute->skin_temperature;
			}
			
		}
		
	}
	
	if (wearing_minutes > 0)
	{
		// Finally, average heart rate and skin temperature
		day_streaming->heart_rate = overall_heart_rate / wearing_minutes;
		day_streaming->temperature = overall_temperature / wearing_minutes;
	}
	else
	{
		day_streaming->heart_rate = 0;
		day_streaming->temperature = 0;
	}
}

I32U TRACKING_get_sleep_by_noon(BOOLEAN b_previous_day)
{
	I32U epoch_start = RTC_get_epoch_day_start(0);
	I32U sleep_seconds = 0;
	I32U offset = 0;
	I32U dw_buf[4];
	MINUTE_TRACKING_CTX *minute = (MINUTE_TRACKING_CTX *)dw_buf;
	I8U *pbuf = (I8U *)dw_buf;
	I8U sleep_active_state;
	
	// Check whether we want to get previous 24 hours sleep data
	if (b_previous_day) {
		epoch_start -= (EPOCH_DAY_SECOND>>1);
	} else {
		if (cling.time.local.hour >= 12) {
			epoch_start += (EPOCH_DAY_SECOND>>1);
		} else {
			epoch_start -= (EPOCH_DAY_SECOND>>1);
		}
	}
	
	while (offset < SYSTEM_TRACKING_SPACE_SIZE) {
		FLASH_Read_App(offset + SYSTEM_TRACKING_SPACE_START, pbuf, 16);
		
		if (pbuf[0] == 0xff) {
			
			offset += 16;
      continue;
		}
		offset += 16;
 
		minute->epoch &= 0x7fffffff;
		if (minute->epoch >= epoch_start) {
			// For some reason (surely we need to figure out the root cause),
			// if epoch appears invalid, go look for next valid entry
			if (minute->epoch == 0x7fffffff)
				continue;
			
			// Get LSB 3 bits for sleep state, the high bits might be used for other parameters, such as VOC value
			sleep_active_state = minute->sleep_state & 0x07;
			if ((sleep_active_state == SLP_STAT_LIGHT) ||
				  (sleep_active_state == SLP_STAT_SOUND) ||
			    (sleep_active_state == SLP_STAT_REM))
			{
				sleep_seconds += 60;
			}
		}
	}
	
	return sleep_seconds;
}

I32U TRACKING_get_daily_total(DAY_TRACKING_CTX *day_total)
{
	I32U epoch_start = RTC_get_epoch_day_start(0);
	I32U offset = 0;
	I32U empty_offset=0;
	I32U dw_buf[4];
	BOOLEAN b_init_offset = FALSE;
	MINUTE_TRACKING_CTX *minute = (MINUTE_TRACKING_CTX *)dw_buf;
	I8U *pbuf = (I8U *)dw_buf;
	RUNNING_TRACK_CTX *r = &cling.run_stat;
	N_SPRINTF("[TRACKING] total epoch start: %d", epoch_start);
	
	// Adding running statistics
	r->calories = 0;
	r->distance = 0;
	r->steps = 0;
	r->time_min = 0;
	r->accu_heart_rate = 0;

	day_total->walking = 0;
	day_total->running = 0;
	day_total->calories = 0;
	day_total->distance = 0;
	day_total->active_time = 0;
	while (offset < SYSTEM_TRACKING_SPACE_SIZE) {
		FLASH_Read_App(offset + SYSTEM_TRACKING_SPACE_START, pbuf, 16);
		
		if (minute->epoch == 0xffffffff) {
			
			if (!b_init_offset) {
				empty_offset = offset;
				b_init_offset = TRUE;
				
				N_SPRINTF("[TRACKING] get first empty offset: %d", empty_offset);
			}
			offset += 16;
      continue;
		}
		offset += 16;
		minute->epoch &= 0x7fffffff;

		if (minute->epoch >= epoch_start) {
			// For some reason (surely we need to figure out the root cause),
			// if epoch appears invalid, go look for next valid entry
			if (minute->epoch != 0x7fffffff) {
				
				
				day_total->walking += minute->walking;
				day_total->running += minute->running;
				day_total->distance += minute->distance;// Denormalize distance as it is in unit of 2 meters
				day_total->calories += minute->calories;
				if ((minute->walking+minute->running) >= 40) {
					day_total->active_time ++;
				}
#if 0
				{
					SYSTIME_CTX local;

					RTC_get_local_clock(minute->epoch, &local);

					N_SPRINTF("Activity total(%d, %d): %d, %d (%d:%d)", offset, minute->epoch, day_total->walking, day_total->running, local.hour, local.minute);
				}
#endif
				// Make sure we have activity work out type enabled
				if (minute->uv_and_activity_type) { 
					if (minute->running > 99) {
						r->time_min ++;
						r->accu_heart_rate += minute->heart_rate;
						r->calories += minute->calories;
						r->steps += minute->walking + minute->running;
						r->distance += minute->distance;
					}
				}
			}
		}
		
	}

	day_total->distance <<= 5; // denormalize distance as it is in unit of 2 meters
	day_total->calories <<= 4;
	r->distance <<= 1; // Denormalize distance as it is in unit of 2 meters
	
	
	N_SPRINTF("Totals: %d, %d, %d, %d",  day_total->walking, day_total->running, day_total->calories>>4, day_total->distance>>4);

	return empty_offset;
}

