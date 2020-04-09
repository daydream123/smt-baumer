project(smt-baumer VERSION 1.0.0)

if(UNIX)
    add_definitions("-D_GNULINUX")
endif()

find_package(Baumer MODULE REQUIRED)
find_package(smt-logger CONFIG REQUIRED)

include_directories(${Baumer_INCLUDE_DIRS})
aux_source_directory(. DIR_SRCS)

if(UNIX)
    add_library(${PROJECT_NAME} SHARED ${DIR_SRCS})
elseif(WIN32)
    add_library(${PROJECT_NAME} STATIC ${DIR_SRCS})
elseif(APPLE)
    add_library(${PROJECT_NAME} SHARED ${DIR_SRCS})
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
        PRIVATE ${Baumer_LIBRARIES}
        PRIVATE Leica::smt-logger)

# ------------------------------- install script -------------------------------

# Configuration
set(PROJECT_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake")
set(VERSION_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake")

# Add definitions for targets
# Values:
#   * Debug: -Dxxx_DEBUG=1
#   * Release: -Dxxx_DEBUG=0
#   * other: -Dxxx_DEBUG=0
target_compile_definitions(${PROJECT_NAME} PUBLIC smt_baumer_DEBUG=$<CONFIG:Debug>)

# Include module with function 'write_basic_package_version_file'
# Configure 'xxxConfigVersion.cmake'
include(CMakePackageConfigHelpers)
write_basic_package_version_file(${VERSION_CONFIG}
        VERSION ${PACKAGE_VERSION}
        COMPATIBILITY SameMajorVersion)

# Configure 'xxxConfig.cmake'
configure_package_config_file(Config.cmake.in ${PROJECT_CONFIG}
        INSTALL_DESTINATION lib/cmake/${PROJECT_NAME})

# Targets:
#   * <prefix>/lib/libxxx.so
#   * header location after install: <prefix>/include/xxx.h
#   * headers can be included by C++ code `#include <xxx.h>`
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include)

# Headers:
#   * include/xxx.h -> <prefix>/include/xxx.h
file(GLOB_RECURSE HEADER_FILES ${CMAKE_SOURCE_DIR}/include/*.h)
install(FILES ${HEADER_FILES} DESTINATION include)

# Config
#   * <prefix>/lib/cmake/xxx/xxxConfig.cmake
#   * <prefix>/lib/cmake/xxx/xxxConfigVersion.cmake
install(FILES ${PROJECT_CONFIG} ${VERSION_CONFIG}
        DESTINATION lib/cmake/${PROJECT_NAME})

# Config
#   * <prefix>/lib/cmake/xxx/xxxTargets.cmake
install(EXPORT ${PROJECT_NAME}Targets
        NAMESPACE Leica::
        DESTINATION lib/cmake/${PROJECT_NAME})