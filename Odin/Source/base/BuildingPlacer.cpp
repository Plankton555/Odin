#include "Common.h"
#include "BuildingPlacer.h"

BuildingPlacer::BuildingPlacer() : boxTop(100000), boxBottom(-1), boxLeft(100000), boxRight(-1)
{
	reserveMap = std::vector< std::vector<bool> >(BWAPI::Broodwar->mapWidth(), std::vector<bool>(BWAPI::Broodwar->mapHeight(), false));
	buildDistance = 0;
	computeResourceBox();
}

bool BuildingPlacer::isInResourceBox(int x, int y) const
{
	int posX(x*32);
	int posY(y*32);

	if (posX >= boxLeft && posX < boxRight && posY >= boxTop && posY < boxBottom)
	{
		return true;
	}

	return false;
}

void BuildingPlacer::computeResourceBox()
{
	BWAPI::Position start(BWAPI::Broodwar->self()->getStartLocation());
	std::vector<BWAPI::Unit *> unitsAroundNexus;

	BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->getAllUnits())
	{
		// if the units are less than 400 away add them if they are resources
		if (unit->getDistance(start) < 400 && unit->getType().isResourceContainer())
		{
			unitsAroundNexus.push_back(unit);
		}
	}

	BOOST_FOREACH(BWAPI::Unit * unit, unitsAroundNexus)
	{
		int x = unit->getPosition().x();
		int y = unit->getPosition().y();

		int left = x - unit->getType().dimensionLeft();
		int right = x + unit->getType().dimensionRight() + 1;
		int top = y - unit->getType().dimensionUp();
		int bottom = y + unit->getType().dimensionDown() + 1;

		boxTop = top < boxTop ? top : boxTop;
		boxBottom = bottom > boxBottom ? bottom : boxBottom;
		boxLeft = left < boxLeft ? left : boxLeft;
		boxRight = right > boxRight ? right : boxRight;
	}

	//BWAPI::Broodwar->printf("%d %d %d %d", boxTop, boxBottom, boxLeft, boxRight);
}

// makes final checks to see if a building can be built at a certain location
bool BuildingPlacer::canBuildHere(BWAPI::TilePosition position, const Building & b) const
{
	if (!b.type.isRefinery() && !InformationManager::Instance().tileContainsUnit(position))
	{
		return false;
	}

	//returns true if we can build this type of unit here. Takes into account reserved tiles.
	if (!BWAPI::Broodwar->canBuildHere(b.builderUnit, position, b.type))
	{
		return false;
	}

	// check the reserve map
	for(int x = position.x(); x < position.x() + b.type.tileWidth(); x++)
	{
		for(int y = position.y(); y < position.y() + b.type.tileHeight(); y++)
		{
			if (reserveMap[x][y])
			{
				return false;
			}
		}
	}

	// if it overlaps a base location return false
	if (tileOverlapsBaseLocation(position, b.type))
	{
		return false;
	}

	return true;
}

//returns true if we can build this type of unit here with the specified amount of space.
//space value is stored in this->buildDistance.
bool BuildingPlacer::canBuildHereWithSpace(BWAPI::TilePosition position, const Building & b, int buildDist, bool horizontalOnly) const
{
	//if we can't build here, we of course can't build here with space
	if (!this->canBuildHere(position, b))
	{
		return false;
	}

	// height and width of the building
	int width(b.type.tileWidth());
	int height(b.type.tileHeight());

	//make sure we leave space for add-ons. These types of units can have addons:
	if (b.type==BWAPI::UnitTypes::Terran_Command_Center ||
		b.type==BWAPI::UnitTypes::Terran_Factory || 
		b.type==BWAPI::UnitTypes::Terran_Starport ||
		b.type==BWAPI::UnitTypes::Terran_Science_Facility)
	{
		width += 2;
	}

	// define the rectangle of the building spot
	int startx = position.x() - buildDist;
	int starty = position.y() - buildDist;
	int endx   = position.x() + width + buildDist;
	int endy   = position.y() + height + buildDist;

	if (horizontalOnly)
	{
		starty += buildDist;
		endy -= buildDist;
	}

	// if this rectangle doesn't fit on the map we can't build here
	if (startx < 0 || starty < 0 || endx > BWAPI::Broodwar->mapWidth() || endx < position.x() + width || endy > BWAPI::Broodwar->mapHeight()) 
	{
		return false;
	}

	// if we can't build here, or space is reserved, or it's in the resource box, we can't build here
	for(int x = startx; x < endx; x++)
	{
		for(int y = starty; y < endy; y++)
		{
			if (!b.type.isRefinery())
			{
				if (!buildable(x, y) || reserveMap[x][y] || ((b.type != BWAPI::UnitTypes::Protoss_Photon_Cannon) && isInResourceBox(x,y)))
				{
					return false;
				}
			}
		}
	}

	// special cases for terran buildings that can land into addons?
	if (position.x() > 3 && b.type.isFlyingBuilding())
	{
		int startx2 = startx - 2;
		if (startx2 < 0) 
		{
			startx2 = 0;
		}

		for(int x = startx2; x < startx; x++)
		{
			for(int y = starty; y < endy; y++)
			{
				BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->getUnitsOnTile(x, y))
				{
					if (!unit->isLifted())
					{
						BWAPI::UnitType type(unit->getType());

						if (b.type==BWAPI::UnitTypes::Terran_Command_Center || b.type==BWAPI::UnitTypes::Terran_Factory || 
							b.type==BWAPI::UnitTypes::Terran_Starport || b.type==BWAPI::UnitTypes::Terran_Science_Facility)
						{
							return false;
						}
					}
				}
			}
		}
	}

	return true;
}

