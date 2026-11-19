//
// Created by dkueh on 19.11.2026.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "logwriter/SIPLogWriter.h"

class SIPLogWriterTest : public ::testing::Test {
protected:

};

TEST(SIPLogWriterTest, ThrowExceptionWhenLogpathIsEmpty) {
    LogConfig config = {
        .log_dir = "",
    };

    EXPECT_THROW(SIPLogWriter{config}, std::invalid_argument);
}

TEST(SIPLogWriterTest, ThrowExceptionWhenLogpathNotFound) {
    LogConfig config{
        .log_dir = R"(C:\Users\dkuehnlein\log)"
    };

    try {
        SIPLogWriter{config};
        FAIL() << "Filesystem_error expected but not reached";
    } catch (const std::filesystem::filesystem_error& e) {
        EXPECT_EQ(e.code(), std::errc::no_such_file_or_directory);
    }
}

TEST(SIPLogWriterTest, ThrowExceptionWhenLogpathNotADir) {
    LogConfig config{
        .log_dir = R"(C:\Users\dkueh\testfile.pcap)"
    };

    try {
        SIPLogWriter{config};
        FAIL() << "Filesystem_error expected but not reached";
    } catch (const std::filesystem::filesystem_error& e) {
        EXPECT_EQ(e.code(), std::errc::not_a_directory);
    }
}

