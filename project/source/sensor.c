/***************************************************************************
 *
 * File: sensor.c
 * 
 * Description: sensor processing routines
 *
 * Created on Jan 29, 2014
 *
 ******************************************************************************/



#include "main.h"
#include  "wrist_detect.h"
#ifdef __WIRST_DETECT_ENABLE__
static 	CLASS(wrist_detect) *p_wrist_obj = NULL;
#endif
void _low_power_process_hw(I8U int_source)
{
	I32U t_diff_ms = CLK_get_system_time();
	
	t_diff_ms -= cling.lps.ts;
	
	// 
	// INT1 is spurious, so if set, read the INT cause and report in the FIFO
	//
	
	if (int_source & 0x6a) {
		N_SPRINTF("[SENSOR] Acc interrupt: %02x", int_source);
		TRACKING_exit_low_power_mode(FALSE);
	}
	
	// if system stays in low power mode for more than 2 hour, wake up 
	if ((SLEEP_is_sleep_state(SLP_STAT_SOUND)) || (SLEEP_is_sleep_state(SLP_STAT_LIGHT))) {
		if (t_diff_ms >= 1800000) {
			TRACKING_exit_low_power_mode(TRUE);
		}
	}
}

static void _navigation_wrist_shaking(I8U jitter_counts)
{
#ifdef _ENABLE_TOUCH_
	if (jitter_counts > 1) {
		N_SPRINTF(" +++ jitter: %d +++\n", jitter_counts);
		if (!UI_is_idle()) {
			if ((cling.ui.frame_index >= UI_DISPLAY_CAROUSEL) && (cling.ui.frame_index <= UI_DISPLAY_CAROUSEL_END)) {
				cling.touch.b_valid_gesture = TRUE;
				cling.touch.gesture = TOUCH_SWIPE_LEFT;
			} else {
				if (cling.ui.frame_index != UI_DISPLAY_SMART_MESSAGE) {
					cling.touch.b_double_tap = TRUE;
				}
			}
		}
	}
#endif
}
#ifndef __WIRST_DETECT_ENABLE__
static void _screen_activiation_wrist_flip(ACCELEROMETER_3D G, I8U accCnt, I32U t_curr, BOOLEAN b_motion)
{
	I8U i;
	I8U up_counts, side_counts;
	I32U x, y, z, t_diff;
	DEVICE_ORIENTATION_TYPE currOrientation;

	N_SPRINTF("[SENSOR] face up: %d", face_up);
	
	if (accCnt) {
		G.x /= accCnt;
		G.y /= accCnt;
		G.z /= accCnt;
		
		x = BASE_abs(G.x);
		y = BASE_abs(G.y);
		z = BASE_abs(G.z);
		
		N_SPRINTF("[SENSOR] orientation: %d, %d, %d", G.x, G.y, G.z);
	} else {
		x = 0;
		y = 0;
		z = 0;
		G.z = 0;
		G.x = 0;
		G.y = 0;
	}
	
	if ((G.z < -1600) && (x < 700) && (G.y < 200) && (G.y > -1200)) {
		cling.activity.orientation[cling.activity.face_up_index] = FACE_UP;
		currOrientation = FACE_UP;
		N_SPRINTF("[SENSOR] --- FACE UP ---");
	} else if ((z < 500) && ((x > 1600) || (y > 1600))) {
		cling.activity.orientation[cling.activity.face_up_index] = FACE_SIDE;
		currOrientation = FACE_SIDE;
		N_SPRINTF("[SENSOR] --- FACE SIDE ---");
	} else {
		cling.activity.orientation[cling.activity.face_up_index] = FACE_UNKNOWN;
		currOrientation = FACE_UNKNOWN;
		N_SPRINTF("[SENSOR] --- FACE UNKNOWN ---");
	}

	cling.activity.face_up_index++;
	if (cling.activity.face_up_index >= 5) {
		cling.activity.face_up_index = 0;
	}
	
	// motion time stamp
	if (b_motion) {
		cling.activity.motion_ts = t_curr;
		N_SPRINTF("[SENSOR] -- motion!!! ---");
	}
	
	// Motion should be well detected within 2 seconds.
	t_diff = t_curr - cling.activity.motion_ts;
	
	if (t_diff > 2000) {
		N_SPRINTF("[SENSOR] -- return (no motion): %d ---", t_diff);
		return;
	}
	
	if (currOrientation != FACE_UP) {
		N_SPRINTF("[SENSOR] -- return (not face up orientation) ---");
		return;
	}
	
	// Wrist flip detection
	up_counts = 0;
	side_counts = 0;
	for (i = 0; i < 5; i++) {
		if (cling.activity.orientation[i] == FACE_UP)
			up_counts ++;
		if (cling.activity.orientation[i] == FACE_SIDE)
			side_counts ++;
	}
	
	if ((up_counts>= 1) && (up_counts <= 2)) {
		N_SPRINTF("-------- [SENSOR] fliped !!!! -----\n");
		
		if (LINK_is_authorized()) {
			
			if (UI_is_idle()) {
				// Turn on screen
				UI_turn_on_display(UI_STATE_TOUCH_SENSING, 20);
			}
		}
	}
}
#endif
#define STATIONARY_ENG_TH  48 /* 0.023 g */

