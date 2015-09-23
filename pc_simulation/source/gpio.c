/***************************************************************************/
/**
 * File: gpio.c
 * 
 * Description: All GPIO related for Wristband device
 *
 * Created on Jan 6, 2014
 *
 ******************************************************************************/

#include "main.h"
#include "gpio.h"

#ifndef _CLING_PC_SIMULATION_
static __INLINE void _gpio_cfg_output(uint32_t pin_number, BOOLEAN b_drive)
#else
static void _gpio_cfg_output(uint32_t pin_number, BOOLEAN b_drive)
#endif
{
#ifndef _CLING_PC_SIMULATION_
	// Output pin, we need to disconnect it from input buffer.
    NRF_GPIO->PIN_CNF[pin_number] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                                        | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
                                        | (NRF_GPIO_PIN_NOPULL << GPIO_PIN_CNF_PULL_Pos)
                                        | (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                                        | (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);
	if (b_drive) {
		nrf_gpio_pin_set(pin_number);
	} else {
		nrf_gpio_pin_clear(pin_number);
	}
#endif
}

#ifndef _CLING_PC_SIMULATION_
static __INLINE void _gpio_cfg_disconnect_input(uint32_t pin_number)
#else
static void _gpio_cfg_disconnect_input(uint32_t pin_number)
#endif
{
#ifndef _CLING_PC_SIMULATION_
	// when system is disconnected from input buffer, none of other parameter matters
    NRF_GPIO->PIN_CNF[pin_number] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                                        | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
                                        | (NRF_GPIO_PIN_NOPULL << GPIO_PIN_CNF_PULL_Pos)
                                        | (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                                        | (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
#endif
}

#ifndef _CLING_PC_SIMULATION_
static __INLINE void _gpio_cfg_connect_input(uint32_t pin_number, nrf_gpio_pin_sense_t sense_config, nrf_gpio_pin_pull_t pull_config)
{
	// Always enable the input buffer in a separate write operation, before enabling the sense functionality
	NRF_GPIO->PIN_CNF[pin_number] = (NRF_GPIO_PIN_NOSENSE << GPIO_PIN_CNF_SENSE_Pos)
																			| (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
																			| (pull_config << GPIO_PIN_CNF_PULL_Pos)
																			| (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
																			| (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
	
	if (sense_config != NRF_GPIO_PIN_NOSENSE) {
    // when system is connected to input buffer, we need to configure the sensing and pull
		// Note: drive strength only matters to output
    NRF_GPIO->PIN_CNF[pin_number] = (sense_config << GPIO_PIN_CNF_SENSE_Pos)
                                        | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
                                        | (pull_config << GPIO_PIN_CNF_PULL_Pos)
                                        | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
                                        | (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
	}
}
#endif

void GPIO_system_powerup()
{
	_gpio_cfg_output(GPIO_CHARGER_SD      , TRUE);     // Charger Shut down pin, pull UP to power up system
	
	cling.system.b_powered_up = TRUE;
}

void GPIO_system_powerdown()
{
	return;
	_gpio_cfg_output(GPIO_CHARGER_SD      , FALSE);     // Charger Shut down pin, pull down
	cling.system.b_powered_up = FALSE;
}

void GPIO_init()
{
#ifndef _CLING_PC_SIMULATION_

	_gpio_cfg_disconnect_input(GPIO_CHARGER_IN      );  // ADC analog pin
  _gpio_cfg_disconnect_input(GPIO_THERMISTOR_IN   );  // thermistor input
	#if 0
	_gpio_cfg_disconnect_input(GPIO_ADC_RDY         );  // PPG pin, not used for now
#endif
	_gpio_cfg_output(GPIO_VIBRATOR_EN, FALSE);          // Vibrator pin, pull down
	_gpio_cfg_disconnect_input(GPIO_TWI0_DATA       );  // I2C
	_gpio_cfg_disconnect_input(GPIO_TWI0_CLK        );  // I2C
	_gpio_cfg_disconnect_input(GPIO_TWI_DATA        );  // I2C
	_gpio_cfg_disconnect_input(GPIO_TWI_CLK         );  // I2C
	_gpio_cfg_disconnect_input(GPIO_TEMPERATURE_POWER_ON );  // Temperature ready pin, not used
	_gpio_cfg_connect_input(GPIO_SENSOR_INT_1    , NRF_GPIO_PIN_SENSE_HIGH, NRF_GPIO_PIN_NOPULL); // Needed, sense high, no pull
		_gpio_cfg_connect_input(GPIO_TOUCH_INT       , NRF_GPIO_PIN_SENSE_LOW, NRF_GPIO_PIN_PULLUP);  // Touch pin, sense low, no pull
	_gpio_cfg_connect_input(GPIO_CHARGER_CHGFLAG , NRF_GPIO_PIN_NOSENSE, NRF_GPIO_PIN_PULLUP); // Charger Flag, no sense, internal pull up
	#if 0

	_gpio_cfg_output(GPIO_PPG_AFE_PDN     , TRUE);      // PPG power down pin, pull high
	_gpio_cfg_disconnect_input(GPIO_PPG_AFE_RST     );  // PPG reset pin, not used
	_gpio_cfg_output(GPIO_SPI_1_CS_PPG    , TRUE);      // SPI CS PPG, drive HIGH
	#endif

	_gpio_cfg_output(GPIO_CHARGER_EN      , TRUE);      // Charge enable, drive HIGH
	_gpio_cfg_output(GPIO_OLED_RST        , TRUE);      // OLED reset, drive HIGH
	_gpio_cfg_disconnect_input(GPIO_UART_RX         );  // UART pin, not used
	_gpio_cfg_disconnect_input(GPIO_UART_TX         );  // UART pin, not used
	_gpio_cfg_output(GPIO_HSSP_XRST       , FALSE);     // Touch IC, Reset, drive LOW (deasset the RESET pin)
	
	_gpio_cfg_output(GPIO_TOUCH_CONTROL   , TRUE);      // Touch control, drive HIGH
	_gpio_cfg_disconnect_input(GPIO_SPI_0_MOSI      );  // SPI bus, not used
	_gpio_cfg_disconnect_input(GPIO_SPI_0_SCK       );  // SPI bus, not used
	_gpio_cfg_disconnect_input(GPIO_SPI_0_MISO      );  // SPI bus, not used
	#if 0
	_gpio_cfg_output(GPIO_SPI_0_CS_FONT   , TRUE);      // SPI CS (FONT), drive HIGH
	#endif
	_gpio_cfg_output(GPIO_SPI_0_CS_NFLASH , TRUE);      // SPI CS (NOR FLASH), drive HIGH
	_gpio_cfg_output(GPIO_SPI_0_CS_ACC    , TRUE);      // SPI CS (ACCC), drive HIGH
	_gpio_cfg_output(GPIO_SPI_0_CS_OLED   , TRUE);      // SPI CS (OLED), drive HIGH
	_gpio_cfg_output(GPIO_OLED_A0         , FALSE);     // OLED A0, drive LOW
	_gpio_cfg_connect_input(GPIO_HOMEKEY      , NRF_GPIO_PIN_NOSENSE, NRF_GPIO_PIN_PULLUP);
	#if 0
	_gpio_cfg_disconnect_input(GPIO_SPI_1_SCK     );  // GPIO 30, not used
	_gpio_cfg_disconnect_input(GPIO_SPI_1_MISO     );  // GPIO 31, not used
	#endif
#endif
}

void GPIO_interrupt_enable()
{
#ifndef _CLING_PC_SIMULATION_
	NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_PORT_Msk;
	sd_nvic_SetPriority(GPIOTE_IRQn, NRF_APP_PRIORITY_LOW);
	sd_nvic_EnableIRQ(GPIOTE_IRQn);
#endif
}

void GPIO_spi_disabled(I8U spi_no)
{
#ifndef _CLING_PC_SIMULATION_
	// Configure input, and disconnect it from input buffer
	if ( spi_no == SPI_MASTER_0 )
	{
		NRF_SPI0->ENABLE = SPI_ENABLE_ENABLE_Disabled << SPI_ENABLE_ENABLE_Pos;
		
		_gpio_cfg_disconnect_input(GPIO_SPI_0_SCK        );  // spi
		_gpio_cfg_disconnect_input(GPIO_SPI_0_MOSI        );  // spi
		_gpio_cfg_disconnect_input(GPIO_SPI_0_MISO        );  // spi
	}
	else
	{
		#if 0
		NRF_SPI1->ENABLE = SPI_ENABLE_ENABLE_Disabled << SPI_ENABLE_ENABLE_Pos;
		
		_gpio_cfg_disconnect_input(GPIO_SPI_1_SCK        );  // spi
		_gpio_cfg_disconnect_input(GPIO_SPI_1_MOSI        );  // spi
		_gpio_cfg_disconnect_input(GPIO_SPI_1_MISO        );  // spi
		#endif
	}
#endif
}

void GPIO_spi_init_config(I8U spi_no)
{
#ifndef _CLING_PC_SIMULATION_
	if (spi_no == SPI_MASTER_0) {
		// Configure spi bus
		_gpio_cfg_output(GPIO_SPI_0_SCK         , FALSE);     // spi
		_gpio_cfg_output(GPIO_SPI_0_MOSI        , FALSE);  // spi
		_gpio_cfg_connect_input(GPIO_SPI_0_MISO , NRF_GPIO_PIN_NOSENSE, NRF_GPIO_PIN_NOPULL); // spi
#if 0    
		_gpio_cfg_output(GPIO_SPI_0_CS_FONT   , TRUE); 
#endif
		_gpio_cfg_output(GPIO_SPI_0_CS_OLED, TRUE);
		_gpio_cfg_output(GPIO_SPI_0_CS_ACC, TRUE);
		_gpio_cfg_output(GPIO_SPI_0_CS_NFLASH, TRUE);
	} else {
		#if 0
		_gpio_cfg_output(GPIO_SPI_1_SCK         , FALSE);     // spi
		_gpio_cfg_output(GPIO_SPI_1_MOSI        , FALSE);  // spi
		_gpio_cfg_connect_input(GPIO_SPI_1_MISO , NRF_GPIO_PIN_NOSENSE, NRF_GPIO_PIN_NOPULL); // spi
		
		_gpio_cfg_output(GPIO_SPI_1_CS_PPG, TRUE);
		#endif
	}
#endif
}

void GPIO_twi_disabled(I8U twi_master_instance)
{
#ifndef _CLING_PC_SIMULATION_
	if (twi_master_instance==TWI_MASTER_0) {
		NRF_TWI0->ENABLE = TWI_ENABLE_ENABLE_Disabled << TWI_ENABLE_ENABLE_Pos;
		_gpio_cfg_disconnect_input(GPIO_TWI0_DATA);
		_gpio_cfg_disconnect_input(GPIO_TWI0_CLK);
	} else if (twi_master_instance==TWI_MASTER_1) {
		NRF_TWI1->ENABLE = TWI_ENABLE_ENABLE_Disabled << TWI_ENABLE_ENABLE_Pos;
		_gpio_cfg_disconnect_input(GPIO_TWI_DATA);
		_gpio_cfg_disconnect_input(GPIO_TWI_CLK);
	}
#endif
}

void GPIO_twi_init_config(I8U twi_master_instance)
{
#ifndef _CLING_PC_SIMULATION_
	if (twi_master_instance==TWI_MASTER_0) {
    NRF_GPIO->PIN_CNF[GPIO_TWI0_CLK] = 
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
      | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos)
      | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)
      | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos)
      | (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos);    

    NRF_GPIO->PIN_CNF[GPIO_TWI0_DATA] = 
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
      | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos)
      | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)
      | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos)
      | (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos);    
	} else if (twi_master_instance==TWI_MASTER_1) {
    NRF_GPIO->PIN_CNF[GPIO_TWI_CLK] = 
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
      | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos)
      | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)
      | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos)
      | (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos);    

    NRF_GPIO->PIN_CNF[GPIO_TWI_DATA] = 
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
      | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos)
      | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)
      | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos)
      | (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos);    
	}	
