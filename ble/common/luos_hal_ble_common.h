#ifndef LUOS_HAL_BLE_COMMON_H
#define LUOS_HAL_BLE_COMMON_H

/*      INCLUDES                                                    */

// C STANDARD
#include <stdint.h>         // uint16_t

// NRF
#include "nrf_ble_gatt.h"   // nrf_ble_gatt_t
#include "nrf_sdh_ble.h"    // nrf_sdh_ble_evt_handler_t
#include "sdk_errors.h"     // ret_code_t

/*      CONSTANTS                                                   */

// Name of the Luos BLE server.
#define LUOS_SERVER_NAME    "Luos Actuator"

// Connection configuration tag.
#define CONN_CFG_TAG        1

// Function updating the ATT MTU size.
typedef ret_code_t(*att_mtu_update_t)(nrf_ble_gatt_t*, uint16_t);

// Enables the BLE stack.
void ble_stack_enable(void);

// Initializes the GATT module.
void gatt_module_init(att_mtu_update_t att_mtu_update);

// Blocks until a BLE connection is established.
void ble_connection_wait(void);

// Registers the GAP and GATT BLE observers.
void ble_observers_register(const nrf_sdh_ble_evt_handler_t ble_gap_obs_cb);

// Signals end of connection and goes in infinite loop.
void connection_end_signal(void);

#endif /* ! LUOS_HAL_BLE_COMMON_H */
