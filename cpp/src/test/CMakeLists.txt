enable_testing()

include_directories(${GTEST_INCLUDE_DIRS})

add_executable(gtfunctions_test gtirb-functions.test.cpp main.test.cpp)

target_link_libraries(gtfunctions_test gtest gtest_main  gtirb_functions gtirb)

target_link_directories(gtfunctions_test PUBLIC ${CMAKE_SOURCE_DIR}/lib)


add_test(NAME gtfunctions_test COMMAND gtfunctions_test)

add_test(NAME testgfunctions COMMAND $<TARGET_FILE:gtfunctions_test>)
