#include "StrategyManager.h"


StrategyManager::StrategyManager()
{
	BWAPI::Broodwar->sendText("Inside constructor of StrategyManager");
}


StrategyManager::~StrategyManager()
{
}


void StrategyManager::onStart() {
}

void StrategyManager::onEnd(bool isWinner) {
}

void StrategyManager::onFrame() {
}

void StrategyManager::onSendText(std::string text) {
}

void StrategyManager::onReceiveText(BWAPI::Player player, std::string text) {
}

void StrategyManager::onPlayerLeft(BWAPI::Player player) {
}

void StrategyManager::onNukeDetect(BWAPI::Position target) {
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