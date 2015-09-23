
#include "main.h"

#ifndef _CLING_PC_SIMULATION_
static void _config_therm_volt_meas()
{
	/* Enable interrupt on ADC sample ready event*/		
	NRF_ADC->INTENSET = ADC_INTENSET_END_Disabled;
	sd_nvic_EnableIRQ(ADC_IRQn);
	
	GPIO_therm_adc_enable();
	
	/* Enable ADC*/
	NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled;

	sd_nvic_ClearPendingIRQ(ADC_IRQn);
	sd_nvic_SetPriority(ADC_IRQn, APP_IRQ_PRIORITY_LOW);
	sd_nvic_EnableIRQ(ADC_IRQn);

	NRF_ADC->INTENSET = ADC_INTENSET_END_Enabled;

	NRF_ADC->EVENTS_END  = 0;
	NRF_ADC->TASKS_START = 1;
}

static void _config_power_volt_meas()
{
	/* Enable interrupt on ADC sample ready event*/		
	NRF_ADC->INTENSET = ADC_INTENSET_END_Disabled;
	sd_nvic_EnableIRQ(ADC_IRQn);	
	
	//GPIO_power_pin_adc_enable();
	
	/* Enable ADC*/
	NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled;

	sd_nvic_ClearPendingIRQ(ADC_IRQn);
	sd_nvic_SetPriority(ADC_IRQn, APP_IRQ_PRIORITY_LOW);
	sd_nvic_EnableIRQ(ADC_IRQn);

	NRF_ADC->INTENSET = ADC_INTENSET_END_Enabled;

	NRF_ADC->EVENTS_END  = 0;
	NRF_ADC->TASKS_START = 1;
}
#endif
static void _calc_temperature()
{
	THERMISTOR_CTX *t = &cling.therm;
	I32S volt_diff;
	I32S resistance;
	I16S i, temperature;

	// resistance-temperature talbe of FTN18WF104
	// temperature from 20.0 to 47.95, step 0.05
	const I32S FTN18WF104_tab[560] = {
    127080, 126771, 126463, 126156, 125850, 125544, 125239, 124935, 124632, 124330, 124028, 123727, 123427, 123128, 122830, 122532, 
    122235, 121939, 121644, 121350, 121056, 120763, 120471, 120180, 119890, 119600, 119311, 119023, 118736, 118450, 118164, 117879, 
    117595, 117312, 117029, 116747, 116466, 116186, 115906, 115627, 115368, 115091, 114814, 114538, 114263, 113989, 113715, 113442, 
    113170, 112899, 112628, 112358, 112089, 111820, 111552, 111285, 111019, 110753, 110488, 110224, 109960, 109697, 109435, 109174, 
    108913, 108653, 108394, 108135, 107877, 107620, 107363, 107107, 106852, 106597, 106343, 106090, 105837, 105585, 105334, 105083, 
    104852, 104603, 104354, 104106, 103859, 103612, 103366, 103121, 102876, 102632, 102389, 102146, 101904, 101663, 101422, 101182, 
    100942, 100703, 100465, 100227,  99990,  99754,  99518,  99283,  99048,  98814,  98581,  98348,  98116,  97885,  97654,  97424, 
     97194,  96965,  96737,  96509,  96282,  96055,  95829,  95604,  95398,  95174,  94950,  94727,  94504,  94282,  94061,  93840, 
     93620,  93400,  93181,  92963,  92745,  92528,  92311,  92095,  91879,  91664,  91450,  91236,  91023,  90810,  90598,  90386, 
     90175,  89964,  89754,  89545,  89336,  89128,  88920,  88713,  88506,  88300,  88094,  87889,  87684,  87480,  87276,  87073, 
     86889,  86687,  86485,  86284,  86084,  85884,  85685,  85486,  85288,  85090,  84893,  84696,  84500,  84304,  84109,  83914, 
     83720,  83526,  83333,  83140,  82948,  82756,  82565,  82374,  82184,  81994,  81805,  81616,  81428,  81240,  81053,  80866, 
     80680,  80494,  80309,  80124,  79939,  79755,  79571,  79388,  79222,  79040,  78858,  78677,  78496,  78316,  78136,  77957, 
     77778,  77599,  77421,  77243,  77066,  76889,  76713,  76537,  76362,  76187,  76013,  75839,  75665,  75492,  75319,  75147, 
     74975,  74804,  74633,  74463,  74293,  74123,  73954,  73785,  73617,  73449,  73282,  73115,  72948,  72782,  72616,  72451, 
     72306,  72141,  71977,  71813,  71650,  71487,  71325,  71163,  71001,  70840,  70679,  70519,  70359,  70199,  70040,  69881, 
     69723,  69565,  69407,  69250,  69093,  68937,  68781,  68625,  68470,  68315,  68161,  68007,  67853,  67700,  67547,  67395, 
     67243,  67091,  66940,  66789,  66639,  66489,  66339,  66190,  66061,  65912,  65764,  65616,  65468,  65321,  65174,  65028, 
     64882,  64736,  64591,  64446,  64301,  64157,  64013,  63869,  63726,  63583,  63441,  63299,  63157,  63016,  62875,  62734, 
     62594,  62454,  62314,  62175,  62036,  61897,  61759,  61621,  61483,  61346,  61209,  61073,  60937,  60801,  60666,  60531, 
     60415,  60280,  60146,  60012,  59878,  59745,  59612,  59479,  59347,  59215,  59083,  58952,  58821,  58690,  58560,  58430, 
     58300,  58171,  58042,  57913,  57785,  57657,  57529,  57402,  57275,  57148,  57022,  56896,  56770,  56645,  56520,  56395, 
     56270,  56146,  56022,  55898,  55775,  55652,  55529,  55407,  55306,  55184,  55062,  54941,  54820,  54699,  54579,  54459, 
     54339,  54219,  54100,  53981,  53862,  53744,  53626,  53508,  53391,  53274,  53157,  53040,  52924,  52808,  52692,  52577, 
     52462,  52347,  52232,  52118,  52004,  51890,  51777,  51664,  51551,  51438,  51326,  51214,  51102,  50991,  50880,  50769, 
     50677,  50567,  50457,  50347,  50237,  50128,  50019,  49910,  49801,  49693,  49585,  49477,  49369,  49262,  49155,  49048, 
     48942,  48836,  48730,  48624,  48519,  48414,  48309,  48204,  48100,  47996,  47892,  47788,  47685,  47582,  47479,  47376, 
     47274,  47172,  47070,  46968,  46867,  46766,  46665,  46564,  46482,  46382,  46282,  46182,  46083,  45984,  45885,  45786, 
     45687,  45589,  45491,  45393,  45295,  45198,  45101,  45004,  44907,  44811,  44715,  44619,  44523,  44428,  44333,  44238, 
     44143,  44048,  43954,  43860,  43766,  43672,  43579,  43486,  43393,  43300,  43207,  43115,  43023,  42931,  42839,  42748, 
     42675,  42584,  42493,  42402,  42312,  42222,  42132,  42042,  41952,  41863,  41774,  41685,  41596,  41507,  41419,  41331, 
     41243,  41155,  41068,  40981,  40894,  40807,  40720,  40634,  40548,  40462,  40376,  40290,  40204,  40119,  40034,  39949, 
     39864,  39779,  39695,  39611,  39527,  39443,  39359,  39276,  39213,  39130,  39047,  38964,  38882,  38800,  38718,  38636, 
     38554,  38473,  38392,  38311,  38230,  38149,  38069,  37989,  37909,  37829,  37749,  37670,  37591,  37512,  37433,  37354, 
     37275,  37197,  37119,  37041,  36963,  36885,  36808,  36731,  36654,  36577,  36500,  36423,  36347,  36271,  36195,  36119};

		// calculate resistance of ntc.
	volt_diff  = ( t->power_volts_reading - t->therm_volts_reading) * 100000;
		 
#if 1
  for (resistance=0; (volt_diff>0) ; resistance++) {
		volt_diff -= t->therm_volts_reading;
	}
#endif

#if 1
	// calculate temperature.
	i = 0;
	for (i=0; ; i++) {
		if (resistance>=FTN18WF104_tab[i]) { 
			break;
		}
	}

	temperature = 2000 + i * 5;
  Y_SPRINTF( "therm: %d  power: %d  resis: %d  temp: %d", t->therm_volts_reading, t->power_volts_reading, resistance, temperature );
#endif
}

