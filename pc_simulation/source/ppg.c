/***************************************************************************/
/**
* @file ppg.c
* @brief Driver implementation for TI AFE4400 Analog Front-End for
* @Heart Rate Monitors and Low-Cost Pulse Oximeters.
*
******************************************************************************/

#include "main.h"

extern I8U g_spi_tx_buf[];
extern I8U g_spi_rx_buf[];

// MOVE TO PPG.h START
#define CONTROL0_VAL                       0x000000            // 0x000000            // 0x000000
#define LED2STC_VAL                           45120            //    45015            //    45015
#define LED2ENDC_VAL                          48758            //    45015            //    59985
#define LED2LEDSTC_VAL                        45000            //    45000            //    45000
#define LED2LEDENDC_VAL                       48760            //    45000            //    59992
#define ALED2STC_VAL                            120            //       15            //       15
#define ALED2ENDC_VAL                          3758            //       15            //    14985
#define LED1STC_VAL                           15015            //    15120            //    15015
#define LED1ENDC_VAL                          15015            //    18758            //    29985
#define LED1LEDSTC_VAL                        15000            //    15000            //    15000
#define LED1LEDENDC_VAL                       15000            //    18760            //    29992
#define ALED1STC_VAL                          30000            //    30120            //    30015
#define ALED1ENDC_VAL                         30000            //    33758            //    44985
#define LED2CONVST_VAL                          100            //       15            //       15
#define LED2CONVEND_VAL                        3758            //       15            //    14992
#define ALED2CONVST_VAL                       15100            //    15015            //    15015
#define ALED2CONVEND_VAL                      18758            //    15015            //    29992
#define LED1CONVST_VAL                        30015            //    30100            //    30015
#define LED1CONVEND_VAL                       30015            //    33758            //    44992
#define ALED1CONVST_VAL                       45015            //    45100            //    45015
#define ALED1CONVEND_VAL                      45015            //    48758            //    59992
#define ADCRSTSTCT0_VAL                           0            //        0            //        0
#define ADCRSTENDCT0_VAL                         10            //       10            //       10
#define ADCRSTSTCT1_VAL                       15000            //    15000            //    15000
#define ADCRSTENDCT1_VAL                      15010            //    15010            //    15010
#define ADCRSTSTCT2_VAL                       30000            //    30000            //    30000
#define ADCRSTENDCT2_VAL                      30010            //    30010            //    30010
#define ADCRSTSTCT3_VAL                       45000            //    45000            //    45000
#define ADCRSTENDCT3_VAL                      45010            //    45010            //    45010
#define PRPCOUNT_VAL                          59999            //    59999            //    59999
#define CONTROL1_VAL                       0x000103            // 0x000103            // 0x000103
#define SPARE1_VAL                         0x000000            // 0x000000            // 0x000000
#define TIAGAIN_VAL                        0x000000            // 0x000000            // 0x000000
#define TIA_AMB_GAIN_VAL                   0x000005            // 0x000005            // 0x000005
#define LEDCNTRL_VAL                       0x010033            // 0x010022            // 0x010022
#define CONTROL2_VAL                       0x040000            // 0x040000            // 0x040000
// MOVE TO PPG.h END
#if 0
static void _set_reg(I8U reg_idx, I32U config)
{
#ifndef _CLING_PC_SIMULATION_	
  GPIO_twi_disabled();

//if (!cling.system.b_spi_1_ON) {
	if (TRUE) {
		spi_master_init(SPI_MASTER_1, spi_master_1_event_handler, FALSE);
		cling.system.b_spi_1_ON = TRUE;
	}

	g_spi_tx_buf[0] = reg_idx;
	g_spi_tx_buf[1] = (I8U)((config & 0x00ff0000) >> 16);
	g_spi_tx_buf[2] = (I8U)((config & 0x0000ff00) >>  8);
	g_spi_tx_buf[3] = (I8U)((config & 0x000000ff) >>  0);
	
		N_SPRINTF("[spi] 164");
	spi_master_tx_rx(SPI_MASTER_1, g_spi_tx_buf, 4, 0, 0, g_spi_rx_buf, 0, 0, GPIO_SPI_1_CS_PPG);
#endif
}

