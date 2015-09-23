/***************************************************************************//**
 * File ui.h
 * 
 * Description: User Interface header
 *
 *
 ******************************************************************************/

#ifndef __LED_HEADER__
#define __LED_HEADER__

enum {
	LED_STATE_IDLE,
	LED_STATE_RENDERING,
	LED_STATE_OFF
};

#define LED_RENDER_INTERVAL 60 // 100 ms

typedef struct tagPEBBLE_LED_CTX {
	// State 
	I8U state;
	
	// timing control
	I32U ts;
	
	// Level-1 controlling variables
	I8U l1_stage_idx;
	I8U l1_pattern_idx;
	
	// Level-2 controlling variables
	I8U l2_pattern_rpt_count;
	I8U l2_pattern_rpt_max;
	I8U l2_pattern_idx;
	I8U l2_byte_idx;
	I8U L2_bit_idx;
	
} PEBBLE_LED_CTX;

void LED_state_machine_update(void);
void LED_startup(I8U index);
BOOLEAN LED_is_state_idle(void);

#endif
