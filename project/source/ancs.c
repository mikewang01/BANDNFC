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
 
/* Disclaimer: This client implementation of the Apple Notification Center Service can and will be changed at any time by Nordic Semiconductor ASA.
 * Server implementations such as the ones found in iOS can be changed at any time by Apple and may cause this client implementation to stop working.
 */

#include "main.h"
#include "btle.h"
#include "pstorage.h"
#include "ancs.h"

#include "ble_ancs_c.h"
#include <string.h>
#include <stdbool.h>
#include "ble_err.h"
#include "ble_srv_common.h"
#include "nordic_common.h"
#include "nrf_assert.h"
#include "device_manager.h"
#include "ble_db_discovery.h"
#include "app_error.h"
#include "oled.h"

#ifdef _ENABLE_ANCS_

#define BLE_ANCS_NOTIF_EVT_ID_INDEX       0                       /**< Index of the Event ID field when parsing notifications. */
#define BLE_ANCS_NOTIF_FLAGS_INDEX        1                       /**< Index of the Flags field when parsing notifications. */
#define BLE_ANCS_NOTIF_CATEGORY_ID_INDEX  2                       /**< Index of the Category ID field when parsing notifications. */
#define BLE_ANCS_NOTIF_CATEGORY_CNT_INDEX 3                       /**< Index of the Category Count field when parsing notifications. */
#define BLE_ANCS_NOTIF_NOTIF_UID          4                       /**< Index of the Notification UID field when patsin notifications. */

#define START_HANDLE_DISCOVER            0x0001                   /**< Value of start handle during discovery. */

#define TX_BUFFER_MASK                   0x07                     /**< TX buffer mask. Must be a mask of contiguous zeroes followed by a contiguous sequence of ones: 000...111. */
#define TX_BUFFER_SIZE                   (TX_BUFFER_MASK + 1)     /**< Size of send buffer, which is 1 higher than the mask. */
#define WRITE_MESSAGE_LENGTH             13                       /**< Length of the write message for CCCD/control point. */
#define BLE_CCCD_NOTIFY_BIT_MASK         0x0001                   /**< Enable notification bit. */


/**@brief ANCS request types.
 */
typedef enum
{
    READ_REQ = 1,  /**< Type identifying that this tx_message is a read request. */
    WRITE_REQ      /**< Type identifying that this tx_message is a write request. */
} ancs_tx_request_t;


/**@brief Structure used for holding the characteristic found during the discovery process.
 */
typedef struct
{
    ble_uuid_t            uuid;          /**< UUID identifying the characteristic. */
    ble_gatt_char_props_t properties;    /**< Properties for the characteristic. */
    uint16_t              handle_decl;   /**< Characteristic Declaration Handle for the characteristic. */
    uint16_t              handle_value;  /**< Value Handle for the value provided in the characteristic. */
    uint16_t              handle_cccd;   /**< CCCD Handle value for the characteristic. */
} ble_ancs_c_characteristic_t;


/**@brief Structure used for holding the Apple Notification Center Service found during the discovery process.
 */
typedef struct
{
    uint8_t                     handle;         /**< Handle of Apple Notification Center Service, which identifies to which peer this discovered service belongs. */
    ble_gattc_service_t         service;        /**< The GATT Service holding the discovered Apple Notification Center Service. */
    ble_ancs_c_characteristic_t control_point;  /**< Control Point Characteristic for the service. Allows interaction with the peer. */
    ble_ancs_c_characteristic_t notif_source;   /**< Characteristic that keeps track of arrival, modification, and removal of notifications. */
    ble_ancs_c_characteristic_t data_source;    /**< Characteristic where attribute data for the notifications is received from peer. */
} ble_ancs_c_service_t;


/**@brief Structure for writing a message to the central, i.e. Control Point or CCCD.
 */
typedef struct
{
    uint8_t                  gattc_value[WRITE_MESSAGE_LENGTH]; /**< The message to write. */
    ble_gattc_write_params_t gattc_params;                      /**< GATTC parameters for this message. */
} write_params_t;


/**@brief Structure for holding data to be transmitted to the connected master.
 */
