#ifndef LUOS_HAL_BLE_H
#define LUOS_HAL_BLE_H

// Initializes and enables the BLE stack.
void LuosHAL_BleInit(void);

// Initializes parameters for establishing a connection.
void LuosHAL_BleSetup(void);

// Blocks until connection is established.
void LuosHAL_BleConnect(void);

#endif /* ! LUOS_HAL_BLE_H */
