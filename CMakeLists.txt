cmake_minimum_required(VERSION 4.0)
project(sip_parser)

set(CMAKE_CXX_STANDARD 20)

add_executable(sip_parser main.cpp
        parser/SIPMessage.cpp
        parser/SIPMessage.h
        parser/SIPRequest.cpp
        parser/SIPResponse.cpp
        parser/SIPRequest.h
        parser/SIPResponse.h
        logwriter/SIPLogWriter.cpp
        logwriter/SIPLogWriter.h
)
