#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "parser/SIPParser.h"
#include "logwriter/SIPLogWriter.h"

void PrintTo(ErrorCode code, std::ostream* os) {
    *os << SIPLogWriter::parse_error_code(code);
}

namespace {
constexpr std::string_view INVITE_MSG =
    "INVITE sip:bob@ims.telekom.de SIP/2.0\r\n"
    "Via: SIP/2.0/UDP client.example.com:5060;branch=z9hG4bK-12345\r\n"
    "Max-Forwards: 70\r\n"
    "From: <sip:alice111112@ims.telekom.de>;tag=54321\r\n"
    "To: \"dsaflkjdlkj\" <sip:bob222@ims.telekom.de;transport=tcp>;tag=12354\r\n"
    "Call-ID: 1234567890@client.example.com\r\n"
    "P-Asserted-Identity: <sip:+49123456789@tel.t-online.de>\r\n"
    "p-preferred-identity: <sip:4911111111@192.168.178.1:5060>\r\n"
    "CSeq: 1 INVITE\r\n"
    "Allow: INVITE, ACK, OPTIONS, BYE, CANCEL, REGISTER, INFO, UPDATE, PRACK\r\n"
    "Contact: <sip:alice@client.example.com:5060;transport=tcp>;expires=200\r\n"
    "Content-Type: application/sdp\r\n"
    "Content-Length: 148\r\n"
    "\r\n"
    "v=0\r\n"
    "o=alice 53655765 2353687637 IN IP4 client.example.com\r\n"
    "s=-\r\n"
    "c=IN IP4 client.example.com\r\n"
    "t=0 0\r\n"
    "m=audio 49170 RTP/AVP 0\r\n"
    "a=rtpmap:0 PCMU/8000\r\n";

constexpr std::string_view brocken_INVITE_MSG =
    "INVITE sip:bob@ims.telekom.de SIP/2.0\r\n"
    "Via: SIP/2.0/UDP client.example.com:5060;branch=z9hG4bK-12345\r\n"
    "Max-Forwards: 70\r\n"
    "From: <sip:alice111112@ims.telekom.de>;tag=54321\r\n"
    "To: \"dsaflkjdlkj\" <sip:bob222@ims.telekom.de>;tag=12354\r\n"
    "Call-ID: 1234567890@client.example.com\r\n"
    "P-Asserted-Identity: <sip:+49123456789@tel.t-online.de>\r\n"
    "p-preferred-identity: <sip:4911111111@192.168.178.1:5060>\r\n"
    "CSeq: 1 INVITE\r\n"
    "Allow: INVITE, ACK, OPTIONS, BYE, CANCEL, REGISTER, INFO, UPDATE, PRACK\r\n"
    "Contact: <sip:alice@client.example.com:5060;transport=tcp>;expire=200\r\n"
    "Content-Type: application/sdp\r\n";

constexpr std::string_view Response_183_MSG =
    "SIP/2.0 183 Session Progress\r\n"
    "Via: SIP/2.0/TCP 217.0.149.80:5060;branch=z9hG4bKmodidajkli218u1mkd\r\n"
    "Record-Route: <sip:mavodi-0-266-jdfklj-dsie-ffffff-ffsaljfi>\r\n"
    "From: <sip:+4978945135753@tel.t-online.de;user=phone>;tag=4567324867dafds8725\r\n"
    "To: <sip:+48975642357@tel.t-online.de;user=phone>;tag=241584735874\r\n"
    "Call-ID: 1234567890@adce\r\n"
    "CSeq: 1 INVITE\r\n"
    "Require: timer, 100rel\r\n"
    "Supported: replaces, 100rel\r\n"
    "Min-SE: 900\r\n"
    "Session-Expires: 1800;refresher=uas\r\n"
    "Allow: INVITE, ACK, OPTIONS, BYE, CANCEL, REGISTER, INFO, UPDATE, PRACK\r\n"
    "Contact: <sip:+49199296100240014@192.168.178.1:5060>\r\n"
    "Server: OpenScape\r\n"
    "Content-Length: 0\r\n"
    "\r\n";

constexpr std::string_view Response_200_MSG =
    "SIP/2.0 200 OK\r\n"
    "Via: SIP/2.0/TCP 217.0.149.80:5060;branch=z9hG4bKmodidajkli218u1mkd\r\n"
    "Record-Route: <sip:mavodi-0-266-jdfklj-dsie-ffffff-ffsaljfi>\r\n"
    "From: <sip:+4978945135753@tel.t-online.de;user=phone>;tag=4567324867dafds8725\r\n"
    "To: <sip:+48975642357@tel.t-online.de;user=phone>;tag=241584735874\r\n"
    "Call-ID: 1234567890@adce\r\n"
    "CSeq: 1 INVITE\r\n"
    "Require: timer\r\n"
    "Supported: 100rel\r\n"
    "Supported: replaces\r\n"
    "Allow: INVITE, ACK, OPTIONS, BYE, CANCEL, REGISTER, INFO, UPDATE, PRACK\r\n"
    "Contact: <sip:+49199296100240014@192.168.178.1:5060>\r\n"
    "Server: OpenScape\r\n"
    "Min-SE: 900\r\n"
    "Session-Expires: 1800\r\n"
    "Content-Length: 129\r\n"
    "\r\n"
    "v=0\r\n"
    "o=alice 53655765 2353687637 IN IP4 client.example.com\r\n"
    "s=-\r\n"
    "c=IN IP4 client.example.com\r\n"
    "t=0 0\r\n"
    "m=audio 49170 RTP/AVP 0\r\n"
    "a=rtpmap:0 PCMU/8000\r\n";

