#include "MapTools.h"

MapTools & MapTools::Instance() 
{
	static MapTools instance;
	return instance;
}

// constructor for MapTools
MapTools::MapTools() 
	: rows(BWAPI::Broodwar->mapHeight())
	, cols(BWAPI::Broodwar->mapWidth())
	, calculatedEnemyDistance(false)
	, calculatedMyDistance(false)			
{
	map			= std::vector<bool>(rows*cols, false);
	units		= std::vector<bool>(rows*cols, false);
	fringe		= std::vector<int> (rows*cols, 0);

	setBWAPIMapData();
}

// return the index of the 1D array from (row,col)
inline int MapTools::getIndex(int row, int col)
{
	return row * cols + col;
}

bool MapTools::unexplored(DistanceMap & dmap, const int index) const
{
	return (index != -1) && dmap[index] == -1 && map[index];
}

// resets the distance and fringe vectors, call before each search
void MapTools::reset()
{
	std::fill(fringe.begin(), fringe.end(), 0);
}

// reads in the map data from bwapi and stores it in our map format
void MapTools::setBWAPIMapData()
{
	// for each row and column
	for (int r(0); r < rows; ++r)
	{
		for (int c(0); c < cols; ++c)
		{
			bool clear = false;

			// check each walk tile within this TilePosition
			for (int i=0; i<4; ++i)
			{
				for (int j=0; j<4; ++j)
				{
					if (BWAPI::Broodwar->isWalkable(c*4 + i, r*4 + j))
					{
						clear = true;
						break;
					}

					if (clear)
					{
						break;
					}
				}
			}
			
			// set the map as binary clear or not
			map[getIndex(r,c)] = clear;
		}
	}
}

void MapTools::resetFringe()
{
	std::fill(fringe.begin(), fringe.end(), 0);
}


void MapTools::update()
{
	BWAPI::TilePosition nextExp(getNextExpansion());
	
	if (nextExp != BWAPI::TilePositions::None)
	{
		BWAPI::Position exp = BWAPI::Position(getNextExpansion());
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawBoxMap(exp.x(), exp.y(), exp.x()+4*32, exp.y()+3*32, BWAPI::Colors::Green, true);
	}

	// draws distance map to screen
	/*for (int x=0; x<BWAPI::Broodwar->mapWidth(); ++x)
	{
		for (int y=0; y<BWAPI::Broodwar->mapHeight(); ++y)
		{
			BWAPI::Position p(x*32, y*32);

			if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextMap(p.x(), p.y(), "%d", getMyBaseDistance(p));
		}
	}*/

	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->getAllUnits())
	{
		BWAPI::Color c(BWAPI::Colors::Yellow);

		if (unit->getPlayer() != BWAPI::Broodwar->self())
		{
			if (unit->getPlayer() == BWAPI::Broodwar->enemy())
			{
				c = BWAPI::Colors::Green;
			}

			if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawCircleMap(unit->getPosition().x(), unit->getPosition().y(), 3, c);
			//if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextMap(unit->getPosition().x(), unit->getPosition().y(), "%s", unit->getType().getName().c_str());
		}
	}

	//drawMyRegion();
}

void MapTools::drawMyRegion()
{
	BWTA::Region * myStartRegion = BWTA::getStartLocation(BWAPI::Broodwar->self())->getRegion();
	//BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());
	//BWTA::Region * enemyRegion = enemyBaseLocation ? enemyBaseLocation->getRegion() : NULL;
	BWTA::Region * startRegion = myStartRegion; //enemyRegion ? enemyRegion : myStartRegion;

	for (int i=0; i<BWAPI::Broodwar->mapWidth(); ++i)
	{
		for (int j=0; j<BWAPI::Broodwar->mapHeight(); ++j)
		{
			BWAPI::TilePosition tile(i,j);

			if (BWTA::getRegion(tile) == startRegion)
			{
				BWAPI::Color c = BWAPI::Colors::Yellow;

				if (!BWAPI::Broodwar->isBuildable(tile))
				{
					c = BWAPI::Colors::Red;
				}

				if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawCircleMap(i*32+16, j*32+16, 5, c);
			}
		}
	}
}

int MapTools::getGroundDistance(BWAPI::Position origin, BWAPI::Position destination)
{
	// if we haven't yet computed the distance map to the destination
	if (allMaps.find(destination) == allMaps.end())
	{
		//BWAPI::Broodwar->printf("Computing DistanceMap for new destination");

		// add the map and compute it
		allMaps[destination] = DistanceMap();
		computeDistance(allMaps[destination], destination);
	}

	// get the distance from the map
	return allMaps[destination][origin];
}

// computes walk distance from Position P to all other points on the map
void MapTools::computeDistance(DistanceMap & dmap, const BWAPI::Position p)
{
	search(dmap, p.y() / 32, p.x() / 32);
}

