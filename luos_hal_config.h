/******************************************************************************
 * @file luosHAL_Config
 * @brief This file allow you to configure LuosHAL according to your design
 *        this is the default configuration created by Luos team for this MCU Family
 *        Do not modify this file if you want to ovewrite change define in you project
 * @MCU Family NRF52832 PCA10040
 * @author Pierre Foucart
 * @version 0.0.1
 ******************************************************************************/
#ifndef _LUOSHAL_CONFIG_H_
#define _LUOSHAL_CONFIG_H_

//include file relative to your MCU family

#define DISABLE 0x00
/*******************************************************************************
 * PINOUT CONFIG
 ******************************************************************************/
#ifndef PORT_CLOCK_ENABLE
#define PORT_CLOCK_ENABLE() do { \
                            \
                            } while(0U)
#endif

// Max amount of milliseconds the systick can wait.
#ifndef MAX_SYSTICK_MS_VAL
#define MAX_SYSTICK_MS_VAL 250
#endif

#include "luos_hal_flash_config.h"
#include "luos_hal_timer_config.h"
#include "luos_hal_ptp_config.h"
#include "luos_hal_com_config.h"

#endif /* _LUOSHAL_CONFIG_H_ */
