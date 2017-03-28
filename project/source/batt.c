#include "main.h"
#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_PACE_MODEL_)		 
#include "exti_hal.h"
#endif


#define ADC_4P2_VOLTS 646
#define ADC_4P0_VOLTS 616
#define ADC_3P8_VOLTS 585
#define ADC_3P4_VOLTS 523
#define ADC_3P2_VOLTS 492
#define ADC_3P0_VOLTS 462

#define BATT_SPRINTF Y_SPRINTF

#define BATTERY_PERCENTAGE_LEVEL 60

static const I16U battery_vol_tab[BATTERY_PERCENTAGE_LEVEL] = {
631, 630, 629, 628, 626, 623, 621, 619, 617, 615, 613, 611, 609, 607, 605, 603, 
601, 600, 598, 596, 595, 593, 591, 590, 588, 587, 585, 584, 583, 582, 580, 579, 
578, 577, 576, 576, 575, 574, 573, 573, 572, 571, 571, 570, 569, 569, 568, 568, 
567, 567, 565, 564, 564, 563, 562, 560, 559, 556, 555, 553};

static const I16U battery_perc_tab[BATTERY_PERCENTAGE_LEVEL] = {
100,98,97,95,93,92,90,88,87,85,83,82,80,78,77,75,
 73,72,70,68,67,65,63,62,60,58,57,55,53,52,50,48,
 47,45,43,42,40,38,37,35,33,32,30,28,27,25,23,22,
 20,18,17,15,13,12,10, 8, 7, 5, 3, 2};

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_PACE_MODEL_)		 
 // Charging IC I2C address: 0x09 (in 7-bit mode)
 #define CHARGING_I2C_ADDR 0x12 
 
static EN_STATUSCODE BATT_read_reg(I8U cmdID, I8U bytes, I8U *pRegVal)
{
#ifndef _CLING_PC_SIMULATION_

	I32U err_code;
	const nrf_drv_twi_t twi = NRF_DRV_TWI_INSTANCE(1);
	
	if (pRegVal==NULL) return STATUSCODE_NULL_POINTER;
	
	if (!cling.system.b_twi_1_ON) {
		GPIO_twi_enable(1);
	}
	
	err_code = nrf_drv_twi_tx(&twi, (CHARGING_I2C_ADDR>>1), &cmdID, 1, true);
	if (err_code == NRF_SUCCESS) {
		N_SPRINTF("BATT: Read TX PASS: ");
	} else {
		BATT_SPRINTF("BATT: Read TX FAIL - %d", err_code);
		APP_ERROR_CHECK(err_code);
		return STATUSCODE_FAILURE;
	}
	err_code = nrf_drv_twi_rx(&twi, (CHARGING_I2C_ADDR>>1), pRegVal, bytes, false);
	if (err_code == NRF_SUCCESS) {
		N_SPRINTF("BATT: Read RX PASS: ");
		return STATUSCODE_SUCCESS;
	} else {
		BATT_SPRINTF("BATT: Read RX FAIL: %d", err_code);
		APP_ERROR_CHECK(err_code);
		return STATUSCODE_FAILURE;
	}
#else
	return STATUSCODE_SUCCESS;
#endif
}

static BOOLEAN BATT_write_reg(I8U cmdID, I8U regVal)
{
#ifndef _CLING_PC_SIMULATION_

	I32U err_code;
	const nrf_drv_twi_t twi = NRF_DRV_TWI_INSTANCE(1);
	I8U acData[2];
	
	acData[0] = cmdID;
	acData[1] = regVal;
	
	if (!cling.system.b_twi_1_ON) {
		GPIO_twi_enable(1);
	}
	
	err_code = nrf_drv_twi_tx(&twi, (CHARGING_I2C_ADDR>>1), acData, 2, false);
	if (err_code == NRF_SUCCESS) {
		N_SPRINTF("BATT: Write PASS: 0x%02x  0x%02x", cmdID, regVal);
		return STATUSCODE_SUCCESS;
	} else {
		BATT_SPRINTF("BATT: Write FAIL(%d): 0x%02x  0x%02x", err_code, cmdID, regVal);
		APP_ERROR_CHECK(err_code);
		return STATUSCODE_FAILURE;
	}
#else
	return STATUSCODE_SUCCESS;
#endif
}

