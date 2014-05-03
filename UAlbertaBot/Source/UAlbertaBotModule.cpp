/* 
 +----------------------------------------------------------------------+
 | UAlbertaBot                                                          |
 +----------------------------------------------------------------------+
 | University of Alberta - AIIDE StarCraft Competition                  |
 +----------------------------------------------------------------------+
 |                                                                      |
 +----------------------------------------------------------------------+
 | Author: David Churchill <dave.churchill@gmail.com>                   |
 +----------------------------------------------------------------------+
 | Modified by:                                                         |
 | Björn Persson Mattsson <bjorn.pm@plankter.se>                        |
 | Filip Brynfors <brynfors@student.chalmers.se>                        |
 | Jakob Jarmar <jarmar@student.chalmers.se>                            |
 | Jakob Svensson <svjakob@student.chalmers.se>                         |
 | Henrik Alburg <alburgh@student.chalmers.se>                          |
 | Florian Minges <minges@student.chalmers.se>                          |
 +----------------------------------------------------------------------+
*/

#include "Common.h"
#include "UAlbertaBotModule.h"
#include "DataModule.h"
#include "BayesianNet.h"
#include <boost/algorithm/string.hpp>

#define BN_SNAPSHOT_EVERY_X_FRAMES 1000

BWAPI::AIModule * __NewAIModule()
{
	return new UAlbertaBotModule();
}

UAlbertaBotModule::UAlbertaBotModule()  {}
UAlbertaBotModule::~UAlbertaBotModule() {}

void UAlbertaBotModule::onStart()
{	
	DataModule::init();

	int gameID = odin_utils::getID();
	BN_output_file = odin_utils::setOutputFile(gameID);

	if(BWAPI::Broodwar->isReplay()){

		/* If we want to show stuff on the screen. */
		Options::Debug::DRAW_UALBERTABOT_DEBUG = true;

		/* Speed up replay. */
		BWAPI::Broodwar->setLocalSpeed(0);
		BWAPI::Broodwar->setFrameSkip(2400); //optional, but improves speed
		BWAPI::Broodwar->setGUI(false);

		replayModule.onStart();

	}else{
		BWAPI::Broodwar->sendText("Game ID: " + odin_utils::getID());
		odin_utils::logBN(BN_output_file, gameID);

		//BWAPI::Broodwar->sendText("Hello, my name is Odin!");
		//Logger::Instance().log("Hello, my name is Odin2!\n");

		BWAPI::Broodwar->setLocalSpeed(10);
		//BWAPI::Broodwar->setFrameSkip(240);

		SparCraft::init();

		BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);
		//BWAPI::Broodwar->enableFlag(BWAPI::Flag::CompleteMapInformation);

		//Options::BotModes::SetBotMode(Options::BotModes::AIIDE_TOURNAMENT);
		Options::Modules::checkOptions();
	
		if (Options::Modules::USING_GAMECOMMANDER)
		{
			BWTA::readMap();
			BWTA::analyze();
		}
	
		if (Options::Modules::USING_MICRO_SEARCH)
		{
			SparCraft::init();
		
			//micro.onStart();
		}

		if (Options::Modules::USING_BUILD_LEARNER)
		{
			BuildOrderSearch::getStarcraftDataInstance().init(BWAPI::Broodwar->self()->getRace());
			SparCraft::Hash::initHash();
		}

	}

}

void UAlbertaBotModule::onEnd(bool isWinner) 
{
	BWAPI::Broodwar->sendText("gg");
	if(BWAPI::Broodwar->isReplay())
	{
		replayModule.onEnd(isWinner);
	}else{
		odin_utils::increaseID();
		if (Options::Modules::USING_GAMECOMMANDER)
		{
			StrategyManager::Instance().onEnd(isWinner);

			std::stringstream result;
			std::string win = isWinner ? "win" : "lose";

			double sum = 0;
			BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->self()->getUnits())
			{
				if (unit->getType() == BWAPI::UnitTypes::Protoss_Dragoon)
				{
					sum += sqrt((double)(unit->getHitPoints() + unit->getShields()));
				}
			}
			BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
			{
				if (unit->getType() == BWAPI::UnitTypes::Protoss_Dragoon)
				{
					sum -= sqrt((double)(unit->getHitPoints() + unit->getShields()));
				}
			}

			//result << "Game against " << BWAPI::Broodwar->enemy()->getName() << " " << win << " with strategy " << StrategyManager::Instance().getCurrentStrategy() << "\n";

			result << sum << " " << BWAPI::Broodwar->getFrameCount() << "\n";

			Logger::Instance().log(result.str());

			ProductionManager::Instance().onGameEnd();
		}	
	}
	DataModule::destroy();
}

