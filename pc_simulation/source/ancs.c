//
// File: ancs.c
//
// Description: Handle Apple notification center service.
//
#include "ble_ancs_c.h"
#include <string.h>
#include <stdbool.h>
#include "main.h"
#include "ble_err.h"
#include "nordic_common.h"
#include "nrf_assert.h"
#include "ble_flash.h"
#include "btle.h"
#include "pstorage.h"
#include "ancs.h"

#ifdef _ENABLE_ANCS_

#define START_HANDLE_DISCOVER            0x0001                                            /**< Value of start handle during discovery. */

#define NOTIFICATION_DATA_LENGTH         2                                                 /**< The mandatory length of notification data. After the mandatory data, the optional message is located. */

#define TX_BUFFER_MASK                   0x07                                              /**< TX Buffer mask, must be a mask of contiguous zeroes, followed by contiguous sequence of ones: 000...111. */
#define TX_BUFFER_SIZE                   (TX_BUFFER_MASK + 1)                              /**< Size of send buffer, which is 1 higher than the mask. */
#define WRITE_MESSAGE_LENGTH             20                                                /**< Length of the write message for CCCD/control point. */
#define BLE_CCCD_NOTIFY_BIT_MASK         0x0001                                            /**< Enable Notification bit. */


#define DISCOVERED_SERVICE_DB_SIZE \
    CEIL_DIV(sizeof(apple_service_t), sizeof(uint32_t))  /**< Size of bonded masters database in word size (4 byte). */

//#define ANCS_ATTRIBUTE_MESSAGE_DATA_MAX       96
typedef enum
{
    READ_REQ = 1,                                                                          /**< Type identifying that this tx_message is a read request. */
    WRITE_REQ                                                                              /**< Type identifying that this tx_message is a write request. */
} ancs_tx_request_t;

typedef enum
{
    STATE_UNINITIALIZED,                                                                   /**< Uninitialized state of the internal state machine. */
    STATE_IDLE,                                                                            /**< Idle state, this is the state when no master has connected to this device. */
    STATE_DISC_SERV,                                                                       /**< A BLE master is connected and a service discovery is in progress. */
    STATE_DISC_CHAR,                                                                       /**< A BLE master is connected and characteristic discovery is in progress. */
    STATE_DISC_DESC,                                                                       /**< A BLE master is connected and descriptor discovery is in progress. */
    STATE_RUNNING,                                                                         /**< A BLE master is connected and complete service discovery has been performed. */
    STATE_WAITING_ENC,                                                                     /**< A previously bonded BLE master has re-connected and the service awaits the setup of an encrypted link. */
    STATE_RUNNING_NOT_DISCOVERED,                                                          /**< A BLE master is connected and a service discovery is in progress. */
    STATE_RUNNING_ENABLE_DATA_SOURCE,
    STATE_STORE_SYS_ATTR,
    STATE_RUNNING_ENABLE_NOTIF_SOURCE,
} ancs_state_t;


/* brief Structure used for holding the characteristic found during discovery process.
 */
typedef struct
{
    ble_uuid_t               uuid;                                                         /**< UUID identifying this characteristic. */
    ble_gatt_char_props_t    properties;                                                   /**< Properties for this characteristic. */
    uint16_t                 handle_decl;                                                  /**< Characteristic Declaration Handle for this characteristic. */
    uint16_t                 handle_value;                                                 /**< Value Handle for the value provided in this characteristic. */
    uint16_t                 handle_cccd;                                                  /**< CCCD Handle value for this characteristic. BLE_ANCS_INVALID_HANDLE if not present in the master. */
} apple_characteristic_t;

/**@brief Structure used for holding the Apple Notification Center Service found during discovery process.
 */
typedef struct
{
    uint8_t                  handle;                                                       /**< Handle of Apple Notification Center Service which identifies to which master this discovered service belongs. */
    ble_gattc_service_t      service;                                                      /**< The GATT service holding the discovered Apple Notification Center Service. */
    apple_characteristic_t   control_point; 
    apple_characteristic_t   notification_source;
    apple_characteristic_t   data_source;
} apple_service_t;

/**@brief Structure for writing a message to the master, i.e. Control Point
 * or CCCD.
 */
typedef struct
{
    /* The message to write. */
    uint8_t gattc_value[WRITE_MESSAGE_LENGTH];
    /* GATTC parameters for this message. */
    ble_gattc_write_params_t gattc_params;
} write_params_t;


/**@brief Structure for writing a message to the master, i.e. Control Point or CCCD.
 */
typedef struct
{
	uint16_t						conn_handle;
	ancs_tx_request_t		type;
	union {
		uint16_t					read_handle;
		write_params_t		write_req;
	} req;
} tx_message_t;

/**@brief ANCS notification attribute parsing states.
 */
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

static ancs_state_t          m_client_state = STATE_UNINITIALIZED;                         /**< Current state of the Apple Notification Center State Machine. */

static apple_service_t       m_service;                                                    /**< Current service data. */

static parse_state_t         m_parse_state = COMMAND_ID;                                   /**< ANCS notification attribute parsing state. */

#define APPLE_NOTIFICATIONS_OFF 0xFF
#define APPLE_NOTIFICATIONS_ON 0x0F

I8U apple_notifications_enabled = APPLE_NOTIFICATIONS_ON;

const ble_uuid128_t ble_ancs_base_uuid128 =
{
   {
    // 7905F431-B5CE-4E99-A40F-4B1E122D00D0
    0xd0, 0x00, 0x2d, 0x12, 0x1e, 0x4b, 0x0f, 0xa4,
    0x99, 0x4e, 0xce, 0xb5, 0x31, 0xf4, 0x05, 0x79

   }
};

const ble_uuid128_t ble_ancs_cp_base_uuid128 =
{
   {
    // 69d1d8f3-45e1-49a8-9821-9bbdfdaad9d9
    0xd9, 0xd9, 0xaa, 0xfd, 0xbd, 0x9b, 0x21, 0x98,
    0xa8, 0x49, 0xe1, 0x45, 0xf3, 0xd8, 0xd1, 0x69

   }
};

