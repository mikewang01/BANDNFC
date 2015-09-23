/***************************************************************************//**
 * @addtogroup WATCHDOG
 * @{ 
 ******************************************************************************/

#include "main.h"

EN_STATUSCODE Watchdog_Init(void)
{
#ifndef _CLING_PC_SIMULATION_
	NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) | ( WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos);
	NRF_WDT->CRV = 16*32768;   //ca 3 sek. timout
	NRF_WDT->RREN |= WDT_RREN_RR0_Msk;  //Enable reload register 0
	NRF_WDT->TASKS_START = 1;
#endif
	N_SPRINTF("[WDG] Watchdog init");
  return STATUSCODE_SUCCESS;
}

// Configure the watchdog based on selected timeout.
EN_STATUSCODE Watchdog_Enable()
{ 
  return STATUSCODE_SUCCESS;
}

EN_STATUSCODE Watchdog_Disable(void)
{
  return STATUSCODE_SUCCESS;
}

void Watchdog_Feed(void)
{
#ifndef _CLING_PC_SIMULATION_
	NRF_WDT->RR[0] = 0x6E524635;  //Reload watchdog register 0
#endif
}

/** @} */
