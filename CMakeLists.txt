cmake_minimum_required(VERSION 4.0)
project(sip_parser)

set(CMAKE_CXX_STANDARD 20)

add_executable(sip_parser main.cpp
        SIPMessage.cpp
        SIPMessage.h
        SIPRequest.cpp
        SIPResponse.cpp
        SIPRequest.h
        SIPResponse.h
)
