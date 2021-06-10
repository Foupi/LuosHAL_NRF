/******************************************************************************
 * @file luosHAL
 * @brief Luos Hardware Abstration Layer. Describe Low layer fonction
 * @MCU Family NRF52832 PCA10040
 * @author Pierre Foucart
 * @version 0.0.1
 ******************************************************************************/
#ifndef _LUOSHAL_H_
#define _LUOSHAL_H_

#include <stdint.h>
#include <luos_hal_config.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
// FIXME Define Luos UUID
#define LUOS_UUID ((uint32_t *)0x0000000000)

#define ADDRESS_ALIASES_FLASH ADDRESS_LAST_PAGE_FLASH
#define ADDRESS_BOOT_FLAG_FLASH (ADDRESS_LAST_PAGE_FLASH + PAGE_SIZE) - 4
/*******************************************************************************
 * Function
 ******************************************************************************/
void LuosHAL_Init(void);
void LuosHAL_SetIrqState(uint8_t Enable);
uint8_t LuosHAL_GetPTPState(uint8_t PTPNbr);
void LuosHAL_ComputeCRC(uint8_t *data, uint8_t *crc);

// luos_hal_com_*.c
void LuosHAL_ComInit(uint32_t Baudrate);
void LuosHAL_SetTxState(uint8_t Enable);
void LuosHAL_SetRxState(uint8_t Enable);
uint8_t LuosHAL_ComTransmit(uint8_t *data, uint16_t size);
void LuosHAL_SetTxLockDetecState(uint8_t Enable);
uint8_t LuosHAL_GetTxLockState(void);
void LuosHAL_SetRxDetecPin(uint8_t Enable);
void LuosHAL_ComTxComplete(void);

// luos_hal_flash.c
void LuosHAL_FlashWriteLuosMemoryInfo(uint32_t addr, uint16_t size, uint8_t *data);
void LuosHAL_FlashReadLuosMemoryInfo(uint32_t addr, uint16_t size, uint8_t *data);

// luos_hal_ptp_*.c
void LuosHAL_SetPTPDefaultState(uint8_t PTPNbr);
void LuosHAL_SetPTPReverseState(uint8_t PTPNbr);
void LuosHAL_PushPTP(uint8_t PTPNbr);

// luos_hal_systick.c
uint32_t LuosHAL_GetSystick(void);

// luos_hal_timer_*.c
void LuosHAL_ResetTimeout(uint16_t nbrbit);

#endif /* _HAL_H_ */