#endif
}

void GPIO_twi_output_config(I8U twi_master_instance)
{
#ifndef _CLING_PC_SIMULATION_
	if (twi_master_instance==TWI_MASTER_0) {
    NRF_GPIO->PIN_CNF[GPIO_TWI0_CLK] =   \
            (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) \
          | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos) \
          | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)  \
          | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos) \
          | (GPIO_PIN_CNF_DIR_Output     << GPIO_PIN_CNF_DIR_Pos);    

    NRF_GPIO->PIN_CNF[GPIO_TWI0_DATA] = \
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) \
      | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos) \
      | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)  \
      | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos) \
      | (GPIO_PIN_CNF_DIR_Output     << GPIO_PIN_CNF_DIR_Pos);    
	} else if (twi_master_instance==TWI_MASTER_1) {
    NRF_GPIO->PIN_CNF[GPIO_TWI_CLK] =   \
            (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) \
          | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos) \
          | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)  \
          | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos) \
          | (GPIO_PIN_CNF_DIR_Output     << GPIO_PIN_CNF_DIR_Pos);    

    NRF_GPIO->PIN_CNF[GPIO_TWI_DATA] = \
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) \
      | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos) \
      | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)  \
      | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos) \
      | (GPIO_PIN_CNF_DIR_Output     << GPIO_PIN_CNF_DIR_Pos);    
	}