static void _get_reg(I8U reg_idx)
{                                                                                                                      
#ifndef _CLING_PC_SIMULATION_
  GPIO_twi_disabled();	

//if (!cling.system.b_spi_1_ON) {
	if (TRUE) {
		spi_master_init(SPI_MASTER_1, spi_master_1_event_handler, FALSE);
		cling.system.b_spi_1_ON = TRUE;
	}
	
	g_spi_tx_buf[0] = reg_idx;
	g_spi_tx_buf[1] = 0;
	
		N_SPRINTF("[spi] 146");
	spi_master_tx_rx(SPI_MASTER_1, g_spi_tx_buf, 2, 0, 0, g_spi_rx_buf, 0, 4, GPIO_SPI_1_CS_PPG);
	spi_master_op_wait_done();
#endif
}

static void _module_turn_off()
{
#ifndef _CLING_PC_SIMULATION_
	#if 0
	nrf_gpio_pin_clear(GPIO_PPG_AFE_PDN);
	GPIO_spi1_disconnect();
	#endif
#endif
}

static void _module_turn_on()
{
#ifndef _CLING_PC_SIMULATION_
	nrf_gpio_pin_set(GPIO_PPG_AFE_PDN);
#endif
}

static BOOLEAN _is_measure_allowed()
{
  return TRUE;      // TBD: for debugging...

	if (OTA_if_enabled()) {
		return FALSE;
	}
	
	// 
	// Two conditions to jump to "duty on"
	// 
	// 1) device authorized, 2) skin touch detection is positive		
	if (PPG_is_skin_touched())
		return TRUE;

	return FALSE;
}

static void _update_heart_rate_statistics()
{
	HEARTRATE_CTX *h = &cling.hr;
	
	if (!h->high_rate || !h->low_rate) {
		h->high_rate = h->current_rate;
		h->low_rate = h->current_rate;
		return;
	}
	
	if (h->current_rate > h->high_rate)
		h->high_rate = h->current_rate;

	if (h->current_rate < h->low_rate)
		h->low_rate = h->current_rate;
}

static void _ppg_regs_init(void)
{
	_set_reg(CONTROL0,     0x000008);
	_set_reg(LED2STC,      LED2STC_VAL);
  _set_reg(LED2ENDC,     LED2ENDC_VAL);
  _set_reg(LED2LEDSTC,   LED2LEDSTC_VAL);
  _set_reg(LED2LEDENDC,  LED2LEDENDC_VAL);
  _set_reg(ALED2STC,     ALED2STC_VAL);
  _set_reg(ALED2ENDC,    ALED2ENDC_VAL);
  _set_reg(LED1STC,      LED1STC_VAL);
  _set_reg(LED1ENDC,     LED1ENDC_VAL);
  _set_reg(LED1LEDSTC,   LED1LEDSTC_VAL);
  _set_reg(LED1LEDENDC,  LED1LEDENDC_VAL);
  _set_reg(ALED1STC,     ALED1STC_VAL);
  _set_reg(ALED1ENDC,    ALED1ENDC_VAL);
  _set_reg(LED2CONVST,   LED2CONVST_VAL);
  _set_reg(LED2CONVEND,  LED2CONVEND_VAL);
  _set_reg(ALED2CONVST,  ALED2CONVST_VAL);
  _set_reg(ALED2CONVEND, ALED2CONVEND_VAL);
  _set_reg(LED1CONVST,   LED1CONVST_VAL);
  _set_reg(LED1CONVEND,  LED1CONVEND_VAL);
  _set_reg(ALED1CONVST,  ALED1CONVST_VAL);
  _set_reg(ALED1CONVEND, ALED1CONVEND_VAL);
  _set_reg(ADCRSTSTCT0,  ADCRSTSTCT0_VAL);
  _set_reg(ADCRSTENDCT0, ADCRSTENDCT0_VAL);
  _set_reg(ADCRSTSTCT1,  ADCRSTSTCT1_VAL);
  _set_reg(ADCRSTENDCT1, ADCRSTENDCT1_VAL);
  _set_reg(ADCRSTSTCT2,  ADCRSTSTCT2_VAL);
  _set_reg(ADCRSTENDCT2, ADCRSTENDCT2_VAL);
  _set_reg(ADCRSTSTCT3,  ADCRSTSTCT3_VAL);
  _set_reg(ADCRSTENDCT3, ADCRSTENDCT3_VAL);
  _set_reg(PRPCOUNT,     PRPCOUNT_VAL);
	_set_reg(CONTROL1,     CONTROL1_VAL);
	_set_reg(TIA_AMB_GAIN, TIA_AMB_GAIN_VAL);
	_set_reg(LEDCNTRL,     LEDCNTRL_VAL);
	_set_reg(CONTROL2,     CONTROL2_VAL);
	_set_reg(ALARM,        0x000000);
	_set_reg(CONTROL0,     0x000001);
}