typedef struct
{
    uint16_t          conn_handle;  /**< Connection handle to be used when transmitting this message. */
    ancs_tx_request_t type;         /**< Type of this message, i.e. read or write message. */
    union
    {
        uint16_t       read_handle; /**< Read request message. */
        write_params_t write_req;   /**< Write request message. */
    } req;
} tx_message_t;



static tx_message_t   m_tx_buffer[2]; //m_tx_buffer[TX_BUFFER_SIZE];       /**< Transmit buffer for messages to be transmitted to the Notification Provider. */
static uint32_t       m_tx_insert_index = 0;                               /**< Current index in the transmit buffer where the next message should be inserted. */
static uint32_t       m_tx_index        = 0;                               /**< Current index in the transmit buffer from where the next message to be transmitted resides. */

static ble_ancs_c_service_t   m_service;                                   /**< Current service data. */
static ble_ancs_c_t         * mp_ble_ancs;                                 /**< Pointer to the current instance of the ANCS client module. The memory for this is provided by the application.*/
static ble_ancs_c_evt_t       m_ancs_evt;                                  /**< The ANCS event that is created in this module and propagated to the application. */
static uint8_t                m_ancs_uuid_type;                            /**< Store ANCS UUID. */

static ble_ancs_c_evt_notif_t ancs_notif;
static I16U      notif_handle_value;                                        
static I16U      data_source_handle_value;                                      

typedef enum
{
    COMMAND_ID,
    NOTIFICATION_UID1,
    NOTIFICATION_UID2,
    NOTIFICATION_UID3,
    NOTIFICATION_UID4,
    ATTRIBUTE_ID,
    ATTRIBUTE_LEN1,
    ATTRIBUTE_LEN2,
    ATTRIBUTE_READY
} parse_state_t;

static parse_state_t         m_parse_state = COMMAND_ID;                                   /**< ANCS notification attribute parsing state. */
static I8U                   ATTR_title_or_message_index=0; 


/**@brief 128-bit service UUID for the Apple Notification Center Service.
 */
const ble_uuid128_t ble_ancs_base_uuid128 =
{
  {
		// 7905F431-B5CE-4E99-A40F-4B1E122D00D0
		0xd0, 0x00, 0x2d, 0x12, 0x1e, 0x4b, 0x0f, 0xa4,
		0x99, 0x4e, 0xce, 0xb5, 0x31, 0xf4, 0x05, 0x79
  }
};


/**@brief 128-bit control point UUID.
 */
const ble_uuid128_t ble_ancs_cp_base_uuid128 =
{
	{
		// 69d1d8f3-45e1-49a8-9821-9BBDFDAAD9D9
		0xd9, 0xd9, 0xaa, 0xfd, 0xbd, 0x9b, 0x21, 0x98,
		0xa8, 0x49, 0xe1, 0x45, 0xf3, 0xd8, 0xd1, 0x69
	}
};

/**@brief 128-bit notification source UUID.
*/
const ble_uuid128_t ble_ancs_ns_base_uuid128 =
{
	{
		// 9FBF120D-6301-42D9-8C58-25E699A21DBD
		0xbd, 0x1d, 0xa2, 0x99, 0xe6, 0x25, 0x58, 0x8c,
		0xd9, 0x42, 0x01, 0x63, 0x0d, 0x12, 0xbf, 0x9f

	}
};

/**@brief 128-bit data source UUID.
*/
const ble_uuid128_t ble_ancs_ds_base_uuid128 =
{
	{
		// 22EAC6E9-24D6-4BB5-BE44-B36ACE7C7BFB
		0xfb, 0x7b, 0x7c, 0xce, 0x6a, 0xb3, 0x44, 0xbe,
		0xb5, 0x4b, 0xd6, 0x24, 0xe9, 0xc6, 0xea, 0x22
	}
};


