#include "CombatManager.h"

using namespace BWAPI;

CombatManager::CombatManager()
{
	Broodwar->sendText("Inside constructor of CombatManager");
}


CombatManager::~CombatManager()
{
}


void CombatManager::onStart() {
}

void CombatManager::onEnd(bool isWinner) {
}

void CombatManager::onFrame() {
}

void CombatManager::onSendText(std::string text) {
}

void CombatManager::onReceiveText(BWAPI::Player player, std::string text) {
}

void CombatManager::onPlayerLeft(BWAPI::Player player) {
}

void CombatManager::onNukeDetect(BWAPI::Position target) {
}

void CombatManager::onUnitDiscover(BWAPI::Unit unit) {
}

void CombatManager::onUnitEvade(BWAPI::Unit unit) {
}

void CombatManager::onUnitShow(BWAPI::Unit unit) {
}

void CombatManager::onUnitHide(BWAPI::Unit unit) {
}

void CombatManager::onUnitCreate(BWAPI::Unit unit) {
}

void CombatManager::onUnitDestroy(BWAPI::Unit unit) {
}

void CombatManager::onUnitMorph(BWAPI::Unit unit) {
}

void CombatManager::onUnitRenegade(BWAPI::Unit unit) {
}

void CombatManager::onSaveGame(std::string gameName) {
}

void CombatManager::onUnitComplete(BWAPI::Unit unit) {
}
