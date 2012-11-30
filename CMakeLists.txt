cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

if(WIN32 OR NOT EXISTS /dev/urandom)
  cmake_minimum_required(VERSION 2.8.5)
endif()

project(QThybrid)

set(CMAKE_MODULE_PATH ${QThybrid_SOURCE_DIR}/CMake ${CMAKE_MODULE_PATH} ${QThybrid_SOURCE_DIR}/lib/cmake/ANGEL-1.0/Modules)

include(ModuleMacros)

# QThybrid version number.
set(QThybrid_VERSION_MAJOR "1")
set(QThybrid_VERSION_MINOR "0")
set(QThybrid_VERSION_PATCH "0")


if(NOT ANGEL_INSTALL_RUNTIME_DIR)
  set(ANGEL_INSTALL_RUNTIME_DIR bin)
endif()
if(NOT ANGEL_INSTALL_LIBRARY_DIR)
  set(ANGEL_INSTALL_LIBRARY_DIR lib)
endif()
if(NOT ANGEL_INSTALL_ARCHIVE_DIR)
  set(ANGEL_INSTALL_ARCHIVE_DIR lib)
endif()
if(NOT ANGEL_INSTALL_INCLUDE_DIR)
  set(ANGEL_INSTALL_INCLUDE_DIR include/ANGEL-${QThybrid_VERSION_MAJOR}.${QThybrid_VERSION_MINOR})
endif()
if(NOT ANGEL_INSTALL_DATA_DIR)
  set(ANGEL_INSTALL_DATA_DIR share/ANGEL-${QThybrid_VERSION_MAJOR}.${QThybrid_VERSION_MINOR})
endif()
if(NOT ANGEL_INSTALL_DOC_DIR)
  set(ANGEL_INSTALL_DOC_DIR share/doc/ANGEL-${QThybrid_VERSION_MAJOR}.${QThybrid_VERSION_MINOR})
endif()
if(NOT ANGEL_INSTALL_PACKAGE_DIR)
  set(ANGEL_INSTALL_PACKAGE_DIR "lib/cmake/ANGEL-${QThybrid_VERSION_MAJOR}.${QThybrid_VERSION_MINOR}")
endif()

 set(ANGEL_BINARY_DIR ${QThybrid_BINARY_DIR})

# Setup build locations.
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ANGEL_BINARY_DIR}/bin)
endif()
if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${ANGEL_BINARY_DIR}/lib)
endif()
if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${ANGEL_BINARY_DIR}/lib)
endif()
set(ANGEL_MODULES_DIR "${ANGEL_BINARY_DIR}/${ANGEL_INSTALL_PACKAGE_DIR}/Modules")




add_subdirectory(htmapp)
add_subdirectory(frwk)
add_subdirectory(cii)
add_subdirectory(example)
add_subdirectory(plugin-in)