static void _module_configured_for_measuring()
{
	HEARTRATE_CTX *h = &cling.hr;
	
	// 1. turn on PPG and configure GPIO related.
	_module_turn_on();
  
	// 2. Start 20 ms timer for screen rendering
	SYSCLK_timer_start();
		
  // 3. turn on LED
  _ppg_regs_init();
	
	// 4. init variables
	h->m_pre_zero_point  = 0;
	h->m_pre_sample      = 0;
	h->m_pre_pulse_width = 0;
	h->update_cnt        = 0;
}

static I32S _get_light_strength_register(I8U regIdx)
{
	I32S regVal;	
	_get_reg(regIdx);
	if ( g_spi_rx_buf[1] & 0x20 ) {  // negative value read, 22-bit adc, the two MSB bits can ignored.
		regVal = (I32S)(
                     ( 0xffc00000                      ) | 
			               ( ( (I32U) g_spi_rx_buf[1]) << 16 ) | 
			               ( ( (I32U) g_spi_rx_buf[2]) <<  8 ) | 
			               ( ( (I32U) g_spi_rx_buf[3]) <<  0 )
		               );
	} else {          
		regVal = (I32S)(
                     ( ( (I32U) g_spi_rx_buf[1]) << 16 ) | 
                     ( ( (I32U) g_spi_rx_buf[2]) <<  8 ) | 
                     ( ( (I32U) g_spi_rx_buf[3]) <<  0 )
                   );
	}
	return regVal;
}

static void _dynamic_power_adjust(I32S sampleAvg)
{
	I32U regVal;
	I32S currentVal;
	
	_set_reg(CONTROL0, 0x000001);
	_get_reg(LEDCNTRL);
	
	regVal =  (uint32_t) (
			(((uint32_t)g_spi_rx_buf[1]) << 16) | 
			(((uint32_t)g_spi_rx_buf[2]) <<  8) | 
			(((uint32_t)g_spi_rx_buf[3]) <<  0)
			);
			
  currentVal = (regVal & 0x00ff00) >> 8;      // read LED1 current setting			
			
  N_SPRINTF("[PPG] LEDCNTRL setting: 0x%06x    %d", regVal, sampleAvg);

	// sample value are nearby -1550000 or +1550000
	if ((sampleAvg>0 && sampleAvg<1850000) || (sampleAvg<0 && sampleAvg>-1850000)) {
		if (currentVal<0xe0)
			currentVal = currentVal + 0x2;            // turn current setting up
	} else	if (sampleAvg>1950000 || sampleAvg<-1950000) {
		if (currentVal>0x20)
			currentVal = currentVal - 0x2;            // turn current setting down
	}

  regVal = regVal & 0xffff00ff;
  regVal = regVal | (currentVal << 8);

	_set_reg(CONTROL0, 0x000000);      // set ppg register set in WRITE mode
	_set_reg(LEDCNTRL, regVal);
	_set_reg(CONTROL0, 0x000001);      // set ppg register set back to READ mode
}

