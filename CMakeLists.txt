# * Need 3.10 to support CXX_STANDARD=17
# * Need 3.13 to support "add_link_options(--coverage)"

cmake_minimum_required(VERSION 3.10.0)

# Read version.txt and setup project
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" VERSION_FILE)

string(REGEX MATCH "VERSION_MAJOR ([0-9]*)" _ ${VERSION_FILE})
set(GTIRB_FUNCTIONS_MAJOR_VERSION ${CMAKE_MATCH_1})

string(REGEX MATCH "VERSION_MINOR ([0-9]*)" _ ${VERSION_FILE})
set(GTIRB_FUNCTIONS_MINOR_VERSION ${CMAKE_MATCH_1})

string(REGEX MATCH "VERSION_PATCH ([0-9]*)" _ ${VERSION_FILE})
set(GTIRB_FUNCTIONS_PATCH_VERSION ${CMAKE_MATCH_1})

cmake_policy(SET CMP0048 NEW)
project(
  gtirb-functions
  VERSION
    "${GTIRB_FUNCTIONS_MAJOR_VERSION}.${GTIRB_FUNCTIONS_MINOR_VERSION}.${GTIRB_FUNCTIONS_PATCH_VERSION}"
)

if(NOT DEFINED CMAKE_PROJECT_VERSION)
  set(CMAKE_PROJECT_VERSION ${PROJECT_VERSION})
  set(VERSION ${PROJECT_VERSION})
endif()

set(GTIRB_FUNCTIONS_VERSION ${CMAKE_PROJECT_VERSION})

option(ENABLE_CONAN "Use Conan to inject dependencies" OFF)

option(ENABLE_CODE_COVERAGE
       "Build with instrumentation for collecting code coverage" OFF)

if(ENABLE_CODE_COVERAGE)
  if(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU OR ${CMAKE_CXX_COMPILER_ID} STREQUAL
                                              Clang)
    add_compile_options(--coverage)
    add_link_options(--coverage)
  else()
    message(FATAL_ERROR "no support for code coverage on this target")
  endif()
endif()

if(ENABLE_CONAN)

  include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
  conan_basic_setup()
endif()

# ---------------------------------------------------------------------------
# Build options
# ---------------------------------------------------------------------------

# Wrapper for BUILD_SHARED_LIBS which defaults to 'ON'
option(GTIRB_FUNCTIONS_BUILD_SHARED_LIBS "Build shared libraries." ON)
if(GTIRB_FUNCTIONS_BUILD_SHARED_LIBS)
  set(BUILD_SHARED_LIBS ON)
else()
  set(BUILD_SHARED_LIBS OFF)
endif()

option(GTIRB_FUNCTIONS_ENABLE_TESTS "Enable build and running unit tests." OFF)
option(ENABLE_DEBUG OFF)

# Determine whether or not to strip debug symbols and set the build-id. This is
# only really needed when we are building ubuntu *-dbg packages
option(GTIRB_FUNCTIONS_STRIP_DEBUG_SYMBOLS
       "Whether or not to strip debug symbols and set the build-id." OFF)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
find_package(gtirb REQUIRED)

# ---------------------------------------------------------------------------
# Global settings
# ---------------------------------------------------------------------------

# set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
# Error if it's not available
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
  set(CMAKE_DEBUG_POSTFIX
      "d"
      CACHE STRING "add a postfix, usually d on windows")
endif()
set(CMAKE_RELEASE_POSTFIX
    ""
    CACHE STRING "add a postfix, usually empty on windows")
set(CMAKE_RELWITHDEBINFO_POSTFIX
    ""
    CACHE STRING "add a postfix, usually empty on windows")
set(CMAKE_MINSIZEREL_POSTFIX
    ""
    CACHE STRING "add a postfix, usually empty on windows")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---------------------------------------------------------------------------
# Compile options
# ---------------------------------------------------------------------------
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
  add_compile_options(-D_MBCS)
  add_compile_options(-D_UNICODE)
  add_compile_options(-DUNICODE)
  add_compile_options(-D_WIN32)

  # Enabled a sensible warning level and treat all warnings as errors.
  add_compile_options(-W4)
  add_compile_options(-WX)

  add_compile_options(-EHsc) # Enable exceptions
  add_compile_options(-sdl) # Enable extra security checks
  add_compile_options(-permissive-) # Disable permissive mode
  add_compile_options($<$<CONFIG:Release>:-GL>) # Enable whole program
                                                # optimization
  add_link_options($<$<CONFIG:Release>:-ltcg>) # Enable link-time code
                                               # generation
elseif((${CMAKE_CXX_COMPILER_ID} STREQUAL GNU) OR (${CMAKE_CXX_COMPILER_ID}
                                                   STREQUAL Clang))
  add_compile_options(-Wall -Wextra -Wpointer-arith -Wshadow -Werror)
  add_compile_options(-fPIC)