BWAPI::TilePosition BuildingPlacer::getBuildLocationNear(const Building & b, int buildDist, bool inRegionPriority, bool horizontalOnly) const
{
	//returns a valid build location near the specified tile position.
	//searches outward in a spiral.
	int x      = b.desiredPosition.x();
	int y      = b.desiredPosition.y();
	int length = 1;
	int j      = 0;
	bool first = true;
	int dx     = 0;
	int dy     = 1;

    SparCraft::Timer t;
    t.start();
    int iter = 0;

	while (length < BWAPI::Broodwar->mapWidth()) //We'll ride the spiral to the end
	{

		//if we can build here, return this tile position
		if (x >= 0 && x < BWAPI::Broodwar->mapWidth() && y >= 0 && y < BWAPI::Broodwar->mapHeight())
		{
            iter++;

            if (iter % 50 == 0)
            {
                if (t.getElapsedTimeInMilliSec() > 30)
                {
                    return BWAPI::TilePositions::None;
                }
            }

			// can we build this building at this location
			bool canBuild					= this->canBuildHereWithSpace(BWAPI::TilePosition(x, y), b, buildDist, horizontalOnly);

			// my starting region
			BWTA::Region * myRegion			= BWTA::getRegion(BWTA::getStartLocation(BWAPI::Broodwar->self())->getTilePosition()); 

			// the region the build tile is in
			BWTA::Region * tileRegion		= BWTA::getRegion(BWAPI::TilePosition(x,y));

			// is the proposed tile in our region?
			bool tileInRegion				= (tileRegion == myRegion);

			// if this location has priority to be built within our own region
			if (inRegionPriority)
			{
				// if the tile is in region and we can build it there
				if (tileInRegion && canBuild)
				{
					// return that position
					return BWAPI::TilePosition(x, y);
				}
			}
			// otherwise priority is not set for this building
			else
			{
				if (canBuild)
				{
					 return BWAPI::TilePosition(x, y);
				}
			}
		}

		//otherwise, move to another position
		x = x + dx;
		y = y + dy;

		//count how many steps we take in this direction
		j++;
		if (j == length) //if we've reached the end, its time to turn
		{
			//reset step counter
			j = 0;

			//Spiral out. Keep going.
			if (!first)
				length++; //increment step counter if needed

			//first=true for every other turn so we spiral out at the right rate
			first =! first;

			//turn counter clockwise 90 degrees:
			if (dx == 0)
			{
				dx = dy;
				dy = 0;
			}
			else
			{
				dy = -dx;
				dx = 0;
			}
		}
		//Spiral out. Keep going.
	}

	return  BWAPI::TilePositions::None;
}

