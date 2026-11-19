include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.18.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        FIND_PACKAGE_ARGS NAMES GTest
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(sipx_parser_tests
    parser/SIPParserTest.cpp
        parser/SIPTestMessages.h
        logwriter/SIPLogWriterTest.cpp
)
target_link_libraries(sipx_parser_tests PRIVATE sipx_parser GTest::gtest_main GTest::gmock)

gtest_discover_tests(sipx_parser_tests DISCOVERY_MODE PRE_TEST)