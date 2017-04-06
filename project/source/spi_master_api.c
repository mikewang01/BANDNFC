/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#include "main.h"
#include "nrf.h"
#include "app_util.h"
#include "app_util_platform.h"
#include "nrf_error.h"
#include "nrf_drv_spi.h"

uint8_t g_spi_tx_buf[10];
uint8_t g_spi_rx_buf[10];

static const nrf_drv_spi_t m_spi_master_0 = NRF_DRV_SPI_INSTANCE(0);

static BOOLEAN  m_transfer_completed = TRUE;

void SPI_master_disable()
{
	if (cling.system.b_spi_0_ON) {
		nrf_drv_spi_uninit(&m_spi_master_0);
		cling.system.b_spi_0_ON = FALSE;
	}

	N_SPRINTF("[SPI] deinit ...");
}

static void _spi_master_op_wait_done()
{
	I16U i;
	
	if (m_transfer_completed)
		return;
	
	// Wait for up to 200 ms
	for (i = 0; i < 200; i++) {
		BASE_delay_msec(1);
		
		if (m_transfer_completed)
			break;
	}
	
  if(!m_transfer_completed)
	Y_SPRINTF("[SPI] transfer incompleted, wait for 200ms");
}

// When spi bus transfer failed,pull up all cs pin.
static void _spi_master_clear_cs_pin()
{
	nrf_gpio_cfg_output(GPIO_SPI_0_CS_ACC);
	nrf_gpio_pin_set(GPIO_SPI_0_CS_ACC);
  
	nrf_gpio_cfg_output(GPIO_SPI_0_CS_OLED);
	nrf_gpio_pin_set(GPIO_SPI_0_CS_OLED);

	nrf_gpio_cfg_output(GPIO_SPI_0_CS_NFLASH);
	nrf_gpio_pin_set(GPIO_SPI_0_CS_NFLASH);
	
#ifdef _CLINGBAND_2_PAY_MODEL_	
	nrf_gpio_cfg_output(GPIO_SPI_0_CS_FM1280B);
	nrf_gpio_pin_set(GPIO_SPI_0_CS_FM1280B);
#endif	
}

/**@brief Handler for SPI0 master events.
 *
 * @param[in] event SPI master event.
 */
void spi_master_0_event_handler(nrf_drv_spi_event_t event)
{
	switch (event)
	{
		case NRF_DRV_SPI_EVENT_DONE:
				m_transfer_completed = TRUE;
				break;

		default:
				// No implementation needed.
				break;
	}
}

/**@brief Function for initializing a SPI master driver.
 */
void SPI_master_init(spi_master_hw_instance_t   spi_master_instance, BOOLEAN lsb, nrf_drv_spi_mode_t mode)
{
	I32U err_code = NRF_SUCCESS;

	nrf_drv_spi_config_t config ;
	
	config.sck_pin      = SPIM0_SCK_PIN;
	config.mosi_pin     = SPIM0_MOSI_PIN;
	config.miso_pin     = SPIM0_MISO_PIN;
	config.ss_pin       = NRF_DRV_SPI_PIN_NOT_USED;
  config.irq_priority = APP_IRQ_PRIORITY_LOW;	
	config.orc          = 0xCC;
	config.frequency    = NRF_DRV_SPI_FREQ_8M;		
//config.mode         = NRF_DRV_SPI_MODE_0;
	config.mode         = mode;
	config.bit_order    = (lsb ?  NRF_DRV_SPI_BIT_ORDER_LSB_FIRST : NRF_DRV_SPI_BIT_ORDER_MSB_FIRST);

	if (spi_master_instance == SPI_MASTER_0)
	{
		err_code = nrf_drv_spi_init(&m_spi_master_0, &config, spi_master_0_event_handler);
		
		if (err_code == NRF_ERROR_INVALID_STATE) {
				Y_SPRINTF("[SPI] Uninitialized - figure out why!");
				nrf_drv_spi_uninit(&m_spi_master_0);
				err_code = nrf_drv_spi_init(&m_spi_master_0, &config, spi_master_0_event_handler);
		}
		APP_ERROR_CHECK(err_code);
	}
}


/**@brief Function for sending and receiving data.
*/
void SPI_master_tx_rx(spi_master_hw_instance_t   spi_master_instance,
                          I8U * tx_data_buf, I16U tx_data_size,
                          I8U * rx_data_buf, I16U rx_data_size, I8U pin_cs)
{
  I32U err_code;
	
	_spi_master_op_wait_done();

	if(!cling.system.b_spi_0_ON){
		N_SPRINTF("[SPI] re-init: %d", cling.system.b_spi_0_ON);
	  SPI_master_init(spi_master_instance,false, NRF_DRV_SPI_MODE_0);
		cling.system.b_spi_0_ON = TRUE;				
	}

	if(!m_transfer_completed){
		Y_SPRINTF("[SPI] transfer failed, disable SPI bus and re-init SPI");
		nrf_drv_spi_uninit(&m_spi_master_0);
	  SPI_master_init(spi_master_instance,false, NRF_DRV_SPI_MODE_0);
		// To ensure all cs pin pull up, prevent mutual interference.
		_spi_master_clear_cs_pin();
	}

	m_transfer_completed = FALSE;
	
	for(I8U i=0;i<10;i++)
	{
    err_code = nrf_drv_spi_transfer(&m_spi_master_0,tx_data_buf, tx_data_size, rx_data_buf, rx_data_size,pin_cs);
		if(err_code == NRF_SUCCESS)
		   break;
	  else{
			nrf_drv_spi_uninit(&m_spi_master_0);
	    SPI_master_init(spi_master_instance,false, NRF_DRV_SPI_MODE_0);
		  BASE_delay_msec(20);
	  }
	}
  
  APP_ERROR_CHECK(err_code);
}