const ble_uuid128_t ble_ancs_ns_base_uuid128 =
{
   {
    // 9FBF120D-6301-42D9-8C58-25E699A21DBD
    0xbd, 0x1d, 0xa2, 0x99, 0xe6, 0x25, 0x58, 0x8c,
    0xd9, 0x42, 0x01, 0x63, 0x0d, 0x12, 0xbf, 0x9f

   }
};

const ble_uuid128_t ble_ancs_ds_base_uuid128 =
{
   {
    // 22EAC6E9-24D6-4BB5-BE44-B36ACE7C7BFB
    0xfb, 0x7b, 0x7c, 0xce, 0x6a, 0xb3, 0x44, 0xbe,
    0xb5, 0x4b, 0xd6, 0x24, 0xe9, 0xc6, 0xea, 0x22

   }
};
	
#define MESSAGE_BUFFER_SIZE             18                                                   /**< Size of buffer holding optional messages in notifications. */

#define MAX_CHARACTERS_PER_LINE         16                                                   /**< Maximum characters that are visible in one line on nRF6350 display */
#define DISPLAY_BUFFER_SIZE             ANCS_ATTRIBUTE_DATA_MAX                              /**< Size of LCD display buffer */

#define INVALID_CENTRAL_HANDLE (-1)
static uint8_t                          m_ancs_uuid_type;
static pstorage_handle_t  m_flash_handle;
int8_t ancs_master_handle = INVALID_CENTRAL_HANDLE;

/* Security requirements for this application. */
ble_gap_sec_params_t m_ancs_sec_params = {
    /* Timeout for Pairing Request or Security Request (in seconds). */
//    .timeout = SEC_PARAM_TIMEOUT,
    /* Perform bonding. */
    .bond = SEC_PARAM_BOND,
    /* Man In The Middle protection not required. */
    .mitm = SEC_PARAM_MITM,
    /* No I/O capabilities. */
    .io_caps = BLE_GAP_IO_CAPS_NONE,
    /* Out Of Band data not available. */
    .oob = SEC_PARAM_OOB,
    /* Minimum encryption key size. */
    .min_key_size = SEC_PARAM_MIN_KEY_SIZE,
    /* Maximum encryption key size. */
    .max_key_size = SEC_PARAM_MAX_KEY_SIZE
};

#ifdef __DEBUG_BASE__

static const char *lit_catid[] =
{
    "Other",
    "IncomingCall",
    "MissedCall",
    "VoiceMail",
    "Social",
    "Schedule",
    "Email",
    "News",
    "HealthAndFitness",
    "BusinessAndFinance",
    "Location",
    "Entertainment"
};

static const char *lit_eventid[] =
{
    "Added",
    "Modified",
    "Removed"
};
#endif

#define ANCS_CONN_RETRY_MAX 5
static uint8_t ancs_conn_retry_count;

uint32_t ble_ancs_c_service_store(void) {
    uint32_t err_code;
    
    pstorage_handle_t block_handle;
    
    err_code = pstorage_block_identifier_get(&m_flash_handle, 0, &block_handle);
    if (err_code != NRF_SUCCESS) {
        N_SPRINTF("[ANCS] Error storing ANCS service data: %08x", err_code);
    }
    err_code = pstorage_store(&block_handle, (I8U *) &m_service, sizeof(apple_service_t)*4, 0);
    
    if (err_code != NRF_SUCCESS) {
        N_SPRINTF("[ANCS] Error storing ANCS service data: %08x", err_code);
    }
    return err_code;
}

static void ble_ancs_event_disconnect() {
    
    
    N_SPRINTF("[ANCS] disconnect!");
    I32U err_code;
    if (m_client_state == STATE_RUNNING) { // ONLY store valid ANCS data!
        // store ancs service info 
        err_code = ble_ancs_c_service_store();
        APP_ERROR_CHECK(err_code);
    }
    
    
    
    m_client_state = STATE_IDLE;
    if (m_service.handle == INVALID_SERVICE_HANDLE_DISC &&
        ancs_master_handle != INVALID_CENTRAL_HANDLE)
    {
        m_service.handle = ancs_master_handle;
    }
}
  
bool ANCS_is_listening_category(uint8_t category_id) 
{
	if (category_id >= ANCS_CATEGORY_ID_MAX) {
		return false;
	}

	return true;
}

void _got_new_ancs_notification(ble_ancs_c_evt_ios_notification_t * noti) {

    I8U *p_uid = noti->notification_uid;
		I16U id_mask;
	//#if 0
    I32U nid = (I32U) *p_uid;
    nid |= ((I32U) *(p_uid + 1)) << 8;
    nid |= ((I32U) *(p_uid + 2)) << 16;
    nid |= ((I32U) *(p_uid + 3)) << 24;
   //#endif
   	
		N_SPRINTF("[ANCS]  category id: %d, count: %d", noti->category_id, noti->category_count);

		if (noti->category_id >= ANCS_CATEGORY_ID_MAX) 
			return;
		
		if (noti->category_id == 0)
			return;

		id_mask = 1 << (noti->category_id-1);
		
    N_SPRINTF("[ANCS] adding - category %s, %02x, %02x", lit_catid[noti->category_id], id_mask, cling.ancs.supported_categories);
   	N_SPRINTF("[ANCS] adding - category: %s, count: %d", lit_catid[noti->category_id], noti->category_count);

    if (id_mask & cling.ancs.supported_categories) {
		
			cling.ancs.cat_count[noti->category_id] = noti->category_count;
			N_SPRINTF("[ANCS] start notification (%02x)", id_mask);

			// Inform NOTIFIC state machine to notify user
			NOTIFIC_start_notifying(noti->category_id);
		}
}

void _got_update_ancs_notification(ble_ancs_c_evt_ios_notification_t * new_notif) 
{

    I8U *p_uid = new_notif->notification_uid;
    I32U nid = (I32U) *p_uid;
    nid |= ((I32U) *(p_uid + 1)) << 8;
    nid |= ((I32U) *(p_uid + 2)) << 16;
    nid |= ((I32U) *(p_uid + 3)) << 24;

		// Notification updated so we should just stop vibrating ...
		NOTIFIC_stop_notifying();
}

