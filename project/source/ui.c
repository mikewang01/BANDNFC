/***************************************************************************//**
 * File: ui.c
 * 
 * Description: master UI controller
 *
 ******************************************************************************/

#include "main.h"
#include "ui_frame.h"
#include "ui_animation.h"
#include "font.h"
#ifdef _CLINGBAND_2_PAY_MODEL_
#include "ui_matrix_pay.h"
#include "player_controller.h"
#include "ble_pay_app.h"
#elif defined _CLINGBAND_PACE_MODEL_
#include "ui_matrix_pace.h"
#elif defined _CLINGBAND_VOC_MODEL_
#include "ui_matrix_voc.h"
#include "player_controller.h"
#elif defined _CLINGBAND_NFC_MODEL_
#include "ui_matrix_nfc.h"
#elif defined _CLINGBAND_UV_MODEL_
#include "ui_matrix_uv.h"
#endif

#define UI_SPRINTF Y_SPRINTF

/*------------------------------------------------------------------------------------------
*  Function:	_is_regular_page(I8U frame_index)
*
*  Description: Determine whether the current page is regular page.
*
*------------------------------------------------------------------------------------------*/
static BOOLEAN _is_regular_page(I8U frame_index)
{
	if (frame_index == UI_DISPLAY_HOME)
		return TRUE;
	else if ((frame_index >= UI_DISPLAY_TRACKER) && (frame_index <= UI_DISPLAY_TRACKER_END))
		return TRUE;
	else if ((frame_index >= UI_DISPLAY_VITAL) && (frame_index <= UI_DISPLAY_VITAL_END))
		return TRUE;		
#ifdef _CLINGBAND_PACE_MODEL_		
	else if (frame_index == UI_DISPLAY_RUNNING_STAT_RUN_ANALYSIS)
    return TRUE;		
	else if (frame_index == UI_DISPLAY_SMART_PM2P5)
    return TRUE;
	else if (frame_index == UI_DISPLAY_SMART_WEATHER)
    return TRUE;
	else if (frame_index == UI_DISPLAY_TRAINING_STAT_START)
    return TRUE;
#endif	
	else 
		return FALSE;
}

/*------------------------------------------------------------------------------------------
*  Function:	_is_system_page(I8U frame_index)
*
*  Description: Determine whether the current page is system page.
*
*------------------------------------------------------------------------------------------*/
#if 0
static BOOLEAN _is_system_page(I8U frame_index)
{
  if ((frame_index >= UI_DISPLAY_SYSTEM) && (frame_index <= UI_DISPLAY_SYSTEM_END))
		return TRUE;
	else 
		return FALSE;
}
#endif

/*------------------------------------------------------------------------------------------
*  Function:	_is_running_analysis_page(I8U frame_index)
*
*  Description: Determine whether the current page is running analysis page.
*
*------------------------------------------------------------------------------------------*/
static BOOLEAN _is_running_analysis_page(I8U frame_index)
{
  if ((frame_index >= UI_DISPLAY_RUNNING_STAT_DISTANCE) && (frame_index <= UI_DISPLAY_RUNNING_STATATISTICS_END))
		return TRUE;
	else 
		return FALSE;
}

/*------------------------------------------------------------------------------------------
*  Function:	_is_running_active_mode_page(I8U frame_index)
*
*  Description: Determine whether the current page is running active mode page.
*
*------------------------------------------------------------------------------------------*/
static BOOLEAN _is_running_active_mode_page(I8U frame_index)
{
  if ((frame_index >= UI_DISPLAY_TRAINING_STAT_TIME) && (frame_index <= UI_DISPLAY_TRAINING_STAT_RUN_STOP))
		return TRUE;
#ifndef _CLINGBAND_PACE_MODEL_	
  else if ((frame_index >= UI_DISPLAY_CYCLING_OUTDOOR_STAT_TIME) && (frame_index <= UI_DISPLAY_CYCLING_OUTDOOR_STAT_STOP))
		return TRUE;
  else if ((frame_index >= UI_DISPLAY_WORKOUT_RT_TIME) && (frame_index <= UI_DISPLAY_WORKOUT_RT_END))
		return TRUE;	
#endif	
	else 
		return FALSE;
}

/*------------------------------------------------------------------------------------------
*  Function:	_is_smart_incoming_notifying_page(I8U frame_index)
*
*  Description: Determine whether the current page is notification smart page.
*
*------------------------------------------------------------------------------------------*/
static BOOLEAN _is_smart_incoming_notifying_page(I8U frame_index)
{
	UI_ANIMATION_CTX *u = &cling.ui;

#ifdef _CLINGBAND_PACE_MODEL_		
  if (u->frame_index == UI_DISPLAY_SMART_DETAIL_NOTIF) {
		u->notific.b_in_incoming_detail_page = TRUE;
		return TRUE;
	}
#else
	if ((u->notific.b_in_incoming_detail_page) && (u->frame_index == UI_DISPLAY_SMART_DETAIL_NOTIF))
		return TRUE;
#endif	
  else if (u->frame_index == UI_DISPLAY_SMART_INCOMING_CALL)
		return TRUE;
	else if (u->frame_index == UI_DISPLAY_SMART_INCOMING_MESSAGE)
		return TRUE;
	else if (u->frame_index == UI_DISPLAY_SMART_ALARM_CLOCK_REMINDER)
		return TRUE;	
	else if (u->frame_index == UI_DISPLAY_SMART_IDLE_ALERT)
		return TRUE;	
	else if (u->frame_index == UI_DISPLAY_SMART_HEART_RATE_ALERT)
		return TRUE;
	else if (u->frame_index == UI_DISPLAY_SMART_STEP_10K_ALERT)
		return TRUE;	
	else
		return FALSE;
}

/*------------------------------------------------------------------------------------------
*  Function:	_is_other_need_store_page(I8U frame_index)
*
*  Description: Some page nedd store.
*
*------------------------------------------------------------------------------------------*/
#ifndef _CLINGBAND_PACE_MODEL_	
static BOOLEAN _is_other_need_store_page(I8U frame_index)
{
  if ((frame_index >= UI_DISPLAY_CAROUSEL) && (frame_index <= UI_DISPLAY_CAROUSEL_END))
		return TRUE;	
  else if ((frame_index >= UI_DISPLAY_WORKOUT_TREADMILL) && (frame_index <= UI_DISPLAY_WORKOUT_OTHERS))
		return TRUE;	
  else if ((frame_index >= UI_DISPLAY_STOPWATCH) && (frame_index <= UI_DISPLAY_STOPWATCH_END))
		return TRUE;	
	else if (frame_index == UI_DISPLAY_SETTING_VER) 
		return TRUE;	
	else if (frame_index == UI_DISPLAY_SMART_WEATHER) 
		return TRUE;	
	else if (frame_index == UI_DISPLAY_SMART_MESSAGE) 
		return TRUE;	
	else if (frame_index == UI_DISPLAY_SMART_ALARM_CLOCK_DETAIL) 
		return TRUE;		
#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
  else if ((frame_index >= UI_DISPLAY_MUSIC) && (frame_index <= UI_DISPLAY_MUSIC_END))
		return TRUE;	
#endif
  else 
    return FALSE;		
}
#endif

/*------------------------------------------------------------------------------------------
*  Function:	_ui_vertical_animation(I8U frame_index)
*
*  Description: Judge whether the current page to allow vertical animation.
*
*------------------------------------------------------------------------------------------*/
static BOOLEAN _ui_vertical_animation(I8U frame_index)
{	
#ifdef _CLINGBAND_PACE_MODEL_			
  if (frame_index == UI_DISPLAY_SMART_DETAIL_NOTIF) {
		return TRUE; 
	} else if (frame_index == UI_DISPLAY_SMART_INCOMING_MESSAGE) {
		return TRUE; 
	} else if (frame_index == UI_DISPLAY_SMART_ALARM_CLOCK_REMINDER) {
		return TRUE; 
	} else if (frame_index == UI_DISPLAY_SMART_IDLE_ALERT) {
		return TRUE; 
	} else if (frame_index == UI_DISPLAY_SMART_HEART_RATE_ALERT) {
		return TRUE;
	} else if (frame_index == UI_DISPLAY_SMART_STEP_10K_ALERT) {
		return TRUE;
	} else {
		return FALSE;
	}
#endif
	
#ifndef _CLINGBAND_PACE_MODEL_		
	if ((frame_index >= UI_DISPLAY_TRACKER) && (frame_index <= UI_DISPLAY_TRACKER_ACTIVE_TIME)) 
		return TRUE;
	else if (frame_index == UI_DISPLAY_SMART_MESSAGE)
		return TRUE;
	else if (frame_index == UI_DISPLAY_SMART_APP_NOTIF)
		return TRUE;
	else if (frame_index == UI_DISPLAY_SMART_DETAIL_NOTIF)
		return TRUE;
	else if (frame_index == UI_DISPLAY_SMART_INCOMING_MESSAGE)
		return TRUE;	
	else if (frame_index == UI_DISPLAY_SMART_ALARM_CLOCK_REMINDER)
		return TRUE;		
	else if (frame_index == UI_DISPLAY_SMART_ALARM_CLOCK_DETAIL)
		return TRUE;	
	else if (frame_index == UI_DISPLAY_SMART_IDLE_ALERT)
		return TRUE;	
	else if (frame_index == UI_DISPLAY_SMART_HEART_RATE_ALERT)
		return TRUE;		
	else if (frame_index == UI_DISPLAY_SMART_STEP_10K_ALERT)
		return TRUE;		
	else 
		return FALSE;
#endif	
}

