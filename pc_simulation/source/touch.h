/***************************************************************************//**

 * touch.h
 *
 ******************************************************************************/

#ifndef __TOUCH_H__
#define __TOUCH_H__

#include "standards.h"

#define _ENABLE_TOUCH_

#define TOUCH_DEEP_SLEEP_TIME_INTERVAL 3000 // 60 Seconds, we enter deep sleep

enum {
	TOUCH_I2C_REG_CONTROL = 0,
	TOUCH_I2C_REG_VERSION,
	TOUCH_I2C_REG_GESTURE,
	TOUCH_I2C_REG_STATUS
};

typedef enum {
	TOUCH_POWER_HIGH_20MS,
	TOUCH_POWER_MIDIAN_200MS,
	TOUCH_POWER_LOW_2000MS,
	TOUCH_POWER_DEEP_SLEEP,
} TOUCH_POWER_MODE;

enum {
	TOUCH_NONE = 0,
	TOUCH_SWIPE_DOWN,
	TOUCH_SWIPE_UP,
	TOUCH_SWIPE_LEFT,
	TOUCH_SWIPE_RIGHT,
	TOUCH_FINGER_DOWN,
	TOUCH_FINGER_UP,
	TOUCH_FINGER_LEFT,
	TOUCH_FINGER_RIGHT,
	TOUCH_SKIN_PAD_0,
	TOUCH_SKIN_PAD_1,
	TOUCH_SKIN_PAD_2,
	TOUCH_SKIN_PAD_3,
	TOUCH_SKIN_PAD_4,
	TOUCH_DOUBLE_TAP,
	TOUCH_BUTTON_SINGLE,
	TOUCH_BUTTON_PRESS_HOLD,
	TOUCH_MAX
};

typedef enum {
	TOUCH_STATE_IDLE,
	TOUCH_STATE_MODE_SET,
	TOUCH_STATE_MODE_CONFIRMING,
} TOUCH_STATE;

typedef struct tagTOUCH_CTX {
	// Tap activities
	I32U tap_time_base;
	I8U tap_counts;
	BOOLEAN b_double_tap;
	
	// Gesture
	BOOLEAN b_valid_gesture;
	I8U gesture;
	
	// State machine
	TOUCH_STATE state;
	
	// Power mode
	TOUCH_POWER_MODE power_mode;
	TOUCH_POWER_MODE power_new_mode;
	
	// Skin touch update
	I8U skin_touch_type;
} TOUCH_CTX;

void TOUCH_init(void);
I8U TOUCH_get_gesture_panel(void);
void TOUCH_gesture_check(void);
void TOUCH_power_set(TOUCH_POWER_MODE mode);
TOUCH_POWER_MODE TOUCH_power_get(void);
I8U TOUCH_get_skin_pad(void);
void TOUCH_power_mode_state_machine(void);
BOOLEAN TOUCH_new_gesture(void);
BOOLEAN TOUCH_is_skin_touched(void);

#endif // __TOUCH_H__
/** @} */
