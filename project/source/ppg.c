
#include "main.h"

#define PPG_LOGGING N_SPRINTF

static EN_STATUSCODE ppg_read_reg(I8U cmdID, I8U bytes, I8U *pRegVal)
{
#ifndef _CLING_PC_SIMULATION_

	I32U err_code;
	const nrf_drv_twi_t twi = NRF_DRV_TWI_INSTANCE(1);
	
	if (pRegVal==NULL) return STATUSCODE_NULL_POINTER;
	
	if (!cling.system.b_twi_1_ON) {
		GPIO_twi_enable(1);
	}
	
	err_code = nrf_drv_twi_tx(&twi, (ppg_I2C_ADDR>>1), &cmdID, 1, true);
	if (err_code == NRF_SUCCESS) {
		N_SPRINTF("[PPG] Read TX PASS: ");
	} else {
		PPG_LOGGING("[PPG] Read TX FAIL - %d", err_code);
		APP_ERROR_CHECK(err_code);
		return STATUSCODE_FAILURE;
	}
	err_code = nrf_drv_twi_rx(&twi, (ppg_I2C_ADDR>>1), pRegVal, bytes, false);
	if (err_code == NRF_SUCCESS) {
		N_SPRINTF("[PPG] Read RX PASS: ");
		return STATUSCODE_SUCCESS;
	} else {
		PPG_LOGGING("[PPG] Read RX FAIL: %d", err_code);
		APP_ERROR_CHECK(err_code);
		return STATUSCODE_FAILURE;
	}
#else
	return STATUSCODE_SUCCESS;
#endif
}

static BOOLEAN ppg_write_reg(I8U cmdID, I8U regVal)
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
	
	err_code = nrf_drv_twi_tx(&twi, (ppg_I2C_ADDR>>1), acData, 2, false);
	if (err_code == NRF_SUCCESS) {
		N_SPRINTF("[PPG] Write PASS: 0x%02x  0x%02x", cmdID, regVal);
		return STATUSCODE_SUCCESS;
	} else {
		PPG_LOGGING("[PPG] Write FAIL(%d): 0x%02x  0x%02x", err_code, cmdID, regVal);
		APP_ERROR_CHECK(err_code);
		return STATUSCODE_FAILURE;
	}
#else
	return STATUSCODE_SUCCESS;
#endif
}

static void _calc_heart_rate()
{
	HEARTRATE_CTX *h = &cling.hr;
	
	I8U  epoch_num;
	I16S pulse_width;	
	I8S  heart_rate;	
	I32S one_minute;
	I32S sum;
	I8S  non_num;
	I8S  avg_epoch_num;
	I8U  i;	
#ifdef __YLF__
	I16S epoch_max=0,epoch_min=0;
#endif
	
	// current pulse width
	epoch_num = 0;
	pulse_width = h->m_pre_pulse_width;
	while (pulse_width>=0) {
  	pulse_width -= 20;        // 20 milliseconds per sample
		epoch_num++;             // samples number in one heart rate pulse
	}	
	
	N_SPRINTF("[PPG] sample num: %d", epoch_num);
	
	// 50bpm: 60 samples
	// 60bpm: 50 samples
	// 70bpm: 43 samples
	// 80bpm: 38 samples
	// 90bpm: 33 samples
	// 100bpm: 30 samples
#ifndef __YLF__
	for (i=5; i>0; i--)
#else
	for (i=7; i>0; i--)
#endif
		N_SPRINTF("[PPG] sample num: %dth,%d", i,h->m_epoch_num[i-1]);
	  h->m_epoch_num[i] = h->m_epoch_num[i-1];
#ifndef __YLF__
	if(h->m_epoch_cnt<3){
		//if((h->m_epoch_num[0] - epoch_num)>300||(h->m_epoch_num[0] - epoch_num)<-300)
		if((epoch_num>33) &&(epoch_num<47)){
				h->m_epoch_num[0] = epoch_num;
		}
		h->m_epoch_cnt ++;
	}else{
		h->m_epoch_num[0] = epoch_num;
	}
#else
	h->m_epoch_num[0] = epoch_num;
#endif
	
	//h->m_epoch_num[0] = Kalman_Filter(epoch_num);

  //-------------------------------------------------------
	// calculate average
	sum = 0;
	non_num = 0;
#ifndef __YLF__
	for (i=0; i<6; i++)
#else
	for (i=0; i<8; i++)
#endif
	{
		if (h->m_epoch_num[i]!=0)
		{
#ifdef __YLF__
			if(epoch_max < h->m_epoch_num[i])
				epoch_max = h->m_epoch_num[i];
			if(epoch_min > h->m_epoch_num[i])
				epoch_min = h->m_epoch_num[i];
#endif
			non_num++;
			sum += h->m_epoch_num[i];
		}
	}
#ifdef __YLF__	
	if(epoch_max!=0){
		sum -= epoch_max;
		non_num--;
	}
	if(epoch_min!=0){
		sum -= epoch_min;
		non_num--;
	}
#endif
	avg_epoch_num = 0;
	while (sum>=0)
	{
		sum -= non_num;
		avg_epoch_num++;
	}
#ifndef __YLF__	
	h->m_epoch_num[0] = Kalman_Filter(avg_epoch_num);
	//h->m_epoch_num[0] = avg_epoch_num;
#endif
  //-------------------------------------------------------
	
  one_minute = 3000;
	heart_rate = 0;
	while (one_minute>=0)
	{
		one_minute -= avg_epoch_num;
		heart_rate++;
	}
	
	N_SPRINTF("%d", heart_rate);
	
	h->update_cnt++;
	
	if ((h->update_cnt)&0x1) {
		h->current_rate = heart_rate;
		N_SPRINTF("[PPG] current rate: %d", h->current_rate);
	}
}