void BATT_charging_init()
{
//	I8U data[10];
	// Charging setup: 
	// termination voltage: 4.3 volts
	// constant current: 0.5% lower than the battery limit
	// constant current charging at 65 mA
	BATT_write_reg(0x01, 0x95);
	
	// LDO 1 -- ON
	BATT_write_reg(0x0A, 0xbc);
//	BATT_read_reg(0x0A, 1, &data[0]);
	// LDO 2 -- ON
	BATT_write_reg(0x0B, 0xbc);
	//BATT_read_reg(0x0B, 1, &data[1]);
	// LDO 3 -- ON
	BATT_write_reg(0x0C, 0xbc);
//	BATT_read_reg(0x0C, 1, &data[2]);
	// LDO 4 -- ON
#ifdef _CLINGBAND_PACE_MODEL_		
	BATT_write_reg(0x0D, 0x00);
#endif
#ifdef _CLINGBAND_2_PAY_MODEL_		
	BATT_write_reg(0x0D, 0xbc);
#endif	
//	BATT_read_reg(0x0D, 1, &data[3]);
//	BATT_SPRINTF("BATT REG: 0x%02x, 0x%02x, 0x%02x, 0x%02x", data[0], data[1], data[2], data[3]);
}

void BATT_charging_deinit()
{
	// Turn off LDO 1/2/3/4
	//BATT_write_reg(0x0A, 0x00); // LDO 1, POWER FOR CPU, TURN OFF AT LAST	
	BATT_write_reg(0x0B, 0x00);

//BATT_write_reg(0x0C, 0x00);

#ifdef _CLINGBAND_PACE_MODEL_	
	BATT_write_reg(0x0D, 0x00);
#endif
}
#endif

I8U BATT_get_level()
{
	return cling.system.mcu_reg[REGISTER_MCU_BATTERY];
}