/*------------------------------------------------------------------------------------------
*  Function:	_stote_frame_cached_index()
*
*  Description: Store cached frame index.
*
*	 1>regular page. 2>running analysis. 3>workout active page.(Other pages don't need to save)
*
*------------------------------------------------------------------------------------------*/
static void _stote_frame_cached_index()
{
	UI_ANIMATION_CTX *u = &cling.ui;

  // 1. Store regular page cached index
	if (_is_regular_page(u->frame_index)) {
		u->frame_cached_index = u->frame_index;			
	}
	
	// 2. Store running analysis page cached index
	if (_is_running_analysis_page(u->frame_index)) {
		u->frame_cached_index = u->frame_index;					
	}
	
	// 3. Store running active page cached index(In addition to running alarm page)
	if (_is_running_active_mode_page(u->frame_index) && (!u->b_in_running_alert_page)) {
		 u->frame_cached_index = u->frame_index;			
	}
	
	// 4. Store other needed page cached index
#ifndef _CLINGBAND_PACE_MODEL_		
	if (_is_other_need_store_page(u->frame_index)) {
		u->frame_cached_index = u->frame_index;	
	}	
#endif	
}

/*--------------------------------------------------------------------------------------------
*  Function:	_restore_perv_frame_index()
*
*  Description:  Go back to previous UI page.
*
*	 Priority: Running mode page >> Notification page >>  Other pervious page before screen dark.
*
*--------------------------------------------------------------------------------------------*/
static void _restore_perv_frame_index()
{
	UI_ANIMATION_CTX *u = &cling.ui;
	I32U t_curr = CLK_get_system_time();
	TRACKING_CTX *a = &cling.activity;

	// 1. Check if user in running active mode.
	if (a->b_workout_active) {
	
		if (_is_running_active_mode_page(u->frame_cached_index)) {
			u->frame_index = u->frame_cached_index;				
			return;
		}
		
		if (a->workout_type == WORKOUT_RUN_OUTDOOR) {
			u->frame_index = UI_DISPLAY_TRAINING_STAT_TIME;		
		}	
#ifndef _CLINGBAND_PACE_MODEL_			
	  else if (a->workout_type == WORKOUT_CYCLING_OUTDOOR) {
			u->frame_index = UI_DISPLAY_CYCLING_OUTDOOR_STAT_TIME;		
		}	else {
			u->frame_index = UI_DISPLAY_WORKOUT_RT_TIME;		
		}
#endif		
#ifdef _CLINGBAND_PACE_MODEL_				
		else {
			u->frame_index = UI_DISPLAY_TRAINING_STAT_TIME;		
		} 	
#endif		
		return;
	}	
	
	// 3. Go back to previous UI page.
	if (u->frame_index == UI_DISPLAY_PREVIOUS) {
		if (t_curr > (u->dark_time_stamp + UI_STORE_FRAME_MAX_TIME_IN_MS)) {
			u->frame_index = UI_DISPLAY_HOME;					
		} else {
#ifdef _CLINGBAND_PACE_MODEL_	
      // Go back to Home clock page.			
			u->frame_index = UI_DISPLAY_HOME;		
#else			
			// Go back to previous UI page.
			u->frame_index = u->frame_cached_index;	
#endif			
#ifdef _CLINGBAND_PACE_MODEL_						
	    if ((u->frame_index == UI_DISPLAY_VITAL_HEART_RATE) || (u->frame_index == UI_DISPLAY_TRAINING_STAT_HEART_RATE)) {	
#else
			if ((u->frame_index == UI_DISPLAY_VITAL_HEART_RATE) 
				||(u->frame_index == UI_DISPLAY_TRAINING_STAT_HEART_RATE)
				||(u->frame_index == UI_DISPLAY_WORKOUT_RT_HEART_RATE)	
				||(u->frame_index == UI_DISPLAY_CYCLING_OUTDOOR_STAT_HEART_RATE)) {
#endif					
#ifdef _ENABLE_PPG_						
		    PPG_closing_to_skin_detect_init();
#endif					
		  } 		
		}	
	}
}

/*-------------------------------------------------------------------------------
*  Function:	_get_regular_page_enable_index(I8U frame_index)
*
*  Description: Get regular page enable index.
*
*-------------------------------------------------------------------------------*/
static I16U _get_regular_page_enable_index(I8U frame_index)
{
	if (frame_index == UI_DISPLAY_HOME)
		return UI_FRAME_ENABLE_HOME;
	else if (frame_index == UI_DISPLAY_TRACKER_STEP)
		return UI_FRAME_ENABLE_STEP;		
	else if (frame_index == UI_DISPLAY_TRACKER_DISTANCE)
		return UI_FRAME_ENABLE_DISTANCE;		
	else if (frame_index == UI_DISPLAY_TRACKER_CALORIES)
		return UI_FRAME_ENABLE_CALORIES;			
	else if (frame_index == UI_DISPLAY_TRACKER_ACTIVE_TIME)
		return UI_FRAME_ENABLE_ACTIVE_TIME;		
	else if (frame_index == UI_DISPLAY_VITAL_HEART_RATE)
		return UI_FRAME_ENABLE_HEART_RATE;			
#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)	
	else if (frame_index == UI_DISPLAY_VITAL_SKIN_TEMP)
		return UI_FRAME_ENABLE_SKIN_TEMP;		
#endif	
#ifdef _CLINGBAND_UV_MODEL_	
	else if (frame_index == UI_DISPLAY_TRACKER_UV_IDX)
		return UI_FRAME_ENABLE_UV_INDEX;	
#endif	
#ifdef _CLINGBAND_PACE_MODEL_			
	else if (frame_index == UI_DISPLAY_SMART_WEATHER)
		return UI_FRAME_ENABLE_WEATHER;			
	else if (frame_index == UI_DISPLAY_SMART_PM2P5)
		return UI_FRAME_ENABLE_PM2P5;		
	else if (frame_index == UI_DISPLAY_TRAINING_STAT_START)
		return UI_FRAME_ENABLE_START_RUN;				
	else if (frame_index == UI_DISPLAY_RUNNING_STAT_RUN_ANALYSIS)
		return UI_FRAME_ENABLE_RUN_ANALYSIS;			
#endif	
	else 
		return 0x0000;
}

/*-------------------------------------------------------------------------------
*  Function:	_get_running_analysis_page_enable_index(I8U frame_index)
*
*  Description: Get running analysis page enable index.
*
*-------------------------------------------------------------------------------*/
static I8U _get_running_analysis_page_enable_index(I8U frame_index)
{
	if (frame_index == UI_DISPLAY_RUNNING_STAT_DISTANCE)
		return UI_FRAME_ENABLE_RUNNING_DISTANCE;
	else if (frame_index == UI_DISPLAY_RUNNING_STAT_TIME)
		return UI_FRAME_ENABLE_RUNNING_TIME;		
	else if (frame_index == UI_DISPLAY_RUNNING_STAT_PACE)
		return UI_FRAME_ENABLE_RUNNING_PACE;		
	else if (frame_index == UI_DISPLAY_RUNNING_STAT_STRIDE)
		return UI_FRAME_ENABLE_RUNNING_STRIDE;			
	else if (frame_index == UI_DISPLAY_RUNNING_STAT_CADENCE)
		return UI_FRAME_ENABLE_RUNNING_CADENCE;		
	else if (frame_index == UI_DISPLAY_RUNNING_STAT_HEART_RATE)
		return UI_FRAME_ENABLE_RUNNING_HEART_RATE;			
	else if (frame_index == UI_DISPLAY_RUNNING_STAT_CALORIES)
		return UI_FRAME_ENABLE_RUNNING_CALORIES;				
#ifdef _CLINGBAND_PACE_MODEL_					
	else if (frame_index == UI_DISPLAY_RUNNING_STAT_STOP_ANALYSIS)
		return UI_FRAME_ENABLE_RUNNING_STOP_ANALYSIS;			
#endif	
	else 
		return 0x00;
}

/*-------------------------------------------------------------------------------
*  Function:	_update_page_filtering_pro(UI_ANIMATION_CTX *u, const I8U *p_matrix)
*
*  Description: Page filtering.
*
*-------------------------------------------------------------------------------*/
static void _update_page_filtering_pro(UI_ANIMATION_CTX *u, const I8U *p_matrix)
{
	I16U regular_page_filtering = 0;	
	I16U regular_page_enable = 0;
	I8U run_analysis_page_filtering = 0;
	I8U run_analysis_page_enable = 0;	
	I8U i = 0;
	USER_PROFILE_CTX *p = &cling.user_data.profile;

	if (_is_regular_page(u->frame_index)) {
		
		regular_page_filtering = p->regular_page_display_2;
		regular_page_filtering <<= 8;
		regular_page_filtering |= p->regular_page_display_1;
		
#ifdef _CLINGBAND_PACE_MODEL_			
		// Allways open home page and run analysis page.
	  regular_page_filtering |= 0x8200; 	
#else
		// Allways open home page. 
	  regular_page_filtering |= 0x8000; 
#endif

		for (i=0;i<10;i++) {
			u->frame_index = p_matrix[u->frame_index];
			if (_is_regular_page(u->frame_index)) {
				regular_page_enable = _get_regular_page_enable_index(u->frame_index);
				if (regular_page_enable & regular_page_filtering) 
					break;				
			} else {
#ifdef _CLINGBAND_PACE_MODEL_							
				if (u->frame_index == UI_DISPLAY_TRAINING_STAT_CONNECT_GPS) {
				  if (u->frame_prev_idx != UI_DISPLAY_TRAINING_STAT_START) {
						u->frame_index = UI_DISPLAY_TRAINING_STAT_START;
					} 
				}
				if (u->frame_index == UI_DISPLAY_RUNNING_STAT_DISTANCE) {
				  if (u->frame_prev_idx != UI_DISPLAY_RUNNING_STAT_RUN_ANALYSIS) {
						u->frame_index = UI_DISPLAY_RUNNING_STAT_RUN_ANALYSIS;
					} 					
				}
#endif				
				break;	
			}
		}
		
		u->frame_next_idx = u->frame_index;
	} else if (_is_running_analysis_page(u->frame_index)) {
		
	  run_analysis_page_filtering = p->running_page_display;

#ifdef _CLINGBAND_PACE_MODEL_			
		// Allways open Running distance page and stop analysis page.
	  run_analysis_page_filtering |= 0x81;	
#else 
		// Allways open Running distance page.
	  run_analysis_page_filtering |= 0x01;	 	
#endif
		
		for (i=0;i<10;i++) {
			u->frame_index = p_matrix[u->frame_index];
			if (_is_running_analysis_page(u->frame_index)) {
		    run_analysis_page_enable = _get_running_analysis_page_enable_index(u->frame_index);		
		    if (run_analysis_page_enable & run_analysis_page_filtering) 
				  break;								
			} else {
				break;
			}
		}		
		
	  u->frame_next_idx = u->frame_index;
	} else {
		
		u->frame_index = p_matrix[u->frame_index];
		u->frame_next_idx = u->frame_index;
	}
}

/*------------------------------------------------------------------------------------------
*  Function:	_update_frame_index(UI_ANIMATION_CTX *u, const I8U *p_matrix, I8U gesture)
*
*  Description: Switch to next display frame. 
*
*------------------------------------------------------------------------------------------*/
static void _update_frame_index(UI_ANIMATION_CTX *u, const I8U *p_matrix, I8U gesture)
{
#ifdef _CLINGBAND_PACE_MODEL_		
	if (u->frame_index == UI_DISPLAY_SMART_INCOMING_MESSAGE) {				
		if (!u->b_detail_page) {
			u->frame_next_idx = u->frame_index;					
			return;
		}
	}
#endif
	
	_update_page_filtering_pro(u, p_matrix);
	
	// Display running analysis page only user has actual running distance. 
#ifdef _CLINGBAND_PACE_MODEL_	
	if (u->frame_index == UI_DISPLAY_RUNNING_STAT_RUN_ANALYSIS) {
		if (!cling.run_stat.distance) {
			u->frame_index = UI_DISPLAY_HOME;
			u->frame_next_idx = u->frame_index;			
		}		
	}
#else	
	if ((u->frame_index == UI_DISPLAY_SMART_MESSAGE) && (u->b_detail_page) && (gesture == TOUCH_FINGER_RIGHT)) {
		u->frame_index = UI_DISPLAY_SMART_APP_NOTIF;
		u->frame_next_idx = u->frame_index;		
    return;		
	}
	
	if ((u->frame_index == UI_DISPLAY_SMART_APP_NOTIF) && (u->b_detail_page) && (gesture == TOUCH_FINGER_RIGHT)) {
		u->frame_index = UI_DISPLAY_SMART_DETAIL_NOTIF;
		u->frame_next_idx = u->frame_index;		
		u->notific.b_in_incoming_detail_page = FALSE;
    return;		
	}

	if ((u->frame_prev_idx == UI_DISPLAY_SMART_INCOMING_MESSAGE) && (u->frame_index == UI_DISPLAY_SMART_DETAIL_NOTIF)) {
		u->notific.b_in_incoming_detail_page = TRUE;
	}
	
	if ((u->frame_index != UI_DISPLAY_SMART_INCOMING_MESSAGE) && (u->frame_index != UI_DISPLAY_SMART_DETAIL_NOTIF)) {
		u->notific.b_in_incoming_detail_page = FALSE;		
	}
	
	if ((gesture == TOUCH_SWIPE_LEFT) || (gesture == TOUCH_SWIPE_RIGHT)) {
		if (u->frame_index == UI_DISPLAY_SMART_DETAIL_NOTIF) {
			if (!u->notific.b_in_incoming_detail_page) {
				u->frame_index = UI_DISPLAY_SMART_APP_NOTIF;
				u->frame_next_idx = u->frame_index;		
				return;
			}
		}
	}		
	
	if ((u->frame_prev_idx == UI_DISPLAY_CAROUSEL_1) && (u->frame_index == UI_DISPLAY_TRAINING_STAT_START)) {
		if (cling.run_stat.distance) {
			u->frame_index = UI_DISPLAY_TRAINING_STAT_START_OR_ANALYSIS;
			u->frame_next_idx = u->frame_index;			
		}					
	}
#endif		
}

/*------------------------------------------------------------------------------------------
*  Function:	_set_animation_with_a_finger_touch(I8U frame_index, I8U gesture)
*
*  Description: According to the current page choose the corresponding animation effects. 
*
*------------------------------------------------------------------------------------------*/
#ifndef _CLINGBAND_PACE_MODEL_			
static void _set_animation_with_a_finger_touch(I8U frame_index, I8U gesture)
{
	BOOLEAN b_vertical_animation = FALSE;
	
	if ((gesture == TOUCH_FINGER_RIGHT) && _ui_vertical_animation(frame_index) && cling.ui.b_detail_page) {
		b_vertical_animation = TRUE;
	}
	
  if (b_vertical_animation) {
		UI_set_animation(ANIMATION_TILTING_OUT, TRANSITION_DIR_UP);
	} else {
  	UI_set_animation(ANIMATION_IRIS, TRANSITION_DIR_NONE);				
	}
}
#endif

/*------------------------------------------------------------------------------------------
*  Function:	_set_animation_with_a_swipe_touch(I8U frame_index, I8U gesture)
*
*  Description: According to the current page choose the corresponding animation effects. 
*
*------------------------------------------------------------------------------------------*/
#ifndef _CLINGBAND_PACE_MODEL_			
static void _set_animation_with_a_swipe_touch(I8U frame_index, I8U gesture)
{
	if (gesture == TOUCH_SWIPE_RIGHT) {
		UI_set_animation(ANIMATION_PANNING_OUT, TRANSITION_DIR_RIGHT);
	} 
	
	if (gesture == TOUCH_SWIPE_LEFT) {
		UI_set_animation(ANIMATION_PANNING_OUT, TRANSITION_DIR_LEFT);		
	}
}
#endif

/*------------------------------------------------------------------------------------------
*  Function:	_set_animation_with_button_single(I8U frame_index)
*
*  Description: According to the current page choose the corresponding animation effects. 
*
*------------------------------------------------------------------------------------------*/

static void _set_animation_with_button_single(I8U frame_index)
{
#ifdef _CLINGBAND_PACE_MODEL_			
	if (_is_regular_page(frame_index) || 
		  _is_running_analysis_page(frame_index) ||
	    _is_running_active_mode_page(frame_index)) {
				
		UI_set_animation(ANIMATION_PANNING_OUT, TRANSITION_DIR_LEFT);
    return;		
	} 
			
  if (_ui_vertical_animation(frame_index) && cling.ui.b_detail_page) {
		UI_set_animation(ANIMATION_TILTING_OUT, TRANSITION_DIR_UP);
	} else {
		UI_set_animation(ANIMATION_IRIS, TRANSITION_DIR_NONE);	
	}
#else
	UI_set_animation(ANIMATION_IRIS, TRANSITION_DIR_NONE);	
#endif	
}

/*------------------------------------------------------------------------------------------
*  Function:	_set_animation_with_button_hold(I8U frame_index)
*
*  Description: According to the current page choose the corresponding animation effects. 
*
*------------------------------------------------------------------------------------------*/
#ifdef _CLINGBAND_PACE_MODEL_	
static void _set_animation_with_button_hold(I8U frame_index)
{
	BOOLEAN b_vertical_animation = FALSE;
	
	if ((frame_index == UI_DISPLAY_HOME) ||
		  (frame_index == UI_DISPLAY_TRACKER_STEP) ||
	    (frame_index == UI_DISPLAY_TRACKER_DISTANCE) ||
	    (frame_index == UI_DISPLAY_TRACKER_CALORIES) ||
	    (frame_index == UI_DISPLAY_TRACKER_ACTIVE_TIME) ||
      (frame_index == UI_DISPLAY_VITAL_HEART_RATE) ||
	    (frame_index == UI_DISPLAY_SMART_WEATHER) ||
	    (frame_index == UI_DISPLAY_SMART_PM2P5)) {
						 
		UI_set_animation(ANIMATION_PANNING_OUT, TRANSITION_DIR_LEFT);
		return;					 
	}

  if (_ui_vertical_animation(frame_index) && cling.ui.b_detail_page) {
		b_vertical_animation = TRUE;
	}
	
	if (b_vertical_animation) {
		UI_set_animation(ANIMATION_TILTING_OUT, TRANSITION_DIR_UP);
	} else {
		UI_set_animation(ANIMATION_IRIS, TRANSITION_DIR_NONE);	
	}	
}
#endif

/*------------------------------------------------------------------------------------------
*  Function:	_update_notif_repeat_look_time(I8U frame_index)
*
*  Description: If user cycle look this notification, go back to previous UI page. 
*
*------------------------------------------------------------------------------------------*/
static void _update_notif_repeat_look_time(I8U frame_index)
{	
	BOOLEAN b_look_finished = FALSE;
  UI_NOTIFICC_CTX *notific = &cling.ui.notific;
	
	switch (frame_index) {
		case UI_DISPLAY_SMART_INCOMING_CALL: {
		  // Don't need to look the incoming call detail information, and go back to previous UI page directly.
		  b_look_finished = TRUE;		
			break;
		}
		case UI_DISPLAY_SMART_INCOMING_MESSAGE: {	
			// Do nothing.
			break;
		}
		case UI_DISPLAY_SMART_DETAIL_NOTIF: {
			if (notific->b_in_incoming_detail_page) {
			  if (notific->repeat_look_time >= UI_LOOK_MESSAGE_MAX_REPEAT_TIME) {
				  b_look_finished = TRUE;
					notific->b_in_incoming_detail_page = FALSE;
			  }			
		  }
			break;
		}		
		case UI_DISPLAY_SMART_ALARM_CLOCK_REMINDER: {
			notific->repeat_look_time++;
			if (notific->repeat_look_time >= UI_LOOK_ALARM_CLOCK_REMINDER_MAX_REPEAT_TIME) {
				b_look_finished = TRUE;
			}	
			break;
		}			
		case UI_DISPLAY_SMART_IDLE_ALERT: {
			notific->repeat_look_time++;
			if (notific->repeat_look_time >= UI_LOOK_IDLE_ALERT_MAX_REPEAT_TIME) {
				b_look_finished = TRUE;
			}		
			break;
		}		
		case UI_DISPLAY_SMART_HEART_RATE_ALERT: {
			notific->repeat_look_time++;
			if (notific->repeat_look_time >= UI_LOOK_HEART_RATE_ALERT_MAX_REPEAT_TIME) {
				b_look_finished = TRUE;
			}		
			break;
		}		
		case UI_DISPLAY_SMART_STEP_10K_ALERT: {
			notific->repeat_look_time++;
			if (notific->repeat_look_time >= UI_LOOK_STEP_10K_ALERT_MAX_REPEAT_TIME) {
				b_look_finished = TRUE;
			}	
			break;
		}
    default: {
			notific->repeat_look_time = 0;
      break;			
		}
	}

	if (b_look_finished) {
	  // Go back to previous UI page		
	  cling.ui.frame_index = UI_DISPLAY_PREVIOUS;
		// Clear message store flag
		notific->b_need_stored = FALSE;
		// Reset message look time
		notific->repeat_look_time = 0;		
	}	
}

/*------------------------------------------------------------------------------------------
*  Function:	_update_message_detail_index(UI_ANIMATION_CTX *u)
*
*  Description: Get next page notification string index. 
*
*------------------------------------------------------------------------------------------*/
static void _update_message_detail_index(UI_ANIMATION_CTX *u)
{	
	char data[128];	
	I8U max_frame_num = 0;
  UI_NOTIFICC_CTX *notific = &cling.ui.notific;
	
	if ((u->frame_index == UI_DISPLAY_SMART_DETAIL_NOTIF) && (u->frame_prev_idx == UI_DISPLAY_SMART_DETAIL_NOTIF)) {

#ifdef _CLINGBAND_PACE_MODEL_			
	  NOTIFIC_get_app_message_detail(0 ,data);
#else 		
	  NOTIFIC_get_app_message_detail(notific->app_notific_index ,data);
#endif
		
	  if (data[0] == 0)
		  return;
	
		max_frame_num = FONT_get_string_display_depth(data);
		
		N_SPRINTF("[UI] max frame num : %d, notific detail index: %d", max_frame_num, u->notif_detail_index);

	  // Update vertical Notific detail index
		notific->detail_idx++;
		
		if (notific->detail_idx >= max_frame_num) {
			notific->detail_idx = 0;
			notific->repeat_look_time++;
		}
	} else {
		notific->detail_idx = 0;		
	}
}

/*------------------------------------------------------------------------------------------
*  Function:	_update_alarm_clock_control(UI_ANIMATION_CTX *u, I8U gesture)
*
*  Description: update alram clock index and display control.
*
*------------------------------------------------------------------------------------------*/
#ifndef _CLINGBAND_PACE_MODEL_
static void _update_alarm_clock_control(UI_ANIMATION_CTX *u, I8U gesture)
{
	if ((u->frame_prev_idx == UI_DISPLAY_CAROUSEL_3) && (u->frame_index == UI_DISPLAY_SMART_ALARM_CLOCK_DETAIL)) {
		N_SPRINTF("[UI] check on alarm clock");
		// Always set to the first alarm clock
		REMINDER_get_time_at_index(0);
		cling.ui.vertical_index = 0;
		return;
	}
	
	if (gesture == TOUCH_FINGER_RIGHT) {
		if (u->frame_index == UI_DISPLAY_SMART_ALARM_CLOCK_DETAIL) {
			cling.ui.vertical_index = REMINDER_get_time_at_index(cling.ui.vertical_index);
			N_SPRINTF("[UI] new vertical index(Reminder): %d", cling.ui.vertical_index);		
		}
  }
}
#endif

/*------------------------------------------------------------------------------------------
*  Function:	_update_vertical_detail_page(UI_ANIMATION_CTX *u, I8U gesture)
*
*  Description: Control switch vertical detail page. 
*
*------------------------------------------------------------------------------------------*/
static void _update_vertical_detail_page(UI_ANIMATION_CTX *u, I8U gesture)
{
#ifdef _CLINGBAND_PACE_MODEL_		
  if (_ui_vertical_animation(u->frame_index)) {
		u->b_detail_page = TRUE;
	} else {
		u->b_detail_page = FALSE;
	}
#endif
	
#ifndef _CLINGBAND_PACE_MODEL_		
	I8U max_frame_num;

	if ((gesture == TOUCH_SWIPE_LEFT) || (gesture == TOUCH_SWIPE_RIGHT) || (gesture == TOUCH_BUTTON_SINGLE)) {
		u->b_detail_page = FALSE;
		u->vertical_index = 0;
		return;
	}
	
	if (_ui_vertical_animation(u->frame_index) && _ui_vertical_animation(u->frame_prev_idx)) {
		if (!u->b_detail_page) {
			u->b_detail_page = TRUE;
			return;
		}
	} else {
		u->b_detail_page = FALSE;
		u->vertical_index = 0;
	}
	
	if (gesture != TOUCH_FINGER_RIGHT)
		return;
	
	if (u->frame_index == UI_DISPLAY_TRACKER_STEP) {
		max_frame_num = FRAME_DEPTH_STEP;
	} else if (u->frame_index == UI_DISPLAY_TRACKER_DISTANCE) {
		max_frame_num = FRAME_DEPTH_DISTANCE;
	} else if (u->frame_index == UI_DISPLAY_TRACKER_CALORIES) {
		max_frame_num = FRAME_DEPTH_CALORIES;
	} else if (u->frame_index == UI_DISPLAY_TRACKER_ACTIVE_TIME) {
		max_frame_num = FRAME_DEPTH_ACTIVE_TIME;
	} else if (u->frame_index == UI_DISPLAY_SMART_ALARM_CLOCK_DETAIL) {
		max_frame_num = cling.reminder.total;
		N_SPRINTF("[UI] max reminder num: %d", max_frame_num);
	} else {
		max_frame_num = 0;
	}
	
	if (max_frame_num == 0) {
		u->vertical_index = 0;
		return;
	}
	
	// Update vertical index
	u->vertical_index++;
	if (u->vertical_index >= max_frame_num) {
		u->vertical_index = 0;
	}
	
	N_SPRINTF("[UI] new vertical index: %d", cling.ui.level_1_index);
#endif	
}

/*------------------------------------------------------------------------------------------
*  Function:	_update_horizontal_app_notific_index(UI_ANIMATION_CTX *u, I8U gesture)
*
*  Description: Update app notification display index. 
*
*------------------------------------------------------------------------------------------*/
#ifndef _CLINGBAND_PACE_MODEL_	
static void _update_horizontal_app_notific_index(UI_ANIMATION_CTX *u, I8U gesture)
{
	I8U max_frame_num=0;
  BOOLEAN b_up;
  UI_NOTIFICC_CTX *notific = &cling.ui.notific;
	
	if ((u->frame_index != UI_DISPLAY_SMART_APP_NOTIF) && (u->frame_index != UI_DISPLAY_SMART_DETAIL_NOTIF)) {
		notific->app_notific_index = 0;
		return;
	}

  if (u->frame_index == UI_DISPLAY_SMART_APP_NOTIF) {
#ifdef _ENABLE_ANCS_				
		max_frame_num = NOTIFIC_get_message_total();		
#else 		
		max_frame_num = 0;
#endif		
		if (max_frame_num == 0) {
			notific->app_notific_index = 0;
			return;
		}

		if (gesture == TOUCH_SWIPE_LEFT) 
			b_up = TRUE;
		else if (gesture == TOUCH_SWIPE_RIGHT) 
			b_up = FALSE;
		else
			return;
			
		// Update App notific index
		if (b_up) {
			notific->app_notific_index++;
			
			if (notific->app_notific_index >= max_frame_num) {
				notific->app_notific_index = 0;
			}
			
		} else {
			if (notific->app_notific_index == 0) {
				notific->app_notific_index = max_frame_num - 1;
			} else {
				notific->app_notific_index --;
			}
		}
	} 
}
#endif

/*------------------------------------------------------------------------------------------
*  Function:	_update_ppg_switch_control(UI_ANIMATION_CTX *u)
*
*  Description: Control ppg according current display page. 
*
*------------------------------------------------------------------------------------------*/
#ifdef _ENABLE_PPG_	
static void _update_ppg_switch_control(UI_ANIMATION_CTX *u)
{
	BOOLEAN b_ppg_switch_open = FALSE;
	TRACKING_CTX *a = &cling.activity;

	// Initialize PPG approximation detection with a swipe
#ifdef _CLINGBAND_PACE_MODEL_		
	if ((u->frame_index == UI_DISPLAY_VITAL_HEART_RATE) || (u->frame_index == UI_DISPLAY_TRAINING_STAT_HEART_RATE)) {
		
	  b_ppg_switch_open = TRUE;
	}
#endif
	
	// Initialize PPG approximation detection with a swipe
#ifndef _CLINGBAND_PACE_MODEL_		
	if ((u->frame_index == UI_DISPLAY_VITAL_HEART_RATE) 
		||(u->frame_index == UI_DISPLAY_TRAINING_STAT_HEART_RATE)
		||(u->frame_index == UI_DISPLAY_WORKOUT_RT_HEART_RATE)	
		||(u->frame_index == UI_DISPLAY_CYCLING_OUTDOOR_STAT_HEART_RATE)) {
			
			 b_ppg_switch_open = TRUE;
		}
#endif		
		
	// Close PPG when batt is charging.	
	if (BATT_is_charging()) {
		PPG_disable_sensor();
		cling.hr.b_closing_to_skin = FALSE;
		cling.hr.heart_rate_ready  = FALSE;
		cling.hr.current_rate = 0;
		return;
	} 
		
	if (b_ppg_switch_open) {
		PPG_closing_to_skin_detect_init();
	}	else {
		if (!a->b_workout_active) {
			cling.hr.heart_rate_ready  = FALSE;
			PPG_disable_sensor();
			cling.hr.state = PPG_STAT_DUTY_OFF;
		}
	}	
}
#endif	

/*------------------------------------------------------------------------------------------
*  Function:	_update_workout_active_control(UI_ANIMATION_CTX *u)
*
*  Description: Workout active mode master control. 
*
*------------------------------------------------------------------------------------------*/
static void _update_workout_active_control(UI_ANIMATION_CTX *u)
{
  UI_RUNNING_INFO_CTX *running_info = &cling.ui.running_info;
	I32U t_curr = CLK_get_system_time();	
	BOOLEAN b_enter_active_mode = FALSE;
	TRACKING_CTX *a = &cling.activity;
	
#ifdef _CLINGBAND_PACE_MODEL_		
	if ((u->frame_prev_idx == UI_DISPLAY_TRAINING_STAT_START) && (u->frame_index == UI_DISPLAY_TRAINING_STAT_CONNECT_GPS)) {
		Y_SPRINTF("[UI] Enter outdoor running mode");	
		a->workout_type = WORKOUT_RUN_OUTDOOR;		
		b_enter_active_mode = TRUE;
	}
#endif
		
#ifndef _CLINGBAND_PACE_MODEL_		
	if ((u->frame_prev_idx == UI_DISPLAY_TRAINING_STAT_START) || (u->frame_prev_idx == UI_DISPLAY_TRAINING_STAT_START_OR_ANALYSIS)) {	
	  if (u->frame_index == UI_DISPLAY_TRAINING_STAT_READY) {
		  Y_SPRINTF("[UI] Enter outdoor running mode");	
		  a->workout_type = WORKOUT_RUN_OUTDOOR;		
		  b_enter_active_mode = TRUE;			
		}
	}
#endif
	
#ifndef _CLINGBAND_PACE_MODEL_		
	if ((u->frame_prev_idx >= UI_DISPLAY_WORKOUT_TREADMILL) && (u->frame_prev_idx <= UI_DISPLAY_WORKOUT_OTHERS)) {
		if (u->frame_index == UI_DISPLAY_WORKOUT_RT_READY) {
		  b_enter_active_mode = TRUE;			
			N_SPRINTF("[UI] Enter normal workout mode");	
			if (u->frame_prev_idx == UI_DISPLAY_WORKOUT_TREADMILL)
		    a->workout_type = WORKOUT_TREADMILL_INDOOR;
			else if (u->frame_prev_idx == UI_DISPLAY_WORKOUT_CYCLING)
		    a->workout_type = WORKOUT_CYCLING_INDOOR;
			else if (u->frame_prev_idx == UI_DISPLAY_WORKOUT_STAIRS)
		    a->workout_type = WORKOUT_STAIRS_INDOOR;			
			else if (u->frame_prev_idx == UI_DISPLAY_WORKOUT_ELLIPTICAL)
		    a->workout_type = WORKOUT_ELLIPTICAL_INDOOR;	
			else if (u->frame_prev_idx == UI_DISPLAY_WORKOUT_ROW)
		    a->workout_type = WORKOUT_ROWING;	
			else if (u->frame_prev_idx == UI_DISPLAY_WORKOUT_AEROBIC)
		    a->workout_type = WORKOUT_AEROBIC;		
			else if (u->frame_prev_idx == UI_DISPLAY_WORKOUT_PILOXING)
		    a->workout_type = WORKOUT_PILOXING;		
			else if (u->frame_prev_idx == UI_DISPLAY_WORKOUT_OTHERS)
		    a->workout_type = WORKOUT_OTHER;		
      else			
				a->workout_type = WORKOUT_OTHER;
		}
	}

	if ((u->frame_prev_idx == UI_DISPLAY_CYCLING_OUTDOOR_STAT_START) && (u->frame_index == UI_DISPLAY_CYCLING_OUTDOOR_STAT_READY)) {
		N_SPRINTF("[UI] Enter CYCLING_OUTDOOR workout mode");	
		a->workout_type = WORKOUT_CYCLING_OUTDOOR;		
		b_enter_active_mode = TRUE;		
	}
#endif
	
	if (b_enter_active_mode) {
		running_info->ready_idx = 0;
		running_info->t_ready_stamp = 0;
		running_info->time_start_in_ms = 0;

	  // Reset all training data - distance, time stamp, calories, and session ID
	  cling.train_stat.distance = 0;
	  cling.train_stat.time_start_in_ms = t_curr;
	  cling.train_stat.session_id = cling.time.time_since_1970;
	  cling.train_stat.calories = 0;
#ifdef __YLF_STRIDE__
		GPS_distance_Init();
#endif

#ifndef _CLINGBAND_PACE_MODEL_
//#ifndef __YLF_CYCLING__
		cling.train_stat.cycling_curr_distance = 0;
		cling.train_stat.cycling_pre_distance = 0;
		cling.train_stat.b_cycling_state = FALSE;
		cling.train_stat.cycling_pre_delta_distance = 0;
#endif
	  // Running pace time stamp
	  cling.run_stat.pace_calc_ts = CLK_get_system_time();
	  cling.run_stat.last_10sec_distance = 0;
	  cling.run_stat.last_10sec_pace_min = 0;
	  cling.run_stat.last_10sec_pace_sec = 0;
#ifdef __YLF__
	  //reset the run_stat
	  cling.run_stat.calories = 0;
	  cling.run_stat.distance = 0;
	  cling.run_stat.steps = 0;
	  cling.run_stat.time_min = 0;
		cling.run_stat.time_sec = 0;
	  cling.run_stat.accu_heart_rate = 0;
#endif
	  cling.run_stat.pace_buf_idx = 0;
	  for (I8U i = 0; i < PACE_BUF_LENGTH; i++) {
		cling.run_stat.last_t_buf[i] = 0xffff;
		cling.run_stat.last_d_buf[i] = 0xffff;
	  }
			
#ifdef _CLINGBAND_PACE_MODEL_		
		// Reset connect gps time stamp.
		cling.ui.pace_private.conn_gps_t_stamp = CLK_get_system_time();
		// Clear app positon service status.
		cling.train_stat.app_positon_service_status = POSITION_NO_SERVICE;
#endif
		
		if (BTLE_is_connected()) {
			if ((a->workout_type == WORKOUT_RUN_OUTDOOR) || 
					(a->workout_type == WORKOUT_CYCLING_OUTDOOR))
			{
				Y_SPRINTF("[UI] cp create workout rt msg");	
				CP_create_workout_rt_msg(a->workout_type);
			}
		}	

    return;		
	}
	
#ifndef _CLINGBAND_PACE_MODEL_	
  if (_is_regular_page(u->frame_index) || _is_running_analysis_page(u->frame_index) || _is_other_need_store_page(u->frame_index)) {
		a->b_workout_active = FALSE;		
		a->workout_type = WORKOUT_NONE;
	}
#endif

#ifdef _CLINGBAND_PACE_MODEL_		
	if (_is_regular_page(u->frame_index) || _is_running_analysis_page(u->frame_index)) {
		a->b_workout_active = FALSE;		
		a->workout_type = WORKOUT_NONE;
	} 
#endif
}

/*------------------------------------------------------------------------------------------
*  Function:	_update_vibrator_control(UI_ANIMATION_CTX *u)
*
*  Description: Include vibrator and reminder and notification control. 
*
*------------------------------------------------------------------------------------------*/
static void _update_vibrator_control(UI_ANIMATION_CTX *u)
{
	USER_PROFILE_CTX *p = &cling.user_data.profile;
	if (_is_smart_incoming_notifying_page(u->frame_index)) {
	  // Stop notification
		NOTIFIC_stop_notifying();
	} 

	// If a valid gesture is detected, vibrate it for interaction
	if (p->touch_vibration) {
		GPIO_vibrator_on_block(UI_TOUCH_VIBRATION_ON_TIME_IN_MS);
		GPIO_vibrator_set(FALSE);
	}	
}

/*------------------------------------------------------------------------------------------
*  Function:	_update_phone_finder_control(UI_ANIMATION_CTX *u, I8U gesture)
*
*  Description: Control send phone finder message to the App. 
*
*------------------------------------------------------------------------------------------*/
#ifdef _CLINGBAND_VOC_MODEL_ 
static void _update_phone_finder_control(UI_ANIMATION_CTX *u, I8U gesture)
{
	if (gesture != TOUCH_FINGER_MIDDLE)
		return;
	
	// If UI lands on phone finder page, send out message to the phone
	if ((u->frame_prev_idx == UI_DISPLAY_CAROUSEL_4) && (u->frame_index == UI_DISPLAY_SMART_PHONE_FINDER)) {
		// Send phone finder message to the App
		CP_create_phone_finder_msg();
	}
}
#endif

/*------------------------------------------------------------------------------------------
*  Function:	_update_music_control(UI_ANIMATION_CTX *u, I8U gesture)
*
*  Description: Music control. 
*
*------------------------------------------------------------------------------------------*/
#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
static void _update_music_control(UI_ANIMATION_CTX *u, I8U gesture)
{
#ifdef __PLAYER_CONTROLLER_ENABLE__
	CLASS(PlayerController)* media_player_obj_p = PlayerController_get_instance();
	
	if ((u->frame_index < UI_DISPLAY_MUSIC) || (u->frame_index > UI_DISPLAY_MUSIC_END))
		return;
	
	if (u->frame_index == UI_DISPLAY_MUSIC_PLAY) {
		if (gesture == TOUCH_FINGER_MIDDLE) {
			media_player_obj_p->mute(media_player_obj_p);
			N_SPRINTF("[UI] Play mute");
		} else if (gesture == TOUCH_FINGER_LEFT) {
			media_player_obj_p->play_pause(media_player_obj_p);
			N_SPRINTF("[UI] Play pause");
		}
	} else if (u->frame_index == UI_DISPLAY_MUSIC_VOLUME) {
		if (gesture == TOUCH_FINGER_MIDDLE) {
			media_player_obj_p->volume_up(media_player_obj_p);
			N_SPRINTF("[UI] Play UP UP");
		} else if (gesture == TOUCH_FINGER_LEFT) {
			media_player_obj_p->volume_down(media_player_obj_p);
			N_SPRINTF("[UI] Play Down Down");
		}
	} else if (u->frame_index == UI_DISPLAY_MUSIC_SONG) {
		if (gesture == TOUCH_FINGER_MIDDLE) {
			media_player_obj_p->next_track(media_player_obj_p);
			N_SPRINTF("[UI] Play Next");
		} else if (gesture == TOUCH_FINGER_LEFT) {
			media_player_obj_p->prev_track(media_player_obj_p);
			N_SPRINTF("[UI] Play Previous");
		}
	}
#endif
}
#endif

#ifdef _CLINGBAND_2_PAY_MODEL_
static int get_balance(uint32_t bus_balance, uint32_t shande_balance){
	UI_BALANCE_CTX *balance = &cling.ui.balance;

	balance->bus_card_balance = bus_balance;
	balance->bank_card_balance = shande_balance;
	return 1;
}

/*------------------------------------------------------------------------------------------
*  Function:	_update_get_bus_and_bank_card_balance()
*
*  Description: Update balance display control. 
*
*------------------------------------------------------------------------------------------*/
static void _update_get_bus_and_bank_card_balance(UI_ANIMATION_CTX *u)
{
	if (u->frame_prev_idx == UI_DISPLAY_CAROUSEL_4) {
		
		if ((u->frame_index == UI_DISPLAY_PAY_BUS_CARD_BALANCE_ENQUIRY) || (u->frame_index == UI_DISPLAY_PAY_BANK_CARD_BALANCE_ENQUIRY)) {
	    
			CLASS(cling_pay_app)*p =cling_pay_app_get_instance();		
			p->get_balance(p, get_balance);	
		}
	}
}
#endif

/*------------------------------------------------------------------------------------------
*  Function:	_update_stopwatch_operation_control(UI_ANIMATION_CTX *u, I8U gesture)
*
*  Description: Stopwatch control. 
*
*------------------------------------------------------------------------------------------*/
#ifndef _CLINGBAND_PACE_MODEL_		
static void _update_stopwatch_operation_control(UI_ANIMATION_CTX *u, I8U gesture)
{
	BOOLEAN b_exit_stopwatch_mode = FALSE;
  UI_STOPWATCH_CTX	*stopwatch = &cling.ui.stopwatch;
	
	// 1. Start stopwatch
	if ((u->frame_prev_idx == UI_DISPLAY_CAROUSEL_2) && (u->frame_index == UI_DISPLAY_STOPWATCH_START)) {
		stopwatch->t_start_stamp = 0;
		stopwatch->t_stop_stamp = 0;
		stopwatch->b_without_exit_flag = TRUE;
    stopwatch->b_in_pause_mode = FALSE;		
    return;		
	}

	// 2. Pause stopwatch
	if ((u->frame_index == UI_DISPLAY_STOPWATCH_START) && (gesture == TOUCH_FINGER_RIGHT)) {
		if (stopwatch->b_in_pause_mode) 
		  stopwatch->b_in_pause_mode = FALSE;
		else
			stopwatch->b_in_pause_mode = TRUE;
	}
		
  // 3. Exit stopwatch
	if (_is_regular_page(u->frame_index) || _is_running_active_mode_page(u->frame_index) || _is_running_analysis_page(u->frame_index)) {
		b_exit_stopwatch_mode = TRUE;
	} 
	
	if ((u->frame_prev_idx == UI_DISPLAY_STOPWATCH_STOP) && (u->frame_index == UI_DISPLAY_CAROUSEL_2)) {
		b_exit_stopwatch_mode = TRUE;
	}
	
	if (b_exit_stopwatch_mode) {
		stopwatch->t_start_stamp = 0;
		stopwatch->t_stop_stamp = 0;
	  stopwatch->b_without_exit_flag = FALSE;		
	}
}
#endif	

/*------------------------------------------------------------------------------------------
*  Function:	_update_all_feature_switch_control(UI_ANIMATION_CTX *u,  const I8U *p_matrix, I8U gesture)
*
*  Description: All feature control. 
*
*------------------------------------------------------------------------------------------*/
static void _update_all_feature_switch_control(UI_ANIMATION_CTX *u, const I8U *p_matrix, I8U gesture)
{
	// 1. Update next frame index.		
	_update_frame_index(u, p_matrix, gesture);
	
	// 2. Enter or exit workout active mode.
	_update_workout_active_control(u);	

	// 3. Switch to next vertical page.
	_update_vertical_detail_page(u, gesture);

#ifndef _CLINGBAND_PACE_MODEL_	
	// 4. Update alarm clock reminder index and display control.	
	_update_alarm_clock_control(u, gesture);
#endif

	// 5. Open or close PPG According to the current frame.
#ifdef _ENABLE_PPG_	
	_update_ppg_switch_control(u);
#endif
	
	// 6. Update incoming message detail index.
	_update_message_detail_index(u);
	
	// 7. Cheak notification weather look finished.
	_update_notif_repeat_look_time(u->frame_index);

  // 8. Update notifying switch control.
  _update_vibrator_control(u);
	
#ifndef _CLINGBAND_PACE_MODEL_		
	// 9. Update app notification display index.
	_update_horizontal_app_notific_index(u, gesture);
	
	// 10.Stopwatch control.
	_update_stopwatch_operation_control(u, gesture);
#endif

#ifdef _CLINGBAND_VOC_MODEL_ 
	// 11. Phone finder control.
	_update_phone_finder_control(u, gesture);
#endif

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_VOC_MODEL_)	
	// 12. Music control.
	_update_music_control(u, gesture);
#endif

#ifdef _CLINGBAND_2_PAY_MODEL_
	// 13. Update balance display.
	_update_get_bus_and_bank_card_balance(u);
#endif	
}

