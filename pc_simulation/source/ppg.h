/***************************************************************************//**
 * @file ppg.h
 * @brief Driver implementation for TI AFE4400 Analog Front-End for
 * @Heart Rate Monitors and Low-Cost Pulse Oximeters.
 *
 ******************************************************************************/

#ifndef __PPG_H__
#define __PPG_H__

#include "standards.h"

#define PPG_MEASURING_PERIOD_FOREGROUND        1    // real time measuring
#define PPG_SAMPLE_PROCESSING_PERIOD           2
#define PPG_HR_MEASURING_TIMEOUT               25
#define PPG_NEXT_CHECK_LATENCY                 180

#define PPG_WEARING_DETECTION_OVERALL_INTERVAL 180000// 180 seconds
#define PPG_WEARING_DETECTION_LPS_INTERVAL 5000 // 5 seconds
#define PPG_WEARING_DETECTION_BACKIDLE_INTERVAL 30000// 300 seconds


typedef enum {
	PPG_BODY_NOT_WEAR, 
	PPG_BODY_WEAR, 
	PPG_UNKNOWN
} PPG_WEARING_STATUS;

typedef enum {
	PPG_WEARING_IDLE,
	PPG_WEARING_MEASURING,
	PPG_WEARING_DETECTION,
	PPG_WEARING_CONFIRM
} PPG_WEARING_DETECTION_STATES;

#define PPG_WEAR_TH1    -1700000
#define PPG_WEAR_TH2    -1200000
#define PPG_WEAR_TH3    -1000000

typedef enum {
	PPG_SUCCESS,
	PPG_NO_SKIN_TOUCH,
	PPG_INIT_AFE_FAIL,
} PPG_STATUSCODE;

typedef enum {
	PPG_STAT_IDLE,
	PPG_STAT_WEARING_CHECKING,
	PPG_STAT_WEARING_CONFIRMED,
	PPG_STAT_DUTY_ON,
	PPG_STAT_SAMPLE_READY,
	PPG_STAT_DUTY_OFF,
} PPG_STATES;

typedef struct tagHEARTRATE_CTX{
	I32U m_measuring_timer_in_s;
	I32U m_zero_point_timer;
	
	BOOLEAN sample_ready;
	BOOLEAN heart_rate_ready;
	I32U heart_rate_measured_time;

	// PPG state machine
	PPG_STATES state;
	PPG_WEARING_DETECTION_STATES wd_state;
	I8U  m_sample_sum;
	I32S m_sample_avg;
	I32U wd_timestamp;
	PPG_WEARING_STATUS wearing_state;
	BOOLEAN b_force_wearing_detection;
	BOOLEAN b_perform_wearing_detection;

	// Heart rate statistics
	I8U current_rate;
	I8U update_cnt;
	I8U high_rate;
	I8U low_rate;
	I8U minute_rate;
	
	// Heart rate calculation variables
	I32U  m_pre_zero_point;
	I16S  m_pre_sample;
	I16U  m_pre_pulse_width;
		
	I8U   m_sample_num[8];
	BUTT_CTX butterworth_filter_context;
} HEARTRATE_CTX;

EN_STATUSCODE PPG_init(void);
void PPG_state_machine(void);
BOOLEAN PPG_switch_to_duty_on_state(void);
void PPG_wearing_detection_state_machine(void);
BOOLEAN PPG_is_skin_touched(void);

//+++++++++++++++++++++++++++++++++++++++++++++++++++
// timing period constants
//+++++++++++++++++++++++++++++++++++++++++++++++++++
#define PPG_REMOVE_DC_WINDOW_LEN   8
#define PPG_POWER_ADJ_PERIOD       4

#define PPG_SAMPLE_THRESHOLD  600

#define PPG_MEAS_LENGTH_2_S   134
#define PPG_MEAS_LENGTH_3_S   200
#define PPG_MEAS_LENGTH_4_S   267
#define PPG_MEAS_LENGTH_6_S   333
#define PPG_MEAS_LENGTH_5_S   400
#define PPG_MEAS_LENGTH_7_S   467
#define PPG_MEAS_LENGTH_8_S   533
#define PPG_MEAS_LENGTH_9_S   600
#define PPG_MEAS_LENGTH_10_S  667
#define PPG_MEAS_LENGTH_12_S  800
#define PPG_MEAS_LENGTH_15_S  1000
#define PPG_MEAS_LENGTH_18_S  1200
#define PPG_MEAS_LENGTH_20_S  1334
#define PPG_MEAS_LENGTH_27_S  1800

#define PPG_HR_TABLE_SIZE     61
#define PPG_HR_TABLE_BOTM     20

//+++++++++++++++++++++++++++++++++++++++++++++++++++
// AFE4400 registers index
//+++++++++++++++++++++++++++++++++++++++++++++++++++
#define CONTROL0           0
#define LED2STC            1
#define LED2ENDC           2
#define LED2LEDSTC         3
#define LED2LEDENDC        4
#define ALED2STC           5
#define ALED2ENDC          6
#define LED1STC            7
#define LED1ENDC           8
#define LED1LEDSTC         9
#define LED1LEDENDC       10
#define ALED1STC          11
#define ALED1ENDC         12
#define LED2CONVST        13
#define LED2CONVEND       14
#define ALED2CONVST       15
#define ALED2CONVEND      16
#define LED1CONVST        17
#define LED1CONVEND       18
#define ALED1CONVST       19
#define ALED1CONVEND      20
#define ADCRSTSTCT0       21
#define ADCRSTENDCT0      22
#define ADCRSTSTCT1       23
#define ADCRSTENDCT1      24
#define ADCRSTSTCT2       25
#define ADCRSTENDCT2      26
#define ADCRSTSTCT3       27
#define ADCRSTENDCT3      28
#define PRPCOUNT          29
#define CONTROL1          30
#define SPARE1            31
#define TIAGAIN           32
#define TIA_AMB_GAIN      33
#define LEDCNTRL          34
#define CONTROL2          35
#define SPARE2            36
#define SPARE3            37
#define SPARE4            38
#define RESERVED1         39
#define RESERVED2         40
#define ALARM             41
#define LED2VAL           42
#define ALED2VAL          43
#define LED1VAL           44
#define ALED1VAL          45
#define LED2MINUSALED2VAL 46
#define LED1MINUSALED1VAL 47
#define DIAG              48

#endif // __PPG_H__
/** @} */
