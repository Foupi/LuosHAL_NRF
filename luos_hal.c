/******************************************************************************
 * @file luosHAL
 * @brief Luos Hardware Abstration Layer. Describe Low layer fonction 
 * @MCU Family NRF52832 PCA10040
 * @author Pierre Foucart
 * @version 0.0.1
 ******************************************************************************/
#include "luos_hal.h"
#include "luos_hal_ble.h"
#include "luos_hal_board.h"
#include "luos_hal_flash.h"
#include "luos_hal_ptp.h"
#include "luos_hal_systick.h"

#include <stdbool.h>
#include <string.h>

#include "reception.h"
#include "context.h"
#include "msg_alloc.h"

//MCU dependencies this HAL is for family NRF52832 PCA10040 you can find
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/
static void LuosHAL_CRCInit(void);

/////////////////////////Luos Library Needed function///////////////////////////

/******************************************************************************
 * @brief Luos HAL general initialisation
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_Init(void)
{
    // Board Initialization
    LuosHAL_BoardInit();

    //Systick Initialization
    LuosHAL_SystickInit();

    // BLE Initialization
    LuosHAL_BleInit();

    //IO Initialization
    LuosHAL_GPIOInit();

    // Flash Initialization
    LuosHAL_FlashInit();

    // BLE Setup and connection
    LuosHAL_BleSetup();
    LuosHAL_BleConnect();

    // CRC Initialization
    LuosHAL_CRCInit();

    //Com Initialization
    LuosHAL_ComInit(DEFAULTBAUDRATE);
}
/******************************************************************************
 * @brief Luos HAL general disable IRQ
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_SetIrqState(uint8_t Enable)
{
    if (Enable == true)
    {
        // FIXME ENABLE IRQ
    }
    else
    {
        // FIXME DISABLE IRQ
    }
}
/******************************************************************************
 * @brief Initialize CRC Process
 * @param None
 * @return None
 ******************************************************************************/
static void LuosHAL_CRCInit(void)
{
    //CRC initialisation
}
/******************************************************************************
 * @brief Compute CRC
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_ComputeCRC(uint8_t *data, uint8_t *crc)
{
#ifdef CRC_HW
    //CRC HW calculation
#else
    for (uint8_t i = 0; i < 1; ++i)
    {
        uint16_t dbyte = data[i];
        *(uint16_t *)crc ^= dbyte << 8;
        for (uint8_t j = 0; j < 8; ++j)
        {
            uint16_t mix = *(uint16_t *)crc & 0x8000;
            *(uint16_t *)crc = (*(uint16_t *)crc << 1);
            if (mix)
                *(uint16_t *)crc = *(uint16_t *)crc ^ 0x0007;
        }
    }
#endif
}