#endif  
}

void GPIO_vbat_adc_config()
{
#ifndef _CLING_PC_SIMULATION_
	// config ADC
	NRF_ADC->CONFIG	= (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos) /* Bits 17..16 : ADC external reference pin selection. */
										| (ADC_CONFIG_PSEL_AnalogInput4 << ADC_CONFIG_PSEL_Pos)					/*!< Use analog input 2 as analog input. */
										| (ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos)							/*!< Use internal 1.2V bandgap voltage as reference for conversion. */
										| (ADC_CONFIG_INPSEL_AnalogInputNoPrescaling << ADC_CONFIG_INPSEL_Pos) /*!< Analog input specified by PSEL with no prescaling used as input for the conversion. */
										| (ADC_CONFIG_RES_10bit << ADC_CONFIG_RES_Pos);									/*!< 10bit ADC resolution. */ 	
#endif
}

void GPIO_vbat_adc_disable()
{
#ifndef _CLING_PC_SIMULATION_
	NRF_ADC->TASKS_STOP = 1;
	NRF_ADC->ENABLE = 0;
	
	// Disable all the input analog pins
	NRF_ADC->CONFIG	= (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos) /* Bits 17..16 : ADC external reference pin selection. */
									| (ADC_CONFIG_PSEL_Disabled << ADC_CONFIG_PSEL_Pos)					/*! Analog input pins disabled. */
									| (ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos)							/*!< Use internal 1.2V bandgap voltage as reference for conversion. */
									| (ADC_CONFIG_INPSEL_AnalogInputNoPrescaling << ADC_CONFIG_INPSEL_Pos) /*!< Analog input specified by PSEL with no prescaling used as input for the conversion. */
									| (ADC_CONFIG_RES_10bit << ADC_CONFIG_RES_Pos);									/*!< 10bit ADC resolution. */ 	

	_gpio_cfg_disconnect_input(GPIO_CHARGER_IN      );  // ADC analog pin
#endif
}