static BOOLEAN _heart_rate_overflow(I16S sample_num)
{
	HEARTRATE_CTX *h = &cling.hr;
	I32S t_curr;

	t_curr = CLK_get_system_time();
	
	if ( sample_num<(15*20) || sample_num>(67*20) ) {
		h->m_pre_zero_point = t_curr;
//	h->m_pre_pulse_width = 0;
		return TRUE;
	}
	return FALSE;
}

static void _reset_heart_rate_calculator()
{
	HEARTRATE_CTX *h = &cling.hr;
	I8U  i;
	I8U  sample_num;
	I16S pulse_width;
	
	sample_num = 0;
	pulse_width = h->m_pre_pulse_width;
	while (pulse_width>=0) {
		pulse_width -= 20;
		sample_num++;
	}
	for (i=0; i<8; i++) h->m_sample_num[i] = sample_num;
}

static void _calc_heart_rate()
{
	HEARTRATE_CTX *h = &cling.hr;
	
	I8U  sample_num;
	I16S pulse_width;	
	I8S  heart_rate;	
	I32S one_minute;
	I32S sum;
	I8S  non_num;
	I8S  avg_sample_num;
	I8U  i;	
	I32S t_diff;
	
	// current pulse width
	sample_num = 0;
	pulse_width = h->m_pre_pulse_width;
	while (pulse_width>=0) {
		pulse_width -= 20;        // 20 milliseconds per sample
		sample_num++;             // samples number in one heart rate pulse
	}	
	
	// 50bpm: 60 samples
	// 60bpm: 50 samples
	// 70bpm: 43 samples
	// 80bpm: 38 samples
	// 90bpm: 33 samples
	// 100bpm: 30 samples

	for (i=7; i>0; i--)
	  h->m_sample_num[i] = h->m_sample_num[i-1];

//h->m_sample_num[0] = sample_num;
	h->m_sample_num[0] = Kalman_Filter(sample_num);

  //-------------------------------------------------------
	// calculate average
	sum = 0;
	non_num = 0;
	for (i=0; i<8; i++)
	{
		if (h->m_sample_num[i]!=0)
		{
			non_num++;
			sum += h->m_sample_num[i];
		}
	}
	
	avg_sample_num = 0;
	while (sum>=0)
	{
		sum -= non_num;
		avg_sample_num++;
	}	
  //-------------------------------------------------------
	
	one_minute = 3000;
	heart_rate = 0;
	while (one_minute>=0)
	{
		one_minute -= avg_sample_num;
		heart_rate++;
	}
	
	N_SPRINTF("%d", heart_rate);
	
	h->update_cnt++;
	
	if ((h->update_cnt)&0x1) {
		h->current_rate = heart_rate;
		Y_SPRINTF("[PPG] current rate: %d", h->current_rate);
	}
	
	t_diff = cling.time.system_clock_in_sec - h->m_measuring_timer_in_s;
	
	if (t_diff>22) 
		_update_heart_rate_statistics();
}

