/*
 * UUID.cpp
 *
 *  Created on: 30 окт. 2018 г.
 *      Author: root
 */

#include "UUID.h"

namespace dev
{

	std::list<int> UUID::ids;

	bool UUID::init()
	{
		for (int i = 0; i < RANGE; i++)
			ids.push_back(i);
		return 1;
	}

	int UUID::getID()
	{
		if (ids.size() == 0) return -1;
		int temp = ids.front();
		ids.pop_front();
		return temp;
	}

	void UUID::setID(int id)
	{
		ids.push_back(id);
	}

	const bool UUID::init_invoker = UUID::init();

}