bool BuildingPlacer::tileOverlapsBaseLocation(BWAPI::TilePosition tile, BWAPI::UnitType type) const
{
	// if it's a resource depot we don't care if it overlaps
	if (type.isResourceDepot())
	{
		return false;
	}

	// dimensions of the proposed location
	int tx1 = tile.x();
	int ty1 = tile.y();
	int tx2 = tx1 + type.tileWidth();
	int ty2 = ty1 + type.tileHeight();

	// for each base location
	BOOST_FOREACH (BWTA::BaseLocation * base, BWTA::getBaseLocations())
	{
		// dimensions of the base location
		int bx1 = base->getTilePosition().x();
		int by1 = base->getTilePosition().y();
		int bx2 = bx1 + BWAPI::Broodwar->self()->getRace().getCenter().tileWidth();
		int by2 = by1 + BWAPI::Broodwar->self()->getRace().getCenter().tileHeight();

		// conditions for non-overlap are easy
		bool noOverlap = (tx2 < bx1) || (tx1 > bx2) || (ty2 < by1) || (ty1 > by2);

		// if the reverse is true, return true
		if (!noOverlap)
		{
			return true;
		}
	}

	// otherwise there is no overlap
	return false;
}

bool BuildingPlacer::buildable(int x, int y) const
{
	//returns true if this tile is currently buildable, takes into account units on tile
	if (!BWAPI::Broodwar->isBuildable(x,y)) 
	{
		return false;
	}

	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->getUnitsOnTile(x, y))
	{
		if (unit->getType().isBuilding() && !unit->isLifted()) 
		{
			return false;
		}
	}

	for (int i=x-1; i <= x+1; ++i)
	{
		for (int j=y-1; j <= y+1; ++j)
		{
			BWAPI::TilePosition tile(i,j);

			if (!tile.isValid())
			{
				continue;
			}

			BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->getUnitsOnTile(i, j))
			{
				if (unit->getType() == BWAPI::UnitTypes::Protoss_Gateway) 
				{
					return false;
				}
			}
		}	
	}

	return true;
}

void BuildingPlacer::reserveTiles(BWAPI::TilePosition position, int width, int height)
{
	int rwidth = reserveMap.size();
	int rheight = reserveMap[0].size();
	for(int x = position.x(); x < position.x() + width && x < rwidth; x++) 
	{
		for(int y = position.y(); y < position.y() + height && y < rheight; y++) 
		{
			reserveMap[x][y] = true;
		}
	}
}

void BuildingPlacer::drawReservedTiles()
{
	int rwidth = reserveMap.size();
	int rheight = reserveMap[0].size();

	for(int x = 0; x < rwidth; ++x) 
	{
		for(int y = 0; y < rheight; ++y) 
		{
			if (reserveMap[x][y] || isInResourceBox(x,y))
			{
				int x1 = x*32 + 8;
				int y1 = y*32 + 8;
				int x2 = (x+1)*32 - 8;
				int y2 = (y+1)*32 - 8;

				if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawBoxMap(x1, y1, x2, y2, BWAPI::Colors::Yellow, false);
			}
		}
	}
}

void BuildingPlacer::freeTiles(BWAPI::TilePosition position, int width, int height)
{
	int rwidth = reserveMap.size();
	int rheight = reserveMap[0].size();

	for(int x = position.x(); x < position.x() + width && x < rwidth; x++) 
	{
		for(int y = position.y(); y < position.y() + height && y < rheight; y++) 
		{
			reserveMap[x][y] = false;
		}
	}
}

void BuildingPlacer::setBuildDistance(int distance)
{
	this->buildDistance=distance;
}

int BuildingPlacer::getBuildDistance() const
{
	return this->buildDistance;
}

BWAPI::TilePosition BuildingPlacer::getRefineryPosition()
{
	// for each of our units
	BOOST_FOREACH (BWAPI::Unit * depot, BWAPI::Broodwar->self()->getUnits())
	{
		// if it's a resource depot
		if (depot->getType().isResourceDepot())
		{
			// for all units around it
			BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->getAllUnits())
			{
				// if it's a geyser around it
				if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser && unit->getDistance(depot) < 300)
				{
					return unit->getTilePosition();
				}
			}
		}
	}

	return BWAPI::TilePositions::None;
}

bool BuildingPlacer::isReserved(int x, int y) const
{
	int rwidth = reserveMap.size();
	int rheight = reserveMap[0].size();
	if (x < 0 || y < 0 || x >= rwidth || y >= rheight) 
	{
		return false;
	}

	return reserveMap[x][y];
}

BuildingPlacer & BuildingPlacer::Instance() {

	static BuildingPlacer instance;
	return instance;
}