/**@brief Function for handling events from the database discovery module.*/
static void db_discover_evt_handler(ble_db_discovery_evt_t * p_evt)
{
	Y_SPRINTF("[ANCS]: Database Discovery handler called with event 0x%x\r\n", p_evt->evt_type);

	ble_ancs_c_evt_t evt;
	ble_db_discovery_char_t * p_chars;

	p_chars = p_evt->params.discovered_db.charateristics;

	// Check if the ANCS Service was discovered.
	if (p_evt->evt_type == BLE_DB_DISCOVERY_COMPLETE &&
			p_evt->params.discovered_db.srv_uuid.uuid == ANCS_UUID_SERVICE &&
			p_evt->params.discovered_db.srv_uuid.type == BLE_UUID_TYPE_VENDOR_BEGIN)
  {
			mp_ble_ancs->conn_handle = p_evt->conn_handle;

			// Find the handles of the ANCS characteristic.
			uint32_t i;

			for (i = 0; i < p_evt->params.discovered_db.char_count; i++)
			{
					switch (p_evt->params.discovered_db.charateristics[i].characteristic.uuid.uuid)
					{
							case ANCS_UUID_CHAR_CONTROL_POINT:
									Y_SPRINTF("[ANCS]: Control Point Characteristic found.\n\r");
									m_service.control_point.properties   = p_chars[i].characteristic.char_props;
									m_service.control_point.handle_decl  = p_chars[i].characteristic.handle_decl;
									m_service.control_point.handle_value = p_chars[i].characteristic.handle_value;
							    m_service.control_point.handle_cccd  = p_chars[i].cccd_handle;
									break;

							case ANCS_UUID_CHAR_DATA_SOURCE:
									Y_SPRINTF("[ANCS]: Data Source Characteristic found.\n\r");
									m_service.data_source.properties   = p_chars[i].characteristic.char_props;
									m_service.data_source.handle_decl  = p_chars[i].characteristic.handle_decl;
									m_service.data_source.handle_value = p_chars[i].characteristic.handle_value;
									data_source_handle_value=p_chars[i].characteristic.handle_value;
							    m_service.data_source.handle_cccd  = p_chars[i].cccd_handle;
									break;

							case ANCS_UUID_CHAR_NOTIFICATION_SOURCE:
									Y_SPRINTF("[ANCS]: Notification point Characteristic found.\n\r");
									m_service.notif_source.properties   = p_chars[i].characteristic.char_props;
									m_service.notif_source.handle_decl  = p_chars[i].characteristic.handle_decl;
									m_service.notif_source.handle_value = p_chars[i].characteristic.handle_value;
									notif_handle_value=p_chars[i].characteristic.handle_value;								
							    m_service.notif_source.handle_cccd  = p_chars[i].cccd_handle;
									break;

									default:
									break;
					}
			}
			evt.evt_type = BLE_ANCS_C_EVT_DISCOVER_COMPLETE;
			mp_ble_ancs->evt_handler(&evt);
	}
	else
	{
			evt.evt_type = BLE_ANCS_C_EVT_DISCOVER_FAILED;
			mp_ble_ancs->evt_handler(&evt);
	}
}


/**@brief Function for passing any pending request from the buffer to the stack.
 */
static void tx_buffer_process(void)
{
	if (m_tx_index != m_tx_insert_index)
	{
			uint32_t err_code;

			if (m_tx_buffer[m_tx_index].type == READ_REQ) {
			
				err_code = sd_ble_gattc_read(m_tx_buffer[m_tx_index].conn_handle, m_tx_buffer[m_tx_index].req.read_handle, 0);
			}
			else {
			
				err_code = sd_ble_gattc_write(m_tx_buffer[m_tx_index].conn_handle,&m_tx_buffer[m_tx_index].req.write_req.gattc_params);
			}
			if (err_code == NRF_SUCCESS){
			
				++m_tx_index;
				m_tx_index &= TX_BUFFER_MASK;
			}
	}
}



static void _nflash_store_one_message(I8U *data)
{
	//use message 4k space from nflash
	I32U addr=SYSTEM_NOTIFICATION_SPACE_START;
	
	addr += (cling.ancs.message_total-1)*256;
	FLASH_Write_App(addr, data, 128);
	addr += 128;
	FLASH_Write_App(addr, data+128, 128);
	N_SPRINTF("[ANCS] ADDR :%d, %02x, %02x, %02x, %02x",addr, data[0], data[1], data[2], data[3]);		
}


/**@brief Function for parsing received notification attribute response data.
 */
