# Library and headers
configure_file("${CMAKE_SOURCE_DIR}/include/gtirb_functions/version.h.in"
               "${CMAKE_BINARY_DIR}/include/gtirb_functions/version.h" @ONLY)

set(GTIRB_FUNCTION_HEADERS
    "${CMAKE_SOURCE_DIR}/include/gtirb_functions/gtirb_functions.hpp"
    "${CMAKE_BINARY_DIR}/include/gtirb_functions/version.h")

add_library(gtirb-functions ${CMAKE_CURRENT_SOURCE_DIR}/gtirb_functions.cpp
                            ${GTIRB_FUNCTION_HEADERS})

set_target_properties(
  gtirb-functions PROPERTIES VERSION ${GTIRB_FUNCTIONS_VERSION}
                             SOVERSION ${GTIRB_FUNCTIONS_MAJOR_VERSION})

target_compile_options(gtirb-functions PUBLIC -Wno-shadow -Wno-deprecated-copy
                                              -Wno-unused-local-typedefs)

target_include_directories(
  gtirb-functions PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

target_link_libraries(gtirb-functions PUBLIC gtirb)

target_include_directories(
  gtirb-functions PUBLIC $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)

install_linux_debug_info(gtirb-functions gtirb-functions-debug-file)

install(
  TARGETS gtirb-functions
  EXPORT gtirb-functionsTargets
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

# Tests
if(GTIRB_FUNCTIONS_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
