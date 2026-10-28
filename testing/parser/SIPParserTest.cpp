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
    "Via: SIP/2.0/TCP 217.0.150.80:5060;branch=z9hG4bKmodidajkli218u2mkd\r\n"
    "Via: SIP/2.0/TCP 217.0.161.80:5060;branch=z9hG4bKmodidajkli218u3mkd\r\n"
    "Record-Route: <sip:mavodi-0-266-jdfklj-dsie-ffffff-ffsaljfi>\r\n"
    "Record-Route: <sip:mavodi-0-270-jdfklj-dsie-ffffff-ffsaljfi>\r\n"
    "Route: <sip:p1.example.com;lr>, <sip:p2.example.com:5060;lr;transport=tcp>\r\n"
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

constexpr std::string_view Malformed_INVITE_MSG =
    "INVITE sip:bob@b.example.com SIP/2.0\r\n"
    "Via: SIP/2.0/UDP pc1.a.example.com:5060;branch=z9hG4bK1, SIP/2.0/UDP\r\n"   // F8: 1 gültig, 1 malformed
    "Via: SIP/2.0/TCP [2001:db8::1;branch=z9hG4bK2\r\n"                          // F2 (List): malformed
    "v: SIP/2.0/UDP proxy.example.com;branch=z9hG4bK3\r\n"                       // List compact: gültig
    "From: \"Alice <sip:alice@a.example.com>;tag=1\r\n"                          // F2: malformed
    "To: <sip:bob@b.example.com>\r\n"                                            // F1 / F4 erster sauber
    "t: <sip:carol@c.example.com\r\n"                                            // F4: duplicate (kaputt, ungeparst)
    "Call-ID: abc@a.example.com\r\n"                                             // F1 / F3 erster
    "i: def@a.example.com\r\n"                                                   // F3: duplicate (compact)
    "Call-ID: ghi@a.example.com\r\n"                                             // F3: duplicate
    "CSeq: 1\r\n"                                                                // F5: malformed, belegt Slot
    "CSeq: 2 INVITE\r\n"                                                         // F5: duplicate
    "Contact: <sip:alice@pc1.a.example.com>;expires=3600, <alice-no-scheme>\r\n" // F8: 1 gültig, 1 malformed
    "m: <sip:alice@pc2.a.example.com:5070;transport=tcp>\r\n"                    // List compact: gültig
    "Allow: INVITE, ACK\r\n"                                                     // Allow mehrzeilig
    "Allow: BYE, CANCEL\r\n"
    "P-Asserted-Identity: <sip:alice@a.example.com>\r\n"                         // Unknown mehrfach
    "P-Asserted-Identity: <tel:+4930123>\r\n"
    "Content-Type:\r\n"                                                          // F7: leer
    "Content-Length: 0\r\n"                                                      // F6: Max-Forwards fehlt
    "\r\n";

