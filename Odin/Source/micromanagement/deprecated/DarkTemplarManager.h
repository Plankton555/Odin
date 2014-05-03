#pragma once

#include <Common.h>
#include "MicroManager.h"
#include "../InformationManager.h"

class MicroManager;

class DarkTemplarManager : public MicroManager
{
public:

	
	bool goForIt;

	DarkTemplarManager();
	~DarkTemplarManager() {}
	void executeMicro(const UnitVector & targets, BWAPI::Position regroup = BWAPI::Position(0,0));

	BWAPI::Unit * chooseTarget(BWAPI::Unit * zealot, const UnitVector & targets, std::map<BWAPI::Unit *, int> & numTargeting);
	BWAPI::Unit * closestDarkTemplar(BWAPI::Unit * target, std::set<BWAPI::Unit *> & zealotsToAssign);
	int getAttackPriority(BWAPI::Unit * unit);
	BWAPI::Unit * getTarget(BWAPI::Unit * zealot, UnitVector & targets);
};
