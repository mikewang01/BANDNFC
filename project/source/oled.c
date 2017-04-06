#include "main.h"
#ifdef _CLINGBAND_2_PAY_MODEL_
#include "spidev_hal.h"
#endif

extern I8U g_spi_tx_buf[];
extern I8U g_spi_rx_buf[];

#ifdef _CLINGBAND_2_PAY_MODEL_
static void oled_tx_rx(spi_master_hw_instance_t   spi_master_instance,
                          I8U * tx_data_buf, I16U tx_data_size,
                          I8U * rx_data_buf, I16U rx_data_size, I8U pin_cs)
{
	(void)spi_master_instance;
	(void)pin_cs;
	CLASS(SpiDevHal)* p =  SpiDevHal_get_instance();
	spi_dev_handle_t t = p->oled_open(p);
	p->write_read(p, t, tx_data_buf, tx_data_size, rx_data_buf, rx_data_size);
	p->close(p, t);
}
#endif

/**@brief Function for OLED write command.
 *
 */
static void _set_reg(uint8_t command)
{    
	g_spi_tx_buf[0] = command;

	nrf_gpio_pin_clear(GPIO_OLED_A0);
	
#ifdef _CLINGBAND_2_PAY_MODEL_
	oled_tx_rx(SPI_MASTER_0, g_spi_tx_buf, 1, g_spi_rx_buf, 0, GPIO_SPI_0_CS_OLED);
#else 
	SPI_master_tx_rx(SPI_MASTER_0, g_spi_tx_buf, 1, g_spi_rx_buf, 0, GPIO_SPI_0_CS_OLED);	
#endif	
}

static void _set_data(I16U num, I8U *data)
{
	nrf_gpio_pin_set(GPIO_OLED_A0);
	
#ifdef _CLINGBAND_2_PAY_MODEL_	
	oled_tx_rx(SPI_MASTER_0, data, num,g_spi_rx_buf,0, GPIO_SPI_0_CS_OLED);  
#else 
	SPI_master_tx_rx(SPI_MASTER_0, data, num,g_spi_rx_buf,0, GPIO_SPI_0_CS_OLED);  
#endif	
}

void OLED_power_on(void) 
{
#ifndef _CLING_PC_SIMULATION_
	nrf_gpio_pin_set( GPIO_OLED_RST ); // reset driven high
#endif
}

void OLED_power_off()
{
#ifndef _CLING_PC_SIMULATION_
	// Pull down reset pin
	nrf_gpio_pin_clear( GPIO_OLED_RST ); // reset driven high
#endif
}

void OLED_full_scree_show()
{
#ifndef _CLING_PC_SIMULATION_
	I8U i;
	I8U *data;
	
	data = cling.ui.p_oled_up;
	
	for (i = 0; i < 4; i++) {
		_set_reg(0xb0+i);
		_set_reg(0x10);  // Starting address
		_set_reg(0x00);  // Bug fix - start from 0
		// Set "A0" pin for data buffer refresh
		_set_data(128,data);
		
		data += 128;
	}
#endif
}

void OLED_set_display(I8U on_off)
{
	if (on_off)
		_set_reg(0xAF);
	else
		_set_reg(0xAE);
}

void OLED_set_contrast(I8U step)
{
	 _set_reg(0x81);
   _set_reg(step);
}

/**@brief Function for OLED init.
 *
 */
void OLED_init(I8U contrast)
{    
#ifndef _CLING_PC_SIMULATION_

    //BASE_delay_msec(100);
    //nrf_gpio_pin_set(GPIO_OLED_RST);	
    _set_reg(0xAE);     	
    _set_reg(0xd5);    
    _set_reg(0xC1);     
    _set_reg(0xA8);
    _set_reg(0x1F);
	  _set_reg(0xAD);
	  _set_reg(0x00);
		_set_reg(0x20);
	  _set_reg(0x02);

    _set_reg(0xD3);    
    _set_reg(0x00);
    _set_reg(0x40);     
	  _set_reg(0x8D);
	  _set_reg(0x8D);
	  _set_reg(0x14);

	   _set_reg(0xA0);
	   _set_reg(0xC8);
	   _set_reg(0xDA);
	   _set_reg(0x12);			
		   //_set_reg(0x81);
	     //_set_reg(0x40);				
		 OLED_set_contrast(contrast); 
		 _set_reg(0xD9);
	   _set_reg(0x22);			
     _set_reg(0xDB);    
     _set_reg(0x00);   
     _set_reg(0xA4);     
     _set_reg(0xA6);    
   // _set_reg(0xAF);  		
#endif
}

