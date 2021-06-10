#include "luos_hal_ble.h"

/*      INCLUDES                                                    */

// C STANDARD
#include <stdbool.h>                // bool
#include <stdint.h>                 // uint8_t, uint32_t
#include <string.h>                 // memset

// NRF
#include "ble_advdata.h"            /* ble_advdata_t,
                                    ** BLE_ADVDATA_FULL_NAME,
                                    ** ble_advdata_uuid_list_t,
                                    ** ble_advdata_encode
                                */
#include "ble_conn_params.h"        /* ble_conn_params_init_t,
                                    ** ble_conn_params_init,
                                    ** ble_conn_params_evt_t,
                                    ** BLE_CONN_PARAMS_EVT_SUCCEEDED
                                    */
#include "ble_srv_common.h"         // BLE_UUID_DEVICE_INFORMATION_SERVICE
#include "boards.h"                 /* bsp_board_led_on,
                                    ** bsp_board_led_off
                                    */
#include "nrf_ble_gatt.h"           // nrf_ble_gatt_att_mtu_periph_set
#include "sdk_errors.h"             // ret_code_t

#ifdef DEBUG
#include "nrf_log.h"                // NRF_LOG_INFO
#endif /* DEBUG */

// NRF APPS
#include "app_error.h"              // APP_ERROR_CHECK
#include "app_timer.h"              // APP_TIMER_TICKS

// SOFTDEVICE
#include "ble_gap.h"                /* ble_gap_conn_sec_mode_t,
                                    ** BLE_GAP_CONN_SEC_MODE_SET_OPEN,
                                    ** sd_ble_gap_device_name_set,
                                    ** ble_gap_conn_params_t,
                                    ** BLE_GAP_CP_*,
                                    ** sd_ble_gap_ppcp_set,
                                    ** BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE,
                                    ** BLE_GAP_ADV_SET_DATA_SIZE_MAX,
                                    ** ble_gap_adv_data_t,
                                    ** BLE_GAP_ADV_SET_HANDLE_NOT_SET,
                                    ** ble_gap_adv_params_t,
                                    ** BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED,
                                    ** BLE_GAP_ADV_INTERVAL_MAX,
                                    ** BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED,
                                    ** BLE_GAP_ADV_FP_ANY,
                                    ** sd_ble_gap_adv_set_configure,
                                    ** sd_ble_gap_sec_params_reply,
                                    ** BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP,
                                    ** ble_gap_phys_t,
                                    ** sd_ble_gap_adv_start
                                    */
#include "ble_types.h"              /* ble_uuid_t, BLE_UUID_TYPE_BLE,
                                    ** BLE_CONN_HANDLE_INVALID
                                    */

// CUSTOM
#include "luos_hal_ble_common.h"    /* ble_stack_enable,
                                    ** LUOS_SERVER_NAME
                                    */
#include "ptp_server.h"             // g_ptp_server_uuid

/*      STATIC VARIABLES & CONSTANTS                                */

// Delays (in ticks) between connection parameters updates.
static const uint32_t FIRST_UPDATE_DELAY    = APP_TIMER_TICKS(5000);
static const uint32_t NEXT_UPDATE_DELAY     = APP_TIMER_TICKS(30000);

// Buffers for encoded advertising data.
static uint8_t s_adv_data_buffer[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
static uint8_t s_scan_resp_buffer[BLE_GAP_ADV_SET_DATA_SIZE_MAX];

// Advertisement set handle.
static uint8_t s_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;

// Handle for GAP connection.
static uint16_t s_conn_handle = BLE_CONN_HANDLE_INVALID;

// LED signaling advertising.
static const uint8_t ADVERTISING_LED = 3;

/*      INITIALIZATIONS                                             */

// The advertising data.
static ble_gap_adv_data_t s_adv_data =
{
    .adv_data =
    {
        .p_data = s_adv_data_buffer,
        .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX,
    },
    .scan_rsp_data =
    {
        .p_data = s_scan_resp_buffer,
        .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX,
    },
};

/*      STATIC FUNCTIONS                                            */

// Initializes the preferred GAP connection parameters.
static void gap_conn_params_init(void);

// Initialize connection negotiation parameters.
static void conn_nego_params_init(void);

// Initializes the advertisement data.
static void adv_data_init(ble_gap_adv_data_t* advertising_data);

// Initializes the advertisement parameters.
static void adv_params_init(uint8_t* adv_handle,
                            ble_gap_adv_data_t* adv_data);

// Starts advertising.
static void adv_start(uint8_t adv_handle);

/*      CALLBACKS                                                   */

// BLE GAP observer event handler.
static void ble_gap_obs_event_handler(const ble_evt_t* event,
                                      void* context);

// BLE connection parameters negotiation event handler.
static void ble_conn_params_evt_handler(ble_conn_params_evt_t* event);

// BLE SRV error handler.
static void ble_srv_error_handler(uint32_t error);

void LuosHAL_BleInit(void)
{
    // Enable BLE stack.
    ble_stack_enable();
}

void LuosHAL_BleSetup(void)
{
    // Initialize connection parameters.
    gap_conn_params_init();
    conn_nego_params_init();

    // Initialize advertisement parameters and data.
    adv_data_init(&s_adv_data);
    adv_params_init(&s_adv_handle, &s_adv_data);

    // Initialize GATT module.
    gatt_module_init(nrf_ble_gatt_att_mtu_periph_set);

    // Register BLE observers.
    ble_observers_register(ble_gap_obs_event_handler);
}

void LuosHAL_BleConnect(void)
{
    // Start advertising.
    adv_start(s_adv_handle);

    // Loop while not connected.
    ble_connection_wait();
}

static void gap_conn_params_init(void)
{
    ret_code_t err_code;

    ble_gap_conn_sec_mode_t sec_mode;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (uint8_t*)LUOS_SERVER_NAME,
                                          sizeof(LUOS_SERVER_NAME) - 1);
    APP_ERROR_CHECK(err_code);

    ble_gap_conn_params_t connection_parameters;
    memset(&connection_parameters, 0, sizeof(ble_gap_conn_params_t));

    connection_parameters.min_conn_interval  = BLE_GAP_CP_MIN_CONN_INTVL_MIN;
    connection_parameters.max_conn_interval  = BLE_GAP_CP_MAX_CONN_INTVL_MAX;
    connection_parameters.slave_latency      = 0;
    connection_parameters.conn_sup_timeout   = BLE_GAP_CP_CONN_SUP_TIMEOUT_MAX;

    err_code = sd_ble_gap_ppcp_set(&connection_parameters);
    APP_ERROR_CHECK(err_code);
}

