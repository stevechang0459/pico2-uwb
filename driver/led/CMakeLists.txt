add_library(driver_led STATIC
  led.c
)

# Expose this directory for #include "led.h"
target_include_directories(driver_led PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}
)

# Link required libraries
target_link_libraries(driver_led PUBLIC
  pico_stdlib
)

# Only available on Pico W / Pico2 W
if (PICO_CYW43_SUPPORTED)
  target_link_libraries(driver_led PUBLIC pico_cyw43_arch_none)
endif()