/*------------------------------------------------------------------------------------------
*  Function:	_perform_ui_with_touch_finger(UI_ANIMATION_CTX *u, I8U gesture)
*
*  Description: Perform the corresponding action when user use finger click on the touch.
*
*------------------------------------------------------------------------------------------*/
#ifndef _CLINGBAND_PACE_MODEL_			
static void _perform_ui_with_a_finger_touch(UI_ANIMATION_CTX *u, I8U gesture)
{
	const I8U *p_matrix = NULL;

	// Make sure current UI frame supports finger touch
	if (ui_gesture_constrain[u->frame_index] & UFG_FINGER_IRIS) {
		
		// Get finger touch next frame matrix.
		if (gesture == TOUCH_FINGER_MIDDLE) {
		  p_matrix = ui_matrix_finger_middle;
		} else if (gesture == TOUCH_FINGER_LEFT) {
		  p_matrix = ui_matrix_finger_left;			
		} else if (gesture == TOUCH_FINGER_RIGHT) {
			p_matrix = ui_matrix_finger_right;
		} else {
			return;
		}

		// Set current frame animation mode.
		_set_animation_with_a_finger_touch(u->frame_index, gesture);
		
		// Update all control.
    _update_all_feature_switch_control(u, p_matrix, gesture);
		
		N_SPRINTF("[UI] finger touch: %d, %d, %d", u->frame_prev_idx, u->frame_index, u->frame_next_idx);			
	} else {
		// Animation is not needed
		u->frame_next_idx = u->frame_index;
		// No further process
		return;
	}
}
#endif