void UAlbertaBotModule::onFrame()
{
	if(BWAPI::Broodwar->isReplay())
	{
		replayModule.onFrame();

	}else{
		int currentFrame = BWAPI::Broodwar->getFrameCount();
		if (currentFrame == 260)
		{
			BWAPI::Broodwar->sendText("glhf");
		}

		if (currentFrame != 0 && (currentFrame % BN_SNAPSHOT_EVERY_X_FRAMES) == 0) 
		{
			StrategyManager::Instance().getBayesianNet()->PrintBN(BN_output_file);
		}

		if (Options::Modules::USING_UNIT_COMMAND_MGR)
		{
			UnitCommandManager::Instance().update();
		}

		if (Options::Modules::USING_GAMECOMMANDER) 
		{ 
			gameCommander.update(); 
		}
	
		if (Options::Modules::USING_ENHANCED_INTERFACE)
		{
			eui.update();
		}

		if (Options::Modules::USING_MICRO_SEARCH)
		{
			//micro.update();
		}


		if (Options::Modules::USING_REPLAY_VISUALIZER)
		{
			BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->getAllUnits())
			{
				BWAPI::Broodwar->drawTextMap(unit->getPosition().x(), unit->getPosition().y(), "   %d", unit->getPlayer()->getID());

				if (unit->isSelected())
				{
					BWAPI::Broodwar->drawCircleMap(unit->getPosition().x(), unit->getPosition().y(), 1000, BWAPI::Colors::Red);
				}
			}
		}
	}
}

void UAlbertaBotModule::onUnitDestroy(BWAPI::Unit * unit)
{
	if(BWAPI::Broodwar->isReplay())
	{
		replayModule.onUnitDestroy(unit);
	}else{
		if (Options::Modules::USING_GAMECOMMANDER) { gameCommander.onUnitDestroy(unit); }
		if (Options::Modules::USING_ENHANCED_INTERFACE) { eui.onUnitDestroy(unit); }
	}
}

void UAlbertaBotModule::onUnitMorph(BWAPI::Unit * unit)
{	if(BWAPI::Broodwar->isReplay())
	{
		replayModule.onUnitMorph(unit);
	}else{
		if (Options::Modules::USING_GAMECOMMANDER) { gameCommander.onUnitMorph(unit); }
	}
}

void UAlbertaBotModule::onSendText(std::string text) 
{ 
	if(BWAPI::Broodwar->isReplay())
	{
		std::vector<std::string> gameID;
		boost::split(gameID, text, boost::is_any_of(" "));
		if (gameID[0].compare("GameID:") == 0)
		{
			replayModule.gameID = atoi(gameID[1].c_str());
		}

		//Do nothing
	}else{
		if (text.compare("l") == 0)
		{
			BWAPI::Broodwar->leaveGame();
		} else if (text.compare("i") == 0)
		{
			BWAPI::Broodwar->setLocalSpeed(0);
		} else if (text.compare("s") == 0)
		{
			BWAPI::Broodwar->setLocalSpeed(100);
		} else if (text.compare("d") == 0) //debug
		{
			StrategyManager::Instance().updateArmyComposition();
		}

		BWAPI::Broodwar->sendText(text.c_str());

		if (Options::Modules::USING_REPLAY_VISUALIZER && (text.compare("sim") == 0))
		{
			BWAPI::Unit * selected = NULL;
			BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->getAllUnits())
			{
				if (unit->isSelected())
				{
					selected = unit;
					break;
				}
			}

			if (selected)
			{
				#ifdef USING_VISUALIZATION_LIBRARIES
					//ReplayVisualizer rv;
					//rv.launchSimulation(selected->getPosition(), 1000);
				#endif
			}
		}

		if (Options::Modules::USING_BUILD_ORDER_DEMO)
		{

			std::stringstream type;
			std::stringstream numUnitType;
			int numUnits = 0;

			int i=0;
			for (i=0; i<text.length(); ++i)
			{
				if (text[i] == ' ')
				{
					i++;
					break;
				}

				type << text[i];
			}

			for (; i<text.length(); ++i)
			{
				numUnitType << text[i];
			}

			numUnits = atoi(numUnitType.str().c_str());

	
			BWAPI::UnitType t = BWAPI::UnitTypes::getUnitType(type.str());

			BWAPI::Broodwar->printf("Searching for %d of %s", numUnits, t.getName().c_str());

			MetaPairVector goal;
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Probe, 8));
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Gateway, 2));
			goal.push_back(MetaPair(t, numUnits));

			ProductionManager::Instance().setSearchGoal(goal);
		}
	}
}

void UAlbertaBotModule::onUnitCreate(BWAPI::Unit * unit)
{ 
	if(BWAPI::Broodwar->isReplay())
	{
		replayModule.onUnitCreate(unit);
	}else{

		if (Options::Modules::USING_GAMECOMMANDER) { gameCommander.onUnitCreate(unit); }

	}
}

void UAlbertaBotModule::onUnitShow(BWAPI::Unit * unit)
{	
	if(BWAPI::Broodwar->isReplay())
	{
		//Do nothing
	}else{
		if (Options::Modules::USING_GAMECOMMANDER) { gameCommander.onUnitShow(unit); }
	}
}

void UAlbertaBotModule::onUnitHide(BWAPI::Unit * unit)
{ 
	if(BWAPI::Broodwar->isReplay())
	{
		//Do nothing
	}else{
		if (Options::Modules::USING_GAMECOMMANDER) { gameCommander.onUnitHide(unit); }
	}
}

void UAlbertaBotModule::onUnitRenegade(BWAPI::Unit * unit)
{ 
	if(BWAPI::Broodwar->isReplay())
	{
		replayModule.onUnitRenegade(unit);
	}else{
		if (Options::Modules::USING_GAMECOMMANDER) { gameCommander.onUnitRenegade(unit); }
	}

	
}

void UAlbertaBotModule::onUnitComplete(BWAPI::Unit * unit)
{
	if(BWAPI::Broodwar->isReplay())
	{
		replayModule.onUnitComplete(unit);
	}
}
