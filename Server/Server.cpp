/*
 * Server.cpp
 *
 *  Created on: 30 окт. 2018 г.
 *      Author: root
 */

/*

	WARNING!
	THIS CODE IS FROM 2K18 AND IT IS SUPER UNSAFE
	IT LOOKS VERY BAD, I KNOW THAT
	Maybe someday i'll fix it
	
*/

#include "Server.h"

template<class InputIt, class UnaryPredicate>
InputIt find_if(InputIt first, InputIt last, UnaryPredicate p)
{
    for (; first != last; ++first) {
        if (p(*first)) {
            return first;
        }
    }
    return last;
}
//void printBits(size_t const size, void const * const ptr)
//{
//	uint8_t *b = (uint8_t*)ptr;
//	uint8_t uint8_t;
//	int i, j;
//
//	for (i = 0; i < size; i++)
//	{
//		for (j = 7; j >= 0; j--)
//		{
//			uint8_t = (b[i] >> j) & 1;
//			printf("%u", uint8_t);
//		}
//		printf(" ");
//		if(i % 8 == 0 && i != 0 ) printf("\n");
//	}
//	puts("");
//}
Device::Device()
{
	this->id = 0;
	this->attempt = 0;
	this->state = 0;
}

Device::Device(int id, bool state)
{
	this->id = id;
	this->attempt = 0;
	this->state = state;
}

Server::Server() // @suppress("Class members should be properly initialized")
{
	conn = mysql_init(NULL);

	if(!mysql_real_connect(conn, host, user, pass, dbname, port, unix_socket, flag))
	{
		printf("Error in connect to DB\n");
		exit(1);
	}

	is_file = false;
	lsn_socket = socket(AF_INET, SOCK_STREAM, 0);
//	pass = "a020bdf568793dee747a220086c5e53c8b3af5abd959583a31297f303577efe6";
	if (lsn_socket < 0)
	{
		perror("socket isn't alive");
		exit(1);
	}

	int opt = 1;
	if (setsockopt (lsn_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt)) == -1) perror("setsockopt");

	fcntl(lsn_socket, F_SETFL, O_NONBLOCK);

	allsockets.clear();
//	devices.clear();
}

Server::~Server()
{
	if(is_file) fclose(this->pf_db);
	shutdown(lsn_socket, SHUT_RDWR);
	close(lsn_socket);
}

int Server::send(int socket, void *buffer, size_t size, int flags)
{
	uint8_t* response = (uint8_t*)buffer;
	size_t _size = size;

	if(flags == 1)
	{
		_size += 2;
		response = new uint8_t[_size];
		response[0] = 0b10000010;
		response[1] = size;
		memcpy(&response[2], buffer, size);
	}

	::send(socket, response, _size, 0);
}

void Server::startServer(int port)
{
	sockaddr_in addr_serv;
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_addr.s_addr = INADDR_ANY;
	addr_serv.sin_port = htons(port);

	if (bind(lsn_socket, (sockaddr*)&addr_serv, sizeof(addr_serv)) < 0)
	{
		perror("Failed in bind");
		exit(2);
	}

	if (listen(lsn_socket, SOMAXCONN) < 0)
	{
		perror("Failed in listen");
		exit(3);
	}

	pthread_attr_t attr1, attr2;
	pthread_attr_init(&attr1);
	pthread_attr_init(&attr2);

	pthread_create(&process_tid, &attr1, &process_helper, this);
	pthread_create(&ping_tid, &attr2, &ping_helper, this);
}

void* Server::ping_helper(void* context)
{
	return ((Server *)context)->ping();
}

