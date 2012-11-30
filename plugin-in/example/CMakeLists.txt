PROJECT(htmapp_resources)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF (MSVC)
  ADD_DEFINITIONS( /D "NOMINMAX" /D "WIN32_LEAN_AND_MEAN" )
ENDIF (MSVC)

add_definitions( "/W3 /D_CRT_SECURE_NO_WARNINGS /wd4309 /nologo" )

set(htmapp_resources_FILES ${htmapp_resources_SOURCE_DIR})

file(GLOB_RECURSE htmapp_resources_FILES ${htmapp_resources_FILES}/*.cpp  ${htmapp_resources_FILES}/cpp/*.cpp)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}  ${htmapp_INCLUDE_DIRS} )

add_library(htmapp_resources SHARED ${htmapp_resources_FILES} )