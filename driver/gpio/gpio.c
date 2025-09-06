/**
 * Copyright (c) 2025 Steve Chang
 *
 * SPDX-License-Identifier: MIT
 */

#include "gpio.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include <stdio.h>

int gpio_irq_init(const struct gpio_config *gpio_cfg)
{
    printf("%s\n", __func__);
    if ((gpio_cfg == NULL))
        return -1;

    gpio_init(gpio_cfg->pin);
    gpio_set_irq_enabled_with_callback(gpio_cfg->pin, gpio_cfg->event_mask,
        gpio_cfg->enabled, gpio_cfg->callback);

    return 0;
}
