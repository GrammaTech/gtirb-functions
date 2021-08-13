enable_testing()

include_directories(${GTEST_INCLUDE_DIRS})

add_executable(hello_test hello.test.cpp main.test.cpp)

target_link_libraries(hello_test gtest gtest_main hello)

add_test(NAME hello_test COMMAND hello_test)

add_test(NAME testhello COMMAND $<TARGET_FILE:hello_test>)
