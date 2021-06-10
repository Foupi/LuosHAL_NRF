/*      INCLUDES                                                    */

#include "luos_hal_ptp.h"
#include "luos_hal.h"

// C STANDARD
#include <stdint.h>             // uint16_t, uint8_t
#include <string.h>             // memset

// NRF

#ifdef DEBUG
#include "nrf_log.h"            // NRF_LOG_INFO
#endif /* DEBUG */

// LUOS
#include "config.h"             // NBR_PORT
#include "port_manager.h"       // PortMng_PtpHandler

// CUSTOM
#include "ptp_server.h"         // PTP_SERVER_DEF, ptp_server_*
#include "ptp_service.h"        // ptp_char_value_t

/*      STATIC FUNCTIONS                                            */

static void LuosHAL_RegisterPTP(void);
static inline void LuosHAL_GPIOProcess(uint16_t GPIO);

/* Calls LuosHAL_GPIOProcess with the given port's GPIO if the
** conditions are met.
*/
static void LuosHAL_PTPManageWriteEvent(uint8_t port_idx,
                                        ptp_char_value_t val);

/*      STATIC VARIABLES & CONSTANTS                                */

// Expected IT.
typedef enum
{
    // No IT is expected.
    PTP_NO_IT,

    // Rising edge.
    PTP_RISING,

    // Falling edge.
    PTP_FALLING,

} ptp_expected_shift_t;

typedef struct
{
    uint16_t Pin;
    uint32_t Port;
    uint8_t IRQ;

    ptp_char_value_t        state;
    ptp_expected_shift_t    expected_it;
} Port_t;

#if (NBR_PORT > 0)
    Port_t PTP[NBR_PORT];
#endif

// PTP server instance.
PTP_SERVER_DEF(s_ptp_server);

/*      CALLBACKS                                                   */

// Calls GPIOProcess on PTPA pin.
static void LuosHAL_PTPServerWriteEvtHandler(const ptp_char_value_t val,
                                             ptp_server_t* instance);

/******************************************************************************
 * @brief Set PTP for Detection on branch
 * @param PTP branch
 * @return None
 ******************************************************************************/
void LuosHAL_SetPTPDefaultState(uint8_t PTPNbr)
{
    if (PTPNbr >= NBR_PORT)
    {
        return;
    }

    ptp_char_value_t state = 0;

    // Push 0, wait for rising edge.
    PTP[PTPNbr].state       = state;
    PTP[PTPNbr].expected_it = PTP_RISING;

    if (PTPNbr == 0)
    {
        // Send state change to server.
        ptp_server_on_ptp_update(&s_ptp_server, state);
    }
}
/******************************************************************************
 * @brief Set PTP for reverse detection on branch
 * @param PTP branch
 * @return None
 ******************************************************************************/
void LuosHAL_SetPTPReverseState(uint8_t PTPNbr)
{
    if (PTPNbr >= NBR_PORT)
    {
        return;
    }

    // Wait for falling edge.
    PTP[PTPNbr].expected_it = PTP_FALLING;
}
/******************************************************************************
 * @brief Set PTP line
 * @param PTP branch
 * @return None
 ******************************************************************************/
void LuosHAL_PushPTP(uint8_t PTPNbr)
{
    if (PTPNbr >= NBR_PORT)
    {
        return;
    }

    ptp_char_value_t state = 1;

    // Push 1, clear IT.
    PTP[PTPNbr].state       = state;
    PTP[PTPNbr].expected_it = PTP_NO_IT;

    if (PTPNbr == 0)
    {
        // Send state change to server.
        ptp_server_on_ptp_update(&s_ptp_server, state);
    }
}
/******************************************************************************
 * @brief Get PTP line
 * @param PTP branch
 * @return Line state
 ******************************************************************************/
uint8_t LuosHAL_GetPTPState(uint8_t PTPNbr)
{
    // Invalid PTP index.
    if (PTPNbr >= NBR_PORT)
    {
        return 0;
    }

    // Read pin state.
    return PTP[PTPNbr].state;
}
/******************************************************************************
 * @brief Initialisation GPIO
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_GPIOInit(void)
{
    #ifdef DEBUG
    NRF_LOG_INFO("GPIO init!");
    #endif /* DEBUG */

    ptp_server_init_t params;
    memset(&params, 0, sizeof(params));

    params.ptp_write_evt_handler    = LuosHAL_PTPServerWriteEvtHandler;

    ptp_server_init(&s_ptp_server, &params);

    LuosHAL_RegisterPTP();
    for (uint8_t ptp_idx = 0; ptp_idx < NBR_PORT; ptp_idx++)
    {
        LuosHAL_SetPTPDefaultState(ptp_idx);
    }
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
static void LuosHAL_PTPManageWriteEvent(uint8_t port_idx,
                                        ptp_char_value_t val)
{
    Port_t port = PTP[port_idx];

    if (port.state == val)
    {
        return;
    }

    PTP[port_idx].state = val;

    switch(port.expected_it)
    {
    case PTP_NO_IT:
        break;
    case PTP_RISING:
        if (val == 0)
        {
            return;
        }
        #ifdef DEBUG
        NRF_LOG_INFO("PTP Rising IT!");
        #endif /* DEBUG */
        break;
    case PTP_FALLING:
        if (val == 1)
        {
            return;
        }
        #ifdef DEBUG
        NRF_LOG_INFO("PTP Falling IT!");
        #endif /* DEBUG */
        break;
    default:
        return;
    }

    PINOUT_IRQHANDLER(port.Pin);
}
void PINOUT_IRQHANDLER(uint16_t GPIO_Pin)
{
    LuosHAL_GPIOProcess(GPIO_Pin);
}

/******************************************************************************
 * @brief callback for GPIO IT
 * @param GPIO IT line
 * @return None
 ******************************************************************************/
static inline void LuosHAL_GPIOProcess(uint16_t GPIO)
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

static void LuosHAL_PTPServerWriteEvtHandler(const ptp_char_value_t val,
                                             ptp_server_t* instance)
{
    LuosHAL_PTPManageWriteEvent(0, val);
}
