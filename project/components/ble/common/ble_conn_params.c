/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "ble_conn_params.h"
#include <stdlib.h>
#include "nordic_common.h"
#include "ble_hci.h"
#include "app_timer.h"
#include "ble_srv_common.h"
#include "app_util.h"
#include "main.h"

#define  BLE_CONN_PARAMS_DEBUG 1

static ble_conn_params_init_t m_conn_params_config;     /**< Configuration as specified by the application. */
static ble_gap_conn_params_t  m_preferred_conn_params;  /**< Connection parameters preferred by the application. */
static ble_gap_conn_params_t  m_on_conn_params;  /**< Connection parameters preferred by the application. */
static uint8_t                m_update_count;           /**< Number of Connection Parameter Update messages that has currently been sent. */
static uint16_t               m_conn_handle;            /**< Current connection handle. */
static ble_gap_conn_params_t  m_current_conn_params;    /**< Connection parameters received in the most recent Connect event. */
static app_timer_id_t         m_conn_params_timer_id;   /**< Connection parameters timer. */

static uint16_t  andorid_update_times  = 0;
static uint16_t  ios_update_times  = 0;
static bool is_default_params_on_first_conn = true;

static bool m_change_param = false;
static uint8_t  device_type = CONN_PARAMS_MGR_DEVICE_NULL;/*try to record current device taht is connected to device*/
static int conn_mgr_disconnect_for_fast_connection( ble_gap_conn_params_t  *conn_params);


static bool is_conn_params_ok(ble_gap_conn_params_t * p_conn_params)
{
    // Check if interval is within the acceptable range.
    // NOTE: Using max_conn_interval in the received event data because this contains
    //       the client's connection interval.
    if (
        (p_conn_params->max_conn_interval >= m_preferred_conn_params.min_conn_interval)
        && 
        (p_conn_params->max_conn_interval <= m_preferred_conn_params.max_conn_interval)
       )
    {
        return true;
    }
    else
    {
        return false;
    }
}


static void update_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);

    if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        // Check if we have reached the maximum number of attempts
        m_update_count++;
        if (m_update_count <= m_conn_params_config.max_conn_params_update_count)
        {
            uint32_t err_code;

            // Parameters are not ok, send connection parameters update request.
            err_code = sd_ble_gap_conn_param_update(m_conn_handle, &m_preferred_conn_params);
            if ((err_code != NRF_SUCCESS) && (m_conn_params_config.error_handler != NULL))
            {
                m_conn_params_config.error_handler(err_code);
            }
        }
        else
        {
            m_update_count = 0;

            // Negotiation failed, disconnect automatically if this has been configured
            if (m_conn_params_config.disconnect_on_fail)
            {
                uint32_t err_code;

                err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
                if ((err_code != NRF_SUCCESS) && (m_conn_params_config.error_handler != NULL))
                {
                    m_conn_params_config.error_handler(err_code);
                }
            }

            // Notify the application that the procedure has failed
            if (m_conn_params_config.evt_handler != NULL)
            {
                ble_conn_params_evt_t evt;

                evt.evt_type = BLE_CONN_PARAMS_EVT_FAILED;
                m_conn_params_config.evt_handler(&evt);
            }
        }
    }
}


uint32_t ble_conn_params_init(const ble_conn_params_init_t * p_init)
{
    uint32_t err_code;

    m_conn_params_config = *p_init;
    m_change_param = false;
	  /*reset first connection default params bit*/
	  is_default_params_on_first_conn = true;
	
    if (p_init->p_conn_params != NULL)
    {
        m_preferred_conn_params = *p_init->p_conn_params;

        // Set the connection params in stack
        err_code = sd_ble_gap_ppcp_set(&m_preferred_conn_params);
        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }
    }
    else
    {
        // Fetch the connection params from stack
        err_code = sd_ble_gap_ppcp_get(&m_preferred_conn_params);
        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }
    }
		
    err_code = sd_ble_gap_ppcp_get(&m_on_conn_params);
    APP_ERROR_CHECK(err_code);
		
    m_conn_handle  = BLE_CONN_HANDLE_INVALID;
    m_update_count = 0;

    return app_timer_create(&m_conn_params_timer_id,
                            APP_TIMER_MODE_SINGLE_SHOT,
                            update_timeout_handler);
}


uint32_t ble_conn_params_stop(void)
{
    return app_timer_stop(m_conn_params_timer_id);
}


