#pragma once

#include <Common.h>
#include "micromanagement/MicroManager.h"

class GridCell
{
public:

	int					timeLastVisited, timeLastOpponentSeen;
	UnitVector			ourUnits;
	UnitVector			oppUnits;
	BWAPI::Position		center;

	GridCell() :		timeLastVisited(0), timeLastOpponentSeen(0)  {}
	
};


class MapGrid {

	MapGrid();
	MapGrid(int mapWidth, int mapHeight, int cellSize);

	int							cellSize;
	int							mapWidth, mapHeight;
	int							rows, cols;
	int							lastUpdated;

	bool						contains(UnitVector & units, BWAPI::Unit * unit);

	std::vector< GridCell >		cells;

	void						calculateCellCenters();

	void						clearGrid();
	BWAPI::Position				getCellCenter(int x, int y);

	BWAPI::Position				naturalExpansion;

public:

	// yay for singletons!
	static MapGrid &	Instance();

	void				update();
	void				GetUnits(UnitVector & units, BWAPI::Position center, int radius, bool ourUnits, bool oppUnits);
	BWAPI::Position		getLeastExplored();
	BWAPI::Position		getNaturalExpansion();

	GridCell & getCellByIndex(int r, int c)		{ return cells[r*cols + c]; }
	GridCell & getCell(BWAPI::Position pos)		{ return getCellByIndex(pos.y() / cellSize, pos.x() / cellSize); }
	GridCell & getCell(BWAPI::Unit * unit)		{ return getCell(unit->getPosition()); }
};
