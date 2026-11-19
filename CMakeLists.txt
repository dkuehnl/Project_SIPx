cmake_minimum_required(VERSION 4.0)
project(Project_SIPx LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SIPX_BUILD_TESTS "Build unit teste" ON)
option(SIPX_SANITIZE    "Enable ASan/UBSan (GCC/Clang, non-MinGW" OFF)

add_library(sipx_parser STATIC
    parser/SIPParser.cpp
    logwriter/SIPLogWriter.cpp
    parser/SIPMessage.h
        logwriter/ILogWriter.h
)
target_include_directories(sipx_parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(sipx_parser PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Wshadow -Wconversion>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
)

if (SIPX_SANITIZE AND NOT MINGW)
    target_compile_options(sipx_parser PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(sipx_parser PUBLIC -fsanitize=address,undefined)
endif()

add_executable(Project_SIPx
        main.cpp
        app/SIPxApp.cpp
        network/platform/WindowsSocket.cpp
        network/SIPNetwork.cpp
        eventhandler/SIPTimer.cpp
        register/SIPRegister.cpp
)
target_link_libraries(Project_SIPx PRIVATE sipx_parser)

if (WIN32)
    target_link_libraries(Project_SIPx PRIVATE ws2_32)
endif()

if (SIPX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(testing)
endif()