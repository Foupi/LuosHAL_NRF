#include "luos_hal_ble_common.h"

/*      INCLUDES                                                    */

// C STANDARD
#include <stdbool.h>        // bool
#include <stdint.h>         // uint16_t, uint32_t

// NRF
#include "boards.h"         // bsp_board_leds_on
#include "nrf_ble_gatt.h"   // NRF_BLE_GATT_DEF, nrf_ble_gatt_*
#include "nrf_sdh_ble.h"    /* nrf_sdh_ble_default_cfg_set,
                            ** nrf_sdh_ble_enable, NRF_SDH_BLE_OBSERVER,
                            ** nrf_sdh_ble_evt_handler_t
                            */
#include "sdk_config.h"     // NRF_SDH_BLE_GATT_MAX_MTU_SIZE

#ifdef DEBUG
#include "nrf_log.h"        // NRF_LOG_INFO
#endif /* DEBUG */

// NRF APPS
#include "app_error.h"      // APP_ERROR_CHECK

// SOFTDEVICE
#include "ble.h"            // ble_evt_t
#include "ble_gap.h"        // sd_ble_gap_disconnect
#include "ble_gatts.h"      // sd_ble_gatts_sys_attr_set
#include "ble_hci.h"        // BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION

/*      STATIC VARIABLES & CONSTANTS                                */

// BLE Observers priority.
#define BLE_OBS_PRIO 3

// Defines if the device is connected or not.
static bool s_connected = false;

static nrf_sdh_ble_evt_handler_t s_ble_gap_obs_cb = NULL;

/*      INITIALIZATIONS                                             */

// GATT module instance.
NRF_BLE_GATT_DEF(s_gatt_mod);

/*      STATIC FUNCTIONS                                            */

// Initializes the given GATT module instance.
static void gatt_instance_init(nrf_ble_gatt_t* instance);

// Waits until the given boolean is set to true.
static void boolean_wait(bool* condition);

/*      CALLBACKS                                                   */

// BLE GAP observer event handler.
static void ble_gap_obs_event_handler(const ble_evt_t* event,
                                      void* context);
// BLE GATT observer event handler.
static void ble_gatt_obs_event_handler(const ble_evt_t* event,
                                       void* context);

void ble_stack_enable(void)
{
    ret_code_t err_code;
    uint32_t ram_start_addr;

    err_code = nrf_sdh_ble_default_cfg_set(CONN_CFG_TAG,
                                           &ram_start_addr);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_sdh_ble_enable(&ram_start_addr);
    APP_ERROR_CHECK(err_code);
}

void gatt_module_init(att_mtu_update_t att_mtu_update)
{
    gatt_instance_init(&s_gatt_mod);

    ret_code_t err_code = att_mtu_update(&s_gatt_mod,
                                         NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
}

void ble_connection_wait(void)
{
    boolean_wait(&s_connected);
}

void ble_observers_register(nrf_sdh_ble_evt_handler_t ble_gap_obs_cb)
{
    s_ble_gap_obs_cb = ble_gap_obs_cb;

    NRF_SDH_BLE_OBSERVER(s_gap_obs, BLE_OBS_PRIO,
                         ble_gap_obs_event_handler, &s_connected);
    NRF_SDH_BLE_OBSERVER(s_gatt_obs, BLE_OBS_PRIO,
                         ble_gatt_obs_event_handler, NULL);
}

void connection_end_signal(void)
{
    bsp_board_leds_on();
    while(true);
}

static void gatt_instance_init(nrf_ble_gatt_t* instance)
{
    ret_code_t err_code = nrf_ble_gatt_init(instance, NULL);
    APP_ERROR_CHECK(err_code);
}

static void boolean_wait(bool* condition)
{
    while (!(*condition));
}

static void ble_gap_obs_event_handler(const ble_evt_t* event,
                                      void* context)
{
    s_ble_gap_obs_cb(event, context);
}

static void ble_gatt_obs_event_handler(const ble_evt_t* event,
                                       void* context)
{
    ret_code_t  err_code;
    uint16_t    conn_handle = event->evt.gap_evt.conn_handle;

    switch (event->header.evt_id)
    {
    case BLE_GATTS_EVT_SYS_ATTR_MISSING:
        #ifdef DEBUG
        NRF_LOG_INFO("Missing GATT attribute!");
        #endif /* DEBUG */

        err_code = sd_ble_gatts_sys_attr_set(conn_handle, NULL, 0, 0);
        APP_ERROR_CHECK(err_code);
        break;
    case BLE_GATTC_EVT_TIMEOUT:
    case BLE_GATTS_EVT_TIMEOUT:
        #ifdef DEBUG
        NRF_LOG_INFO("GATT timeout!");
        #endif /* DEBUG */

        err_code = sd_ble_gap_disconnect(conn_handle,
            BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        break;
    default:
        break;
    }
}
