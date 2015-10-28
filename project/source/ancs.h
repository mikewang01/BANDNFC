/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 */

/** @file
 *
 * @defgroup ble_sdk_srv_ancs_c Apple Notification Service Client
 * @{
 * @ingroup ble_sdk_srv
 * @brief Apple Notification module - Disclaimer: This module (Apple Notification Center Service) can and will be changed at any time by either Apple or Nordic Semiconductor ASA.
 *
 * @details This module implements the Apple Notification Center Service (ANCS) Client.
 *
 * @note The application must propagate BLE stack events to the Apple Notification Client module
 *       by calling ANCS_on_ble_evt(void) from the from the @ref ble_stack_handler callback.
 */
#ifndef _ANCS_H__
#define _ANCS_H__

#define _ENABLE_ANCS_

#ifndef _CLING_PC_SIMULATION_
#include "ble_ancs_c.h"
#include "standard_types.h"

#define   ANCS_ATTR_TITLE_FLAG       0
#define   ANCS_ATTR_MESSAGE_FLAG     1

#define   ANCS_FILTERING_OLD_MSG_DELAY_TIME    5000     /*5000ms*/  
#define   ANCS_SUPPORT_MAX_TITLE_LEN           64       /*64 byte*/
#define   ANCS_SUPPORT_MAX_MESSAGE_LEN         192      /*192 byte*/

//ANCS get message and store states 
enum {
	ANCS_IDLE,
	ANCS_GET_ATTR,	
	ANCS_STORE_ATTR
};


typedef struct tagANCS_PACKET {
	
	I8U   title_len;
	I8U   message_len;
	I8U   buf[254];
} ANCS_PACKET;

typedef struct tagANCS_CONTEXT {

	I16U supported_categories;
	
	I8U state;
  ble_ancs_c_t   m_ancs_c;                        /**< Structure used to identify the Apple Notification Service Client. */
	ANCS_PACKET    pkt;
	I8U            message_total;
	BOOLEAN        ancs_attr_get_flag;
	BOOLEAN        ancs_attr_store_flag;		
  I8U            filtering_flag;
} ANCS_CONTEXT;


void    ANCS_service_add(void);
void    ANCS_nflash_store_one_message(I8U *data);
void    ANCS_state_machine(void);
#endif

#endif // _ANCS_H__

/** @} */

