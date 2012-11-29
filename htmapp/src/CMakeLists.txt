set(htmapp_SRC
disk-posix.hpp        
disk-windows.hpp     
disk.cpp
resources.cpp         
sockets-posix.hpp
sockets-windows.hpp   
sockets.cpp           
str.cpp
sys-posix.hpp        
sys-windows.hpp       
sys.cpp
threads-posix.hpp     
threads-windows.hpp   
threads.cpp
threads_queue.cpp   
)


if(MSVC)
  add_definitions( /D "NOMINMAX" /D "WIN32_LEAN_AND_MEAN" )
endif ()

add_library(htmapp ${htmapp_SRC})
target_link_libraries(htmapp  ws2_32.lib)

angel_module_target(htmapp)
