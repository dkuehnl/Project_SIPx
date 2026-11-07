//
// Created by dkueh on 05.11.2026.
//

#ifndef PROJECT_SIPX_SIPTESTMESSAGES_H
#define PROJECT_SIPX_SIPTESTMESSAGES_H

#include <string_view>

namespace sipx::test::msg {
    inline constexpr std::string_view INVITE_MSG =
        "INVITE sip:bob@ims.telekom.de SIP/2.0\r\n"
        "Via: SIP/2.0/UDP client.example.com:5060;branch=z9hG4bK-12345\r\n"
        "Max-Forwards: 70\r\n"
        "From: <sip:alice111112@ims.telekom.de>;tag=54321\r\n"
        "To: \"dsaflkjdlkj\" <sip:bob222@ims.telekom.de:5555;transport=tcp>;tag=12354\r\n"
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

    inline constexpr std::string_view INVITE_double_Tag_MSG =
        "INVITE sip:bob@ims.telekom.de SIP/2.0\r\n"
        "Via: SIP/2.0/UDP client.example.com:5060;branch=z9hG4bK-12345\r\n"
        "Max-Forwards: 70\r\n"
        "From: <sip:alice111112@ims.telekom.de>;tag=54321;tag=1111\r\n"
        "To: \"dsaflkjdlkj\" <sip:bob222@ims.telekom.de:5555;transport=tcp>;tag=12354\r\n"
        "Call-ID: 1234567890@client.example.com\r\n"
        "CSeq: 1 INVITE\r\n"
        "Allow: INVITE, ACK, OPTIONS, BYE, CANCEL, REGISTER, INFO, UPDATE, PRACK\r\n"
        "Contact: <sip:alice@client.example.com:5060;transport=tcp>;expires=200\r\n"
        "Content-Type: application/sdp\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

    inline constexpr std::string_view brocken_INVITE_MSG =
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

    inline constexpr std::string_view Response_183_MSG =
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

    inline constexpr std::string_view Response_200_MSG =
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

    inline constexpr std::string_view Malformed_INVITE_MSG =
        "INVITE sip:bob@b.example.com SIP/2.0\r\n"
        "Via: SIP/2.0/UDP pc1.a.example.com:5060;branch=z9hG4bK1, SIP/2.0/UDP\r\n"
        "Via: SIP/2.0/TCP [2001:db8::1;branch=z9hG4bK2\r\n"
        "v: SIP/2.0/UDP proxy.example.com;branch=z9hG4bK3\r\n"
        "From: \"Alice <sip:alice@a.example.com>;tag=1\r\n"
        "To: <sip:bob@b.example.com>\r\n"
        "t: <sip:carol@c.example.com\r\n"
        "Call-ID: abc@a.example.com\r\n"
        "i: def@a.example.com\r\n"
        "Call-ID: ghi@a.example.com\r\n"
        "CSeq: 1\r\n"
        "CSeq: 2 INVITE\r\n"
        "Contact: <sip:alice@pc1.a.example.com>;expires=3600, <alice-no-scheme>\r\n"
        "m: <sip:alice@pc2.a.example.com:5070;transport=tcp>\r\n"
        "Allow: INVITE, ACK\r\n"
        "Allow: BYE, CANCEL\r\n"
        "P-Asserted-Identity: <sip:alice@a.example.com>\r\n"
        "P-Asserted-Identity: <tel:+4930123>\r\n"
        "Content-Type:\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

    inline constexpr std::string_view Malformed_Request_line_MSG =
        "INVITE SIP/2.0\r\n"
        "Via: SIP/2.0/UDP pc1.a.example.com:5060;branch=z9hG4bK1, SIP/2.0/UDP\r\n"
        "Via: SIP/2.0/TCP [2001:db8::1;branch=z9hG4bK2\r\n"
        "v: SIP/2.0/UDP proxy.example.com;branch=z9hG4bK3\r\n\r\n";

    inline constexpr std::string_view Folded_Header_MSG =
        "INVITE sip:bob@biloxi.example.com SIP/2.0\r\n"
        "Via: SIP/2.0/UDP pc33.atlanta.example.com;branch=z9hG4bK776asdhds,\r\n"
        "     SIP/2.0/TCP proxy.example.com:5061;branch=z9hG4bK1\r\n"
        "From: \"Alice\"\r\n"
        "\t<sip:alice@atlanta.example.com>;tag=1928301774\r\n"
        "To:\r\n"
        "   <sip:bob@biloxi.example.com>\r\n"
        "Call-ID: a84b4c76e66710@pc33.atlanta.example.com\r\n"
        "CSeq: 314159\r\n"
        "  INVITE\r\n"
        "Max-Forwards: 70\r\n"
        "Subject: I know you're there,  \r\n"
        "         pick up the phone\r\n"
        " \t \r\n"
        "\tand talk to me!\r\n"
        "Supported: replaces,\r\n"
        "   100rel, timer\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

    inline constexpr std::string_view Orphan_Continuation_MSG =
        "INVITE sip:bob@biloxi.example.com SIP/2.0\r\n"
        "   Via: SIP/2.0/UDP pc33.atlanta.example.com;branch=z9hG4bK776asdhds\r\n"
        "Call-ID: a84b4c76e66710@pc33.atlanta.example.com\r\n"
        "CSeq: 314159 INVITE\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

    inline constexpr std::string_view Message_WithoutColons =
        "INVITE sip:bob@biloxi.example.com SIP/2.0\r\n"
        "Via: SIP/2.0/UDP pc33.atlanta.example.com;branch=z9hG4bK776asdhds\r\n"
        "Call-ID: a84b4c76e66710@pc33.atlanta.example.com\r\n"
        "CSeq: 314159 INVITE\r\n"
        "X-Header\r\n"
        ": testvalue\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

    inline constexpr std::string_view Via_Whitespace_MSG =
        "INVITE sip:bob@biloxi.example.com SIP/2.0\r\n"
        "Via: SIP / 2.0 / UDP pc33.atlanta.example.com : 5060;branch=z9hG4bK1\r\n"
        "Via: SIP/2.0/TCP\t[2001:db8::1]:5061;branch=z9hG4bK2\r\n"
        "v: SIP/2.0/UDP proxy.example.com junk;branch=z9hG4bK3\r\n"
        "Via: SIP/2.0/UDP ;branch=z9hG4bK4\r\n"
        "Call-ID: a84b4c76e66710@pc33.atlanta.example.com\r\n"
        "CSeq: 314159\tINVITE\r\n"
        "Contact: <sip:a@b;transport=tcp?subject=x&priority=urgent>\r\n"
        "From: <sip:bob@b.example.com?subject=x>\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

    inline constexpr std::string_view Empty_Header_section_MSG =
        "OPTIONS sip:x SIP/2.0\r\n\r\n"
        "Via: SIP/2.0/TCP\t[2001:db8::1]:5061;branch=z9hG4bK2\r\n"
        "Call-ID: a84b4c76e66710@pc33.atlanta.example.com\r\n"
        "CSeq: 314159\tINVITE\r\n"
        "Contact: <sip:a@b;transport=tcp?subject=x&priority=urgent>\r\n"
        "From: <sip:bob@b.example.com?subject=x>\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

}
#endif //PROJECT_SIPX_SIPTESTMESSAGES_H