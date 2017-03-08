/***************************************************************************/
/**
 * File: system.c
 * 
 * Description: system layer implementation
 *
 * Created on Jan 26, 2014
 *
 ******************************************************************************/
#include "main.h"
#include "system.h"
#include "fs_file.h"
#ifndef _CLING_PC_SIMULATION_
#include "ble_flash.h"
#include "pstorage.h"
#endif
#include "sysflash_rw.h"
#include "crc_16.h"


I16U CRCCCITT(I8U *data, I32U length, unsigned short seed, unsigned short final)
{
    I16U crc = seed;
    crc = cyg_crc16(data, length, crc);  
    return (crc ^ final);
}

I16U SYSTEM_CRC(I8U *data, I32U len, I16U seed)
{
    return CRCCCITT(data, len, seed, 0x0000);
}

void SYSTEM_get_ble_code(I8U *code)
{
	I8U dev_id[20];
	SYSTEM_get_dev_id(dev_id);
	
	code[0] = dev_id[16];
	code[1] = dev_id[17];
	code[2] = dev_id[18];
	code[3] = dev_id[19];
}

I8U SYSTEM_get_ble_device_name(I8U *device_name)
{
	I8U dev_id[20];
	I8U len;
	SYSTEM_get_dev_id(dev_id);
	
#ifdef _CLINGBAND_PACE_MODEL_
	len = sprintf((char *)device_name, "PBAND ");
#endif

#ifdef _CLINGBAND_NFC_MODEL_
	len = sprintf((char *)device_name, "NBAND ");
#endif
	
#ifdef _CLINGBAND_UV_MODEL_
	len = sprintf((char *)device_name, "CBAND ");
#endif
	
#ifdef _CLINGBAND_2_PAY_MODEL_ 
	len = sprintf((char *)device_name, "MBAND "); // Mobile payment
#endif
	
	device_name[len++] = dev_id[16];
	device_name[len++] = dev_id[17];
	device_name[len++] = dev_id[18];
	device_name[len++] = dev_id[19];
	device_name[len++] = 0;
	
	return len;
}

void SYSTEM_get_dev_id(I8U *twentyCharDevID) {
	I8U v8;
	I16U i;
	I8U *dev_data;
	I32U dev_info[2];
#ifndef _CLING_PC_SIMULATION_
	dev_info[0] = NRF_FICR->DEVICEID[0];
	dev_info[1] = NRF_FICR->DEVICEID[1];
#endif
	// for the 20 character device ID, we use the first 2 bytes, then skip 4B, then take the next 8B
	dev_data = (I8U *)dev_info;
	
	twentyCharDevID[0] = 'H'; // Hi
	twentyCharDevID[1] = 'I'; // Hi
	twentyCharDevID[2] = 'C'; // Cling
	
#ifdef _CLINGBAND_NFC_MODEL_
	twentyCharDevID[3] = 'N'; // NFC Band
#endif

#ifdef _CLINGBAND_UV_MODEL_
	twentyCharDevID[3] = 'B'; // UV Band
#endif

#ifdef _CLINGBAND_PACE_MODEL_
	twentyCharDevID[3] = 'P'; // Pace Band
#endif

#ifdef _CLINGBAND_2_PAY_MODEL_
	twentyCharDevID[3] = 'M'; // M Band (mobile payment)
#endif

	for (i = 0; i < 8; i++) {
		v8 = (dev_data[i]>>4)&0x0f;
		v8 += (v8>9)? 'A' - 0x0a : '0';
		twentyCharDevID[((i)<<1)+4] = v8;

		v8 = (dev_data[i])&0x0f;
		v8 += (v8>9)? 'A' - 0x0a : '0';
		twentyCharDevID[(((i)<<1)+1)+4] = v8;
	}
}

