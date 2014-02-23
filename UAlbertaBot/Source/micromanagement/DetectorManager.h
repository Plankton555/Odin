#pragma once;

#include <Common.h>
#include "MicroManager.h"

class MicroManager;

class DetectorManager : public MicroManager
{

	std::map<BWAPI::Unit *, bool>	cloakedUnitMap;

	bool isAssigned(BWAPI::Unit * unit);

	BWAPI::Unit * unitClosestToEnemy;

public:

	DetectorManager();
	~DetectorManager() {}

	void setUnitClosestToEnemy(BWAPI::Unit * unit) { unitClosestToEnemy = unit; }
	void executeMicro(const UnitVector & targets);

	BWAPI::Unit * closestCloakedUnit(const UnitVector & cloakedUnits, BWAPI::Unit * detectorUnit);
};
