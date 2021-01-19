/*
 * UUID.h
 *
 *  Created on: 30 окт. 2018 г.
 *      Author: root
 */

#ifndef UUID_UUID_H_
#define UUID_UUID_H_

#include <list>

namespace dev
{

	class UUID
	{
	private:
		UUID() {}
		static std::list<int> ids;
		static const int RANGE = 1000;
		static const bool init_invoker;
		static bool init();

	public:
		static int getID();
		static void setID(int id);
	};

}

#endif /* UUID_UUID_H_ */
