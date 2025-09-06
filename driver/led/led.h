#ifndef LED_H
#define LED_H

#include <stdbool.h>

#ifndef LED_DELAY_MS
// #define LED_DELAY_MS 250
#define LED_DELAY_MS 100
#endif

int pico_led_init(void);
void pico_set_led(bool led_on);

#endif  // ~ LED_H