void* Server::ping(void)
{
	while (true)
	{
		for(auto itr = allsockets.begin(); itr != allsockets.end(); ++itr)
		{
			if((*itr).type == EntityType::device)
			{

				if ((*itr).p_dev->attempt == 1)
				{
					// TODO: set up the time of disconnect
					printf("%i disconnected(no response)\n", (*itr).p_dev->id);
					delete (*itr).p_dev;
					auto itr2 = itr;
					int key = ++itr2.key();
					allsockets.erase(itr);
					itr = allsockets.find(key);
					continue;
				}
				printf("PING!!! %i\n", (*itr).p_dev->id);
				char ping = Action::Ping;
				if (send(itr.key(), &ping, 1, 0) < 0)
					printf("Ping failed\n");

				(*itr).p_dev->attempt++;
			}
		}
		if(is_file)
		{
			std::time_t t = std::time(0);
			std::tm* time = std::localtime(&t);

			if((time->tm_min * 60 + time->tm_sec) - (this->time->tm_min * 60 + this->time->tm_sec) > 30)
			{
				fclose(this->pf_db);
				is_file = false;
			}
		}

		sleep(7);
	}
}

void* Server::process_helper(void* context)
{
	return ((Server *)context)->process();
}

void* Server::process(void)
{
	while(true)
	{
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(lsn_socket, &fds);

		int max = 0;

		for(auto itr = allsockets.begin(); itr != allsockets.end(); ++itr)
		{
			FD_SET(itr.key(), &fds);
		}

		max = allsockets.getLast().key();
		int mx = (lsn_socket >= max )? lsn_socket : max;

		if(select(mx+1, &fds, NULL, NULL, NULL) <= 0)
		{
			perror("select");
			exit(3);
		}

		if(FD_ISSET(lsn_socket, &fds))
		{
			int sock;
			sockaddr_in addr;
			unsigned int len = sizeof(addr);

			if ((sock = ::accept(lsn_socket, (sockaddr*)&addr, &len)) < 0)
			{
				perror("accept failed");
				exit(50);
			}
			fcntl(sock, F_SETFL, O_NONBLOCK);

			char ip[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
			printf("TCP is established\nip: %s\n", ip);

			allsockets.insert(sock, inet_client(addr.sin_addr));
		}

		for(auto itr = allsockets.begin(); itr != allsockets.end(); ++itr)
		{
			if(FD_ISSET(itr.key(), &fds))
			{
				char buf[1024];
				memset(buf, 0, 1024);

				// read bytes from client socket
				int bytes_read = recv(itr.key(), buf, 1024, 0);

				if(bytes_read <= 0)
				{
					// connection is closed, deleting socket from sets
					close(itr.key());
					printf("%i disconnected\n\n", (*itr).UUID);
					delete (*itr).p_dev;
					auto itr2 = itr;
					int key = ++itr2.key();
					allsockets.erase(itr);
					itr = allsockets.find(key);
					continue;
				}

				// shows received buffer
				//printf("%i's buffer: %s\n", (*itr).UUID, buf);
//				printf("%i's buffer bits:\n", (*itr).UUID);
//				printBits(90, buf);

				PacketType type = Packet::GetType(buf);

				// FROM browser
				if (type == PacketType::HTTP)
				{
					HTTPtoWSPacket* p = new HTTPtoWSPacket(buf);

					if (p->isKeySet)
					{
						char response[256] =
							"HTTP/1.1 101 Switching Protocols\r\n"
							"Upgrade: websocket\r\n"
							"Connection: Upgrade\r\n"
							"Sec-WebSocket-Accept: ";
						strcat(response, p->getWebSocketAccept().c_str());
						strcat(response, "\r\n\r\n");

						send(itr.key(), response, strlen(response), 0);
					}

					delete p;
				}
				// FROM browser
				else if (type == PacketType::ws || (*itr).type == EntityType::browser)
				{
					WSPacket* w = new WSPacket(buf);

//					printf("Decoded: %s\n", w->decoded);
//					printf("Payload length: %i\n", w->payload_len);
//					printf("Masking key: %s\n", w->masking_key);
//					printf("Masking key: "); printBits(4, w->masking_key); printf("\n");
//					printBits(72, w->decoded);

					if ((*itr).type == EntityType::browser)
					{
						// parse his request

						if ( w->type == WSPacketType::set || w->type == WSPacketType::get )
						{
							int device_id = 0;
							uint8_t state = 0;
							memcpy(&device_id, &w->decoded[1], 4);
							uint8_t* p_id = (uint8_t*)&device_id;
							state = p_id[3] >> 7;
							p_id[3] = p_id[3] & 0b01111111;

							MYSQL_ROW row;

							char query[64] = "SELECT * FROM devices WHERE id = ";
							char c_id[16];
							sprintf(c_id, "%d", device_id); // &query[40]
							strcat(query, c_id);

							mysql_query(conn, query);
							res = mysql_store_result(conn);

							row = mysql_fetch_row(res);

							if(mysql_num_rows(res) && atoi(row[0]) == (*itr).p_brow->id)
							{
								auto it = find_if(allsockets.begin(), allsockets.end(),
										[&device_id](const inet_client& c){ if (c.type == EntityType::device) return c.p_dev->id == device_id; });

								if(it != allsockets.end())
								{


									if (w->type == WSPacketType::set)
									{
										// set state
										uint8_t response[3] = { Action::Set_State, state, Action::Set_State };
										send(it.key(), response, 3, 0);
										(*it).p_dev->state = state;

										printf("set %d to state %d\n", device_id, state);
									}
									else
									{
										// get state
										p_id[3] = p_id[3] | ((*it).p_dev->state << 7);


										// TODO: use htonl() for reverse network byte order instead
										uint8_t response[6];
										response[0] = Action::Get_State;
										for(int i = 3; i >= 0; i--) response[4-i] = p_id[i];
										response[5] = Action::Get_State;

										send(itr.key(), response, 6, 1);
									}

								}
								else
								{
									uint8_t response[6];
									response[0] = Action::Error_Device_Not_Available;
									for(int i = 3; i >= 0; i--) response[4-i] = p_id[i];
									response[5] = Action::Error_Device_Not_Available;

									send(itr.key(), response, 6, 1);
								}
							}
						}
						else if( w->type == WSPacketType::disconnection )
						{
							MYSQL_ROW row;

							char query[64] = "SELECT * FROM users WHERE id = ";
							sprintf(&query[31], "%d", (*itr).p_brow->id);
							strcat(query, " LIMIT 1");

							mysql_query(conn, query);
							res = mysql_store_result(conn);
							row = mysql_fetch_row(res);

							if(mysql_num_rows(res) != 0)
								printf("%s (UUID: %i) have left\n\n", row[1], (*itr).UUID);
							else
								printf("<email> (UUID: %i) have left\n\n", (*itr).UUID);

							delete (*itr).p_brow;
							close(itr.key());
							auto itr2 = itr;
							int key = ++itr2.key();
							allsockets.erase(itr);
							itr = allsockets.find(key);
							continue;
						}

					}
					// if he DON'T consists in browser clients list
					// try to authenticate him
					else if ((*itr).type == EntityType::undefined && w->decoded[0] != Action::Disconnection)
					{
						// it would be first packet which must contains cookies

						// browser wants to connect
						// 0b01111110 + id + hash + remix_id + 0b01111110
						// id   - 4 bytes
						// hash - 64 bytes
						// remix_id - 16 bytes

						if( w->type == WSPacketType::authentication )
						{
							//int id = 0;
							char hash[65];
//							char remix_id[17];

							int* p_id = reinterpret_cast<int*>(&w->decoded[1]);

							//memcpy(&id, &w->decoded[1], 4);
							memcpy(hash, &w->decoded[5], 64);
							hash[64] = '\0';

							MYSQL_ROW row;

							char query[128] = "SELECT * FROM sessions WHERE id = ";
							sprintf(&query[34], "%d", *p_id);
							strcat(query, " LIMIT 1");

							mysql_query(conn, query);
							res = mysql_store_result(conn);

							int num_rows = mysql_num_rows(res);

							if(num_rows)
							{
								row = mysql_fetch_row(res);

								memset(query, 0, 128) ;
								strcpy(query, "SELECT * FROM users WHERE id = ");
								strcat(query, row[0]);
								strcat(query, " LIMIT 1");

								mysql_query(conn, query);
								res = mysql_store_result(conn);
								row = mysql_fetch_row(res);

								num_rows = mysql_num_rows(res);

								int id = atoi(row[0]);
								char* db_hash = row[4];

								if(num_rows != 0 && strcmp(db_hash, hash) == 0)
								{

									for(auto it = allsockets.begin(); it != allsockets.end(); ++it)
									{
										if((*it).type == EntityType::browser)
											if((*it).p_brow->id == id)
												if(it.key() != itr.key())
												{
													// Login from a new browser
													// Notice already connected clients
													char response = Action::Note_New_Connection;
													send(it.key(), &response, 1, 1);
												}
									}

									printf("New connection %i\n", (*itr).UUID);
									(*itr).type = EntityType::browser;
									(*itr).p_brow = new Browser(id);
									char response = Action::OK;
									send(itr.key(), &response, 1, 1);
								}
								else
								{

									char response = Action::Error_Authentication;
									send(itr.key(), &response, 1, 1);
									close(itr.key());
									delete (*itr).p_dev;
									printf("%i authentication error!\n", id);
									auto itr2 = itr;
									int key = ++itr2.key();
									allsockets.erase(itr);
									itr = allsockets.find(key);
									continue;
								}
							}
						}
						else
						{
							if((*itr).type != EntityType::marked)
							{
								// if this is not an authentication packet, send error message
								char response = Action::Error_Not_An_Auth_Packet;
								send(itr.key(), &response, 1, 0);
								(*itr).type = EntityType::marked;
								printf("WARNING: is it DDOS? Ip: %s\n", (*itr).ip);
							}
							else
							{
								close(itr.key());
								printf("WARNING: %s definitely try's DDOS us\n\n", (*itr).ip);
								delete (*itr).p_dev;
								auto itr2 = itr;
								int key = ++itr2.key();
								allsockets.erase(itr);
								itr = allsockets.find(key);
								continue;
							}
						}
					}

					delete w;
				}
				// TO browser
				else if ((*itr).type == EntityType::device || (type >= PacketType::service_connection && type <= PacketType::service_ping))
				{
					// someone device wants to connect
					// 0b01111110 + id + mac + hash + 0b01111110
					// id   - 4 bytes
					// mac  - 6 bytes
					// hash - 64 bytes
					if (type == PacketType::service_connection)
					{
						int id = 0;
						uint8_t mac[6] = {0};
						char hash[65];

						printf("New connection\n");
						memcpy(&id, &buf[1], 4);
						memcpy(mac, &buf[5], 6);
						memcpy(hash, &buf[11], 64);
						hash[64] = '\0';

						if(rc_uuids(id, mac, hash) != 1)
						{
							// TODO: delete it from allsockets list
							printf("Something went wrong\n");
							return;
						}

						// try to search connected client with same id
						auto it = find_if(allsockets.begin(), allsockets.end(),
								[&id](const inet_client& c){if(c.type == EntityType::device) return c.p_dev->id == id; else return false;});

						if(it != allsockets.end())
						{
							// maybe this is really a device?
							printf("WARNING(connection): %i already connected.\n\n", id);
							(*itr).type = waiting_device;
							// waiters.insert(itr.key(), Device(&(itr.val())));
						}
						else
						{
							// if all is ok, insert device in list and show his id

							bool startupState = getStartupState(id);

							(*itr).type = device;
							(*itr).p_dev = new Device(id, startupState);

							MYSQL_ROW row;

							char query[64] = "SELECT * FROM devices WHERE id = ";
							sprintf(&query[33], "%d", id);
							strcat(query, " LIMIT 1");

							mysql_query(conn, query);
							res = mysql_store_result(conn);

							//int num_rows = mysql_num_rows(res);

							row = mysql_fetch_row(res);
							int user_id = atoi(row[0]);

							printf("User id which have this device: %i\n", user_id);

							for(auto it = allsockets.begin(); it != allsockets.end(); ++it)
							{
								if((*it).type == EntityType::browser)
									if((*it).p_brow->id == user_id)
									{
										uint8_t* p_id = (uint8_t*)&id;

										// terrible serialization
										char response[6];
										response[0] = Action::Get_State;
										for(int i = 3; i >= 0; i--) response[4-i] = p_id[i];
										response[1] = response[1] | (startupState << 7);
										response[5] = Action::Get_State;

										send(it.key(), response, 6, 1);
									}
							}

							// TODO:
							// open db
							// search user who have device with this id
							// get user id
							// find user in socket list
							// auto it = find_if(allsockets.begin(), allsockets.end(),
							// 			 [&id](const inet_client& c){ if (c.type == browser) return c.p_brow->id == id; });
							// if(it != allsockets.end())
							// {
							// 		char resp[5];
							// 		resp[0] = Action::Set_State;
							// 		memcpy(&resp[1], &id, 4);
							// 		resp[1] = resp[1] | (startupState << 7);
							// 		send(it.key(), resp, 5, 0);
							// }

							printf("%i signed in, startup state: %i\n", id, startupState);

						}

					}
					else if (type == PacketType::service_ping)
					{
						printf("Ping from %i\n", (*itr).p_dev->id);
						(*itr).p_dev->attempt = 0;
					}
					else if(type == PacketType::service_disconnection)
					{
						unsigned int id = (*itr).p_dev->id;
						MYSQL_ROW row;

						char query[64] = "SELECT * FROM devices WHERE id = ";
						sprintf(&query[33], "%d", id);
						strcat(query, " LIMIT 1");

						mysql_query(conn, query);
						res = mysql_store_result(conn);

						//int num_rows = mysql_num_rows(res);

						row = mysql_fetch_row(res);
						int user_id = atoi(row[0]);

						printf("WARNING! UNEXPECTED DISCONNECTOIN: %i\n", id);

						for(auto it = allsockets.begin(); it != allsockets.end(); ++it)
						{
							if((*it).type == EntityType::browser)
								if((*it).p_brow->id == user_id)
								{
									uint8_t* p_id = (uint8_t*)&id;

									uint8_t response[6];
									response[0] = Action::Error_Device_Not_Available;
									for(int i = 3; i >= 0; i--) response[4-i] = p_id[i];
									response[5] = Action::Error_Device_Not_Available;

									send(it.key(), response, 6, 1);
								}
						}
					}
					else if(type == PacketType::service_get)
					{
						uint8_t state = buf[1];
						int id = (*itr).p_dev->id;

						MYSQL_ROW row;

						char query[64] = "SELECT * FROM devices WHERE id = ";
						sprintf(&query[33], "%d", id);
						strcat(query, " LIMIT 1");

						mysql_query(conn, query);
						res = mysql_store_result(conn);

						//int num_rows = mysql_num_rows(res);

						row = mysql_fetch_row(res);
						int user_id = atoi(row[0]);

						printf("Service_get %i state %i user id: %i\n", id, state, user_id);
						(*itr).p_dev->state = state;

						for(auto it = allsockets.begin(); it != allsockets.end(); ++it)
						{
							if((*it).type == EntityType::browser)
								if((*it).p_brow->id == user_id)
								{
									// Loggined in from new browser
									// Noticed all connected clients
					//						char response = Action::Note_New_Connection;
					//						send(it.key(), &response, 1, 1);

									uint8_t* p_id = (uint8_t*)&id;

									char response[6];
									response[0] = Action::Get_State;
									for(int i = 3; i >= 0; i--) response[4-i] = p_id[i];
									response[1] = response[1] | (state << 7);
									response[5] = Action::Get_State;

									send(it.key(), response, 6, 1);
								}
						}
					}

				}
				else
				{
					printf("Warning: Undefined packet!\n");
				}

				//char buf2[] = "ok\n\r";
				//send((*itr).sock, buf2, sizeof(buf2), 0);
			}
		}
	}
}

int Server::open_my_db()
{
	this->t = std::time(0);
	this->time = std::localtime(&t);

	if(!is_file)
	{
		char path[64] = "devices.bin.db";
		pf_db = fopen(path, "rb+");
		if(!pf_db)
		{
			printf("Unable to open file\n");
			return -2;
		}
		is_file = true;
	}
	return 1;
}

int Server::rc_uuids(int id, uint8_t* mac, const char* hash)
{
	open_my_db();

	uint8_t* umac = reinterpret_cast<uint8_t*>(mac);

	int fs = (id-1)*DB_ROW_SIZE;
	fseek(pf_db, fs, SEEK_SET);
	int DB_id;
	fread(reinterpret_cast<char*>(&DB_id), sizeof(int), 1, pf_db);

	uint8_t* b = (uint8_t*)&DB_id;
	b[3] = b[3] & 0b01111111;

	if(id != DB_id) {printf("id compare failed\n");return -2;}

	uint8_t DB_mac[6];
	fseek(pf_db, fs+sizeof(int)+3, SEEK_SET);
	fread(DB_mac, 1, 6, pf_db);

	for(uint8_t i = 0; i < 6; i++)
		if(DB_mac[i] != umac[i]){printf("mac compare failed\n");return -1;}

	char DB_hash[65];
	fseek(pf_db, fs+sizeof(int)+3+6, SEEK_SET);
	fread(DB_hash, 1, 64, pf_db);
	DB_hash[64] = '\0';

	if(strcmp(DB_hash, hash) == 0)
	{
		return 1;
	}

	return 0;
}

bool Server::getStartupState(int id)
{
	open_my_db();

	int fs = (id-1)*DB_ROW_SIZE;
	fseek(pf_db, fs, SEEK_SET);

	int DB_id;
	fread(reinterpret_cast<char*>(&DB_id), sizeof(int), 1, pf_db);

	uint8_t* b = (uint8_t*)&DB_id;
	bool foo = b[3] & 0b10000000;
	b[3] = b[3] & 0b01111111;

	if(id != DB_id) {printf("id comparing failed\n");return -1;}

	return foo;
}

int Server::setStartupState(int id, bool state)
{
	open_my_db();

	int fs = (id-1)*DB_ROW_SIZE;
	fseek(pf_db, fs, SEEK_SET);

	int DB_id;
	fread(reinterpret_cast<char*>(&DB_id), sizeof(int), 1, pf_db);

	uint8_t* b = (uint8_t*)&DB_id;
	b[3] = b[3] & 0b01111111;

	if(id != DB_id) {printf("id comparing failed\n");return -1;}

	b[3] = (b[3] & 0b01111111) | (state << 7);

	fseek(pf_db, fs, SEEK_SET);
	fwrite(&DB_id, sizeof(int), 1, pf_db);

}

void Server::readDate(int id, uint8_t* date)
{
	uint8_t minimized[3];
	open_my_db();

	int fs = (id-1)*DB_ROW_SIZE;
	fseek(pf_db, fs+sizeof(int), SEEK_SET);

	fread(minimized, 1, 3, pf_db);

	normalizeDate(date, minimized);
}

void Server::writeDate(int id)
{
	uint8_t dateToFile[3];
	open_my_db();

	int fs = (id-1)*DB_ROW_SIZE;
	fseek(pf_db, fs+sizeof(int), SEEK_SET);

	std::time_t t = std::time(0);
	std::tm* time = std::localtime(&t);

	minimizeDate(time, dateToFile);

	fwrite(dateToFile, 3, 1, pf_db);
}

void Server::minimizeDate(std::tm* time, uint8_t* minimized)
{
	minimized[0] = (time->tm_year - 119) << 4;
	minimized[0] = minimized[0] | time->tm_mon;

	minimized[1] = time->tm_mday << 3;
	minimized[1] = minimized[1] | (time->tm_hour >> 2);

	minimized[2] = time->tm_hour << 6;
	minimized[2] = minimized[2] | time->tm_min;
}

void Server::normalizeDate(uint8_t* dest, uint8_t* source)
{
	dest[0] = source[0] >> 4;
	dest[1] = source[0] & 0b00001111;
	dest[2] = source[1] >> 3;
	dest[3] = ((source[1] << 2) & 0b00011100) | (source[2] >> 6);
	dest[4] = source[2] & 0b00111111;
}


