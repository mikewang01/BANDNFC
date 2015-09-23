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

//#define _ENABLE_ANCS_

/**@brief Category IDs for iOS notifications. */
typedef enum
{
    ANCS_CATEGORY_ID_OTHER,
    ANCS_CATEGORY_ID_INCOMING_CALL,
    ANCS_CATEGORY_ID_MISSED_CALL,
    ANCS_CATEGORY_ID_VOICE_MAIL,
    ANCS_CATEGORY_ID_SOCIAL,
    ANCS_CATEGORY_ID_SCHEDULE,
    ANCS_CATEGORY_ID_EMAIL,
    ANCS_CATEGORY_ID_NEWS,
    ANCS_CATEGORY_ID_HEALTH_AND_FITNESS,
    ANCS_CATEGORY_ID_BUSINESS_AND_FINANCE,
    ANCS_CATEGORY_ID_LOCATION,
    ANCS_CATEGORY_ID_ENTERTAINMENT,
		ANCS_CATEGORY_ID_MAX,
} ble_ancs_category_id_values_t;

typedef struct tagANCS_CONTEXT {

	I16U supported_categories;
	BOOLEAN b_enabled;
#if 0
	I8U cccd_enable_count_down;
	#endif
	I8U cat_count[ANCS_CATEGORY_ID_MAX];
	
} ANCS_CONTEXT;

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the ANCS Client.
 *
 * @param[in]   p_ancs     ANCS Client structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
#ifndef _CLING_PC_SIMULATION_
void ANCS_on_ble_evt(const ble_evt_t * p_ble_evt);
void ANCS_uuid_set(ble_uuid_t *uuid);
#endif
void ANCS_service_add(void);
void ANCS_start_ancs_discovery(void);
BOOLEAN ANCS_notifications_to_read(void);
I8U ANCS_get_next_new_notification(I8U index);
void ANCS_apple_notification_setup(void);

#endif // _ANCS_H__

/** @} */

