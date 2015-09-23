/***************************************************************************//**
 * File touch.c
 * 
 * Description: touch panel driver
 *
 ******************************************************************************/

#include "main.h"

BOOLEAN TOUCH_new_gesture()
{
	TOUCH_CTX *t = &cling.touch;
	
	if (t->b_valid_gesture) {
		return TRUE;
	}
	
	return FALSE;
}

I8U TOUCH_get_gesture_panel()
{
	TOUCH_CTX *t = &cling.touch;
	
	if (t->b_double_tap) {
		t->tap_counts = 0;
		t->b_double_tap = FALSE;
		t->b_valid_gesture = FALSE;
		return TOUCH_DOUBLE_TAP;
	} else if (t->b_valid_gesture) {
		t->b_valid_gesture = FALSE;
		return t->gesture;
	}
	
	return TOUCH_NONE;
}

EN_STATUSCODE TOUCH_i2c_read_reg(I8U i8uRegNumber, I8U number_of_bytes, I8U * pi8uRegValue)
{
  BOOLEAN transfer_succeeded = FALSE;

  if (NULL == pi8uRegValue)
  {
    return STATUSCODE_NULL_POINTER;
  }
#ifndef _CLING_PC_SIMULATION_

  transfer_succeeded = twi_master_transfer(0x08, &i8uRegNumber, 1, TWI_DONT_ISSUE_STOP, TWI_MASTER_1);
  transfer_succeeded &= twi_master_transfer(0x09 |TWI_READ_BIT, pi8uRegValue, number_of_bytes, TWI_ISSUE_STOP, TWI_MASTER_1);
#endif
	if (transfer_succeeded) {
		return STATUSCODE_SUCCESS;
	} else {
		return STATUSCODE_FAILURE;
	}
}

BOOLEAN TOUCH_i2c_write_reg( I8U i8uRegNumber, I8U i8uRegValue)
{
  BOOLEAN transfer_succeeded=FALSE;
  unsigned char acData[2];

  acData[0] = i8uRegNumber;
  acData[1] = i8uRegValue;

#ifndef _CLING_PC_SIMULATION_
  transfer_succeeded = twi_master_transfer(0x08, acData, 2, TWI_ISSUE_STOP, TWI_MASTER_1);
#endif
  return transfer_succeeded;
}

TOUCH_POWER_MODE TOUCH_power_get()
{
#ifndef _CLING_PC_SIMULATION_
#ifdef _ENABLE_TOUCH_
	I8U mode;
	// Generate a pulse prior to I2C communication
	nrf_gpio_pin_clear(GPIO_TOUCH_CONTROL);
	BASE_delay_msec(4);
	nrf_gpio_pin_set(GPIO_TOUCH_CONTROL);
	
	twi_master_init(TWI_MASTER_1);
	
	// Write I2C register
	TOUCH_i2c_read_reg(TOUCH_I2C_REG_CONTROL, 1, &mode);

	Y_SPRINTF("\n[TOUCH] power mode get ++ %d ++\n", mode);

	GPIO_twi_disabled(TWI_MASTER_0);
	GPIO_twi_disabled(TWI_MASTER_1);

	return (TOUCH_POWER_MODE)mode;
#endif
#else
	return TOUCH_POWER_HIGH_20MS;
#endif
}

void TOUCH_power_set(TOUCH_POWER_MODE mode)
{
#ifndef _CLING_PC_SIMULATION_
#ifdef _ENABLE_TOUCH_
	// Generate a pulse prior to I2C communication
	nrf_gpio_pin_clear(GPIO_TOUCH_CONTROL);
	BASE_delay_msec(4);
	nrf_gpio_pin_set(GPIO_TOUCH_CONTROL);
	
	twi_master_init(TWI_MASTER_1);
	
	// Write I2C register
	TOUCH_i2c_write_reg(TOUCH_I2C_REG_CONTROL, mode);
	
	cling.touch.power_mode = mode;

	Y_SPRINTF("\n[TOUCH] power mode setting --- %d ---\n", mode);

	GPIO_twi_disabled(TWI_MASTER_0);
	GPIO_twi_disabled(TWI_MASTER_1);
#endif
#endif
}

