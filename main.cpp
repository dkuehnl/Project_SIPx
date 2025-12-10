#include <iostream>
#include <string>

#include "app/SIPxApp.h"
#include "parser/SIPParser.h"

constexpr std::string_view INVITE =
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

constexpr std::string_view Response_180 =
    "SIP/2.0 180 Ringing\r\n"
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

constexpr std::string_view Response_200 =
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


int main() {
    ModuleFlags flags;
    flags.parser = true;
    flags.logwriter = true;
    flags.network = true;

    try {
        const SIPxApp app(flags, "C:\\Users\\dkueh");
        app.parse_raw_message(std::string(INVITE));
        for (const auto& msg : app.get_messages()) {
            std::cout << msg->from() << std::endl;
        }
        /*auto response_180 = SIPResponse(std::string(Response_180));
        std::cout << "Response 180:" << std::endl;
        std::cout
        << "Has Content: " << response_180.is_content_attached() << std::endl
        << "Content-Length: " << response_180.get_content_length() << std::endl
        << "Content-Type: '" << response_180.get_content_type() << "'" << std::endl;
       auto response_200 = SIPResponse(std::string(Response_200));
        std::cout << "Response 200: " << std::endl;
        std::cout
        << "Has Content: " << response_200.is_content_attached() << std::endl
        << "Content-Length: " << response_200.get_content_length() << std::endl
        << "Content-Type: '" << response_200.get_content_type() << "'" << std::endl;*/

    } catch (const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}