I8U ANCS_get_next_new_notification(I8U index)
{
	int i = index;
	
	while ( i < ANCS_CATEGORY_ID_MAX) {
		if (cling.ancs.cat_count[i] > 0) {
			break;
		}
		i++;
	}
	
	return i;
}

BOOLEAN ANCS_notifications_to_read()
{
	int i;
	int count = 0;
	for (i = 0; i < ANCS_CATEGORY_ID_MAX; i++) {
		count += cling.ancs.cat_count[i];
	}
	
	if (count > 0) {
		// Should also initialize the index
		cling.ui.vertical_index = ANCS_get_next_new_notification(0);
		return TRUE;
	} else {
		return FALSE;
	}
}

void _got_delete_ancs_notification(ble_ancs_c_evt_ios_notification_t * noti) {

    I8U *p_uid = noti->notification_uid;
    I32U nid = (I32U) *p_uid;
    nid |= ((I32U) *(p_uid + 1)) << 8;
    nid |= ((I32U) *(p_uid + 2)) << 16;
    nid |= ((I32U) *(p_uid + 3)) << 24;

		// Notification remove so we should stop vibrating too.
		NOTIFIC_stop_notifying();
	
		if (cling.ancs.cat_count[noti->category_id] > 0) {
			cling.ancs.cat_count[noti->category_id] = 0;
    }
		
		N_SPRINTF("[ANCS] delete one notification at cat: %s", lit_catid[noti->category_id]);
}

/**@brief Copy notification attribute data to buffer
 *
 */
static void evt_notif_attribute(ble_ancs_c_evt_notif_attribute_t *p_attr)
{
	// Ignore the title and message at here ...
	#if 0
	char                             display_title[DISPLAY_BUFFER_SIZE];
	char                             display_message[DISPLAY_BUFFER_SIZE];
	
    if (p_attr->attribute_id == BLE_ANCS_NOTIFICATION_ATTRIBUTE_ID_TITLE)
    {
        memcpy(display_title, p_attr->data, p_attr->attribute_len);
    }
    else if (p_attr->attribute_id == BLE_ANCS_NOTIFICATION_ATTRIBUTE_ID_MESSAGE)
    {
        memcpy(display_message, p_attr->data, p_attr->attribute_len);
    }
		#endif
}

void _got_ancs_data(ble_ancs_c_evt_t *p_evt) 
{
    uint32_t err_code = NRF_SUCCESS;
    
    switch (p_evt->evt_type) {
        
        case BLE_ANCS_C_EVT_DISCOVER_COMPLETE:
				{
            N_SPRINTF("[ANCS] Got data - Doing GAP authentication:");
            err_code = sd_ble_gap_authenticate(cling.ble.conn_handle, &m_ancs_sec_params);
            APP_ERROR_CHECK(err_code);
            break;
				}
        case BLE_ANCS_C_EVT_IOS_NOTIFICATION:
				{
            switch(p_evt->data.notification.event_id) {
                case BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED: //added
                    // find an empty notification
										N_SPRINTF("[ANCS] Got data - new notification.");
                    _got_new_ancs_notification(&p_evt->data.notification);
                    break;
                case BLE_ANCS_EVENT_ID_NOTIFICATION_MODIFIED: //modified
										N_SPRINTF("[ANCS] Got data - modified notification.");
                    _got_update_ancs_notification(&p_evt->data.notification);
                    break;
                    
                case BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED: 
										N_SPRINTF("[ANCS] Got data - delete notification.");
                    _got_delete_ancs_notification(&p_evt->data.notification);
                    break;
                    
                default:
                    break;
            }
            break;
				}
        case BLE_ANCS_C_EVT_NOTIF_ATTRIBUTE:
				{
						evt_notif_attribute(&p_evt->data.attribute);
            break;
        }
        case BLE_ANCS_C_EVT_DISCOVER_FAILED:
				{
            N_SPRINTF("[ANCS] Discovery failed!!");
            break;
        }
        default:
            break;
    }
}
/**@brief Function for updating the current state and sending an event on discovery failure.
 */
static void handle_discovery_failure( uint32_t code)
{
    m_client_state        = STATE_RUNNING_NOT_DISCOVERED;
#if 0
    ble_ancs_c_evt_t event;

    event.evt_type        = BLE_ANCS_C_EVT_DISCOVER_FAILED;
    event.data.error_code = code;

    // No error handling
#endif
}


/**@brief Function for executing the Service Discovery Procedure.
 */
static void service_disc_req_send()
{
    uint16_t   handle = START_HANDLE_DISCOVER;
    ble_uuid_t ancs_uuid;
    uint32_t   err_code;

		N_SPRINTF("[ANCS] Executing the Service Discovery Procedure (packets sent out)");
    // Discover services on uuid for ANCS.
    BLE_UUID_BLE_ASSIGN(ancs_uuid, BLE_UUID_APPLE_NOTIFICATION_CENTER_SERVICE);
    ancs_uuid.type = BLE_UUID_TYPE_VENDOR_BEGIN;

    err_code = sd_ble_gattc_primary_services_discover(cling.ble.conn_handle, handle, &ancs_uuid);
    if (err_code != NRF_SUCCESS)
    {
        handle_discovery_failure( err_code);
    }
    else
    {
        m_client_state = STATE_DISC_SERV;
    }
}


/**@brief Function for executing the Characteristic Discovery Procedure.
 */
static void characteristic_disc_req_send(const ble_gattc_handle_range_t * p_handle)
{
    uint32_t err_code;

    // With valid service, we should discover characteristics.
    err_code = sd_ble_gattc_characteristics_discover(cling.ble.conn_handle, p_handle);

    if (err_code == NRF_SUCCESS)
    {
        m_client_state = STATE_DISC_CHAR;
    }
    else
    {
        handle_discovery_failure( err_code);
    }
}

/**@brief Function for setup of apple notifications in central.
 *
 * @details This function will be called when a successful connection has been established.
 */
