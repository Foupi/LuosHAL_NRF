#ifndef LUOS_HAL_BLE_CLIENT_CTX_H
#define LUOS_HAL_BLE_CLIENT_CTX_H

/*      INCLUDES                                                    */

// NRF
#include "ble_nus_c.h"  // ble_nus_c_t

// CUSTOM
#include "ptp_client.h" // ptp_client_t

/* FIXME    This is a dirty plaster for solving a DB Discovery issue.
**          Long story short, the BLE HAL module needs to dispatch DB
**          discovery events among BLE client instances, but the PTP and
**          NUS client instances are declared as static in their
**          respective HAL modules, and therefore not accessible from
**          the BLE HAL module. This solution allows access to the
**          client instance all while keeping the PTP client code
**          idiomatic according to NRF standards.
*/

// Global accessor for PTP instance.
extern ptp_client_t*    g_ptp_client_ptr;

// Global accessor for NUS client instance.
extern ble_nus_c_t*     g_nus_c_ptr;

#endif /* ! LUOS_HAL_BLE_CLIENT_CTX_H */
