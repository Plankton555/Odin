#pragma once;

#include <Common.h>
#include "MicroManager.h"

class MicroManager;

class DragoonManager : public MicroManager
{
public:

	DragoonManager();
	~DragoonManager() {}
	void executeMicro(const UnitVector & targets, BWAPI::Position regroup = BWAPI::Position(0,0));

	BWAPI::Unit * chooseTarget(BWAPI::Unit * dragoon, const UnitVector & targets, std::map<BWAPI::Unit *, int> & numTargeting);
	BWAPI::Unit * closestDragoon(BWAPI::Unit * target, std::set<BWAPI::Unit *> & dragoonsToAssign);

	int getAttackPriority(BWAPI::Unit * unit);
	BWAPI::Unit * getTarget(BWAPI::Unit * dragoon, UnitVector & targets);
};
