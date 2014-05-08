#pragma once;

#include <Common.h>
#include "MicroManager.h"

class MicroManager;

class HighTemplarManager : public MicroManager
{
	BWAPI::Unit * unitClosestToEnemy;
public:

	HighTemplarManager();
	~HighTemplarManager() {}

	void executeMicro(const UnitVector & targets);
	void smartStormUnit(BWAPI::Unit * attacker, BWAPI::Unit * target) const ;

	void setUnitClosestToEnemy(BWAPI::Unit * unit) { unitClosestToEnemy = unit; }
};
