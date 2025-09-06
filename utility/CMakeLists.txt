add_library(utility_print STATIC
  print.c
)

target_include_directories(utility_print PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}
)
