/********************************************************************************

 **** Copyright (C), 2015, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : wrist_detect.h
 * Author        : MikeWang
 * Date          : 2016-03-28
 * Description   : wrist_detect.c header file
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2016-03-28
 *   Author      : MikeWang
 *   Modification: Created file

*************************************************************************************************************/

#ifndef __WRIST_DETECT_H__
#define __WRIST_DETECT_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "stdint.h"
#include "oop_hal.h"
//#define __WIRST_DETECT_ENABLE__

    /** An interface for the wrist_detect to take media loacated in host device under control 
     *
     * @code
     * //Uses read the touch or swipe inforamtion through the i2c protocol and prococess touch interupt
     *
     * #include "wrist_detect.h"
     *
     * CLASS(wrist_detect) *media_ctl_obj = NULL;
     *
     * int main() {
     *			 media_ctl_obj = wrist_detect_get_instance;
     *     while(1) {
		 *				if(conditon){	
     *					touch_onj->isr_process(touch_ist);
		 *				}
     *     }
     *
     * }
		 * void ble_dispatch(ble_evt_t * p_ble_evt){
		 *		
		 *		touch_obj->on_ble(touch_obj, p_ble_evt);
		 *}
     * @endcode
     */

    /*object prototype declaration*/
    DEF_CLASS(wrist_detect)
		int (*gravity_obtain) 	   										(CLASS(wrist_detect) *arg, int16_t x, int16_t y, int16_t z, uint32_t time_stamp);				/*mute  device*/
		int (*screen_wakeup_callback_register) 				(CLASS(wrist_detect) *arg, int (*p_func)());				/*stop playing*/
    void *user_data;/*point to user private data*/
    END_DEF_CLASS(wrist_detect)
    CLASS(wrist_detect)* wrist_detect_get_instance(void);

/*================================CONFIGRATION===============================================*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
enum dev_orientation{
		DEV_HORIZON = 1,
		DEV_VERTICAL_90,
		DEV_VERTICAL_270,
};

#define GET_CURRENT_DEV_ORIENTATION()  (cling.ui.clock_orientation)

#endif /* __WRIST_DETECT_H__ */