static void _reset_heart_rate_calculator()
{
	HEARTRATE_CTX *h = &cling.hr;
	I8U  i;
	I8U  epoch_num;
	I16S pulse_width;
	
	epoch_num = 0;
	pulse_width = h->m_pre_pulse_width;
	while (pulse_width>=0) {
		pulse_width -= 20;
		epoch_num++;
	}
#ifndef __YLF__
	h->m_epoch_cnt = 0;
	for (i=0; i<6; i++) h->m_epoch_num[i] = epoch_num;
#else
	for (i=0; i<8; i++) h->m_epoch_num[i] = epoch_num;
#endif
}

static BOOLEAN _heart_rate_overflow(I16S epoch_num)
{
	HEARTRATE_CTX *h = &cling.hr;
	I32S t_curr;

	t_curr = CLK_get_system_time();
	
//if ( epoch_num<(15*20) || epoch_num>(67*20) ) {      // 15=(60*50)/(200rpm), 67=(60*50)/(45rpm)
	if ( epoch_num<(  300) || epoch_num>( 1340) ) {      //  6=(60*25)/(200rpm), 27=(60*25)/(45rpm)
		h->m_pre_zero_point  = t_curr;
//	h->m_pre_pulse_width = 0;
		return TRUE;
	}
	return FALSE;
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
	
	if (h->m_pre_ppg_sample==0) {
		h->m_pre_ppg_sample = instSample;
		h->m_zero_point_timer = CLK_get_system_time();
		return;
	}
  
  if (t_diff>(20*2)) {
  	h->m_zero_point_timer = CLK_get_system_time();
  	
  	if (h->m_pre_ppg_sample<0 && instSample>0) {
  		if (h->m_pre_zero_point==0) {
  			h->m_pre_zero_point = t_curr;
  			h->m_pre_ppg_sample = instSample;
  			return;
  		} else {
  			if (h->m_pre_pulse_width==0) {
  				h->m_pre_ppg_sample = instSample;
  				
  				if (_heart_rate_overflow(t_peak_diff)) 
  					return;
  				
  				h->m_pre_zero_point  = t_curr;
  				h->m_pre_pulse_width = t_peak_diff;
					h->heart_rate_ready  = TRUE;
					h->b_closing_to_skin = TRUE;
					h->b_start_detect_skin_touch = FALSE;
					h->first_hr_measurement_in_ms = CLK_get_system_time();
					// The time of latest heart rate measured
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
  				h->m_pre_ppg_sample = instSample;
  				return;
  			}
  			else if (t_peak_diff>pulse_width_th3) {
  				h->m_pre_ppg_sample = instSample;
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
  	h->m_pre_ppg_sample = instSample;
  }
}

void PPG_closing_to_skin_detect_init()
{
	HEARTRATE_CTX *h = &cling.hr;

	h->b_start_detect_skin_touch = TRUE;
  h->m_closing_to_skin_detection_timer = CLK_get_system_time();
	h->approximation_decision_ts = CLK_get_system_time();
	h->m_sample_cnt = 0;
	h->m_sample_sum = 0;
	N_SPRINTF("[PPG] start approximation at %d ----", h->approximation_decision_ts);
}

static void _skin_touch_detect(I16S sample)
{
	HEARTRATE_CTX *h = &cling.hr;
	I32S sample_val;
	I32S t_curr, t_diff;

	t_curr = CLK_get_system_time();	
	N_SPRINTF("[PPG] counter: %4d", h->m_sample_cnt);
	
	h->m_sample_cnt++;
	if (h->m_sample_cnt<PPG_SAMPLE_COUNT_THRESHOLD) {
		h->m_sample_sum += sample;
		h->m_closing_to_skin_detection_timer = t_curr;
		return;
	} else {
		h->m_sample_cnt = PPG_SAMPLE_COUNT_THRESHOLD;

		sample_val = (h->m_sample_sum) >> 5;
		h->m_sample_sum  = sample_val * (PPG_SAMPLE_COUNT_THRESHOLD - 1);
		h->m_sample_sum += sample;

		sample_val = (h->m_sample_sum) >> 5;
		N_SPRINTF("[PPG] sample_val: %d",sample_val);
#ifndef __YLF__
		if ((sample_val>PPG_SAMPLE_AVERATE_THRESHOLD)&&(sample_val<27000)) {
#else
		if ((sample_val>PPG_SAMPLE_AVERATE_THRESHOLD)||(sample_val<(-PPG_SAMPLE_AVERATE_THRESHOLD)) ){//YLF2017.3.24
#endif
			h->m_closing_to_skin_detection_timer = CLK_get_system_time();
		}
	}
	
	t_diff = t_curr - h->m_closing_to_skin_detection_timer;
	if (t_diff > 500) {
		h->b_start_detect_skin_touch = FALSE;
    h->b_closing_to_skin = FALSE;    
		h->heart_rate_ready  = FALSE;
		h->approximation_decision_ts = t_curr;
		N_SPRINTF("[PPG] ppg is detached at %d ---- %d", h->approximation_decision_ts, t_diff);
	}
}
static I16S _get_light_strength_register()
{
	I16S proxData;
	I8U  buf[2];

	ppg_read_reg(REGS_PS1_DATA0,  1, (buf+0));
	ppg_read_reg(REGS_PS1_DATA1,  1, (buf+1));
	
	proxData = ((I16S)buf[1]<<8) | buf[0];

	return proxData;
}

static void _ppg_sample_proc()
{
	HEARTRATE_CTX *h = &cling.hr;

	double high_pass_filter_val = 0.0;
	double low_pass_filter_val  = 0.0;
	I16S sample = 0;	
	I16S filt_sample = 0;
	I32U t_curr_ms = CLK_get_system_time();
	I32U t_ms_diff;
	
	t_ms_diff = t_curr_ms - h->m_duty_on_time_in_ms;

	sample = _get_light_strength_register();

	_skin_touch_detect(sample);

	high_pass_filter_val = Butterworth_Filter_HP( (double) sample);
	low_pass_filter_val  = Butterworth_Filter_LP(high_pass_filter_val);
	filt_sample = (I16S)low_pass_filter_val;
#ifndef __YLF__
	if (t_ms_diff > 3000) {//3 Seconds
#else
	if (t_ms_diff > 6000) {
#endif
//if (TRUE) {
		N_SPRINTF("%d  %d", filt_sample, sample);
		/* check whether current epoch ppg sample is zero-crossing */
  	_seek_cross_point(filt_sample);
	}
}
/*------------ !!! PPG SUBROUTINES END   !!! ------------*/



/*------------ !!! BASIC SUBROUTINES FOR LT1PH03 DRIVER START !!! ------------*/
void ppg_Send_Command(I8U cmd)      { ppg_write_reg(REGS_COMMAND, cmd); }
void ppg_Send_Parameter(I8U param)  { ppg_write_reg(REGS_PARAM_WR, param); }
void ppg_Send_Command_Parameter(I8U cmd, I8U param)
{
	ppg_Send_Parameter(param);
	ppg_Send_Command(cmd);
}
void ppg_Send_Command_Query(I8U offset, I8U param) { ppg_Send_Command_Parameter((offset+0x80), param); }
void ppg_Send_Command_SET  (I8U offset, I8U param) { ppg_Send_Command_Parameter((offset+0xA0), param); }
void ppg_Send_Command_AND  (I8U offset, I8U param) { ppg_Send_Command_Parameter((offset+0xC0), param); }
void ppg_Send_Command_OR   (I8U offset, I8U param) { ppg_Send_Command_Parameter((offset+0xE0), param); }
void ppg_Send_Command_NOP()    { ppg_Send_Command(0x00); }
void ppg_Send_Command_Reset()  { ppg_Send_Command(0x01); }
void ppg_PS_Force()            { ppg_Send_Command(0x05); }
void ppg_ALS_Force()           { ppg_Send_Command(0x06); }
void ppg_PS_ALS_Force()        { ppg_Send_Command(0x07); }
void ppg_PS_Pause()            { ppg_Send_Command(0x09); }
void ppg_ALS_Pause()           { ppg_Send_Command(0x0a); }
void ppg_PS_ALS_Pause()        { ppg_Send_Command(0x0B); }
void ppg_PS_Auto()             { ppg_Send_Command(0x0D); }
void ppg_ALS_Auto()            { ppg_Send_Command(0x0E); }
void ppg_PS_ALS_Auto()         { ppg_Send_Command(0x0F); }
void ppg_Modify_Command_I2C_Addr(I8U i2c_addr)
{
	ppg_Send_Command_SET(I2C_ADDR, i2c_addr);
	ppg_Send_Command(0x02);
}
/*------------ !!! BASIC SUBROUTINES FOR LT1PH03 DRIVER END   !!! ------------*/

void ppg_Init_Sensor()
{
  I8U reg_defaults[] = {
  	0x00,                                                                 // part id
	  0x00,                                                                 // rev id
	  0x00,                                                                 // seq id
  	B_INT_STICKY      | B_INT_OUTPUT_DISABLE,                             // int cfg
  	B_CMD_INT_ENABLE  | B_PS1_INT_ENBALE       | B_ALS_INT_DISABLE,       // irq enable
  	B_PS1_IM_COMPLETE | B_ALS_IM_NONE,                                    // int mode1
  	B_CMD_INT_ERR,                                                        // int mode2
  	0x17,                                                                 // hw key
	  B_MEAS_RATE_10MS,                                                     // meas rate
  	B_ALS_RATE_FORCE,                                                     // als rate
  	B_PS_RATE_1,                                                          // ps rate
	  0x00,                                                                 // als low  th  7:0
  	0x00,                                                                 // als low  th 15:8
  	0x00,                                                                 // als high th  7:0
  	0x00,                                                                 // als high th 15:8
//  B_PS_LED_NONE,                                                        // ps led drive current
//  B_PS_LED_5_6MA,
//  B_PS_LED_11_2MA,
//  B_PS_LED_22_4MA,
    B_PS_LED_44_8MA,  
  };
	
	BASE_delay_msec(50);

	ppg_Send_Command_Reset();

  BASE_delay_msec(10);

  ppg_write_reg(REGS_HW_KEY, reg_defaults[REGS_HW_KEY]);

	ppg_Send_Command_SET(CHLIST,0x01);        //  enble ps only

	ppg_write_reg(REGS_INT_CFG,    reg_defaults[REGS_INT_CFG]);
	ppg_write_reg(REGS_IRQ_ENABLE, reg_defaults[REGS_IRQ_ENABLE]);
	ppg_write_reg(REGS_IRQ_MODE1,  reg_defaults[REGS_IRQ_MODE1]);
	ppg_write_reg(REGS_IRQ_MODE2,  reg_defaults[REGS_IRQ_MODE2]);
	ppg_write_reg(REGS_MEAS_RATE,  reg_defaults[REGS_MEAS_RATE]);
	ppg_write_reg(REGS_ALS_RATE,   reg_defaults[REGS_ALS_RATE]);
	ppg_write_reg(REGS_PS_RATE,    reg_defaults[REGS_PS_RATE]);
	ppg_write_reg(REGS_PS_LED21,   reg_defaults[REGS_PS_LED21]);
}

void ppg_Enable_Sensor()
{
	ppg_write_reg(REGS_IRQ_STATUS, 0x27);  // Clear all int
	ppg_write_reg(REGS_INT_CFG, B_INT_STICKY|B_INT_OUTPUT_ENABLE);
	
	ppg_Send_Command_SET(CHLIST,0x01);         // Enble ps only
	ppg_PS_Auto();

#if 1
	// TBD: Need Tuning...
  ppg_Send_Command_SET(PS_ENCODING,  0x60);       // LSB 16 bits of 17bit ADC
  ppg_Send_Command_SET(PS_ADC_GAIN,  0x03);
  ppg_Send_Command_SET(PS1_ADCMUX,   0x00);
  ppg_Send_Command_SET(PS_ADC_MISC,  0x04);
	
//ppg_Send_Command_SET(ALS_ENCODING, 0x30);
//ppg_Send_Command_SET(PS1_ADCMUX,   0x00);
#endif
}

void PPG_disable_sensor()
{
	HEARTRATE_CTX *h = &cling.hr;

	/* pause */
//ppg_PS_Pause();
//ppg_ALS_Pause();
  ppg_PS_ALS_Pause();

	/* disable mearing rate */
	ppg_write_reg(REGS_MEAS_RATE, 0);
	ppg_write_reg(REGS_ALS_RATE,  0);
	ppg_write_reg(REGS_PS_RATE,   0);
	
	h->b_start_detect_skin_touch = FALSE;
}

static void _configure_sensor()
{
	HEARTRATE_CTX *h = &cling.hr;

	// 1. Initlize and Enable sensor.
  ppg_Init_Sensor();
  ppg_Enable_Sensor();
	
	// 2. Initlize ppg context variables each measuring stage.
	h->m_pre_zero_point  = 0;
	h->m_pre_ppg_sample  = 0;
	h->m_pre_pulse_width = 0;
	h->update_cnt        = 0;
}


void PPG_init()
{
	HEARTRATE_CTX *h = &cling.hr;
	I8U i;
	
	N_SPRINTF("[PPG] initialization");

	I8U partid = 0xaa;
	I8U revid  = 0xbb;
	I8U seqid  = 0xcc;

	ppg_read_reg(REGS_PART_ID, 1, &partid);
	ppg_read_reg(REGS_REV_ID,  1, &revid);
	ppg_read_reg(REGS_SEQ_ID,  1, &seqid);	
	N_SPRINTF("LT1PH03 0x%02x  0x%02x  0x%02x", partid, revid, seqid);

	PPG_disable_sensor();

	// Initialize butterworth filter
	// below initialization code segment only called once
	Butterworth_Filter_Init();

	h->state = PPG_STAT_IDLE;

	h->current_rate      = 70;
	h->minute_rate       = 70;
	
	h->m_zero_point_timer   = CLK_get_system_time();
#ifndef __YLF__
	h->b_training = FALSE;
#endif
#ifndef __YLF__
  for (i=0; i<6; i++) h->m_epoch_num[i] = 42;
	h->m_epoch_cnt = 0;
#else
  for (i=0; i<8; i++) h->m_epoch_num[i] = 42;
#endif
}

BOOLEAN _is_user_viewing_heart_rate()
{
#if 1	
	if (OLED_panel_is_turn_off()) {

		if (!cling.activity.b_workout_active) {
			if (cling.hr.heart_rate_ready) {
				cling.hr.heart_rate_ready  = FALSE;
			}
		}
		return FALSE;
	}
	
	if (cling.ui.frame_index != UI_DISPLAY_VITAL_HEART_RATE) {
		return FALSE;
	}

	return TRUE;
#endif

#if 0	
	if (cling.activity.b_workout_active)
		return TRUE;

	if (OLED_panel_is_turn_off()) {
		cling.hr.heart_rate_ready  = FALSE;
		return FALSE;
	}	
	
	if (cling.ui.frame_index != UI_DISPLAY_VITAL_HEART_RATE)  {
		cling.hr.heart_rate_ready  = FALSE;		
		return FALSE;
	}
	
	return TRUE;
#endif	
}

void PPG_state_machine()
{
	HEARTRATE_CTX *h = &cling.hr;
	I32U t_step_diff_sec, t_threshold;
	I32U t_curr_ms, t_curr_sec;
	I32U t_ms_diff, t_sec_diff, t_measuring_ms_diff;
	
	if (OTA_if_enabled()) {
		return;
	}
	
	t_curr_ms = CLK_get_system_time();
	t_curr_sec = cling.time.system_clock_in_sec;
	
	// PPG measuring main state machine
	switch (h->state) {

		case PPG_STAT_IDLE:
			h->state = PPG_STAT_DUTY_OFF;
		  break;

		case PPG_STAT_DUTY_ON:
		{
			// Update time stamp to start measuring right away
			h->m_measuring_timer_in_sec = t_curr_sec;
			h->m_duty_on_time_in_ms = t_curr_ms;
			h->first_hr_measurement_in_ms = t_curr_ms; // Assuming we don't have a HR read yet
			h->state = PPG_STAT_SAMPLE_READY;
			N_SPRINTF("[PPG] duty on: %d", h->m_measuring_timer_in_s);
  	  _configure_sensor();			// initialize the PPG module
		}
		break;

		case PPG_STAT_SAMPLE_READY:		
		{						
			t_step_diff_sec = t_curr_sec - cling.activity.step_detect_t_sec;
		  RTC_start_operation_clk();
		  if (h->sample_ready) {
			  h->sample_ready = FALSE;
				N_SPRINTF("[PPG] wearing confirmed, %d, %d", t_sec, h->m_measuring_timer_in_s);
				
				// Check if measuring is timed up
				t_ms_diff = t_curr_ms - h->m_duty_on_time_in_ms;
				if (t_ms_diff > PPG_SAMPLE_PROCESSING_PERIOD) {
					if (h->heart_rate_ready) {
						if (cling.activity.b_workout_active && (t_step_diff_sec < 5)) {
							_ppg_sample_proc();				    // Process current sample
						} else {
							// If heart rate is ready, we should just keep measuring for no more than 5 sec
							t_measuring_ms_diff = t_curr_ms - h->first_hr_measurement_in_ms;
							if (t_measuring_ms_diff > PPG_CONTINEOUS_MEASURING_TH) {
								// If user is not viewing HR page, exit measuring state
								// otherwise, simply just stop updating
								if (!_is_user_viewing_heart_rate()) {
									PPG_disable_sensor();					       // Turn off ppg sensor module
									h->state = PPG_STAT_DUTY_OFF;
								} else {
									// If user is viewing HR page, but no steps for over 30 seconds, go 
									// turn off sensors
									if (t_step_diff_sec > 30) {
										PPG_disable_sensor();					       // Turn off ppg sensor module
										h->state = PPG_STAT_DUTY_OFF;
									}
								}
							} else {
								_ppg_sample_proc();				    // Process current sample
							}
						}
					} else {
						_ppg_sample_proc();				    // Process current sample
					}
        }

  			if ((t_ms_diff > PPG_HR_MEASURING_TIMEOUT) && !_is_user_viewing_heart_rate() ) {
          PPG_disable_sensor();					       // Turn off ppg sensor module
          h->state = PPG_STAT_DUTY_OFF;
          N_SPRINTF("[PPG] PPG (TIME OUT) State off, %d, %d", t_diff, cling.ui.state);
				}
			}

			if ((!h->b_closing_to_skin) && (!h->b_start_detect_skin_touch)) {
				PPG_LOGGING("[PPG] not closing to skin.");				
				PPG_disable_sensor();
				h->state = PPG_STAT_DUTY_OFF;
			}
		}
		break;

		case PPG_STAT_DUTY_OFF:
		{		
			t_sec_diff = t_curr_sec - h->m_measuring_timer_in_sec;
			t_step_diff_sec = t_curr_sec - cling.activity.step_detect_t_sec;

			// If device is in a charging state, do not perform PPG detection
			if (BATT_is_charging()) {
				h->b_closing_to_skin = FALSE;
				h->heart_rate_ready  = FALSE;
				break;
			}
			
			if (cling.activity.b_workout_active) {
				// Start PPG detection right away if the device is in motion
#ifndef __YLF__
				h->b_training = TRUE;
#endif
				if (!cling.lps.b_low_power_mode) {
					h->state = PPG_STAT_DUTY_ON;
					// We need initialize skin touch detection routine
					PPG_closing_to_skin_detect_init();
					PPG_LOGGING("[PPG] PPG State on (force)");
				}
			} else if (_is_user_viewing_heart_rate() && (cling.hr.b_closing_to_skin || cling.hr.b_start_detect_skin_touch)) 
			{
				h->state = PPG_STAT_DUTY_ON;
				PPG_LOGGING("[PPG] PPG State on (force)");
			} else {
				if (t_step_diff_sec < PPG_MEASURING_PERIOD_BACKGROUND_NIGHT) {
					t_threshold = PPG_MEASURING_PERIOD_BACKGROUND_DAY;
				} else {
					t_threshold = PPG_MEASURING_PERIOD_BACKGROUND_NIGHT;
				}

				// Normal background heart detection requires low power stationary mode
				if (t_sec_diff < t_threshold) {
					break;
				}

				// Make sure it is in a low power stationary mode
				if (!cling.lps.b_low_power_mode) {
					break;
				}

				// State in low power stationary for more than 5 seconds
				if (t_curr_ms < (cling.lps.ts + PPG_WEARING_DETECTION_LPS_INTERVAL)) {
					break;
				}
				
				// If device stays in background LPS for more than 60 minutes, no heart rate detection detection needed
				if (t_curr_ms > (cling.lps.ts + PPG_WEARING_DETECTION_BG_IDLE_INTERVAL)) {
					break;
				}

				N_SPRINTF("[PPG] start ppg, %d, %d", t_sec, h->m_measuring_timer_in_s);

				// otherwise, using optical sensor to double check
				h->state = PPG_STAT_DUTY_ON;
				
				// We need initialize skin touch detection routine
				PPG_closing_to_skin_detect_init();
			}
		}
		break;

		default:
			break;
	}
}

I8U PPG_minute_hr_calibrate(BOOLEAN b_training)
{
	I8U hr_diff;
	I8U hr_rendering;
#ifndef __YLF__
	if(!b_training){
		return (cling.hr.current_rate);
	}else{
		if ((cling.hr.minute_rate < 90) && (cling.hr.current_rate < 90)) {
			hr_rendering = cling.hr.current_rate;
		} else {
			if (cling.hr.current_rate > cling.hr.minute_rate) {
				hr_diff = cling.hr.current_rate - cling.hr.minute_rate;
				if (hr_diff < 10) {
					hr_rendering = cling.hr.current_rate;
				} else {
					hr_rendering = (cling.hr.minute_rate+(hr_diff&0x07));
				} 
			} else {
				hr_diff = cling.hr.minute_rate-cling.hr.current_rate;
				
				if (hr_diff < 16)
					hr_rendering = cling.hr.current_rate;
				else 
					hr_rendering = (cling.hr.minute_rate - (hr_diff&0x07));
			}
		}
		
		if (hr_rendering < 60) {
			if (cling.hr.minute_rate > cling.hr.current_rate)
				hr_diff = cling.hr.minute_rate - cling.hr.current_rate;
			else
				hr_diff = cling.hr.current_rate - cling.hr.minute_rate;
			hr_rendering = 60 + (hr_diff & 0x07);
		} 
		return hr_rendering;
	}
#endif
}
