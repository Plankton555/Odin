#pragma once;

#include <Common.h>
#include "MicroManager.h"

class MicroManager;

class DetectorManager : public MicroManager
{

	std::map<BWAPI::Unit *, bool>	cloakedUnitMap;

	bool isAssigned(BWAPI::Unit * unit);

	BWAPI::Unit * unitClosestToEnemy;

	bool detectorsInMain;
	bool detectorsInNat;

	bool scoutBase(BWAPI::Unit * obs, BWTA::Region * base);
	bool baseRecentlyScouted(BWTA::Region * base);
	void scoutOtherBases(BWAPI::Unit * obs);
	void setDetectorsInBase(BWTA::Region * base);
	bool detectorInBase(BWTA::Region * base);

	BWTA::Region * getNatBase();
	BWTA::Region * natBase;
	BWTA::Region * getMainBase();

public:

	DetectorManager();
	~DetectorManager() {}

	void setUnitClosestToEnemy(BWAPI::Unit * unit) { unitClosestToEnemy = unit; }
	void executeMicro(const UnitVector & targets);

	BWAPI::Unit * closestCloakedUnit(const UnitVector & cloakedUnits, BWAPI::Unit * detectorUnit);
};
