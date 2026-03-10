# Library and headers
configure_file("${CMAKE_SOURCE_DIR}/include/gtirb_functions/version.h.in"
               "${CMAKE_BINARY_DIR}/include/gtirb_functions/version.h" @ONLY)

set(GTIRB_FUNCTION_HEADERS
    "${CMAKE_SOURCE_DIR}/include/gtirb_functions/gtirb_functions.hpp"
    "${CMAKE_BINARY_DIR}/include/gtirb_functions/version.h")

add_library(gtirb-functions INTERFACE)

# set_target_properties( gtirb-functions PROPERTIES VERSION
# ${GTIRB_FUNCTIONS_VERSION} SOVERSION ${GTIRB_FUNCTIONS_MAJOR_VERSION})

target_include_directories(
  gtirb-functions INTERFACE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
                            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_compile_features(gtirb-functions INTERFACE cxx_std_17)
target_link_libraries(gtirb-functions INTERFACE gtirb)

install(
  TARGETS gtirb-functions
  EXPORT gtirb-functionsTargets
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

if(NOT ENABLE_DEBUG)
  add_compile_options(-DNDEBUG)
endif()