static void parse_get_notif_attrs_response( const uint8_t *data, int len)
{
    static uint8_t *ptr;
    static uint16_t current_len;
    static ble_ancs_c_evt_t        evt;
    static uint16_t buff_idx;
    int i;
    //static uint16_t reference_len=32;
    for (i = 0; i < len; i++)
    {
        switch (m_parse_state)
        {
        case COMMAND_ID:
             //not store cmmand id
             m_parse_state = NOTIFICATION_UID1;
             break;

        case NOTIFICATION_UID1:
             evt.attr.notif_uid = data[i];
             m_parse_state = NOTIFICATION_UID2;
             break;

        case NOTIFICATION_UID2:
					   evt.attr.attr_len |= (data[i] << 8);
             m_parse_state = NOTIFICATION_UID3;
             break;

        case NOTIFICATION_UID3:
             evt.attr.attr_len |= (data[i] << 8);
             m_parse_state = NOTIFICATION_UID4;
             break;

        case NOTIFICATION_UID4:
             evt.attr.attr_len |= (data[i] << 8);
             m_parse_state = ATTRIBUTE_ID;
             break;

        case ATTRIBUTE_ID:
             evt.attr.attr_id = (ble_ancs_c_notif_attr_id_values_t)data[i];
             m_parse_state = ATTRIBUTE_LEN1;
             break;

        case ATTRIBUTE_LEN1:
					   evt.attr.attr_len=data[i];
             m_parse_state = ATTRIBUTE_LEN2;
             break;

        case ATTRIBUTE_LEN2:
             evt.attr.attr_len |= (data[i] << 8);
				     if(ATTR_title_or_message_index == 0){

             if((I8U)evt.attr.attr_len > 64){

							 Y_SPRINTF("[ANCS] get title len overstep the boundary and parse err");
						   return;
						 }							 
					   cling.ancs.pkt.title_len=(I8U)evt.attr.attr_len;
						 ptr = cling.ancs.pkt.buf;
	           Y_SPRINTF("[ANCS] get attr title len  :%d",evt.attr.attr_len );	
						}
						else{
							
               if((I8U)evt.attr.attr_len > 164){

							   Y_SPRINTF("[ANCS] get message len overstep the boundary and parse err");
						     return;
						   }							
						 cling.ancs.pkt.message_len=(I8U)evt.attr.attr_len;
						 ptr = &cling.ancs.pkt.buf[cling.ancs.pkt.title_len];	
             Y_SPRINTF("[ANCS] get attr message len  :%d",evt.attr.attr_len );							
						}
            m_parse_state = ATTRIBUTE_READY;

            buff_idx = 0;
            current_len = 0;							

            break;

        case ATTRIBUTE_READY:
					   ptr [buff_idx++] = data[i];
             current_len++;

             if (current_len == evt.attr.attr_len)
             {
                if(ATTR_title_or_message_index == 0){
								
									cling.ancs.pkt.buf[cling.ancs.pkt.title_len]=0;
									Y_SPRINTF("[ANCS] get attr title data :%s",(char *)cling.ancs.pkt.buf);	
								  ATTR_title_or_message_index=1;
									m_parse_state = ATTRIBUTE_ID;
								}
                else {
									
									cling.ancs.pkt.buf[cling.ancs.pkt.title_len+cling.ancs.pkt.message_len]=0;
								  ATTR_title_or_message_index=0;
			            Y_SPRINTF("[ANCS] get attr message data:%s",(char*)&cling.ancs.pkt.buf[cling.ancs.pkt.title_len]);		
								  m_parse_state = COMMAND_ID;
								  cling.ancs.ancs_attr_store_flag=TRUE;
									return;
								}
            }
            break;
				
    }
  }
}


/**@brief Function for checking if data in an iOS notification is out of bounds.*/

static uint32_t ble_ancs_verify_notification_format(const ble_ancs_c_evt_notif_t  notif)
{
    if(   (notif.evt_id >= BLE_ANCS_NB_OF_EVT_ID)
       || (notif.category_id >= BLE_ANCS_NB_OF_CATEGORY_ID))
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    return NRF_SUCCESS;
}



/**@brief Function for receiving and validating notifications received from the Notification Provider.*/

