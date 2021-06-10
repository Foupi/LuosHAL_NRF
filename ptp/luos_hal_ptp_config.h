#ifndef LUOS_HAL_PTP_CONFIG_H
#define LUOS_HAL_PTP_CONFIG_H

#include <stdint.h>             // uint16_t

#include "luos_hal_config.h"    // DISABLE

// Stub
void pinout_irq_handler(uint16_t pin);

//PTP pin definition
#ifndef PTPA_PIN
#define PTPA_PIN    0x0001
#endif
#ifndef PTPA_PORT
#define PTPA_PORT   DISABLE
#endif
#ifndef PTPA_IRQ
#define PTPA_IRQ    DISABLE
#endif

#ifndef PTPB_PIN
#define PTPB_PIN    DISABLE
#endif
#ifndef PTPB_PORT
#define PTPB_PORT   DISABLE
#endif
#ifndef PTPB_IRQ
#define PTPB_IRQ    DISABLE
#endif

#ifndef PINOUT_IRQHANDLER
#define PINOUT_IRQHANDLER(PIN)  pinout_irq_handler(PIN)
#endif

#endif /* ! LUOS_HAL_PTP_CONFIG_H */
