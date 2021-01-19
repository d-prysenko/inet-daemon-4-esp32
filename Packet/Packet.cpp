#include "Packet.h"

Packet::Packet()
{
}

Packet::Packet(const char* content)
{
	
}

Packet::~Packet()
{
}


PacketType Packet::GetType(const char* content, size_t size)
{

	if (size <= 1)
		return PacketType::unknown;

	if (content[0] == content[size - 1] &&
		(uint8_t)content[0] < sizeof(PDAPPacket::packetSizes) &&
		PDAPPacket::packetSizes[(uint8_t)content[0]] == size)
	{
		return PacketType::PDAP;
	}

	if (size > 3 && content[0] == (char)0b10000010 && (content[1] & 0b10000000))
		return PacketType::ws;


	/*
	rfc2616
	Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
	Method, Request-URI and HTTP-Version cannot contain spaces
	
	In the interest of robustness, servers SHOULD ignore any empty
	line(s) received where a Request-Line is expected. In other words, if
	the server is reading the protocol stream at the beginning of a
	message and receives a CRLF first, it should ignore the CRLF.
	*/


	int i = 0;
	while (content[i] == '\r' || content[i] == '\n') i++;

	uint8_t spaces = 0;
	for (; i < size && content[i] != '\n'; i++)
	{
		if (content[i] == ' ')
		{
			spaces++;
		}
	}

	if (content[i - 1] == '\r' && spaces == 2 && strncmp(&content[i - 9], "HTTP/1.1", 5) == NULL)
		return PacketType::HTTP;
	else
		return PacketType::unknown;

}