/*------------------------------------------------------------------------------------------
*  Function:	_perform_ui_with_a_swipe_touch(UI_ANIMATION_CTX *u, I8U gesture)
*
*  Description: Perform the corresponding action when user use swipe touch.
*
*------------------------------------------------------------------------------------------*/
#ifndef _CLINGBAND_PACE_MODEL_			
static void _perform_ui_with_a_swipe_touch(UI_ANIMATION_CTX *u, I8U gesture)
{
	const I8U *p_matrix = NULL;

	// Make sure the UI frame can take swipe gesture.
	if (ui_gesture_constrain[u->frame_index] & UFG_SWIPE_PANNING) {
		
		// Get swipe touch next frame matrix.
		if (gesture == TOUCH_SWIPE_LEFT) {
		  p_matrix = ui_matrix_swipe_left;
		} else if (gesture == TOUCH_SWIPE_RIGHT) {
		  p_matrix = ui_matrix_swipe_right;			
		} else {
			return;
		}

		// Set current frame animation mode.
		_set_animation_with_a_swipe_touch(u->frame_index, gesture);
		
		// Update all control.
    _update_all_feature_switch_control(u, p_matrix, gesture);
		
		UI_SPRINTF("[UI] swipe left: %d, %d, %d", u->frame_prev_idx, u->frame_index, u->frame_next_idx);			
	} else {
		// Animation is not needed
		u->frame_next_idx = u->frame_index;
		// No further process
		return;
	}
}
#endif

