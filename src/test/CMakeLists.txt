enable_testing()
# TODO: get coverage working

include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/include)

add_compile_options(-Wno-shadow -Wno-deprecated-copy -Wno-unused-local-typedefs)

add_executable(gtfunctions_test gtirb-functions.test.cpp main.test.cpp)

target_link_libraries(gtfunctions_test gtest gtest_main gtirb-functions gtirb)

add_test(NAME gtfunctions_test COMMAND gtfunctions_test)

add_test(NAME testgfunctions COMMAND $<TARGET_FILE:gtfunctions_test>)
