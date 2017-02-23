/********************************************************************************

 **** Copyright (C), 2015, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : wrist_detect.c
 * Author        : MikeWang
 * Date          : 2016-03-28
 * Description   : wrist_detect.c source file
 * Version       : 1.0
 * Function List :
 *
 * Record        :
 * 1.Date        : 2016-03-28
 *   Author      : MikeWang
 *   Modification: Created file

*************************************************************************************************************/
#include  "mics_function.h"
#include 	"stdint.h"
#include  "standards.h"
#include  "string.h"
#include  "wrist_detect.h"
#include 	"main.h"

#define _WRIST_DETECTION_LOG N_SPRINTF

#ifdef __WIRST_DETECT_ENABLE__
/*====================MACROS===================================================================================*/
#define __DEVICE_INMOTION_THREAHOLD__  (2200)

/*for diffrent way of wearing x axix are actaully mirrored*/
#define __X_AXIS_THREAHOLD_LOW__		   (-500)
#define __X_AXIS_THREAHOLD_HIGH__		   (500)

/*this is for the orientation when you ware device with button outside*/
#define __Y_AXIS_THREAHOLD_LOW__		   (000)
#define __Y_AXIS_THREAHOLD_HIGH__		   (1400)

/*this is for the orientation when you ware device with button outside*/
#define __Y_AXIS_THREAHOLD_LOW_90_		   (-1400)
#define __Y_AXIS_THREAHOLD_HIGH_90_		   (0)


#define __Z_AXIS_THREAHOLD_LOW__		   (-2000)
#define __Z_AXIS_THREAHOLD_HIGH__		   (-1200)

#define __GRAVITY_CHANGE_THREAHOLD__ 2000 //MS
#define __TIMESTAMP_THREAHOLD__ 300 //MS

#define _ACC_FIFO_LENTH_   16
#define _BOX_FILTER_DEPTH_ 8
#define __ACC_AXIS_NUM__   3

/*====================LOCAL VARIES===================================================================================*/
static struct acc_fifo acc_local_fifo;
static struct box_filter_buffer_16bits  filter_x;
static struct box_filter_buffer_16bits  filter_y;
static struct box_filter_buffer_16bits  filter_z;
static  CLASS(wrist_detect) *p_this = NULL;
static int (*wake_up_callback)() = NULL;
static int wrist_wakeup_callback_register(CLASS(wrist_detect) *arg, int (*p_func)());
static int acc_data(CLASS(wrist_detect) *arg, int16_t x, int16_t y, int16_t z, uint32_t time_stamp);
/*========================typedef==========================================================================================*/
struct acc_element {
    int16_t 	gravity_amp_x;
    int16_t 	gravity_amp_y;
    int16_t 	gravity_amp_z;
    uint32_t  time_stamp;
};

struct acc_fifo {
    struct acc_element  data[_ACC_FIFO_LENTH_];
    struct acc_element  *cur_pos;
    struct acc_element  *read_pos;
};
/*====================fifo realed operation==================================================================================*/
/*****************************************************************************
 * Function      : static inline int acc_fifo_pop(struct acc_fifo *p, struct acc_element *p_element)
 * Description   : fifo pop function
 * Input         : CLASS(PlayerController) *p object pointer, struct acc_element *p_element,element pointer
 * Output        : None
 * Return        : TRUE,SUCESS; FALSE, fifo que has been full
 * Others        :
 * Record
 * 1.Date        : 20163028
 *   Author      : MikeWang
 *   Modification: Created function

*****************************************************************************/
static inline int acc_fifo_pop(struct acc_fifo *p, struct acc_element *p_element)
{
    *p_element = *(acc_local_fifo.read_pos);
    acc_local_fifo.read_pos++;
    if(acc_local_fifo.read_pos > acc_local_fifo.data + _ACC_FIFO_LENTH_ - 1) {
        acc_local_fifo.read_pos = acc_local_fifo.data;
    }
    /*fifo has been empty*/
    if(acc_local_fifo.read_pos == acc_local_fifo.cur_pos) {
        return FALSE;
    }
    return TRUE;

}
/*****************************************************************************
 * Function      : static inline int acc_fifo_push(struct acc_fifo *p, struct acc_element element)
 * Description   : used to tell if this device has reach expected positon
 * Input         : CLASS(PlayerController) *p object pointer
 * Output        : None
 * Return        : TRUE,SUCESS; FALSE,FAILED
 * Others        :
 * Record
 * 1.Date        : 20163028
 *   Author      : MikeWang
 *   Modification: Created function

*****************************************************************************/
static inline int acc_fifo_push(struct acc_fifo *p, struct acc_element element)
{
    *p->cur_pos = element;
    p->cur_pos++;
    /**/
    if(acc_local_fifo.cur_pos > acc_local_fifo.data + _ACC_FIFO_LENTH_ - 1) {
        acc_local_fifo.cur_pos = acc_local_fifo.data;
    }
    /*fifo has been full*/
    if(acc_local_fifo.read_pos == acc_local_fifo.cur_pos) {
        return FALSE;
    }
    return TRUE;
}

