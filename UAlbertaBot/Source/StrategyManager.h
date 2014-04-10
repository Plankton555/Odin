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

	std::vector<std::string>	protossOpeningBook;
	std::vector<std::string>	terranOpeningBook;
	std::vector<std::string>	zergOpeningBook;

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

	const	int					getScore(BWAPI::Player * player) const;
	const	double				getUCBValue(const size_t & strategy) const;
	
	// protoss strategy
	const	bool				expandProtossZealotRush() const;
	const	std::string			getProtossZealotRushOpeningBook() const;
	const	MetaPairVector		getProtossZealotRushBuildOrderGoal() const;

	const	bool				expandProtossDarkTemplar() const;
	const	std::string			getProtossDarkTemplarOpeningBook() const;
	const	MetaPairVector		getProtossDarkTemplarBuildOrderGoal() const;

	const	bool				expandProtossDragoons() const;
	const	std::string			getProtossDragoonsOpeningBook() const;
	const	MetaPairVector		getProtossDragoonsBuildOrderGoal() const;

	const	bool				expandProtossObserver() const;
	const	std::string			getProtossObserverOpeningBook() const;
	const	MetaPairVector		getProtossObserverBuildOrderGoal() const;

	const	MetaPairVector		getTerranBuildOrderGoal() const;
	const	MetaPairVector		getZergBuildOrderGoal() const;

	const	MetaPairVector		getStaticDefenceGoal() const;

	const	MetaPairVector		getProtossOpeningBook() const;
	const	MetaPairVector		getTerranOpeningBook() const;
	const	MetaPairVector		getZergOpeningBook() const;

public:

	enum { ProtossZealotRush=0, ProtossDarkTemplar=1, ProtossDragoons=2, ProtossObserver=3, ProtossZealotArchon=4, NumProtossStrategies=20 };
	enum { TerranMarineRush=0, NumTerranStrategies=1 };
	enum { ZergZerglingRush=0, NumZergStrategies=1 };
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
