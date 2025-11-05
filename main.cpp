#include <iostream>
#include <string>

#include "sipmessage.h"
#include "SIPRequest.h"

constexpr std::string_view INVITE =
    "INVITE sip:bob@ims.telekom.de SIP/2.0\r\n"
    "Via: SIP/2.0/UDP client.example.com:5060;branch=z9hG4bK-12345\r\n"
    "Max-Forwards: 70\r\n"
    "From: <sip:alice11111@ims.telekom.de>;tag=54321\r\n"
    "To: \"dsaflkjdlkj\" <sip:bob222@ims.telekom.de>;tag=12354\r\n"
    "Call-ID: 1234567890@client.example.com\r\n"
    "P-Asserted-Identity: <sip:+49123456789@tel.t-online.de>\r\n"
    "p-preferred-identity: <sip:4911111111@192.168.178.1_5060>\r\n"
    "CSeq: 1 INVITE\r\n"
    "Contact: <sip:alice@client.example.com:5060>\r\n"
    "Content-Type: application/sdp\r\n"
    "Content-Length: 129\r\n"
    "\r\n"
    "v=0\r\n"
    "o=alice 53655765 2353687637 IN IP4 client.example.com\r\n"
    "s=-\r\n"
    "c=IN IP4 client.example.com\r\n"
    "t=0 0\r\n"
    "m=audio 49170 RTP/AVP 0\r\n"
    "a=rtpmap:0 PCMU/8000\r\n";

int main() {
    try {
        auto request = SIPRequest(std::string(INVITE));
        std::cout << "URI: '" << request.get_to_uri() << "'" << std::endl;
        std::cout << "Host: '" << request.get_to_host() << "'" << std::endl;

        if (request.get_to_tag().empty()) {
            std::cout << "No To-Tag" << std::endl;
        } else {
            std::cout << "Tag: '" << request.get_to_tag() << "'" << std::endl;

        }

    } catch (const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}