static void parse_notif(const ble_ancs_c_t * p_ancs,
                        ble_ancs_c_evt_t   * p_ancs_evt,
                        const uint8_t      * p_data_src,
                        const uint16_t       hvx_data_len)
{
    uint32_t err_code;
    if (hvx_data_len != BLE_ANCS_NOTIFICATION_DATA_LENGTH)
    {
        m_ancs_evt.evt_type = BLE_ANCS_C_EVT_INVALID_NOTIF;
        p_ancs->evt_handler(&m_ancs_evt);
    }

    /*lint --e{415} --e{416} -save suppress Warning 415: possible access out of bond */
    p_ancs_evt->notif.evt_id                    =
            (ble_ancs_c_evt_id_values_t) p_data_src[BLE_ANCS_NOTIF_EVT_ID_INDEX];

    p_ancs_evt->notif.evt_flags.silent          =
            (p_data_src[BLE_ANCS_NOTIF_FLAGS_INDEX] >> BLE_ANCS_EVENT_FLAG_SILENT) & 0x01;

    p_ancs_evt->notif.evt_flags.important       =
            (p_data_src[BLE_ANCS_NOTIF_FLAGS_INDEX] >> BLE_ANCS_EVENT_FLAG_IMPORTANT) & 0x01;

    p_ancs_evt->notif.evt_flags.pre_existing    =
            (p_data_src[BLE_ANCS_NOTIF_FLAGS_INDEX] >> BLE_ANCS_EVENT_FLAG_PREEXISTING) & 0x01;

    p_ancs_evt->notif.evt_flags.positive_action =
            (p_data_src[BLE_ANCS_NOTIF_FLAGS_INDEX] >> BLE_ANCS_EVENT_FLAG_POSITIVE_ACTION) & 0x01;

    p_ancs_evt->notif.evt_flags.negative_action =
            (p_data_src[BLE_ANCS_NOTIF_FLAGS_INDEX] >> BLE_ANCS_EVENT_FLAG_NEGATIVE_ACTION) & 0x01;

    p_ancs_evt->notif.category_id               =
        (ble_ancs_c_category_id_values_t) p_data_src[BLE_ANCS_NOTIF_CATEGORY_ID_INDEX];

    p_ancs_evt->notif.category_count            = p_data_src[BLE_ANCS_NOTIF_CATEGORY_CNT_INDEX];
    p_ancs_evt->notif.notif_uid = uint32_decode(&p_data_src[BLE_ANCS_NOTIF_NOTIF_UID]);
    /*lint -restore*/

    err_code = ble_ancs_verify_notification_format(m_ancs_evt.notif);
    if (err_code == NRF_SUCCESS)
    {
        m_ancs_evt.evt_type = BLE_ANCS_C_EVT_NOTIF;
    }
    else
    {
        m_ancs_evt.evt_type = BLE_ANCS_C_EVT_INVALID_NOTIF;
    }

    p_ancs->evt_handler(&m_ancs_evt);
}


/**@brief Function for receiving and validating notifications received from the Notification Provider.
 * 
 * @param[in] p_ancs    Pointer to an ANCS instance to which the event belongs.
 * @param[in] p_ble_evt Bluetooth stack event.
 */
static void on_evt_gattc_notif(ble_ancs_c_t * p_ancs, const ble_evt_t * p_ble_evt)
{
    const ble_gattc_evt_hvx_t * p_notif = &p_ble_evt->evt.gattc_evt.params.hvx;

    if (p_notif->handle == notif_handle_value)
		{
			  N_SPRINTF("[ANCS] 11 get notif source len is :%d",p_notif->len);
        ATTR_title_or_message_index=0;
			  BLE_UUID_COPY_INST(m_ancs_evt.uuid, m_service.notif_source.uuid);
        parse_notif(p_ancs, &m_ancs_evt,p_notif->data,p_notif->len);
    }

    else if (p_notif->handle == data_source_handle_value)
		{
			  N_SPRINTF("[ANCS] 11 get data source len is :%d",p_notif->len);
	      BLE_UUID_COPY_INST(m_ancs_evt.uuid, m_service.data_source.uuid);
        parse_get_notif_attrs_response(p_notif->data, p_notif->len);
    }
    else
    {
        // No applicable action.
    }
}

/**@brief Function for handling write response events.
 */
static void on_evt_write_rsp()
{
    tx_buffer_process();
}