/*****************************************************************************
 * Function      : int position_recongize(int16_t x, int16_t y, int16_t z, int16_t t)
 * Description   : used to tell if this device has reach expected positon
 * Input         : CLASS(PlayerController) *p object pointer
 * Output        : None
 * Return        : TRUE,SUCESS; FALSE,FAILED
 * Others        :
 * Record
 * 1.Date        : 20163028
 *   Author      : MikeWang
 *   Modification: Created function

*****************************************************************************/
static int position_recongize(int16_t x, int16_t y, int16_t z, int32_t t)
{

    if(t < __DEVICE_INMOTION_THREAHOLD__ * __DEVICE_INMOTION_THREAHOLD__) {
        if((x > __X_AXIS_THREAHOLD_LOW__) && (x < __X_AXIS_THREAHOLD_HIGH__)) {
            N_SPRINTF("x satisfied");
            enum dev_orientation cur_orientation = (enum dev_orientation)GET_CURRENT_DEV_ORIENTATION();
            if(cur_orientation == DEV_VERTICAL_90 || cur_orientation == DEV_HORIZON) {
                if((y > __Y_AXIS_THREAHOLD_LOW_90_) && (y < __Y_AXIS_THREAHOLD_HIGH_90_)) {
                    N_SPRINTF("[WD] __Y_AXIS_THREAHOLD_270_ satisfied");
                    // DATA_LOG("y satisfied %d", i++);
                    if((z < __Z_AXIS_THREAHOLD_HIGH__) && (z > __Z_AXIS_THREAHOLD_LOW__)) {
                        //	Y_SPRINT0F("z satisfied");
                        // DATA_LOG("acc x_ai x = %f, y = %f, z = %f time_eclapses =%f", x,y,z,time_eclapse.elapsed());
                        // DATA_LOG("device in right positon %d", i++);
                        return TRUE;
                    }
                }
            } else {
                if((y > __Y_AXIS_THREAHOLD_LOW__) && (y < __Y_AXIS_THREAHOLD_HIGH__)) {
                    N_SPRINTF("__Y_AXIS_THREAHOLD_90_ y satisfied");
                    // DATA_LOG("y satisfied %d", i++);
                    if((z < __Z_AXIS_THREAHOLD_HIGH__) && (z > __Z_AXIS_THREAHOLD_LOW__)) {
                        //	_WRIST_DETECTION_LOG("z satisfied");
                        // DATA_LOG("acc x_ai x = %f, y = %f, z = %f time_eclapses =%f", x,y,z,time_eclapse.elapsed());
                        // DATA_LOG("device in right positon %d", i++);
                        return TRUE;
                    }
                }

            }


        }
        //	_WRIST_DETECTION_LOG("[wrist_detect] not in motion");
    } else {
        //	_WRIST_DETECTION_LOG("[wrist_detect] in motion");
    }
    return FALSE;
}

/*****************************************************************************
 * Function      : int position_recongize(int16_t x, int16_t y, int16_t z, int16_t t)
 * Description   : used to tell if this device has reach expected positon
 * Input         : CLASS(PlayerController) *p object pointer
 * Output        : None
 * Return        : TRUE,SUCESS; FALSE,FAILED
 * Others        :
 * Record
 * 1.Date        : 20163028
 *   Author      : MikeWang
 *   Modification: Created function

*****************************************************************************/
static int wrist_detect_init(CLASS(wrist_detect) *arg)
{
    if(arg == NULL) {
        return FALSE;
    }
    static int16_t buffer[__ACC_AXIS_NUM__][_BOX_FILTER_DEPTH_];
    struct box_filter_buffer_16bits *t[__ACC_AXIS_NUM__] = {&filter_x, &filter_y, &filter_z};
    /*initialte box filter parameter*/
    for(uint8_t i = 0; i < __ACC_AXIS_NUM__; i++) {
        memset(t[i], 0, _BOX_FILTER_DEPTH_);
        t[i]->buffer = buffer[i];
        t[i]->lenth  = _BOX_FILTER_DEPTH_;
        t[i]->pos  = 0;
    }
    /*initilate cordinate fifo*/
    memset(&acc_local_fifo, 0, sizeof(struct acc_fifo));
    /*initilate current pointer to the entrance of  data line*/
    acc_local_fifo.cur_pos = acc_local_fifo.data;
    acc_local_fifo.read_pos = acc_local_fifo.cur_pos;
    /*initalte mobject method*/
    arg->gravity_obtain = acc_data;
    arg->screen_wakeup_callback_register = wrist_wakeup_callback_register;
    return TRUE;
}

