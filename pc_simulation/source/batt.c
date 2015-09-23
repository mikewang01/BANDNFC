#include "main.h"

#define ADC_4P2_VOLTS 646
#define ADC_4P0_VOLTS 616
#define ADC_3P8_VOLTS 585
#define ADC_3P4_VOLTS 523
#define ADC_3P2_VOLTS 492
#define ADC_3P0_VOLTS 462

#define BATTERY_PERCENTAGE_LEVEL 72

static const I16U battery_vol_tab[BATTERY_PERCENTAGE_LEVEL] = {
645,641,639,637,636,634,632,630,628,627,625,624,622,620,618,616,
614,613,612,610,609,608,606,605,603,602,601,599,598,596,595,594,
593,592,591,590,589,588,587,586,586,585,584,584,583,583,582,581,
581,580,580,579,579,578,578,578,577,576,576,575,574,573,572,570,
569,567,567,566,566,566,565,564};

static const I16U battery_perc_tab[BATTERY_PERCENTAGE_LEVEL] = {
100,99,98,96,95,94,92,91,89,88,87,85,84,82,81,80,
78,77,75,74,73,71,70,69,67,66,64,63,62,60,59,57,
56,55,53,52,50,49,48,46,45,44,42,41,39,38,37,35,
34,32,31,30,28,27,25,24,23,21,20,19,17,16,14,13,
12,10,9,7,6,5,3,2};

I8U BATT_get_level()
{
	return cling.system.mcu_reg[REGISTER_MCU_BATTERY];
}

BOOLEAN BATT_is_charging()
{
#ifndef _CLING_PC_SIMULATION_
	if (nrf_gpio_pin_read(GPIO_CHARGER_CHGFLAG)) {
		return FALSE;
	} else {
		return TRUE;
	}
#else
	return FALSE;
#endif
}

