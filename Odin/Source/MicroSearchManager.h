#pragma once
/*
#include "Common.h"
#include "BWAPI.h"
#include "GameCommander.h"
#include "UnitCommandManager.h"

#include <boost/shared_ptr.hpp>

class SparCraftManager
{
	SparCraft::SearchResults previousResults;
	SparCraft::TTPtr TT;

	bool gameOver;

	float initialTotalSqrt[2];

public:

	SparCraftManager();

	SparCraft::GameState				extractGameState();
		
	const IDType						getPlayerID(BWAPI::Player * player) const;
	const bool							isCombatUnit(BWAPI::Unit * unit) const;
	const MoveTuple						getMoveTuple(SparCraft::GameState & state, const IDType & method);
	void                        		getMoves(SparCraft::GameState & state, std::vector<SparCraft::Move> & moveVec);
	SparCraft::Player *				getSparCraftPlayer(const IDType & playerModel, const IDType & player) const;
	SparCraft::SparCraftParameters	getSearchParameters() const;

	void update();
	void onStart();
	
	void doUnitMove(SparCraft::GameState & currentState, SparCraft::Unit & unit, SparCraft::Move & move);
	void drawUnitMove(SparCraft::GameState & currentState, SparCraft::Unit & unit, SparCraft::Move & move);
	void drawUnitCooldown(BWAPI::Unit * unit);
	void drawUnitHP(BWAPI::Unit * unit);
	void drawSearchResults(int x, int y);
	void drawAttackDebug();
	void performSparCraft();

	BWAPI::Unit * getUnit(SparCraft::Unit & unit);
	const std::pair<int, int> getUnitCooldown(BWAPI::Unit * unit, SparCraft::Unit & u) const;
};*/