#ifndef _USE_HW_MOTION_DETECTION_
static void _low_power_process_sw()
{
	I8U i, sample_count;
	ACC_AXIS xyz;
	ACCELEROMETER_3D A;
	BOOLEAN b_motion = FALSE;
	I32S diff, max_diff;
	I32S mag, prev, v;
	I32S mag_win[4];

	// Check if we have new data available.
	sample_count = LIS3DH_is_FIFO_ready();
	
	if (!sample_count)
		return;
#ifdef USING_VIRTUAL_ACTIVITY_SIM
		return;
#endif
	// Reset the accumulator
	b_motion = FALSE;
	v = 0;
	max_diff = 0;
	
	for (i = 0; i < sample_count; i++) {
		
		LIS3DH_retrieve_data(&xyz);
			
		// Convert (normalization) input 3-axis data to 32-bit signed for internal process
		// Accelerometer output format: 4G, pedometer input format: 16G
		A.x = (I32S)(xyz.x>>2);
		A.y = (I32S)(xyz.y>>2);
		A.z = (I32S)(xyz.z>>2);
		
		// Calculate the magnitude and we might not need filter out the magnitude
		//
		if (A.x > 0)
			mag = A.x;
		else
			mag = -A.x;
		
		if (A.y > 0)
			mag += A.y;
		else
			mag -= A.y;
		
		if (A.z > 0)
			mag += A.z;
		else
			mag -= A.z;
		
		// filtering needed, here, we use a simple RC filter to smooth out the magnitude
		// 
		if (i == 0) {
			prev = mag;
			v = mag;
			mag_win[0] = mag_win[1] = mag_win[2] = mag_win[3] = mag;
		} else {
			prev = v;
			v = prev + ((mag-prev)>>4);
		}
		
    // a second filter to smooth out magnitude for stationary detection
	
    mag_win[0] = mag_win[1];
    mag_win[1] = mag_win[2];
    mag_win[2] = mag_win[3];
    mag_win[3] = v;

		if (mag_win[0] > mag_win[3]) {
			diff = mag_win[0] - mag_win[3];
		} else {
			diff = mag_win[3] - mag_win[0];;
		}
		
    if (diff > STATIONARY_ENG_TH) {
			b_motion = TRUE;
    }
		
		if (diff > max_diff) {
			max_diff = diff;
		}

		// Sleep monitoring module
		SLEEP_algorithms_proc(&xyz);
	}

	if (b_motion) {
		TRACKING_exit_low_power_mode(FALSE);
	N_SPRINTF("-- max diff: %d", max_diff);
	}
	
}
#endif