endif()

# ---------------------------------------------------------------------------
# Google Test Application
# ---------------------------------------------------------------------------
if(GTIRB_FUNCTIONS_ENABLE_TESTS)
  configure_file(CMakeLists.googletest googletest-download/CMakeLists.txt)

  execute_process(
    COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download")

  if(result)
    message(WARNING "CMake step for googletest failed: ${result}")
  endif()

  execute_process(
    COMMAND "${CMAKE_COMMAND}" --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download")

  if(result)
    message(WARNING "Build step for googletest failed: ${result}")
  endif()

  # Prevent overriding the parent project's compiler/linker settings on Windows
  set(gtest_force_shared_crt
      ON
      CACHE BOOL "" FORCE)

  # Add googletest directly to our build. This defines the gtest and gtest_main
  # targets.
  add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src"
                   "${CMAKE_BINARY_DIR}/googletest-build" EXCLUDE_FROM_ALL)

  include_directories("${gtest_SOURCE_DIR}/include")

  enable_testing()
endif()

# ---------------------------------------------------------------------------
# Source files
# ---------------------------------------------------------------------------
function(install_linux_debug_info TARGET COMPONENT_NAME)
  if(UNIX
     AND NOT CYGWIN
     AND ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo"
          OR "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
     AND ${GTIRB_FUNCTIONS_STRIP_DEBUG_SYMBOLS})
    string(
      RANDOM
      LENGTH 32
      ALPHABET "abcdef0123456789" BUILD_ID)
    string(SUBSTRING "${BUILD_ID}" 0 2 BUILD_ID_PREFIX)
    string(SUBSTRING "${BUILD_ID}" 2 32 BUILD_ID_SUFFIX)
    target_link_libraries(${TARGET} PRIVATE "-Wl,--build-id=0x${BUILD_ID}")
    add_custom_command(
      TARGET ${TARGET}
      POST_BUILD
      COMMAND objcopy --only-keep-debug $<TARGET_FILE:${TARGET}>
              ${CMAKE_BINARY_DIR}/bin/${BUILD_ID_SUFFIX}.debug
      COMMAND objcopy --strip-debug $<TARGET_FILE:${TARGET}>)
    install(
      FILES "${CMAKE_BINARY_DIR}/bin/${BUILD_ID_SUFFIX}.debug"
      COMPONENT "${COMPONENT_NAME}"
      DESTINATION "lib/debug/.build-id/${BUILD_ID_PREFIX}")
  endif()
endfunction()

# Add src subdirectory which contains targets
add_subdirectory(src)
# ---------------------------------------------------------------------------
# Export config for use by other CMake projects
# ---------------------------------------------------------------------------
export(TARGETS gtirb-functions
       FILE "${CMAKE_CURRENT_BINARY_DIR}/gtirbFunctionsTargets.cmake")

# Build tree config
configure_file("${CMAKE_CURRENT_LIST_DIR}/gtirbFunctionsConfig.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/gtirbFunctionsConfig.cmake")

# Install tree config
configure_file("${CMAKE_CURRENT_LIST_DIR}/gtirbFunctionsConfig.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/export/gtirbFunctionsConfig.cmake")
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/export/gtirbFunctionsConfig.cmake
  DESTINATION lib/gtirb-functions
  COMPONENT cmake_config)
install(
  EXPORT gtirb-functionsTargets
  DESTINATION lib/gtirb-functions
  COMPONENT cmake_target)

export(PACKAGE gtirb-functions)

# ---------------------------------------------------------------------------
# Debian/RPM package generation
# ---------------------------------------------------------------------------
set(CPACK_PROJECT_CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cpack-config.cmake)
set(CPACK_GTIRB_VERSION ${gtirb_VERSION})
math(EXPR next_gtirb_patch "${gtirb_PATCH_VERSION}+1")
set(CPACK_NEXT_GTIRB_VERSION
    ${gtirb_MAJOR_VERSION}.${gtirb_MINOR_VERSION}.${next_gtirb_patch})
set(CPACK_PACKAGE_VERSION_MAJOR ${GTIRB_FUNCTIONS_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${GTIRB_FUNCTIONS_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${GTIRB_FUNCTIONS_PATCH_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "A convenience library for working with functions in GTIRB")
set(CPACK_PACKAGE_VENDOR "GrammaTech Inc.")
set(CPACK_PACKAGE_CONTACT adavis@grammatech.com)
set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
set(CPACK_PACKAGE_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt)

set(CPACK_DEBIAN_PACKAGE_SECTION devel)
set(CPACK_GTIRB_FUNCTIONS_VERSION "${GTIRB_FUNCTIONS_VERSION}")

include(CPack)
