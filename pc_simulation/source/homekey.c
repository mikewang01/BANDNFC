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

	b_pin = nrf_gpio_pin_read(GPIO_HOMEKEY);	

	// button released
	if (b_pin) stat = OFF_CLICK;

	k->stable_st = stat;
	k->temp_st = stat;
	k->ticks[stat] = CLK_get_system_time();
#endif
}

static BOOLEAN _check_on_hook_change(HOMEKEY_CLICK_STAT *k)
{
#ifndef _CLING_PC_SIMULATION_

	I32U t_curr;
	I8U b_pin;
	I8U stat = ON_CLICK;
	
	// Get a temporarily BUTTON status
	b_pin = nrf_gpio_pin_read(GPIO_HOMEKEY);	

	if (b_pin) {
		stat = OFF_CLICK;
	} 

	if (stat != k->temp_st) {

		t_curr = CLK_get_system_time();
		
		// update the click time stamp
		k->temp_st = stat;
		k->ticks[stat] = t_curr;
		Y_SPRINTF("[HOMEKEY] --- BUTTON Event ---");

		if (k->temp_st == ON_CLICK) {
			k->click_on_ts = t_curr;
		
			// Make sure OLED display panel is faced up.
			if (LINK_is_authorized()) {
					
				Y_SPRINTF("[TOUCH] ------------ TURN ON SCREEN --------");

				// Set to highest sensitivity
				cling.touch.state = TOUCH_STATE_MODE_SET;
				cling.touch.power_new_mode = TOUCH_POWER_HIGH_20MS;

				// Start 20 ms timer for screen rendering
				SYSCLK_timer_start();
					
				// Turn on OLED panel
				OLED_set_panel_on();
				
				cling.ui.true_display = TRUE;
				
				// Update touch event time
				cling.ui.touch_time_stamp = CLK_get_system_time();
				
				if (UI_is_idle()) {
					// UI initial state, a glance of current clock
					UI_switch_state(UI_STATE_CLOCK_GLANCE, 0);
					Y_SPRINTF("[KEY] set UI: button clicked (glance)");
				} else {
					// 
					UI_switch_state(UI_STATE_TOUCH_SENSING, 0);
					Y_SPRINTF("[KEY] set UI: button clicked (sensing)");
				}
			}
		} 
		return TRUE;
	} else {
		return FALSE;
	}
#else
	return FALSE;
#endif
}

/****************************************************************************
    Check button status for on-the-fly events.
****************************************************************************/
void HOMEKEY_click_check()
{
	HOMEKEY_CLICK_STAT *k = &cling.key;
	I32U t_curr = CLK_get_system_time();
	I32U offset = 0;
	I32U t_click_and_hold = CLICK_HOLD_TIME_LONG;

	// detect a possible state change
	_check_on_hook_change(k);
	
	// only update hook state machine when there is possible state switch 
	if (k->stable_st == k->temp_st) {
		
		// If it is press and hold, go return
		if (k->temp_st == ON_CLICK) {
			return;
		}
	} 

	// Handle the debouncing of the click status change
	if (k->temp_st == ON_CLICK) {

  		// Check how long we have been in "ON-CLICK" status
		offset = t_curr - k->ticks[ON_CLICK];

		// Debouncing
		if (offset < HOMEKEY_CLICK_DEBOUNCE) 
			return;

		if (offset >= t_click_and_hold) { 
			// announce a stable state --> ON_CLICK
			k->stable_st = ON_CLICK;
			// clear possible hook flash flag
			k->half_click = 0;

			// Increase the counter for "press + hold"
			k->click_hold_num ++;
			
			cling.touch.b_valid_gesture = TRUE;
			cling.touch.gesture = TOUCH_BUTTON_PRESS_HOLD;
			Y_SPRINTF("[HOMEKEY] +++ button press and hold");
				
		} else {
			// indicate a possible double click, no state update just yet!
			k->half_click = 1;

		} 
	} else {
		// Add debounce logic to prevent multiple button click
		offset = t_curr - k->ticks[OFF_CLICK];
		if (offset < HOMEKEY_CLICK_DEBOUNCE) 
			return;

		offset = k->ticks[OFF_CLICK] - k->ticks[ON_CLICK];

		// if there is half click flag, we are good to declare a single click event
		if(k->half_click && (offset < t_click_and_hold)) {
			k->half_click = 0;

			k->single_click_num ++;
			
			cling.touch.b_valid_gesture = TRUE;
			cling.touch.gesture = TOUCH_BUTTON_SINGLE;
			Y_SPRINTF("[HOMEKEY] +++ button single click");
				
		}
		else {
			// declare a new state --> OFF HOOK
			k->stable_st = OFF_CLICK;
			k->half_click = 0; 
		}
	}
}

I32U time_key=0, sim_idx = 0, sim_started = 0;
I8U sim_key_tab[30] = {	
	//TOUCH_BUTTON_PRESS_HOLD,
	TOUCH_SWIPE_LEFT,
	TOUCH_SWIPE_LEFT,
	TOUCH_BUTTON_SINGLE,
	TOUCH_BUTTON_SINGLE,
	TOUCH_BUTTON_SINGLE,
	TOUCH_SWIPE_LEFT,
	TOUCH_BUTTON_SINGLE,
	TOUCH_SWIPE_LEFT,
	TOUCH_BUTTON_SINGLE,
	TOUCH_SWIPE_LEFT,
	TOUCH_BUTTON_SINGLE,
	TOUCH_SWIPE_LEFT,
	TOUCH_BUTTON_SINGLE,
	TOUCH_SWIPE_LEFT,
	TOUCH_BUTTON_SINGLE,
	TOUCH_SWIPE_LEFT,
	TOUCH_BUTTON_SINGLE,
	TOUCH_SWIPE_LEFT,
	TOUCH_SWIPE_LEFT,
	TOUCH_BUTTON_SINGLE,
	TOUCH_BUTTON_SINGLE,
	//TOUCH_FINGER_LEFT,
	TOUCH_BUTTON_SINGLE,
	TOUCH_BUTTON_SINGLE,
	//TOUCH_FINGER_LEFT,
	TOUCH_BUTTON_SINGLE,
	TOUCH_BUTTON_SINGLE,
	TOUCH_BUTTON_SINGLE,
	TOUCH_BUTTON_SINGLE,
	TOUCH_FINGER_LEFT,
};

void HOMEKEY_sim_kickoff()
{
	sim_started = 1;
	time_key = 0;
	cling.time.system_clock_in_sec = 0;
}

void HOMEKEY_sim()
{
	if (!sim_started)
		return;
	
	if (cling.time.system_clock_in_sec > (time_key+2)) {
		time_key = cling.time.system_clock_in_sec;
		cling.touch.b_valid_gesture = TRUE;
		cling.touch.gesture = sim_key_tab[sim_idx++];
		if (sim_idx >= 28)
			sim_idx = 0;
		
		Y_SPRINTF("[HOMEKEY] index: %d, key: %d", sim_idx, cling.touch.gesture);
	}
}