#ifndef _CLINGBAND_PACE_MODEL_
#ifdef _ENABLE_ANCS_
static void _notification_msg_init()
{
	I32U dw_buf[16];
	I8U *p_byte_addr;
	I32U offset = 0;
	
	// If this device is unauthorized, wipe out everything in the critical info section
	if (!LINK_is_authorized()) {
		cling.ancs.message_total = 0;
		FLASH_erase_App(SYSTEM_NOTIFICATION_SPACE_START);
		Y_SPRINTF("[SYSTEM] message: 0 (unauthorized)");
		return ;
	}

	p_byte_addr = (I8U *)dw_buf;
	cling.ancs.message_total = 0;
	while (offset < FLASH_ERASE_BLK_SIZE) {
		// Read out the first 4 bytes -
		FLASH_Read_App(SYSTEM_NOTIFICATION_SPACE_START+offset, p_byte_addr, 4);
			
		// Every message, we store 256 bytes, which includes message title and length info.
		if (p_byte_addr[0] == 0xFF) {
			break;
		}
		
		if (p_byte_addr[0] >= 128) {
			Y_SPRINTF("[SYSTEM] message (%d) wrong info - erased: %08x!", offset, dw_buf[0]);
			// something wrong
			cling.ancs.message_total = 0;
			FLASH_erase_App(SYSTEM_NOTIFICATION_SPACE_START);
			break;
		}
		offset += 256;
		cling.ancs.message_total ++;
					
		N_SPRINTF("[SYSTEM] message buffer update(%d) - %d, %d", 
		cling.ancs.message_total, p_byte_addr[0], p_byte_addr[1]);

	}
	
	// If nothing gets restored from system
	if (cling.ancs.message_total == 16) {
		Y_SPRINTF("[SYSTEM] message buffer full - erased!");
		cling.ancs.message_total = 0;
		FLASH_erase_App(SYSTEM_NOTIFICATION_SPACE_START);
	}
	
	Y_SPRINTF("[SYSTEM] message OVERALL: %d", cling.ancs.message_total);

}
#endif
#endif

static void _activity_info_restored(BOOLEAN b_reset)
{
	TRACKING_CTX *a = &cling.activity;
	
	if (b_reset) {
		// Make sure the minute file has correct offset
		a->tracking_flash_offset = TRACKING_get_daily_total(&a->day);
		
		Y_SPRINTF("-- tracking offset (critical restored) ---: %d", a->tracking_flash_offset);

		// Update stored total
		a->day_stored.walking = a->day.walking;
		a->day_stored.running = a->day.running;
		a->day_stored.distance = a->day.distance;
		a->day_stored.calories = a->day.calories;
		Y_SPRINTF("[SYSTEM] activity: %d, %d, %d, %d", a->day.walking, a->day.running, a->day.distance, a->day.calories);
		
		// Get sleep seconds by noon
		a->sleep_by_noon = TRACKING_get_sleep_by_noon(FALSE);
		a->sleep_stored_by_noon = TRACKING_get_sleep_by_noon(TRUE);
			
	} else {
		// Make sure the minute file has correct offset
		a->tracking_flash_offset = 0;
		a->day_stored.calories = a->day.calories;
		a->day_stored.distance = a->day.distance;
		a->day_stored.running = a->day.running;
		a->day_stored.walking = a->day.walking;

		// Get sleep by noon from flash
		a->sleep_by_noon = 0;
		a->sleep_stored_by_noon = 0;
		
	}
}

