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
	N_SPRINTF("[ADC INT] test:  %d",  ((NRF_ADC->CONFIG & 0x0000FF00)>>8));
	
	// check if data ready event occurs
	if (!nrf_adc_conversion_finished())
		return;
	
	nrf_adc_conversion_event_clean();

	//Use the STOP task to save current. Workaround for PAN_028 rev1.2 anomaly 1.
  nrf_adc_stop();
}

#ifndef _CLINGBAND_2_PAY_MODEL_
#if defined(_ENABLE_UART_)
void UART0_IRQHandler(void)
{
}
#endif
#endif

#ifdef _CLINGBAND_2_PAY_MODEL_
#if defined(_ENABLE_RTT_) || defined(ISO7816_ENABLE)
#include "nrf_drv_uart.h"
void tx_sucess_int (void);
void rx_sucess_int (uint8_t data);
void _config_iso7816_output(void);
void UART0_IRQHandler(void)
{
#if defined(ISO7816_ENABLE)
	 if (NRF_UART0->EVENTS_ERROR != 0)
    {
			  NRF_UART0->ERRORSRC = 0xff;
        //nrf_uart_event_clear(NRF_UART0, NRF_UART_EVENT_TXDRDY);
       // nrf_uart_event_clear(NRF_UART0, NRF_UART_EVENT_RXTO);
        nrf_uart_event_clear(NRF_UART0, NRF_UART_EVENT_ERROR);  /* thanks for correction Lalit*/
				if((NRF_UART0->INTENSET&UART_INTENCLR_TXDRDY_Msk) == UART_INTENCLR_TXDRDY_Msk){
						/*check under which circumstance this error happened*/
						//nrf_uart_disable(NRF_UART0);
						_config_iso7816_output();
					//	nrf_uart_enable(NRF_UART0);
					  NRF_UART0->EVENTS_TXDRDY = 0;
						tx_sucess_int ();
				
				}else if((NRF_UART0->INTENSET&UART_INTENCLR_RXDRDY_Msk) == UART_INTENCLR_RXDRDY_Msk){
				}
    }
		if ( NRF_UART0->EVENTS_RXDRDY != 0 && ((NRF_UART0->INTENSET&UART_INTENCLR_RXDRDY_Msk) == UART_INTENCLR_RXDRDY_Msk)) {
        NRF_UART0->EVENTS_RXDRDY = 0;
				uint8_t t =  NRF_UART0->RXD;
        rx_sucess_int (t);

    }
		if(NRF_UART0->EVENTS_TXDRDY != 0 && ((NRF_UART0->INTENSET&UART_INTENCLR_TXDRDY_Msk) == UART_INTENCLR_TXDRDY_Msk)) {
        NRF_UART0->EVENTS_TXDRDY = 0;
        tx_sucess_int ();

    }
#endif
}
#endif
#endif

#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)
void GPIOTE_IRQHandler(void)
{
	// Event ausing the interrupt must be cleared
	if (NRF_GPIOTE->EVENTS_PORT) {
		NRF_GPIOTE->EVENTS_PORT = 0;
	}
}
#endif