/*****************************************************************************
 * Function      : int acc_data(int16_t x, int16_t y, int16_t z, int16_t t)
 * Description   : used to tell if this device has reach expected positon
 * Input         : CLASS(PlayerController) *p object pointer
 * Output        : None
 * Return        : TRUE,SUCESS; FALSE,FAILED
 * Others        :
 * Record
 * 1.Date        : 20163028
 *   Author      : MikeWang
 *   Modification: Created function

*****************************************************************************/
static int acc_data(CLASS(wrist_detect) *arg, int16_t x, int16_t y, int16_t z, uint32_t time_stamp)
{
    if(arg == NULL) {
        return FALSE;
    }
    /*start  box filter for each axis*/
    x = box_filter_16bits(&filter_x, x);
    y = box_filter_16bits(&filter_y, y);
    z = box_filter_16bits(&filter_z, z);

    struct acc_element t;
    /*if device finalaly stay in expected postion then check rotatoion speed*/
    if(position_recongize(x, y, z, x * x + y * y + z * z) == TRUE) {
        _WRIST_DETECTION_LOG("[wrist_detect] device in position");
        for(int i = acc_fifo_pop(&acc_local_fifo, &t); i == TRUE; i = acc_fifo_pop(&acc_local_fifo, &t)) {
            _WRIST_DETECTION_LOG("[wrist_detect] amp   = %d time  = %d", t.gravity_amp_y, t.time_stamp);
            /*this is the conditon which has the hisghest possibility that customer do a rotation wakeup*/
            if((MISC_ABS(t.gravity_amp_y - y) > __GRAVITY_CHANGE_THREAHOLD__)) {
                _WRIST_DETECTION_LOG("[wrist_detect] amp threhold satisfied");
                if((time_stamp - t.time_stamp) < __TIMESTAMP_THREAHOLD__) {
                    _WRIST_DETECTION_LOG("[wrist_detect] timestamp threhold satisfied");
                    /*turn on screen right now*/
                    if(wake_up_callback != NULL) {
                        wake_up_callback();
                    }
                    break;
                }
            } else if(((MISC_ABS(t.gravity_amp_y - y) > 1000) && MISC_ABS(t.gravity_amp_z - z) > 2000)) {
                if((MISC_ABS(t.gravity_amp_y - y) > __GRAVITY_CHANGE_THREAHOLD__)) {
                    /*turn on screen right now*/
                    if(wake_up_callback != NULL) {
                        wake_up_callback();
                    }
                    break;
                }
            }
        }
    }
    /*push latest data into fifo*/
    /*move fifo forward one element*/
    //memcpy(acc_local_fifo.data, acc_local_fifo.data + 1, (sizeof(acc_local_fifo.data) - sizeof(acc_local_fifo.data[0])));
    //t = acc_local_fifo.data + _ACC_FIFO_LENTH_ - 1;
    t.gravity_amp_x = x;
    t.gravity_amp_y = y;
    t.gravity_amp_z = z;
    t.time_stamp = time_stamp;
    acc_fifo_push(&acc_local_fifo, t);
    return TRUE;
}
/*****************************************************************************
 * Function      : CLASS(wrist_detect)* wrist_detect_get_instance(void)
 * Description   : get one instance
 * Input         : None
 * Output        : None
 * Return        : instance pointer
 * Others        :
 * Record
 * 1.Date        : 20163028
 *   Author      : MikeWang
 *   Modification: Created function

*****************************************************************************/
CLASS(wrist_detect)* wrist_detect_get_instance(void)
{
    static CLASS(wrist_detect) t;
    if(p_this == NULL) {
        p_this = &t;
        wrist_detect_init(p_this);
    }
    return p_this;
}
/*****************************************************************************
 * Function      : int* wrist_wakeup_callback_register(void)
 * Description   : register_callback_function
 * Input         : None
 * Output        : None
 * Return        : instance pointer
 * Others        :
 * Record
 * 1.Date        : 20163028
 *   Author      : MikeWang
 *   Modification: Created function

*****************************************************************************/
static int wrist_wakeup_callback_register(CLASS(wrist_detect) *arg, int (*p_func)())
{
    if(arg == NULL) {
        return FALSE;
    }
    if(p_func != NULL) {
        wake_up_callback = p_func;
    }
    return TRUE;
}
#endif

