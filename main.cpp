//============================================================================
// Name        : main.cpp
// Author      : Arktis
// Version     :
// Copyright   : Your copyright notice
// Description : Server in C++, Ansi-style
//============================================================================

/*

	WARNING!
	THIS CODE IS FROM 2K18
	IT LOOKS VERY BAD, I KNOW THAT
	Maybe someday i'll fix it
	
*/

#include "Server/Server.h"

Server s;

int main()
{

	s.startServer(666);

	printf("The server was successfully started\n\n");

//	char buf[128] = {0};
//	buf[0] = 0b11111101;
//
//	buf[1] = 3;
//	buf[2] = 0;
//	buf[3] = 0;
//	buf[4] = 0;
//
//	buf[5] = 0xEC;
//	buf[6] = 0xFA;
//	buf[7] = 0xBC;
//	buf[8] = 0x13;
//	buf[9] = 0x8A;
//	buf[10] = 0x76;
//
//	char* uid3 = "e020bdf568793dee747a220086c5e53c8b3af5abd959383a31297f303577efe6"; //
//
//	memcpy(&buf[11], uid3, 64);
//
//	buf[75] = 0b11111101;
//
////	MYPacket* m = new MYPacket(buf);
//
//	int id = 0;
//	byte mac[6] = {0};
//	char hash[65];
//
//	printf("New connection\n");
//	memcpy(&id, &buf[1], 4);
//	memcpy(mac, &buf[5], 6);
//	memcpy(hash, &buf[11], 64);
//	hash[64] = '\0';
//
//	for(int i = 0; i < 6; i++)
//		printf("%02X:", mac[i]);
//
//	printf("\n");
//	if(s.rc_uuids(id, mac, hash) != 1)
//	{
//		printf("Something went wrong\n");
//	}

//	delete m;

	pthread_join(s.process_tid, NULL);
	pthread_join(s.ping_tid, NULL);

	return 0;
}

