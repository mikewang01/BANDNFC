/***************************************************************************//**
 * File batt.h
 * 
 * Description: Power measurement and management header
 *
 *
 ******************************************************************************/

#ifndef __BATT_HEADER__
#define __BATT_HEADER__

#define BATTERY_MEASURE_INTERVAL 600   // 600 seconds (10 MINUTES)

#define BATTERY_LOW_PERCENTAGE 19

#define SLEEP_ACTIVE_SECONDS_THRESHOLD 120 // 120 seconds
#define SLEEP_STEP_ACCUMULATED_THRESHOLD 20 // 20 STEPS
enum {
	CHARGER_REMOVED,
	CHARGER_FULL,
	CHARGER_IN_CHARGING,
};

enum {
	CHARGER_ADC_IDLE,
	CHARGER_ADC_MEASURING,
	CHARGER_ADC_ACQUIRED,
};

typedef struct tagBATT_CTX {
	I32U charging_timebase;			// Charging percentage update -- 1 percent every 100 seconds, complete in 7200 seconds
	I16U charging_overall_time;
	I16U volts_reading;
	I16U level_update_timebase;		// Battery percentage update timer -- 15 seconds
	I8U charging_state;
	I8U adc_state;
	I8U state_switching_duration;
	I8U toggling_number;
	I8U shut_down_time;
	I8U battery_measured_perc;
	BOOLEAN b_initial_measuring;
	
	// charging detection
	I8U non_charging_accumulated_active_sec;
	I8U non_charging_accumulated_steps;
	
} BATT_CTX;

I8U BATT_get_level(void);
BOOLEAN BATT_is_charging(void);
void BATT_init(void);
void BATT_monitor_state_machine(void);
void BATT_start_first_measure(void);
BOOLEAN BATT_is_low_battery(void);
void BATT_charging_update_steps(I8U steps);
void BATT_charging_update_sec(I8U sec);
void BATT_update_charging_sec(I8U tick_in_s);
BOOLEAN BATT_charging_det_for_sleep(void);

#endif // __BATT_HEADER__
