#pragma once

#include "Common.h"
#include <vector>
#include "BWAPI.h"
#include "DistanceMap.hpp"
#include "InformationManager.h"
#include "base/BuildingPlacer.h"

// provides useful tools for analyzing the starcraft map
// calculates connectivity and distances using flood fills
class MapTools
{
	std::map<BWAPI::Position, DistanceMap>	allMaps;

	// holds distance maps from enemy base and my base
	DistanceMap			enemyBaseMap;
	DistanceMap			myBaseMap;

	// whether or not we have calculated 
	bool				calculatedEnemyDistance;
	bool				calculatedMyDistance;

	// the map stored at TilePosition resolution 
	// values are 0/1 for walkable or not walkable
	std::vector<bool>	map;

	// map that stores whether a unit is on this position
	std::vector<bool>	units;

	// the fringe vector which is used as a sort of 'open list'
	std::vector<int>	fringe;

	// the size of the map
	int					rows,
						cols;

	// constructor for MapTools
	MapTools();
	~MapTools() {}

	// return the index of the 1D array from (row,col)
	inline int getIndex(int row, int col);

	bool unexplored(DistanceMap & dmap, const int index) const;
	
	// resets the distance and fringe vectors, call before each search
	void reset();

	// reads in the map data from bwapi and stores it in our map format
	void setBWAPIMapData();
	
	// reset the fringe
	void resetFringe();
	
public:

	static MapTools &	Instance();

	BWAPI::TilePosition getNextExpansion();

	void update();
	void drawMyRegion();
	void computeConnectedRegions();

	// computes walk distance from Position P to all other points on the map
	void computeDistance(DistanceMap & dmap, const BWAPI::Position p);

	// does the dynamic programming search
	void search(DistanceMap & dmap, const int sR, const int sC);
	
	// computes connectivity for the map
	void fill(const int index, const int region);

	// get the ground distance between (from, to)
	int getGroundDistance(BWAPI::Position from, BWAPI::Position to);

	// get distance to various bases
	int	getEnemyBaseDistance(BWAPI::Position p);
	int	getMyBaseDistance(BWAPI::Position p);
	BWAPI::Position getEnemyBaseMoveTo(BWAPI::Position p);
	
	void parseMap();
};