/*------------------------------------------------------------------------------------------
*  Function:	_perform_ui_with_button_single(UI_ANIMATION_CTX *u, I8U gesture)
*
*  Description: Perform the corresponding action when user click on the touch button.
*
*------------------------------------------------------------------------------------------*/
static void _perform_ui_with_button_single(UI_ANIMATION_CTX *u, I8U gesture)
{
	const I8U *p_matrix = NULL;

	// Make sure the UI frame can take button click gesture.
	if (ui_gesture_constrain[u->frame_index] & UFG_BUTTON_SINGLE) {

		// Get next button click frame matrix.
		p_matrix = ui_matrix_button_single;

		// Set current frame animation mode.
		_set_animation_with_button_single(u->frame_index);

		// Update all control.
    _update_all_feature_switch_control(u, p_matrix, gesture);
		
		UI_SPRINTF("[UI] button single: %d, %d, %d", u->frame_prev_idx, u->frame_index, u->frame_next_idx);			
	} else {
		// Animation is not needed
		u->frame_next_idx = u->frame_index;
		// No further process
		return;
	}
}

/*------------------------------------------------------------------------------------------
*  Function:	_perform_ui_with_button_press_hold(UI_ANIMATION_CTX *u, I8U gesture)
*
*  Description: Perform the corresponding action when user press hold the touch button.
*
*------------------------------------------------------------------------------------------*/
#ifdef _CLINGBAND_PACE_MODEL_		
static void _perform_ui_with_button_press_hold(UI_ANIMATION_CTX *u, I8U gesture)
{
	const I8U *p_matrix = NULL;

	// Make sure the UI frame can take botton hold gesture.
	if (ui_gesture_constrain[u->frame_index] & UFG_BUTTON_HOLD) {
		// Get next button hold frame matrix.			
		p_matrix = ui_matrix_button_hold;

		// Set current frame animation mode.
		_set_animation_with_button_hold(u->frame_index);		
			
		// Update all control.
    _update_all_feature_switch_control(u, p_matrix, gesture);

		UI_SPRINTF("[UI] botton hold: %d, %d, %d", u->frame_prev_idx, u->frame_index, u->frame_next_idx);			
	} else {
		// Animation is not needed
		u->frame_next_idx = u->frame_index;
		// No further process
		return;
	}
}
#endif

