/***************************************************************************//**
 * File uart.h
 * 
 * Description: UART header
 *
 *
 ******************************************************************************/

#ifndef __UART_HEADER__
#define __UART_HEADER__

#include "standards.h"

#include <string.h>
#ifndef _CLING_PC_SIMULATION_
//#include "simple_uart.h"
#endif
#define HWFC           false

void UART_init(void);
void UART_print(char *str);
void UART_disabled(void);

#endif // __BTLE_HEADER__
/** @} */
