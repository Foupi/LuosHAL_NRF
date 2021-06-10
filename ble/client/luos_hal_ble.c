#include "luos_hal_ble.h"

/*      INCLUDES                                                    */

// C STANDARD
#include <stdbool.h>                    // bool
#include <stdint.h>                     // uint8_t, uint16_t, uint32_t
#include <string.h>                     // memset

// NRF
#include "ble_db_discovery.h"           /* BLE_DB_DISCOVERY_DEF,
                                        ** ble_db_discovery_*
                                        */
#include "ble_nus_c.h"                  // ble_nus_c_on_db_disc_evt
#include "boards.h"                     /* bsp_board_led_on,
                                        ** bsp_board_led_off
                                        */
#include "nrf_ble_gatt.h"               // nrf_ble_gatt_att_mtu_central_set
#include "nrf_ble_scan.h"               /* NRF_BLE_SCAN_DEF,
                                        ** nrf_ble_scan_t,
                                        ** nrf_ble_scan_init_t,
                                        ** nrf_ble_scan_init,
                                        ** nrf_ble_scan_filters_enable,
                                        ** NRF_BLE_SCAN_*_FILTER,
                                        ** nrf_ble_scan_filter_set,
                                        ** SCAN_*_FILTER,
                                        ** nrf_ble_scan_start
                                        */
#include "sdk_errors.h"                 // ret_code_t

#ifdef DEBUG
#include "nrf_log.h"                    // NRF_LOG_INFO
#endif /* DEBUG */

// NRF APPS
#include "app_error.h"                  // APP_ERROR_CHECK

// SOFTDEVICE
#include "ble.h"                        // ble_evt_t
#include "ble_gap.h"                    /* ble_gap_phys_t,
                                        ** sd_ble_gap_phy_update,
                                        ** ble_gap_evt_t,
                                        ** sd_ble_gap_conn_param_update
                                        */

// CUSTOM
#include "luos_hal_ble_client_ctx.h"    // g_ptp_client_ptr, g_nus_c_ptr
#include "luos_hal_ble_common.h"        /* CONN_CFG_TAG,
                                        ** LUOS_SERVER_NAME,
                                        ** gatt_module_init,
                                        ** ble_observers_register,
                                        ** ble_connection_wait
                                        */
#include "ptp_client.h"                 // g_ptp_service_uuid

/*      STATIC VARIABLES & CONSTANTS                                */

// LED signaling scanning.
static uint8_t SCANNING_LED = 3;

/*      INITIALIZATIONS                                             */

// Scan module instance.
NRF_BLE_SCAN_DEF(s_scan_mod);

// DB Discovery module instance.
BLE_DB_DISCOVERY_DEF(s_db_disc);

/*      STATIC FUNCTIONS                                            */

// Initializes scan parameters for the given module instance.
static void scan_module_init(nrf_ble_scan_t* instance);

// Initializes scan filters for the given module instance.
static void scan_filters_init(nrf_ble_scan_t* instance);

// Start scanning for the given module instance.
static void scan_module_start(nrf_ble_scan_t* instance);

// Initializes the given DB Discovery module instance.
static void db_disc_init(void);

/*      CALLBACKS                                                   */

// BLE GAP observer event handler.
static void ble_gap_obs_event_handler(const ble_evt_t* event,
                                      void* context);

// Dispatches the event between the client event handlers.
static void db_discovery_event_handler(ble_db_discovery_evt_t* event);

void LuosHAL_BleInit(void)
{
    // Enable BLE stack.
    ble_stack_enable();

    // Initialize DB Discovery module.
    db_disc_init();
}

void LuosHAL_BleSetup(void)
{
    // Initialize scan module and filters.
    scan_module_init(&s_scan_mod);
    scan_filters_init(&s_scan_mod);

    // Initialize GATT module.
    gatt_module_init(nrf_ble_gatt_att_mtu_central_set);

    // Register BLE observers.
    ble_observers_register(ble_gap_obs_event_handler);
}