/*------------------------------------------------------------------------------------------
*  Function:	_perform_ui_with_button_press_sos(UI_ANIMATION_CTX *u, I8U gesture)
*
*  Description: Display sos page.
*
*------------------------------------------------------------------------------------------*/
#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)	
static void _perform_ui_with_button_press_sos(UI_ANIMATION_CTX *u)
{	
  u->frame_index = UI_DISPLAY_SMART_SOS_ALERT;
  u->frame_next_idx = u->frame_index;
  u->touch_time_stamp = CLK_get_system_time();
}
#endif

/*------------------------------------------------------------------------------------------
*  Function:	_ui_touch_sensing()
*
*  Description: TOUCH button action master control.
*
*------------------------------------------------------------------------------------------*/
#ifndef _CLINGBAND_PACE_MODEL_		
static I8U _ui_touch_sensing()
{
	UI_ANIMATION_CTX *u = &cling.ui;
	I8U gesture;
	
	gesture = TOUCH_get_gesture_panel();

	// If user light up screen by touch button, Do not perform any action for the first time.
	if ((gesture != TOUCH_NONE) && (u->b_invalid_touch_action)) {
		u->b_invalid_touch_action = FALSE;
		u->frame_next_idx = u->frame_index;
		return FALSE;
	}
	
	// UI state switching based on valid gesture from Cypress IC
	switch (gesture) {
		case TOUCH_NONE:
			break;
		case TOUCH_FINGER_LEFT:
		{
			_perform_ui_with_a_finger_touch(u, gesture);
			break;
		}				
		case TOUCH_FINGER_MIDDLE:
		{
			_perform_ui_with_a_finger_touch(u, gesture);
			break;
		}	
		case TOUCH_FINGER_RIGHT:
		{
			_perform_ui_with_a_finger_touch(u, gesture);
			break;
		}		
		case TOUCH_SWIPE_LEFT:
		{
			_perform_ui_with_a_swipe_touch(u, gesture);
			break;
		}				
		case TOUCH_SWIPE_RIGHT:
		{
			_perform_ui_with_a_swipe_touch(u, gesture);
			break;
		}		
		case TOUCH_BUTTON_SINGLE:
		{
			_perform_ui_with_button_single(u, gesture);
			break;
		}			
#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)		
		case TOUCH_BUTTON_PRESS_SOS:
		{
			_perform_ui_with_button_press_sos(u);
			break;
		}		
#endif		
		default:
		{
			gesture = TOUCH_NONE;
			break;
		}
	}
	
	return gesture;
}
#endif