static BOOLEAN _critical_info_restored()
{
	// We should add system restoration for critical info
	CLING_TIME_CTX *t = &cling.time;
	I32U dw_buf[16];
	I8U *p_byte_addr;
	I32U offset = 0;
	
	// If this device is unauthorized, wipe out everything in the critical info section
	if (!LINK_is_authorized()) {
		USER_default_setup();
		
		return FALSE;
	}
	
	p_byte_addr = (I8U *)dw_buf;
	
	while (offset < FLASH_ERASE_BLK_SIZE) {
		// Read out the first 4 bytes -
		FLASH_Read_App(SYSTEM_INFORMATION_SPACE_START+offset, p_byte_addr, 4);
			
		// Every minute, we store 64 bytes, each 64 bytes is seperated as a block,
		// which saves info, including UTC time, time zone, activity info.
		if (p_byte_addr[0] == 0xFF) {
			break;
		}
		offset += 64;
	}
	
	// If nothing gets restored from system
	if (offset == 0) {
		cling.batt.b_no_batt_restored = TRUE;
		Y_SPRINTF("[SYSTEM] Notthing gets restored");
		return FALSE;
	}
	
	// If we do have something restored from system, good, initialize all the parameters that should be initialized.
	offset -= 64;
	
	// Read out the first 4 bytes -
	FLASH_Read_App(SYSTEM_INFORMATION_SPACE_START+offset, p_byte_addr, 64);
	
	// Epoch time
	t->time_since_1970 = dw_buf[0];
	
	// add 30 seconds to correct bias of time when system rebooting..
	t->time_since_1970 += 30;

	// Bytes: 4 - 38: reserved

	// Pm2.5
	cling.pm2p5 = p_byte_addr[39];
	cling.pm2p5 = (cling.pm2p5 << 8) | p_byte_addr[40];

	// Running seconds
	cling.run_stat.time_sec = p_byte_addr[41];
	
	// 42,43: HR skin attachment 
	cling.hr.b_closing_to_skin = p_byte_addr[42];
	cling.hr.current_rate = p_byte_addr[43];
	
	// Restore weather info
	cling.weather[0].day = p_byte_addr[44];
	cling.weather[0].month = p_byte_addr[45];
	cling.weather[0].type = p_byte_addr[46];
	cling.weather[0].high_temperature = p_byte_addr[47];
	cling.weather[0].low_temperature = p_byte_addr[48];

	// Restoring the time zone info
	t->time_zone = p_byte_addr[49];

	// Restoring amount of reminders
	cling.reminder.total = p_byte_addr[51];
#ifdef _ENABLE_ANCS_
	// Restore ancs supported set.
	cling.ancs.supported_categories = p_byte_addr[52];
  cling.ancs.supported_categories = (cling.ancs.supported_categories << 8) | p_byte_addr[53];	
	
	Y_SPRINTF("[SYSTEM] restored ANCS categories: %04x", cling.ancs.supported_categories);
#endif
#ifdef _ENABLE_TOUCH_
	// Restore Skin touch
	cling.touch.b_skin_touch = p_byte_addr[54];
#endif

	// 55: clock orientation.
	cling.ui.clock_orientation = p_byte_addr[55];
	
	// Restore battery level
	if (p_byte_addr[56] == 0)
		cling.batt.b_no_batt_restored = TRUE;
	else if (p_byte_addr[56] > 100)
		cling.batt.b_no_batt_restored = TRUE;
	else 
		cling.system.mcu_reg[REGISTER_MCU_BATTERY] = p_byte_addr[56];
		
	Y_SPRINTF("[SYSTEM] restore battery at: %d", p_byte_addr[56]);

	cling.batt.non_charging_accumulated_active_sec = p_byte_addr[57];
	cling.batt.non_charging_accumulated_steps = p_byte_addr[58];
	Y_SPRINTF("[SYSTEM] restore charging param: %d, %d", p_byte_addr[57], p_byte_addr[58]);

	cling.ui.language_type = p_byte_addr[59];
	cling.gcp.host_type = p_byte_addr[60];
	
	// Restore sleep state
	cling.sleep.state = (SLEEP_STATUSCODE)p_byte_addr[61];

	// Minute file critical timing info
	cling.system.reset_count = p_byte_addr[62];
	cling.system.reset_count = (cling.system.reset_count << 8) | p_byte_addr[63];

	if (cling.system.reset_count & 0x8000) {
		cling.system.reset_count = 0;
	} else {
		cling.system.reset_count ++;
	}
	
	// Always erase this segment
	FLASH_erase_App(SYSTEM_INFORMATION_SPACE_START);
	
	// Add latency right before backup critical info (Erasure latency: 50 ms)
	BASE_delay_msec(50);

	// Initialize INFO-D section with the new time
	SYSTEM_backup_critical();
	
	return TRUE;
}

