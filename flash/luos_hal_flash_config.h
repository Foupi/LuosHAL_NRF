#ifndef LUOS_HAL_FLASH_CONFIG_H
#define LUOS_HAL_FLASH_CONFIG_H

#include <stdint.h> // uint32_t

/*******************************************************************************
 * FLASH CONFIG
 ******************************************************************************/
// Hopefully NRF macros can be found later on, for modularity...

#ifndef PAGE_SIZE
#define PAGE_SIZE                   (uint32_t) 0x1000
#endif
#ifndef ADDRESS_LAST_PAGE_FLASH
#define ADDRESS_LAST_PAGE_FLASH     ((uint32_t)(0x80000 - PAGE_SIZE))
#endif

# endif /* ! LUOS_HAL_FLASH_CONFIG_H */
