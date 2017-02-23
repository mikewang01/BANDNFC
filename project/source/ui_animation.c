/***************************************************************************//**
 * File ui_frame.h
 * 
 * Description: UI display animation.
 *
 *
 ******************************************************************************/

#include "main.h"
#include "ui_animation.h"
#include "ui_frame.h"

const I8U panning_table[] = {
	16, 16, 16, 16, 16, 16, 16, 16, 0, 0, 10
};

static void _frame_buffer_move_forward(I8U offset)
{
	I8U *p0;
	I8U len = 128 - offset;
	int i;

	p0 = cling.ui.p_oled_up;
	
	for (i = 0; i < 4; i++) {
		memcpy(p0, p0+offset, len); p0 += 128;
	}
	
	p0 = cling.ui.p_oled_up+len;
	
	for (i = 0; i < 4; i++) {
		memset(p0, 0, offset); p0 += 128;
	}
}

static void _frame_buffer_move_backward(I8U offset)
{
	I8U *p0;
	I8U len = 128 - offset;
	int i;

	p0 = cling.ui.p_oled_up;

	for (i = 0; i < 4; i++) {
		memcpy(p0+offset, p0, len); p0 += 128;
	}
	
	p0 = cling.ui.p_oled_up;
	
	for (i = 0; i < 4; i++) {
		memset(p0, 0, offset); p0 += 128;
	}
}

const I8U iris_tab[4][12] = {
 {128,0,0,
	48,32,48,
	48,32,48,
	128,0,0},
 {128,0,0,
	32,64,32,
	32,64,32,
	128,0,0},
 {16,96,16,
	16,96,16,
	16,96,16,
	16,96,16},
 {8,112,8,
	8,112,8,
	8,112,8,
	8,112,8},
};

static void _iris_frame_core(I8U offset)
{
	I8U *p0;
	I8U i;
	I8U len, index;
	const I8U *p_tab;
	
	p0 = cling.ui.p_oled_up;

	if (offset == 1) {
		memset(p0, 0, 512);
		return;
	} else if (offset == 5) {
		return;
	}
	
	p_tab = iris_tab[offset-2];
	for (i = 0; i < 4; i++) {
		len = *p_tab++; // reset  
		index = len;
		memset(p0,0,len);
		
		index += *p_tab++;  // Keep value
		
		len = *p_tab++; // reset
		memset(p0+index,0,len); //
		p0 += 128;
	}
	
}

static BOOLEAN _frame_panning(I8U dir, BOOLEAN b_out)
{
	UI_ANIMATION_CTX *u = &cling.ui;
	I8U shift_pos = 0;
	I8U i;
	
	// Update animation index
	u->animation_index ++;
	
	if (u->animation_index >= panning_table[OVERALL_PANNING_FRAMES]) {
		u->animation_index = 0;
		return TRUE;
	}

	// Shift screen
	if (b_out) {
		// Panning out current frame
		UI_frame_display_appear(u->frame_prev_idx, FALSE);
		
		for (i = 0; i < u->animation_index; i++) {
			shift_pos += panning_table[i];
		}
		
		if (dir == TRANSITION_DIR_LEFT) {
			_frame_buffer_move_forward(shift_pos);
		} else {
			_frame_buffer_move_backward(shift_pos);
		}
	} else {
		// Panning in next frame
		UI_frame_display_appear(u->frame_next_idx, FALSE);
		shift_pos = 128;
		for (i = 0; i < u->animation_index; i++) {
			shift_pos -= panning_table[i];
		}
		
		if (dir == TRANSITION_DIR_LEFT) {
			_frame_buffer_move_backward(shift_pos);
		} else {
			_frame_buffer_move_forward(shift_pos);
		}
	}
	
	// update frame buffer and render it
	UI_render_screen();
	
	return FALSE;
}

static BOOLEAN _frame_iris()
{
	UI_ANIMATION_CTX *u = &cling.ui;
	
	// Update animation index
	u->animation_index ++;
	
	if (u->animation_index >= 6) {
		u->animation_index = 0;
		return TRUE;
	}

	// Iris  in next frame
	UI_frame_display_appear(u->frame_next_idx, FALSE);
	
	//
	_iris_frame_core(u->animation_index);
		
	// update frame buffer and render it
	UI_render_screen();
	
	return FALSE;
}

