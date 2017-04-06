/********************************************************************************

 **** Copyright (C), 2016, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : PlayerController.c
 * Author        : MikeWang
 * Date          : 2016-2-1
 * Description   : PlayerController.c source file
 * Version       : 1.0
 * Function List :
 *
 * Record        :
 * 1.Date        : 2016-2-25
 *   Author      : MikeWang
 *   Modification: Created file

*************************************************************************************************************/
#include "player_controller.h"
#include "uicodata.h"
#include "string.h"
#include "ble_hids.h"

/*=====================================MACROS=====================================================================*/
#define INPUT_CC_REPORT_KEYS_MAX_LEN	 2
#define INPUT_CCONTROL_KEYS_INDEX		 1
#define INPUT_CC_REP_REF_ID				 2
#define OUTPUT_REP_REF_ID                0
#define OUTPUT_REPORT_INDEX              0                                              /**< Index of Output Report. */
#define OUTPUT_REPORT_MAX_LEN            1                                              /**< Maximum length of Output Report. */
#define INPUT_REPORT_KEYS_INDEX          0                                              /**< Index of Input Report. */
#define OUTPUT_REPORT_BIT_MASK_CAPS_LOCK 0x02                                           /**< CAPS LOCK bit in Output Report (based on 'LED Page (0x08)' of the Universal Serial Bus HID Usage Tables). */
#define INPUT_REP_REF_ID                 0                                              /**< Id of reference to Keyboard Input Report. */
#define OUTPUT_REP_REF_ID                0                                              /**< Id of reference to Keyboard Output Report. */
#define BASE_USB_HID_SPEC_VERSION        0x0101                                         /**< Version number of base USB HID Specification implemented by this application. */
#define DEAD_BEEF                        0xDEADBEEF                                     /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
#define MODIFIER_KEY_POS                 0                                              /**< Position of the modifier byte in the Input Report. */
#define SCAN_CODE_POS                    2                                              /**< This macro indicates the start position of the key scan code in a HID Report. As per the document titled 'Device Class Definition for Human Interface Devices (HID) V1.11, each report shall have one modifier byte followed by a reserved constant byte and then the key scan code. */
#define SHIFT_KEY_CODE                   0x02                                           /**< Key code indicating*/
#define MAX_BUFFER_ENTRIES 							 3
#define MULTI_MEDIA_KEYBOARD_ID   			 0X02
#define INPUT_REPORT_KEYS_MAX_LEN       sizeof(m_caps_off_key_pattern)



/*===========================================TYPEDEF================================================================*/
typedef enum {
    RELEASE_KEY                     = 0x0000,
    CONSUMER_CTRL_PLAY              = 0x0001,
    CONSUMER_CTRL_ALCCC             = 0x0002,
    CONSUMER_CTRL_SCAN_NEXT_TRACK   = 0x0004,
    CONSUMER_CTRL_SCAN_PREV_TRACK   = 0x0008,
    CONSUMER_CTRL_VOL_DW            = 0x0010,
    CONSUMER_CTRL_VOL_UP            = 0x0020,
    CONSUMER_CTRL_AC_FORWARD        = 0x0040,
    CONSUMER_CTRL_AC_BACK           = 0x0080,
		CONSUMER_CTRL_MUTE           		= 0x0100,
		CONSUMER_CTRL_STOP           		= 0x0200,
} consumer_control_t;


typedef struct hid_key_buffer {
    uint8_t    data_offset;   /**< Max Data that can be buffered for all entries */
    uint8_t    data_len;      /**< Total length of data */
    uint8_t    p_data[8];      /**< Scanned key pattern */
    ble_hids_t * p_instance;  /**< Identifies peer and service instance */
} buffer_entry_t;

/** Circular buffer list */
typedef struct {
    buffer_entry_t buffer[MAX_BUFFER_ENTRIES]; /**< Maximum number of entries that can enqueued in the list */
    uint8_t        rp;                         /**< Index to the read location */
    uint8_t        wp;                         /**< Index to write location */
    uint8_t        count;                      /**< Number of elements in the list */
} buffer_list_t;


struct media_control {
    uint8_t report_id;
    union {
        uint16_t value;
        struct {
            uint8_t next_track: 1;
            uint8_t prev_track: 1;
            uint8_t stop: 1;
            uint8_t play_pause: 1;
            uint8_t mute: 1;
            uint8_t volume_up: 1;
            uint8_t volume_down: 1;
            uint8_t www_home: 1;
            uint8_t my_computer: 1;
            uint8_t calculator: 1;
            uint8_t www_fav: 1;
            uint8_t www_search: 1;
            uint8_t www_stop: 1;
            uint8_t www_back: 1;
            uint8_t media_sel: 1;
            uint8_t mail: 1;
        } bits;
    } ctl_byte;
};

