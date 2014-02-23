#pragma once;

#include <Common.h>
#include "MicroManager.h"

class MicroManager;

class TransportManager : public MicroManager
{

public:

	TransportManager();
	~TransportManager() {}

	void executeMicro(const UnitVector & targets);
};
