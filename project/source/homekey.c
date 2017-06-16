/***************************************************************************//**
 * 
 * File: homekey.c
 *
 * Description: Home key basics - click, (double click, click and hold)
 *
 * Created on April/29/2015
 * 
 *****************************************************************************/

#include "main.h"
#include "homekey.h"

void HOMEKEY_click_init()
{
#ifndef _CLING_PC_SIMULATION_
	I8U b_pin;
	I8U stat = ON_CLICK;
	HOMEKEY_CLICK_STAT *k = &cling.key;

	b_pin = nrf_gpio_pin_read(GPIO_TOUCH_HOMEKEY_INT);	

	// button released
	if (b_pin) stat = OFF_CLICK;

	k->stable_st = stat;
	k->temp_st = stat;
	k->ticks[stat] = CLK_get_system_time();
#endif
}

void HOMEKEY_check_on_hook_change()
{
#ifndef _CLING_PC_SIMULATION_
	HOMEKEY_CLICK_STAT *k = &cling.key;
	I32U t_curr;
	I8U b_pin;
	I8U stat = ON_CLICK;

	// Get a temporarily BUTTON status
	b_pin = nrf_gpio_pin_read(GPIO_TOUCH_HOMEKEY_INT);	

	t_curr = CLK_get_system_time();
	
	if (b_pin) {
		stat = OFF_CLICK;
#ifndef __YLF_SOS__
		k->b_defective = FALSE;
		k->defective_time_out = 0;
#endif
	} 
  
	if (stat != k->temp_st) {
		
		// update the click time stamp
		k->temp_st = stat;
		k->ticks[stat] = t_curr;
		N_SPRINTF("[HOMEKEY] --- BUTTON Event at %d---(%d)", t_curr, stat);

#ifdef _CLINGBAND_PACE_MODEL_		
		if (OLED_panel_is_turn_on()) {
		  if (k->temp_st != OFF_CLICK)
        return;					
		}				
#else			
		if (k->temp_st != ON_CLICK) 
			return;	
#endif

		// Make sure OLED display panel is faced up.
		if (LINK_is_authorized()) {
				
			N_SPRINTF("[TOUCH] ------------ TURN ON SCREEN --------");

			if (OLED_panel_is_turn_off()) {
				cling.ui.b_invalid_touch_action = TRUE;
			}
								
			UI_turn_on_display(UI_STATE_TOUCH_SENSING);
		} else {
			RTC_start_operation_clk();
		}
	} 
#endif
}

