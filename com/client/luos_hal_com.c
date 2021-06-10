/*      INCLUDES                                                    */

#include "luos_hal.h"

// C STANDARD
#include <stdbool.h>                    // bool
#include <stdint.h>                     // uint32_t, uint8_t, uint16_t
#include <string.h>                     // memset

// NRF
#include "ble_nus_c.h"                  // BLE_NUS_C_*, ble_nus_c_*
#include "sdk_errors.h"                 // ret_code_t

#ifdef DEBUG
#include "nrf_log.h"                    // NRF_LOG_INFO
#endif /* DEBUG */

// NRF APPS
#include "app_error.h"                  // APP_ERROR_CHECK

// LUOS
#include "context.h"                    // ctx
#include "reception.h"                  // Recep_Timeout

// CUSTOM
#include "luos_hal_timer.h"             /* LuosHAL_TimeoutInit,
                                        ** DEFAULT_TIMEOUT
                                        */
#include "luos_hal_ble_client_ctx.h"    // g_nus_c_ptr
#include "msg_queue.h"                  // msg_queue_*, TX_BUF_SIZE

/*      STATIC FUNCTIONS                                            */

static inline void LuosHAL_ComReceive(void);

// Sends the current buffer if needed and possible.
static bool LuosHAL_ComSendOp(void);

/*      CALLBACKS                                                   */

/* DB discovery complete:   Assigns handles and enables notifications.
** TX event:                Manage received data in LuosHAL_ComReceive.
*/
static void LuosHAL_ComClientEventHandler(ble_nus_c_t* instance,
                                          const ble_nus_c_evt_t* event);

/*      GLOBAL/STATIC VARIABLES & CONSTANTS                         */

// Current read byte.
volatile static uint8_t s_curr_rx_byte;

// Global NUS client instance accessor.
ble_nus_c_t*            g_nus_c_ptr;

// Timeout to wait for resources.
static const uint32_t   COM_TX_WAIT_RESOURCES   = 25;

/*      INITIALIZATIONS                                             */

// NUS client instance.
BLE_NUS_C_DEF(s_nus_c);

/******************************************************************************
 * @brief Luos HAL Initialize Generale communication inter node
 * @param Select a baudrate for the Com
 * @return none
 ******************************************************************************/
void LuosHAL_ComInit(uint32_t Baudrate)
{
    LuosHAL_TimeoutInit();

    ble_nus_c_init_t params;
    memset(&params, 0, sizeof(ble_nus_c_init_t));

    params.evt_handler = LuosHAL_ComClientEventHandler;

    ret_code_t err_code = ble_nus_c_init(&s_nus_c, &params);
    APP_ERROR_CHECK(err_code);

    // FIXME Read explanation in `luos_hal_ble_client_ctx.h`.
    g_nus_c_ptr = &s_nus_c;
}
/******************************************************************************
 * @brief Tx enable/disable relative to com
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_SetTxState(uint8_t Enable)
{
}
/******************************************************************************
 * @brief Rx enable/disable relative to com
 * @param
 * @return
 ******************************************************************************/
void LuosHAL_SetRxState(uint8_t Enable)
{
}

/******************************************************************************
 * @brief Process data transmit
 * @param None
 * @return None
 ******************************************************************************/
