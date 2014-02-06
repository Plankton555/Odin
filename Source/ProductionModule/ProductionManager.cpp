#include "ProductionManager.h"


ProductionManager::ProductionManager()
{
	BWAPI::Broodwar->sendText("Inside constructor of ProductionManager");
}


ProductionManager::~ProductionManager()
{
}


void ProductionManager::onStart() {
}

void ProductionManager::onEnd(bool isWinner) {
}

void ProductionManager::onFrame() {
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