#ifdef _ENABLE_UART_
static void _print_out_dev_name()
{
	char dev_name[21];
	memset(dev_name, 0, 21);
	SYSTEM_get_dev_id((I8U *)dev_name);
	Y_SPRINTF("####  system reboot  (%s)", dev_name);
}

static void _print_out_dev_version()
{
	I16U major;
	I16U minor;
	
	major = cling.system.mcu_reg[REGISTER_MCU_REVH]>>4;
	minor = cling.system.mcu_reg[REGISTER_MCU_REVH]&0x0f;
	minor <<= 8;
	minor |= cling.system.mcu_reg[REGISTER_MCU_REVL];
	Y_SPRINTF("#### ver: %d.%d ", major, minor);

}
#endif
static void _startup_logging()
{
#ifdef _ENABLE_UART_
	Y_SPRINTF("  ");
	Y_SPRINTF("####  ");
	_print_out_dev_name();
	Y_SPRINTF("####  ");
	_print_out_dev_version();
	Y_SPRINTF("####  ");
	Y_SPRINTF("[MAIN] WHO AM I -------- ");
	Y_SPRINTF("acc: 0x%02x, ", cling.whoami.accelerometer);
	Y_SPRINTF("issp: 0x%02x, ", cling.whoami.hssp);
	Y_SPRINTF("nor flash:0x%02x%02x, ", cling.whoami.nor[0], cling.whoami.nor[1]);
	Y_SPRINTF("Touch dev: %d.%d.%d", cling.whoami.touch_ver[0], cling.whoami.touch_ver[1], cling.whoami.touch_ver[2]);
	Y_SPRINTF("-----------------------");
	Y_SPRINTF("[MAIN] SYSTEM init ...");
#endif
}

void SYSTEM_init(void)
{
	BOOLEAN b_valid_info;
	
#if 0 // Enable it if wiping off everything on the Flash
	FLASH_erase_all(TRUE);
#endif
	
	// Start RTC timer
	RTC_Start();
			
	// Logging when syistem boots up
	_startup_logging();
	
	// Check whether this is a authenticated device
	LINK_init();

	// If our FAT file system is NOT mounted, please erase NOR flash, and create it
	if(!FAT_chk_mounted())
	{
		// Erase Nor Flash if device is not authorized or File system undetected
		if (LINK_is_authorized()) {
			FLASH_erase_all(FALSE);
			// Print out the amount of page that gets erased
			Y_SPRINTF("[MAIN] No FAT, With Auth ");
		} else {
			FLASH_erase_all(TRUE);
			
			// Print out the amount of page that gets erased
			Y_SPRINTF("[MAIN] No FAT, No Auth");
		}
		
		// Extra latency before initializing file system (Erasure latency: 50 ms)
		BASE_delay_msec(50);

		// File system initialization
		// clear FAT and root directory
		FAT_clear_fat_and_root_dir();
		
	} else if (!LINK_is_authorized()) {
			FLASH_erase_application_data(TRUE);
			// Extra latency before initializing file system (Erasure latency: 50 ms)
			BASE_delay_msec(50);
			// Print out the amount of page that gets erased
			Y_SPRINTF("[MAIN] YES FAT, No AUTH");		
	} else {
		Y_SPRINTF("[MAIN] YES FAT, YES AUTH");
	}
	
	// Init the file system
	FILE_init();

	// Restore critical system information, rtc and others
	// If nothing got stored before, this is an unauthorized device, let's initialize time
	//
	b_valid_info = _critical_info_restored();
	
#if defined(_CLINGBAND_2_PAY_MODEL_) || defined(_CLINGBAND_PACE_MODEL_)	
	// RTC hardware init with restored time zone info
	RTC_hw_init();
#endif
	
	// Restore activity info
	_activity_info_restored(b_valid_info);

#ifndef _CLINGBAND_PACE_MODEL_
#ifdef _ENABLE_ANCS_
	// Initialize smart notification messages
	_notification_msg_init();
#endif	
#endif
	
	// Over-the-air update check
	OTA_main();
#ifndef _CLING_PC_SIMULATION_	

	// Watchdog init, and disabled for debugging purpose
	Watchdog_Init();
#endif
	// Start first battery measurement
	BATT_start_first_measure();
	
	// Device specifics restoration
	// Initialize user profile (name, weight, height, stride length)
	USER_device_specifics_init();
}