BOOLEAN BATT_is_charging()
{
#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_PACE_MODEL_)		
	if (!cling.batt.charging_connected) {
#else		
	if (nrf_gpio_pin_read(GPIO_CHARGER_CHGFLAG)) {
#endif		
		return FALSE;
	} else {
		return TRUE;
	}
}

BOOLEAN BATT_charging_det_for_sleep()
{
	BATT_CTX *b = &cling.batt;
	
	if (b->charging_state == CHARGER_IN_CHARGING) {
		b->non_charging_accumulated_steps = 0;
		b->non_charging_accumulated_active_sec = 0;
		return TRUE;
	} else {
		
		if (b->non_charging_accumulated_steps > SLEEP_STEP_ACCUMULATED_THRESHOLD) {
			return FALSE;
		}
		
		if (b->non_charging_accumulated_active_sec > SLEEP_ACTIVE_SECONDS_THRESHOLD) {
			return FALSE;
		}
		
		return TRUE;
	}
}

void BATT_update_charging_time(I8U tick_in_s)
{
	if (BATT_is_charging()) {
		cling.batt.charging_overall_time += tick_in_s;
		
		if (cling.batt.charging_overall_time > BATTERY_MAXIMUM_CHARGING_TIME) {
			cling.batt.charging_overall_time = BATTERY_MAXIMUM_CHARGING_TIME;
		}
	}
}

void BATT_exit_charging_state(I8U sec)
{
	BATT_CTX *b = &cling.batt;
	
	if (b->charging_state == CHARGER_IN_CHARGING) {
		return;
	}
	
	if (cling.lps.b_low_power_mode)
		return;
	
	if (b->non_charging_accumulated_active_sec <= SLEEP_ACTIVE_SECONDS_THRESHOLD) {
		b->non_charging_accumulated_active_sec += sec;
	}
}

void BATT_charging_update_steps(I8U steps)
{
	BATT_CTX *b = &cling.batt;
	
	if (b->charging_state == CHARGER_IN_CHARGING) {
		return;
	}
	
	if (b->non_charging_accumulated_steps <= SLEEP_STEP_ACCUMULATED_THRESHOLD) {
		b->non_charging_accumulated_steps += steps;
	}
}

I8U _get_battery_perc()
{
	BATT_CTX *b = &cling.batt;
	I8U i, percent;

	// 
	percent = 0;
#ifndef _CLING_PC_SIMULATION_

	// Disable Charger SD 
	// If battery voltage is lower than a threshold (3.2 Volts), cut it off -
	if (b->volts_reading < ADC_3P2_VOLTS) {
		
		if (!BATT_is_charging()) {
			// No DC-in then, put device into a super-low power state
			//
			N_SPRINTF("[BATT] low power shut-down (reading: %d, )", b->volts_reading);
			
#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_PACE_MODEL_)					
		  /*disable all interrupts on top of charger int*/
			CLASS(HalExti)*	p_exti_instance = HalExti_get_instance();
			p_exti_instance->set_low_power(p_exti_instance);
#endif
			
			/*disconnect all gpio system*/
			GPIO_system_powerdown();
			// No need for a shutdown timer reconfiguration
			// RTC_system_shutdown_timer();
		}else{
#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_PACE_MODEL_)							
			/*enable all interrupts*/
			CLASS(HalExti)*	p_exti_instance = HalExti_get_instance();
			p_exti_instance->set_active(p_exti_instance);
#endif			
		}
	}
#endif
	// Look up table to get battery percentage
	for (i = 0; i < BATTERY_PERCENTAGE_LEVEL; i ++) {
		if (b->volts_reading > battery_vol_tab[i]) {
			percent = battery_perc_tab[i];
			break;
		}
	}
	// The minimum battery percentage is set to 95%
	if (percent > 95) {
		percent = 100;
	}

	return percent;  // percentage
}

#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_PACE_MODEL_)		 
void BATT_interrupt_process(uint8_t b_init)
{
	I8U b_pin = nrf_gpio_pin_read(GPIO_CHARGER_INT);	
	I8U int_status = 0;
	/*check if this is a init process or interrrupt process*/
	if (!b_init) {
		// button released
		if (b_pin) {
			return;
		}else{
				BATT_read_reg(0x03, 1, &int_status);
		}
	}else{
			if(load_one_byte_from_rtc_ram(&int_status) != NRF_SUCCESS){
					BATT_read_reg(0x03, 1, &int_status);
			}
	}
	/*only plug and unplug evnts are the stuff we concerned*/
	if((int_status&(0xa0)) != 0 ){
			BATT_SPRINTF("BATT: stored int - 0x%02x", int_status&(0xa0));
			/*key status is unneccesarry to be stored*/
			store_one_byte_to_rtc_ram(int_status&(0xa0));
	}
	//BATT_SPRINTF("BATT: int - 0x%02x", int_status);
	
	if (int_status) {
		// Clear register
		BATT_write_reg(0x03, 0);
	}
	
	if (int_status & 0x80) {
		// Charging cable unpluged
		cling.batt.charging_connected = FALSE;
	}
	
	if (int_status & 0x20) {
		// Charging cable unpluged
		cling.batt.charging_connected = TRUE;
	}
	
	if (int_status & 0x10) {
		// Fully charged
		cling.system.mcu_reg[REGISTER_MCU_BATTERY] = 100;
	}
	
	if (int_status & 0x01) {
		// Button clicked
		if (!b_init) {
#ifdef _CLINGBAND_2_PAY_MODEL_		
      HOMEKEY_button_clicked();
#endif			

#ifdef _CLINGBAND_PACE_MODEL_					
			// Reboot system
			//SYSTEM_restart_from_reset_vector();
#endif			
		}
	}
}
#endif

void BATT_init()
{
#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_PACE_MODEL_)		 	
	/* Enable interrupt on ADC sample ready event*/		
	BATT_interrupt_process(TRUE);
#else 
	// Initialize the battery measuring buffer
	cling.system.mcu_reg[REGISTER_MCU_BATTERY] = 0;
	cling.batt.charging_overall_time = 0;
	cling.batt.shut_down_time = 0;
#endif
	
	if (BATT_is_charging()) {
		cling.batt.charging_state = CHARGER_IN_CHARGING;
		cling.batt.non_charging_accumulated_steps = 0;
		cling.batt.non_charging_accumulated_active_sec = 0;
	} else {
		cling.batt.charging_state = CHARGER_REMOVED;
	}
}

BOOLEAN BATT_device_unauthorized_shut_down()
{
	if (LINK_is_authorized()) {
		return FALSE;
	}
	
	if (LINK_is_authorizing()) {
		return FALSE;
	}
	
#ifndef _CLING_PC_SIMULATION_
	// If device is un-authorized and DC-power is removed
	if (BATT_is_charging()) {
		cling.batt.shut_down_time = 0;
	
		return FALSE;
	} else {
		
		if (cling.batt.shut_down_time < BATTERY_SYSTEM_UNAUTH_POWER_DOWN) {
		
			N_SPRINTF("[BATT] Unauthorized, count down: %d", cling.batt.shut_down_time);

			return FALSE;
		}

		// Disconnect BLE if device is connected.
		if (BTLE_is_connected()) {
			BATT_SPRINTF("[RTC] disconnect, unauthorized device");
			// Disconnect BLE service
			BTLE_disconnect(BTLE_DISCONN_REASON_SYSTEM_SHUTDOWN);
		}

		// No DC-in then, put device into a super-low power state
		//
		BATT_SPRINTF("[BATT] Unauthorized, SD (Level: %d, time: %d)", cling.system.mcu_reg[REGISTER_MCU_BATTERY], cling.batt.shut_down_time);
		
#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_PACE_MODEL_)						
		CLASS(HalExti)*	p_exti_instance = HalExti_get_instance();
		p_exti_instance->set_low_power(p_exti_instance);
#endif
		
		GPIO_system_powerdown();
		
		// No need for a shutdown timer reconfiguration
		//RTC_system_shutdown_timer();
		
		return TRUE;
		
	}
#endif
}

static void _battery_adc_acquired(BATT_CTX *b, I32U t_curr)
{
#ifndef _CLING_PC_SIMULATION_
	// Convert ADC reading to battery percentage
	b->battery_measured_perc = _get_battery_perc();

	if (b->b_initial_measuring) {
		b->b_initial_measuring = FALSE;
		// No need to initialize battery level here as system restore its value from 
		// critical area.
		if ((!LINK_is_authorized()) || cling.batt.b_no_batt_restored) {
			cling.batt.b_no_batt_restored = FALSE;
			cling.system.mcu_reg[REGISTER_MCU_BATTERY] = b->battery_measured_perc;
		}
		
		N_SPRINTF("[BATT] initial battery: %d, %d", b->battery_measured_perc, b->volts_reading);
		
		// If charger is in a charging state, we should initialize "vBAse" and "timebase"
		if (b->charging_state == CHARGER_IN_CHARGING) {
			b->charging_timebase = t_curr;
		}
	} else {

		// Hold current battery percentage if the battery is measured at a high voltage
		if (b->battery_measured_perc < cling.system.mcu_reg[REGISTER_MCU_BATTERY]) {
			cling.system.mcu_reg[REGISTER_MCU_BATTERY] = b->battery_measured_perc;
			
			// if measured voltage is less than calculated charging voltage, re-sync charging state
			if (b->charging_state == CHARGER_IN_CHARGING) {
				b->charging_timebase = t_curr;
			}
		}
						
		N_SPRINTF("[BATT] measured perc: %d, %d", cling.system.mcu_reg[REGISTER_MCU_BATTERY], b->volts_reading);
		
	}

	// Disable VBAT ADC
//	GPIO_vbat_adc_disable();
#endif
}

static void _battery_adc_idle(BATT_CTX *b, I32U t_curr)
{
	I32U t_diff;
#ifndef _CLING_PC_SIMULATION_

	N_SPRINTF("[BATT] --- adc idle---");
	
	// Charging state switching
	if (BATT_is_charging()) {
		b->non_charging_accumulated_steps = 0;
		b->non_charging_accumulated_active_sec = 0;
		if (b->charging_state != CHARGER_IN_CHARGING) {
			b->toggling_number ++;
			N_SPRINTF("[BATT] ---5+ V is supplied (%d)---", b->toggling_number);
			b->charging_state = CHARGER_IN_CHARGING;
			b->charging_timebase = t_curr;
			// if device is in sleep state, wake it up!
			SLEEP_wake_up_by_force(FALSE);
			if (b->state_switching_duration > 10) {
				// Turn on OLED panel
				b->state_switching_duration = 0;
				if (b->toggling_number >= 10) {
						N_SPRINTF("[BATT] --- Charger reset (Supplied: %d, %d)---", b->state_switching_duration, b->toggling_number);
						b->b_toggle_lock = TRUE;
						GPIO_charger_reset();
				}
				b->toggling_number = 0;
			}
#ifdef _ENABLE_TOUCH_
			// Reset skin pads
			cling.touch.b_skin_touch = FALSE;
#endif
			// When something goes very wrong, do not flip display
			if (!b->b_toggle_lock) {
				UI_turn_on_display(UI_STATE_CLOCK_GLANCE, 2000);
			}
		} else {
			
			if (t_curr > (b->charging_timebase + BATTERY_CHARGING_SPEED)) {
				b->charging_timebase = t_curr;
				
				if (cling.system.mcu_reg[REGISTER_MCU_BATTERY] < 100) {
					t_diff = cling.system.mcu_reg[REGISTER_MCU_BATTERY]+1;
					
					// Up limit on battery percentage at 99% (if charging overall time is less than 2 hours)
					if (t_diff > 99) {
						if (b->charging_overall_time < BATTERY_MAXIMUM_CHARGING_TIME) {
							t_diff = 99;
							
							// Time stay-in 99% should be less than 30 minutes
							if (b->charging_overall_time < BATTERY_75_PERC_FULL_CHARGE)
								b->charging_overall_time = BATTERY_75_PERC_FULL_CHARGE;
						} else {
							t_diff = 100;
						}
					}
					
					// Update battery level during charging.
					cling.system.mcu_reg[REGISTER_MCU_BATTERY] = t_diff;
				
					N_SPRINTF("[BATT] battery update: %d", t_diff);
				}
			}
		}
	} else {
		b->charging_overall_time = 0;
		if (b->charging_state != CHARGER_REMOVED) {
			b->toggling_number ++;
			b->charging_state = CHARGER_REMOVED;
			N_SPRINTF("[BATT] ---5+ V is removed (%d, %d)---", b->toggling_number, cling.lps.b_low_power_mode);
			b->charging_timebase = t_curr;
			
			// If charger is automatically removed because of fully charge, then set to a high percentage
			if (cling.lps.b_low_power_mode) {
				if (t_curr > (cling.lps.ts + 5000)) {
					cling.system.mcu_reg[REGISTER_MCU_BATTERY] = b->battery_measured_perc;
					N_SPRINTF("[BATT] --- fully charged: %d ---", b->battery_measured_perc);
				}
			}
			
			// Set to 100% if charger is removed.
			if (cling.system.mcu_reg[REGISTER_MCU_BATTERY] >= 99) {
				cling.system.mcu_reg[REGISTER_MCU_BATTERY] = 100;
			} else if (b->charging_overall_time > BATTERY_MINIMUM_CHARGING_TIME) {
				// if device has been charging over 45 minutes, and battery level is greater than 80%
				// just force it set to 100 %
				if (cling.system.mcu_reg[REGISTER_MCU_BATTERY] > 80) {
					cling.system.mcu_reg[REGISTER_MCU_BATTERY] = 100;
				}
			}

			if (b->state_switching_duration > 10) {
				// Turn on OLED panel
				b->state_switching_duration = 0;
				if (b->toggling_number >= 10) {
						N_SPRINTF("[BATT] --- Charger reset (Removed: %d, %d)---", b->state_switching_duration, b->toggling_number);
						b->b_toggle_lock = TRUE;
						GPIO_charger_reset();
				}

				b->toggling_number = 0;
			}
			// When something goes very wrong, do not flip display
			if (!b->b_toggle_lock) {
				UI_turn_on_display(UI_STATE_CLOCK_GLANCE, 2000);
			}
			// Exit low power mode when power is removed.
			TRACKING_exit_low_power_mode(TRUE);
		}
	}
	
	// Measuring battery percentage
	if (b->level_update_timebase > BATTERY_MEASURE_INTERVAL) {
		b->level_update_timebase = 0;
		
		GPIO_vbat_adc_config();
		cling.batt.volts_reading = nrf_adc_convert_single(BATT_ADC_CHANNEL);
	
		cling.batt.adc_state = CHARGER_ADC_ACQUIRED;

		if (BATT_is_charging()) {
			N_SPRINTF("[BATT] Measureing (charging) - adc: %d", cling.batt.volts_reading); 
		} else {
			N_SPRINTF("[BATT] Measureing (discharging) - adc: %d", cling.batt.volts_reading); 
		}
	}		
#endif
}

BOOLEAN BATT_is_low_battery()
{
	if (BATT_is_charging())
		return FALSE;
	
	if (cling.system.mcu_reg[REGISTER_MCU_BATTERY] > BATTERY_LOW_PERCENTAGE)
		return FALSE;

	return TRUE;
}

void BATT_monitor_state_machine()
{
	BATT_CTX *b = &cling.batt;
	I32U t_curr;
	
	t_curr = CLK_get_system_time();
	switch (b->adc_state) {
		case CHARGER_ADC_IDLE:
			_battery_adc_idle(b, t_curr);
			break;
		case CHARGER_ADC_MEASURING:
			break;
		case CHARGER_ADC_ACQUIRED:
			_battery_adc_acquired(b, t_curr);
			b->adc_state = CHARGER_ADC_IDLE;
			break;
		default:
			break;
	}
}

void BATT_start_first_measure()
{
#ifndef _CLING_PC_SIMULATION_

	GPIO_vbat_adc_config();
	cling.batt.volts_reading = nrf_adc_convert_single(BATT_ADC_CHANNEL);
	
	cling.batt.b_initial_measuring = TRUE;
	cling.batt.adc_state = CHARGER_ADC_ACQUIRED;
	
	BATT_SPRINTF("[BATT] start first measure - adc: %d", cling.batt.volts_reading);
#endif
}
