#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "parser/SIPParser.h"
#include "logwriter/SIPLogWriter.h"
#include "SIPTestMessages.h"

void PrintTo(ErrorCode code, std::ostream* os) {
    *os << SIPLogWriter::parse_error_code(code);
}

namespace {
    class SIPParserTest : public ::testing::Test {
        protected:
            SIPParser m_parser;
    };

    TEST_F(SIPParserTest, FullSIPMessageDetected) {
        const auto message = m_parser.parse_message(sipx::test::msg::INVITE_MSG);

        EXPECT_TRUE(message.is_complete);
    }

    TEST_F(SIPParserTest, NoExceptionWithEmptyInput) {
        const auto message = m_parser.parse_message("");

        EXPECT_FALSE(message.is_complete);
    }

    TEST_F(SIPParserTest, DetectIncompleteMessage) {
        const auto message = m_parser.parse_message(sipx::test::msg::brocken_INVITE_MSG);

        EXPECT_FALSE(message.is_complete);
    }

    TEST_F(SIPParserTest, DetectMessageAsRequest) {
        const auto message = m_parser.parse_message(sipx::test::msg::INVITE_MSG);

        EXPECT_THAT(message.type, MessageType::REQUEST);
    }

    TEST_F(SIPParserTest, DetectMessageAsResponse) {
        const auto message = m_parser.parse_message(sipx::test::msg::Response_183_MSG);

        EXPECT_THAT(message.type, MessageType::RESPONSE);
    }

    TEST_F(SIPParserTest, ParseRequestLineRequest) {
        const auto message = m_parser.parse_message(sipx::test::msg::INVITE_MSG);

        EXPECT_EQ(message.method, "INVITE");
        EXPECT_EQ(message.request_uri, "sip:bob@ims.telekom.de");
    }

    TEST_F(SIPParserTest, ParseRequestLineResponse) {
        const auto message = m_parser.parse_message(sipx::test::msg::Response_183_MSG);

        EXPECT_EQ(message.status_code, "183");
        EXPECT_EQ(message.reason, "Session Progress");
    }

    TEST_F(SIPParserTest, ParseSimpleHeaders) {
        const auto message = m_parser.parse_message(sipx::test::msg::INVITE_MSG);

        EXPECT_EQ(message.call_id, "1234567890@client.example.com");
        EXPECT_EQ(message.content_length, "148");
        EXPECT_EQ(message.content_type, "application/sdp");
        EXPECT_EQ(message.max_forwards, "70");
    }

    TEST_F(SIPParserTest, ParseFromHeader) {
        const auto message = m_parser.parse_message(sipx::test::msg::INVITE_MSG);

        EXPECT_EQ(message.from.user, "alice111112");
        EXPECT_EQ(message.from.host, "ims.telekom.de");
        EXPECT_EQ(message.from.tag, "54321");
    }

    TEST_F(SIPParserTest, ParseToHeader) {
        const auto message = m_parser.parse_message(sipx::test::msg::INVITE_MSG);

        EXPECT_EQ(message.to.scheme, "sip");
        EXPECT_EQ(message.to.user, "bob222");
        EXPECT_EQ(message.to.host, "ims.telekom.de");
        EXPECT_EQ(message.to.tag, "12354");
        EXPECT_EQ(message.to.params, "");
        EXPECT_EQ(message.to.uri_param, "transport=tcp");
    }

    TEST_F(SIPParserTest, ParseCSeq) {
        const auto message = m_parser.parse_message(sipx::test::msg::INVITE_MSG);

        EXPECT_EQ(message.cseq.method, "INVITE");
        EXPECT_EQ(message.cseq.number, "1");
    }

    TEST_F(SIPParserTest, ParseSupportedFlags) {
        const auto message = m_parser.parse_message(sipx::test::msg::Response_200_MSG);

        EXPECT_THAT(message.supported, ::testing::Contains("100rel"));
        EXPECT_THAT(message.supported, ::testing::Contains("replaces"));
    }

    TEST_F(SIPParserTest, ParseRequiredFlags) {
        const auto message = m_parser.parse_message(sipx::test::msg::Response_200_MSG);

        EXPECT_THAT(message.require, ::testing::Contains("timer"));
    }

    TEST_F(SIPParserTest, ParseAllowHeader) {
        const auto message = m_parser.parse_message(sipx::test::msg::Response_200_MSG);

        EXPECT_THAT(message.allow, ::testing::ElementsAre(
            "INVITE",
            "ACK",
            "OPTIONS",
            "BYE",
            "CANCEL",
            "REGISTER",
            "INFO",
            "UPDATE",
            "PRACK"
        ));
    }