#ifdef _CLINGBAND_PACE_MODEL_		
static I8U _ui_touch_sensing()
{
	UI_ANIMATION_CTX *u = &cling.ui;
	I8U gesture;
	
	gesture = HOMEKEY_get_gesture_panel();

	// If user light up screen by touch button, Do not perform any action for the first time.
	if ((gesture != HOMEKEY_BUTTON_NONE) && (u->b_invalid_touch_action)) {
		u->b_invalid_touch_action = FALSE;
		u->frame_next_idx = u->frame_index;
		return FALSE;
	}
	
	// UI state switching based on valid gesture from Cypress IC
	switch (gesture) {
		case HOMEKEY_BUTTON_NONE:
			break;
		case HOMEKEY_BUTTON_SINGLE:
		{
			_perform_ui_with_button_single(u, gesture);
			break;
		}		
		case HOMEKEY_BUTTON_HOLD:
		{
			_perform_ui_with_button_press_hold(u, gesture);
			break;
		}
		default:
		{
			gesture = HOMEKEY_BUTTON_NONE;
			break;
		}
	}

	return gesture;
}
#endif

/*----------------------------------------------------------------------------------
*  Function:	UI_start_notifying(I8U frame_index)
*
*  Description: Display received notification.
*
*  New features:Change the way of notifying.
*
*----------------------------------------------------------------------------------*/
void UI_start_notifying(I8U frame_index)
{
	UI_ANIMATION_CTX *u = &cling.ui;
  UI_NOTIFICC_CTX *notific = &cling.ui.notific;
	TRACKING_CTX *a = &cling.activity;
	
	I32U t_curr = CLK_get_system_time(); 
	// 1. Start system timer.
  RTC_start_operation_clk();
	
	// 2. Update display frame index
	u->frame_index = frame_index;

	// 3. Update notif time stamp.
	notific->t_stamp = t_curr;

	// 4. Turn on oled display.
	UI_turn_on_display(UI_STATE_TOUCH_SENSING);
	
	// 5. Store current incoming message(if not in running active mode).
	if (_is_smart_incoming_notifying_page(u->frame_index) && (!a->b_workout_active)) {
		notific->b_need_stored = TRUE;		
	} else {
		notific->b_need_stored = FALSE;
  }
	
	// 6. Update running alarm flag
  if ((u->frame_index == UI_DISPLAY_TRAINING_STAT_PACE) || (u->frame_index == UI_DISPLAY_TRAINING_STAT_HEART_RATE)) {
	  u->b_in_running_alert_page = TRUE;
	} else {
		u->b_in_running_alert_page = FALSE;
	}
}

/*------------------------------------------------------------------------------------------
*  Function:	UI_switch_state(I8U state, I32U interval)
*
*  Description: Used to switch ui state..
*
*------------------------------------------------------------------------------------------*/
void UI_switch_state(I8U state, I32U interval)
{
	UI_ANIMATION_CTX *u = &cling.ui;

  // 1. Switch ui state	
	u->state = state;
	
	// 2. Set frame display interval
	u->frame_interval = interval;
	
	// 3. Update display base time satmp
	u->display_to_base = CLK_get_system_time();
	
	N_SPRINTF("[UI] state switching: %d, %d, %d, %d", state, u->display_to_base, u->frame_index, u->frame_cached_index);
}

/*------------------------------------------------------------------------------------------
*  Function:	UI_turn_on_display(UI_ANIMATION_STATE state, I32U time_offset)
*
*  Description: Initialize the UI.
*
*------------------------------------------------------------------------------------------*/
BOOLEAN UI_turn_on_display(UI_ANIMATION_STATE state)
{
	UI_ANIMATION_CTX *u = &cling.ui;	
  BOOLEAN b_panel_on = FALSE;;
	
	// 1. Turn on OLED panel	
	b_panel_on = OLED_set_panel_on();

	// 2. Update touch stamp time 
	u->touch_time_stamp = CLK_get_system_time();
	
	// 3. Switch state
	if (!OLED_panel_is_turn_on()) {
		UI_switch_state(UI_STATE_HOME, 0);
	} else {
		if ((u->state != UI_STATE_CHARGING_GLANCE) && (u->state != UI_STATE_HOME) && (u->state != UI_STATE_CLING_START)) {
		  UI_switch_state(state, 0);
		}
	}

	N_SPRINTF("[UI] turn on display :%d, :%d", b_panel_on, cling.oled.state);
	return b_panel_on;
}

/*------------------------------------------------------------------------------------------
*  Function:	UI_init()
*
*  Description: Initialize the UI.
*
*------------------------------------------------------------------------------------------*/
void UI_init()
{
	// Turn on OLED panel
	OLED_set_panel_on();

	// UI initial state
	UI_switch_state(UI_STATE_CLING_START, 3000);		
}

