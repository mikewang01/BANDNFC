/***************************************************************************//**
 * File ui_frame.h
 * 
 * Description: UI display frame, include horizontal and vertical.
 *
 *
 ******************************************************************************/

#ifndef __UI_FRAME_HEADER__
#define __UI_FRAME_HEADER__


enum {
	LANGUAGE_TYPE_ENGLISH=0,
	LANGUAGE_TYPE_SIMPLIFIED_CHINESE,
	LANGUAGE_TYPE_TRADITIONAL_CHINESE,
	LANGUAGE_TYPE_MAX
};

typedef struct tagUI_RENDER_CTX {
  void (*top_row_render)();
  void (*middle_row_render)();
  void (*bottom_row_render)();
} UI_RENDER_CTX;

void UI_frame_display_appear(I8U index, BOOLEAN b_render);
void UI_render_screen(void);
#endif