/*======================================LOCAL VARIES============================================================================*/
static buffer_list_t buffer_list;
/**Buffer queue access macros
 *
 * @{ */
/** Initialization of buffer list */
#define BUFFER_LIST_INIT()                                                                        \
        do                                                                                        \
        {                                                                                         \
            buffer_list.rp = 0;                                                                   \
            buffer_list.wp = 0;                                                                   \
            buffer_list.count = 0;                                                                \
        } while (0)

/** Provide status of data list is full or not */
#define BUFFER_LIST_FULL()\
        ((MAX_BUFFER_ENTRIES == buffer_list.count - 1) ? true : false)

/** Provides status of buffer list is empty or not */
#define BUFFER_LIST_EMPTY()\
        ((0 == buffer_list.count) ? true : false)

#define BUFFER_ELEMENT_INIT(i)\
        do                                                                                        \
        {                                                                                         \
             memset(buffer_list.buffer[(i)].p_data, 0 , sizeof(buffer_list.buffer[(i)].p_data)); \
        } while (0)

/** @} */
static CLASS(PlayerController)* p_instance = NULL;

static const uint8_t m_caps_on_key_pattern[] =                                                /**< Key pattern to be sent when the output report has been written with the CAPS LOCK bit set. */
{
    0x00, /* Modifier status byte */
    0x00, /* Reserved 0x00 */
    0x06, /* Key C */
    0x04, /* Key a */
    0x13, /* Key p */
    0x16, /* Key s */
    0x12, /* Key o */
    0x11  /* Key n */
};

static const uint8_t m_caps_off_key_pattern[] =                                               /**< Key pattern to be sent when the output report has been written with the CAPS LOCK bit cleared. */
{
    0x00, /* Modifier status byte */
    0x00, /* Reserved 0x00 */
    0x06, /* Key C */
    0x04, /* Key a */
    0x13, /* Key p */
    0x16, /* Key s */
    0x12, /* Key o */
    0x09  /* Key f */
};

static const uint8_t m_sample_key_release_pattern[INPUT_REPORT_KEYS_MAX_LEN] =                /**< Key pattern to be sent to simulate releasing keys. */
{
    0x00
};