void TOUCH_reset_tap_module()
{
	TOUCH_CTX *t = &cling.touch;
	
	t->tap_counts = 0;
	t->b_double_tap = FALSE;
}

void _tap_process(TOUCH_CTX *t, I32U t_curr)
{
		UI_ANIMATION_CTX *u = &cling.ui;

#if 0
		if (!LINK_is_authorized()) 
			return;
#endif
		
		if ((u->frame_index >= UI_DISPLAY_CAROUSEL) && (u->frame_index <= UI_DISPLAY_CAROUSEL_END))
			return;
		
		if (cling.user_data.b_navigation_tapping) {
			t->tap_counts ++;
			t->tap_time_base = t_curr;
			
			if (t->tap_counts >= 3) {

				N_SPRINTF("[TOUCH] +++ double tapped, %d, %d", t->tap_counts, t->tap_time_base);
				// Double tap - let's jump to icon page
				//
				t->b_double_tap = TRUE;
			}
		}
}

void TOUCH_power_mode_state_machine(void)
{
	TOUCH_CTX *t = &cling.touch;
	TOUCH_POWER_MODE mode;
	
	switch (t->state) {
		case TOUCH_STATE_IDLE:
			break;
		case TOUCH_STATE_MODE_SET:
		{
			if (t->power_mode == t->power_new_mode) {
				t->state = TOUCH_STATE_IDLE;
			} else {
				TOUCH_power_set(t->power_new_mode);
				t->state = TOUCH_STATE_MODE_CONFIRMING;
			}
			break;
		}
		case TOUCH_STATE_MODE_CONFIRMING:
		{
			mode = TOUCH_power_get();
			if (mode != cling.touch.power_mode) {
				t->state = TOUCH_STATE_MODE_SET;
			} else {
				t->state = TOUCH_STATE_IDLE;
			}
			break;
		}
		default:
			break;
	}
}

static BOOLEAN _touch_screen_activation()
{
	if (cling.user_data.b_screen_press_hold_1)
		return TRUE;
	
	if (cling.user_data.b_screen_press_hold_3)
		return TRUE;
	
	return FALSE;
}

static void _finger_down_processing(TOUCH_CTX *t, I8U op_detail, I32U t_curr)
{
	if (UI_is_idle()) {

		if (!_touch_screen_activation())
			return;
	}
	
	// finger down
	if (op_detail == 0) {
		t->gesture = TOUCH_FINGER_LEFT;
	} else if (op_detail == 1) {
		t->gesture = TOUCH_FINGER_UP;
	} else if (op_detail == 2) {
		t->gesture = TOUCH_FINGER_UP;
	} else if (op_detail == 3) {
		t->gesture = TOUCH_FINGER_RIGHT;
	}
					
	// Make sure OLED display panel is faced up.
//		if (LINK_is_authorized() && (cling.activity.z_mean < 0)) {
	if (LINK_is_authorized()) {
			
		N_SPRINTF("[TOUCH] ------------ TURN ON SCREEN --------");

		// Set to highest sensitivity
		cling.touch.state = TOUCH_STATE_MODE_SET;
		cling.touch.power_new_mode = TOUCH_POWER_HIGH_20MS;

		// Start 20 ms timer for screen rendering
		SYSCLK_timer_start();
			
		// Turn on OLED panel
		if (!OLED_set_panel_on()) {
			t->b_valid_gesture = TRUE;
					
			// Tap detection
			_tap_process(t, t_curr);
		} 
		
		cling.ui.true_display = TRUE;
		
		// Update touch event time
		cling.ui.touch_time_stamp = CLK_get_system_time();
		
		if (UI_is_idle()) {
			// UI initial state, a glance of current clock
			UI_switch_state(UI_STATE_CLOCK_GLANCE, 0);
			N_SPRINTF("[TOUCH] set UI: clock glance (finger down)");
		} else {
			// 
			UI_switch_state(UI_STATE_TOUCH_SENSING, 0);
			N_SPRINTF("[TOUCH] set UI: touch sensing (finger down)");
		}
	}
}

