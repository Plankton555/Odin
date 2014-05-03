#pragma once

#include "Common.h"
#include "Squad.h"

class SquadData
{
	// All squads. Indexed by SquadOrder enum
	std::vector<Squad>	squads;

	void				updateAllSquads();

public:

	SquadData();
	~SquadData() {}

	void				clearSquadData();

	void				addSquad(const Squad & squad);
	void				drawSquadInformation(int x, int y);

	void				update();
	void				setSquad(const UnitVector & units, const SquadOrder & order);
	void				setRegroup();
};
