#include "StrategyManager.h"

using namespace BWAPI;

StrategyManager::StrategyManager()
{
	Broodwar->sendText("Inside constructor of StrategyManager");

	prodManager = new ProductionManager();
	combManager = new CombatManager();
}


StrategyManager::~StrategyManager()
{
}


void StrategyManager::onStart() {
}

void StrategyManager::onEnd(bool isWinner) {
}

void StrategyManager::onFrame() {
	prodManager->onFrame();
}

void StrategyManager::onSendText(std::string text) {
}

void StrategyManager::onReceiveText(BWAPI::Player player, std::string text) {
}

void StrategyManager::onPlayerLeft(BWAPI::Player player) {
}

void StrategyManager::onNukeDetect(BWAPI::Position target) {
	
	// Check if the target is a valid position
	if ( target )
	{
		// if so, print the location of the nuclear strike target
		Broodwar << "Nuclear Launch Detected at " << target << std::endl;
	}
	else 
	{
		// Otherwise, ask other players where the nuke is!
		Broodwar->sendText("Where's the nuke?");
	}
	// You can also retrieve all the nuclear missile targets using Broodwar->getNukeDots()!
}

void StrategyManager::onUnitDiscover(BWAPI::Unit unit) {
}

void StrategyManager::onUnitEvade(BWAPI::Unit unit) {
}

void StrategyManager::onUnitShow(BWAPI::Unit unit) {
}

void StrategyManager::onUnitHide(BWAPI::Unit unit) {
}

void StrategyManager::onUnitCreate(BWAPI::Unit unit) {
}

void StrategyManager::onUnitDestroy(BWAPI::Unit unit) {
}

void StrategyManager::onUnitMorph(BWAPI::Unit unit) {
}

void StrategyManager::onUnitRenegade(BWAPI::Unit unit) {
}

void StrategyManager::onSaveGame(std::string gameName) {
}

void StrategyManager::onUnitComplete(BWAPI::Unit unit) {
}