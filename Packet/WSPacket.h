/*
 * WSPacket.h
 *
 *  Created on: 12 февр. 2019 г.
 *      Author: root
 */

#ifndef WSPACKET_H_
#define WSPACKET_H_

#include "Packet.h"
#include <stdio.h>

//namespace CTRL_BITS
//{
//	static const byte CONNECTION					= 0b11111101;
//	static const byte SET_STATE						= 0b11111110;
//	static const byte GET_STATE						= 0b11111100;
//	static const byte GET_ALL_STATES				= 0b11111000;
//	static const byte ERROR_AUTHENTICATION			= 0b00000001;
//	static const byte ERROR_NOT_AN_AUTH_PACKET		= 0b00000010;
//	static const byte ERROR_DEVICE_NOT_AVAILABLE	= 0b00000100;
//	static const byte OK							= 0b11111111;
//}
void printBits(size_t const size, void const * const ptr);
enum WSPacketType
{
	set,
	get,
	authentication,
	disconnection
};

class WSPacket : public Packet
{
public:
	int initial_payload_len;
	int payload_len;
	byte masking_key[4];
	char* decoded;
	WSPacketType type;

	WSPacket();
	WSPacket(const char* content);
	~WSPacket();

	void Parse(const char* content) override;
};

#endif /* WSPACKET_H_ */
