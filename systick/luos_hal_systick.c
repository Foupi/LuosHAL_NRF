#include "luos_hal_systick.h"

#include <stdio.h>

// NRFX DRIVERS
#include "nrfx_systick.h"   /* nrfx_systick_init, nrfx_systick_state_t,
                            ** nrfx_systick_get
                            */

// Amount of bytes in the systick register
#define SYSTICK_NB_BITS 24

// Maximum value of the systick register.
static const uint32_t SYSTICK_MAX_VAL = (1 << SYSTICK_NB_BITS) - 1;

// Systick frequency: 64MHz.
#define TICKS_IN_SECOND ((uint32_t)64000000UL)

// Milliseconds in a second.
#define MS_IN_SECOND    ((uint32_t)1000UL)

// Number of ticks in a millisecond.
static const uint32_t TICKS_IN_MS = TICKS_IN_SECOND / MS_IN_SECOND;

/******************************************************************************
 * @brief Luos HAL general systick tick at 1ms initialize
 * @param None
 * @return tick Counter
 ******************************************************************************/
void LuosHAL_SystickInit(void)
{
    nrfx_systick_init();
}

/******************************************************************************
 * @brief Luos HAL general systick tick at 1ms
 * @param None
 * @return tick Counter
 ******************************************************************************/
uint32_t LuosHAL_GetSystick(void)
{
    nrfx_systick_state_t systick_state;
    nrfx_systick_get(&systick_state);

    uint32_t systick_time = systick_state.time;
    uint32_t inc_val = SYSTICK_MAX_VAL - systick_time;

    return inc_val / TICKS_IN_MS;
}
