get_filename_component(_ANGELModuleMacros_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

macro(_angel_module_use_recurse mod)
  if(NOT ${dep}_USED)
    set(${mod}_USED 1)
    angel_module_load("${mod}")
    foreach(dep IN LISTS ${mod}_DEPENDS)
      _angel_module_use_recurse(${dep})
    endforeach()
    if(${mod}_INCLUDE_DIRS)
      include_directories(${${mod}_INCLUDE_DIRS})
    endif()
    if(${mod}_LIBRARY_DIRS)
      link_directories(${${mod}_LIBRARY_DIRS})
    endif()
  endif()
endmacro()

# angel_module_use([modules...])
#
# Adds include directories and link directories for the given modules and
# their dependencies.
macro(angel_module_use)
  foreach(mod ${ARGN})
    _angel_module_use_recurse("${mod}")
  endforeach()
endmacro()

macro(angel_module_load mod)
  if(NOT ${mod}_LOADED)
    include("${ANGEL_MODULES_DIR}/${mod}.cmake" OPTIONAL)
    if(NOT ${mod}_LOADED)
      message(FATAL_ERROR "No such module: \"${mod}\"")
    endif()
  endif()
endmacro()

macro(angel_module _name)
  angel_module_check_name(${_name})
  set(angel-module ${_name})
  set(angel-module-test ${_name}-Test)
  set(_doing "")
  set(ANGEL_MODULE_${angel-module}_DECLARED 1)
  set(ANGEL_MODULE_${angel-module-test}_DECLARED 1)
  set(ANGEL_MODULE_${angel-module}_DEPENDS "")
  set(ANGEL_MODULE_${angel-module-test}_DEPENDS "${angel-module}")
  set(ANGEL_MODULE_${angel-module}_DESCRIPTION "description")
  set(ANGEL_MODULE_${angel-module}_EXCLUDE_FROM_ALL 0)
  foreach(arg ${ARGN})
    if("${arg}" MATCHES "^(DEPENDS|TEST_DEPENDS|DESCRIPTION|DEFAULT)$")
      set(_doing "${arg}")
    elseif("${arg}" MATCHES "^EXCLUDE_FROM_ALL$")
      set(_doing "")
      set(ANGEL_MODULE_${angel-module}_EXCLUDE_FROM_ALL 1)
    elseif("${arg}" MATCHES "^[A-Z][A-Z][A-Z]$")
      set(_doing "")
      message(AUTHOR_WARNING "Unknown argument [${arg}]")
    elseif("${_doing}" MATCHES "^DEPENDS$")
      list(APPEND ANGEL_MODULE_${angel-module}_DEPENDS "${arg}")
    elseif("${_doing}" MATCHES "^TEST_DEPENDS$")
      list(APPEND ANGEL_MODULE_${angel-module-test}_DEPENDS "${arg}")
    elseif("${_doing}" MATCHES "^DESCRIPTION$")
      set(_doing "")
      set(ANGEL_MODULE_${angel-module}_DESCRIPTION "${arg}")
    elseif("${_doing}" MATCHES "^DEFAULT")
      message(FATAL_ERROR "Invalid argument [DEFAULT]")
    else()
      set(_doing "")
      message(AUTHOR_WARNING "Unknown argument [${arg}]")
    endif()
  endforeach()
  list(SORT ANGEL_MODULE_${angel-module}_DEPENDS) # Deterministic order.
  list(SORT ANGEL_MODULE_${angel-module-test}_DEPENDS) # Deterministic order.
endmacro()


macro(angel_module_check_name _name)
  if( NOT "${_name}" MATCHES "^[a-zA-Z][a-zA-Z0-9]*$")
    message(FATAL_ERROR "Invalid module name: ${_name}")
  endif()
endmacro()

macro(angel_module_impl)
  include(angel-module.cmake) # Load module meta-data
  set(${angel-module}_INSTALL_RUNTIME_DIR ${ANGEL_INSTALL_RUNTIME_DIR})  #在之前设置
  set(${angel-module}_INSTALL_LIBRARY_DIR ${ANGEL_INSTALL_LIBRARY_DIR})
  set(${angel-module}_INSTALL_ARCHIVE_DIR ${ANGEL_INSTALL_ARCHIVE_DIR})
  set(${angel-module}_INSTALL_INCLUDE_DIR ${ANGEL_INSTALL_INCLUDE_DIR})

  # Collect all sources and headers for IDE projects.
  set(_srcs "")
  if("${CMAKE_GENERATOR}" MATCHES "Xcode|Visual Studio|KDevelop"
      OR CMAKE_EXTRA_GENERATOR)
    # Add sources to the module target for easy editing in the IDE.
    set(_include ${${angel-module}_SOURCE_DIR}/include)
    if(EXISTS ${_include})
      set(_src ${${angel-module}_SOURCE_DIR}/src)
      file(GLOB_RECURSE _srcs ${_src}/*.cxx  ${_src}/*.cpp)
      file(GLOB_RECURSE _hdrs ${_include}/*.h ${_include}/*.hxx ${_include}/*.hpp)
      list(APPEND _srcs ${_hdrs})
    endif()
  endif()

  # Create a ${angel-module}-all target to build the whole module.
  add_custom_target(${angel-module}-all ALL SOURCES ${_srcs})

  angel_module_use(${ANGEL_MODULE_${angel-module}_DEPENDS})

  if(NOT DEFINED ${angel-module}_LIBRARIES)
    set(${angel-module}_LIBRARIES "")
    foreach(dep IN LISTS ANGEL_MODULE_${angel-module}_DEPENDS)
      list(APPEND ${angel-module}_LIBRARIES "${${dep}_LIBRARIES}")
    endforeach()
    if(${angel-module}_LIBRARIES)
      list(REMOVE_DUPLICATES ${angel-module}_LIBRARIES)
    endif()
  endif()

  if(EXISTS ${${angel-module}_SOURCE_DIR}/include)
    list(APPEND ${angel-module}_INCLUDE_DIRS ${${angel-module}_SOURCE_DIR}/include)
    install(DIRECTORY include/ DESTINATION ${${angel-module}_INSTALL_INCLUDE_DIR} COMPONENT Development)
  endif()

  if(${angel-module}_INCLUDE_DIRS)
    include_directories(${${angel-module}_INCLUDE_DIRS})
  endif()
  if(${angel-module}_SYSTEM_INCLUDE_DIRS)
    include_directories(${${angel-module}_SYSTEM_INCLUDE_DIRS})
  endif()

  if(${angel-module}_SYSTEM_LIBRARY_DIRS)
    link_directories(${${angel-module}_SYSTEM_LIBRARY_DIRS})
  endif()

  if(EXISTS ${${angel-module}_SOURCE_DIR}/src/CMakeLists.txt AND NOT ${angel-module}_NO_SRC)
    set_property(GLOBAL APPEND PROPERTY ANGELTargets_MODULES ${angel-module})
    add_subdirectory(src)
  endif()



  set(angel-module-EXPORT_CODE-build "${${angel-module}_EXPORT_CODE_BUILD}")
  set(angel-module-EXPORT_CODE-install "${${angel-module}_EXPORT_CODE_INSTALL}")

  set(angel-module-DEPENDS "${ANGEL_MODULE_${angel-module}_DEPENDS}")
  set(angel-module-LIBRARIES "${${angel-module}_LIBRARIES}")
  set(angel-module-INCLUDE_DIRS-build "${${angel-module}_INCLUDE_DIRS}")
  set(angel-module-INCLUDE_DIRS-install "\${ANGEL_INSTALL_PREFIX}/${${angel-module}_INSTALL_INCLUDE_DIR}")
  if(${angel-module}_SYSTEM_INCLUDE_DIRS)
    list(APPEND angel-module-INCLUDE_DIRS-build "${${angel-module}_SYSTEM_INCLUDE_DIRS}")
    list(APPEND angel-module-INCLUDE_DIRS-install "${${angel-module}_SYSTEM_INCLUDE_DIRS}")
  endif()
  set(angel-module-LIBRARY_DIRS "${${angel-module}_SYSTEM_LIBRARY_DIRS}")
  set(angel-module-INCLUDE_DIRS "${angel-module-INCLUDE_DIRS-build}")
  set(angel-module-EXPORT_CODE "${angel-module-EXPORT_CODE-build}")
  configure_file(${_ANGELModuleMacros_DIR}/ANGELModuleInfo.cmake.in ${ANGEL_MODULES_DIR}/${angel-module}.cmake @ONLY)
  set(angel-module-INCLUDE_DIRS "${angel-module-INCLUDE_DIRS-install}")
  set(angel-module-EXPORT_CODE "${angel-module-EXPORT_CODE-install}")
  configure_file(${_ANGELModuleMacros_DIR}/ANGELModuleInfo.cmake.in CMakeFiles/${angel-module}.cmake @ONLY)
  install(FILES
    ${${angel-module}_BINARY_DIR}/CMakeFiles/${angel-module}.cmake
    DESTINATION ${ANGEL_INSTALL_PACKAGE_DIR}/Modules
    COMPONENT Development
    )

endmacro()


macro(angel_module_target_label _target_name)
  if(angel-module)
    set(_label ${angel-module})
    if(TARGET ${angel-module}-all)
      add_dependencies(${angel-module}-all ${_target_name})
    endif()
  else()
    set(_label ${_ANGELModuleMacros_DEFAULT_LABEL})
  endif()
  set_property(TARGET ${_target_name} PROPERTY LABELS ${_label})
endmacro()

macro(angel_module_target_name _name)
  set_property(TARGET ${_name} PROPERTY VERSION 1)
  set_property(TARGET ${_name} PROPERTY SOVERSION 1)
  if("${_name}" MATCHES "^[Ii][Tt][Kk]")
    set(_angel "")
  else()
    set(_angel "QThybrid")
  endif()
  set_property(TARGET ${_name} PROPERTY OUTPUT_NAME ${_angel}${_name}-${QThybrid_VERSION_MAJOR}.${QThybrid_VERSION_MINOR})
endmacro()

macro(angel_module_target_export _name)
  export(TARGETS ${_name} APPEND FILE ${${angel-module}-targets-build})
endmacro()

macro(angel_module_target_install _name)
  #Use specific runtime components for executables and libraries separately when installing a module,
  #considering that the target of a module could be either an executable or a library.
  get_property(_ttype TARGET ${_name} PROPERTY TYPE)
  if("${_ttype}" STREQUAL EXECUTABLE)
    set(runtime_component Runtime)
  else()
    set(runtime_component RuntimeLibraries)
  endif()
  install(TARGETS ${_name}
    EXPORT  ${${angel-module}-targets}
    RUNTIME DESTINATION ${${angel-module}_INSTALL_RUNTIME_DIR} COMPONENT ${runtime_component}
    LIBRARY DESTINATION ${${angel-module}_INSTALL_LIBRARY_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${${angel-module}_INSTALL_ARCHIVE_DIR} COMPONENT Development
    )
endmacro()

macro(angel_module_target _name)
  set(_install 1)
  foreach(arg ${ARGN})
    if("${arg}" MATCHES "^(NO_INSTALL)$")
      set(_install 0)
    else()
      message(FATAL_ERROR "Unknown argument [${arg}]")
    endif()
  endforeach()
  angel_module_target_name(${_name})
  angel_module_target_label(${_name})
  #angel_module_target_export(${_name})
  if(_install)
    angel_module_target_install(${_name})
  endif()
endmacro()