static void _high_power_process_FIFO()
{
	I8U i, sample_count;
	I8U jitter_counts, iSample, accCnt;
	ACC_AXIS xyz;
	ACCELEROMETER_3D A;
	ACCELEROMETER_3D G;
	I32U x, y, t_curr;
	BOOLEAN b_motion = FALSE;
#ifdef __WIRST_DETECT_ENABLE__
	UNUSED_VARIABLE(b_motion);
	UNUSED_VARIABLE(t_curr);
#endif
	// Get current ms
	t_curr = CLK_get_system_time();
	
	// Check if we have new data available.
	sample_count = LIS3DH_is_FIFO_ready();
	
	
	if (!sample_count)
		return;
	
			N_SPRINTF("[SENSOR] sample count: %d", sample_count);

#ifdef USING_VIRTUAL_ACTIVITY_SIM
		return;
#endif
	jitter_counts = 0;
	cling.activity.z_mean = 0;
	
	// Reset the accumulator
	G.x = 0;
	G.y = 0;
	G.z = 0;
	
	if (sample_count > 15) {
		iSample = sample_count - 15;
	} else {
		iSample = 0;
	}
	accCnt = 0;

	for (i = 0; i < sample_count; i++) {
		
		LIS3DH_retrieve_data(&xyz);
			
		// Convert (normalization) input 3-axis data to 32-bit signed for internal process
		// Accelerometer output format: 4G, pedometer input format: 16G
		A.x = (I32S)(xyz.x>>2);
		A.y = (I32S)(xyz.y>>2);
		A.z = (I32S)(xyz.z>>2);
		
		// Calculate Z mean
		cling.activity.z_mean += A.z;

		N_SPRINTF("[SENSOR] data: %d,%d,%d,", A.x, A.y, A.z);
		
		x = BASE_abs(A.x);
		y = BASE_abs(A.y);
		
		if ((A.z < -1700) && ((x > 1700) || (y > 1700))) {
			jitter_counts ++;
		}
		
		// Set to 1.5g threshold for identifying a motion
		if ((x > 2300) || (y > 2300)) {
			b_motion = TRUE;
		}
		
		// Accumulating gravity
		if (i >= iSample) {
			G.x += A.x;
			G.y += A.y;
			G.z += A.z;
			accCnt ++;
		}
		
		// Pedometer core data processing	
		TRACKING_algorithms_proc(A);
		
		// Sleep monitoring module
		SLEEP_algorithms_proc(&xyz);
#ifdef __WIRST_DETECT_ENABLE__
		if(cling.user_data.b_screen_wrist_flip){
			/*wrist detetction moudule*/
			p_wrist_obj->gravity_obtain(p_wrist_obj, A.x, A.y, A.z,  CLK_get_system_time());
		}
#endif
		 
	}
	
	// See if user purposely shakes device to navigate
	//
	if (cling.user_data.b_navigation_wrist_shake) {
		_navigation_wrist_shaking(jitter_counts);
	}
#ifndef __WIRST_DETECT_ENABLE__
	if (cling.user_data.b_screen_wrist_flip) {
		_screen_activiation_wrist_flip(G, accCnt, t_curr, b_motion);
	}
#endif
}

void SENSOR_accel_processing()
{
	I8U int_pin;
	I8U int_source;
	ACC_AXIS xyz;
	
	memset(&xyz, 0, sizeof(ACC_AXIS));
	
	if (cling.lps.b_low_power_mode) {
#ifdef _USE_HW_MOTION_DETECTION_
		SLEEP_algorithms_proc(&xyz); 
#endif
		
		N_SPRINTF("[SENSOR] Touch & Sensor: %d, %d, %d", cling.lps.b_touch_deep_sleep_mode, t_curr, cling.lps.ts);
		
	}
#ifndef _CLING_PC_SIMULATION_
	
	// One known bug: Accelerometer interrupts process 2x times than expected.
	int_pin = nrf_gpio_pin_read(GPIO_SENSOR_INT_1);	
	if (int_pin == 0) {
		N_SPRINTF("[LIS3DH] Not an accelerometer interrupt, %d", CLK_get_system_time());
		return;
	}
#endif
	int_source = LIS3DH_get_interrupt();
	if (cling.lps.b_low_power_mode) {
		// Software motion detection
#ifdef _USE_HW_MOTION_DETECTION_
		_low_power_process_hw(int_source);
#else
		_low_power_process_sw();
#endif
	} else 
	{
		// INT2 (accelerometer data ready),
		_high_power_process_FIFO();
	}
	
}

#ifdef __WIRST_DETECT_ENABLE__
int wake_up_screen()
{
	UI_turn_on_display(UI_STATE_IDLE, 0);
	return TRUE;
}
#endif

void SENSOR_init()
{
#ifndef _CLING_PC_SIMULATION_
	// delay before waking up accelerometer
	BASE_delay_msec(10);
	
	// Initialize accelerometer.
	LIS3DH_normal_FIFO();
	
	cling.whoami.accelerometer = LIS3DH_who_am_i();
#endif	
	// Initialize sleep monitoring module.
	SLEEP_init();
#ifdef __WIRST_DETECT_ENABLE__
	/*get wrist detection instance*/
	p_wrist_obj = wrist_detect_get_instance();
	p_wrist_obj->screen_wakeup_callback_register(p_wrist_obj, wake_up_screen);
#endif
}