static void conn_params_negotiation(void)
{
    // Start negotiation if the received connection parameters are not acceptable
    if (!is_conn_params_ok(&m_current_conn_params))
    {
        uint32_t err_code;
        uint32_t timeout_ticks;

        if (m_change_param)
        {
            // Notify the application that the procedure has failed
            if (m_conn_params_config.evt_handler != NULL)
            {
                ble_conn_params_evt_t evt;

                evt.evt_type = BLE_CONN_PARAMS_EVT_FAILED;
                m_conn_params_config.evt_handler(&evt);
            }
        }
        else
        {
            if (m_update_count == 0)
            {
                // First connection parameter update
                timeout_ticks = m_conn_params_config.first_conn_params_update_delay;
            }
            else
            {
                timeout_ticks = m_conn_params_config.next_conn_params_update_delay;
            }

            err_code = app_timer_start(m_conn_params_timer_id, timeout_ticks, NULL);
            if ((err_code != NRF_SUCCESS) && (m_conn_params_config.error_handler != NULL))
            {
                m_conn_params_config.error_handler(err_code);
            }
        }
    }
    else
    {
        // Notify the application that the procedure has succeded
        if (m_conn_params_config.evt_handler != NULL)
        {
            ble_conn_params_evt_t evt;

            evt.evt_type = BLE_CONN_PARAMS_EVT_SUCCEEDED;
            m_conn_params_config.evt_handler(&evt);
        }
    }
    m_change_param = false;
}


static void on_connect(ble_evt_t * p_ble_evt)
{
    // Save connection parameters
    m_conn_handle         = p_ble_evt->evt.gap_evt.conn_handle;
    m_current_conn_params = p_ble_evt->evt.gap_evt.params.connected.conn_params;
    m_update_count        = 0;  // Connection parameter negotiation should re-start every connection
	
		andorid_update_times  = 0;
		ios_update_times  = 0;
	  m_preferred_conn_params = m_on_conn_params;
	 /*we take the first connnection as the fast params*/
	  is_default_params_on_first_conn = true;
	   
		Y_SPRINTF("[BLE CONN]-- on_connect Current (on CONN) Max: %d, Min: %d", 
				m_current_conn_params.max_conn_interval, m_current_conn_params.min_conn_interval);

    // Check if we shall handle negotiation on connect
    if (m_conn_params_config.start_on_notify_cccd_handle == BLE_GATT_HANDLE_INVALID)
    {
        conn_params_negotiation();
    }
}


static void on_disconnect(ble_evt_t * p_ble_evt)
{
    uint32_t err_code;

    m_conn_handle = BLE_CONN_HANDLE_INVALID;

    // Stop timer if running
    m_update_count = 0; // Connection parameters updates should happen during every connection

		andorid_update_times = 0;
		ios_update_times = 0;
    err_code = app_timer_stop(m_conn_params_timer_id);
    if ((err_code != NRF_SUCCESS) && (m_conn_params_config.error_handler != NULL))
    {
        m_conn_params_config.error_handler(err_code);
    }
}


static void on_write(ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    // Check if this the correct CCCD
    if (
        (p_evt_write->handle == m_conn_params_config.start_on_notify_cccd_handle)
        &&
        (p_evt_write->len == 2)
       )
    {
        // Check if this is a 'start notification'
        if (ble_srv_is_notification_enabled(p_evt_write->data))
        {
            // Do connection parameter negotiation if necessary
            conn_params_negotiation();
        }
        else
        {
            uint32_t err_code;

            // Stop timer if running
            err_code = app_timer_stop(m_conn_params_timer_id);
            if ((err_code != NRF_SUCCESS) && (m_conn_params_config.error_handler != NULL))
            {
                m_conn_params_config.error_handler(err_code);
            }
        }
    }
}


static void on_conn_params_update(ble_evt_t * p_ble_evt)
{
    // Copy the parameters
    m_current_conn_params = p_ble_evt->evt.gap_evt.params.conn_param_update.conn_params;
	
		Y_SPRINTF("[BLE CONN]-- Current (on UPDATE) Max: %d, Min: %d", 
				m_current_conn_params.max_conn_interval, m_current_conn_params.min_conn_interval);

    conn_params_negotiation();
}

void ble_conn_params_on_ble_evt(ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_ble_evt);
            break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE:
            on_conn_params_update(p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}