// does the dynamic programming search
void MapTools::search(DistanceMap & dmap, const int sR, const int sC)
{
	// reset the internal variables
	resetFringe();

	// set the starting position for this search
	dmap.setStartPosition(sR,sC);
	
	// set the distance of the start cell to zero
	dmap[getIndex(sR, sC)] = 0;
	
	// set the fringe variables accordingly
	int fringeSize(1);
	int fringeIndex(0);
	fringe[0] = getIndex(sR,sC);
	
	// temporary variables used in search loop
	int currentIndex, nextIndex;
	int newDist;
	
	// the size of the map
	int size = rows*cols;
	
	// while we still have things left to expand
	while (fringeIndex < fringeSize)
	{
		// grab the current index to expand from the fringe
		currentIndex = fringe[fringeIndex++];
		newDist = dmap[currentIndex] + 1;
		
		// search up
		nextIndex = (currentIndex > cols) ? (currentIndex - cols) : -1;
		if (unexplored(dmap, nextIndex))
		{
			// set the distance based on distance to current cell
			dmap.setDistance(nextIndex, newDist);
			dmap.setMoveTo(nextIndex, 'D');
				
			// put it in the fringe
			fringe[fringeSize++] = nextIndex;				
		}
		
		// search down
		nextIndex = (currentIndex + cols < size) ? (currentIndex + cols) : -1;
		if (unexplored(dmap, nextIndex))
		{
			// set the distance based on distance to current cell
			dmap.setDistance(nextIndex, newDist);
			dmap.setMoveTo(nextIndex, 'U');

			// put it in the fringe
			fringe[fringeSize++] = nextIndex;
		}
		
		// search left
		nextIndex = (currentIndex % cols > 0) ? (currentIndex - 1) : -1;
		if (unexplored(dmap, nextIndex))
		{
			// set the distance based on distance to current cell
			dmap.setDistance(nextIndex, newDist);
			dmap.setMoveTo(nextIndex, 'R');

			// put it in the fringe
			fringe[fringeSize++] = nextIndex;
		}
		
		// search right
		nextIndex = (currentIndex % cols < cols - 1) ? (currentIndex + 1) : -1;
		if (unexplored(dmap, nextIndex))
		{
			// set the distance based on distance to current cell
			dmap.setDistance(nextIndex, newDist);
			dmap.setMoveTo(nextIndex, 'L');

			// put it in the fringe
			fringe[fringeSize++] = nextIndex;
		}
	}
}


int MapTools::getEnemyBaseDistance(BWAPI::Position p)
{
	BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());
	assert(enemyBaseLocation != NULL);

	if (!calculatedEnemyDistance)
	{
		BWAPI::Position enemyBasePosition = enemyBaseLocation->getPosition();
		MapTools::Instance().computeDistance(enemyBaseMap, enemyBasePosition);
		calculatedEnemyDistance = true;
	}

	return enemyBaseMap[p];
}

BWAPI::Position MapTools::getEnemyBaseMoveTo(BWAPI::Position p)
{
	BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());
	assert(enemyBaseLocation != NULL);

	if (!calculatedEnemyDistance)
	{
		BWAPI::Position enemyBasePosition = enemyBaseLocation->getPosition();
		MapTools::Instance().computeDistance(enemyBaseMap, enemyBasePosition);
		calculatedEnemyDistance = true;
	}

	return enemyBaseMap.getMoveTo(p,1);
}

int MapTools::getMyBaseDistance(BWAPI::Position p)
{
	if (!calculatedMyDistance)
	{
		BWAPI::Position myBasePosition(BWAPI::Broodwar->self()->getStartLocation());
		MapTools::Instance().computeDistance(myBaseMap, myBasePosition);
		calculatedMyDistance = true;
	}

	return myBaseMap[p];
}

BWAPI::TilePosition MapTools::getNextExpansion()
{
	BWTA::BaseLocation * closestBase = NULL;
	double minDistance = 100000;

	BWAPI::TilePosition homeTile = BWAPI::Broodwar->self()->getStartLocation();

	// for each base location
	BOOST_FOREACH(BWTA::BaseLocation * base, BWTA::getBaseLocations())
	{
		// if the base has gas
		if(!base->isMineralOnly() && !(base == BWTA::getStartLocation(BWAPI::Broodwar->self())))
		{
			// get the tile position of the base
			BWAPI::TilePosition tile = base->getTilePosition();

			// the rectangle for this base location
			int x1 = tile.x() * 32;
			int y1 = tile.y() * 32;
			int x2 = x1 + BWAPI::UnitTypes::Protoss_Nexus.tileWidth() * 32;
			int y2 = y1 + BWAPI::UnitTypes::Protoss_Nexus.tileHeight() * 32;

			bool buildingInTheWay = false;

			// for each unit in the rectangle where we want to build it
			BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->getUnitsInRectangle(x1, y1, x2, y2))
			{
				// if the unit is a building, we can't build here
				if (unit->getType().isBuilding())
				{
					buildingInTheWay = true;
					break;
				}
			}

			if (buildingInTheWay)
			{
				continue;
			}

			// the base's distance from our main nexus
			double distanceFromHome = MapTools::Instance().getMyBaseDistance(BWAPI::Position(tile));//homeTile.getDistance(tile);

			// if it is not connected, continue
			if (!BWTA::isConnected(homeTile, tile) || distanceFromHome < 0)
			{
				continue;
			}

			if(!closestBase || distanceFromHome < minDistance)
			{
				closestBase = base;
				minDistance = distanceFromHome;
			}
		}

	}

	if (closestBase)
	{
		return closestBase->getTilePosition();
	}
	else
	{
		return BWAPI::TilePositions::None;
	}
}


void MapTools::parseMap()
{

	BWAPI::Broodwar->printf("Parsing Map Information");
	std::ofstream mapFile;
	std::string file = "c:\\scmaps\\" + BWAPI::Broodwar->mapName() + ".txt";
	mapFile.open(file.c_str());

	mapFile << BWAPI::Broodwar->mapWidth()*4 << "\n";
	mapFile << BWAPI::Broodwar->mapHeight()*4 << "\n";

	for (int j=0; j<BWAPI::Broodwar->mapHeight()*4; j++) {
		for (int i=0; i<BWAPI::Broodwar->mapWidth()*4; i++) {

			if (BWAPI::Broodwar->isWalkable(i,j)) {
				mapFile << "0";
			} else {
				mapFile << "1";
			}
		}

		mapFile << "\n";
	}
	
	BWAPI::Broodwar->printf(file.c_str());

	mapFile.close();
}
