/*      INCLUDES                                                    */

#include "luos_hal.h"

// C STANDARD
#include <stdbool.h>        // bool
#include <stdint.h>         // uint32_t, uint8_t, uint16_t

// LUOS
#include "msg_alloc.h"      // MsgAlloc_PullMsgFromTxTask

// CUSTOM
#include "luos_hal_timer.h" // LuosHAL_TimeoutInit, DEFAULT_TIMEOUT

/*      STATIC FUNCTIONS                                            */

static inline void LuosHAL_ComReceive(void);

/*      STATIC VARIABLES & CONSTANTS                                */

/******************************************************************************
 * @brief Luos HAL Initialize Generale communication inter node
 * @param Select a baudrate for the Com
 * @return none
 ******************************************************************************/
void LuosHAL_ComInit(uint32_t Baudrate)
{
    LUOS_COM_CLOCK_ENABLE();

    // Initialise USART1

    // FIXME Enable COM
    LuosHAL_TimeoutInit();
}
/******************************************************************************
 * @brief Tx enable/disable relative to com
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_SetTxState(uint8_t Enable)
{
    if (Enable == true)
    {
        //put Tx COM pin in push pull
        if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
        {
            //Tx enable 
        }
    }
    else
    {
        //put Tx COM pin in Open drain
        if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
        {
            //Tx Disable
        }
    }
}
/******************************************************************************
 * @brief Rx enable/disable relative to com
 * @param
 * @return
 ******************************************************************************/
void LuosHAL_SetRxState(uint8_t Enable)
{
    if (Enable == true)
    {
        //clear data register
        // Enable Rx com
        // Enable Rx IT
        if ((RX_EN_PIN != DISABLE) || (RX_EN_PORT != DISABLE))
        {
            //Rx enable
        }
    }
    else
    {
        // disable Rx com
        // disable Rx IT
        if ((RX_EN_PIN != DISABLE) || (RX_EN_PORT != DISABLE))
        {
            //Rx Disable
        }
    }
}
/******************************************************************************
 * @brief Process data transmit
 * @param None
 * @return None
 ******************************************************************************/
uint8_t LuosHAL_ComTransmit(uint8_t *data, uint16_t size)
{
    MsgAlloc_PullMsgFromTxTask();

    return 0;
}
/******************************************************************************
 * @brief Luos Tx communication complete
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_ComTxComplete(void)
{
    //while tx complete
    LuosHAL_ResetTimeout(DEFAULT_TIMEOUT);
}
/******************************************************************************
 * @brief set state of Txlock detection pin
 * @param None
 * @return Lock status
 ******************************************************************************/
void LuosHAL_SetTxLockDetecState(uint8_t Enable)
{
    if (TX_LOCK_DETECT_IRQ != DISABLE)
    {
        //clear tx detect IT
        if (Enable == true)
        {
            //clear flag
        }
        else
        {
            ////clear flag
        }
    }
}
/******************************************************************************
 * @brief get Lock Com transmit status this is the HW that can generate lock TX
 * @param None
 * @return Lock status
 ******************************************************************************/
uint8_t LuosHAL_GetTxLockState(void)
{
    uint8_t result = false;
    
    #ifdef USART_ISR_BUSY
        //busy flag
        LuosHAL_ResetTimeout(DEFAULT_TIMEOUT);
        result = true;
    #else
    if ((TX_LOCK_DETECT_PIN != DISABLE) && (TX_LOCK_DETECT_PORT != DISABLE))
    {
        if(TX_LOCK_DETECT_IRQ == DISABLE)
        {
            //result //
            if(result == true)
            {
                LuosHAL_ResetTimeout(DEFAULT_TIMEOUT);
            }
        }
    }
    #endif
    return result;
}
/******************************************************************************
 * @brief set state of Txlock detection pin
 * @param None
 * @return Lock status
 ******************************************************************************/
void LuosHAL_SetRxDetecPin(uint8_t Enable)
{
    if (TX_LOCK_DETECT_IRQ != DISABLE)
    {
        //clear tx detect IT
        if (Enable == true)
        {
            //clear flag
        }
        else
        {
            //set flag
        }
    }
}

void com_clock_enable(void)
{
    // FIXME Enable COM clock.
}
void LUOS_COM_IRQHANDLER()
{
    LuosHAL_ComReceive();
}

/******************************************************************************
 * @brief Process data receive
 * @param None
 * @return None
 ******************************************************************************/
static inline void LuosHAL_ComReceive(void)
{
    LuosHAL_ResetTimeout(DEFAULT_TIMEOUT);

    //data receive IT

    //Framming error IT
}
