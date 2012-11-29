SET(QT_USE_QTWEBKIT TRUE)

set(frwk_SRC
mainwindow.cpp
network.cpp
web_page.cpp
)


if(MSVC)
  add_definitions( /D "NOMINMAX" /D "WIN32_LEAN_AND_MEAN" )
endif ()

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

QT4_WRAP_CPP(frwk_MOCS mainwindow.h network.h web_page.h)

add_library(frwk ${frwk_SRC} ${frwk_MOCS})
target_link_libraries(frwk  ${QT_LIBRARIES} )

angel_module_target(frwk)
