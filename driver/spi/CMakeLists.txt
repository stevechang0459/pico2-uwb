add_library(driver_spi STATIC
  spi.c
)

# Expose this directory for #include "spi.h"
target_include_directories(driver_spi PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}
)

# Link required libraries
target_link_libraries(driver_spi PUBLIC
  pico_stdlib
  hardware_spi
  utility_print
)

# Optionally link to LED driver if enabled
if (ENABLE_DRIVER_LED)
  target_link_libraries(driver_spi PUBLIC driver_led)
  target_compile_definitions(driver_spi PUBLIC DRIVER_SPI_WITH_LED=1)
endif()