uint8_t LuosHAL_ComTransmit(uint8_t *data, uint16_t size)
{
    #ifdef DEBUG
    NRF_LOG_INFO("Prepare %u bytes for sending!", size);
    NRF_LOG_HEXDUMP_INFO(data, size);
    #endif /* DEBUG */

    if (size <= TX_BUF_SIZE)
    {
        bool enqueued = msg_queue_enqueue(data, size);
        if (!enqueued)
        {
            #ifdef DEBUG
            NRF_LOG_INFO("Message could not be enqueued!");
            #endif /* DEBUG */

            return 0;
        }
    }
    else
    {
        uint16_t curr_idx   = 0;
        uint16_t cp_size    = TX_BUF_SIZE;
        while (curr_idx < size)
        {
            bool enqueued = msg_queue_enqueue(data + curr_idx, cp_size);
            if (!enqueued)
            {
                #ifdef DEBUG
                NRF_LOG_INFO("Message could not be enqueued!");
                #endif /* DEBUG */

                return 0;
            }

            curr_idx += TX_BUF_SIZE;
            if ((size - curr_idx) >= TX_BUF_SIZE)
            {
                cp_size = TX_BUF_SIZE;
            }
            else
            {
                cp_size = size - curr_idx;
            }
        }
    }

    while (true)
    {
        bool message_present = LuosHAL_ComSendOp();
        if (!message_present)
        {
            // Message queue empty.
            break;
        }

        // Wait for resources to cool down... There is no TX complete.
        uint32_t curr_tick  = LuosHAL_GetSystick();
        uint32_t waited_ms  = 0;
        while (waited_ms < COM_TX_WAIT_RESOURCES)
        {
            uint32_t old_val    = curr_tick;
            curr_tick           = LuosHAL_GetSystick();
            if (curr_tick < old_val)
            {
                // There was an overflow.
                uint32_t remaining;
                if (old_val < MAX_SYSTICK_MS_VAL)
                {
                    remaining = 0;
                }
                else
                {
                    remaining = (old_val - MAX_SYSTICK_MS_VAL);
                }
                waited_ms += (remaining + curr_tick);
            }
            else
            {
                waited_ms += (curr_tick - old_val);
            }
        }
    }

    LuosHAL_ResetTimeout(DEFAULT_TIMEOUT);

    return 1;
}
/******************************************************************************
 * @brief Luos Tx communication complete
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_ComTxComplete(void)
{
    LuosHAL_ResetTimeout(DEFAULT_TIMEOUT);
}
/******************************************************************************
 * @brief set state of Txlock detection pin
 * @param None
 * @return Lock status
 ******************************************************************************/
void LuosHAL_SetTxLockDetecState(uint8_t Enable)
{
}
/******************************************************************************
 * @brief get Lock Com transmit status this is the HW that can generate lock TX
 * @param None
 * @return Lock status
 ******************************************************************************/
uint8_t LuosHAL_GetTxLockState(void)
{
    return 0;
}
/******************************************************************************
 * @brief set state of Txlock detection pin
 * @param None
 * @return Lock status
 ******************************************************************************/
void LuosHAL_SetRxDetecPin(uint8_t Enable)
{
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
    ctx.rx.callback(&s_curr_rx_byte);
}

static bool LuosHAL_ComSendOp(void)
{
    tx_buffer_t* tx_buffer = msg_queue_peek();
    if (tx_buffer == NULL)
    {
        // Queue was empty: no message to send.
        return false;
    }

    uint16_t    size    = tx_buffer->size;
    uint8_t*    data    = tx_buffer->buffer;

    #ifdef DEBUG
    NRF_LOG_INFO("Sending %u bytes to server!", size);
    #endif /* DEBUG */

    ret_code_t err_code = ble_nus_c_string_send(&s_nus_c, data, size);
    APP_ERROR_CHECK(err_code);

    msg_queue_pop();

    return true;
}

static void LuosHAL_ComClientEventHandler(ble_nus_c_t* instance,
                                          const ble_nus_c_evt_t* event)
{
    ret_code_t err_code;

    switch(event->evt_type)
    {
    case BLE_NUS_C_EVT_DISCOVERY_COMPLETE:
        err_code = ble_nus_c_handles_assign(instance,
                                            event->conn_handle,
                                            &(event->handles));
        APP_ERROR_CHECK(err_code);
        err_code = ble_nus_c_tx_notif_enable(instance);
        APP_ERROR_CHECK(err_code);
        break;
    case BLE_NUS_C_EVT_NUS_TX_EVT:
    {
        uint16_t len = event->data_len;

        #ifdef DEBUG
        NRF_LOG_INFO("Received %u bytes!", len);
        NRF_LOG_HEXDUMP_INFO(event->p_data, len);
        #endif /* DEBUG */

        for (uint16_t rx_byte_idx = 0; rx_byte_idx < len; rx_byte_idx++)
        {
            s_curr_rx_byte = event->p_data[rx_byte_idx];
            LUOS_COM_IRQHANDLER();
        }
        if (len == 1) // Ack
        {
            // Manage Ack: reset recep callback and pop TX task.
            Recep_Timeout();
        }
        else if (len < BLE_NUS_MAX_DATA_LEN - 1)
        {
            // Complete message: no need to wait for the rest.
            Recep_Reset();
        }
        else
        {
            // Else it's partial data...
            LuosHAL_ResetTimeout(DEFAULT_TIMEOUT);
        }
    }
        break;
    default:
        break;
    }
}
