//
//  File: homekey.h
//
//  Header file for home key processing
//
#ifndef _HOMEKEY_HEADER_
#define _HOMEKEY_HEADER_


#define CLICK_HOLD_TIME_LONG     2000
#define CLICK_HOLD_TIME_RESTART  3000
#define CLICK_HOLD_TIME_SOS      4000

#define HOMEKEY_CLICK_DEBOUNCE_ON	 50 // 50 milli-second for debouncing
#define HOMEKEY_CLICK_DEBOUNCE_OFF	 50 // 50 milli-second for debouncing

typedef struct tagHOMEKEY_CLICK_STAT {
  I8U     temp_st;     			// temporarily hook status
  I8U     stable_st;        // stable hook event 
  I32U    ticks[2]; 		    // 0: ON 1: OFF, 2:SWITCHING
  I8U     half_click;   	  // on hook for a short time; but not long enough to call it on hook yet
	I8U     gesture;
	BOOLEAN b_valid_gesture;
#ifndef __YLF_SOS__
	BOOLEAN b_defective;
	I16U defective_time_out;
#endif
} HOMEKEY_CLICK_STAT;

// BUTTON STATUS
enum {
	ON_CLICK,
	OFF_CLICK,
};

typedef enum {
	CLICK_NONE,			// No event
	CLICK_HOLD,			// click and hold
	SINGLE_CLICK,		// click 
	DOUBLE_CLICK,		// double click

} CLICK_SUB_EVENTS;

enum {
	HOMEKEY_BUTTON_NONE = 0,
	HOMEKEY_BUTTON_SINGLE,	
	HOMEKEY_BUTTON_HOLD,
	HOMEKEY_BUTTON_SYSTEM_RESTART
};


void HOMEKEY_click_init(void);
void HOMEKEY_click_check(void);
void HOMEKEY_check_on_hook_change(void);

#ifdef _CLINGBAND_PACE_MODEL_
I8U HOMEKEY_get_gesture_panel(void);
#endif

#endif
