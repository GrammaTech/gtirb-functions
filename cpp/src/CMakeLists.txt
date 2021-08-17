# Library and headers
configure_file("${CMAKE_SOURCE_DIR}/include/gtirb_functions/version.h.in"
               "${CMAKE_BINARY_DIR}/include/gtirb_functions/version.h" @ONLY)

set(GTIRB_FUNCTION_HEADERS
    "${CMAKE_SOURCE_DIR}/include/gtirb_functions/gtirb_functions.hpp"
    "${CMAKE_BINARY_DIR}/include/gtirb_functions/version.h")

add_library(gtirb_functions ${CMAKE_CURRENT_SOURCE_DIR}/gtirb_functions.cpp
                            ${GTIRB_FUNCTION_HEADERS})

set_target_properties(
  gtirb_functions PROPERTIES VERSION ${GTIRB_FUNCTIONS_VERSION}
                             SOVERSION ${GTIRB_FUNCTION_MAJOR_VERSION})


target_include_directories(
  gtirb_functions PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
target_include_directories(gtirb_functions PUBLIC /usr/include)
# target_include_directories(
#  gtirb_functions PUBLIC $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)


install_linux_debug_info(gtirb_functions gtirb_functions-debug-file)

install(
  TARGETS gtirb_functions
  EXPORT gtirb_functionsTargets
  COMPONENT library
  INCLUDES
  DESTINATION include
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install(
  FILES ${GTIRB_FUNCTION_HEADERS}
  DESTINATION include/gtirb_functions
  COMPONENT headers)

# Driver
# add_executable(gtirb_functions-driver ${GTIRB_FUNCTION_HEADERS}
#                                       ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

# target_link_libraries(gtirb_functions-driver PRIVATE gtirb_functions)

# install_linux_debug_info(gtirb_functions-driver
#                          gtirb_functions-driver-debug-file)

# install(
#   TARGETS gtirb_functions-driver
#  DESTINATION bin
#  COMPONENT driver)

# Tests
if(GTIRB_FUNCTION_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
