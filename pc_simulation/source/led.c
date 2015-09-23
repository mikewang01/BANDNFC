/***************************************************************************//**
 * File: led.c
 * 
 * Description: master UI controller
 *
 ******************************************************************************/

#include "main.h"
#include "pattern.h"

void LED_startup(I8U index)
{
	PEBBLE_LED_CTX *l = &pebble.led;
	I8U *p_level1_tab;
	
	Y_SPRINTF("[UI] display pattern: %d (20 ms)", index);
	
	// Turn on LED driver.
	TMP006_init();
	
	// UI needs a fast timer
	pebble.t.catnap_deep_interval = 20;
	SYSCLK_timer_config(SYSCLK_INTERVAL_20MS);
	
	l->state = LED_STATE_RENDERING;
	l->ts = CLK_get_system_time();
	l->l1_stage_idx = 0;
	l->l1_pattern_idx = index;
	
	p_level1_tab = (I8U *)level_1_pattern_ptrs[l->l1_pattern_idx];
	
	l->l2_pattern_rpt_max = p_level1_tab[l->l1_stage_idx+1];
	l->l2_pattern_idx = p_level1_tab[l->l1_stage_idx];
	
	l->l2_pattern_rpt_count = 0;
	
	l->L2_bit_idx = 0;
	l->l2_byte_idx = 0;
}

static void _render_one_line()
{
	PEBBLE_LED_CTX *l = &pebble.led;
	I32U curr_t = CLK_get_system_time();
	I32U l1_len, l2_len, needed_len;
	I32U render_pattern;
	I8U *p_level1_tab;
	I8U *p_level2_tab;

	if (curr_t < (l->ts + LED_RENDER_INTERVAL)) 
		return;

	// Update rendering time stamp
	l->ts += LED_RENDER_INTERVAL;
	
	
	p_level2_tab = (I8U *)level_2_pattern_ptrs[l->l2_pattern_idx];
	l2_len = level_2_pattern_lens[l->l2_pattern_idx];
	
	// Expected pattern table length
	needed_len = l->l2_byte_idx+3; // Need at least 2 extra bytes to form a pattern
	
	// If we don't have enough bytes for rendering
	if (needed_len > l2_len) {
		
		// Repeat one more time
		l->l2_pattern_rpt_count ++;
		
		// if we reach to maximum repeat count
		if (l->l2_pattern_rpt_count >= l->l2_pattern_rpt_max) {
			
			// Move onto next pattern
			l->l1_stage_idx += 2;
		
			// Get level-2 pattern index and repeat number
			p_level1_tab = (I8U *)level_1_pattern_ptrs[l->l1_pattern_idx];
			
			l1_len = level_1_pattern_lens[l->l1_pattern_idx];
			
			// if we reach the end of all the stages
			if (l->l1_stage_idx >= l1_len) {
				// Go return
				l->state = LED_STATE_OFF;
				return;
			} else {
				l->l2_pattern_rpt_count = 0;
				
				// Get pattern repeat info and index
				l->l2_pattern_rpt_max = p_level1_tab[l->l1_stage_idx+1];
				l->l2_pattern_idx = p_level1_tab[l->l1_stage_idx];
			}
		} 
		
		l->l2_byte_idx = 0;
		l->L2_bit_idx = 0;
	}
	
	if (l->L2_bit_idx)  {
		render_pattern = p_level2_tab[l->l2_byte_idx++] & 0x0f;
		l->L2_bit_idx = 0;
		render_pattern <<= 8;
		render_pattern |= p_level2_tab[l->l2_byte_idx++];
		render_pattern <<= 8;
		render_pattern |= p_level2_tab[l->l2_byte_idx++];
	} else {
		render_pattern = p_level2_tab[l->l2_byte_idx++];
		render_pattern <<= 8;
		render_pattern |= p_level2_tab[l->l2_byte_idx++];
		render_pattern <<= 4;
		render_pattern |= (p_level2_tab[l->l2_byte_idx] >> 4) & 0x0f;
		l->L2_bit_idx = 4;
	}
	
//	TMP006_pattern_render(render_pattern);
}

BOOLEAN LED_is_state_idle()
{
	PEBBLE_LED_CTX *l = &pebble.led;

	if (l->state == LED_STATE_IDLE) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void LED_state_machine_update()
{
	PEBBLE_LED_CTX *l = &pebble.led;

	switch (l->state) {
		case LED_STATE_IDLE:
			break;
		case LED_STATE_RENDERING:
			_render_one_line();
			break;
		case LED_STATE_OFF:
//			TMP006_pattern_render(0);
			TMP006_turn_off();
			pebble.t.catnap_deep_interval = 100;
			SYSCLK_timer_config(SYSCLK_INTERVAL_100MS);
			Y_SPRINTF("[LED] state off (100ms).");
			l->state = LED_STATE_IDLE;
			pebble.ui.enabled = FALSE;
		default:
			break;
	}
}

