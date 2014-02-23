#pragma once

#include "Common.h"
#include "GameState.h"
#include "Player.h"
#include "Game.h"
#include "UnitAction.hpp"
#include "UnitScriptData.h"
#include <boost/shared_ptr.hpp>

namespace SparCraft
{
	
typedef	boost::shared_ptr<Player> PlayerPtr;

class PortfolioGreedySearch
{
protected:
	
	const IDType				_player;
	const IDType				_enemyScript;
	const size_t				_iterations;
    const size_t                _responses;
	std::vector<IDType>			_playerScriptPortfolio;
    size_t                      _totalEvals;
    size_t                      _timeLimit;

    void                doPortfolioSearch(const IDType & player, const GameState & state, UnitScriptData & currentData);
    std::vector<UnitAction>   getMoveVec(const IDType & player, const GameState & state, const std::vector<IDType> & playerScripts);
    StateEvalScore      eval(const IDType & player, const GameState & state, UnitScriptData & playerScriptsChosen);
    IDType              calculateInitialSeed(const IDType & player, const GameState & state);
    void                setAllScripts(const IDType & player, const GameState & state, UnitScriptData & data, const IDType & script);

public:
	
	PortfolioGreedySearch(const IDType & player, const IDType & enemyScript, const size_t & iter, const size_t & responses, const size_t & timeLimit);
    std::vector<UnitAction>   search(const IDType & player, const GameState & state);
};

}