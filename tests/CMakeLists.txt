project(tests)

enable_testing()
find_package(GTest CONFIG REQUIRED)

include_directories(../include)
aux_source_directory(. DIR_SRCS)
add_executable(${PROJECT_NAME} ${DIR_SRCS})

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${CMAKE_PROJECT_NAME}
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        GTest::gmock_main)