static const uint8_t report_map_data[] = {
#if 0
        0x05, 0x01,                 // Usage Page (Generic Desktop)
        0x09, 0x06,                 // Usage (Keyboard)
        0xA1, 0x01,                 // Collection (Application)
        0x85, 0x01,                     //     Report Id (1)
        0x05, 0x07,                 //     Usage Page (Key Codes)
        0x19, 0xe0,                 //     Usage Minimum (224)
        0x29, 0xe7,                 //     Usage Maximum (231)
        0x15, 0x00,                 //     Logical Minimum (0)
        0x25, 0x01,                 //     Logical Maximum (1)
        0x75, 0x01,                 //     Report Size (1)
        0x95, 0x08,                 //     Report Count (8)
        0x81, 0x02,                 //     Input (Data, Variable, Absolute)

        0x95, 0x01,                 //     Report Count (1)
        0x75, 0x08,                 //     Report Size (8)
        0x81, 0x01,                 //     Input (Constant) reserved byte(1)

        0x95, 0x05,                 //     Report Count (5)
        0x75, 0x01,                 //     Report Size (1)
        0x05, 0x08,                 //     Usage Page (Page# for LEDs)
        0x19, 0x01,                 //     Usage Minimum (1)
        0x29, 0x05,                 //     Usage Maximum (5)
        0x91, 0x02,                 //     Output (Data, Variable, Absolute), Led report
        0x95, 0x01,                 //     Report Count (1)
        0x75, 0x03,                 //     Report Size (3)
        0x91, 0x01,                 //     Output (Data, Variable, Absolute), Led report padding

        0x95, 0x06,                 //     Report Count (6)
        0x75, 0x08,                 //     Report Size (8)
        0x15, 0x00,                 //     Logical Minimum (0)
        0x25, 0x65,                 //     Logical Maximum (101)
        0x05, 0x07,                 //     Usage Page (Key codes)
        0x19, 0x00,                 //     Usage Minimum (0)
        0x29, 0x65,                 //     Usage Maximum (101)
        0x81, 0x00,                 //     Input (Data, Array) Key array(6 bytes)

        0x09, 0x05,                 //     Usage (Vendor Defined)
        0x15, 0x00,                 //     Logical Minimum (0)
        0x26, 0xFF, 0x00,           //     Logical Maximum (255)
        0x75, 0x08,                 //     Report Count (2)
        0x95, 0x02,                 //     Report Size (8 bit)
        0xB1, 0x02,                 //     Feature (Data, Variable, Absolute)
        0xC0,                       // End Collection (Application)
#endif
        // Report ID 2: Advanced buttons
        0x05, 0x0C,                     // Usage Page (Consumer)
        0x09, 0x01,                     // Usage (Consumer Control)
        0xA1, 0x01,                     // Collection (Application)
        0x85, 0x02,                     //     Report Id (2)
        0x15, 0x00,                     //     Logical minimum (0)
        0x25, 0x01,                     //     Logical maximum (1)
        0x75, 0x01,                     //     Report Size (1)
        0x95, 0x01,                     //     Report Count (1)

        0x09, 0xCD,                     //     Usage (Play/Pause)
        0x81, 0x02,                     //     Input (Data,Value,Relative,Bit Field)
        0x0A, 0x83, 0x01,               //     Usage (AL Consumer Control Configuration)
        0x81, 0x02,                     //     Input (Data,Value,Relative,Bit Field)
        0x09, 0xB5,                     //     Usage (Scan Next Track)
        0x81, 0x02,                     //     Input (Data,Value,Relative,Bit Field)
        0x09, 0xB6,                     //     Usage (Scan Previous Track)
        0x81, 0x02,                     //     Input (Data,Value,Relative,Bit Field)

        0x09, 0xEA,                     //     Usage (Volume Down)
        0x81, 0x02,                     //     Input (Data,Value,Relative,Bit Field)
        0x09, 0xE9,                     //     Usage (Volume Up)
        0x81, 0x02,                     //     Input (Data,Value,Relative,Bit Field)
        0x0A, 0x25, 0x02,               //     Usage (AC Forward)
        0x81, 0x02,                     //     Input (Data,Value,Relative,Bit Field)
        0x0A, 0x24, 0x02,               //     Usage (AC Back)
        0x81, 0x02,                     //     Input (Data,Value,Relative,Bit Field)
				0x09, 0xe2, 										// 		 USAGE (Mute) 0x01
				0x81, 0x02,                     //     Input (Data,Value,Relative,Bit Field)
				0x09, 0xb7, 										// 		 USAGE (Stop) 0x05
				0x81, 0x02,											// 		 Input (Data,Value,Relative,Bit Field)
				0x09, 0xB8, 										// 		 USAGE (eject) 0x05
				0x81, 0x02,											// 		 Input (Data,Value,Relative,Bit Field)
				0x95, 0x01,                 		//     Report Count (1)
        0x75, 0x05,                 		//     Report Size (6)
        0x81, 0x02,                			 //    Output (Data, Variable, Absolute), control report padding			
        0xC0                            // 		  End Collection
};

const uint8_t m_sample_key_a[] =                                           /**< Key pattern to be sent when the key press button has been pushed. */
{
    0x00, /* Modifier status byte */
    0x00, /* Reserved 0x00 */
    0x04, /* Key a */
    0x00, /* Key null */
    0x00, /* Key l */
    0x00, /* Key l */
    0x00, /* Key o */
    0x00  /* Key Return */
};
const uint8_t m_sample_key_release[] =                                           /**< Key pattern to be sent when the key press button has been pushed. */
{
    0x00, /* Modifier status byte */
    0x00, /* Reserved 0x00 */
    0x00, /* Key a */
    0x00, /* Key null */
    0x00, /* Key l */
    0x00, /* Key l */
    0x00, /* Key o */
    0x00  /* Key Return */
};		
/*========================================funciton statement============================================================*/
static void hids_init(ble_hids_t * p_hids);
static bool m_in_boot_mode = false;                        /**< Current protocol mode. */
static void on_hid_rep_char_write(CLASS(PlayerController) *p, ble_hids_evt_t *p_evt);
static uint32_t buffer_dequeue( CLASS(PlayerController) *p, bool tx_flag);
static void on_hids_evt(ble_hids_t * p_hids, ble_hids_evt_t *p_evt);
static void hids_init(ble_hids_t * p_hids);
static int player_contoller_on_ble(CLASS(PlayerController) *p, ble_evt_t * p_ble_evt);
void media_player_send_charactor_a(void);
static uint32_t consumer_control_send(CLASS(PlayerController) *p, consumer_control_t cmd);
static int media_player_volume_up(CLASS(PlayerController) *p);
static int media_player_volume_down(CLASS(PlayerController) *p);
static int media_player_play_pause(CLASS(PlayerController) *p);
static int media_player_next_track(CLASS(PlayerController) *p);
static int media_player_prev_track(CLASS(PlayerController) *p);
static int media_player_ac_forward(CLASS(PlayerController) *p);
static int media_player_ac_back(CLASS(PlayerController) *p);
static int media_player_stop(CLASS(PlayerController) *p);
static int media_player_mute(CLASS(PlayerController) *p);
static uint32_t keys_send(CLASS(PlayerController) *p, uint8_t key_pattern_len,  uint8_t * p_key_pattern);


