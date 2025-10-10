#include "gpio.h"
#include "hardware/gpio.h"

#include "pico/stdlib.h"

int gpio_irq_init(const struct gpio_irq_config *irq_cfg)
{
    gpio_init(irq_cfg->gpio);
    gpio_set_irq_enabled_with_callback(irq_cfg->gpio, irq_cfg->event_mask,
        irq_cfg->enabled, irq_cfg->callback);

    return 0;
}
