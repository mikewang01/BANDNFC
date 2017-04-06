/********************************************************************************

 **** Copyright (C), 2016, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : PlayerController.h
 * Author        : MikeWang
 * Date          : 2016-2-1
 * Description   : uicotouch.c header file
 * Version       : 1.0
 * Function List :
 *
 * Record        :
 * 1.Date        : 2016-2-1
 *   Author      : MikeWang
 *   Modification: Created file

*************************************************************************************************************/

#ifndef __PLAYER_CONTROLLER__
#define __PLAYER_CONTROLLER__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include "stdint.h"
#include "stdio.h"
#include "oop_hal.h"
#include "standards.h"
#include "main.h"

#ifdef  __PLAYER_CONTROLLER_ENABLE__
#undef  __PLAYER_CONTROLLER_ENABLE__
#endif
	
#define __PLAYER_CONTROLLER_ENABLE__

    /** An interface for the PlayerController to take media loacated in host device under control 
     *
     * @code
     * //Uses read the touch or swipe inforamtion through the i2c protocol and prococess touch interupt
     *
     * #include "PlayerController.h"
     *
     * CLASS(PlayerController) *media_ctl_obj = NULL;
     *
     * int main() {
     *			 media_ctl_obj = PlayerController_get_instance;
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
    DEF_CLASS(PlayerController)
		int (*play_pause) 			(CLASS(PlayerController) *arg);				/*media player start pause controller*/
		int (*volume_up) 				(CLASS(PlayerController) *arg);				 /*volume up*/
		int (*volume_down) 				(CLASS(PlayerController) *arg);				/*volume down*/
		int (*next_track) 				(CLASS(PlayerController) *arg);				/*switch to next track*/
		int (*prev_track) 				(CLASS(PlayerController) *arg);				/*switch to previos track*/
		int (*audio_forward) 			(CLASS(PlayerController) *arg);				/*forward the process of music*/
		int (*audio_back) 				(CLASS(PlayerController) *arg);				/*pull back the process of music*/
		int (*mute) 							(CLASS(PlayerController) *arg);				/*mute  device*/
		int (*stop) 							(CLASS(PlayerController) *arg);				/*stop playing*/
		int (*on_ble) 						(CLASS(PlayerController) *arg, ble_evt_t * p_ble_evt);				/* event recieving method */
    void *user_data;/*point to user private data*/
    END_DEF_CLASS(PlayerController)
    CLASS(PlayerController)* PlayerController_get_instance(void);

/*===========================PlayerController HARDWARE DRIVER porting ==================================*/
/*============================================================================================================*/
#define __LOG_DBG Y_SPRINTF

/*****************************************************************************
 * Function      : PlayerController_get_instance
 * Description   : get single instance method function
 * Input         : void
 * Output        : void
 * Return        : player contoller pointer
*****************************************************************************/
CLASS(PlayerController)* PlayerController_get_instance(void);
/*=============================================================================*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__PlayerController_H__*/
