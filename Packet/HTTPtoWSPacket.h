#ifndef HTTPTOWSPACKET_H
#define HTTPTOWSPACKET_H

#include "Packet.h"
#include <stdio.h>
#include <string>
#include <sstream>
#include <algorithm>
#include <boost/uuid/detail/sha1.hpp>
#include <boost/beast/core/detail/base64.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>

using boost::algorithm::to_lower;
using boost::algorithm::trim;


class HTTPtoWSPacket : public Packet
{
public:
	
	constexpr static char GUID[37] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	bool isKeySet = false;
	char Sec_WebSocket_Key[64];
	
	HTTPtoWSPacket();
	HTTPtoWSPacket(const char * content);
	~HTTPtoWSPacket();

	void Parse(const char* content) override;

	std::string getWebSocketAccept();

private:
	void get_sha1(const std::string& p_arg, uint8_t* dest);

};



#endif // !HTTPTOWSPACKET_H