    class SIPParserTest : public ::testing::Test {
        protected:
            SIPParser m_parser;
    };

    TEST_F(SIPParserTest, FullSIPMessageDetected) {
        const auto message = m_parser.parse_message(INVITE_MSG);

        EXPECT_TRUE(message.is_complete);
    }

    TEST_F(SIPParserTest, DetectIncompleteMessage) {
        const auto message = m_parser.parse_message(brocken_INVITE_MSG);

        EXPECT_FALSE(message.is_complete);
    }

    TEST_F(SIPParserTest, DetectMessageAsRequest) {
        const auto message = m_parser.parse_message(INVITE_MSG);

        EXPECT_TRUE(message.is_request);
    }

    TEST_F(SIPParserTest, DetectMessageAsResponse) {
        const auto message = m_parser.parse_message(Response_183_MSG);

        EXPECT_FALSE(message.is_request);
    }

    TEST_F(SIPParserTest, ParseRequestLineRequest) {
        const auto message = m_parser.parse_message(INVITE_MSG);

        EXPECT_EQ(message.method, "INVITE");
        EXPECT_EQ(message.request_uri, "sip:bob@ims.telekom.de");
    }

    TEST_F(SIPParserTest, ParseRequestLineResponse) {
        const auto message = m_parser.parse_message(Response_183_MSG);

        EXPECT_EQ(message.status_code, "183");
        EXPECT_EQ(message.reason, "Session Progress");
    }

    TEST_F(SIPParserTest, ParseSimpleHeaders) {
        const auto message = m_parser.parse_message(INVITE_MSG);

        EXPECT_EQ(message.call_id, "1234567890@client.example.com");
        EXPECT_EQ(message.content_length, "148");
        EXPECT_EQ(message.content_type, "application/sdp");
        EXPECT_EQ(message.max_forwards, "70");
        EXPECT_EQ(message.allow, "INVITE, ACK, OPTIONS, BYE, CANCEL, REGISTER, INFO, UPDATE, PRACK");
    }

    TEST_F(SIPParserTest, ParseFromHeader) {
        const auto message = m_parser.parse_message(INVITE_MSG);

        EXPECT_EQ(message.from.user, "alice111112");
        EXPECT_EQ(message.from.host, "ims.telekom.de");
        EXPECT_EQ(message.from.tag, "54321");
    }

    TEST_F(SIPParserTest, ParseToHeader) {
        const auto message = m_parser.parse_message(INVITE_MSG);

        EXPECT_EQ(message.to.scheme, "sip");
        EXPECT_EQ(message.to.user, "bob222");
        EXPECT_EQ(message.to.host, "ims.telekom.de");
        EXPECT_EQ(message.to.tag, "12354");
        EXPECT_EQ(message.to.params, "");
        EXPECT_EQ(message.to.uri_param, "transport=tcp");
    }

    TEST_F(SIPParserTest, ParseCSeq) {
        const auto message = m_parser.parse_message(INVITE_MSG);

        EXPECT_EQ(message.cseq.method, "INVITE");
        EXPECT_EQ(message.cseq.number, "1");
    }

    TEST_F(SIPParserTest, ParseSupportedFlags) {
        const auto message = m_parser.parse_message(Response_200_MSG);

        EXPECT_THAT(message.supported, ::testing::Contains("100rel"));
        EXPECT_THAT(message.supported, ::testing::Contains("replaces"));
    }

    TEST_F(SIPParserTest, ParseRequiredFlags) {
        const auto message = m_parser.parse_message(Response_200_MSG);

        EXPECT_THAT(message.require, ::testing::Contains("timer"));
    }

    TEST_F(SIPParserTest, ParseContact) {
        const auto message = m_parser.parse_message(INVITE_MSG);

        EXPECT_THAT(message.contacts[0].scheme, "sip");
        EXPECT_THAT(message.contacts[0].user, "alice");
        EXPECT_THAT(message.contacts[0].host, "client.example.com");
        EXPECT_THAT(message.contacts[0].port, "5060");
        EXPECT_THAT(message.contacts[0].transport, "tcp");
        EXPECT_THAT(message.contacts[0].expires, "200");
        EXPECT_THAT(message.contacts[0].params, "");
        EXPECT_THAT(message.contacts[0].uri_params, "");
    }
}