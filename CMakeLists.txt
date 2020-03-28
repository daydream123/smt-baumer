cmake_minimum_required(VERSION 3.0.0)

SET(CMAKE_TOOLCHAIN_FILE "/home/zhangfei/workspace/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE PATH "" FORCE)

project(smt-baumer)

# change output dirs
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_DEBUG_POSTFIX d)

include_directories(include)

add_subdirectory(src)
add_subdirectory(tests)

# for test
SET(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install)