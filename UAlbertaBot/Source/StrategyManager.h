#pragma once

#include "Common.h"
#include "BWTA.h"
#include "base/BuildOrderQueue.h"
#include "InformationManager.h"
#include "base/WorkerManager.h"
#include "base/StarcraftBuildOrderSearchManager.h"
#include "BayesianNet.h"
#include "BNetParser.h"
#include <sys/stat.h>
#include <cstdlib>
#include "OdinUtils.h"
#include "DataModule.h"
#include <math.h>

#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\StarcraftData.hpp"

typedef std::pair<int, int> IntPair;
typedef std::pair<MetaType, UnitCountType> MetaPair;
typedef std::vector<MetaPair> MetaPairVector;


class StrategyManager 
{
	StrategyManager();
	~StrategyManager() {}

	std::map<std::vector<BWAPI::UnitType>*, double>	armyCounters;
	BayesianNet*				bayesianNet;
	int							lastBnUpdate;

	std::vector<std::string>	protossOpeningBook;

	std::string					readDir;
	std::string					writeDir;
	std::vector<IntPair>		results;
	std::string					readFile;
	std::vector<int>			usableStrategies;
	int							openingStrategy;
	int							currentStrategy;
	int							timeSinceLastStateUpdate;
	bool						doStateUpdateNow;

	BWAPI::Race					selfRace;
	BWAPI::Race					enemyRace;

	bool						firstAttackSent;
	bool						enemyIsRandom;

	void	loadBayesianNetwork();
	void	addStrategies();
	void	setStrategy();
	void	readResults();
	void	writeResults();
	void	loadStrategiesFromFile(std::string filename);
	void	updateState();
	bool	doStateUpdate();
	
	double	getArmyPotential(BWAPI::Player *player, double economy);
	double	getEconomyPotential(BWAPI::Player *player);
	double	getDefensePotential(BWAPI::Player *player);

	const	int					getScore(BWAPI::Player * player) const;
	const	double				getUCBValue(const size_t & strategy) const;
	const	bool				shouldExpand() const;
	// protoss strategy
	const	MetaPairVector		getProtossCounterBuildOrderGoal();
	const	MetaPairVector		getDefaultBuildOrderGoal() const;
	const	MetaPairVector		getStaticDefenceGoal() const;
	const	MetaPairVector		getProtossOpeningBook() const;

public:

	enum {NumProtossStrategies=20 };
	enum State { OPENING, ATTACK, DEFEND, EXPAND };

	static	StrategyManager &	Instance();

			void				updateArmyComposition();
			BayesianNet*		getBayesianNet() { return bayesianNet; }
			void				onUnitShow(BWAPI::Unit * unit);
			void				onEnd(const bool isWinner);
	
	const	bool				regroup(int numInRadius);
	const	bool				doAttack(const std::set<BWAPI::Unit *> & freeUnits);
	const	int				    defendWithWorkers();
	const	bool				rushDetected();

	const	int					getCurrentStrategy();

	const	MetaPairVector		getBuildOrderGoal();
	const	std::string			getOpeningBook() const;

	void						update();
	State						state;
};