static void conn_nego_params_init(void)
{
    ble_conn_params_init_t conn_init;
    memset(&conn_init, 0, sizeof(ble_gap_conn_params_t));

    conn_init.p_conn_params                     = NULL;
    conn_init.first_conn_params_update_delay    = FIRST_UPDATE_DELAY;
    conn_init.next_conn_params_update_delay     = NEXT_UPDATE_DELAY;
    conn_init.max_conn_params_update_count      = 3;
    conn_init.start_on_notify_cccd_handle       = BLE_CONN_HANDLE_INVALID;
    conn_init.disconnect_on_fail                = true;
    conn_init.evt_handler                       = ble_conn_params_evt_handler;
    conn_init.error_handler                     = ble_srv_error_handler;

    ret_code_t err_code = ble_conn_params_init(&conn_init);
    APP_ERROR_CHECK(err_code);
}

static void adv_start(uint8_t adv_handle)
{
    ret_code_t err_code = sd_ble_gap_adv_start(adv_handle,
                                               CONN_CFG_TAG);
    APP_ERROR_CHECK(err_code);

    bsp_board_led_on(ADVERTISING_LED);
}

static void adv_data_init(ble_gap_adv_data_t* advertising_data)
{
    ret_code_t err_code;

    ble_advdata_t adv_data;
    memset(&adv_data, 0, sizeof(ble_advdata_t));

    adv_data.name_type          = BLE_ADVDATA_FULL_NAME;
    adv_data.include_appearance = false;
    adv_data.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    err_code = ble_advdata_encode(&adv_data, s_adv_data_buffer,
                                  &(advertising_data->adv_data.len));
    APP_ERROR_CHECK(err_code);

    ble_uuid_t uuids[] =
    {
        g_ptp_server_uuid,
    };
    ble_advdata_uuid_list_t uuid_list =
    {
        .p_uuids    = uuids,
        .uuid_cnt   = sizeof(uuids) / sizeof(ble_uuid_t),
    };

    ble_advdata_t scan_resp;
    memset(&scan_resp, 0, sizeof(ble_advdata_t));

    scan_resp.uuids_complete = uuid_list;

    err_code = ble_advdata_encode(&scan_resp, s_scan_resp_buffer,
        &(advertising_data->scan_rsp_data.len));
    APP_ERROR_CHECK(err_code);
}

static void adv_params_init(uint8_t* adv_handle,
                            ble_gap_adv_data_t* adv_data)
{
    ble_gap_adv_params_t adv_params;
    memset(&adv_params, 0, sizeof(ble_gap_adv_params_t));

    adv_params.properties.type  = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
    adv_params.p_peer_addr      = NULL;
    adv_params.interval         = 50;
    adv_params.duration         = BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED;
    adv_params.filter_policy    = BLE_GAP_ADV_FP_ANY;
    adv_params.primary_phy      = BLE_GAP_PHY_1MBPS;

    ret_code_t err_code = sd_ble_gap_adv_set_configure(adv_handle,
                                                       adv_data,
                                                       &adv_params);
    APP_ERROR_CHECK(err_code);
}

static void ble_gap_obs_event_handler(const ble_evt_t* event,
                                      void* context)
{
    ret_code_t err_code;
    bool* connected = (bool*)context;

    switch (event->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        #ifdef DEBUG
        NRF_LOG_INFO("GAP connection!");
        #endif /* DEBUG */

        s_conn_handle = event->evt.gap_evt.conn_handle;
        *connected = true;
        bsp_board_led_off(ADVERTISING_LED);
        break;
    case BLE_GAP_EVT_DISCONNECTED:
        #ifdef DEBUG
        NRF_LOG_INFO("GAP disconnection!");
        #endif /* DEBUG */

        connection_end_signal();
        break;
    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
        #ifdef DEBUG
        NRF_LOG_INFO("Security parameters requested: Pairing not supported!");
        #endif /* DEBUG */

        err_code = sd_ble_gap_sec_params_reply(s_conn_handle,
            BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
        APP_ERROR_CHECK(err_code);
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
            err_code = sd_ble_gap_phy_update(s_conn_handle,
                                             &updated_phys);
            APP_ERROR_CHECK(err_code);
        }
        break;
    default:
        break;
    }
}

static void ble_conn_params_evt_handler(ble_conn_params_evt_t* event)
{
    #ifdef DEBUG
    if (event->conn_handle == s_conn_handle
        && event->evt_type == BLE_CONN_PARAMS_EVT_SUCCEEDED)
    {
        NRF_LOG_INFO("Connection parameters negotiation succeeded!");
    }
    else
    {
        NRF_LOG_INFO("Connection parameters negotiation failed!");
    }
    #endif /* DEBUG */
}

static void ble_srv_error_handler(uint32_t error)
{
    APP_ERROR_CHECK(error);
}
