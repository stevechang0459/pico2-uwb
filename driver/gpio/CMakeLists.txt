# pico-2-uwb/driver/gpio/CMakeLists.txt
add_library(driver_gpio STATIC
  gpio.c
)

# Expose this directory for #include "gpio.h"
target_include_directories(driver_gpio PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}
)

# Link required libraries
target_link_libraries(driver_gpio PUBLIC
  pico_stdlib
)
