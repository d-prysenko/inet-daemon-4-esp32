/*
 * WSPacket.cpp
 *
 *  Created on: 12 февр. 2019 г.
 *      Author: root
 */

#include "WSPacket.h"

WSPacket::WSPacket()
{
	// TODO Auto-generated constructor stub
}

WSPacket::WSPacket(const char* content)
{
	Parse(content);
}

WSPacket::~WSPacket()
{
	delete[] this->decoded;
}

void printBits(size_t const size, void const * const ptr)
{
	byte *b = (byte*)ptr;
	byte byte;
	int i, j;

	for (i = 0; i < size; i++)
	{
		for (j = 7; j >= 0; j--)
		{
			byte = (b[i] >> j) & 1;
			printf("%u", byte);
		}
		printf(" ");
		if(i % 8 == 0 && i != 0 ) printf("\n");
	}
	puts("");
}

void WSPacket::Parse(const char * content)
{
	// strcpy(this->content, content);
	memcpy(this->content, content, 128);

	this->payload_len = this->initial_payload_len = this->content[1] & 0b01111111;
	//printf("Payload_len before: "); printBits(4, &payload_len); printf("\n");
	char* encoded = NULL;
	byte size = 0;

	if (this->initial_payload_len <= 125)
	{
		encoded = &this->content[6];
	}
	else
	{
		if (this->initial_payload_len == 126)
		{
			size = 2;
			encoded = &this->content[8];
		}
		else if (this->initial_payload_len == 127)
		{
			size = 8;
			encoded = &this->content[14];
		}

		byte j = 0;
		byte *b = (byte*)&(this->payload_len);
		for (int i = 1+size; i > 1; i--)
			b[j++] = this->content[i];

		//printf("Payload_len after: "); printBits(4, &payload_len); printf("\n");
	}

	memcpy(this->masking_key, &(this->content[2+size]), 4);
	//printf("Masking_key: "); printBits(4, masking_key); printf("\n");

	if (encoded)
	{
		this->decoded = new char[this->payload_len + 1];
		int i;
		for (i = 0; i < this->payload_len; i++)
		{
			this->decoded[i] = encoded[i] ^ this->masking_key[i % 4];
		}

		this->decoded[this->payload_len] = '\0';

		if ((byte)this->decoded[0] == (byte)Action::Set_State && (byte)this->decoded[5] == (byte)Action::Set_State)
			this->type = WSPacketType::set;

		else if ((byte)this->decoded[0] == (byte)Action::Get_State && (byte)this->decoded[5] == (byte)Action::Get_State)
			this->type = WSPacketType::get;

		else if ((byte)this->decoded[0] == (byte)Action::Connection && (byte)this->decoded[69] == (byte)Action::Connection)
			this->type = WSPacketType::authentication;

		else if ((byte)this->decoded[0] == (byte)Action::Disconnection)
			this->type = WSPacketType::disconnection;

//		printf("String: %s\n", this->decoded);
	}
	else
	{
		printf("ERROR: How could this happen?");
		//exit(0);
	}

}
