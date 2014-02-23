#pragma once;

#include <Common.h>
#include "MicroManager.h"

class MicroManager;


class ZealotManager : public MicroManager
{

public:

	ZealotManager();
	~ZealotManager() {}
	void executeMicro(const UnitVector & targets, BWAPI::Position regroup = BWAPI::Position(0,0));

	BWAPI::Unit * chooseTarget(BWAPI::Unit * zealot, const UnitVector & targets, std::map<BWAPI::Unit *, int> & numTargeting);
	BWAPI::Unit * closestZealot(BWAPI::Unit * target, std::set<BWAPI::Unit *> & zealotsToAssign);
	int getAttackPriority(BWAPI::Unit * unit);
	BWAPI::Unit * getTarget(BWAPI::Unit * zealot, UnitVector & targets);
};
