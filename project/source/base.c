//
//  base.c
//
//  Cling base function support.
//
#include <math.h>

#include "main.h"

I32U BASE_dword_decode(I8U *in)
{
	I32U out = *in;
	out <<= 8;
	out |= *(in+1);
	out <<= 8;
	out |= *(in+2);
	out <<= 8;
	out |= *(in+3);
	return out;
}

I8U BASE_dword_encode(I32U in, I8U *out)
{
	*out = (I8U) ((in & 0xFF000000) >> 24);
	*(out+1) = (I8U) ((in & 0x00FF0000) >> 16);
	*(out+2) = (I8U) ((in & 0x0000FF00) >> 8);
	*(out+3) = (I8U) ((in & 0x000000FF) >> 0);
	return 4;
}

I32S BASE_abs(I32S a) {
  	if (a>=0) {
		return a;
	} else {
		return -a;
	}  
}

I32S BASE_sqrt(I32S a) {
    return (I32S)sqrt(a);
}

I8U BASE_calculate_occurance(I32U in, I8U size)
{
    I8U cnt, i;
    I32U idx = in;

    cnt = 0;
    for (i = 0; i < size; i++) {
        if (idx & 1) {
            cnt ++;
        }
        idx >>= 1;
    }
    return cnt;
}

// get local time in a precision of 1 ms.
I32U CLK_get_system_time()
{
	return (SYSCLK_GetFineTime());
}


// Tiny little function to delay for a number of milliseconds.
// I'm using a busy wait because I assume that the BMP085 code expects
// the delay to be constant, whereas a CatNap would wake up on an
// interrupt as well.
// Also, the delay is in 1/1024 seconds instead of milliseconds.  I
// can't imagine such a small difference is a problem in this context.
void BASE_delay_msec(I32U uiMSec)
{
#ifndef _CLING_PC_SIMULATION_
	// Fine time is in 1/32768 seconds, so we need to leftshift uiMSec by 5 bits.
	nrf_delay_ms(uiMSec);
#endif
}