constexpr std::string_view Malformed_Request_line_MSG =
    "INVITE sip:x SIP/2.0\r\n\r\n"
    "Via: SIP/2.0/UDP pc1.a.example.com:5060;branch=z9hG4bK1, SIP/2.0/UDP\r\n"   // F8: 1 gültig, 1 malformed
    "Via: SIP/2.0/TCP [2001:db8::1;branch=z9hG4bK2\r\n"                          // F2 (List): malformed
    "v: SIP/2.0/UDP proxy.example.com;branch=z9hG4bK3\r\n";

    class SIPParserTest : public ::testing::Test {
        protected:
            SIPParser m_parser;
    };

    TEST_F(SIPParserTest, FullSIPMessageDetected) {
        const auto message = m_parser.parse_message(INVITE_MSG);

        EXPECT_TRUE(message.is_complete);
    }

    TEST_F(SIPParserTest, NoExceptionWithEmptyInput) {
        const auto message = m_parser.parse_message("");

        EXPECT_FALSE(message.is_complete);
    }

    TEST_F(SIPParserTest, DetectIncompleteMessage) {
        const auto message = m_parser.parse_message(brocken_INVITE_MSG);

        EXPECT_FALSE(message.is_complete);
    }

    TEST_F(SIPParserTest, DetectMessageAsRequest) {
        const auto message = m_parser.parse_message(INVITE_MSG);

        EXPECT_THAT(message.type, MessageType::REQUEST);
    }

    TEST_F(SIPParserTest, DetectMessageAsResponse) {
        const auto message = m_parser.parse_message(Response_183_MSG);

        EXPECT_THAT(message.type, MessageType::RESPONSE);
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

    TEST_F(SIPParserTest, ParseAllowHeader) {
        const auto message = m_parser.parse_message(Response_200_MSG);

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

    TEST_F(SIPParserTest, ParseOneViaHeader) {
        const auto message = m_parser.parse_message(INVITE_MSG);

        EXPECT_THAT(message.via[0].transport, "SIP/2.0/UDP");
        EXPECT_THAT(message.via[0].host, "client.example.com");
        EXPECT_THAT(message.via[0].port, "5060");
        EXPECT_THAT(message.via[0].branch, "z9hG4bK-12345");
    }

    TEST_F(SIPParserTest, ParseMoreViaHeader) {
        const auto message = m_parser.parse_message(Response_183_MSG);

        EXPECT_THAT(message.via, ::testing::ElementsAre(
            ::testing::Field(&Via::branch, "z9hG4bKmodidajkli218u1mkd"),
            ::testing::Field(&Via::branch, "z9hG4bKmodidajkli218u2mkd"),
            ::testing::Field(&Via::branch, "z9hG4bKmodidajkli218u3mkd"))
            );
    }

    TEST_F(SIPParserTest, ParseOneRecordRoute) {
        const auto message = m_parser.parse_message(Response_183_MSG);

        EXPECT_THAT(message.record_route[0], "<sip:mavodi-0-266-jdfklj-dsie-ffffff-ffsaljfi>");
    }

    TEST_F(SIPParserTest, ParseMoreRecordRoute) {
        const auto message = m_parser.parse_message(Response_183_MSG);

        EXPECT_THAT(message.record_route, ::testing::ElementsAre(
            "<sip:mavodi-0-266-jdfklj-dsie-ffffff-ffsaljfi>",
            "<sip:mavodi-0-270-jdfklj-dsie-ffffff-ffsaljfi>")
            );
    }

    TEST_F(SIPParserTest, ParseOneRouteHeader) {
        const auto message = m_parser.parse_message(Response_183_MSG);

        EXPECT_THAT(message.route[0], "<sip:p1.example.com;lr>");
    }

    TEST_F(SIPParserTest, ParseMoreRouteHeader) {
        const auto message = m_parser.parse_message(Response_183_MSG);

        EXPECT_THAT(message.route, ::testing::ElementsAre(
            "<sip:p1.example.com;lr>",
            "<sip:p2.example.com:5060;lr;transport=tcp>")
            );
    }

    TEST_F(SIPParserTest, ParseUnknownHeader) {
        const auto message = m_parser.parse_message(Response_200_MSG);

        EXPECT_THAT(message.unknown_header, ::testing::Contains(Header{"Session-Expires", "1800"}));
    }

    TEST_F(SIPParserTest, ExtractSIPBody) {
        const auto message = m_parser.parse_message(INVITE_MSG);

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
        const auto message = m_parser.parse_message(Malformed_INVITE_MSG);

        EXPECT_THAT(message.duplicate_header, ::testing::ElementsAre(
            Header{"t","<sip:carol@c.example.com"},
            Header{"i","def@a.example.com"},
            Header{"Call-ID","ghi@a.example.com"},
            Header{"CSeq","2 INVITE"}
        ));
    }

    TEST_F(SIPParserTest, ParseMalformedHeader) {
        const auto message = m_parser.parse_message(Malformed_INVITE_MSG);

        EXPECT_THAT(message.malformed_header, ::testing::ElementsAre(
            Header{"Via","SIP/2.0/UDP"},
            Header{"Via","SIP/2.0/TCP [2001:db8::1;branch=z9hG4bK2"},
            Header{"From","\"Alice <sip:alice@a.example.com>;tag=1"},
            Header{"CSeq","1"},
            Header{"Contact","<alice-no-scheme>"}
        ));
    }

    TEST_F(SIPParserTest, ParseDuplicateUnknownHeader) {
        const auto message = m_parser.parse_message(Malformed_INVITE_MSG);

        EXPECT_THAT(message.unknown_header, ::testing::ElementsAre(
            Header{"P-Asserted-Identity","<sip:alice@a.example.com>"},
            Header{"P-Asserted-Identity","<tel:+4930123>"}
        ));
    }

    TEST_F(SIPParserTest, ParseEmptyRequiredHeader) {
        const auto message = m_parser.parse_message(Malformed_INVITE_MSG);

        EXPECT_EQ(message.from, NameAddr{});
        EXPECT_EQ(message.cseq, CSeq{});
        EXPECT_TRUE(message.content_type.empty());
        EXPECT_TRUE(message.max_forwards.empty());
    }
}
