cmake_minimum_required(VERSION 4.0)
project(Project_SIPx)

set(CMAKE_CXX_STANDARD 20)

add_executable(Project_SIPx main.cpp
        parser/SIPMessage.cpp
        parser/SIPMessage.h
        parser/SIPRequest.cpp
        parser/SIPResponse.cpp
        parser/SIPRequest.h
        parser/SIPResponse.h
        logwriter/SIPLogWriter.cpp
        logwriter/SIPLogWriter.h
        app/SIPxApp.cpp
        app/SIPxApp.h
        parser/SIPParser.cpp
        parser/SIPParser.h
        eventhandler/EventDispatcher.h
        eventhandler/Event.h
        eventhandler/EventHandler.h
        network/platform/socket_base.h
        network/platform/WindowsSocket.cpp
        network/platform/WindowsSocket.h
        network/SIPNetwork.cpp
        network/SIPNetwork.h
)

if (WIN32)
    target_link_libraries(Project_SIPx PRIVATE ws2_32)
endif()