/*****************************************************************************
 * Function      : PlayerController_init
 * Description   : PlayerController construction function used to initialize PlayerController
 * Input         : CLASS(PlayerController) *p object pointer
 * Output        : None
 * Return        : TRUE,SUCESS; FALSE,FAILED
 * Others        :
 * Record
 * 1.Date        : 2016201
 *   Author      : MikeWang
 *   Modification: Created function

*****************************************************************************/

static int PlayerController_init(CLASS(PlayerController) *p)
{
#define  PlayerController_CONNECTION_TEST_THREHOLD 5
    static ble_hids_t p_hids;
    p->volume_up = media_player_volume_up;
    p->volume_down = media_player_volume_down;
    p->next_track = media_player_next_track;
    p->prev_track = media_player_prev_track;
    p->play_pause = media_player_play_pause;
	  p->on_ble = player_contoller_on_ble;
		p->audio_back = media_player_ac_back;
		p->audio_forward = media_player_ac_forward;
		p->mute = media_player_mute;
		p->stop = media_player_stop;
    /*add user config file to object  private data*/
    p->user_data = (void*)&p_hids;
    hids_init((ble_hids_t *)p->user_data);
    if(p == NULL) {
        return FALSE;
    }
    /*object initilize peocess*/

    return TRUE;
}
/*****************************************************************************
 * Function      : PlayerController_get_instance
 * Description   : open api provded for user to get sinal instance onject pointer to make it single instance mode
 * Input         : void
 * Output        : None
 * Return        : object pointer
 * Others        :
 * Record
 * 1.Date        : 2016201
 *   Author      : MikeWang
 *   Modification: Created function

*****************************************************************************/
CLASS(PlayerController)* PlayerController_get_instance(void)
{

    if( p_instance == (CLASS(PlayerController)*)NULL) {
        static CLASS(PlayerController) p;
        p_instance = &p;
        PlayerController_init(p_instance);
    }
    return  p_instance;
}


/**@brief HID event handler.
 *
 * @details This function will be called for all HID events which are passed to the application.
 *
 * @param[in]   p_hids  HID service stucture.
 * @param[in]   p_evt   Event received from the HID service.
 */
static void on_hids_evt(ble_hids_t * p_hids, ble_hids_evt_t *p_evt)
{
    switch (p_evt->evt_type) {
        case BLE_HIDS_EVT_BOOT_MODE_ENTERED:
            m_in_boot_mode = true;
            break;

        case BLE_HIDS_EVT_REPORT_MODE_ENTERED:
            m_in_boot_mode = false;
            break;

        case BLE_HIDS_EVT_REP_CHAR_WRITE:
            (p_instance != NULL) ? (on_hid_rep_char_write(p_instance, p_evt)) : (void)NULL;
            break;

        case BLE_HIDS_EVT_NOTIF_ENABLED: {
					  dm_service_context_t   service_context;
            service_context.service_type = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;
            service_context.context_data.len = 0;
            service_context.context_data.p_data = NULL;
						static dm_handle_t m_bonded_peer_handle;                          /**< Device reference handle to the current bonded central. */
						dm_handle_get(p_hids->conn_handle, &m_bonded_peer_handle);
#if 1
            if (m_in_boot_mode) {
							
                // Protocol mode is Boot Protocol mode.
                if (
                    p_evt->params.notification.char_id.uuid
                    ==
                    BLE_UUID_BOOT_KEYBOARD_INPUT_REPORT_CHAR
                ) {
                    // The notification of boot keyboard input report has been enabled.
                    // Save the system attribute (CCCD) information into the flash.
									  uint32_t err_code;

                    err_code = dm_service_context_set(&m_bonded_peer_handle, &service_context);
                    if (err_code != NRF_ERROR_INVALID_STATE)
                    {
                        APP_ERROR_CHECK(err_code);
                    }
                    else
                    {
                        // The system attributes could not be written to the flash because
                        // the connected central is not a new central. The system attributes
                        // will only be written to flash only when disconnected from this central.
                        // Do nothing now.
                    }
										
                } else {
                    // Do nothing
                }
            } else if (p_evt->params.notification.char_id.rep_type == BLE_HIDS_REP_TYPE_INPUT) {
                // The protocol mode is Report Protocol mode. And the CCCD for the input report
                // is changed. It is now time to store all the CCCD information (system
                // attributes) into the flash.
									uint32_t err_code;

                err_code = dm_service_context_set(&m_bonded_peer_handle, &service_context);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
                else
                {
                    // The system attributes could not be written to the flash because
                    // the connected central is not a new central. The system attributes
                    // will only be written to flash only when disconnected from this central.
                    // Do nothing now.
                }
            } else {
                // The notification of the report that was enabled by the master is not interesting to this
                // application. So do nothing.
            }
            break;
#endif
        }

        default:
            break;
    }

}

