/***************************************************************************//**
 * File gpio.h
 * 
 * Description: User Interface header
 *
 *
 ******************************************************************************/

#ifndef __GPIO_HEADER__
#define __GPIO_HEADER__

#include "standards.h"

#define _LEMON_DVT2_

#ifdef _LEMON_DVT2_

#define GPIO_NFC_CLK         0
#define GPIO_SENSOR_INT_1    1
#define GPIO_SPI_0_CS_ACC    2
#define GPIO_CHARGER_IN      3
#define GPIO_TEMPERATURE_POWER_ON 4

#define GPIO_THERMISTOR_IN   5
#define GPIO_SPI_0_CS_OLED   6
#define GPIO_OLED_RST        7
#define GPIO_UART_TX         8
#define GPIO_VIBRATOR_EN     9

#define GPIO_TOUCH_CONTROL   10
#define GPIO_TOUCH_INT       11
#define GPIO_HSSP_XRST       12
#define GPIO_RESERVE_13      13
#define GPIO_RESERVE_14      14

#define GPIO_TWI_CLK         15
#define GPIO_TWI_DATA        16
#define GPIO_RESERVE_17      17
#define GPIO_CHARGER_CHGFLAG 18
#define GPIO_CHARGER_EN      19

#define GPIO_CHARGER_SD      20
#define GPIO_HOMEKEY         21
#define GPIO_SPI_0_CS_NFLASH 22
#define GPIO_NFC_RESET       23
#define GPIO_TWI0_CLK        24

#define GPIO_SPI_0_MISO      25
#define GPIO_TWI0_DATA       26
#define GPIO_SPI_0_MOSI      27
#define GPIO_SPI_0_SCK       28
#define GPIO_NFC_DATA        29

#define GPIO_UART_RX         30
#define GPIO_OLED_A0         31


#if 0
#define GPIO_SPI_0_CS_FONT   0
#define GPIO_PPG_AFE_PDN     15
#define GPIO_PPG_AFE_RST     16
#define GPIO_ADC_RDY         17
#define GPIO_SPI_1_SCK       18
#define GPIO_SPI_1_CS_PPG    20
#define GPIO_SPI_1_MISO      25
#define GPIO_SPI_1_MOSI      27
#endif

#define GPIO_UART_CTS_FAKE   2
#define GPIO_UART_RTS_FAKE	 2

#endif

#ifdef _LEMON_DVT_

#define GPIO_SPI_0_CS_FONT   0
#define GPIO_OLED_RST        1
#define GPIO_SPI_0_CS_OLED   2
#define GPIO_OLED_A0         3
#define GPIO_CHARGER_SD      4

#define GPIO_CHARGER_IN      5
#define GPIO_THERMISTOR_IN   6
#define GPIO_CHARGER_EN      7
#define GPIO_UART_TX         8
#define GPIO_VIBRATOR_EN     9

#define GPIO_HOMEKEY         10
#define GPIO_SPI_0_CS_NFLASH 11
#define GPIO_UART_RX         12
#define GPIO_TEMPERATURE_POWER_ON 13
#define GPIO_SPI_1_MISO      14

#define GPIO_PPG_AFE_PDN     15
#define GPIO_PPG_AFE_RST     16
#define GPIO_ADC_RDY         17
#define GPIO_SPI_1_SCK       18
#define GPIO_SPI_1_MOSI      19

#define GPIO_SPI_1_CS_PPG    20
#define GPIO_TWI_DATA        21
#define GPIO_TWI_CLK         22
#define GPIO_HSSP_XRST       23
#define GPIO_TOUCH_INT       24

#define GPIO_TOUCH_CONTROL   25
#define GPIO_SPI_0_SCK       26
#define GPIO_SPI_0_MOSI      27
#define GPIO_SPI_0_MISO      28
#define GPIO_SENSOR_INT_1    29


#define GPIO_SPI_0_CS_ACC    30
#define GPIO_CHARGER_CHGFLAG 31

#define GPIO_UART_CTS_FAKE   2
#define GPIO_UART_RTS_FAKE	 2

#endif

#ifdef _LEMON_EVALUATION_

#define GPIO_CHARGER_SD             0
#define GPIO_CHARGER_IN             1
#define GPIO_SPI_1_MOSI             2
#define GPIO_UART_CTS   	          2
#define GPIO_UART_RTS   		        2
#define GPIO_ADC_RDY                3
#define GPIO_THERMISTOR_IN          4

#define GPIO_VIBRATOR_EN            5
#define GPIO_TWI_DATA               6
#define GPIO_TWI_CLK                7
#define GPIO_TEMPERATURE_POWER_ON   8
#define GPIO_SENSOR_INT_1           9

#define GPIO_PPG_AFE_PDN            10
#define GPIO_HSSP_XRST              11
#define GPIO_CHARGER_CHGFLAG        12
#define GPIO_PPG_AFE_RST            13
#define GPIO_SPI_1_CS_PPG           14

#define GPIO_CHARGER_EN             15
#define GPIO_OLED_RST               16
#define GPIO_UART_RX                17
#define GPIO_UART_TX                18
#define GPIO_TOUCH_INT              19

#define GPIO_TOUCH_CONTROL          20
#define GPIO_SPI_0_MOSI             21
#define GPIO_SPI_0_SCK              22
#define GPIO_NFLASH_WP              23
#define GPIO_SPI_0_MISO             24

#define GPIO_SPI_0_CS_NFLASH        25
#define GPIO_SPI_0_CS_ACC           26
#define GPIO_SPI_0_CS_OLED          27
#define GPIO_OLED_A0                28
#define GPIO_HOMEKEY                29

#define GPIO_SPI_1_SCK              30
#define GPIO_SPI_1_MISO             31
#endif

// Initialization
void GPIO_init(void);

// ADC analog pin configuration for battery
void GPIO_vbat_adc_config(void);
void GPIO_vbat_adc_disable(void);

// ADC analog pin configuration for thermistor
void GPIO_therm_adc_enable(void);
void GPIO_therm_adc_disable(void);
void GPIO_therm_power_on(void);

// TWI pin configuration
void GPIO_twi_init_config(I8U twi_master_instance);
void GPIO_twi_output_config(I8U twi_master_instance);
void GPIO_twi_disabled(I8U twi_master_instance);

// SPI pin configuration
void GPIO_spi_disabled(I8U spi_no);
void GPIO_spi_init_config(I8U spi_no);

// Vibrator control
void GPIO_vibrator_set(BOOLEAN b_on);

// Interrupt control
void GPIO_interrupt_enable(void);
void GPIO_spi1_disconnect(void);

// Charger reset
void GPIO_charger_reset(void);
void GPIO_system_powerup(void);
void GPIO_system_powerdown(void);

#endif
