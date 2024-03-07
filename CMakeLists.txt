cmake_minimum_required(VERSION 3.27)
project(exam_environment C)

set(CMAKE_C_STANDARD 99)

add_executable(exam_environment main.c)

# Test Configurations
configure_file(student_tests.json student_tests.json COPYONLY)
add_executable(student_test test/student_test.c cjson/cJSON.c)
enable_testing()
add_test(
        NAME student_tests
        COMMAND student_test
)