static void _seek_cross_point(I16S instSample)
{
	HEARTRATE_CTX *h = &cling.hr;
	I32S t_curr;
	I32S t_diff;
	I32S t_peak_diff;
	
	I32U pulse_width_th1 = 0;
	I32U pulse_width_th2 = 0;
	I32U pulse_width_th3 = 0;
	
	t_curr = CLK_get_system_time();
	t_diff = t_curr - h->m_zero_point_timer;
	t_peak_diff = t_curr - h->m_pre_zero_point;
	
	if (h->m_pre_sample==0) {
		h->m_pre_sample = instSample;
		h->m_zero_point_timer = CLK_get_system_time();
		return;
	}
  
  if (t_diff>(20*2)) {
  	h->m_zero_point_timer = CLK_get_system_time();
  	
  	if (h->m_pre_sample<0 && instSample>0) {
  		if (h->m_pre_zero_point==0) {
  			h->m_pre_zero_point = t_curr;
  			h->m_pre_sample = instSample;
  			return;
  		} else {
  			if (h->m_pre_pulse_width==0) {
  				h->m_pre_sample = instSample;
  				
  				if (_heart_rate_overflow(t_peak_diff)) 
  					return;
  				
  				h->m_pre_zero_point  = t_curr;
  				h->m_pre_pulse_width = t_peak_diff;
					h->heart_rate_ready  = TRUE;
					// The time of latest heart rate measured
					h->heart_rate_measured_time = CLK_get_system_time();
  				_reset_heart_rate_calculator();
  				_calc_heart_rate();
  				return;
  			} else {
					pulse_width_th1 = (h->m_pre_pulse_width) - ((h->m_pre_pulse_width)>>3);
					pulse_width_th2 = (h->m_pre_pulse_width) + ((h->m_pre_pulse_width)>>3);
					pulse_width_th3 = (h->m_pre_pulse_width) + ((h->m_pre_pulse_width)>>1);
  			}
  			
  			if ( 
  				   (t_peak_diff<pulse_width_th1) || 
  				   (t_peak_diff>pulse_width_th2 && t_peak_diff<pulse_width_th3) 
  				 ) 
  			{
  				h->m_pre_sample = instSample;
  				return;
  			}
  			else if (t_peak_diff>pulse_width_th3) {
  				h->m_pre_sample = instSample;
  				t_peak_diff >>= 1;
  				
  				if (_heart_rate_overflow(t_peak_diff)) 
  					return;
  				
  				h->m_pre_zero_point = t_curr;
  				h->m_pre_pulse_width = t_peak_diff;
  				_calc_heart_rate();
  				return;
  			}
  			
  			if (_heart_rate_overflow(t_peak_diff)) 
  				return;
  			
  			h->m_pre_zero_point = t_curr;
  			h->m_pre_pulse_width = t_peak_diff;
  			_calc_heart_rate();
  		}
  	}
  	h->m_pre_sample = instSample;
  }
}

static void _ppg_sample_proc()
{
	HEARTRATE_CTX *h = &cling.hr;
	
	double high_pass_filter_val = 0.0;
	double low_pass_filter_val  = 0.0;
	
	I16S filt_sample = 0;
	I32S t_diff;
	I32S ledVal = 0;
	
	t_diff = cling.time.system_clock_in_sec - h->m_measuring_timer_in_s;
	
	// Input light strength - LedVal
	ledVal = _get_light_strength_register(LED2VAL);
	
	if (t_diff<4) {
//	_dynamic_power_adjust(ledVal);
	} else {
		N_SPRINTF("%d", ledVal);
	}
	
	high_pass_filter_val = Butterworth_Filter_HP( (double) ledVal);
	low_pass_filter_val  = Butterworth_Filter_LP(high_pass_filter_val);
	filt_sample = (I16S)low_pass_filter_val;
	
	if (t_diff>13) {
		N_SPRINTF("%d", filt_sample);
		_seek_cross_point(filt_sample);
	}
}

EN_STATUSCODE PPG_init()
{
	HEARTRATE_CTX *h = &cling.hr;
	I8U  i;
	
	h->state = PPG_STAT_IDLE;
	
	h->m_measuring_timer_in_s  = cling.time.system_clock_in_sec;
	h->m_zero_point_timer   = CLK_get_system_time();
	
  for (i=0; i<8; i++) h->m_sample_num[i] = 42;
		
	// Power down PPG after initialization
  _module_turn_off();

	// Init butterworth filter
	Butterworth_Filter_Init();
	
	h->b_perform_wearing_detection = TRUE;
	h->wearing_state = PPG_BODY_WEAR;
	h->current_rate = 70;
	h->heart_rate_ready = TRUE;
	return STATUSCODE_SUCCESS;
}

