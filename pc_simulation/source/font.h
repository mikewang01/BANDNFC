#ifndef __FONT__
#define __FONT__

#include <stdint.h>
#include "standard_types.h"

//#define _ENABLE_FONT_TRANSFER_

typedef struct fag_CLING_FONT_CTX {
	
I8U      data[32];
	
}FONT_CTX;	

#ifdef _ENABLE_FONT_TRANSFER_
void  FONT_flash_setup(void);
void  FONT_init(void);
#endif


I8U FONT_load_characters(I8U *ptr,char *data,I8U height, BOOLEAN b_center);


#endif // __GT23L16U2Y__
/** @} */