/**@brief Function for initializing HID Service.
 */

static void hids_init(ble_hids_t * p_hids)
{
    uint32_t                   err_code;
    ble_hids_init_t            hids_init_obj;
    ble_hids_inp_rep_init_t    input_report_array[2];
    ble_hids_inp_rep_init_t  * p_input_report;
    ble_hids_outp_rep_init_t   output_report_array[1];
    ble_hids_outp_rep_init_t * p_output_report;
    uint8_t                    hid_info_flags;

    // Initialize HID Service - keyboard descriptor input and output report
    p_input_report                      = &input_report_array[INPUT_REPORT_KEYS_INDEX];
    p_input_report->max_len             = INPUT_REPORT_KEYS_MAX_LEN;
    p_input_report->rep_ref.report_id   = INPUT_REP_REF_ID;
    p_input_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_INPUT;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.write_perm);

    p_output_report                      = &output_report_array[OUTPUT_REPORT_INDEX];
    p_output_report->max_len             = OUTPUT_REPORT_MAX_LEN;
    p_output_report->rep_ref.report_id   = OUTPUT_REP_REF_ID;
    p_output_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_OUTPUT;

    //Initialize HID Service - ConsumerControl

    // Initialize HID Service
    p_input_report                      = &input_report_array[INPUT_CCONTROL_KEYS_INDEX];
    p_input_report->max_len             = INPUT_CC_REPORT_KEYS_MAX_LEN;
    p_input_report->rep_ref.report_id   = INPUT_CC_REP_REF_ID;
    p_input_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_INPUT;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_output_report->security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_output_report->security_mode.write_perm);

    hid_info_flags = HID_INFO_FLAG_REMOTE_WAKE_MSK | HID_INFO_FLAG_NORMALLY_CONNECTABLE_MSK;

    memset(&hids_init_obj, 0, sizeof(hids_init_obj));

    hids_init_obj.evt_handler                    = on_hids_evt;
    //hids_init_obj.error_handler                  = service_error_handler;
    hids_init_obj.is_kb                          = true;
    hids_init_obj.is_mouse                       = false;
    hids_init_obj.inp_rep_count                  = 2;
    hids_init_obj.p_inp_rep_array                = input_report_array;
    hids_init_obj.outp_rep_count                 = 1;
    hids_init_obj.p_outp_rep_array               = output_report_array;
    hids_init_obj.feature_rep_count              = 0;
    hids_init_obj.p_feature_rep_array            = NULL;
    hids_init_obj.rep_map.data_len               = sizeof(report_map_data);
    hids_init_obj.rep_map.p_data                 = (uint8_t*)report_map_data;
    hids_init_obj.hid_information.bcd_hid        = BASE_USB_HID_SPEC_VERSION;
    hids_init_obj.hid_information.b_country_code = 0;
    hids_init_obj.hid_information.flags          = hid_info_flags;
    hids_init_obj.included_services_count        = 0;
    hids_init_obj.p_included_services_array      = NULL;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.rep_map.security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.rep_map.security_mode.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.hid_information.security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.hid_information.security_mode.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(
        &hids_init_obj.security_mode_boot_kb_inp_rep.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_boot_kb_inp_rep.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.security_mode_boot_kb_inp_rep.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_boot_kb_outp_rep.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_boot_kb_outp_rep.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_protocol.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_protocol.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.security_mode_ctrl_point.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_ctrl_point.write_perm);

    err_code = ble_hids_init(p_hids, &hids_init_obj);
    APP_ERROR_CHECK(err_code);
}