void OLED_set_panel_off()
{
	CLING_OLED_CTX *o = &cling.oled;
	
	o->state = OLED_STATE_GOING_OFF;
}

BOOLEAN OLED_set_panel_on()
{
#ifndef _CLING_PC_SIMULATION_
	CLING_OLED_CTX *o = &cling.oled;

	// Start 20 ms timer for screen rendering
	RTC_start_operation_clk();
	
	// We are about to turn on OLED, if BLE is in idle mode, we should start advertising
	if (BTLE_is_idle()) {
		BTLE_execute_adv(TRUE);
	}
	
	if (OLED_STATE_IDLE == o->state) {
		o->state = OLED_STATE_APPLYING_POWER;
		
		Y_SPRINTF("[OLED] panel is turned on");
		
		return TRUE;
	} 
#endif
	return FALSE;
}

BOOLEAN OLED_panel_is_turn_off()
{
	CLING_OLED_CTX *o = &cling.oled;
	
	if (o->state == OLED_STATE_IDLE) {
		return TRUE;
	}
	
	return FALSE;
}

BOOLEAN OLED_panel_is_turn_on()
{
	CLING_OLED_CTX *o = &cling.oled;
	
	if (o->state == OLED_STATE_ON) {
		return TRUE;
	}
	
	return FALSE;
}

// This routine implements the main display state machine.  If someone wants to write something to the
// display, it first issues the command SSD1306_SetDisplay(
//static I8U oledstate;
void OLED_state_machine(void) 
{
#ifndef _CLING_PC_SIMULATION_
	I32U t_curr;
	CLING_OLED_CTX *o = &cling.oled;
	t_curr = CLK_get_system_time();
	
	N_SPRINTF("[OLED] new oled state: %d @ %d", o->state, CLK_get_system_time());
	
	if (o->state != OLED_STATE_IDLE) {
		// Start system timer
		RTC_start_operation_clk();
	}

	switch (o->state) {
		case OLED_STATE_IDLE:
			break;
		case OLED_STATE_APPLYING_POWER:
		{
#ifdef _CLINGBAND_2_PAY_MODEL_			
			pm_stay_alive();
#endif			
			// turn on the power pins and wait the appropriate time
			o->ts = CLK_get_system_time(); // stores time we start this
			OLED_power_on();
			o->state = OLED_STATE_RESET_OLED;
			break;
		}
		case OLED_STATE_RESET_OLED:
		{
			if ((t_curr - o->ts) > OLED_POWER_START_DELAY_TIME){
				// Pulse the reset low for the minimum time.
				nrf_gpio_pin_clear(GPIO_OLED_RST);
				// now pulse reset for at least 3us
				BASE_delay_msec(2);
				nrf_gpio_pin_set(GPIO_OLED_RST);
				o->state = OLED_STATE_INIT_REGISTERS;
			}
			break;
		}
		case OLED_STATE_INIT_REGISTERS:
		{
			OLED_init(0xf0);
			o->state = OLED_STATE_INIT_UI;
			break;
		}
		case OLED_STATE_INIT_UI:
		{
				// Update display timeout base.
				cling.ui.display_to_base = CLK_get_system_time();
			
				N_SPRINTF("[0LED] ui to base: %d", cling.ui.display_to_base);
				
				// Reset blinking state
				cling.ui.icon_sec_blinking = TRUE;
			
			  // Update touch time stamp.
	      cling.ui.touch_time_stamp = CLK_get_system_time();			
 
			  // Update ui display base time stamp.
        cling.ui.display_to_base = CLK_get_system_time();  			
				o->state = OLED_STATE_ON;
			break;
		}
		case OLED_STATE_ON:
			break;
		case OLED_STATE_GOING_OFF:
		{
			OLED_set_display(0);    /*display off*/
			OLED_power_off();
			o->ts = CLK_get_system_time(); // stores time we start this
			o->state = OLED_STATE_OFF;
#ifdef _CLINGBAND_2_PAY_MODEL_						
		  pm_resume_sleep();
#endif			
			break;
		}
		case OLED_STATE_OFF:
		{
			// don't let power come back on for 100ms
			if (CLK_get_system_time() - o->ts > OLED_POWER_OFF_DELAY_TIME){
				o->state = OLED_STATE_IDLE;
			}
			break;
		}
		default:
			break;
	}
#endif
}