/****************************************************************************
    Check button status for on-the-fly events.
****************************************************************************/
void HOMEKEY_click_check()
{
	HOMEKEY_CLICK_STAT *k = &cling.key;
	I32U offset = 0;
	I32U t_curr = CLK_get_system_time();
	I32U t_click_and_hold = CLICK_HOLD_TIME_LONG;
#ifdef _CLINGBAND_PACE_MODEL_			
	I32U t_sys_restart = CLICK_HOLD_TIME_RESTART;
	I8U  pdata[2];
#else
	I32U t_sos = CLICK_HOLD_TIME_SOS;
#endif

#ifndef __YLF_SOS__
	if (k->b_defective) return;
#endif
	
	// only update hook state machine when there is possible state switch 
	if (k->stable_st == k->temp_st) {
		
		// If it is press and hold, go return
		if (k->temp_st == ON_CLICK) {
	
			offset = t_curr - k->ticks[ON_CLICK];

#ifdef _CLINGBAND_PACE_MODEL_					
			if (offset >= t_sys_restart) {
				
				k->ticks[ON_CLICK] += 2000;
	
				if (!cling.system.b_powered_up) {

          if (!LINK_is_authorized()) {					
				  	pdata[0] = 0x07;
					  pdata[1] = 0x18;

            BATT_charging_init();
						
            FLASH_erase_App(SYSTEM_REMINDER_SPACE_START);
						
						BASE_delay_msec(50);
					
            FLASH_Write_App(SYSTEM_REMINDER_SPACE_START, pdata, 2);			
						
						N_SPRINTF("[HOMEKEY] Pace restart system and open vibration");	
						
            SYSTEM_restart_from_reset_vector();					
					}	
				}
			}
#else
#ifndef __YLF_SOS__
			if (k->defective_time_out >= 15000) {
				k->b_defective = TRUE;
				return;
			}
#endif
			// Go ahead to check if SOS is pressed			
			if (offset >= t_sos) {
			  // Ignore SOS request if device is not authorized.
			  if (!LINK_is_authorized())
				  return;

				k->ticks[ON_CLICK] += 2000;
#ifndef __YLF_SOS__
				k->defective_time_out += 2000;
#endif
				cling.touch.b_valid_gesture = TRUE;
				cling.touch.gesture = TOUCH_BUTTON_PRESS_SOS;
				N_SPRINTF("[HOMEKEY] +++ button (SOS), %d, %d, %d", t_curr, offset, t_sos);
				// Send SOS message to the App
				CP_create_sos_msg();
			}
#endif
		  return;
		}
	} 

	// Handle the debouncing of the click status change
	if (k->temp_st == ON_CLICK) {

  		// Check how long we have been in "ON-CLICK" status
		offset = t_curr - k->ticks[ON_CLICK];
		
		// Debouncing
		if (offset < HOMEKEY_CLICK_DEBOUNCE_ON) 
			return;
			
		if (offset >= t_click_and_hold) { 
			// announce a stable state --> ON_CLICK
			k->stable_st = ON_CLICK;
			// clear possible hook flash flag
			k->half_click = 0;

#ifdef _CLINGBAND_PACE_MODEL_
			k->b_valid_gesture = TRUE;
			k->gesture = HOMEKEY_BUTTON_HOLD;
#else			
			cling.touch.b_valid_gesture = TRUE;
			cling.touch.gesture = TOUCH_BUTTON_PRESS_HOLD;
			
			// if unauthorized, we should turn off screen, or turn it on
			if (!cling.system.b_powered_up) {
				SYSTEM_restart_from_reset_vector();
			} else {
				cling.batt.shut_down_time = BATTERY_SYSTEM_UNAUTH_POWER_DOWN;
			}				
#endif	
			N_SPRINTF("[HOMEKEY] +++ button press and hold: %d, %d, %d, %d", offset, t_curr, k->ticks[ON_CLICK], CLK_get_system_time());		
		} else {
			// indicate a possible double click, no state update just yet!
			k->half_click = 1;
		} 
	} else {
		// Add debounce logic to prevent multiple button click
		offset = t_curr - k->ticks[OFF_CLICK];
		if (offset < HOMEKEY_CLICK_DEBOUNCE_OFF)
			return;

		offset = k->ticks[OFF_CLICK] - k->ticks[ON_CLICK];
		if (offset < HOMEKEY_CLICK_DEBOUNCE_ON)
			return;
		
		// if there is half click flag, we are good to declare a single click event
		if(k->half_click && (offset < t_click_and_hold)) {
			k->half_click = 0;

#ifdef _CLINGBAND_PACE_MODEL_			
			k->b_valid_gesture = TRUE;
			k->gesture = HOMEKEY_BUTTON_SINGLE;
#else			
			cling.touch.b_valid_gesture = TRUE;
			cling.touch.gesture = TOUCH_BUTTON_SINGLE;		
#endif
			
			N_SPRINTF("[HOMEKEY] +++ button single click");
		} else {
			// declare a new state --> OFF HOOK
			k->stable_st = OFF_CLICK;
			k->half_click = 0; 
		}
	}
}

#ifdef _CLINGBAND_PACE_MODEL_
I8U HOMEKEY_get_gesture_panel()
{
	HOMEKEY_CLICK_STAT *k = &cling.key;
	
	if (k->b_valid_gesture) {
		k->b_valid_gesture = FALSE;
		return k->gesture;
	}
	
	return HOMEKEY_BUTTON_NONE;
}
#endif
