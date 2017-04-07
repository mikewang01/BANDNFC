/**
 * \addtogroup BSP
 * \{
 * \addtogroup OSAL
 * \{
 * \addtogroup RESOURCE_MANAGEMENT
 * 
 * \brief OSAL resource management
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file resmgmt.h
 *
 * @brief Resource management API
 *
 * Copyright (C) 2015. Dialog Semiconductor Ltd, unpublished work. This computer
 * program includes Confidential, Proprietary Information and is a Trade Secret of
 * Dialog Semiconductor Ltd. All use, disclosure, and/or reproduction is prohibited
 * unless authorized in writing. All Rights Reserved.
 *
 * <black.orca.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

#ifndef RESMGMT_H_
#define RESMGMT_H_

#include <stdint.h>
//#include <osal.h>

/**
 * \brief Shared resource ids
 *
 * \sa RES_MASK
 *
 */
typedef enum {
        RES_ID_UART1 = 0,
        RES_ID_UART2,
        RES_ID_SPI1,
        RES_ID_SPI2,
        RES_ID_I2C1,
        RES_ID_I2C2,
        RES_ID_QSPI,
        RES_ID_TIMER0,
        RES_ID_TIMER1,
        RES_ID_TIMER2,
        RES_ID_DMA_MUX01,
        RES_ID_DMA_MUX23,
        RES_ID_DMA_MUX45,
        RES_ID_DMA_MUX67,
        RES_ID_GPADC,
        RES_ID_COUNT
} RES_ID;

/**
 * \brief Make resource mask from ID
 *
 * Use this macro to prepare argument for resource_acquire().
 *
 * \param [in] id value from RES_IS enum
 *
 * \sa resource_acquire
 *
 */
#define RES_MASK(id) (1 << (id))

/**
 * \brief Constant to use when resource_acquire() should wait till resource is available
 *
 */
#define RES_WAIT_FOREVER 0

/**
 * \brief Initialize resource management structures
 *
 * Function allocates internal structure so resource allocation can be done
 * on OS level.
 * This function must be called before any calls to resource_acquire(), resource_release().
 *
 * \sa resource_acquire
 * \sa resource_release
 *
 */
void resource_init(void);

/**
 * \brief Acquire resource(s)
 *
 * Function acquires resource(s) so they can be accessed exclusively.
 *
 * \param [in] resource_mask bit mask of requested resource
 *             it can have single resource:
 *                RES_MASK(RES_ID_UART1)
 *             or group of them:
 *                RES_MASK(RES_ID_UART1) | RES_MASK(RES_ID_SPI2) | RES_MASK(RES_ID_I2C1)
 * \param [in] timeout how long to wait for resources to be available
 *             0 - no wait take resource if is available
 *             RES_WAIT_FOREVER - wait till all resources are available
 *             other value specifies how many ticks to wait for resources
 *
 * \return \p resource_mask on success, 0 on timeout
 *
 * \sa resource_release
 *
 */
uint32_t resource_acquire(uint32_t resource_mask, uint32_t timeout);

/**
 * \brief Release resource(s)
 *
 * Function releases resources so they can be used by other tasks.
 * If there is task waiting for resources just released it will be
 * scheduled to run (provided that all requested resources are free).
 *
 * It's possible to acquire resources in one resource_acquire() call,
 * and then release them separately.
 *   resource_acquire(RES_MASK(RES_ID_UART1) | RES_MASK(RES_ID_SPI2);
 * ...
 *   resource_release(RES_MASK(RES_ID_UART1);
 *   resource_release(RES_MASK(RES_ID_SPI2);
 *
 * \param [in] resource_mask bit mask of released resources
 *             it can have single resource:
 *                RES_MASK(RES_ID_UART1)
 *             or group of them:
 *                RES_MASK(RES_ID_UART1) | RES_MASK(RES_ID_SPI2) | RES_MASK(RES_ID_I2C1)
 *
 * \sa resource_acquire
 *
 */
void resource_release(uint32_t resource_mask);

#ifndef CONFIG_NO_DYNAMIC_RESOURCE_ID

/**
 * \brief Add resource at run time
 *
 * Resources that can be acquired are defined in RES_ID enum. This creates pool of resources at
 * compile time.
 * It allows to add resources to extend this list at run time. This does not
 * affects resource management functions, it just allows to create resource ids in a safe way
 * without changing enum in this file.
 * When all resources needed by the user are identified this feature can be switch off and all
 * dynamic resource ids can be put in enum.
 *
 */
int resource_add(void);

#endif

#endif /* RESMGMT_H_ */

/**
 * \}
 * \}
 * \}
 */