static void _frame_buffer_move_upward(I8U offset)
{
	I8U *p0, *p1;
	I8U i;
	I8U len = 4 - offset;

	p0 = cling.ui.p_oled_up;
	p1 = p0+128*offset; // Get offset of sourcing buffer

	for (i = 0; i < len; i++) {
		memcpy(p0, p1, 128);
		p0 += 128;
		p1 += 128;
	}
	for (; i < 4; i++) {
		memset(p0, 0, 128);
		p0 += 128;
	}
}

static void _frame_buffer_move_downward(I8U offset)
{
	I8U *p0, *p1;
	I8U i;
	I8U len = 4 - offset;

	p0 = cling.ui.p_oled_up+128*3;
	p1 = p0-128*offset; // Get offset of sourcing buffer

	for (i = 0; i < len; i++) {
		memcpy(p0, p1, 128);
		p0 -= 128;
		p1 -= 128;
	}
	for (; i < 4; i++) {
		memset(p0, 0, 128);
		p0 -= 128;
	}
}

static BOOLEAN _frame_tilting(I8U dir, BOOLEAN b_out)
{
	UI_ANIMATION_CTX *u = &cling.ui;
	I8U shift_pos = 0;
	
	// Update animation index
	u->animation_index ++;
	
	if (u->animation_index >= 4) {
		u->animation_index = 0;
		return TRUE;
	}

	// Shift screen
	if (b_out) {
		// Panning out current frame
		UI_frame_display_appear(u->frame_prev_idx, FALSE);
		
		shift_pos = u->animation_index;
		
		if (dir == TRANSITION_DIR_UP) {
			_frame_buffer_move_upward(shift_pos);
		} else {
			_frame_buffer_move_downward(shift_pos);
		}
	} else {
		// Panning in next frame
		UI_frame_display_appear(u->frame_next_idx, FALSE);
		
		shift_pos = 4 - u->animation_index;
		
		if (dir == TRANSITION_DIR_UP) {
			_frame_buffer_move_downward(shift_pos);
		} else {
			_frame_buffer_move_upward(shift_pos);
		}
	}
	
	// update frame buffer and render it
	UI_render_screen();
	
	return FALSE;
}

void UI_frame_animating()
{
	UI_ANIMATION_CTX *u = &cling.ui;
	
	N_SPRINTF("[UI] animating: %d, %d, %d, %d, %d", u->animation_mode, u->animation_index, u->frame_prev_idx,
	u->frame_index, u->frame_next_idx);
	
	switch (u->animation_mode) {
		case ANIMATION_PANNING_OUT:
			if (_frame_panning(u->direction, TRUE)) {
				u->animation_mode = ANIMATION_PANNING_IN;
			}
			break;
		case ANIMATION_PANNING_IN:
			//if (_frame_panning(u->direction, FALSE)) {
			_frame_panning(u->direction, FALSE);
			{
				UI_switch_state(UI_STATE_APPEAR, 0);
			}
			break;
		case ANIMATION_TILTING_OUT:
			if (_frame_tilting(u->direction, TRUE)) {
				u->animation_mode = ANIMATION_TILTING_IN;
			}
			break;
		case ANIMATION_TILTING_IN:
			//if (_frame_tilting(u->direction, FALSE)) {
			_frame_tilting(u->direction, FALSE);
			{
				u->state = UI_STATE_APPEAR;
			}
			break;
		case ANIMATION_IRIS:
			if (_frame_iris()) {
				UI_switch_state(UI_STATE_APPEAR, 0);
			}
			break;
		default:
			break;
	}
}

void UI_set_animation(I8U mode, I8U dir)
{
	UI_ANIMATION_CTX *u = &cling.ui;

	UI_switch_state(UI_STATE_ANIMATING, 0);

	u->animation_index = 0;
	u->direction = dir;
	u->frame_prev_idx = u->frame_index;
	u->animation_mode = mode;
}
