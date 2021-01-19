#include "PDAPPacket.h"

PDAPPacket::PDAPPacket(const char* content, size_t size)
{
	type = static_cast<PDAPPacket::Type>(content[0]);
}
