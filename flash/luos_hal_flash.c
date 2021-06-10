/*      INCLUDES                                                    */

#include "luos_hal_flash.h"
#include "luos_hal.h"

// C STANDARD
#include <stdint.h>             // uint32_t
#include <string.h>             // memcpy

// NRF
#include "nrf_fstorage.h"       /* nrf_fstorage_evt_t, NRF_FSTORAGE_DEF
                                ** nrf_fstorage_t, nrf_fstorage_init,
                                ** nrf_fstorage_read,
                                ** nrf_fstorage_erase,
                                ** nrf_fstorage_write,
                                ** nrf_fstorage_is_busy
                                */
#include "nrf_fstorage_sd.h"    // nrf_storage_sd
#include "nrf_soc.h"            // sd_app_evt_wait

#ifdef DEBUG
#include "nrf_log.h"            // NRF_LOG_INFO
#endif /* DEBUG */

// NRF APPS
#include "app_error.h"          // APP_ERROR_CHECK

/*      STATIC FUNCTIONS                                            */

// Wait until flash memory is not busy anymore.
static void LuosHAL_WaitFlashReady(void);

static void LuosHAL_FlashEraseLuosMemoryInfo(void);

// Stores Luos persistent data in the given buffer.
static void LuosHAL_FlashReadAliasesPage(uint8_t* buffer);

// Logs the event on the UART.
static void LuosHAL_FStorageEvtHandler(nrf_fstorage_evt_t* event);

/*      INITIALIZATIONS                                             */

// Init fstorage
NRF_FSTORAGE_DEF(nrf_fstorage_t s_fstorage_inst) =
{
    .evt_handler    = LuosHAL_FStorageEvtHandler,
    .start_addr     = ADDRESS_ALIASES_FLASH,
    .end_addr       = ADDRESS_ALIASES_FLASH + PAGE_SIZE,
};

/******************************************************************************
 * @brief Flash Initialisation
 * @param None
 * @return None
 ******************************************************************************/
void LuosHAL_FlashInit(void)
{
    uint32_t err_code = nrf_fstorage_init(
        &s_fstorage_inst,
        &nrf_fstorage_sd,
        NULL
    );
    APP_ERROR_CHECK(err_code);
}

static void LuosHAL_WaitFlashReady(void)
{
    #ifdef DEBUG
    NRF_LOG_INFO("Waiting for flash to be ready...\r\n");
    #endif /* DEBUG */

    bool fstorage_busy;
    do
    {
        fstorage_busy = nrf_fstorage_is_busy(&s_fstorage_inst);
        (void)sd_app_evt_wait();
    } while (fstorage_busy);

    #ifdef DEBUG
    NRF_LOG_INFO("Flash is available!\r\n");
    #endif /* DEBUG */
}

/******************************************************************************
 * @brief Erase flash page where Luos keep permanente information
 * @param None
 * @return None
 ******************************************************************************/
static void LuosHAL_FlashEraseLuosMemoryInfo(void)
{
    uint32_t page_error = 0;
    //routine to erase flash page

    #ifdef DEBUG
    NRF_LOG_INFO("Erasing aliases page...\r\n");
    #endif /* DEBUG */

    page_error = nrf_fstorage_erase(
        &s_fstorage_inst,
        ADDRESS_ALIASES_FLASH,
        1,
        NULL
    );
    APP_ERROR_CHECK(page_error);
    LuosHAL_WaitFlashReady();
}

static void LuosHAL_FlashReadAliasesPage(uint8_t* buffer)
{
    #ifdef DEBUG
    NRF_LOG_INFO("Reading aliases page...\r\n");
    #endif /* DEBUG */

    uint32_t err_code = nrf_fstorage_read(
        &s_fstorage_inst,
        ADDRESS_ALIASES_FLASH,
        buffer,
        PAGE_SIZE
    );
    APP_ERROR_CHECK(err_code);
}

/******************************************************************************
 * @brief Write flash page where Luos keep permanente information
 * @param Address page / size to write / pointer to data to write
 * @return
 ******************************************************************************/
void LuosHAL_FlashWriteLuosMemoryInfo(uint32_t addr, uint16_t size, uint8_t *data)
{
    // Before writing we have to erase the entire page
    // to do that we have to backup current falues by copying it into RAM
    uint8_t page_backup[PAGE_SIZE] = { '\0' };
    LuosHAL_FlashReadAliasesPage(page_backup);

    // Now we can erase the page
    LuosHAL_FlashEraseLuosMemoryInfo();

    // Then add input data into backuped value on RAM
    uint32_t RAMaddr = (addr - ADDRESS_ALIASES_FLASH);
    memcpy(&page_backup[RAMaddr], data, size);

    #ifdef DEBUG
    NRF_LOG_INFO("Writing data in flash...\r\n");
    #endif /* DEBUG */

    // and copy it into flash
    uint32_t err_code = nrf_fstorage_write(
        &s_fstorage_inst,
        ADDRESS_ALIASES_FLASH,
        page_backup,
        PAGE_SIZE,
        NULL
    );
    APP_ERROR_CHECK(err_code);
    LuosHAL_WaitFlashReady();
}

/******************************************************************************
 * @brief read information from page where Luos keep permanente information
 * @param Address info / size to read / pointer callback data to read
 * @return
 ******************************************************************************/
void LuosHAL_FlashReadLuosMemoryInfo(uint32_t addr, uint16_t size, uint8_t *data)
{
    // Create buffer for page.
    uint8_t page[PAGE_SIZE] = { 0 };
    LuosHAL_FlashReadAliasesPage(page);

    uint32_t RAMaddr = addr - ADDRESS_ALIASES_FLASH;
    memcpy(data, page + RAMaddr, size);
}

static void LuosHAL_FStorageEvtHandler(nrf_fstorage_evt_t* event)
{
    #ifdef DEBUG
    switch (event->id)
    {
    case NRF_FSTORAGE_EVT_ERASE_RESULT:
        NRF_LOG_INFO("Erase event at address 0x%lx!\r\n", event->addr);
        break;
    case NRF_FSTORAGE_EVT_WRITE_RESULT:
        NRF_LOG_INFO("Write event: %lu bytes at address 0x%lx!\r\n",
                      event->len, event->addr);
        break;
    default:
        break;
    }
    #endif /* DEBUG */
}