static int player_contoller_on_ble(CLASS(PlayerController) *p, ble_evt_t * p_ble_evt)
{
    uint32_t err_code;
    ble_gatts_rw_authorize_reply_params_t auth_reply;
    if(p_instance != NULL) {
        switch (p_ble_evt->header.evt_id) {
            case BLE_GAP_EVT_CONNECTED:
                break;

            case BLE_EVT_TX_COMPLETE:
                // Send next key event
                (void) buffer_dequeue(p_instance, true);
                break;

            case BLE_GAP_EVT_DISCONNECTED:
                // Dequeue all keys without transmission.
                (void) buffer_dequeue(p_instance, false);

                break;

            case BLE_EVT_USER_MEM_REQUEST:
#if 1
                err_code = sd_ble_user_mem_reply(((ble_hids_t*)(p_instance->user_data))->conn_handle, NULL);
                APP_ERROR_CHECK(err_code);
#endif
                break;

            case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
#if 1
                if(p_ble_evt->evt.gatts_evt.params.authorize_request.type
                   != BLE_GATTS_AUTHORIZE_TYPE_INVALID) {
                    if ((p_ble_evt->evt.gatts_evt.params.authorize_request.request.write.op
                         == BLE_GATTS_OP_PREP_WRITE_REQ)
                        || (p_ble_evt->evt.gatts_evt.params.authorize_request.request.write.op
                            == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW)
                        || (p_ble_evt->evt.gatts_evt.params.authorize_request.request.write.op
                            == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL)) {
                        if (p_ble_evt->evt.gatts_evt.params.authorize_request.type
                            == BLE_GATTS_AUTHORIZE_TYPE_WRITE) {
                            auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                        } else {
                            auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                        }
                        auth_reply.params.write.gatt_status = BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2;
                        err_code = sd_ble_gatts_rw_authorize_reply(((ble_hids_t*)(p_instance->user_data))->conn_handle, &auth_reply);
                        APP_ERROR_CHECK(err_code);
                    }
                }
#endif
                break;

            case BLE_GATTC_EVT_TIMEOUT:
            case BLE_GATTS_EVT_TIMEOUT:
                break;
						
            default:
                // No implementation needed.
                break;
        }

        ble_hids_on_ble_evt(p_instance->user_data, p_ble_evt);
    }
		return err_code;
}


void media_player_send_charactor_a()
{

    keys_send(p_instance, sizeof(m_sample_key_a), (uint8_t*)m_sample_key_a);
    keys_send(p_instance, sizeof(m_sample_key_release), (uint8_t*)m_sample_key_release);
}


static int media_player_volume_up(CLASS(PlayerController) *p)
{

    (consumer_control_send(p, CONSUMER_CTRL_VOL_UP));
    (consumer_control_send(p, RELEASE_KEY));
		return TRUE;
}

static int media_player_volume_down(CLASS(PlayerController) *p)
{
    (consumer_control_send(p, CONSUMER_CTRL_VOL_DW));
    (consumer_control_send(p, RELEASE_KEY));
		return TRUE;
}

static int media_player_play_pause(CLASS(PlayerController) *p)
{
    (consumer_control_send(p, CONSUMER_CTRL_PLAY));
    (consumer_control_send(p, RELEASE_KEY));
		return TRUE;
}

static int media_player_next_track(CLASS(PlayerController) *p)
{
    (consumer_control_send(p, CONSUMER_CTRL_SCAN_NEXT_TRACK));
    (consumer_control_send(p, RELEASE_KEY));
		return TRUE;
}

static int media_player_prev_track(CLASS(PlayerController) *p)
{
    (consumer_control_send(p, CONSUMER_CTRL_SCAN_PREV_TRACK));
    (consumer_control_send(p, RELEASE_KEY));
		return TRUE;
}

static int media_player_ac_forward(CLASS(PlayerController) *p)
{
    (consumer_control_send(p, CONSUMER_CTRL_AC_FORWARD));
    (consumer_control_send(p, RELEASE_KEY));
		return TRUE;
}

static int media_player_ac_back(CLASS(PlayerController) *p)
{
    (consumer_control_send(p, CONSUMER_CTRL_AC_BACK));
    (consumer_control_send(p, RELEASE_KEY));
		return TRUE;
}

static int media_player_mute(CLASS(PlayerController) *p)
{
    (consumer_control_send(p, CONSUMER_CTRL_MUTE));
    (consumer_control_send(p, RELEASE_KEY));
		return TRUE;
}
static int media_player_stop(CLASS(PlayerController) *p)
{
    (consumer_control_send(p, CONSUMER_CTRL_STOP));
    (consumer_control_send(p, RELEASE_KEY));
		return TRUE;
}