void ble_ancs_c_on_device_manager_evt(ble_ancs_c_t      * p_ans,
                                      dm_handle_t const * p_handle,
                                      dm_event_t const  * p_dm_evt)
{
    switch (p_dm_evt->event_id)
    {
        case DM_EVT_CONNECTION:
            // Fall through.
        case DM_EVT_SECURITY_SETUP_COMPLETE:
            p_ans->central_handle = p_handle->device_id;
            break;

        default:
            // Do nothing.
            break;
    }
}


void ble_ancs_c_on_ble_evt(ble_ancs_c_t * p_ancs, const ble_evt_t * p_ble_evt)
{
    uint16_t evt = p_ble_evt->header.evt_id;

    switch (evt)
    {
        case BLE_GATTC_EVT_WRITE_RSP:
            on_evt_write_rsp();
            break;

        case BLE_GATTC_EVT_HVX:
           on_evt_gattc_notif(p_ancs, p_ble_evt);
            break;

        default:
            break;
    }
}


uint32_t ble_ancs_c_init(ble_ancs_c_t * p_ancs, const ble_ancs_c_init_t * p_ancs_init)
{
    if ((p_ancs == NULL) || p_ancs_init == NULL || (p_ancs_init->evt_handler == NULL))
    {
        return NRF_ERROR_NULL;
    }

    mp_ble_ancs = p_ancs;

    mp_ble_ancs->evt_handler    = p_ancs_init->evt_handler;
    mp_ble_ancs->error_handler  = p_ancs_init->error_handler;
    mp_ble_ancs->service_handle = BLE_GATT_HANDLE_INVALID;
    mp_ble_ancs->central_handle = DM_INVALID_ID;
    mp_ble_ancs->conn_handle    = BLE_CONN_HANDLE_INVALID;

    memset(&m_service, 0, sizeof(ble_ancs_c_service_t));
    memset(m_tx_buffer, 0, TX_BUFFER_SIZE);

    m_service.handle = BLE_GATT_HANDLE_INVALID;

    ble_uuid_t ancs_uuid;
    ancs_uuid.uuid = ANCS_UUID_SERVICE;
    ancs_uuid.type = BLE_UUID_TYPE_VENDOR_BEGIN;//BLE_UUID_TYPE_BLE;

    return ble_db_discovery_evt_register(&ancs_uuid, db_discover_evt_handler);
}


/**@brief Function for creating a TX message for writing a CCCD.*/

static uint32_t cccd_configure(const uint16_t conn_handle, const uint16_t handle_cccd, bool enable)
{
    tx_message_t * p_msg;
    uint16_t       cccd_val = enable ? BLE_CCCD_NOTIFY_BIT_MASK : 0;

    p_msg              = &m_tx_buffer[m_tx_insert_index++];
    m_tx_insert_index &= TX_BUFFER_MASK;

    p_msg->req.write_req.gattc_params.handle   = handle_cccd;
    p_msg->req.write_req.gattc_params.len      = 2;
    p_msg->req.write_req.gattc_params.p_value  = p_msg->req.write_req.gattc_value;
    p_msg->req.write_req.gattc_params.offset   = 0;
    p_msg->req.write_req.gattc_params.write_op = BLE_GATT_OP_WRITE_REQ;
    p_msg->req.write_req.gattc_value[0]        = LSB(cccd_val);
    p_msg->req.write_req.gattc_value[1]        = MSB(cccd_val);
    p_msg->conn_handle                         = conn_handle;
    p_msg->type                                = WRITE_REQ;

    tx_buffer_process();
    return NRF_SUCCESS;
}


uint32_t ble_ancs_c_notif_source_notif_enable(void)
{
    Y_SPRINTF("[ANCS]: Enable Notification Source notifications. writing to handle: %i \n\r",
        m_service.notif_source.handle_cccd);
    return cccd_configure(cling.ble.conn_handle, m_service.notif_source.handle_cccd, true);
}


uint32_t ble_ancs_c_notif_source_notif_disable(void)
{
    return cccd_configure(cling.ble.conn_handle, m_service.notif_source.handle_cccd, false);
}


