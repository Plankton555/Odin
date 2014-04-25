#pragma once;

#include <Common.h>
#include "MicroManager.h"

class MicroManager;

class HighTemplarManager : public MicroManager
{
	
public:

	HighTemplarManager();
	~HighTemplarManager() {}

	void executeMicro(const UnitVector & targets);
	void smartStormUnit(BWAPI::Unit * attacker, BWAPI::Unit * target) const ;
};