BOOLEAN PPG_switch_to_duty_on_state()
{
	HEARTRATE_CTX *h = &cling.hr;
	I32U t_curr = CLK_get_system_time();
	
	if (!_is_measure_allowed()) {
		return FALSE;
	}
	
	if (h->state == PPG_STAT_DUTY_ON) {
		return TRUE;
	}
	
	if (h->state != PPG_STAT_DUTY_OFF) {
		return FALSE;
	}
	
	N_SPRINTF("[PPG] Force switching to 'Duty ON'");
	
	h->state = PPG_STAT_WEARING_CHECKING;

	return TRUE;
}

BOOLEAN _is_user_viewing_heart_rate()
{
	
	// If user is asleep, we reduce the duty cycle by 2
	if ((cling.sleep.state == SLP_STAT_LIGHT) ||
		 (cling.sleep.state == SLP_STAT_SOUND) ||
		 (cling.sleep.state == SLP_STAT_REM))
	{
		return FALSE;
	}
		
	if (UI_is_idle()) {
		return FALSE;
	}
	
	if (cling.ui.frame_index != UI_DISPLAY_VITAL_HEART_RATE) {
		return FALSE;
	}
	
	return TRUE;
}

void PPG_state_machine()
{
	HEARTRATE_CTX *h = &cling.hr;
	I32U t_curr, t_diff, t_threshold, t_sec;
	I32S ledVal;
	
	t_sec = cling.time.system_clock_in_sec;
	
	// State machien switching
	switch (h->state) {
		case PPG_STAT_IDLE:
			{
				h->state = PPG_STAT_DUTY_OFF;
			}
			break;
		case PPG_STAT_WEARING_CHECKING:
			{
				// Force check wearing status
				h->wd_state = PPG_WEARING_IDLE;
				h->b_force_wearing_detection = TRUE;
				h->state = PPG_STAT_WEARING_CONFIRMED;
			}
			break;
		case PPG_STAT_WEARING_CONFIRMED:
			if (cling.hr.wd_state == PPG_WEARING_CONFIRM) {
				if (cling.hr.wearing_state == PPG_BODY_WEAR) {
					
					h->state = PPG_STAT_DUTY_ON;
				} else {
					
					// Update time stamp to delay 3 minutes before next measuring
					h->m_measuring_timer_in_s += PPG_NEXT_CHECK_LATENCY;

					h->state = PPG_STAT_DUTY_OFF;
				}
			}
			break;
		case PPG_STAT_DUTY_ON:
			// Update time stamp to start measuring right away
			h->m_measuring_timer_in_s = t_sec;
			Y_SPRINTF("[PPG] duty on: %d", h->m_measuring_timer_in_s);
			h->state = PPG_STAT_SAMPLE_READY;
		  _module_configured_for_measuring();			// Initialize Analog front end
			break;
		case PPG_STAT_SAMPLE_READY:
			{
				// Start system clock timer
				SYSCLK_timer_start();
				
				if (h->sample_ready) {
					h->sample_ready = FALSE;
										
					N_SPRINTF("[PPG] wearing confirmed, %d, %d", t_sec, h->m_measuring_timer_in_s);

					// Check if measuring is timed up
					t_diff = t_sec - h->m_measuring_timer_in_s;
					
					if (t_diff > PPG_SAMPLE_PROCESSING_PERIOD)
					{
						_ppg_sample_proc();				    // Process current sample
					}
					
					ledVal = _get_light_strength_register(LED1VAL);

					//if ( ((t_diff > PPG_HR_MEASURING_TIMEOUT) && UI_is_idle()) || ((ledVal>-1300000) && h->heart_rate_ready) ) {
					if (t_diff > PPG_HR_MEASURING_TIMEOUT) {
						_module_turn_off();					       // Turn off AFE module
						h->state = PPG_STAT_DUTY_OFF;
						Y_SPRINTF("[PPG] PPG (TIME OUT) State off, %d, %d", t_diff, cling.ui.state);
					}
				}
			}
			break;
		case PPG_STAT_DUTY_OFF:
			{
				t_diff = t_sec - h->m_measuring_timer_in_s;
				
				if (_is_user_viewing_heart_rate()) {
					t_threshold = PPG_MEASURING_PERIOD_FOREGROUND;
					
					if ( t_diff > t_threshold ) 
					{
						
						// If skin touch is positive, go ahead to measure HR
						// otherwise, using optical sensor to double check
						if (TOUCH_is_skin_touched())
							h->state = PPG_STAT_DUTY_ON;
						else
							h->state = PPG_STAT_WEARING_CHECKING;
						Y_SPRINTF("[PPG] PPG State on (force)");
					}
				} else {					
					
					t_threshold = cling.user_data.ppg_day_interval>>10;
					
					//t_threshold = 900;
					
					// If user is asleep, we reduce the duty cycle by 2
					if ((cling.sleep.state == SLP_STAT_LIGHT) ||
						 (cling.sleep.state == SLP_STAT_SOUND) ||
						 (cling.sleep.state == SLP_STAT_REM))
					{
						t_threshold = cling.user_data.ppg_night_interval>>10;
						//t_threshold = 1800;
						N_SPRINTF("[PPG] sleeping mode: %d", t_threshold);
					}

					//t_threshold = 60;
					
					// Normal background heart detection requires low power stationary mode
					if (t_diff < t_threshold) {
						break;
					}
									
					// Make sure it is in a low power stationary mode
					if (!cling.lps.b_low_power_mode) {
						break;
					}
						
					t_curr = CLK_get_system_time();
					// State in low power stationary for more than 5 seconds
					if (t_curr < (cling.lps.ts + PPG_WEARING_DETECTION_LPS_INTERVAL)) {
						break;
					}
					
					// Motion needed if current state is no-wearing
					if (h->wearing_state == PPG_BODY_NOT_WEAR) {
						if (!h->b_perform_wearing_detection) {
							break;
						}
					}
					
					Y_SPRINTF("[PPG] start ppg, %d, %d", t_sec, h->m_measuring_timer_in_s);
					
					// If skin touch is positive, go ahead to measure HR
					// otherwise, using optical sensor to double check
					if (TOUCH_is_skin_touched())
						h->state = PPG_STAT_DUTY_ON;
					else
						h->state = PPG_STAT_WEARING_CHECKING;
				}
			}
			break;
		default:
			break;
	}
}

