#ifndef __FONT__
#define __FONT__

#include <stdint.h>
#include "standard_types.h"

//#define _ENABLE_FONT_TRANSFER_


// 5x7 ascii space
#define FONT_ASCII_5X7_SPACE_START	           (376832) //start addr:368k 	
#define FONT_ASCII_5X7_SPACE_SIZE	             (768)		 //occupy 768 byte


// 8x16 ascii space
#define FONT_ASCII_8X16_SPACE_START	           (FONT_ASCII_5X7_SPACE_START+FONT_ASCII_5X7_SPACE_SIZE)		
#define FONT_ASCII_8X16_SPACE_SIZE		         (1536)


//15x16 simple chinese space
#define FONT_SIMPLE_CHINESE_SPACE_START	       (FONT_ASCII_8X16_SPACE_START+FONT_ASCII_8X16_SPACE_SIZE)		
#define FONT_SIMPLE_CHINESE_SPACE_SIZE		     (669440)


//15x16 traditional chinese space
#define FONT_TRADITIONAL_CHINESE_SPACE_START	 (376832 + 4096)		
#define FONT_TRADITIONAL_CHINESE_SPACE_SIZE		 (333824)


//Unicode to GB code Digit Translation space
#define FONT_DIGIT_TRANSLATION_SPACE_START	   (FONT_TRADITIONAL_CHINESE_SPACE_START+FONT_TRADITIONAL_CHINESE_SPACE_SIZE)		
#define FONT_DIGIT_TRANSLATION_SPACE_SIZE		   (59076)


enum {
	FONT_TYPE_NONE,
	FONT_TYPE_SIMPLE_CHINESE,
	FONT_TYPE_TRADITIONAL_CHINESE
};


typedef struct fag_CLING_FONT_CTX {
	// font type
  I8U font_type;	
}FONT_CTX;	

#ifdef _ENABLE_FONT_TRANSFER_
void  FONT_flash_setup(void);
#endif

void  FONT_init(void);
I8U   FONT_load_characters(I8U *ptr,char *data,I8U height, BOOLEAN b_center);

#endif // __GT23L16U2Y__
/** @} */
