#pragma once

#include "Common.h"
#include "BWAPI.h"

#include "..\..\SparCraft\source\SparCraft.h"

class UnitCommandData
{	
	BWAPI::Unit * _unit;

	int		_phase,
			_enteredReady,
			_enteredAttacking,
			_enteredReloading,
			_waitCommandGiven;

public:

	enum	{ NONE, READY, ATTACKING, RELOADING };

	UnitCommandData();
	UnitCommandData(BWAPI::Unit * unit);

	void update();
	void waitCommand();
	void attackCommand();
	const bool isWaiting() const;
	const int getPhase() const;
};
