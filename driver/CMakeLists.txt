# Options (enabled by default)
option(ENABLE_DRIVER_LED "Enable LED driver" ON)
option(ENABLE_DRIVER_SPI "Enable SPI driver" ON)

# Each submodule can still exist independently
if (ENABLE_DRIVER_LED)
  add_subdirectory(led)
endif()

if (ENABLE_DRIVER_SPI)
  add_subdirectory(spi)
endif()

# Aggregated target: provides a single entry point for the top-level
add_library(drivers INTERFACE)

if (ENABLE_DRIVER_LED)
  target_link_libraries(drivers INTERFACE driver_led)
endif()

if (ENABLE_DRIVER_SPI)
  target_link_libraries(drivers INTERFACE driver_spi)
endif()
