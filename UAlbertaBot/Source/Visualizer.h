#pragma once

#include "..\..\SparCraft\source\SparCraft.h"

#ifdef USING_VISUALIZATION_LIBRARIES

#include <Common.h>

class Visualizer 
{
	Visualizer();
	Visualizer(int mapWidth, int mapHeight, int cellSize);

	SparCraft::Display display;
	SparCraft::GameState state;
	SparCraft::Map map;

	const IDType getPlayer(BWAPI::Unit * unit) const;
	const IDType getPlayer(BWAPI::Player * player) const;

public:

	// yay for singletons!
	static Visualizer &	Instance();
	
	void setBWAPIState();
	void setState(const SparCraft::GameState & state);
	void onFrame();
};

#endif