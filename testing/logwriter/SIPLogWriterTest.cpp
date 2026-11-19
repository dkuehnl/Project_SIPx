//
// Created by dkueh on 19.11.2026.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "logwriter/SIPLogWriter.h"

class SIPLogWriterTest : public ::testing::Test {
protected:

};

TEST(SIPLogWriterTest, ThrowExceptionWhenFilepathIsEmpty) {
    LogConfig config = {
        .filepath = "",
    };

    EXPECT_THROW(SIPLogWriter{config}, std::invalid_argument);
}