/**@brief   Function to dequeue key scan patterns that could not be transmitted either completely of
 *          partially.
 *
 * @warning This handler is an example only. You need to analyze how you wish to send the key
 *          release.
 *
 * @param[in]  tx_flag   Indicative of whether the dequeue should result in transmission or not.
 * @note       A typical example when all keys are dequeued with transmission is when link is
 *             disconnected.
 *
 * @return     NRF_SUCCESS on success, else an error code indicating reason for failure.
 */
static uint32_t buffer_dequeue(CLASS(PlayerController) *p, bool tx_flag)
{
    buffer_entry_t * p_element;
    uint32_t         err_code = NRF_SUCCESS;
    uint16_t         actual_len = 0;

    if (BUFFER_LIST_EMPTY()) {
        err_code = NRF_ERROR_NOT_FOUND;
    } else {
        bool remove_element = true;

        p_element = &buffer_list.buffer[(buffer_list.rp)];

        if (tx_flag) {
            err_code = keys_send(p, p_element->data_len, p_element->p_data);
            // An additional notification is needed for release of all keys, therefore check
            // is for actual_len <= element->data_len and not actual_len < element->data_len
            if ((err_code == BLE_ERROR_NO_TX_BUFFERS) && (actual_len <= p_element->data_len)) {
                // Transmission could not be completed, do not remove the entry, adjust next data to
                // be transmitted
                p_element->data_offset = actual_len;
                remove_element         = false;
            }
        }

        if (remove_element) {
            BUFFER_ELEMENT_INIT(buffer_list.rp);

            buffer_list.rp++;
            buffer_list.count--;

            if (buffer_list.rp == MAX_BUFFER_ENTRIES) {
                buffer_list.rp = 0;
            }
        }
    }

    return err_code;
}


/**@brief Function for enqueuing key scan patterns that could not be transmitted either completely
 *        or partially.
 *
 * @warning This handler is an example only. You need to analyze how you wish to send the key
 *          release.
 *
 * @param[in]  p_hids         Identifies the service for which Key Notifications are buffered.
 * @param[in]  p_key_pattern  Pointer to key pattern.
 * @param[in]  pattern_len    Length of key pattern.
 * @param[in]  offset         Offset applied to Key Pattern when requesting a transmission on
 *                            dequeue, @ref buffer_dequeue.
 * @return     NRF_SUCCESS on success, else an error code indicating reason for failure.
 */
static uint32_t buffer_enqueue(ble_hids_t *            p_hids,
                               uint8_t *               p_key_pattern,
                               uint16_t                pattern_len,
                               uint16_t                offset)
{
    buffer_entry_t * element;
    uint32_t         err_code = NRF_SUCCESS;

    if (BUFFER_LIST_FULL()) {
        // Element cannot be buffered.
        err_code = NRF_ERROR_NO_MEM;
    } else {
        // Make entry of buffer element and copy data.
        element                 = &buffer_list.buffer[(buffer_list.wp)];
        element->p_instance     = p_hids;
        memcpy(element->p_data, p_key_pattern + offset, pattern_len - offset);
        //element->p_data         = p_key_pattern;
        element->data_offset    = offset;
        element->data_len       = pattern_len;

        buffer_list.count++;
        buffer_list.wp++;

        if (buffer_list.wp == MAX_BUFFER_ENTRIES) {
            buffer_list.wp = 0;
        }
    }

    return err_code;
}

/**@brief This function will send sample key presses to the peer.
 *
 * @param[in]   key_pattern_len   Pattern length.
 * @param[in]   p_key_pattern     Pattern to be sent.
 */
