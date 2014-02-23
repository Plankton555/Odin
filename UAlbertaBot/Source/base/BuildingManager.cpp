#include "Common.h"
#include "BuildingManager.h"


BuildingManager::BuildingManager() 
	: debugMode(false)
	, reservedMinerals(0)
	, reservedGas(0)
	, buildingSpace(1)
	, totalBuildTasks(0)
{

}

// gets called every frame from GameCommander
void BuildingManager::update() 
{
	// Step through building logic, issue orders, manage data as necessary
	//drawBuildingInformation(340, 50);
	
	// check to see if assigned workers have died en route or while constructing
	validateWorkersAndBuildings();	

	// assign workers to the unassigned buildings and label them 'planned'
	assignWorkersToUnassignedBuildings();

	// for each planned building, if the worker isn't constructing, send the command
	constructAssignedBuildings();

	// check to see if any buildings have started construction and update data structures
	checkForStartedConstruction();

	// if we are terran and a building is under construction without a worker, assign a new one
	checkForDeadTerranBuilders();

	// check to see if any buildings have completed and update data structures
	checkForCompletedBuildings();

	// draw some debug information
	//BuildingPlacer::Instance().drawReservedTiles();

}

// checks all relevant data structures to see if the given type is being built
bool BuildingManager::isBeingBuilt(BWAPI::UnitType type)
{
	// check unassigned buildings
	return buildingData.isBeingBuilt(type);
}

// STEP 1: DO BOOK KEEPING ON WORKERS WHICH MAY HAVE DIED
void BuildingManager::validateWorkersAndBuildings() 
{
	// TODO: if a terran worker dies while constructing and its building
	//       is under construction, place unit back into buildingsNeedingBuilders

	buildingData.begin(ConstructionData::UnderConstruction);
	while (buildingData.hasNextBuilding(ConstructionData::UnderConstruction))
	{
		Building & b = buildingData.getNextBuilding(ConstructionData::UnderConstruction);

		if (b.buildingUnit == NULL || !b.buildingUnit->getType().isBuilding() || b.buildingUnit->getHitPoints() <= 0)
		{
			buildingData.removeCurrentBuilding(ConstructionData::UnderConstruction);
			break;
		}
	}
}

// STEP 2: ASSIGN WORKERS TO BUILDINGS WITHOUT THEM
void BuildingManager::assignWorkersToUnassignedBuildings() 
{
	// for each building that doesn't have a builder, assign one
	buildingData.begin(ConstructionData::Unassigned);
	while (buildingData.hasNextBuilding(ConstructionData::Unassigned)) 
	{
		Building & b = buildingData.getNextBuilding(ConstructionData::Unassigned);
		if (debugMode) { BWAPI::Broodwar->printf("Assigning Worker To: %s", b.type.getName().c_str()); }

		// remove all refineries after 3, i don't know why this happens
		if (b.type.isRefinery() && (BWAPI::Broodwar->self()->allUnitCount(b.type) >= 3))
		{
			buildingData.removeCurrentBuilding(ConstructionData::Unassigned);
			break;
		}

		// get the building location
		BWAPI::TilePosition testLocation = getBuildingLocation(b);

		// if we can't find a valid build location, we can't assign this building
		if (!testLocation.isValid())
		{
			continue;
		}

		// set the final position of the building as this location
		b.finalPosition = testLocation;

		// grab a worker unit from WorkerManager which is closest to this final position
		BWAPI::Unit * workerToAssign = WorkerManager::Instance().getBuilder(b);

		// if the worker exists
		if (workerToAssign) {
			//BWAPI::Broodwar->printf("VALID WORKER BEING ASSIGNED: %d", workerToAssign->getID());

			// TODO: special case of terran building whose worker died mid construction
			//       send the right click command to the buildingUnit to resume construction
			//		 skip the buildingsAssigned step and push it back into buildingsUnderConstruction
			
			// set the worker we have assigned
			b.builderUnit = workerToAssign;

			// re-search for a building location with the builder unit ignored for space
			testLocation = getBuildingLocation(b);

			// hopefully this will not blow up
			if (!testLocation.isValid())
			{
				continue;
			}

			// set the final position of the building
			b.finalPosition = testLocation;

			// reserve this space
			BuildingPlacer::Instance().reserveTiles(b.finalPosition, b.type.tileWidth(), b.type.tileHeight());

			// this building has now been assigned
			buildingData.addBuilding(ConstructionData::Assigned, b);

			// remove this Building
			buildingData.removeCurrentBuilding(ConstructionData::Unassigned);
		}
	}
}