void GPIO_therm_power_on()
{
	// power on
	_gpio_cfg_output(GPIO_TEMPERATURE_POWER_ON      , TRUE);      // thermistor power on, drive HIGH
}

void GPIO_therm_adc_enable()  // TBD
{
#ifndef _CLING_PC_SIMULATION_
	// config ADC
	_gpio_cfg_connect_input(GPIO_THERMISTOR_IN,    NRF_GPIO_PIN_NOSENSE,   NRF_GPIO_PIN_NOPULL      );

	// Configure ADC with analog input pin 5
	NRF_ADC->CONFIG	= (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos) /* Bits 17..16 : ADC external reference pin selection. */
										| (ADC_CONFIG_PSEL_AnalogInput5 << ADC_CONFIG_PSEL_Pos)					/*!< Use analog input 5 as analog input. */
										| (ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos)							/*!< Use internal 1.2V bandgap voltage as reference for conversion. */
										| (ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos) /*!< Analog input specified by PSEL with no prescaling used as input for the conversion. */
										| (ADC_CONFIG_RES_10bit << ADC_CONFIG_RES_Pos);									/*!< 10bit ADC resolution. */ 	

	// power on
	_gpio_cfg_output(GPIO_TEMPERATURE_POWER_ON      , TRUE);      // thermistor power on, drive HIGH
#endif
}

void GPIO_therm_adc_disable()
{
#ifndef _CLING_PC_SIMULATION_
	NRF_ADC->TASKS_STOP = 1;
	// Diabel ADC
	NRF_ADC->ENABLE = 0;
	
	// Disable all anolog pins
	NRF_ADC->CONFIG	= (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos) /* Bits 17..16 : ADC external reference pin selection. */
									| (ADC_CONFIG_PSEL_Disabled << ADC_CONFIG_PSEL_Pos)					/*!< Use analog input 5 as analog input. */
									| (ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos)							/*!< Use internal 1.2V bandgap voltage as reference for conversion. */
									| (ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos) /*!< Analog input specified by PSEL with no prescaling used as input for the conversion. */
									| (ADC_CONFIG_RES_10bit << ADC_CONFIG_RES_Pos);									/*!< 10bit ADC resolution. */ 	

	// Disconnect Theristor pin
	_gpio_cfg_disconnect_input(GPIO_THERMISTOR_IN      );  // ADC analog pin
	
	// Disonnect Thermistor power on
	_gpio_cfg_disconnect_input(GPIO_TEMPERATURE_POWER_ON);
#endif
}

void GPIO_vibrator_set(BOOLEAN b_on)
{
	if (b_on)
		_gpio_cfg_output(GPIO_VIBRATOR_EN, TRUE);
	else
		_gpio_cfg_output(GPIO_VIBRATOR_EN, FALSE);
}

void GPIO_spi1_disconnect()
{
	#if 0
	_gpio_cfg_disconnect_input(GPIO_PPG_AFE_RST       );
	_gpio_cfg_disconnect_input(GPIO_SPI_1_CS_PPG       );
	_gpio_cfg_disconnect_input(GPIO_ADC_RDY       );
#endif
	}

void GPIO_charger_reset()
{
#ifndef _CLING_PC_SIMULATION_

	_gpio_cfg_output(GPIO_CHARGER_EN      , FALSE);      // Charge enable, drive LOW
	nrf_delay_ms(100);
	_gpio_cfg_output(GPIO_CHARGER_EN      , TRUE);      // Charge enable, drive HIGH

#endif
}