BOOLEAN SYSTEM_get_mutex(I8U value)
{
	BOOLEAN b_mutex_obtained = FALSE;
	if (MUTEX_NOLOCK_VALUE == cling.system.mcu_reg[REGISTER_MCU_MUTEX]) {
		cling.system.mcu_reg[REGISTER_MCU_MUTEX] = value;
		b_mutex_obtained = TRUE;
	} else {
		if (value == cling.system.mcu_reg[REGISTER_MCU_MUTEX]) {
			b_mutex_obtained = TRUE;
		}
	}
	return b_mutex_obtained;
}

BOOLEAN SYSTEM_backup_critical()
{
	// We should add system restoration for critical info
	CLING_TIME_CTX *t = &cling.time;
	I32U cbuf[16];
	I8U *critical = (I8U *)cbuf;
	I32U offset = 0;
		
	while (offset < FLASH_ERASE_BLK_SIZE) {
		// Read out the first 4 bytes -
		FLASH_Read_App(SYSTEM_INFORMATION_SPACE_START+offset, critical, 4);
			
		// Every minute, we store 64 bytes, each 64 bytes is seperated as a block,
		// which saves info, including UTC time, time zone, activity info.
		if (critical[0] == 0xFF) {
			break;
		}
		offset += 64;
	}
	
	if (offset >= FLASH_ERASE_BLK_SIZE) {
		N_SPRINTF("[FLASH] erase critical space as it gets full");

		FLASH_erase_App(SYSTEM_INFORMATION_SPACE_START);
		// add latency before backing up critical info (Erasure latency: 50 ms)
		BASE_delay_msec(50);
		offset = 0;
	}

	// Reset critical buffer
	memset(critical, 0, 64);

	// Bytes: 4 - 38: reserved
	
	// Store time
	cbuf[0] = t->time_since_1970;

	// Pm2.5
	critical[39] = (I8U)((cling.pm2p5>>8) & 0xFF);
	critical[40] = (I8U)(cling.pm2p5 & 0xFF);

	// Running
	critical[41] = cling.run_stat.time_sec;
	
	// 42, 43: HR skin attachment
	critical[42] = cling.hr.b_closing_to_skin;
	critical[43] = cling.hr.current_rate;
	
	// Store weather info
	critical[44] = cling.weather[0].day;
	critical[45] = cling.weather[0].month;
	critical[46] = cling.weather[0].type;
	critical[47] = cling.weather[0].high_temperature;
	critical[48] = cling.weather[0].low_temperature;
	
	// Store time zone info to prevent unexpected day rollover
	critical[49] = t->time_zone;

	// Store total reminders
	critical[51] = cling.reminder.total;
#ifdef _ENABLE_ANCS_
	// Store ancs supported set.
	critical[52] = (I8U)((cling.ancs.supported_categories>>8) & 0xFF);
	critical[53] = (I8U)(cling.ancs.supported_categories & 0xFF);
#endif
#ifdef _ENABLE_TOUCH_	
	// Store skin touch type
	critical[54] = cling.touch.b_skin_touch;
#endif
	// 55: Clock orientation
	critical[55] = cling.ui.clock_orientation;
	
	// Store battery level
	critical[56] = cling.system.mcu_reg[REGISTER_MCU_BATTERY];
	if (critical[56] == 0) {
		critical[56] = 1;
	}
	
	N_SPRINTF("[SYSTEM] backup battery at: %d", critical[56]);
	
	critical[57] = cling.batt.non_charging_accumulated_active_sec;
	critical[58] = cling.batt.non_charging_accumulated_steps;
	critical[59] = cling.ui.language_type;
	N_SPRINTF("[SYSTEM] Backup charging param: %d, %d", cling.batt.non_charging_accumulated_active_sec, 
		cling.batt.non_charging_accumulated_steps);

	// Store the  phone type
	critical[60] = cling.gcp.host_type;
	critical[61] = cling.sleep.state;
	// Store the reset count	
	critical[62] = (I8U)((cling.system.reset_count>>8) & 0xFF);
	critical[63] = (I8U)((cling.system.reset_count) & 0xFF);

	// Put it in info flash area
	FLASH_Write_App(SYSTEM_INFORMATION_SPACE_START+offset, critical, 64);
	
	//
	N_SPRINTF("[SYSTEM] system backedup (%d)", cling.user_data.day.steps);
	
	return TRUE;
}