/*****************************************************************************
 * Function      : ble_conn_params_change_conn_params
 * Description   : general api to change connection parameter 
 * Input         : None
 * Output        : None
 * Return        : int -1:
 * Others        :
 * Record				:
 * 1.Date        : 20151019
 *   Author      : MikeWang
 *   Modification: Created function

*****************************************************************************/
uint32_t ble_conn_params_change_conn_params(ble_gap_conn_params_t * new_params)
{  uint32_t err_code;
#define ANDOIRD_CONN_PARAMS_TIMES 2
    m_preferred_conn_params = *new_params;
    // Set the connection params in stack
    err_code = sd_ble_gap_ppcp_set(&m_preferred_conn_params);
    if (err_code == NRF_SUCCESS) {
        if (!is_conn_params_ok(&m_current_conn_params)) {
            m_update_count ++;
            if( device_type == CONN_PARAMS_MGR_DEVICE_IOS) {
								ios_update_times++;
								m_change_param = true;
                err_code = sd_ble_gap_conn_param_update(m_conn_handle, &m_preferred_conn_params);
                m_update_count = 0;
							  //err_code = app_timer_start(m_conn_params_timer_id, APP_TIMER_TICKS(1*1000, 0), NULL);
            } else {
							  andorid_update_times++;
							 /*for andoird can only update parameter for once*/
								if(m_update_count < ANDOIRD_CONN_PARAMS_TIMES){
									 err_code = sd_ble_gap_conn_param_update(m_conn_handle, &m_preferred_conn_params);
								}else{
									m_update_count = 0;
									conn_mgr_disconnect_for_fast_connection(&m_preferred_conn_params);
								}
               
            }

        } else {
            // Notify the application that the procedure has succeded
            if (m_conn_params_config.evt_handler != NULL) {
                ble_conn_params_evt_t evt;

                evt.evt_type = BLE_CONN_PARAMS_EVT_SUCCEEDED;
                m_conn_params_config.evt_handler(&evt);
            }
            err_code = NRF_SUCCESS;
        }
				Y_SPRINTF("[BLE CONN]-- IOS update times = %d, andorid update times = %d",ios_update_times, andorid_update_times);

    }
    return err_code;
}


bool conn_params_mgr_set_device_type(uint16_t dev_type)
{
    device_type = dev_type;
    return true;
}

/*****************************************************************************
 * Function      : conn_mgr_disconnect_for_fast_connection
 * Description   : set connection parameter to a higher one if the device is an andoird one
 * Input          : None
 * Output        : None
 * Return        : int -1:
 * Others        :
 * Record				:
 * 1.Date        : 20151019
 *   Author      : MikeWang
 *   Modification: Created function

*****************************************************************************/
static int conn_mgr_disconnect_for_fast_connection( ble_gap_conn_params_t  *conn_params)
{

    uint32_t err_code = sd_ble_gap_ppcp_set(conn_params);
    err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION); //BLE_HCI_STATUS_CODE_SUCCESS);

    if (err_code != NRF_SUCCESS) {
        return err_code;
    }
    return true;
}

bool ble_conn_params_com_conn_params(ble_gap_conn_params_t new_params)
{
    ble_gap_conn_params_t old_params;
    sd_ble_gap_ppcp_get(&old_params);
    if(is_conn_params_ok(&m_current_conn_params)) {
    if((new_params.max_conn_interval == old_params.max_conn_interval) && (new_params.min_conn_interval == old_params.min_conn_interval)) {
          N_SPRINTF("[CONN]still the sem max_conn_interval = %d,  min_conn_interval = %d", old_params.max_conn_interval, old_params.min_conn_interval); 
					return true;
						
        }
    }
		/*this means  the first  connection happened and we take it as the fast one, for some andoird devices whose ble stack is nor in accordance 
		with standart ble prototol, a parameter out of the range of fast parameter ranges gonna appear, we need to fake the current connection parameters to 
		cheat the ble stack*/
		if(is_default_params_on_first_conn == true){
				/*used to take in ble stack */
				m_current_conn_params = m_on_conn_params;
			  is_default_params_on_first_conn = false;
			  return true;
		}
		// Y_SPRINTF("[CONN]diffrent max_conn_interval = %d,  min_conn_interval = %d", old_params.max_conn_interval, old_params.min_conn_interval); 
    return false;
}
ble_gap_conn_params_t get_current_conn_params()
{
    return m_current_conn_params;
}
