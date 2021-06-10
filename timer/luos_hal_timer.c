/*      INCLUDES                                                    */

#include "luos_hal_timer.h"
#include "luos_hal.h"

// C STANDARD
#include <stdbool.h>    // bool
#include <stdint.h>     // uint16_t

// NRF
#include "sdk_errors.h" // ret_code_t

#ifdef DEBUG
#include "nrf_log.h"    // NRF_LOG_INFO
#endif /* DEBUG */

// NRF APPS
#include "app_error.h"  // APP_ERROR_CHECK
#include "app_timer.h"  // APP_TIMER_*, app_timer_*

// LUOS
#include "context.h"    // ctx
#include "reception.h"  // Recep_Timeout

/*      INITIALIZATIONS                                             */

// Timer instance.
APP_TIMER_DEF(s_timer);

/*      STATIC FUNCTIONS                                            */

static inline void LuosHAL_ComTimeout(void);

/*      CALLBACKS                                                   */

// Calls the Luos Timer interruption handler. Context is not used.
static void LuosHAL_TimerEventHandler(void* context);


/******************************************************************************
 * @brief Luos Timeout initialisation
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_TimeoutInit(void)
{
    ret_code_t err_code = app_timer_create(&s_timer,
                                           APP_TIMER_MODE_SINGLE_SHOT,
                                           LuosHAL_TimerEventHandler);
    APP_ERROR_CHECK(err_code);
}

/******************************************************************************
 * @brief Luos Timeout communication
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_ResetTimeout(uint16_t nbrbit)
{
    ret_code_t err_code;

    err_code = app_timer_stop(s_timer);
    APP_ERROR_CHECK(err_code);

    if (nbrbit != 0)
    {
        const uint32_t nb_ticks = APP_TIMER_TICKS(nbrbit);
        err_code = app_timer_start(s_timer, nb_ticks, NULL);
        APP_ERROR_CHECK(err_code);
    }
}

void LUOS_TIMER_IRQHANDLER()
{
    LuosHAL_ComTimeout();
}

/******************************************************************************
 * @brief Luos Timeout for Rx communication
 * @param None
 * @return None
 ******************************************************************************/
static inline void LuosHAL_ComTimeout(void)
{
    ret_code_t err_code = app_timer_stop(s_timer);
    APP_ERROR_CHECK(err_code);

    if (ctx.tx.lock == true)
    {
        Recep_Timeout();
    }
}

static void LuosHAL_TimerEventHandler(void* context)
{
    #ifdef DEBUG
    NRF_LOG_INFO("COM timeout!");
    #endif /* DEBUG */

    LUOS_TIMER_IRQHANDLER();
}
