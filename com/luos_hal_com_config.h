#ifndef LUOS_HAL_COM_CONFIG_H
#define LUOS_HAL_COM_CONFIG_H

#include "luos_hal_config.h"    // DISABLE

// Stubs
void com_clock_enable(void);
void com_irq_handler(void);

//COM pin definition
#ifndef TX_LOCK_DETECT_PIN
#define TX_LOCK_DETECT_PIN  DISABLE
#endif
#ifndef TX_LOCK_DETECT_PORT
#define TX_LOCK_DETECT_PORT DISABLE
#endif
#ifndef TX_LOCK_DETECT_IRQ
#define TX_LOCK_DETECT_IRQ  DISABLE
#endif

#ifndef RX_EN_PIN
#define RX_EN_PIN           DISABLE
#endif
#ifndef RX_EN_PORT
#define RX_EN_PORT          DISABLE
#endif

#ifndef TX_EN_PIN
#define TX_EN_PIN           DISABLE
#endif
#ifndef TX_EN_PORT
#define TX_EN_PORT          DISABLE
#endif

#ifndef COM_TX_PIN
#define COM_TX_PIN          DISABLE
#endif
#ifndef COM_TX_PORT
#define COM_TX_PORT         DISABLE
#endif
#ifndef COM_TX_AF
#define COM_TX_AF           DISABLE
#endif

#ifndef COM_RX_PIN
#define COM_RX_PIN          DISABLE
#endif
#ifndef COM_RX_PORT
#define COM_RX_PORT         DISABLE
#endif
#ifndef COM_RX_AF
#define COM_RX_AF           DISABLE
#endif

/*******************************************************************************
 * COM CONFIG
 ******************************************************************************/
#ifndef LUOS_COM_CLOCK_ENABLE
#define LUOS_COM_CLOCK_ENABLE() com_clock_enable()
#endif
#ifndef LUOS_COM
#define LUOS_COM                DISABLE
#endif
#ifndef LUOS_COM_IRQ
#define LUOS_COM_IRQ            DISABLE
#endif
#ifndef LUOS_COM_IRQHANDLER
#define LUOS_COM_IRQHANDLER()   com_irq_handler()
#endif


#endif /* ! LUOS_HAL_COM_CONFIG_H */