BOOLEAN _need_check_wearing_state()
{
	I32U t_curr = CLK_get_system_time();
	
	// If force wearing detection, go ahead
	if (cling.hr.b_force_wearing_detection) {
		cling.hr.b_force_wearing_detection = FALSE;
		cling.hr.b_perform_wearing_detection = FALSE;
		
		Y_SPRINTF("[PPG] force wearing check");

		return TRUE;
	}

	// Do not conflict with regular HR detection
	if (cling.hr.state!=PPG_STAT_IDLE && cling.hr.state!=PPG_STAT_DUTY_OFF) { 
		return FALSE;
	}
	
	// Make sure it is in a low power stationary mode
	if (!cling.lps.b_low_power_mode) {
		return FALSE;
	}
	
	// State in low power stationary for more than 30 seconds
	if (t_curr < (cling.lps.ts + PPG_WEARING_DETECTION_LPS_INTERVAL)) {
		return FALSE;
	}
	
	// Only perform wearing detection once after device gets into stationary mode
	if (!cling.hr.b_perform_wearing_detection) {
		return FALSE;
	}
	
	// Make sure at least 2 minutes measuring interval
	if (t_curr < (cling.hr.wd_timestamp + PPG_WEARING_DETECTION_OVERALL_INTERVAL)) {
		return FALSE;
	}
	
	Y_SPRINTF("[PPG] LPS state - do the wearing check");

	cling.hr.b_perform_wearing_detection = FALSE;
	return TRUE;
}

