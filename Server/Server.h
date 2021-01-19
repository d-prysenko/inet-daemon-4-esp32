/*
 * Server.h
 *
 *  Created on: 30 окт. 2018 г.
 *      Author: root
 */

/*

	WARNING!
	THIS CODE IS FROM 2K18
	IT LOOKS VERY BAD, I KNOW THAT
	Maybe someday i'll fix it
	
*/

#ifndef SERVER_H_
#define SERVER_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <ctime>
#include <stdlib.h>
#include <chrono>
#include <pthread.h>
//#include <set>

//#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <algorithm>
#include <mysql/mysql.h>
#include "../BSTree.h"

#include "../Packet/Packet.h"
#include "../UUID/UUID.h"

using namespace std::chrono;

struct inet_client;

struct Device
{
	uint8_t attempt;
	bool state;
	int id;

	Device();
	Device(int id, bool state = 0);
};

struct Browser
{
	int id;
	
	Browser(int id)
		: this->id(id)
	{}
};

enum EntityType
{
	browser,
	device,
	waiting_device,
	undefined,
	marked
};

struct inet_client
{
	char ip[INET_ADDRSTRLEN];
	int UUID;
	EntityType type;

	union
	{
		Device* p_dev;
		Browser* p_brow;
	};

	inet_client()
		: UUID(-1), type(EntityType::undefined), p_dev(nullptr)
	{ }

	inet_client(struct in_addr& sin_addr)
		: type(EntityType::undefined), p_dev(nullptr)
	{
		inet_ntop(AF_INET, &sin_addr, ip, INET_ADDRSTRLEN);
		UUID = dev::UUID::getID();
		type = EntityType::undefined;
		p_dev = nullptr;
	}
};

class Server
{
private:

	int lsn_socket;
	avl_array<int, inet_client, std::uint16_t, 640, true> allsockets;

	int send(int socket, void *buffer, size_t size, int flags = 0);
	// void kick(avl_array<int, inet_client, std::uint16_t, 640, true>::iterator &itr);
	// void kick(int device_id);

	constexpr static char* host = "localhost";
	constexpr static char* user = "admin";
	constexpr static char* pass = "@root-13";
	constexpr static char* dbname = "crythan";

	unsigned int port = 3306;
	constexpr static char* unix_socket = NULL;
	unsigned int flag = 0;

	MYSQL* conn;
	MYSQL_RES* res;

	// TODO: remove this. c'mon you have a normal database
	FILE* pf_db;
	bool is_file;
	const uint8_t DB_ROW_SIZE = sizeof(int) + 3 + 6 + 64;
	std::time_t t;
	std::tm* time;

	int open_my_db();

	bool getStartupState(int id);
	int  setStartupState(int id, bool state);
	void minimizeDate(std::tm* time, uint8_t* minimized);
	void normalizeDate(uint8_t* dest, uint8_t* source);
	void readDate(int id, uint8_t* date);
	void writeDate(int id);

public:

	pthread_t process_tid, ping_tid;
	int rc_uuids(int id, uint8_t* mac, const char* hash);
	Server();
	~Server();

	void startServer(int);
	static void *process_helper(void *context);
	void* process();
	static void *ping_helper(void *context);
	void* ping();

};

#endif /* SERVER_H_ */