uint32_t ble_ancs_c_data_source_notif_enable(void)
{
    Y_SPRINTF("[ANCS]: Enable Data Source notifications. Writing to handle: %i \n\r",
        m_service.data_source.handle_cccd);
    return cccd_configure(cling.ble.conn_handle, m_service.data_source.handle_cccd, true);
}


uint32_t ble_ancs_c_data_source_notif_disable(void)
{
    return cccd_configure(cling.ble.conn_handle, m_service.data_source.handle_cccd, false);
}


uint32_t ble_ancs_get_notif_attrs( const uint32_t       p_uid)
{
	  I16U len=0;
    tx_message_t * p_msg;
    uint32_t       index                    = 0;
 
    p_msg              = &m_tx_buffer[m_tx_insert_index++];
    m_tx_insert_index &= TX_BUFFER_MASK;

    p_msg->req.write_req.gattc_params.handle   = m_service.control_point.handle_value;
    p_msg->req.write_req.gattc_params.p_value  = p_msg->req.write_req.gattc_value;
    p_msg->req.write_req.gattc_params.offset   = 0;
    p_msg->req.write_req.gattc_params.write_op = BLE_GATT_OP_WRITE_REQ;

    //Encode Command ID.
    p_msg->req.write_req.gattc_value[index++] = BLE_ANCS_COMMAND_ID_GET_NOTIF_ATTRIBUTES;
    
    //Encode Notification UID.
    index += uint32_encode(p_uid, &p_msg->req.write_req.gattc_value[index]);
	
    //Encode Attribute ID.
	  p_msg->req.write_req.gattc_value[index++] = BLE_ANCS_NOTIF_ATTR_ID_TITLE;
		
		//MAX title len
		len=64;
	  p_msg->req.write_req.gattc_value[index++] = (uint8_t) (len);
    p_msg->req.write_req.gattc_value[index++] = (uint8_t) (len >> 8);

    //Encode Attribute ID.
	  p_msg->req.write_req.gattc_value[index++] = BLE_ANCS_NOTIF_ATTR_ID_MESSAGE;
	 
	  //MAX message len
	  len=192;
	  p_msg->req.write_req.gattc_value[index++] = (uint8_t) (len);
    p_msg->req.write_req.gattc_value[index++] = (uint8_t) (len >> 8); 
	 
    p_msg->req.write_req.gattc_params.len = index;
    p_msg->conn_handle                    = cling.ble.conn_handle;
    p_msg->type                           = WRITE_REQ;

    tx_buffer_process();

    return NRF_SUCCESS;
}



uint32_t ble_ancs_c_request_attrs(const ble_ancs_c_evt_notif_t  notif)
{

    uint32_t err_code;
    err_code = ble_ancs_verify_notification_format(notif);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code      = ble_ancs_get_notif_attrs( notif.notif_uid);
  
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    return NRF_SUCCESS;
}


/**@brief Function for setting up GATTC notifications from the Notification Provider.
 *
 * @details This function is called when a successful connection has been established.
 */
static void apple_notification_setup(void)
{
    uint32_t err_code;

    nrf_delay_ms(100); // Delay because we cannot add a CCCD to close to starting encryption. iOS specific.

    err_code = ble_ancs_c_notif_source_notif_enable();	
    APP_ERROR_CHECK(err_code);

    err_code = ble_ancs_c_data_source_notif_enable();
    APP_ERROR_CHECK(err_code);
	
}


/**@brief Function for printing an iOS notification.
 *
 * @param[in] p_notif  Pointer to the iOS notification.
 */
static void notif_print(ble_ancs_c_evt_notif_t * p_notif)
{
	  Y_SPRINTF("[ANCS] Event:       %d", p_notif->evt_id);
    Y_SPRINTF("[ANCS] Category ID: %d", p_notif->category_id);
    Y_SPRINTF("[ANCS] Category Cnt:%u", (unsigned int) p_notif->category_count);
    Y_SPRINTF("[ANCS] UID:         %u", (unsigned int) p_notif->notif_uid);
}
						 



/**@brief Function for handling the Apple Notification Service client.
 *
 * @details This function is called for all events in the Apple Notification client that
 *          are passed to the application.
 *
 * @param[in] p_evt  Event received from the Apple Notification Service client.
 */
