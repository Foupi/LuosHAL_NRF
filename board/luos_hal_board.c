#include "luos_hal_board.h"

/*      INCLUDES                                                    */

// C STANDARD
#include <stdbool.h>                    // bool

// NRF
#include "boards.h"                     // bsp_board_init, BSP_INIT_LEDS
#include "nrf_sdh.h"                    /* nrf_sdh_enable_request,
                                        ** nrf_sdh_is_enabled
                                        */
#include "sdk_errors.h"                 // ret_code_t

#ifdef DEBUG
#include "nrf_log.h"                    // NRF_LOG_INFO
#include "nrf_log_ctrl.h"               // NRF_LOG_INIT
#include "nrf_log_default_backends.h"   // NRF_LOG_DEFAULT_BACKENDS_INIT
#endif /* DEBUG */

// NRF APPS
#include "app_error.h"                  // APP_ERROR_CHECK
#include "app_timer.h"                  // app_timer_init

/*      STATIC FUNCTIONS                                            */

// Initializes logging.
static void log_init(void);

// Initializes the timer module.
static void timer_init(void);

// Initializes LEDs.
static void leds_init(void);

// Initializes the SoftDevice.
static void softdevice_init(void);

void LuosHAL_BoardInit(void)
{
    // Log init
    log_init();

    // Timer init
    timer_init();

    // LEDs init
    leds_init();

    // SoftDevice init
    softdevice_init();
}

static void log_init(void)
{
    #ifdef DEBUG
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
    #endif /* DEBUG */

}

static void timer_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}

static void leds_init(void)
{
    bsp_board_init(BSP_INIT_LEDS);
}

static void softdevice_init(void)
{
    ret_code_t err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    bool sd_enabled;
    do
    {
        sd_enabled = nrf_sdh_is_enabled();
    } while (!sd_enabled);
}

#ifdef DEBUG
void node_assert(char* file, uint32_t line)
{
    NRF_LOG_INFO("Luos assert on file %s, line %u!", file, line);
}
#endif /* DEBUG */
