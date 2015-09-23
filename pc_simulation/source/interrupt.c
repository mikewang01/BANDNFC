/***************************************************************************
 * File: interrupt.c
 * 
 * Description: interrupt handler routines
 *
 * Created on Feb 20, 2014
 *
 ******************************************************************************/

#include "main.h"

/* Interrupt handler for ADC data ready event */
void ADC_IRQHandler(void)
{
	BATT_CTX *b = &cling.batt;
	
	N_SPRINTF("[ADC INT] test:  %d",  ((NRF_ADC->CONFIG & 0x0000FF00)>>8));
	
	// check if data ready event occurs
	if (NRF_ADC->EVENTS_END == 0)
		return;

	/* Clear dataready event */
	NRF_ADC->EVENTS_END = 0;	
	
	//Disable the ADC INT for save power consumption
	NRF_ADC->INTENSET = ADC_INTENSET_END_Disabled;
	
	// check ADC INT source, BATT or THERM?
	if ( ((NRF_ADC->CONFIG & 0x0000FF00)>>8) == ADC_CONFIG_PSEL_AnalogInput4 )    // interrupt for batt
	{
		b->adc_state = CHARGER_ADC_ACQUIRED;
		b->volts_reading = NRF_ADC->RESULT;
		
		// Adjust battery reading with calibrated voltage
		// It is measured 0.1 volts lower than the real voltage
		// 0.1 volts is equivalent to 15 with 10 bit ADC
		b->volts_reading += 15;
	}
	else if ( ((NRF_ADC->CONFIG & 0x0000FF00)>>8) == ADC_CONFIG_PSEL_AnalogInput5 )    // interrupt for therm
	{
  	if (THERMISTOR_is_the_state(THERMISTOR_STAT_START_MEASURING)) {
  		THERMISTOR_set_state(THERMISTOR_STAT_COLLECT_SAMPLES);
		}
	}
	
	//Use the STOP task to save current. Workaround for PAN_028 rev1.2 anomaly 1.
	NRF_ADC->TASKS_STOP = 1;
}


void UART0_IRQHandler(void)
{
}

void GPIOTE_IRQHandler(void)
{
	// Event ausing the interrupt must be cleared
	if (NRF_GPIOTE->EVENTS_PORT) {
		NRF_GPIOTE->EVENTS_PORT = 0;
	}
}