static void _swipe_processing(TOUCH_CTX *t, I8U op_detail)
{
	if (UI_is_idle()) {

		if (!_touch_screen_activation())
			return;
	}
	
	// Swipe
	if (op_detail == 0) {
		t->gesture = TOUCH_SWIPE_LEFT;
	} else if (op_detail == 1) {
		t->gesture = TOUCH_SWIPE_RIGHT;
	} else if (op_detail == 2) {
		t->gesture = TOUCH_SWIPE_UP;
	} else if (op_detail == 3) {
		t->gesture = TOUCH_SWIPE_DOWN;
	}
	
//		if (LINK_is_authorized() && (cling.activity.z_mean < 0)) {
	if (LINK_is_authorized()) {

		// Set to highest sensitivity
		cling.touch.state = TOUCH_STATE_MODE_SET;
		cling.touch.power_new_mode = TOUCH_POWER_HIGH_20MS;
		
		N_SPRINTF("[TOUCH] ------------ TURN ON SCREEN --------");
		
		// Start 20 ms timer for screen rendering
		SYSCLK_timer_start();
		
		// Turn on OLED panel
		if (!OLED_set_panel_on()) {
			t->b_valid_gesture = TRUE;
		}
		
		cling.ui.true_display = TRUE;
		
		// touch time update
		cling.ui.touch_time_stamp = CLK_get_system_time();

		if (UI_is_idle()) {
			// UI initial state, A glance of current time
			UI_switch_state(UI_STATE_CLOCK_GLANCE, 0);
			N_SPRINTF("[TOUCH] set UI: clock glance (swipe)");
		} else {
			UI_switch_state(UI_STATE_TOUCH_SENSING, 0);
			N_SPRINTF("[TOUCH] set UI: touch sensing (swipe)");
		}
	}
}

static void _skin_touch_processing(TOUCH_CTX *t, I8U op_detail)
{
		t->b_valid_gesture = FALSE;

		// skin touch
		if (op_detail == 0) {
			t->gesture = TOUCH_SKIN_PAD_0;
		} else if (op_detail == 1) {
			t->gesture = TOUCH_SKIN_PAD_1;
		} else if (op_detail == 2) {
			t->gesture = TOUCH_SKIN_PAD_2;
		} else if (op_detail == 3) {
			t->gesture = TOUCH_SKIN_PAD_3;
		} else if (op_detail == 4) {
			t->gesture = TOUCH_SKIN_PAD_4;
		} else {
			t->gesture = TOUCH_SKIN_PAD_0;
		}
		
		// 
		// Do something with skin touch
		//
		t->skin_touch_type = t->gesture;
		
		Y_SPRINTF("[TOUCH] skin touch: %d", op_detail);
}