void PPG_wearing_detection_state_machine()
{
	I32U t_curr = CLK_get_system_time();
	I32S ledVal = 0;

//	if (cling.hr.wd_state != stat2_check) {
//		Y_SPRINTF("CURRENT2 STATE: %d", cling.hr.wd_state);
//		stat2_check = cling.hr.wd_state;
//	}
	
	switch (cling.hr.wd_state) {
		case PPG_WEARING_IDLE:
			if (_need_check_wearing_state()) {
				
				// Configure module
				_module_configured_for_measuring();
				
				// set LED current to a fix setting
				_set_reg(CONTROL0, 0x000000);          // set ppg register set in WRITE mode
				_set_reg(LEDCNTRL, 0x012200);
				_set_reg(CONTROL0, 0x000001);          // set ppg register set back to READ mode
				
				// Start up system clock
				SYSCLK_timer_start();
				
				// Up to 80 samples to collect
				cling.hr.m_sample_sum = 0;
				cling.hr.m_sample_avg = 0;
				
				// Switch state
				cling.hr.wd_state = PPG_WEARING_MEASURING;
				
				// Recording measuring time stamp
				cling.hr.wd_timestamp = t_curr;
				
				Y_SPRINTF("[PPG] wearing check - start ...");
			}
			break;
		case PPG_WEARING_MEASURING:

			if ( cling.hr.sample_ready ) {
				cling.hr.m_sample_sum ++;
				cling.hr.sample_ready = FALSE;
				SYSCLK_timer_start();
				
				ledVal = _get_light_strength_register(LED1VAL);
				
				if (cling.hr.m_sample_sum < 16)
					break;
				else if (cling.hr.m_sample_sum < 80)  {
					cling.hr.m_sample_avg += ledVal;
									
					N_SPRINTF("[PPG] wearing check - measuring: %d ...", ledVal);

					//I_SPRINTF("%d  %d", ledVal, sum);
				} else {
					cling.hr.wd_state = PPG_WEARING_DETECTION;
					_module_turn_off();
				}
			}		
			break;
		case PPG_WEARING_DETECTION:
			cling.hr.m_sample_avg >>= 6;
      if (cling.hr.m_sample_avg>-500000 && cling.hr.m_sample_avg<-190000 ) {
				if (BATT_is_charging()) {
					Y_SPRINTF("[PPG] wearing check - Not wear! (CHarging)");
					cling.hr.wearing_state = PPG_BODY_NOT_WEAR;
				} else {
					Y_SPRINTF("[PPG] wearing check - Wear! (%d)", cling.hr.m_sample_avg);
					cling.hr.wearing_state = PPG_BODY_WEAR;
				}
			} else {
					Y_SPRINTF("[PPG] wearing check - Not wear! (%d)", cling.hr.m_sample_avg);
				cling.hr.wearing_state = PPG_BODY_NOT_WEAR;
			}
			cling.hr.wd_state = PPG_WEARING_CONFIRM;
			
			break;
		case PPG_WEARING_CONFIRM:
			if (t_curr > (cling.hr.wd_timestamp+PPG_WEARING_DETECTION_BACKIDLE_INTERVAL)) {
				cling.hr.wd_state = PPG_WEARING_IDLE;
				Y_SPRINTF("[PPG] wearing check - BACK TO idle");
			}
			break;
		default:
			break;
	}
}
#endif

BOOLEAN PPG_is_skin_touched()
{
	return TRUE;
#if 0
	if (BATT_is_charging())
		return FALSE;

	if (cling.hr.wearing_state == PPG_BODY_WEAR) {
		return TRUE;
	} else {
		if (TOUCH_is_skin_touched())
			return TRUE;
		else
			return FALSE;
	}
#endif
}