I32S therm_timer = 0;
void THERMISTOR_init(void)
{
	THERMISTOR_CTX *t = &cling.therm;
	I8U i;
	
	for (i=0; i<8; i++) 
	  t->m_temp_sample[i] = 0;
	
	t->m_sample_cnt = 0;
	
	// init thermistor measue time base
	t->measure_timebase = CLK_get_system_time();
	
	t->state = THERMISTOR_STAT_IDLE;
	t->therm_volts_reading = 0;
	t->power_volts_reading = 0;
	therm_timer = CLK_get_system_time();
}

void THERMISTOR_state_machine()
{
#ifndef _CLING_PC_SIMULATION_
	THERMISTOR_CTX *t = &cling.therm;
	I32S t_curr, t_diff;
	
	t_curr = CLK_get_system_time();
	t_diff = t_curr - therm_timer;
	
	if ( t_diff > 100 ) {
		
		therm_timer = t_curr;
	
		N_SPRINTF("therm state: %d  %d", cling.therm.state, ((NRF_ADC->CONFIG & 0x0000FF00)>>8) );
		N_SPRINTF( "therm: %d    power: %d", t->therm_volts_reading, t->power_volts_reading );
	
		switch (t->state) {

			case THERMISTOR_STAT_IDLE:
  			t->state = THERMISTOR_STAT_THERM_PIN_TURN_ON_ADC;
				break;
		
			case THERMISTOR_STAT_THERM_PIN_TURN_ON_ADC:
				_config_therm_volt_meas();
			  t->state = THERMISTOR_STAT_START_THERM_PIN_MEASURING;
				break;
		
			case THERMISTOR_STAT_START_THERM_PIN_MEASURING:
				break;
		
 			case THERMISTOR_STAT_COLLECT_THERM_PIN_SAMPLES:
			  t->state = THERMISTOR_STAT_POWER_PIN_TURN_ON_ADC;
	//	  GPIO_therm_adc_disable();
				break;

 			case THERMISTOR_STAT_POWER_PIN_TURN_ON_ADC:
				_config_power_volt_meas();
			  t->state = THERMISTOR_STAT_START_POWER_PIN_MEASURING;
				break;

 			case THERMISTOR_STAT_START_POWER_PIN_MEASURING:
				break;
		
 			case THERMISTOR_STAT_COLLECT_POWER_PIN_SAMPLES:
				_calc_temperature();
			  t->state = THERMISTOR_STAT_IDLE;
	//	  GPIO_power_pin_adc_disable();
				break;
		
		  case THERMISTOR_STAT_DUTY_ON:
		  case THERMISTOR_STAT_TURN_ON_ADC:
		  case THERMISTOR_STAT_START_MEASURING:
		  case THERMISTOR_STAT_COLLECT_SAMPLES:
		  case THERMISTOR_STAT_ADC_SAMPLE_READY:
		  case THERMISTOR_STAT_DUTY_OFF:
				break;		

			default:
				break;
		}
	}
#endif
}

BOOLEAN THERMISTOR_is_the_state(THERMISTOR_STATES state_to_check)
{
	THERMISTOR_CTX *t = &cling.therm;
	
	if (t->state == state_to_check)
		return TRUE;
	else
		return FALSE;
}

void THERMISTOR_set_state(THERMISTOR_STATES new_state)
{
	cling.therm.state = new_state;
}


BOOLEAN THERMISTOR_switch_to_duty_on_state()
{
	return TRUE;
}
