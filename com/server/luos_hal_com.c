/*      INCLUDES                                                    */

#include "luos_hal.h"

// C STANDARD
#include <stdbool.h>        // bool
#include <stdint.h>         // uint32_t, uint8_t, uint16_t

// NRF
#include "ble_nus.h"        // BLE_NUS_*, ble_nus_*
#include "sdk_errors.h"     // ret_code_t

#ifdef DEBUG
#include "nrf_log.h"        // NRF_LOG_*
#endif /* DEBUG */

// NRF APPS
#include "app_error.h"      // APP_ERROR_CHECK

// SOFTDEVICE
#include "ble_types.h"      // BLE_CONN_HANDLE_INVALID

// LUOS
#include "context.h"        // ctx
#include "reception.h"      // Recep_Timeout

// CUSTOM
#include "luos_hal_timer.h" // LuosHAL_TimeoutInit, DEFAULT_TIMEOUT

#include "msg_queue.h"      // msg_queue_*, TX_BUF_SIZE

/*      STATIC FUNCTIONS                                            */

static inline void LuosHAL_ComReceive(void);

// Sends the current buffer if needed and possible.
static void LuosHAL_ComSendOp(void);

/*      STATIC VARIABLES & CONSTANTS                                */

// Number of accepted NUS clients.
#define NB_NUS_CLIENTS  1

// Current read byte.
volatile static uint8_t s_curr_rx_byte;

// NUS client connection handle.
static uint16_t         s_conn_handle;

// Defines if a buffer can be sent.
static bool             s_tx_ready      = true;

/*      INITIALIZATIONS                                             */

// NUS server instance.
BLE_NUS_DEF(s_nus, NB_NUS_CLIENTS);

/*      CALLBACKS                                                   */

/* Data received:   Call LuosHAL_ComReceive.
** TX ready:        Send data if available.
** Comm start/stop: // FIXME Set RX state?
*/
static void LuosHAL_ComServerEventHandler(ble_nus_evt_t* event);

/******************************************************************************
 * @brief Luos HAL Initialize Generale communication inter node
 * @param Select a baudrate for the Com
 * @return none
 ******************************************************************************/
void LuosHAL_ComInit(uint32_t Baudrate)
{
    ble_nus_init_t params;
    memset(&params, 0, sizeof(ble_nus_init_t));

    params.data_handler = LuosHAL_ComServerEventHandler;

    ret_code_t err_code = ble_nus_init(&s_nus, &params);
    APP_ERROR_CHECK(err_code);

    LuosHAL_TimeoutInit();
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
    if (s_conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        #ifdef DEBUG
        NRF_LOG_INFO("Connection handle not assigned: leaving...");
        #endif /* DEBUG */

        return 0;
    }

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

    if (s_tx_ready)
    {
        LuosHAL_ComSendOp();
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
    s_tx_ready = true;
    LuosHAL_ComSendOp();
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
    uint8_t result = false;
    #ifdef USART_ISR_BUSY
        //busy flag
        LuosHAL_ResetTimeout(DEFAULT_TIMEOUT);
        result = true;
    #else
    //result //
        if(result == true)
        {
            LuosHAL_ResetTimeout(DEFAULT_TIMEOUT);
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

static void LuosHAL_ComSendOp(void)
{
    tx_buffer_t* tx_buffer = msg_queue_peek();
    if (tx_buffer == NULL)
    {
        // Queue was empty: no message to send.
        return;
    }

    uint16_t    size    = tx_buffer->size;
    uint8_t*    data    = tx_buffer->buffer;

    #ifdef DEBUG
    NRF_LOG_INFO("Sending %u bytes to client!", size);
    #endif /* DEBUG */

    ret_code_t err_code = ble_nus_data_send(&s_nus, data, &size,
                                            s_conn_handle);
    APP_ERROR_CHECK(err_code);

    msg_queue_pop();

    // No sending until this one is complete.
    s_tx_ready = false;
}

static void LuosHAL_ComServerEventHandler(ble_nus_evt_t* event)
{
    switch (event->type)
    {
    case BLE_NUS_EVT_RX_DATA:
    {
        ble_nus_evt_rx_data_t rx_data = event->params.rx_data;
        uint16_t len = rx_data.length;

        #ifdef DEBUG
        NRF_LOG_INFO("Received %u bytes!", len);
        NRF_LOG_HEXDUMP_INFO(rx_data.p_data, len);
        #endif /* DEBUG */

        for (uint16_t rx_byte_idx = 0; rx_byte_idx < len; rx_byte_idx++)
        {
            s_curr_rx_byte = rx_data.p_data[rx_byte_idx];
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
    case BLE_NUS_EVT_COMM_STARTED:
        s_conn_handle = event->conn_handle;
        break;
    case BLE_NUS_EVT_COMM_STOPPED:
        s_conn_handle = BLE_CONN_HANDLE_INVALID;
        break;
    case BLE_NUS_EVT_TX_RDY:
        LuosHAL_ComTxComplete();
    default:
        break;
    }
}
