#include "ProductionManager.h"

using namespace BWAPI;

ProductionManager::ProductionManager()
{
	Broodwar->sendText("Inside constructor of ProductionManager");
}


ProductionManager::~ProductionManager()
{
}


void ProductionManager::onStart() {
}

void ProductionManager::onEnd(bool isWinner) {
}

void ProductionManager::onFrame() {
	// Iterate through all the units that we own
	Unitset myUnits = Broodwar->self()->getUnits();
	for ( Unitset::iterator u = myUnits.begin(); u != myUnits.end(); ++u )
	{
		// Ignore the unit if it no longer exists
		// Make sure to include this block when handling any Unit pointer!
		if ( !u->exists() )
			continue;

		// Ignore the unit if it has one of the following status ailments
		if ( u->isLockedDown() || u->isMaelstrommed() || u->isStasised() )
			continue;

		// Ignore the unit if it is in one of the following states
		if ( u->isLoaded() || !u->isPowered() || u->isStuck() )
			continue;

		// Ignore the unit if it is incomplete or busy constructing
		if ( !u->isCompleted() || u->isConstructing() )
			continue;


		// Finally make the unit do some stuff!


		// If the unit is a worker unit
		if ( u->getType().isWorker() )
		{
			// if our worker is idle
			if ( u->isIdle() )
			{
				// Order workers carrying a resource to return them to the center,
				// otherwise find a mineral patch to harvest.
				if ( u->isCarryingGas() || u->isCarryingMinerals() )
				{
					u->returnCargo();
				}
				else if ( !u->getPowerUp() )  // The worker cannot harvest anything if it
				{                             // is carrying a powerup such as a flag
					// Harvest from the nearest mineral patch or gas refinery
					if ( !u->gather( u->getClosestUnit( BWAPI::Filter::IsMineralField || BWAPI::Filter::IsRefinery )) )
					{
						// If the call fails, then print the last error message
						Broodwar << Broodwar->getLastError() << std::endl;
					}

				} // closure: has no powerup
			} // closure: if idle

		}
		else if ( u->getType().isResourceDepot() ) // A resource depot is a Command Center, Nexus, or Hatchery
		{

			// Order the depot to construct more workers! But only when it is idle.
			if ( u->isIdle() && !u->train(u->getType().getRace().getWorker()) )
			{
				// If that fails, draw the error at the location so that you can visibly see what went wrong!
				// However, drawing the error once will only appear for a single frame
				// so create an event that keeps it on the screen for some frames
				Position pos = u->getPosition();
				Error lastErr = Broodwar->getLastError();
				Broodwar->registerEvent([pos,lastErr](Game*){ Broodwar->drawTextMap(pos, "%c%s", Text::White, lastErr.c_str()); },   // action
					nullptr,    // condition
					Broodwar->getLatencyFrames());  // frames to run

				// Retrieve the supply provider type in the case that we have run out of supplies
				UnitType supplyProviderType = u->getType().getRace().getSupplyProvider();
				static int lastChecked = 0;

				// If we are supply blocked and haven't tried constructing more recently
				if (  lastErr == Errors::Insufficient_Supply &&
					lastChecked + 400 < Broodwar->getFrameCount() &&
					Broodwar->self()->incompleteUnitCount(supplyProviderType) == 0 )
				{
					lastChecked = Broodwar->getFrameCount();

					// Retrieve a unit that is capable of constructing the supply needed
					Unit supplyBuilder = u->getClosestUnit(  BWAPI::Filter::GetType == supplyProviderType.whatBuilds().first &&
						(BWAPI::Filter::IsIdle || BWAPI::Filter::IsGatheringMinerals) &&
						BWAPI::Filter::IsOwned);
					// If a unit was found
					if ( supplyBuilder )
					{
						if ( supplyProviderType.isBuilding() )
						{
							TilePosition targetBuildLocation = Broodwar->getBuildLocation(supplyProviderType, supplyBuilder->getTilePosition());
							if ( targetBuildLocation )
							{
								// Register an event that draws the target build location
								Broodwar->registerEvent([targetBuildLocation,supplyProviderType](Game*)
								{
									Broodwar->drawBoxMap( Position(targetBuildLocation),
										Position(targetBuildLocation + supplyProviderType.tileSize()),
										Colors::Blue);
								},
									nullptr,  // condition
									supplyProviderType.buildTime() + 100 );  // frames to run

								// Order the builder to construct the supply structure
								supplyBuilder->build( supplyProviderType, targetBuildLocation );
							}
						}
						else
						{
							// Train the supply provider (Overlord) if the provider is not a structure
							supplyBuilder->train( supplyProviderType );
						}
					} // closure: supplyBuilder is valid
				} // closure: insufficient supply
			} // closure: failed to train idle unit

		}

	} // closure: unit iterator
}

void ProductionManager::onSendText(std::string text) {
}

void ProductionManager::onReceiveText(BWAPI::Player player, std::string text) {
}

void ProductionManager::onPlayerLeft(BWAPI::Player player) {
}

void ProductionManager::onNukeDetect(BWAPI::Position target) {
}

void ProductionManager::onUnitDiscover(BWAPI::Unit unit) {
}

void ProductionManager::onUnitEvade(BWAPI::Unit unit) {
}

void ProductionManager::onUnitShow(BWAPI::Unit unit) {
}

void ProductionManager::onUnitHide(BWAPI::Unit unit) {
}

void ProductionManager::onUnitCreate(BWAPI::Unit unit) {
}

void ProductionManager::onUnitDestroy(BWAPI::Unit unit) {
}

void ProductionManager::onUnitMorph(BWAPI::Unit unit) {
}

void ProductionManager::onUnitRenegade(BWAPI::Unit unit) {
}

void ProductionManager::onSaveGame(std::string gameName) {
}

void ProductionManager::onUnitComplete(BWAPI::Unit unit) {
}
