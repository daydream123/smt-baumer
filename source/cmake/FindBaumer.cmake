# FindBaumer
# --------
#
# Find the Baumer libraries
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# The following variables will be defined:
#
# ``Baumer_FOUND`` True if Baumer found on the local system
#
# ``Baumer_INCLUDE_DIRS`` Location of Baumer header files
#
# ``Baumer_LIBRARY_DIRS`` Location of Baumer libraries
#
# ``Baumer_LIBRARIES`` List of the Baumer libraries found
#

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
include(${CMAKE_ROOT}/Modules/SelectLibraryConfigurations.cmake)
include(${CMAKE_ROOT}/Modules/CMakeFindDependencyMacro.cmake)

find_package(PkgConfig)
set(Baumer_VERSION "1.0.0")

if (DEFINED ENV{VCPKG_ROOT} AND DEFINED ENV{VCPKG_DEFAULT_TRIPLET})
    set(Baumer_ROOT_DIR "$ENV{VCPKG_ROOT}/installed/$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
endif()

# find path
macro(Baumer_FIND_INCLUDE varname foldername headername)
  if(NOT Baumer_${varname}_INCLUDE_DIR)
    find_path(
      Baumer_${varname}_INCLUDE_DIR
      NAMES ${foldername}/${headername}
      PATHS ${Baumer_ROOT_DIR}/include /usr/local/include)
    list(APPEND Baumer_INCLUDE_DIRS ${Baumer_${varname}_INCLUDE_DIR})
  endif()
endmacro(Baumer_FIND_INCLUDE)

Baumer_FIND_INCLUDE(bgapi2_ext bgapi2_ext bgapi2_ext.h)
Baumer_FIND_INCLUDE(bgapi2_ext_addons bgapi2_ext bgapi2_ext_addons.h)
Baumer_FIND_INCLUDE(bgapi2_ext_sc bgapi2_ext_sc bgapi2_ext_sc.h)
Baumer_FIND_INCLUDE(bgapi2_def bgapi2_genicam bgapi2_def.h)
Baumer_FIND_INCLUDE(bgapi2_featurenames bgapi2_genicam bgapi2_featurenames.h)
Baumer_FIND_INCLUDE(bgapi2_genicam bgapi2_genicam bgapi2_genicam.hpp)

# message("Baumer_INCLUDE_DIRS: " ${Baumer_INCLUDE_DIRS})
set(Baumer_INCLUDE_DIRS ${Baumer_INCLUDE_DIRS} CACHE STRING "")

# find library
macro(Baumer_FIND_LIBRARY libname)
  if(NOT Baumer_${varname}_LIBRARY)
     find_library(
       Baumer_${libname}_LIBRARY
       NAMES ${libname}
       PATHS ${Baumer_ROOT_DIR}/lib /usr/local/lib)
     get_filename_component(Baumer_${libname}_LIBRARY_DIR ${Baumer_${libname}_LIBRARY} DIRECTORY)
     list(APPEND Baumer_LIBRARY_DIRS ${Baumer_${libname}_LIBRARY_DIR})
     list(APPEND Baumer_LIBRARIES ${Baumer_${libname}_LIBRARY})
  endif()
endmacro(Baumer_FIND_LIBRARY)

Baumer_FIND_LIBRARY(libbgapi2_ext.so)
Baumer_FIND_LIBRARY(libbgapi2_ext_sc.so)
Baumer_FIND_LIBRARY(libbgapi2_genicam.so)
Baumer_FIND_LIBRARY(libbgapi2_img.so)
Baumer_FIND_LIBRARY(libGCBase_gcc41_v3_0_baumer.so)
Baumer_FIND_LIBRARY(libGenApi_gcc41_v3_0_baumer.so)
Baumer_FIND_LIBRARY(liblog4cpp_gcc41_v3_0_baumer.so)
Baumer_FIND_LIBRARY(libLog_gcc41_v3_0_baumer.so)
Baumer_FIND_LIBRARY(libMathParser_gcc41_v3_0_baumer.so)
Baumer_FIND_LIBRARY(libNodeMapData_gcc41_v3_0_baumer.so)
Baumer_FIND_LIBRARY(libXmlParser_gcc41_v3_0_baumer.so)

# message("Baumer_LIBRARY_DIRS: " ${Baumer_LIBRARY_DIRS})
# message("Baumer_LIBRARIES: " ${Baumer_LIBRARIES})

set(Baumer_LIBRARY_DIRS ${Baumer_LIBRARY_DIRS} CACHE STRING "")
set(Baumer_LIBRARIES ${Baumer_LIBRARIES} CACHE STRING "")

find_package_handle_standard_args(Baumer REQUIRED_VARS Baumer_LIBRARIES
                                  Baumer_LIBRARY_DIRS Baumer_INCLUDE_DIRS)
mark_as_advanced(Baumer_INCLUDE_DIR Baumer_LIBRARY)