static void on_ancs_c_evt(ble_ancs_c_evt_t * p_evt)
{
    //uint32_t err_code = NRF_SUCCESS;
	
	  I32U  timer;
	  I32U  t_curr, t_diff;		

    switch (p_evt->evt_type)
    {
        case BLE_ANCS_C_EVT_DISCOVER_COMPLETE:
				    {
              Y_SPRINTF("[ANCS]Apple Notification Service discovered on the server.\n");
              apple_notification_setup();
							ATTR_title_or_message_index=0;
						  timer = CLK_get_system_time();
             						
						}
            break;

        case BLE_ANCS_C_EVT_NOTIF:
				    {			
							#ifdef _ENABLE_UART_
							notif_print(&p_evt->notif);	
              #endif	
		          t_curr = CLK_get_system_time();
		          t_diff = t_curr - timer;
							
		          if (t_diff>8000) {
								
								if(p_evt->notif.evt_id == BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED){
										
								ancs_notif=p_evt->notif;
									
							  cling.ancs.ancs_attr_get_flag=TRUE;
             															
								}
							}												
				    }
            break;

        case BLE_ANCS_C_EVT_DISCOVER_FAILED:
            // ANCS not found.
				    Y_SPRINTF("[ANCS] Apple Notification Service not discovered on the server...");
			
				    if(BTLE_is_connected()){
							
							BTLE_disconnect(BTLE_DISCONN_REASON_ANCS_DISC_FAIL);	
						}
         
            break;

        default:
            // No implementation needed.
            break;
    }
}

static void apple_notification_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


void ANCS_service_add(void)
{
    ble_ancs_c_init_t ancs_init_obj;
    ble_uuid_t        service_uuid;
    uint32_t          err_code;

    err_code = sd_ble_uuid_vs_add(&ble_ancs_base_uuid128, &m_ancs_uuid_type);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_uuid_vs_add(&ble_ancs_cp_base_uuid128, &service_uuid.type);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_uuid_vs_add(&ble_ancs_ns_base_uuid128, &service_uuid.type);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_uuid_vs_add(&ble_ancs_ds_base_uuid128, &service_uuid.type);
    APP_ERROR_CHECK(err_code);

    memset(&ancs_init_obj, 0, sizeof(ancs_init_obj));

    ancs_init_obj.evt_handler   = on_ancs_c_evt;
    ancs_init_obj.error_handler = apple_notification_error_handler;

    err_code = ble_ancs_c_init(&cling.ancs.m_ancs_c, &ancs_init_obj);
    APP_ERROR_CHECK(err_code);
}

void _ancs_get_attr_pro(void)
{
	I32U err_code;
	Y_SPRINTF("[ANCS] start get attr message  ... ");		
	
	err_code=ble_ancs_c_request_attrs(ancs_notif);
	if(err_code==NRF_SUCCESS)						
		Y_SPRINTF("[ANCS] start get attr message successed ... ");									

}

void _ancs_store_attr_pro(void)
{
	
		 Y_SPRINTF("[ANCS] start store attr message to nflash ... ");							
		 cling.ancs.notf_updata=TRUE;
			
		 // when a new notification message arrives, start notification state machine
		 NOTIFIC_start_notifying(ancs_notif.category_id);
								
		 if(cling.ancs.message_total >= 16) {
				
			 FLASH_erase_App(SYSTEM_NOTIFICATION_SPACE_START);
		 
			 Y_SPRINTF("[ANCS] message is full, go erase the message space");

			 cling.ancs.message_total = 0;
		 }
		 
		 cling.ancs.message_total++;		
		 Y_SPRINTF("[ANCS] message total is :%d ",cling.ancs.message_total);

		_nflash_store_one_message((I8U *)&cling.ancs.pkt);
		 	 	
}

void ANCS_state_machine(void)
{
	
	//I32U err_code;
	if(!BTLE_is_connected())
		return;

  if(cling.ancs.ancs_attr_get_flag == TRUE){
		
		_ancs_get_attr_pro();
		
   cling.ancs.ancs_attr_get_flag = FALSE;	
	}
	
	if(cling.ancs.ancs_attr_store_flag == TRUE){

    _ancs_store_attr_pro();	
	
		cling.ancs.ancs_attr_store_flag=FALSE;			
	}

}


#endif
