/*      INCLUDES                                                    */

#include "luos_hal_ptp.h"
#include "luos_hal.h"

// C STANDARD
#include <stdint.h>                     // uint16_t, uint8_t
#include <string.h>                     // memset

// NRF

#ifdef DEBUG
#include "nrf_log.h"                    // NRF_LOG_INFO
#endif /* DEBUG */

// LUOS
#include "config.h"                     // NBR_PORT
#include "port_manager.h"               // PortMng_PtpHandler

// CUSTOM
#include "luos_hal_ble_client_ctx.h"    // g_ptp_client_ptr
#include "ptp_client.h"                 // PTP_CLIENT_DEF, ptp_client_*

/*      STATIC FUNCTIONS                                            */

static void LuosHAL_RegisterPTP(void);
static inline void LuosHAL_GPIOProcess(uint16_t GPIO);

// Manages the value according to the given PTP line's expected IT.
static void LuosHAL_PTPManageNotification(uint8_t port_idx,
                                          ptp_char_value_t recv_state);

/*      CALLBACKS                                                   */

/* DB Discovery complete:   Assigns the handles to the client instance.
** Notification:            Calls the appropriate GPIO handler.
*/
static void LuosHAL_PTPClientEvtHandler(const ptp_client_evt_t* event,
                                        ptp_client_t* instance);

/*      GLOBAL/STATIC VARIABLES & CONSTANTS                         */

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

// PTP client instance.
PTP_CLIENT_DEF(s_ptp_client);

// Global PTP client instance accessor.
ptp_client_t* g_ptp_client_ptr;

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
        ptp_client_ptp_char_write(&s_ptp_client, state);
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
        ptp_client_ptp_char_write(&s_ptp_client, state);
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
    ptp_client_init_t params;
    memset(&params, 0, sizeof(ptp_client_init_t));

    params.evt_handler    = LuosHAL_PTPClientEvtHandler;

    ptp_client_init(&s_ptp_client, &params);

    // FIXME Read explanation in `luos_hal_ble_client_ctx.h`.
    g_ptp_client_ptr = &s_ptp_client;

    LuosHAL_RegisterPTP();
    for (uint8_t ptp_idx = 0; ptp_idx < NBR_PORT; ptp_idx++)
    {
        LuosHAL_SetPTPDefaultState(ptp_idx);
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
    for (uint8_t i = 0; i < NBR_PORT; i++)
    {
        if (GPIO == PTP[i].Pin)
        {
            PortMng_PtpHandler(i);
            break;
        }
    }
}

static void LuosHAL_PTPManageNotification(uint8_t port_idx,
                                          ptp_char_value_t recv_state)
{
    Port_t port = PTP[port_idx];

    if (port.state == recv_state)
    {
        return;
    }

    PTP[port_idx].state = recv_state;

    switch(port.expected_it)
    {
    case PTP_NO_IT:
        break;
    case PTP_RISING:
        if (recv_state == 0)
        {
            return;
        }

        #ifdef DEBUG
        NRF_LOG_INFO("PTP Rising IT!");
        #endif /* ! DEBUG */

        break;
    case PTP_FALLING:
        if (recv_state == 1)
        {
            return;
        }

        #ifdef DEBUG
        NRF_LOG_INFO("PTP Falling IT!");
        #endif /* ! DEBUG */

        break;
    default:
        return;
    }

    PINOUT_IRQHANDLER(port.Pin);
}

static void LuosHAL_PTPClientEvtHandler(const ptp_client_evt_t* event,
                                        ptp_client_t* instance)
{
    switch (event->evt_type)
    {
    case PTP_C_DB_DISCOVERY_COMPLETE:
        ptp_client_handles_assign(instance, &(event->content.disc_db));
        ptp_client_ptp_notification_enable(instance, true);
        break;
    case PTP_C_NOTIFICATION_RECEIVED:
        LuosHAL_PTPManageNotification(0, event->content.value);
        break;
    default:
        break;
    }
}
