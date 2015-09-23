//
//  File: hal.h
//  
//  Description: Hardware abstruct layer header
//
//  Created on Feb 26, 2014
//
#ifndef __HAL_HEADER__
#define __HAL_HEADER__

#ifndef _CLING_PC_SIMULATION_	
#include "device_manager.h"
#endif

#define LED_PORT       NRF_GPIO_PORT_SELECT_PORT2
#define LED_OFFSET     2

#define BLINKY_STATE_MASK   0x01

#define WAKEUP_BUTTON_PIN                    NRF6310_BUTTON_0                           /**< Button used to wake up the application. */
#define BONDMNGR_DELETE_BUTTON_PIN_NO        NRF6310_BUTTON_1                           /**< Button used for deleting all bonded masters during startup. */

#define APP_TIMER_PRESCALER                  0                                          /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS                 3                                          /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE              3                                          /**< Size of timer operation queues. */

#define SENSOR_CONTACT_DETECTED_INTERVAL     APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER) /**< Sensor Contact Detected toggle interval (ticks). */

#define FLASH_PAGE_SYS_ATTR                 (BLE_FLASH_PAGE_END - 3)                    /**< Flash page used for bond manager system attribute information. */
#define FLASH_PAGE_BOND                     (BLE_FLASH_PAGE_END - 1)                    /**< Flash page used for bond manager bonding information. */

#define DEAD_BEEF                            0xDEADBEEF                                 /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

void HAL_init(void);
void HAL_advertising_start(void);
BOOLEAN HAL_set_conn_params(BOOLEAN b_fast);
BOOLEAN HAL_set_slow_conn_params(void);
void HAL_disconnect_for_fast_connection(void);

#endif