void ANCS_apple_notification_setup(void)
{
    uint32_t err_code;
	
		if (cling.ble.conn_handle == BLE_CONN_HANDLE_INVALID)
			return;

    err_code = ble_ancs_c_enable_notif_notification_source(cling.ble.conn_handle);
    APP_ERROR_CHECK(err_code);

    err_code = ble_ancs_c_enable_notif_data_source(cling.ble.conn_handle);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for executing the Characteristic Descriptor Discovery Procedure.
 */
static void descriptor_disc_req_send()
{
    ble_gattc_handle_range_t descriptor_handle;
    uint32_t                 err_code = NRF_SUCCESS;

    // If we have not discovered descriptors for all characteristics,
    // we will discover next descriptor.
    if (m_service.notification_source.handle_cccd == BLE_ANCS_INVALID_HANDLE)
    {
        descriptor_handle.start_handle = m_service.notification_source.handle_value + 1;
        descriptor_handle.end_handle = m_service.notification_source.handle_value + 1;

        err_code = sd_ble_gattc_descriptors_discover(cling.ble.conn_handle, &descriptor_handle);
    }
    else if (m_service.data_source.handle_cccd == BLE_ANCS_INVALID_HANDLE)
    {
        descriptor_handle.start_handle = m_service.data_source.handle_value + 1;
        descriptor_handle.end_handle = m_service.data_source.handle_value + 1;

        err_code = sd_ble_gattc_descriptors_discover(cling.ble.conn_handle, &descriptor_handle);
    }


    if (err_code == NRF_SUCCESS)
    {
        m_client_state = STATE_DISC_DESC;
    }
    else
    {
        handle_discovery_failure( err_code);
    }
}


/**@brief Function for indicating that a connection has successfully been established. 
 *        Either when the Service Discovery Procedure completes or a re-connection has been 
 *        established to a bonded master.
 *
 * @details This function is executed when a service discovery or a re-connect to a bonded master
 *          occurs. In the event of re-connection to a bonded master, this function will ensure
 *          writing of the control point according to the Apple Notification Center Service Client
 *          specification.
 *          Finally an event is passed to the application:
 *          BLE_ANCS_C_EVT_RECONNECT         - When we are connected to an existing master and the
 *                                            apple notification control point has been written.
 *          BLE_ANCS_C_EVT_DISCOVER_COMPLETE - When we are connected to a new master and the Service
 *                                            Discovery has been completed.
 */
static void connection_established()
{
	ble_ancs_c_evt_t event;
	event.evt_type = BLE_ANCS_C_EVT_DISCOVER_COMPLETE;

	m_client_state = STATE_RUNNING_ENABLE_NOTIF_SOURCE;
	_got_ancs_data(&event);

	N_SPRINTF("[ANCS] +++ connection established");
}



/**@brief Function for setting the discovered characteristics in the apple service.
 */
static void characteristics_set(apple_characteristic_t * p_characteristic,
                                const ble_gattc_char_t * p_char_resp)
{
    BLE_UUID_COPY_INST(p_characteristic->uuid, p_char_resp->uuid);
    
    p_characteristic->properties   = p_char_resp->char_props;
    p_characteristic->handle_decl  = p_char_resp->handle_decl;
    p_characteristic->handle_value = p_char_resp->handle_value;
    p_characteristic->handle_cccd  = BLE_ANCS_INVALID_HANDLE;
}


static void ble_ancs_event_characteristic_rsp(const ble_evt_t *p_ble_evt) 
{
    if (p_ble_evt->evt.gattc_evt.gatt_status == BLE_GATT_STATUS_ATTERR_ATTRIBUTE_NOT_FOUND ||
        p_ble_evt->evt.gattc_evt.gatt_status == BLE_GATT_STATUS_ATTERR_INVALID_HANDLE)
    {
        if ((m_service.notification_source.handle_value == 0) ||
            (m_service.control_point.handle_value == 0) ||
            (m_service.data_source.handle_value == 0))
        {
            // did not find required characteristic
            handle_discovery_failure(NRF_ERROR_NOT_FOUND);
        } else {
            descriptor_disc_req_send();
        }
    } else if (p_ble_evt->evt.gattc_evt.gatt_status) 
    {
        
        // connected but cannot continue service discovery :(
        handle_discovery_failure(p_ble_evt->evt.gattc_evt.gatt_status);
        
    } else 
    {
        uint32_t i;
        const ble_gattc_char_t *p_char_resp = NULL;
        
        // iterate through characteristics and find them.
        for (i=0; i < p_ble_evt->evt.gattc_evt.params.char_disc_rsp.count; i++) 
        {
            p_char_resp = &(p_ble_evt->evt.gattc_evt.params.char_disc_rsp.chars[i]);
            switch(p_char_resp->uuid.uuid) 
            {
                case BLE_UUID_ANCS_CONTROL_POINT_CHAR:
                    characteristics_set(&m_service.control_point, p_char_resp);
                    N_SPRINTF("[ANCS] Control point assigned characteristic %i", p_char_resp->handle_value);
                    break;
                case BLE_UUID_ANCS_NOTIFICATION_SOURCE_CHAR:
                    characteristics_set(&m_service.notification_source, p_char_resp);
                    N_SPRINTF("[ANCS] Notification source assigned characteristic %i", p_char_resp->handle_value);
                    break;
                case BLE_UUID_ANCS_DATA_SOURCE_CHAR:
                    characteristics_set(&m_service.data_source, p_char_resp);
                    N_SPRINTF("[ANCS] Data source assigned characteristic %i", p_char_resp->handle_value);
                    break;
                
                default:
                    break;
                
            }
            
        }
        
        if (p_char_resp != NULL) {
            // if we didn't get all characteristics, do another round
            ble_gattc_handle_range_t char_handle;
            char_handle.start_handle = p_char_resp->handle_value + 1;
            char_handle.end_handle = m_service.service.handle_range.end_handle;
            
            characteristic_disc_req_send(&char_handle);
        } else {
            characteristic_disc_req_send(&(m_service.service.handle_range));
        }
    
    }
    
}

/**@brief Function for setting the discovered descriptor in the apple service.
 */
static void descriptor_set(apple_service_t * p_service, const ble_gattc_desc_t * p_desc_resp)
{
    if (p_service->control_point.handle_value == (p_desc_resp->handle - 1))
    {
        if (p_desc_resp->uuid.uuid == BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG)
        {
            p_service->control_point.handle_cccd = p_desc_resp->handle;
        }
    }

    else if (p_service->notification_source.handle_value == (p_desc_resp->handle - 1))
    {
        if (p_desc_resp->uuid.uuid == BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG)
        {
            p_service->notification_source.handle_cccd = p_desc_resp->handle;
        }
    }
    else if (p_service->data_source.handle_value == (p_desc_resp->handle - 1))
    {
        if (p_desc_resp->uuid.uuid == BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG)
        {
            p_service->data_source.handle_cccd = p_desc_resp->handle;
        }
    }
}

static void ble_ancs_event_descriptor_rsp(const ble_evt_t *p_ble_evt)
{
    N_SPRINTF("[ANCS] Got a descriptor...");
    if (p_ble_evt->evt.gattc_evt.gatt_status == BLE_GATT_STATUS_ATTERR_ATTRIBUTE_NOT_FOUND ||
        p_ble_evt->evt.gattc_evt.gatt_status == BLE_GATT_STATUS_ATTERR_INVALID_HANDLE)
    {
        handle_discovery_failure(NRF_ERROR_NOT_FOUND);
    }
    else if (p_ble_evt->evt.gattc_evt.gatt_status)
    {
        handle_discovery_failure(p_ble_evt->evt.gattc_evt.gatt_status);
    }
    else
    {
        if (p_ble_evt->evt.gattc_evt.params.desc_disc_rsp.count > 0)
        {
            descriptor_set(&m_service, &(p_ble_evt->evt.gattc_evt.params.desc_disc_rsp.descs[0]));
        }
        
        if (m_service.notification_source.handle_cccd == BLE_ANCS_INVALID_HANDLE ||
            m_service.data_source.handle_cccd == BLE_ANCS_INVALID_HANDLE)
        {
            descriptor_disc_req_send();
        }
        else
        {
            connection_established();
        }
    }
    
}

/**@brief Function for parsing received notification attribute response data.
 */
static void parse_get_notification_attributes_response( const uint8_t *data, int len)
{
    static uint8_t *ptr;
    static uint16_t current_len;
    static ble_ancs_c_evt_t event;
    static uint16_t buff_idx;
    int i;

    for (i = 0; i < len; i++)
    {
        switch (m_parse_state)
        {
        case COMMAND_ID:
            event.data.attribute.command_id = data[i];
            m_parse_state = NOTIFICATION_UID1;
            break;

        case NOTIFICATION_UID1:
            event.data.attribute.notification_uid[0] = data[i];
            m_parse_state = NOTIFICATION_UID2;
            break;

        case NOTIFICATION_UID2:
            event.data.attribute.notification_uid[1] = data[i];
            m_parse_state = NOTIFICATION_UID3;
            break;

        case NOTIFICATION_UID3:
            event.data.attribute.notification_uid[2] = data[i];
            m_parse_state = NOTIFICATION_UID4;
            break;

        case NOTIFICATION_UID4:
            event.data.attribute.notification_uid[3] = data[i];
            m_parse_state = ATTRIBUTE_ID;
            break;

        case ATTRIBUTE_ID:
            event.data.attribute.attribute_id = data[i];
            m_parse_state = ATTRIBUTE_LEN1;
            break;

        case ATTRIBUTE_LEN1:
            event.data.attribute.attribute_len = data[i];
            m_parse_state = ATTRIBUTE_LEN2;
            break;

        case ATTRIBUTE_LEN2:
            event.data.attribute.attribute_len |= (data[i] << 8);
            m_parse_state = ATTRIBUTE_READY;
            ptr = event.data.attribute.data;
            buff_idx = 0;
            current_len = 0;
            break;

        case ATTRIBUTE_READY:
            if (buff_idx < ANCS_ATTRIBUTE_DATA_MAX - 1) // Prevents buffer overflow
                ptr [buff_idx++] = data[i];

            current_len++;

            if (current_len == event.data.attribute.attribute_len)
            {
                ptr [buff_idx] = 0;
                //event.data.attribute.attribute_len = buff_idx - 1;
                event.evt_type = BLE_ANCS_C_EVT_NOTIF_ATTRIBUTE;
								_got_ancs_data(&event);
                m_parse_state = ATTRIBUTE_ID;
            }
            break;
        }
    }
}

/*
 * We got a notification! We should respond only to events from the notification source or 
 * data source. The others are GCP.
 */ 
static void ble_ancs_event_notify(const ble_evt_t * p_ble_evt) {

    ble_ancs_c_evt_t event;
    const ble_gattc_evt_hvx_t * p_notification = &p_ble_evt->evt.gattc_evt.params.hvx;

    N_SPRINTF("[ANCS] Got message from handle: %i len: %i", p_notification->handle, p_notification->len);
	if (p_notification->len < NOTIFICATION_DATA_LENGTH) {
		return; // invalid length
	}
		
    // handle 
	if (p_notification->handle == m_service.notification_source.handle_value) {
		BLE_UUID_COPY_INST(event.uuid, m_service.notification_source.uuid);
		
		event.data.notification.event_id       = p_notification->data[0];
		event.data.notification.event_flags    = p_notification->data[1];
		event.data.notification.category_id    = p_notification->data[2];
		event.data.notification.category_count = p_notification->data[3];
		memcpy(event.data.notification.notification_uid, &p_notification->data[4], 4);
		event.evt_type = BLE_ANCS_C_EVT_IOS_NOTIFICATION;
		
		_got_ancs_data(&event);
///*
		if (event.data.notification.event_id == BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED)   //add new notification
        {
            ble_ancs_attr_list_t attr_list[2];                                              //get_notification_attributes
            //attr_list[0].attribute_id = BLE_ANCS_NOTIFICATION_ATTRIBUTE_ID_TITLE;
            attr_list[0].attribute_id = 1;	//	TITLE
            attr_list[0].attribute_len = 32;
            // attr_list[1].attribute_id = BLE_ANCS_NOTIFICATION_ATTRIBUTE_ID_MESSAGE;
            attr_list[1].attribute_id = 3;		//		MESSAGE	
           //attr_list[1].attribute_len = ANCS_ATTRIBUTE_MESSAGE_DATA_MAX;    //max:96 bite 32???          
             attr_list[1].attribute_len = 32;
		    //Y_SPRINTF("[ANCS+++++]ANCS_get_notification_attributes...");
            uint32_t err_code = ANCS_get_notification_attributes(event.data.notification.notification_uid, 2, attr_list);
		    //Y_SPRINTF("[ANCS+++++]ANCS_get_notification_attributes...err:%d",err_code);
            APP_ERROR_CHECK(err_code);
            m_parse_state = COMMAND_ID;
        }	
				//*/	
	}  
	else if (p_notification->handle == m_service.data_source.handle_value) {
        N_SPRINTF("Got a data source response...");
		BLE_UUID_COPY_INST(event.uuid, m_service.data_source.uuid);
		
		parse_get_notification_attributes_response(p_notification->data, p_notification->len);
	} else {
		return; // this notify is not part of ANCS
	}	
	return;
	
}

/**@brief Display an iOS notification
 *
 */
void evt_ios_notification(ble_ancs_c_evt_ios_notification_t *p_notice)
{
#if 1
	N_SPRINTF("[ANCS] event: %02x%02x%02x%02x",
        p_notice->notification_uid[0], p_notice->notification_uid[1], 
        p_notice->notification_uid[2], p_notice->notification_uid[3]);
    
	N_SPRINTF("[ANCS] event id: %s ", lit_eventid[p_notice->event_id]);
	
	N_SPRINTF("[ANCS] cat id: %s ", lit_catid[p_notice->category_id]);
	#endif
}



static void ble_ancs_event_encrypted_link(const ble_evt_t *p_ble_evt) {
    N_SPRINTF("[ANCS] Link encrypted!");
    // if we are setting up a bonded connection and the UUID of the
    // service is unkown, a new discovery must be performed.
    if (m_service.service.uuid.uuid != BLE_UUID_APPLE_NOTIFICATION_CENTER_SERVICE) {
			N_SPRINTF("[ANCS] new discovery, but not ANCS ");
			m_service.handle = INVALID_SERVICE_HANDLE;
			service_disc_req_send();
    } else {
			N_SPRINTF("[ANCS] new discovery, it is ANCS ");
      connection_established();
    }
}

static void ble_ancs_event_discover_rsp(const ble_evt_t *p_ble_evt) 
{
    N_SPRINTF("[ANCS] Service discovery responded...");
	
    if (p_ble_evt->evt.gattc_evt.gatt_status != BLE_GATT_STATUS_SUCCESS) {
        // This happens when iOS tries to connect without populating its GATT attribute table.
        // Retry later...
        
        // On Android this just fails 5 times and we can get on with our
        // lives.
        handle_discovery_failure(p_ble_evt->evt.gattc_evt.gatt_status);
        
    } else {
        BLE_UUID_COPY_INST(m_service.service.uuid,
                           p_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.services[0].uuid);
        
        if (p_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.count > 0) {
            
            const ble_gattc_service_t * p_service;
            
            p_service = &(p_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.services[0]);
            m_service.service.handle_range.start_handle = p_service->handle_range.start_handle;
            m_service.service.handle_range.end_handle = p_service->handle_range.end_handle;
            
            characteristic_disc_req_send( &(m_service.service.handle_range));
        } else {
            handle_discovery_failure(p_ble_evt->evt.gattc_evt.gatt_status);
        }
    }
}

static BOOLEAN _is_ancs_enabled()
{
    return cling.ancs.b_enabled;
}
    

void ANCS_start_ancs_discovery()
{	
    N_SPRINTF("[ANCS] Enabling ANCS & start discovery ...");

    if (ancs_master_handle != INVALID_CENTRAL_HANDLE)
    {
        m_client_state = STATE_WAITING_ENC;
    }
    else
    {
			m_service.handle = INVALID_SERVICE_HANDLE;
			service_disc_req_send();
		}
}
    
/**@brief Function for handling of BLE stack events.
 */
void ANCS_on_ble_evt(const ble_evt_t * p_ble_evt)
{

    uint16_t event = p_ble_evt->header.evt_id;
    I32U err_code;
    bool is_encrypted;
	
		N_SPRINTF("[ANCS] +++ state: %d, event: %d", m_client_state, event);

    switch (m_client_state)
    {
				case STATE_UNINITIALIZED:
						break;
        // idle->connected
				case STATE_IDLE:
				{
					if (event == BLE_GAP_EVT_CONNECTED) {
							N_SPRINTF("[ANCS] apple notifications enabled: %02x", apple_notifications_enabled);
							ancs_conn_retry_count = 0;
							if (_is_ancs_enabled())
							{
									ANCS_start_ancs_discovery();
							}
					}
					break;
				}
        // connected and waiting for encryption
				case STATE_WAITING_ENC:
				{
					if ((event == BLE_GAP_EVT_AUTH_STATUS) || (event == BLE_GAP_EVT_SEC_INFO_REQUEST)) {
						ble_ancs_event_encrypted_link(p_ble_evt);
					} else if (event == BLE_GAP_EVT_DISCONNECTED) {
						ble_ancs_event_disconnect();
					}
					break;
				}
        // encrypted and doing service discovery
				case STATE_DISC_SERV:
				{
					N_SPRINTF("[ANCS] +++ Service discovered");
					if (event == BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP) {
						ble_ancs_event_discover_rsp(p_ble_evt);
					} else if (event == BLE_GAP_EVT_DISCONNECTED) {
						ble_ancs_event_disconnect();
					}
					break;
				}
        // finally discovering characteristics
				case STATE_DISC_CHAR:
				{
					N_SPRINTF("[ANCS] characteritics found ");
					if (event == BLE_GATTC_EVT_CHAR_DISC_RSP) {
							ble_ancs_event_characteristic_rsp(p_ble_evt);
					} else if (event == BLE_GAP_EVT_DISCONNECTED) {
							ble_ancs_event_disconnect();
					}
					break;
				}
        // services discovered, finding descriptors...
				case STATE_DISC_DESC:
				{
					N_SPRINTF("[ANCS] Service discovered, and got descriptor");
					if (event == BLE_GATTC_EVT_DESC_DISC_RSP) {
						ble_ancs_event_descriptor_rsp(p_ble_evt);
					} else if (event == BLE_GAP_EVT_DISCONNECTED) {
						ble_ancs_event_disconnect();
					}
					break;
				}
        // subscribe to the notification source.
        case STATE_RUNNING_ENABLE_NOTIF_SOURCE:
				{
						N_SPRINTF("[ANCS] +++ subscribe to the Apple notification source");
#if 0					
            // will only happen on iOS (when bonding is done)
            ble_bondmngr_is_link_encrypted(&is_encrypted);
					#else
				//	is_encrypted = FALSE;
										is_encrypted = TRUE;  //jy_add
					
#endif
            if (event == BLE_GAP_EVT_DISCONNECTED) {

							ble_ancs_event_disconnect();
						
							N_SPRINTF("[ANCS] +++ subscription: disconnected.");

						} else if (is_encrypted) {

                // Try enabling the notification source
                err_code = ble_ancs_c_enable_notif_notification_source(cling.ble.conn_handle);

                if ((err_code == NRF_SUCCESS) && (m_service.notification_source.handle_value)) {
                    N_SPRINTF("[ANCS] subscribed to notification source");
                    m_client_state = STATE_RUNNING_ENABLE_DATA_SOURCE;
                } 
            }
            break;
				}
        // We have written the subscription to the notification service, but we also need to
        // enable the data source if we want any useful attributes.
        case STATE_RUNNING_ENABLE_DATA_SOURCE:
				{
						N_SPRINTF("[ANCS] +++ Enable data source in the Apple notification center");
            // We got the write response from enabling the notif. service
            if (event == BLE_GATTC_EVT_WRITE_RSP) {
                err_code = ble_ancs_c_enable_notif_data_source(cling.ble.conn_handle);

                if ((err_code == NRF_SUCCESS) && (m_service.data_source.handle_value)) {
                    N_SPRINTF("[ANCS] subscribed to data source");
                    m_client_state = STATE_STORE_SYS_ATTR;
                } 
            } else if (event == BLE_GAP_EVT_DISCONNECTED) {
							ble_ancs_event_disconnect();
						}
            break;
				}
        case STATE_STORE_SYS_ATTR:
				{
					if (event == BLE_GAP_EVT_DISCONNECTED) {
							ble_ancs_event_disconnect();
					} else {
							// We are encrypted and have subscribed to the ANCS service, now store the attributes
							// so the bond can persist.
							N_SPRINTF("[ANCS] store system atrributes");
#if 0
							// This fails if the bond has already been persisted, which is ok.
							ble_bondmngr_sys_attr_store();
#endif
						
							m_client_state = STATE_RUNNING;
					} 
					break;
				}
        // normal conected operation
        case STATE_RUNNING:
				{
					if (event == BLE_GATTC_EVT_HVX) {
						// we got a notify, make sure it is from the service
						ble_ancs_event_notify(p_ble_evt);
						N_SPRINTF("[ANCS] +++ RUNING: Got some notification data");
					} else if (event == BLE_GATTC_EVT_WRITE_RSP) {
						N_SPRINTF("[ANCS] +++ RUNING: EVT write rsp, Finished write response...");
						// free to transmit next packet, but we do this from the stack in
						// the main context. This is just a notification that the radio is not "busy" waiting
						// for a GATT write response.
					} else if (event == BLE_GAP_EVT_DISCONNECTED) {
						ble_ancs_event_disconnect();
						N_SPRINTF("[ANCS] +++ RUNING: EVT disconnected");
					}
					break;
				}
        // default / not discovered. We got here by failing ANCS discovery. Try again, 
        // (up to a certain number of times)
        // because service discovery fails under certain conditions on iOS (reboot).
				case STATE_RUNNING_NOT_DISCOVERED:
				default:
				{
						if (event == BLE_GAP_EVT_DISCONNECTED)
						{
							ble_ancs_event_disconnect();
						} else {
							if (ancs_conn_retry_count++ < ANCS_CONN_RETRY_MAX) 
							{
								// force service discovery
								N_SPRINTF("[ANCS] failed, try it one more time");
								ancs_master_handle = INVALID_CENTRAL_HANDLE;
								ANCS_start_ancs_discovery();
							}
						}							
						break;
				}
	}
}

// pstorage function callback.
void ancs_pstorage_callback(pstorage_handle_t *handle,
                            uint8_t            op_code,
                            uint32_t           reason,
                            uint8_t           *p_data,
                            uint32_t           param_len) 
{
     if (reason != NRF_SUCCESS) {
         N_SPRINTF("pstorage error: %08x", reason);
        //APP_ERROR_HANDLER(reason);
     }         

}

uint32_t ble_ancs_c_init()
{
    uint32_t           err_code;
    err_code         = NRF_SUCCESS;
    m_client_state   = STATE_IDLE;
    
    memset(&m_service, 0, sizeof(apple_service_t));

    pstorage_module_param_t param;
    param.block_count = 1;
   // param.block_size = DISCOVERED_SERVICE_DB_SIZE * sizeof(uint32_t);
	 param.block_size = sizeof(apple_service_t) * sizeof(uint32_t);
    param.cb = ancs_pstorage_callback;
		
    err_code = pstorage_register(&param, &m_flash_handle);

		return err_code;
}


uint32_t ble_ancs_send_message(tx_message_t * msg) {

	if (msg->type == READ_REQ) {
		return sd_ble_gattc_read(msg->conn_handle, msg->req.read_handle, 0);
	} else {
		return sd_ble_gattc_write(msg->conn_handle, &msg->req.write_req.gattc_params);
	}
}

/*
 * Send a packet to subscribe to an ANCS notification characteristic.
 */

uint32_t ble_ancs_cccd_configure(uint16_t conn_handle, uint16_t handle_cccd, bool enable) {

    tx_message_t   p_msg;
    uint16_t       cccd_val = enable ? BLE_CCCD_NOTIFY_BIT_MASK : 0;

		// Allow device to configure CCCD at any given time as iOS reset ANCS info everytime when it disconnects
		// from Cling device.
    if ((m_client_state != STATE_RUNNING) && (m_client_state != STATE_RUNNING_ENABLE_DATA_SOURCE) && (m_client_state != STATE_RUNNING_ENABLE_NOTIF_SOURCE))
    {
 //       return NRF_ERROR_INVALID_STATE;
    }

    p_msg.req.write_req.gattc_params.handle   = handle_cccd;
    p_msg.req.write_req.gattc_params.len      = 2;
    p_msg.req.write_req.gattc_params.p_value  = p_msg.req.write_req.gattc_value;
    p_msg.req.write_req.gattc_params.offset   = 0;
    p_msg.req.write_req.gattc_params.write_op = BLE_GATT_OP_WRITE_REQ;
    p_msg.req.write_req.gattc_value[0]        = LSB(cccd_val);
    p_msg.req.write_req.gattc_value[1]        = MSB(cccd_val);
    p_msg.conn_handle                         = conn_handle;
    p_msg.type                                = WRITE_REQ;

    return ble_ancs_send_message(&p_msg);
}

uint32_t ble_ancs_c_enable_notif_notification_source(uint16_t conn_handle)
{
    return ble_ancs_cccd_configure(conn_handle,
                          m_service.notification_source.handle_cccd,
                          true);
}


uint32_t ble_ancs_c_disable_notif_notification_source(uint16_t conn_handle)
{
    return ble_ancs_cccd_configure(conn_handle,
                          m_service.notification_source.handle_cccd,
                          false);
}


uint32_t ble_ancs_c_enable_notif_data_source(uint16_t conn_handle)
{
    return ble_ancs_cccd_configure(conn_handle,
                          m_service.data_source.handle_cccd,
                          true);
}


uint32_t ble_ancs_c_disable_notif_data_source(uint16_t conn_handle)
{
    return ble_ancs_cccd_configure(conn_handle,
                          m_service.data_source.handle_cccd,
                          false);
}


// load connection info from flash
uint32_t ble_ancs_c_service_load(void)
{
    uint32_t err_code;
    //uint32_t i;

    err_code = NRF_SUCCESS;

    pstorage_handle_t block_handle;
    
    err_code = pstorage_block_identifier_get(&m_flash_handle, 0, &block_handle);
    
    err_code = pstorage_load((uint8_t *)&m_service,
                             &block_handle,
                             sizeof(apple_service_t),
                             0);

    if (err_code != NRF_SUCCESS)
    {
        
        N_SPRINTF("[ANCS] Could not load stored ANCS service from flash");
        // Problem with loading values from flash, initialize the RAM DB with default.
        m_service.handle = INVALID_SERVICE_HANDLE;

        if (err_code == NRF_ERROR_NOT_FOUND)
        {
            // The flash does not contain any memorized centrals, set the return code to success.
            err_code = NRF_SUCCESS;
        }
    }
    
    
    return err_code;
}

uint32_t ble_ancs_c_service_delete(void)
{
    return pstorage_clear(&m_flash_handle, sizeof(apple_service_t) * 4);   
    
}

void ANCS_service_add(void)
{
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

	err_code = ble_ancs_c_init();
	APP_ERROR_CHECK(err_code);

	err_code = ble_ancs_c_service_load();
	APP_ERROR_CHECK(err_code);

	N_SPRINTF("[ANCS] service added");
}
	

void ANCS_uuid_set(ble_uuid_t *uuid)
{
    uuid->uuid = ((ble_ancs_base_uuid128.uuid128[12]) | (ble_ancs_base_uuid128.uuid128[13] << 8));
    uuid->type = m_ancs_uuid_type;   
}


uint32_t ANCS_get_notification_attributes( uint8_t * p_uid, uint8_t num_attr, ble_ancs_attr_list_t *p_attr)
{
    tx_message_t  p_msg;
    uint32_t i = 0;
    
    if (m_client_state != STATE_RUNNING)
    {
        return NRF_ERROR_INVALID_STATE;
    }

  //  p_msg = &m_tx_buffer[m_tx_insert_index++];
  //  m_tx_insert_index &= TX_BUFFER_MASK;
    p_msg.req.write_req.gattc_params.handle   = m_service.control_point.handle_value;
    p_msg.req.write_req.gattc_params.p_value  = p_msg.req.write_req.gattc_value;
    p_msg.req.write_req.gattc_params.offset   = 0;
    p_msg.req.write_req.gattc_params.write_op = BLE_GATT_OP_WRITE_REQ;

   // p_msg.req.write_req.gattc_value[i++]      = BLE_ANCS_COMMAND_ID_GET_NOTIFICATION_ATTRIBUTES;
		p_msg.req.write_req.gattc_value[i++]      = 0;    //0:Get Notification Attributes   1:Get App Attributes
    memcpy(&p_msg.req.write_req.gattc_value[1], p_uid, 4);
    i += 4;
    
    while (num_attr > 0)
    {
      p_msg.req.write_req.gattc_value[i++] = p_attr->attribute_id;
      if (p_attr->attribute_len > 0)
      {
        p_msg.req.write_req.gattc_value[i++] = (uint8_t) (p_attr->attribute_len);
        p_msg.req.write_req.gattc_value[i++] = (uint8_t) (p_attr->attribute_len >> 8);
      }
      p_attr++;
      num_attr--;
    }
    
        p_msg.req.write_req.gattc_params.len      = i;
        //p_msg.conn_handle                       = p_ancs->conn_handle;
		    p_msg.conn_handle                         =	cling.ble.conn_handle;   //????
        p_msg.type                                = WRITE_REQ;


       // return NRF_SUCCESS;
		    return ble_ancs_send_message(&p_msg);
}

#endif