BOOLEAN BATT_charging_det_for_sleep()
{
	BATT_CTX *b = &cling.batt;
	
	if (b->charging_state == CHARGER_IN_CHARGING) {
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

void BATT_update_charging_sec(I8U tick_in_s)
{
	if (BATT_is_charging()) {
		cling.batt.charging_overall_time += tick_in_s;
		
		if (cling.batt.charging_overall_time > 10800) {
			cling.batt.charging_overall_time = 10800;
		}
	}
}

void BATT_charging_update_sec(I8U sec)
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
			Y_SPRINTF("[BATT] low power shut-down (reading: %d, )", b->volts_reading);
		
			GPIO_system_powerdown();
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

void BATT_init()
{
#ifndef _CLING_PC_SIMULATION_
	/* Enable interrupt on ADC sample ready event*/		
	NRF_ADC->INTENSET = ADC_INTENSET_END_Disabled;
	sd_nvic_EnableIRQ(ADC_IRQn);	
	
	GPIO_vbat_adc_config();
	
	/* Enable ADC*/
	NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled;
	
	// Initialize the battery measuring buffer
	cling.system.mcu_reg[REGISTER_MCU_BATTERY] = 0;
	cling.batt.charging_overall_time = 0;
	
	if (BATT_is_charging()) {
		cling.batt.charging_state = CHARGER_IN_CHARGING;
		cling.batt.non_charging_accumulated_steps = 0;
		cling.batt.non_charging_accumulated_active_sec = 0;
	} else {
		cling.batt.charging_state = CHARGER_REMOVED;
		cling.batt.non_charging_accumulated_steps = 0xff;
		cling.batt.non_charging_accumulated_active_sec = 0xff;
	}
#endif
}

static void _start_adc_measure()
{
#ifndef _CLING_PC_SIMULATION_
	/* Enable interrupt on ADC sample ready event*/		
	NRF_ADC->INTENSET = ADC_INTENSET_END_Disabled;
	
	// Clear all pending IRQ
	sd_nvic_ClearPendingIRQ(ADC_IRQn);
	sd_nvic_SetPriority(ADC_IRQn, APP_IRQ_PRIORITY_LOW);
	sd_nvic_EnableIRQ(ADC_IRQn);

	// Configure ADC pin and add 2 ms delay before starting ADC conversion.
	GPIO_vbat_adc_config();
	
	/* Enable ADC*/
	NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled;

	BASE_delay_msec(5);
	
	// Start ADC interrupt
	NRF_ADC->INTENSET = ADC_INTENSET_END_Enabled;

	NRF_ADC->EVENTS_END  = 0;    // Stop any running conversions.
	NRF_ADC->TASKS_START = 1;          // Sampling and conversion start
#endif
}

static void _device_unauthorized_shut_down()
{
	if (LINK_is_authorized()) {
		return;
	}
	
	if (LINK_is_authorizing()) {
		return;
	}
	
#ifndef _CLING_PC_SIMULATION_
	// If device is un-authorized and DC-power is removed
	if (nrf_gpio_pin_read(GPIO_CHARGER_CHGFLAG)) {
		
		if (cling.batt.shut_down_time < 180)
			return;
		
		// Delete bond info
		BTLE_delete_bond();

		// No DC-in then, put device into a super-low power state
		//
		Y_SPRINTF("[BATT] unau SD (Level: %d, state: %d, time: %d)", cling.system.mcu_reg[REGISTER_MCU_BATTERY], cling.batt.charging_state, cling.batt.level_update_timebase);
		
		GPIO_system_powerdown();
		
	} else {
		cling.batt.shut_down_time = 0;
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
		if (!LINK_is_authorized()) {
			cling.system.mcu_reg[REGISTER_MCU_BATTERY] = b->battery_measured_perc;
		}
		
		Y_SPRINTF("[BATT] initial battery: %d, %d", b->battery_measured_perc, b->volts_reading);
		
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
						
		Y_SPRINTF("[BATT] measured perc: %d, %d", cling.system.mcu_reg[REGISTER_MCU_BATTERY], b->volts_reading);
		
	}

	// Disable VBAT ADC
	GPIO_vbat_adc_disable();
#endif
}

static void _battery_adc_idle(BATT_CTX *b, I32U t_curr)
{
	I32U t_diff;

	// For unauthorized device, we should shut it down if battery is lower than a threshold
	_device_unauthorized_shut_down();
				
	N_SPRINTF("[BATT] --- adc idle---");
	
	// Charging state switching
	if (BATT_is_charging()) {
		if (b->charging_state != CHARGER_IN_CHARGING) {
			b->toggling_number ++;
			Y_SPRINTF("[BATT] ---5+ V is supplied (%d)---", b->toggling_number);
			b->charging_state = CHARGER_IN_CHARGING;
			b->non_charging_accumulated_steps = 0;
			b->non_charging_accumulated_active_sec = 0;
			b->charging_timebase = t_curr;
			if (b->state_switching_duration > 5) {
				// Turn on OLED panel
				b->state_switching_duration = 0;
				if (b->toggling_number >= 10) {
						Y_SPRINTF("[BATT] --- Charger reset (Supplied: %d, %d)---", b->state_switching_duration, b->toggling_number);
						GPIO_charger_reset();
				}
				b->toggling_number = 0;
			}
			
			// Reset skin pads
			cling.touch.skin_touch_type = TOUCH_SKIN_PAD_0;
			
			// Reset PPG wearing detection
			cling.hr.wearing_state = PPG_BODY_NOT_WEAR;
				
			UI_turn_on_display(UI_STATE_CLOCK_GLANCE, 2000);
		} else {
			
			if (t_curr > (b->charging_timebase + 72000)) {
				b->charging_timebase = t_curr;
				
				if (cling.system.mcu_reg[REGISTER_MCU_BATTERY] < 100) {
					t_diff = cling.system.mcu_reg[REGISTER_MCU_BATTERY]+1;
					
					// Up limit on battery percentage at 99% (if charging overall time is less than 3 hours)
					if (t_diff > 99) {
						if (b->charging_overall_time < 10000) {
							t_diff = 99;
							
							// Time stay-in 99% should be less than 30 hour
							if (b->charging_overall_time < 8200) {
								b->charging_overall_time = 8200;
							}
						} else {
							t_diff = 100;
						}
					}
					
					// Update battery level during charging.
					cling.system.mcu_reg[REGISTER_MCU_BATTERY] = t_diff;
				
					Y_SPRINTF("[BATT] battery update: %d", t_diff);
				}
			}
		}
	} else {
		b->charging_overall_time = 0;
		if (b->charging_state != CHARGER_REMOVED) {
			b->toggling_number ++;
			b->charging_state = CHARGER_REMOVED;
			Y_SPRINTF("[BATT] ---5+ V is removed (%d, %d)---", b->toggling_number, cling.lps.b_low_power_mode);
			b->charging_timebase = t_curr;
			
			// If charger is automatically removed because of fully charge, then set to a high percentage
			if (cling.lps.b_low_power_mode) {
				if (t_curr > (cling.lps.ts + 5000)) {
					cling.system.mcu_reg[REGISTER_MCU_BATTERY] = b->battery_measured_perc;
					Y_SPRINTF("[BATT] --- fully charged: %d ---", b->battery_measured_perc);
				}
			}
			
			// Set to 100% if charger is removed.
			if (cling.system.mcu_reg[REGISTER_MCU_BATTERY] >= 99) {
				cling.system.mcu_reg[REGISTER_MCU_BATTERY] = 100;
			}

			if (b->state_switching_duration > 5) {
				// Turn on OLED panel
				b->state_switching_duration = 0;
				if (b->toggling_number >= 10) {
						Y_SPRINTF("[BATT] --- Charger reset (Removed: %d, %d)---", b->state_switching_duration, b->toggling_number);

						GPIO_charger_reset();
				}

				b->toggling_number = 0;
			}
			UI_turn_on_display(UI_STATE_CLOCK_GLANCE, 2000);

			// Exit low power mode when power is removed.
			TRACKING_exit_low_power_mode(TRUE);
		}
	}
	
	// Measuring battery percentage
	if (b->level_update_timebase > BATTERY_MEASURE_INTERVAL) {
		// Measure battery level every 15 seconds
		_start_adc_measure();

		b->level_update_timebase = 0;
		
		b->adc_state = CHARGER_ADC_MEASURING;
		
		N_SPRINTF("[BATT] discharging start measureing ..."); 
	}		
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
	I32U t_curr = CLK_get_system_time();
	
	switch (b->adc_state) {
		case CHARGER_ADC_IDLE:
			_battery_adc_idle(b, t_curr);
			break;
		case CHARGER_ADC_MEASURING:
			if (b->level_update_timebase > 10) {
				// if something goes wrong, reset state machine to IDLE state.
				b->adc_state = CHARGER_ADC_IDLE;
			}
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
	_start_adc_measure();
	
	cling.batt.b_initial_measuring = TRUE;
	cling.batt.adc_state = CHARGER_ADC_MEASURING;
	
	Y_SPRINTF("[BATT] start first measure");
}