    TEST_F(SIPParserTest, ParseContact) {
        const auto message = m_parser.parse_message(sipx::test::msg::INVITE_MSG);

        EXPECT_THAT(message.contacts[0].scheme, "sip");
        EXPECT_THAT(message.contacts[0].user, "alice");
        EXPECT_THAT(message.contacts[0].host, "client.example.com");
        EXPECT_THAT(message.contacts[0].port, "5060");
        EXPECT_THAT(message.contacts[0].transport, "tcp");
        EXPECT_THAT(message.contacts[0].expires, "200");
        EXPECT_THAT(message.contacts[0].params, "");
        EXPECT_THAT(message.contacts[0].uri_params, "");
    }

    TEST_F(SIPParserTest, ParseOneViaHeader) {
        const auto message = m_parser.parse_message(sipx::test::msg::INVITE_MSG);

        EXPECT_THAT(message.via[0].transport, "SIP/2.0/UDP");
        EXPECT_THAT(message.via[0].host, "client.example.com");
        EXPECT_THAT(message.via[0].port, "5060");
        EXPECT_THAT(message.via[0].branch, "z9hG4bK-12345");
    }

    TEST_F(SIPParserTest, ParseMoreViaHeader) {
        const auto message = m_parser.parse_message(sipx::test::msg::Response_183_MSG);

        EXPECT_THAT(message.via, ::testing::ElementsAre(
            ::testing::Field(&Via::branch, "z9hG4bKmodidajkli218u1mkd"),
            ::testing::Field(&Via::branch, "z9hG4bKmodidajkli218u2mkd"),
            ::testing::Field(&Via::branch, "z9hG4bKmodidajkli218u3mkd"))
            );
    }

    TEST_F(SIPParserTest, ParseOneRecordRoute) {
        const auto message = m_parser.parse_message(sipx::test::msg::Response_183_MSG);

        EXPECT_THAT(message.record_route[0], "<sip:mavodi-0-266-jdfklj-dsie-ffffff-ffsaljfi>");
    }

    TEST_F(SIPParserTest, ParseMoreRecordRoute) {
        const auto message = m_parser.parse_message(sipx::test::msg::Response_183_MSG);

        EXPECT_THAT(message.record_route, ::testing::ElementsAre(
            "<sip:mavodi-0-266-jdfklj-dsie-ffffff-ffsaljfi>",
            "<sip:mavodi-0-270-jdfklj-dsie-ffffff-ffsaljfi>")
            );
    }

    TEST_F(SIPParserTest, ParseOneRouteHeader) {
        const auto message = m_parser.parse_message(sipx::test::msg::Response_183_MSG);

        EXPECT_THAT(message.route[0], "<sip:p1.example.com;lr>");
    }

    TEST_F(SIPParserTest, ParseMoreRouteHeader) {
        const auto message = m_parser.parse_message(sipx::test::msg::Response_183_MSG);

        EXPECT_THAT(message.route, ::testing::ElementsAre(
            "<sip:p1.example.com;lr>",
            "<sip:p2.example.com:5060;lr;transport=tcp>")
            );
    }

    TEST_F(SIPParserTest, ParseUnknownHeader) {
        const auto message = m_parser.parse_message(sipx::test::msg::Response_200_MSG);

        EXPECT_THAT(message.unknown_header, ::testing::Contains(
            Header{"Session-Expires", "1800"}
            ));
    }

    TEST_F(SIPParserTest, ExtractSIPBody) {
        const auto message = m_parser.parse_message(sipx::test::msg::INVITE_MSG);

        EXPECT_THAT(message.body,
            "v=0\r\n"
            "o=alice 53655765 2353687637 IN IP4 client.example.com\r\n"
            "s=-\r\n"
            "c=IN IP4 client.example.com\r\n"
            "t=0 0\r\n"
            "m=audio 49170 RTP/AVP 0\r\n"
            "a=rtpmap:0 PCMU/8000\r\n"
        );
    }

    TEST_F(SIPParserTest, ParseDuplicateHeader) {
        const auto message = m_parser.parse_message(sipx::test::msg::Malformed_INVITE_MSG);

        EXPECT_THAT(message.duplicate_header, ::testing::ElementsAre(
            Header{"t","<sip:carol@c.example.com"},
            Header{"i","def@a.example.com"},
            Header{"Call-ID","ghi@a.example.com"},
            Header{"CSeq","2 INVITE"}
        ));
    }

    TEST_F(SIPParserTest, ParseMalformedHeader) {
        const auto message = m_parser.parse_message(sipx::test::msg::Malformed_INVITE_MSG);

        EXPECT_THAT(message.malformed_header, ::testing::ElementsAre(
            Header{"Via","SIP/2.0/UDP"},
            Header{"Via","SIP/2.0/TCP [2001:db8::1;branch=z9hG4bK2"},
            Header{"From","\"Alice <sip:alice@a.example.com>;tag=1"},
            Header{"CSeq","1"},
            Header{"Contact","<alice-no-scheme>"}
        ));
    }