static uint32_t keys_send(CLASS(PlayerController) *p, uint8_t key_pattern_len,  uint8_t * p_key_pattern)
{
    uint32_t err_code;

    if (m_in_boot_mode) {
        err_code = ble_hids_boot_kb_inp_rep_send((ble_hids_t*)(p->user_data),
                   key_pattern_len,
                   p_key_pattern);
        if (err_code == NRF_SUCCESS) {
            err_code = ble_hids_boot_kb_inp_rep_send((ble_hids_t*)(p->user_data),
                       key_pattern_len,
                       (uint8_t*)m_sample_key_release_pattern);
        }
    } else {
        err_code = ble_hids_inp_rep_send((ble_hids_t*)(p->user_data),
                                         INPUT_REPORT_KEYS_INDEX,
                                         key_pattern_len,
                                         p_key_pattern);
        if (err_code == NRF_SUCCESS) {
            err_code = ble_hids_inp_rep_send((ble_hids_t*)(p->user_data),
                                             INPUT_REPORT_KEYS_INDEX,
                                             key_pattern_len,
                                             (uint8_t*)m_sample_key_release_pattern);
        }
    }

    // An additional notification is needed for release of all keys, therefore check
    // is for actual_len <= key_pattern_len and not actual_len < key_pattern_len.
    if ((err_code == BLE_ERROR_NO_TX_BUFFERS)) {
        // Buffer enqueue routine return value is not intentionally checked.
        // Rationale: Its better to have a a few keys missing than have a system
        // reset. Recommendation is to work out most optimal value for
        // MAX_BUFFER_ENTRIES to minimize chances of buffer queue full condition
        UNUSED_VARIABLE(buffer_enqueue((ble_hids_t*)(p->user_data), p_key_pattern, key_pattern_len, 0));
    }

    if (
        (err_code != NRF_SUCCESS)
        &&
        (err_code != NRF_ERROR_INVALID_STATE)
        &&
        (err_code != BLE_ERROR_NO_TX_BUFFERS)
        &&
        (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
    ) {
        APP_ERROR_HANDLER(err_code);
    }
    return err_code;
}



#if 1
/**@brief HID Report Characteristic Write event handler.
 *
 * @param[in]   p_evt   HID service event.
 */
static void on_hid_rep_char_write(CLASS(PlayerController) *p, ble_hids_evt_t *p_evt)
{
    if (p_evt->params.char_write.char_id.rep_type == BLE_HIDS_REP_TYPE_OUTPUT) {
        uint32_t err_code;
        uint8_t  report_val;
        uint8_t  report_index = p_evt->params.char_write.char_id.rep_index;

        if (report_index == OUTPUT_REPORT_INDEX) {
            APP_ERROR_CHECK_BOOL(OUTPUT_REPORT_MAX_LEN == 1);

            err_code = ble_hids_outp_rep_get((ble_hids_t*)(p->user_data),
                                             report_index,
                                             OUTPUT_REPORT_MAX_LEN,
                                             0,
                                             &report_val);
            APP_ERROR_CHECK(err_code);

            if ((report_val & OUTPUT_REPORT_BIT_MASK_CAPS_LOCK) != 0) {
                // Caps Lock On
                //nrf_gpio_pin_set(CAPS_ON_LED_PIN_NO);
                keys_send(p, sizeof(m_caps_on_key_pattern), (uint8_t*)m_caps_on_key_pattern);
            } else {
                // Caps Lock Off
                // nrf_gpio_pin_clear(CAPS_ON_LED_PIN_NO);
                keys_send(p, sizeof(m_caps_off_key_pattern), (uint8_t*)m_caps_off_key_pattern);
            }
        }
    }
}
#endif

/**@brief Function for sending consumer control data
 */
static uint32_t consumer_control_send(CLASS(PlayerController) *p, consumer_control_t cmd)
{
    if(p == NULL) {
        return NRF_SUCCESS;
    }
    uint32_t err_code = ble_hids_inp_rep_send(p->user_data, INPUT_CCONTROL_KEYS_INDEX, INPUT_CC_REPORT_KEYS_MAX_LEN, (uint8_t*)&cmd);
    if ((err_code == BLE_ERROR_NO_TX_BUFFERS)) {
        // Buffer enqueue routine return value is not intentionally checked.
        // Rationale: Its better to have a a few keys missing than have a system
        // reset. Recommendation is to work out most optimal value for
        // MAX_BUFFER_ENTRIES to minimize chances of buffer queue full condition
        UNUSED_VARIABLE(buffer_enqueue((ble_hids_t*)(p->user_data), (uint8_t*)&cmd, INPUT_CC_REPORT_KEYS_MAX_LEN, 0));
    }
		return err_code;
}
#if 0
static uint32_t hid_kbd_send_string(CLASS(PlayerController) *p)
{
    uint8_t temp_buf[INPUT_REPORT_KEYS_MAX_LEN] = {0x00, 0x00, 0xb, 0xc, 0x2c, 0x0, 0x0, 0x0};
    return ble_hids_inp_rep_send(p->user_data, INPUT_REPORT_KEYS_INDEX, INPUT_REPORT_KEYS_MAX_LEN, (uint8_t*)temp_buf);
}

static uint32_t hid_kbd_send_release(CLASS(PlayerController) *p)
{
    uint8_t temp_buf[INPUT_REPORT_KEYS_MAX_LEN] = {0, 0, 0, 0, 0, 0, 0, 0};
    return ble_hids_inp_rep_send(p->user_data, INPUT_REPORT_KEYS_INDEX, INPUT_REPORT_KEYS_MAX_LEN, (uint8_t*)temp_buf);
}
#endif
