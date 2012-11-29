



project(cii)

include(htmapp)

set(cii_SRC
cii.cpp
process_binary.h
process_cpp.h
)

INCLUDE_DIRECTORIES(${htmapp_INCLUDE_DIRS})
add_executable(cii ${cii_SRC})
target_link_libraries(cii  ${htmapp_LIBRARY_DIRS}/${htmapp_LIBRARIES})
add_dependencies(cii htmapp)