/***************************************************************************//**
 * File ui_frame.h
 * 
 * Description: UI display animation.
 *
 *
 ******************************************************************************/

#ifndef __UI_ANIMATION_HEADER__
#define __UI_ANIMATION_HEADER__

#define OVERALL_PANNING_FRAMES 10

enum {
	ANIMATION_PANNING_IN,
	ANIMATION_PANNING_OUT,
	ANIMATION_TILTING_IN,
	ANIMATION_TILTING_OUT,
	ANIMATION_IRIS
};

enum {
	TRANSITION_DIR_NONE,
	TRANSITION_DIR_LEFT,
	TRANSITION_DIR_RIGHT,
	TRANSITION_DIR_UP,
	TRANSITION_DIR_DOWN
};

void UI_frame_animating(void);
void UI_set_animation(I8U mode, I8U dir);
#endif