void LuosHAL_BleConnect(void)
{
    // Start scanning.
    scan_module_start(&s_scan_mod);

    // Loop while not connected.
    ble_connection_wait();
}

static void scan_module_init(nrf_ble_scan_t* instance)
{
    ret_code_t err_code;

    nrf_ble_scan_init_t scan_params;
    memset(&scan_params, 0, sizeof(nrf_ble_scan_init_t));

    scan_params.connect_if_match    = true;
    scan_params.conn_cfg_tag        = CONN_CFG_TAG;

    err_code = nrf_ble_scan_init(instance, &scan_params, NULL);
    APP_ERROR_CHECK(err_code);
}

static void scan_filters_init(nrf_ble_scan_t* instance)
{
    ret_code_t err_code;

    // FIXME We might want to match both name and UUID in the future...
    err_code = nrf_ble_scan_filters_enable(instance,
        NRF_BLE_SCAN_NAME_FILTER | NRF_BLE_SCAN_UUID_FILTER, false);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_scan_filter_set(instance, SCAN_NAME_FILTER,
                                       LUOS_SERVER_NAME);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_scan_filter_set(instance, SCAN_UUID_FILTER,
                                       &g_ptp_service_uuid);

    APP_ERROR_CHECK(err_code);
}

static void scan_module_start(nrf_ble_scan_t* instance)
{
    ret_code_t err_code = nrf_ble_scan_start(instance);
    APP_ERROR_CHECK(err_code);

    bsp_board_led_on(SCANNING_LED);
}

static void db_disc_init(void)
{
    ret_code_t err_code = ble_db_discovery_init(db_discovery_event_handler);
    APP_ERROR_CHECK(err_code);
}

static void ble_gap_obs_event_handler(const ble_evt_t* event,
                                      void* context)
{
    ret_code_t      err_code;
    ble_gap_evt_t   gap_event = event->evt.gap_evt;

    bool* connected = (bool*) context;

    switch (event->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        #ifdef DEBUG
        NRF_LOG_INFO("GAP connection!");
        #endif /* DEBUG */

        *connected = true;
        err_code = ble_db_discovery_start(&s_db_disc,
                                          gap_event.conn_handle);
        APP_ERROR_CHECK(err_code);
        bsp_board_led_off(SCANNING_LED);
        break;
    case BLE_GAP_EVT_DISCONNECTED:
        #ifdef DEBUG
        NRF_LOG_INFO("GAP disconnection: stopping!");
        #endif /* DEBUG */

        connection_end_signal();
        break;
    case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        #ifdef DEBUG
        NRF_LOG_INFO("Updating PHY interface!");
        #endif /* DEBUG */

        {
            ble_gap_phys_t updated_phys =
            {
                .rx_phys = BLE_GAP_PHY_1MBPS,
                .tx_phys = BLE_GAP_PHY_1MBPS,
            };
            err_code = sd_ble_gap_phy_update(gap_event.conn_handle,
                                             &updated_phys);
            APP_ERROR_CHECK(err_code);
        }
        break;
    case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
        #ifdef DEBUG
        NRF_LOG_INFO("Updating connection parameters!");
        #endif /* DEBUG */

        err_code = sd_ble_gap_conn_param_update(gap_event.conn_handle,
            &gap_event.params.conn_param_update_request.conn_params);
        APP_ERROR_CHECK(err_code);
    case BLE_GAP_EVT_TIMEOUT:
        #ifdef DEBUG
        NRF_LOG_INFO("GAP timeout!");
        #endif /* DEBUG */

        // Manage timeout.
        break;
    default:
        break;
    }
}

static void db_discovery_event_handler(ble_db_discovery_evt_t* event)
{
    ptp_client_on_db_discovery_evt(event, g_ptp_client_ptr);
    ble_nus_c_on_db_disc_evt(g_nus_c_ptr, event);
}