BWAPI::TilePosition BuildingManager::getBuildingLocation(const Building & b)
{
	BWAPI::TilePosition testLocation = BWAPI::TilePositions::None;

	int numPylons = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Pylon);

	if (b.type.isRefinery())
	{
		return BuildingPlacer::Instance().getRefineryPosition();
	}

	// special case for early pylons
	if (b.type == BWAPI::UnitTypes::Protoss_Pylon && (numPylons < 3))
	{
		BWAPI::TilePosition posInRegion =    BuildingPlacer::Instance().getBuildLocationNear(b, 4, true);
		BWAPI::TilePosition posNotInRegion = BuildingPlacer::Instance().getBuildLocationNear(b, 4, false);

		testLocation = (posInRegion != BWAPI::TilePositions::None) ? posInRegion : posNotInRegion;
	}
	// every other type of building
	else
	{
		// if it is a protoss building and we have no pylons, quick check
		if (b.type.requiresPsi() && (numPylons == 0))
		{
			// nothing
		}
		// if the unit is a resource depot
		else if (b.type.isResourceDepot())
		{
			// get the location 
			BWAPI::TilePosition tile = MapTools::Instance().getNextExpansion();

			return tile;
		}
		// any other building
		else
		{
			// set the building padding specifically
			int distance = b.type == BWAPI::UnitTypes::Protoss_Photon_Cannon ? 0 : 1;

			// whether or not we want the distance to be horizontal only
            bool horizontalOnly = b.type == BWAPI::UnitTypes::Protoss_Citadel_of_Adun ? true : false;

			// get a position within our region
			BWAPI::TilePosition posInRegion =    BuildingPlacer::Instance().getBuildLocationNear(b, distance, true,  horizontalOnly);

			// get a region anywhere
			BWAPI::TilePosition posNotInRegion = BuildingPlacer::Instance().getBuildLocationNear(b, distance, false, horizontalOnly);

			// set the location with priority on positions in our own region
			testLocation = (posInRegion != BWAPI::TilePositions::None) ? posInRegion : posNotInRegion;
		}
	}

	// send back the location
	return testLocation;
}

// STEP 3: ISSUE CONSTRUCTION ORDERS TO ASSIGN BUILDINGS AS NEEDED
void BuildingManager::constructAssignedBuildings() 
{
	// for each of the buildings which have been assigned a worker
	buildingData.begin(ConstructionData::Assigned);
	while(buildingData.hasNextBuilding(ConstructionData::Assigned)) 
	{
		// get a handy reference to the worker
		Building & b = buildingData.getNextBuilding(ConstructionData::Assigned);

		// if that worker is not currently constructing
		if (!b.builderUnit->isConstructing()) 
		{
			// if we haven't explored the build position, go there
			if (!isBuildingPositionExplored(b))
			{
				b.builderUnit->move(BWAPI::Position(b.finalPosition));
				//BWAPI::Broodwar->printf("Can't see build position, walking there");
			}
			// if this is not the first time we've sent this guy to build this
			// it must be the case that something was in the way of building
			else if (b.buildCommandGiven) 
			{
				//BWAPI::Broodwar->printf("A builder got stuck");
				// tell worker manager the unit we had is not needed now, since we might not be able
				// to get a valid location soon enough
				WorkerManager::Instance().finishedWithWorker(b.builderUnit);

				// free the previous location in reserved
				BuildingPlacer::Instance().freeTiles(b.finalPosition, b.type.tileWidth(), b.type.tileHeight());

				// nullify its current builder unit
				b.builderUnit = NULL;

				// reset the build command given flag
				b.buildCommandGiven = false;

				// add the building back to be assigned
				buildingData.addBuilding(ConstructionData::Unassigned, b);

				// remove the building from Assigned
				buildingData.removeCurrentBuilding(ConstructionData::Assigned);
			}
			else
			{
				if (debugMode) { BWAPI::Broodwar->printf("Issuing Build Command To %s", b.type.getName().c_str()); }

				// issue the build order!
				b.builderUnit->build(b.finalPosition, b.type);

				// set the flag to true
				b.buildCommandGiven = true;
			}
		}
	}
}

