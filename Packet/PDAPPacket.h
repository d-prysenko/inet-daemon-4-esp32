#pragma once

#include <cstdint>


class PDAPPacket
{
public:
	static enum class Type : uint8_t
	{
		Connection = 1,
		Disconnection,
		Ping,

		Set_State,
		Get_State,
		Set_Startup_State,

		Error_Authentication,
		Error_Not_An_Auth_Packet,
		Error_Device_Not_Available,

		Note_New_Connection,

		OK
	};

	constexpr static uint16_t packetSizes[] =
	{
		0,
		76,	// connection
		3,	// disconnection
		3,	// ping
		3,	// set state
		3	// get state
	};

	PDAPPacket(const char* content, size_t size);

	Type type;
};

