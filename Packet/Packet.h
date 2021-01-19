#ifndef PACKET_H
#define PACKET_H

#include <cstring>
#include <cstdint>


enum class PacketType : uint8_t
{
	PDAP = 1,
	HTTP,
	ws,
	unknown,

	OK
};

class Packet
{
public:
	char content[1024];

	Packet();
	Packet(const char* content);
	~Packet();

	virtual void Parse(const char* content) = 0;
	static PacketType GetType(const char* content, size_t size);

};


#include "HTTPtoWSPacket.h"
#include "WSPacket.h"
#include "PDAPPacket.h"


#endif // !PACKET_H