void TOUCH_gesture_check(void)
{
#ifndef _CLING_PC_SIMULATION_
  I8U int_pin;
#ifdef __DEBUG_BASE__
	I8U status_value;
#endif
	I8U reg_value;
	I8U op_code, op_detail;
#ifdef __DEBUG_BASE__
//	I8U op_seq;
#endif
	TOUCH_CTX *t = &cling.touch;
	I32U t_curr = CLK_get_system_time();

	// 1.5 Seconds expiration of double tapping
	if (t_curr > (t->tap_time_base + 1500)) {
		if (t->tap_counts) {
			N_SPRINTF("[TOUCH] +++ clear tap, %d, %d, %d", t->tap_counts, t->tap_time_base, t_curr);
			t->tap_counts = 0;
		}
		t->b_double_tap = FALSE;
	}

	// Check to see if the touch Interrupt pin is pulled down
	int_pin = nrf_gpio_pin_read(GPIO_TOUCH_INT);           

	if(int_pin)
		return;

	// Enable TWI to read out I2C touch panel register
	// I2C initialization
	twi_master_init(TWI_MASTER_1);
	
	// A valid gesture
	t->gesture = TOUCH_NONE;

	TOUCH_i2c_read_reg(TOUCH_I2C_REG_GESTURE, 1, &reg_value);
#ifdef __DEBUG_BASE__
	TOUCH_i2c_read_reg(TOUCH_I2C_REG_STATUS, 1, &status_value);
#endif
	TOUCH_i2c_write_reg(TOUCH_I2C_REG_STATUS, 0);
	
	// Configure TWI to be input to reduce power consumption
	GPIO_twi_disabled(TWI_MASTER_0);
	GPIO_twi_disabled(TWI_MASTER_1);
	N_SPRINTF("[TOUCH] Interrupt status (%d): -- %d --", CLK_get_system_time(), status_value);
	
	//
	// Gesture byte definition -
	//
	// Bit 5-7: unused
	//
	// Bit 3-4: operation code
	//
	//          00: no op
	//          01: swipe
	//          10: finger
	//          11: skin touch
	//
	// Bit 0-2: detail
	//
	op_code = reg_value & 0x18;
	op_detail = reg_value & 0x07;
	
#ifdef __DEBUG_BASE__
//	op_seq = reg_value & 0xe0;
	N_SPRINTF("[TOUCH] reg value: %02x, seq: %d, op: %d, detail: %d", reg_value, op_seq>>5, op_code>>3, op_detail);
#endif

	if (op_code == 0x08) {
		
		// Swipe processing
		_swipe_processing(t, op_detail);

	} else if (op_code == 0x10) {

		// finger down processing
		_finger_down_processing(t, op_detail, t_curr);
		
	} else if (op_code == 0x18) {

		// skin touch processing
		_skin_touch_processing(t, op_detail);
		
	} else {
		// None
		t->b_valid_gesture = FALSE;

	}
	
	// Wait until 10 ms interrupt period is passed.
	BASE_delay_msec(12);
#endif
}

void TOUCH_init(void)
{		
#ifndef _CLING_PC_SIMULATION_
	I8U mode = TOUCH_POWER_MIDIAN_200MS;
	// TWI initialization
	twi_master_init(TWI_MASTER_1);

	// Generate a pulse prior to I2C communication
	nrf_gpio_pin_clear(GPIO_TOUCH_CONTROL);
	BASE_delay_msec(4);
	nrf_gpio_pin_set(GPIO_TOUCH_CONTROL);

	TOUCH_i2c_read_reg(TOUCH_I2C_REG_VERSION, 1, &cling.whoami.touch_ver);

	N_SPRINTF("[TOUCH] init:%d",cling.whoami.touch_ver);
	
	// Write I2C register to set a median power level
	TOUCH_i2c_write_reg(TOUCH_I2C_REG_CONTROL, mode);
	cling.touch.power_mode = TOUCH_POWER_MIDIAN_200MS;

	// Read out version one more time
	TOUCH_i2c_read_reg(TOUCH_I2C_REG_VERSION, 1, &cling.whoami.touch_ver);

	// Initialize skin touch states
	cling.touch.skin_touch_type = TOUCH_SKIN_PAD_0;
	
	GPIO_twi_disabled(TWI_MASTER_0);
	GPIO_twi_disabled(TWI_MASTER_1);
#endif
}

I8U TOUCH_get_skin_pad(void)
{
	if (PPG_is_skin_touched()) {
		return 4;
	} else {
		return (cling.touch.skin_touch_type - TOUCH_SKIN_PAD_0);
	}
}

BOOLEAN TOUCH_is_skin_touched(void)
{
	return TRUE;
	
	if (BATT_is_charging())
		return FALSE;

	if (cling.touch.skin_touch_type < TOUCH_SKIN_PAD_1)
		return FALSE;
	if (cling.touch.skin_touch_type > TOUCH_SKIN_PAD_4)
		return FALSE;

  if (cling.touch.skin_touch_type >= TOUCH_SKIN_PAD_1)
		return TRUE;
	
	return FALSE;
}
