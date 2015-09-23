//
//  File: usb_api.h
//  
//  Description: the USB process header
//
//  Created on July 6, 2012
//
#ifndef __USB_API_HEADER__
#define __USB_API_HEADER__

#include "gcp_api.h"

typedef struct tagUSB_API_CTX {
	I8U conn_stat; // Connection state
} USB_API_CTX;

void USB_api_init(void);
void USB_state_machine_update(void);
BOOLEAN USB_is_connected(void);
BOOLEAN USB_vbus_present(void);
void USB_enter_BSL_mode(void);

#endif  // __USB_API_HEADER__
