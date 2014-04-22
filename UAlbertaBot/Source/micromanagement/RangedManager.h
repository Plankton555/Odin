#pragma once;

#include <Common.h>
#include "MicroManager.h"

class MicroManager;

class RangedManager : public MicroManager
{
	BWAPI::Unit * unitClosestToEnemy;
	void microCarrier(BWAPI::Unit * carrier, UnitVector targets);
	void trainSubUnits(BWAPI::Unit * unit);

public:

	RangedManager();
	~RangedManager() {}
	void executeMicro(const UnitVector & targets);

	BWAPI::Unit * chooseTarget(BWAPI::Unit * rangedUnit, const UnitVector & targets, std::map<BWAPI::Unit *, int> & numTargeting);
	BWAPI::Unit * closestrangedUnit(BWAPI::Unit * target, std::set<BWAPI::Unit *> & rangedUnitsToAssign);

	int getAttackPriority(BWAPI::Unit * rangedUnit, BWAPI::Unit * target);
	BWAPI::Unit * getTarget(BWAPI::Unit * rangedUnit, UnitVector & targets);

	void kiteTarget(BWAPI::Unit * rangedUnit, BWAPI::Unit * target);
	
	void setUnitClosestToEnemy(BWAPI::Unit * unit) { unitClosestToEnemy = unit; }
};
