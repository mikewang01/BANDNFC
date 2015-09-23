/******************** (C) COPYRIGHT 2012 STMicroelectronics ********************
* File Name          : ek3dh.h
* Author             : MSH Application Team
* Version            : $Revision:$
* Date               : $Date:$
* Description        : Descriptor Header for ek3dh file
* HISTORY:
* Date        | Modification                                  | Author
* 17/02/2012  | Initial Revision                              | Andrea Labombarda

********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* THIS SOFTWARE IS SPECIFICALLY DESIGNED FOR EXCLUSIVE USE WITH ST PARTS.
*
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __EK3DH_H
#define __EK3DH_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define BIT(x) ( 1<<(x) )

#define VER                           0
#define DEV                           1
#define ZOFF                          2
#define ZON                           4
#define START                         5
#define STOP                          6
#define R                             7
#define W                             8
#define DEBUG                         9
// FIFO CONFIGURATIONS //
#define FIFO_BYPASS                   10
#define FIFO_STREAM                   11
#define FIFO_MODE	              12
#define FIFO_STOP_TRIG	              13
#define FIFO_READ	              14
#define FIFO_RUN	              15
#define LIST                          16
#define SINGLE                        17
#define ECHOON                        19
#define ECHOOFF                       20
#define LISTDEV                       21
#define DBRESET                       22


#define STATUS_REG                    0x27
#define OUT_X_L                       0x28  
#define OUT_X_H                       0x29   
#define OUT_Y_L                       0x2a
#define OUT_Y_H                       0x2b
#define OUT_Z_L                       0x2c
#define OUT_Z_H                       0x2d

#define STATUS_REG_ZYXDA              3

#define DATAREADY_BIT                 STATUS_REG_ZYXDA

#define FIFO_CONTROL                  0x2E
#define FIFO_CONTROL_TRIG             BIT(5)
#define FIFO_CONTROL_WTMSAMP          (BIT(4) | BIT(3) | BIT(2) | BIT(1) | BIT(0))

#define DEF_FIFO_CTRL_BYPASS          0x00
#define DEF_FIFO_CTRL_FIFO_MODE       BIT(6)
#define DEF_FIFO_CTRL_STREAM          BIT(7)
#define DEF_FIFO_CTRL_STOPTRIG        (BIT(7) | BIT(6))

#define FIFO_SOURCE                   0x2F
#define FIFO_SOURCE_SAMPLES           (BIT(4) | BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define FIFO_SOURCE_EMPTY             BIT(5)
#define FIFO_SOURCE_OVRN              BIT(6)
#define FIFO_SOURCE_WTM               BIT(7)

#define CTRL_REG1                     0x20 

#define CTRL_REG5                     0x24
#define CTRL_REG5_FIFO_EN             BIT(6)

//		INT1_SRC
#define INT1_SRC                      0x31
#define INT1_SRC_IA                   BIT(6)

//		INT2_SRC

#define INT2_SRC                      0x35
#define INT2_SRC_IA                   BIT(6)

#define CLICK_SRC                     0x39
#define CLICK_SRC_IA                  BIT(6)
/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
void ek3dh_AppTick(void);

#endif /* __EK3DH_H */

/******************* (C) COPYRIGHT 2012 STMicroelectronics *****END OF FILE****/
