#ifndef LUOS_HAL_TIMER_CONFIG_H
#define LUOS_HAL_TIMER_CONFIG_H

#include "luos_hal_config.h"    // DISABLE

// Stub
void timer_irq_handler(void);

/*******************************************************************************
 * COM TIMEOUT CONFIG
 ******************************************************************************/
#ifndef LUOS_TIMER_CLOCK_ENABLE
#define LUOS_TIMER_CLOCK_ENABLE()         do { \
                                    \
                                      } while(0U)
#endif
#ifndef LUOS_TIMER
#define LUOS_TIMER              DISABLE
#endif
#ifndef LUOS_TIMER_IRQ
#define LUOS_TIMER_IRQ          DISABLE
#endif
#ifndef LUOS_TIMER_IRQHANDLER
#define LUOS_TIMER_IRQHANDLER() timer_irq_handler()
#endif

#endif /* ! LUOS_HAL_TIMER_CONFIG_H */