// STEP 4: UPDATE DATA STRUCTURES FOR BUILDINGS STARTING CONSTRUCTION
void BuildingManager::checkForStartedConstruction() 
{
	// for each building unit which is being constructed
	BOOST_FOREACH (BWAPI::Unit * buildingStarted, BWAPI::Broodwar->self()->getUnits()) {

		// filter out units which aren't buildings under construction
		if (!(buildingStarted->getType().isBuilding() && buildingStarted->isBeingConstructed()))
		{
			continue;
		}

		// for each Building which is currently assigned, check it
		buildingData.begin(ConstructionData::Assigned);
		while (buildingData.hasNextBuilding(ConstructionData::Assigned)) 
		{
			Building & b = buildingData.getNextBuilding(ConstructionData::Assigned);

			// check if the positions match
			if (b.finalPosition == buildingStarted->getTilePosition()) {
				// the resources should now be spent, so unreserve them
				reservedMinerals -= buildingStarted->getType().mineralPrice();
				reservedGas      -= buildingStarted->getType().gasPrice();

				if (debugMode) { BWAPI::Broodwar->printf("Construction Started: %s", b.type.getName().c_str()); }

				// flag it as started and set the buildingUnit
				b.underConstruction = true;
				b.buildingUnit = buildingStarted;

				// if we are zerg, the buildingUnit now becomes NULL since it's destroyed
				if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg) {
					b.builderUnit = NULL;

				// if we are protoss, give the worker back to worker manager
				} else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Protoss) {
					WorkerManager::Instance().finishedWithWorker(b.builderUnit);
					b.builderUnit = NULL;
				}

				// put it in the under construction vector
				buildingData.addBuilding(ConstructionData::UnderConstruction, b);

				// remove it from buildingsAssigned
				buildingData.removeCurrentBuilding(ConstructionData::Assigned);

				// free this space
				BuildingPlacer::Instance().freeTiles(b.finalPosition, b.type.tileWidth(), b.type.tileHeight());

				// only one building will match
				break;
			}
		}
	}
}

// STEP 5: IF WE ARE TERRAN, THIS MATTERS, SO: LOL
void BuildingManager::checkForDeadTerranBuilders() {}

// STEP 6: CHECK FOR COMPLETED BUILDINGS
void BuildingManager::checkForCompletedBuildings() {

	// for each of our buildings under construction
	buildingData.begin(ConstructionData::UnderConstruction);
	while (buildingData.hasNextBuilding(ConstructionData::UnderConstruction)) {

		Building & b = buildingData.getNextBuilding(ConstructionData::UnderConstruction);

		// if the unit has completed
		if (b.buildingUnit->isCompleted()) 
		{
			if (debugMode) { BWAPI::Broodwar->printf("Construction Completed: %s", b.type.getName().c_str()); }

			// if we are terran, give the worker back to worker manager
			if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Terran)
			{
				WorkerManager::Instance().finishedWithWorker(b.builderUnit);
			}

			// remove this unit from the under construction vector
			buildingData.removeCurrentBuilding(ConstructionData::UnderConstruction);
		}
	}
}

void BuildingManager::printBuildingNumbers()
{
	BWAPI::Broodwar->printf("BUILDINGS: %d %d %d", 
		buildingData.getNumBuildings(ConstructionData::Unassigned), 
		buildingData.getNumBuildings(ConstructionData::Assigned), 
		buildingData.getNumBuildings(ConstructionData::UnderConstruction));
}