    TEST_F(SIPParserTest, ParseDuplicateUnknownHeader) {
        const auto message = m_parser.parse_message(sipx::test::msg::Malformed_INVITE_MSG);

        EXPECT_THAT(message.unknown_header, ::testing::ElementsAre(
            Header{"P-Asserted-Identity","<sip:alice@a.example.com>"},
            Header{"P-Asserted-Identity","<tel:+4930123>"}
        ));
    }

    TEST_F(SIPParserTest, ParseEmptyRequiredHeader) {
        const auto message = m_parser.parse_message(sipx::test::msg::Malformed_INVITE_MSG);

        EXPECT_EQ(message.from, NameAddr{});
        EXPECT_EQ(message.cseq, CSeq{});
        EXPECT_TRUE(message.content_type.empty());
        EXPECT_TRUE(message.max_forwards.empty());
    }

    TEST_F(SIPParserTest, ParseBrokenRequestLine) {
        const auto message = m_parser.parse_message(sipx::test::msg::Malformed_Request_line_MSG);

        EXPECT_TRUE(message.is_complete);
        EXPECT_EQ(message.type, MessageType::UNKNOWN);
        EXPECT_EQ(message.via.size(), 2);
        EXPECT_EQ(message.malformed_start_line, "INVITE SIP/2.0");
    }

    TEST_F(SIPParserTest, LineFoldingParsed) {
        const auto message = m_parser.parse_message(sipx::test::msg::Folded_Header_MSG);

        EXPECT_EQ(message.via.size(), 2);
        EXPECT_EQ(message.from.user, "alice");
        EXPECT_EQ(message.from.host, "atlanta.example.com");
        EXPECT_EQ(message.to.user, "bob");
        EXPECT_EQ(message.to.host, "biloxi.example.com");
        EXPECT_EQ(message.cseq.number, "314159");
        EXPECT_EQ(message.cseq.method, "INVITE");
        EXPECT_THAT(message.unknown_header, ::testing::Contains(
            Header{"Subject", "I know you're there, pick up the phone and talk to me!"}
            ));
        EXPECT_EQ(message.supported.size(), 3);
        EXPECT_EQ(message.malformed_header.size(), 0);
        EXPECT_EQ(message.duplicate_header.size(), 0);
        EXPECT_EQ(message.type, MessageType::REQUEST);
    }

    TEST_F(SIPParserTest, ParseNonConformedLineFolding) {
        const auto message = m_parser.parse_message(sipx::test::msg::Orphan_Continuation_MSG);

        EXPECT_EQ(message.via.size(), 0);
        EXPECT_EQ(message.malformed_header.size(), 1);
        EXPECT_THAT(message.malformed_header, ::testing::Contains(
            Header{"", "Via: SIP/2.0/UDP pc33.atlanta.example.com;branch=z9hG4bK776asdhds"}
        ));
        EXPECT_EQ(message.call_id, "a84b4c76e66710@pc33.atlanta.example.com");
    }

    TEST_F(SIPParserTest, IncrementPaketCount) {
        const auto message = m_parser.parse_message(sipx::test::msg::INVITE_MSG);

        EXPECT_EQ(m_parser.get_total_pkt_count(), 1);
    }

    TEST_F(SIPParserTest, IncrementIncompletePaketCount) {
        const auto message = m_parser.parse_message(sipx::test::msg::brocken_INVITE_MSG);
        const auto message2 = m_parser.parse_message(sipx::test::msg::INVITE_MSG);

        EXPECT_EQ(m_parser.get_incomplete_pkt_count(), 1);
        EXPECT_EQ(m_parser.get_total_pkt_count(), 2);
    }

    TEST_F(SIPParserTest, ParseLineWithoutColon) {
        const auto message = m_parser.parse_message(sipx::test::msg::Message_WithoutColons);

        EXPECT_THAT(message.malformed_header, ::testing::Contains(
            Header{"", "X-Header"}
        ));
    }

    TEST_F(SIPParserTest, ParseValueWithEmptyHeader) {
        const auto message = m_parser.parse_message(sipx::test::msg::Message_WithoutColons);

        EXPECT_THAT(message.malformed_header, ::testing::Contains(
            Header{"", ": testvalue"}
        ));
    }

    TEST_F(SIPParserTest, ParseWhitespacedFields) {
        const auto message = m_parser.parse_message(sipx::test::msg::Via_Whitespace_MSG);

        EXPECT_THAT(message.via, ::testing::ElementsAre(
            Via{"SIP/2.0/UDP", "pc33.atlanta.example.com", "5060", "z9hG4bK1"},
            Via{"SIP/2.0/TCP", "[2001:db8::1]", "5061", "z9hG4bK2"}
        ));
        EXPECT_THAT(message.malformed_header, ::testing::ElementsAre(
            Header{"v", "SIP/2.0/UDP proxy.example.com junk;branch=z9hG4bK3"},
            Header{"Via", "SIP/2.0/UDP ;branch=z9hG4bK4"}
        ));
        EXPECT_EQ(message.cseq.method, "INVITE");
        EXPECT_EQ(message.cseq.number, "314159");
    }
};