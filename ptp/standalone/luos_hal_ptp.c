/*      INCLUDES                                                    */

#include "luos_hal_ptp.h"
#include "luos_hal.h"

// C STANDARD
#include <stdint.h>             // uint16_t, uint8_t

// LUOS
#include "config.h"             // NBR_PORT
#include "port_manager.h"       // PortMng_PtpHandler

/*      STATIC FUNCTIONS                                            */

static void LuosHAL_RegisterPTP(void);
static inline void LuosHAL_GPIOProcess(uint16_t GPIO);

/*      STATIC VARIABLES & CONSTANTS                                */

typedef struct
{
    uint16_t Pin;
    uint32_t Port;
    uint8_t IRQ;
} Port_t;

#if (NBR_PORT > 0)
    Port_t PTP[NBR_PORT];
#endif

/******************************************************************************
 * @brief Set PTP for Detection on branch
 * @param PTP branch
 * @return None
 ******************************************************************************/
void LuosHAL_SetPTPDefaultState(uint8_t PTPNbr)
{
    //clear IT
    // Pull Down / IT mode / Rising Edge
}
/******************************************************************************
 * @brief Set PTP for reverse detection on branch
 * @param PTP branch
 * @return None
 ******************************************************************************/
void LuosHAL_SetPTPReverseState(uint8_t PTPNbr)
{
    //clear IT
    // Pull Down / IT mode / Falling Edge
}
/******************************************************************************
 * @brief Set PTP line
 * @param PTP branch
 * @return None
 ******************************************************************************/
void LuosHAL_PushPTP(uint8_t PTPNbr)
{
    // Pull Down / Output mode
    // Clean edge/state detection and set the PTP pin as output
}
/******************************************************************************
 * @brief Get PTP line
 * @param PTP branch
 * @return Line state
 ******************************************************************************/
uint8_t LuosHAL_GetPTPState(uint8_t PTPNbr)
{
    // Pull Down / Input mode
    return 0;
}
/******************************************************************************
 * @brief Initialisation GPIO
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_GPIOInit(void)
{
    //Activate Clock for PIN choosen in luosHAL
    PORT_CLOCK_ENABLE();

    if ((RX_EN_PIN != DISABLE) || (RX_EN_PORT != DISABLE))
    {
        /*Configure GPIO pins : RxEN_Pin */
    }

    if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
    {
        /*Configure GPIO pins : TxEN_Pin */
    }

    /*Configure GPIO pins : TX_LOCK_DETECT_Pin */
    if ((TX_LOCK_DETECT_PIN != DISABLE) || (TX_LOCK_DETECT_PORT != DISABLE))
    {
        if (TX_LOCK_DETECT_IRQ != DISABLE)
        {
            
        }
        
    }

    /*Configure GPIO pin : TxPin */


    /*Configure GPIO pin : RxPin */


    //configure PTP
    LuosHAL_RegisterPTP();
    for (uint8_t i = 0; i < NBR_PORT; i++) /*Configure GPIO pins : PTP_Pin */
    {
        // Setup PTP lines
        LuosHAL_SetPTPDefaultState(i);
        //activate NVIC IT for PTP
    }

    if (TX_LOCK_DETECT_IRQ != DISABLE)
    {
        //activate NVIC IT for Tx detect
    }
}

void PINOUT_IRQHANDLER(uint16_t GPIO_Pin)
{
    LuosHAL_GPIOProcess(GPIO_Pin);
}

/******************************************************************************
 * @brief Register PTP
 * @param void
 * @return None
 ******************************************************************************/
static void LuosHAL_RegisterPTP(void)
{
    PTP[0].Pin = PTPA_PIN;
    PTP[0].Port = PTPA_PORT;
    PTP[0].IRQ = PTPA_IRQ;

#if (NBR_PORT >= 2)
    PTP[1].Pin = PTPB_PIN;
    PTP[1].Port = PTPB_PORT;
    PTP[1].IRQ = PTPB_IRQ;
#endif

#if (NBR_PORT >= 3)
    PTP[2].Pin = PTPC_PIN;
    PTP[2].Port = PTPC_PORT;
    PTP[2].IRQ = PTPC_IRQ;
#endif

#if (NBR_PORT >= 4)
    PTP[3].Pin = PTPD_PIN;
    PTP[3].Port = PTPD_PORT;
    PTP[3].IRQ = PTPD_IRQ;
#endif
}
/******************************************************************************
 * @brief callback for GPIO IT
 * @param GPIO IT line
 * @return None
 ******************************************************************************/
static inline void LuosHAL_GPIOProcess(uint16_t GPIO)
{
    ////Process for Tx Lock Detec
    if (GPIO == TX_LOCK_DETECT_PIN)
    {
        //clear flag
    }
    else
    {
        for (uint8_t i = 0; i < NBR_PORT; i++)
        {
            if (GPIO == PTP[i].Pin)
            {
                PortMng_PtpHandler(i);
                break;
            }
        }
    }
}