/*------------------------------------------------------------------------------------------
*  Function:	UI_state_machine()
*
*  Description: 
*
*------------------------------------------------------------------------------------------*/
void UI_state_machine()
{
	UI_ANIMATION_CTX *u = &cling.ui;
	I32U t_curr, t_diff, t_threshold;
	TRACKING_CTX *a = &cling.activity;
	
	t_curr = CLK_get_system_time();

	// Don't do anything if oled is turn off.
	if (!OLED_panel_is_turn_on()) 
		return;
	
	// Only display ota if system is updating firmware
	if (OTA_if_enabled()) 
		u->state = UI_STATE_FIRMWARE_OTA;

	// Only display unauthorized page if is unauthorized device.
	if (!LINK_is_authorized()) 
		u->state = UI_STATE_AUTHORIZATION;
	
	N_SPRINTF("[UI] new active state: %d @ %d", u->state, CLK_get_system_time());	

	switch (u->state) {
		case UI_STATE_IDLE:
			break;
		case UI_STATE_CLING_START:
		{
			UI_frame_display_appear(UI_DISPLAY_SYSTEM_RESTART, TRUE);	
			if (t_curr > (u->display_to_base + 3000)) {
				u->touch_time_stamp = t_curr;								
				u->frame_index = UI_DISPLAY_HOME;	
				UI_switch_state(UI_STATE_APPEAR, 0);
			}
			break;
		}
		case UI_STATE_HOME:
		{
      if (BATT_is_low_battery() && (!BATT_is_charging())) {
			  UI_frame_display_appear(UI_DISPLAY_SYSTEM_BATT_POWER, TRUE);			
				UI_switch_state(UI_STATE_CHARGING_GLANCE, 0);				
			} else if (BATT_is_charging()) {
			  UI_frame_display_appear(UI_DISPLAY_SYSTEM_BATT_POWER, TRUE);									
				UI_switch_state(UI_STATE_CHARGING_GLANCE, 0);							
			} else {
				UI_switch_state(UI_STATE_TOUCH_SENSING, 0);						
			}
	  	break;
		}
		case UI_STATE_CHARGING_GLANCE:
		{
			N_SPRINTF("[UI] LOW BATTERY(or charging): %d", cling.system.mcu_reg[REGISTER_MCU_BATTERY]);			
			UI_frame_display_appear(UI_DISPLAY_SYSTEM_BATT_POWER, TRUE);				
			if (t_curr > u->display_to_base + 1500) {
			  // Exit running mode.			
			  u->frame_index = UI_DISPLAY_HOME;	
			  a->workout_type = WORKOUT_NONE;
			  a->b_workout_active = FALSE;	
				u->touch_time_stamp = t_curr;
				UI_switch_state(UI_STATE_TOUCH_SENSING, 0);				
			}
			break;
		}
		case UI_STATE_AUTHORIZATION:
		{
			if (LINK_is_authorizing()) {
				u->frame_index = UI_DISPLAY_SYSTEM_LINKING;
				t_diff = t_curr - cling.link.link_ts;
				if (t_diff > 30000) {
					cling.link.b_authorizing = FALSE;
				}
			} else if (!LINK_is_authorized()) {
				u->frame_index = UI_DISPLAY_SYSTEM_UNAUTHORIZED;	
			}	else {
				u->touch_time_stamp = t_curr;
				u->frame_index = UI_DISPLAY_HOME;
	      UI_switch_state(UI_STATE_TOUCH_SENSING, 0);
			}

      if (t_curr > u->display_to_base + 300) {	
				u->display_to_base = t_curr;
				UI_frame_display_appear(u->frame_index, TRUE);	
#ifdef _CLINGBAND_PACE_MODEL_					
        if (u->pace_private.b_pace_vibration_restart) {		
					u->pace_private.pace_restart_vibration_time++;
					if (u->pace_private.pace_restart_vibration_time < 5) {
				    GPIO_vibrator_on_block(50);
					} else {
						u->pace_private.b_pace_vibration_restart = FALSE;
					}
				}
#endif				
			}
			break;
		}
		case UI_STATE_FIRMWARE_OTA:
		{
			if (!OTA_if_enabled()) {
				u->frame_index = UI_DISPLAY_HOME; 
				UI_switch_state(UI_STATE_TOUCH_SENSING, 0);
			} else {
				if (t_curr > (u->display_to_base+1000)) {
					u->display_to_base = t_curr;
					UI_frame_display_appear(UI_DISPLAY_SYSTEM_OTA, TRUE);			
				}		
			}
		  break;
		}
		case UI_STATE_TOUCH_SENSING:
		{
      if (_ui_touch_sensing()) {
				N_SPRINTF("[UI] new gesture --- %d", t_curr);
				u->touch_time_stamp = t_curr;
				// Record notification time stamp.
			} else if (t_curr > u->display_to_base + u->frame_interval) {
				N_SPRINTF("[UI] Go blinking the icon: %d", u->frame_index);
				UI_switch_state(UI_STATE_APPEAR, 0);
			} 
	
			// 4 seconds screen dark expiration
			// for heart rate measuring, double the Screen ON time.
			if (u->frame_index == UI_DISPLAY_VITAL_HEART_RATE) {		
				t_threshold = cling.user_data.screen_on_heart_rate; // in second				
			} else {
#ifdef __YLF_WRIST_FLIP__
			if(a->b_screen_on_wrist_flip && (!a->b_screen_off_wrist_flip))
				 t_threshold = 3;
			else
			  t_threshold = cling.user_data.screen_on_general; // in second
#else
				t_threshold = cling.user_data.screen_on_general; // in second
#endif
			}
			
			if ((!t_threshold) || (t_threshold == 0xff))
#ifdef __YLF_WRIST_FLIP__
				t_threshold = 3;
#else
				t_threshold = 4;
#endif
				
			t_threshold *= 1000; // second -> milli-second
	
			if (_is_smart_incoming_notifying_page(u->frame_index)) {
				// 1. Turn on display when the motor is vibrating.
				if (!NOTIFIC_is_idle()) {
					u->touch_time_stamp = t_curr;		
					u->notific.t_stamp = t_curr;			
				}	else {
				  if (t_curr > (u->notific.t_stamp + UI_STORE_NOTIFICATION_MAX_TIME_IN_MS)) {
            // 2. Filtering the old information, go back to previous UI page.						
			      u->frame_index = UI_DISPLAY_PREVIOUS;		
					  u->notific.b_need_stored = FALSE;	
		      } else if (t_curr > u->touch_time_stamp + 15000) {
            // 3. Go back to previous UI page.						
				    u->frame_index = UI_DISPLAY_PREVIOUS;		
					  u->notific.b_need_stored = FALSE;					
				  }			
			  }
		  }
			
			if ((u->b_in_running_alert_page) && (a->b_workout_active)) {
        if (t_curr > u->touch_time_stamp + 10000) {
					// Go back to previous UI page.						
					u->frame_index = UI_DISPLAY_PREVIOUS;		
					u->b_in_running_alert_page = FALSE;					
				}						
			}
					
#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)	
			if (u->frame_index == UI_DISPLAY_SMART_SOS_ALERT) {
				t_threshold = 10000;
			}
#endif
			
			// If we don't see any gesture in 4 seconds, dark out screen
			if (t_curr > (u->touch_time_stamp+t_threshold)) {
				
				if (cling.user_data.b_running_alwayson) {
#ifdef _CLINGBAND_PACE_MODEL_							
					if (!a->b_workout_active) {
#else 
					if ((!a->b_workout_active) && (!u->stopwatch.b_without_exit_flag)) {
#endif						
						N_SPRINTF("[UI] gesture monitor time out 1 - %d at %d", t_threshold, t_curr);
						u->state = UI_STATE_DARK;
					}
				} else {
					N_SPRINTF("[UI] gesture monitor time out 1 - %d at %d", t_threshold, t_curr);
					u->state = UI_STATE_DARK;					
				}
			}
			break;
		}
		case UI_STATE_ANIMATING:
		{
			// perform frame animation before a final apperance
			UI_frame_animating();
			break;
		}
		case UI_STATE_APPEAR:
		{
			N_SPRINTF("[UI] appear time: %d, index: %d", t_curr, u->frame_index);
			
			// 1. Restore perv frame index
      if (u->frame_index == UI_DISPLAY_PREVIOUS) {		
				
			  _restore_perv_frame_index();
				
				NOTIFIC_stop_notifying();
			}
			
			// 2. Display current page
			UI_frame_display_appear(u->frame_index, TRUE);
			
      // 3. Store frame cached index			
      _stote_frame_cached_index();	
			
			// 4. Get current page blinking interval.			
      UI_switch_state(UI_STATE_TOUCH_SENSING, ui_matrix_blinking_interval[u->frame_index]);
			break;
		}
		case UI_STATE_DARK:
		{
			UI_SPRINTF("[UI] screen go dark - %d, %d", u->frame_index, u->frame_cached_index);

			// 1. Turn off OLED panel
			OLED_set_panel_off();

			// 2. Switch state to idle
			u->state = UI_STATE_IDLE;

			// 3. Update dark time stamp
			u->dark_time_stamp = t_curr;

			// 4. Clead detail flag
			u->b_detail_page = FALSE;		
	
			// 5. Go back to previous UI page
	    if (!u->notific.b_need_stored)
			  u->frame_index = UI_DISPLAY_PREVIOUS;						
	
			// 6. Update touch power control
#ifdef _ENABLE_TOUCH_				
			if (cling.lps.b_low_power_mode) {
				TOUCH_power_set(TOUCH_POWER_DEEP_SLEEP);
			} else {
				TOUCH_power_set(TOUCH_POWER_HIGH_20MS);
			}
#endif	
			break;
		}
		default:
			break;
	}
}