// System check sume
void SYSTEM_checksum(void *pdata, I32U size, I8U *checksum)
{
	I32U sum = 0;
  I8U *ptr = (I8U *)pdata;

  while (size-- > 0) sum += *ptr++;

	checksum[0] = (sum >> 24)&0xff;
	checksum[1] = (sum >> 16)&0xff;
	checksum[2] = (sum >> 8)&0xff;
	checksum[3] = sum&0xff;
}

void SYSTEM_factory_reset()
{
	BLE_CTX *r = &cling.ble;
	
	Y_SPRINTF("[SYSTEM] factory reset - BLE disconnect");
	
	// Disconnect BLE service
	if (BTLE_is_connected()) 
	BTLE_disconnect(BTLE_DISCONN_REASON_FACTORY_RESET);
	
	// Enable factory reset
	r->disconnect_evt |= BLE_DISCONN_EVT_FACTORY_RESET;
	
}

void SYSTEM_reboot()
{
	BLE_CTX *r = &cling.ble;
	
	Y_SPRINTF("[SYSTEM] rebooting ...");
	
	if (BTLE_is_connected()) {
		// Disconnect BLE service
		BTLE_disconnect(BTLE_DISCONN_REASON_REBOOT);
		
		// Enable factory reset
		r->disconnect_evt |= BLE_DISCONN_EVT_REBOOT;

		return;
	}
	
	// Jumpt to reset vector
	SYSTEM_restart_from_reset_vector();
}

void SYSTEM_format_disk(BOOLEAN b_erase_data)
{
	I16U page_erased = FLASH_erase_all(b_erase_data);
	
	Y_SPRINTF("[FS] system format disk ....");
	
	// File system initialization
	// clear FAT and root directory
	FAT_clear_fat_and_root_dir();
	
	// Before formating disk, adding some latency (Erasure latency: 50 ms)
	BASE_delay_msec(50);

	// Re-initialize file system and user profile & milestone
	FILE_init();

	Y_SPRINTF("[SYSTEM] erase %d blocks", page_erased);
}

void SYSTEM_restart_from_reset_vector()
{
	Y_SPRINTF("[SYSTEM] restarting system");
	
	if (LINK_is_authorized()) {
		// For an authorized device
		// Backup system critical information before reboot
		SYSTEM_backup_critical();
	}
#ifndef _CLING_PC_SIMULATION_
	
	// Reboot system.
	sd_nvic_SystemReset();
#endif
}

void SYSTEM_release_mutex(I8U value)
{
	if (value == cling.system.mcu_reg[REGISTER_MCU_MUTEX]) {
		cling.system.mcu_reg[REGISTER_MCU_MUTEX] = MUTEX_NOLOCK_VALUE;
	}
}