// COMPLETED
bool BuildingManager::isEvolvedBuilding(BWAPI::UnitType type) {

	if (type == BWAPI::UnitTypes::Zerg_Sunken_Colony ||
		type == BWAPI::UnitTypes::Zerg_Spore_Colony ||
		type == BWAPI::UnitTypes::Zerg_Lair ||
		type == BWAPI::UnitTypes::Zerg_Hive ||
		type == BWAPI::UnitTypes::Zerg_Greater_Spire)
	{
		return true;
	}

	return false;
}

// add a new building to be constructed
void BuildingManager::addBuildingTask(BWAPI::UnitType type, BWAPI::TilePosition desiredLocation) {

	if (debugMode) { BWAPI::Broodwar->printf("Issuing addBuildingTask: %s", type.getName().c_str()); }

	totalBuildTasks++;

	// reserve the resources for this building
	reservedMinerals += type.mineralPrice();
	reservedGas	     += type.gasPrice();

	// set it up to receive a worker
	buildingData.addBuilding(ConstructionData::Unassigned, Building(type, desiredLocation));
}

bool BuildingManager::isBuildingPositionExplored(const Building & b) const
{
	BWAPI::TilePosition tile = b.finalPosition;

	// for each tile where the building will be built
	for (int x=0; x<b.type.tileWidth(); ++x)
	{
		for (int y=0; y<b.type.tileHeight(); ++y)
		{
			if (!BWAPI::Broodwar->isExplored(tile.x() + x, tile.y() + y))
			{
				return false;
			}
		}
	}

	return true;
}


char BuildingManager::getBuildingWorkerCode(const Building & b) const
{
	if (b.builderUnit == NULL)	return 'X';
	else						return 'W';
}

int BuildingManager::getReservedMinerals() {
	return reservedMinerals;
}

int BuildingManager::getReservedGas() {
	return reservedGas;
}

void BuildingManager::drawBuildingInformation(int x, int y) {

	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->self()->getUnits())
	{
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextMap(unit->getPosition().x(), unit->getPosition().y()+5, "\x07%d", unit->getID()); 
	}

	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y, "\x04 Building Information:");
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y+20, "\x04 Name");
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x+150, y+20, "\x04 State");

	int yspace = 0;

	buildingData.begin(ConstructionData::Unassigned);
	while (buildingData.hasNextBuilding(ConstructionData::Unassigned)) {

		Building & b = buildingData.getNextBuilding(ConstructionData::Unassigned);

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y+40+((yspace)*10), "\x03 %s", b.type.getName().c_str());
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x+150, y+40+((yspace++)*10), "\x03 Need %c", getBuildingWorkerCode(b));
	}

	buildingData.begin(ConstructionData::Assigned);
	while (buildingData.hasNextBuilding(ConstructionData::Assigned)) {

		Building & b = buildingData.getNextBuilding(ConstructionData::Assigned);

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y+40+((yspace)*10), "\x03 %s %d", b.type.getName().c_str(), b.builderUnit->getID());
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x+150, y+40+((yspace++)*10), "\x03 A %c (%d,%d)", getBuildingWorkerCode(b), b.finalPosition.x(), b.finalPosition.y());

		int x1 = b.finalPosition.x()*32;
		int y1 = b.finalPosition.y()*32;
		int x2 = (b.finalPosition.x() + b.type.tileWidth())*32;
		int y2 = (b.finalPosition.y() + b.type.tileHeight())*32;

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(b.builderUnit->getPosition().x(), b.builderUnit->getPosition().y(), (x1+x2)/2, (y1+y2)/2, BWAPI::Colors::Orange);
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawBoxMap(x1, y1, x2, y2, BWAPI::Colors::Red, false);
	}

	buildingData.begin(ConstructionData::UnderConstruction);
	while (buildingData.hasNextBuilding(ConstructionData::UnderConstruction)) {

		Building & b = buildingData.getNextBuilding(ConstructionData::UnderConstruction);

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y+40+((yspace)*10), "\x03 %s %d", b.type.getName().c_str(), b.buildingUnit->getID());
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x+150, y+40+((yspace++)*10), "\x03 Const %c", getBuildingWorkerCode(b));
	}
}

BuildingManager & BuildingManager::Instance() 
{
	static BuildingManager instance;
	return instance;
}