cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

project(lotos)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type defined; defaulting to 'Debug'")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "The type of build. Possible values are: Debug, Release, RelWithDebInfo and MinSizeRel.")
endif()

message(STATUS "Host is: ${CMAKE_HOST_SYSTEM}.  Build target is: ${CMAKE_SYSTEM}")

set(SRC
  buffer.c  connection.c  dict.c  http_parser.c  lotos_epoll.c  main.c  misc.c  request.c  response.c  server.c  ssstr.c
)


message(STATUS "OS type: ${CMAKE_SYSTEM_NAME}")
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  message( FATAL_ERROR "Only Linux is supported, CMake will exit." )
endif()

add_executable(lotos ${SRC})

set_property(TARGET lotos PROPERTY C_STANDARD 99)
