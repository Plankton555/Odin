#pragma once

#include <Common.h>
#include "WorkerManager.h"
#include "BuildingPlacer.h"
#include "BuildingData.h"
#include "InformationManager.h"
#include "MapTools.h"

class BuildingManager {

	BuildingManager();

	ConstructionData			buildingData;

	bool						debugMode;
	int							totalBuildTasks;

	int							reservedMinerals;				// minerals reserved for planned buildings
	int							reservedGas;					// gas reserved for planned buildings
	int							buildingSpace;					// how much space we want between buildings

	std::vector<BWAPI::Unit *>	builders;						// workers which have been assigned to buildings
	std::vector<Building>		buildingsNeedingBuilders;		// buildings which do not yet have builders assigned
	std::vector<Building>		buildingsAssigned;				// buildings which have workers but not yet under construction
	std::vector<Building>		buildingsUnderConstruction;		// buildings which are under construction
	std::vector<BWAPI::Unit *>	buildingUnitsConstructing;		// units which have been recently detected as started construction

	// functions
	bool						isEvolvedBuilding(BWAPI::UnitType type);
	bool						isBuildingPositionExplored(const Building & b) const;

	// the update() functions
	void						validateWorkersAndBuildings();		// STEP 1
	void						assignWorkersToUnassignedBuildings();	// STEP 2
	void						constructAssignedBuildings();			// STEP 3
	void						checkForStartedConstruction();			// STEP 4
	void						checkForDeadTerranBuilders();			// STEP 5
	void						checkForCompletedBuildings();			// STEP 6

	// functions for performing tedious vector tasks
	void						removeBuildingFromVector(BWAPI::Unit * buildingUnit, std::vector<Building> & vec);
	void						removeBuildingFromVector(Building & b, std::vector<Building> & vec);

	char						getBuildingWorkerCode(const Building & b) const;

public:

	void						update();
	void						onUnitMorph(BWAPI::Unit * unit);
	void						onUnitDestroy(BWAPI::Unit * unit);
	void						addBuildingTask(BWAPI::UnitType type, BWAPI::TilePosition desiredLocation);
	BWAPI::TilePosition			getBuildingLocation(const Building & b);

	int							getReservedMinerals();
	int							getReservedGas();

	static BuildingManager &	Instance();

	void						printBuildingNumbers();

	bool						isBeingBuilt(BWAPI::UnitType type);
	
	void						drawBuildingInformation(int x, int y);
};
