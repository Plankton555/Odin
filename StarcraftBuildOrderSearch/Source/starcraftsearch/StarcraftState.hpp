#ifndef STARCRAFT_STATE_H
#define STARCRAFT_STATE_H

#include <string.h>
#include <queue>
#include <algorithm>
#include <fstream>

#include "BWAPI.h"
#include <limits.h>
#include "ActionSet.hpp"
#include "StarcraftAction.hpp"
#include "StarcraftData.hpp"
#include "ActionInProgress.hpp"
#include "BuildingData.hpp"
#include "StarcraftSearchGoal.hpp"
#include "StarcraftSearchConstraint.hpp"
#include <math.h>

// #define DEBUG_LEGAL
namespace BuildOrderSearch
{
class Hatchery
{ 
public:
	UnitCountType numLarva; 
	FrameCountType lastThirdUsed;
	
	Hatchery(UnitCountType n, FrameCountType t) 
		: numLarva(n), lastThirdUsed(t) {}

	Hatchery() 
		: numLarva(0), lastThirdUsed(0) {}
};

class StarcraftState 
{

private:

	/*********************************************************************
	* DATA
	**********************************************************************/

	StarcraftState *	parent;				        // the state that generated this state

	ActionsInProgress	progress;					
	BuildingData		buildings;

	ActionSet 		    completedUnitSet,			// ActionSet of completed units
			            progressUnitSet;			// ActionSet bitmask of units in progress

	WorkerCountType		mineralWorkers, 		    // number of workers currently mining
			            gasWorkers; 			    // number of workers currently getting gas

	Action			    actionPerformed, 		    // the action which generated this state
						actionPerformedK;

	SupplyCountType		maxSupply, 			        // our maximum allowed supply
			            currentSupply; 			    // our current allocated supply
			            
	FrameCountType 	    currentFrame,
						lastActionFrame;		    // the current frame of the game
			            
	ResourceCountType	minerals, 			        // current mineral count
			            gas,						// current gas count
			            armyValue;					// running army integral value
			          
	UnitCountType 	    numUnits[MAX_ACTIONS],		// how many of each unit are completed
						numHatcheries,				
						numLarva;

	Hatchery			hatcheries[MAX_HATCHERIES];	// zerg specific hatcheries
	

#ifdef EXTRA_STARCRAFTSTATE_STATISTICS
	
	std::vector< std::pair< FrameCountType, ResourceCountType > > armyIntegral;
	
#endif

	
	void xsetGameStartData()
	{
		// starting resources and supply
		minerals = 50; 
		gas = 0; 
		maxSupply = 18; 
		currentSupply = 8;
		mineralWorkers = 4;

		buildings.addBuilding(DATA.getResourceDepot());
		setNumUnits(DATA.getWorker(), 4);
		setNumUnits(DATA.getResourceDepot(), 1);

		if (DATA.getRace() == BWAPI::Races::Zerg)
		{
			setNumUnits(DATA.getAction(BWAPI::UnitTypes::Zerg_Overlord), 1);
			numHatcheries = 1;
			numLarva = 3;
			hatcheries[0] = Hatchery(3,0);
		}
		else
		{
			numHatcheries = 0;
			numLarva = 0;
		}
	}

	bool xspecificIsLegal(Action a)	
	{ 
		// PROTOSS: Nothing
		// TERRAN:	Nothing
		// ZERG:	Nothing

		return true; 
	}
	
	FrameCountType xspecificWhenReady(Action a)
	{
		// PROTOSS: Nothing
		// TERRAN:	Nothing
		
		if (DATA.getRace() == BWAPI::Races::Zerg)
		{
			if (GSN_DEBUG) printf("\tSpecific When Ready\n");
			FrameCountType l = currentFrame;		
		
			// if the current action is a unit
			if (DATA[a].isUnit()) 
			{
				// if we don't have any larva ready, figure out when it will pop
				if (numLarva == 0)
				{
					if (GSN_DEBUG) printf("\t\tZerg Larva Not Ready\n");

					// min value placeholder
					int min = 88888888;

					// figure out the hatchery with the least recent used larva
					for (int i(0); i<numHatcheries; ++i) 
					{
						// set the minimum accordingly
						min = hatcheries[i].lastThirdUsed < min ? hatcheries[i].lastThirdUsed : min;
					}

					l = min + ZERG_LARVA_TIMER + 5;
				}
			}

			if (GSN_DEBUG) printf("\tSpecific When Ready End (return %d)\n", l);
			return l;
		}
		
		return 0;
	}

	void xspecificDoAction(const Action a, const FrameCountType ffTime)
	{
		if (DATA.getRace() == BWAPI::Races::Terran)
		{
			if (DATA[a].isBuilding())
			{
				mineralWorkers--;
			}
		}
		else if (DATA.getRace() == BWAPI::Races::Zerg)
		{
			if (GSN_DEBUG) printf("\tSpecific Do Action %d\n", ffTime);

			// zerg must subtract a larva if the action was unit creation
			if (DATA[a].isUnit() && !DATA[a].isBuilding()) 
			{		
				if (GSN_DEBUG) printf("\t\tZerg Unit - Remove Larva\n");
				// subtract the larva
				numLarva--;
			
				// for each of our hatcheries
				for (int i(0); i<numHatcheries; ++i) 
				{			
					// if this hatchery has a larva available
					if (hatcheries[i].numLarva > 0) 
					{
						// if there were 3 larva previously, update lastUsed
						if (hatcheries[i].numLarva == 3) hatcheries[i].lastThirdUsed = currentFrame;

						// subtract it from this hatchery
						hatcheries[i].numLarva--;
					
						// break out of the hatchery loop
						break;
					}
				}
			}
			else if (DATA[a].isBuilding())
			{
				if (GSN_DEBUG) printf("\t\tZerg Building - Remove Drone\n");

				// special case of morphed buildings
				if (DATA[a].getUnitType() == BWAPI::UnitTypes::Zerg_Lair || DATA[a].getUnitType() == BWAPI::UnitTypes::Zerg_Greater_Spire)
				{
					// the previous building starts morphing into this one
				}
				else
				{
					// otherwise, a drone is subtracted to start construction
					addNumUnits(DATA.getWorker(), -1);
					addMineralWorkers(-1);
					currentSupply -= DATA[DATA.getWorker()].supplyRequired();
				}
			}

			if (GSN_DEBUG) printf("\tSpecific Do Action End\n");
		}
	}

	void xspecificFastForward(const FrameCountType toFrame)
	{
		if (DATA.getRace() == BWAPI::Races::Zerg)
		{
			if (GSN_DEBUG) printf("\t\tSpecific Fast Forward\n");

			// for each hatchery we have
			for (int i(0); i<numHatcheries; ++i) 
			{	
				// only do this if there's less than 3 larva present
				if (hatcheries[i].numLarva < 3) 
				{
					// how much time will have past since the last larva used
					FrameCountType diff = toFrame - hatcheries[i].lastThirdUsed;

					// figure out how many larva should have spawned
					UnitCountType numLarvaSpawn = (UnitCountType)(diff / ZERG_LARVA_TIMER);

					// cut that number down to make sure no more than 3 total exist
					if (numLarvaSpawn > (3 - hatcheries[i].numLarva)) numLarvaSpawn = 3 - hatcheries[i].numLarva;

					// add that to the number of larva
					hatcheries[i].numLarva += numLarvaSpawn;
					numLarva += numLarvaSpawn;

					// if it's less than 3, update the timer ticker
					if (hatcheries[i].numLarva < 3) 
					{
						hatcheries[i].lastThirdUsed += (numLarvaSpawn * ZERG_LARVA_TIMER);
					}
				
					// if it's exactly 3, set it to toFrame (this might not even matter)
					if (hatcheries[i].numLarva == 3) 
					{
						hatcheries[i].lastThirdUsed = toFrame;
					}

					//printf("Num Larva Spawn: %d %d\n", numLarvaSpawn, hatcheries[i].numLarva);
				} 
				else 
				{
					hatcheries[i].lastThirdUsed = toFrame;
				}	
			}
		}
	}

	void xspecificFinishAction(const Action a)
	{
		if (DATA.getRace() == BWAPI::Races::Terran)
		{
			// if it's a building, release the worker back
			if (DATA[a].isBuilding())
			{
				mineralWorkers++;
			}
		}
		else if (DATA.getRace() == BWAPI::Races::Zerg)
		{
			// if it's a Lair, remove the hatchery
			if (DATA[a].isBuilding() && (DATA[a].getUnitType() == BWAPI::UnitTypes::Zerg_Lair))
			{
				numUnits[DATA.getAction(BWAPI::UnitTypes::Zerg_Hatchery)]--;

				// take away the supply that was provided by the hatchery
				maxSupply -= DATA[DATA.getAction(BWAPI::UnitTypes::Zerg_Hatchery)].supplyProvided();
			}

			// if it's a Greater Spire, remove the spire
			//if (DATA[a].isBuilding() && (DATA[a].getUnitType() == BWAPI::UnitTypes::Zerg_Greater_Spire))
			//{
			///	numUnits[DATA.getAction(BWAPI::UnitTypes::Zerg_Spire)]--;
			//}

			fixZergUnitMasks();
		}
	}

	void fixZergUnitMasks()
	{
		ActionSet completedZergUnits(__ZERO);

		for (Action a(0); a<DATA.size(); ++a)
		{
			if (numUnits[a] > 0)
			{
				completedZergUnits.add(a);
			}
		}

		if (numUnits[DATA.getAction(BWAPI::UnitTypes::Zerg_Lair)] > 0)
		{
			completedZergUnits.add(DATA.getAction(BWAPI::UnitTypes::Zerg_Hatchery));
		}

		//if (numUnits[DATA.getAction(BWAPI::UnitTypes::Zerg_Greater_Spire)] > 0)
		//{
		//	completedZergUnits.add(DATA.getAction(BWAPI::UnitTypes::Zerg_Spire));
		//}
	}

public: 

	/**********************************************************************************
	*
	*                 Action Performing Functions - Changes State
	*
	**********************************************************************************/


	StarcraftState(bool init = false)
		: parent(NULL)
		, actionPerformed(255)
		, actionPerformedK(1)
		, currentFrame(0)
		, currentSupply(0)
		, lastActionFrame(0)
		, maxSupply(0)
		, mineralWorkers(0)
		, gasWorkers(0)
		, armyValue(0)
		, numHatcheries(0)
		, numLarva(0)
	{
		for (int i=0; i<MAX_ACTIONS; ++i)
		{
			numUnits[i] = 0;
		}
		
		if (init)
		{
			xsetGameStartData();
		}

		setUnitMask(); 
		setProgressMask();
	}


	// do an action, action must be legal for this not to break
	void doAction(const Action action, const FrameCountType ffTime)
	{
		if (GSN_DEBUG) printf("Do Action - %s\n", DATA[action].getName().c_str());

		// set the actionPerformed
		actionPerformed = action;
		actionPerformedK = 1;

		// fast forward to when this action can be performed
		if (ffTime != -1) 
		{ 
			fastForward(ffTime);
		} 
		else 
		{
			fastForward(resourcesReady(action));
		}
		
		// how much time has elapsed since the last action was queued?
		FrameCountType 		elapsed(currentFrame - lastActionFrame);
		armyValue 			+= elapsed * getArmyValue();
		lastActionFrame		= currentFrame;
		
		// extra statistics here
		#ifdef EXTRA_STARCRAFTSTATE_STATISTICS
			armyIntegral.push_back( std::pair<FrameCountType, ResourceCountType>(currentFrame, getArmyValue()) );
		#endif

		// create the struct
		FrameCountType tempTime = currentFrame + DATA[action].buildTime();

		// if building, add a constant amount of time to find a building placement
		if (DATA[action].isBuilding()) 
		{
			// add 2 seconds
			tempTime += BUILDING_PLACEMENT;
		}

		// add it to the actions in progress
		progress.addAction(action, tempTime);

		// update the progress bitmask
		progressUnitSet.add(action);
			
		// if what builds this unit is a building
		if (DATA[action].whatBuildsIsBuilding() && (DATA.getRace() != BWAPI::Races::Zerg))
		{
			// add it to a free building, which MUST be free since it's called from doAction
			// which must be already fastForwarded to the correct time
			buildings.queueAction(action);
		}

		// modify our resources
		minerals 		-= DATA[action].mineralPrice();
		gas	 			-= DATA[action].gasPrice();
		currentSupply 	+= DATA[action].supplyRequired();
		 
		
		if (minerals < 0 || gas < 0)
		{
			printf("Resource Error: m:%d g:%d\n", minerals, gas);
		}

		// do race specific things here, like subtract a larva
		xspecificDoAction(action, ffTime);

		if (GSN_DEBUG) printf("Do Action End\n");
	}

	// fast forwards the current state to time toFrame
	void fastForward(const FrameCountType toFrame) 
	{
		if (GSN_DEBUG) printf("\tFast Forward to %d from %d progress size %d\n", toFrame, currentFrame, progress.size());

		// do specific things here, like figure out how many larva we will have
		xspecificFastForward(toFrame);
		
		// fast forward the building timers to the current frame
		buildings.fastForwardBuildings(toFrame - currentFrame);

		// update resources & finish each action
		FrameCountType 		lastActionFinished(currentFrame);
		FrameCountType		totalTime(0);
		double 				moreGas(0);
		double				moreMinerals(0);

		while (!progress.isEmpty() && progress.nextActionFinishTime() <= toFrame) 
		{		
			// figure out how long since the last action was finished
			FrameCountType timeElapsed 	= (FrameCountType)progress.nextActionFinishTime() - lastActionFinished;
			totalTime 			+= timeElapsed;

			// update our mineral and gas count for that period
			moreMinerals 		+= timeElapsed * getMineralsPerFrame();
			moreGas 			+= timeElapsed * getGasPerFrame();

			// update when the last action was finished
			lastActionFinished 	= (FrameCountType)progress.nextActionFinishTime();

			// finish the action, which updates mineral and gas rates if required
			finishNextActionInProgress();
		}

		// update resources from the last action finished to toFrame
		FrameCountType elapsed 	=  toFrame - lastActionFinished;
		moreMinerals 			+= elapsed * getMineralsPerFrame();
		moreGas 				+= elapsed * getGasPerFrame();
		totalTime 				+= elapsed;

		minerals 				+= (ResourceCountType)(ceil(moreMinerals));
		gas 					+= (ResourceCountType)(ceil(moreGas));

		// we are now in the FUTURE... "the future, conan?"
		currentFrame 			= toFrame;

		if (GSN_DEBUG) printf("\tFast Forward End\n");
	}


	// when we want to 'complete' an action (its time is up) do this
	void finishNextActionInProgress() 
	{	
		if (GSN_DEBUG) printf("\t\tFinishing Action %s\n", DATA[progress.nextAction()].getName().c_str());

		// get the actionUnit from the progress data
		Action actionUnit = progress.nextAction();

		// add the unit to the unit counter
		addNumUnits(actionUnit, DATA[actionUnit].numProduced());
	
		// add to the max supply if it's a supplying unit
		maxSupply += DATA[actionUnit].supplyProvided();
	
		// if it's a worker, put it on minerals
		if (DATA[actionUnit].isWorker()) 
		{ 
			mineralWorkers++;
		}

		// if it's an extractor
		if (DATA[actionUnit].isRefinery()) 
		{
			// take those workers from minerals and put them into it
			mineralWorkers -= 3; gasWorkers += 3;
		}	
		
		// if it's a building that can produce units, add it to the building data
		if (DATA[actionUnit].isBuilding() && !DATA[actionUnit].isSupplyProvider())
		{
			buildings.addBuilding(actionUnit);
		}

		// pop it from the progress vector
		progress.popNextAction();
	
		// we need to re-set the whole progress mask again :(
		setProgressMask();
		
		// do race specific action finishings
		xspecificFinishAction(actionUnit);

		if (GSN_DEBUG) printf("\t\tFinish Action End\n");
	}
	
	/**********************************************************************************
	*
	*                 When Resources Ready Function + Sub Functions
	*
	**********************************************************************************/

	// returns the time at which all resources to perform an action will be available
	FrameCountType resourcesReady(const Action action) 
	{
		if (GSN_DEBUG) printf(">\n>\n> NEW ACTION: %s\n>\n> Progress Size %d\n>\n\nResource Calculation - %s\n", DATA[action].getName().c_str(), progress.size(), DATA[action].getName().c_str());

		// the resource times we care about
		FrameCountType 	
				m(currentFrame), 	// minerals
				g(currentFrame), 	// gas
				l(currentFrame), 	// class-specific
				s(currentFrame), 	// supply
				p(currentFrame), 	// prerequisites
				maxVal(currentFrame);
	
		// figure out when prerequisites will be ready
		p = whenPrerequisitesReady(action);

		// check minerals
		m = whenMineralsReady(action);

		// check gas
		g = whenGasReady(action);

		// race specific timings (Zerg Larva)
		l = xspecificWhenReady(action);

		// set when we will have enough supply for this unit
		s = whenSupplyReady(action);

		// figure out the max of all these times
		maxVal = (m > maxVal) ? m : maxVal;
		maxVal = (g > maxVal) ? g : maxVal;
		maxVal = (l > maxVal) ? l : maxVal;
		maxVal = (s > maxVal) ? s : maxVal;
		maxVal = (p > maxVal) ? p : maxVal;
	
		if (GSN_DEBUG) 
		{
			printf("\tMinerals  \t%d\n\tGas  \t\t%d\n\tSpecific  \t%d\n\tSupply  \t%d\n\tPreReq  \t%d\n", m, g, l, s, p);
			printf("Resource Calculation End (return %d)\n\n", maxVal);
		}
		
		// return the time
		return maxVal + 1;
	}
	
	FrameCountType whenSupplyReady(const Action action) const
	{
		if (GSN_DEBUG) printf("\tWhen Supply Ready\n");
		FrameCountType s = currentFrame;
	
		// set when we will have enough supply for this unit
		int supplyNeeded = DATA[action].supplyRequired() + currentSupply - maxSupply;
		if (supplyNeeded > 0) 
		{	
			//if (GSN_DEBUG) printf("\t\tSupply Is Needed: %d\n", supplyNeeded);
		
			// placeholder for minimum olord time
			FrameCountType min = 99999;

			// if we don't have the resources, this action would only be legal if there is an
			// overlord in progress, so check to see when the first overlord will finish
			for (int i(0); i<progress.size(); ++i) 
			{
				if (GSN_DEBUG) printf("\t\tSupply Check: %s %d Progress Size %d\n", DATA[progress.getAction(i)].getName().c_str(), DATA[progress.getAction(i)].supplyProvided(), progress.size());
			
				// so, if the unit provides the supply we need
				if (DATA[progress.getAction(i)].supplyProvided() > supplyNeeded) 
				{
					// set 'min' to the min of these times
					min = (progress.getTime(i) < min) ? progress.getTime(i) : min;
					
					//if (GSN_DEBUG) printf("\t\tSupply Found: %s %d\n", DATA[progress.getAction(i)].getName().c_str(), min);
				}

				// then set supply time to min
				s = min;
			}
		}
		
		return s;
	}
	
	FrameCountType whenPrerequisitesReady(const Action action) const
	{
		if (GSN_DEBUG) printf("\tCalculating Prerequisites\n");
	
		FrameCountType p = currentFrame;
	
		// if a building builds this action
		if (DATA[action].whatBuildsIsBuilding())
		{
			if (GSN_DEBUG) printf("\t\tAction Needs Building\n");
		
			// get when the building / prereqs will be ready
			p = whenBuildingPrereqReady(action);
		}
		// otherwise something else builds this action so we don't worry about buildings
		else
		{
			// if requirement in progress (and not already made), set when it will be finished
			ActionSet reqInProgress = (DATA[action].getPrerequisites() & progressUnitSet) - completedUnitSet;
			
			if (GSN_DEBUG) printf("\t\tAction Does Not Need Building\n");
			
			// if it's not empty, check when they will be done
			if (!reqInProgress.isEmpty())
			{
				if (GSN_DEBUG) printf("\t\tAction Has Prerequisites In Progress\n");
			
				p = progress.whenActionsFinished(reqInProgress);
			}
		}
		
		if (GSN_DEBUG) printf("\tCalculating Prerequisites End (return %d)\n", p);
		return p;
	}
	
	FrameCountType whenBuildingPrereqReady(const Action action) const
	{
		if (GSN_DEBUG) printf("\t\tWhen Building Prereq Ready\n");
	
		Action builder = DATA[action].whatBuildsAction();
	
		assert(DATA[builder].isBuilding());
	
		// is the building currently constructed
		bool buildingConstructed = numUnits[builder] > 0;
		
		// is the building currently in progress
		bool buildingProgress = progress.numInProgress(builder) > 0;
	
		// when the building will be available
		FrameCountType buildingAvailableTime;
	
		// if the building is both finished and in progress
		if (buildingConstructed && buildingProgress)
		{
			if (GSN_DEBUG) printf("\t\t\tBuilding Constructed %d and In Progress %d\n", numUnits[builder], progress.numInProgress(builder));
		
			// get the time the constructed version will be free
			FrameCountType A = (FrameCountType)whenConstructedBuildingReady(builder);
			
			// get the time the progress version will be free
			FrameCountType B = (FrameCountType)progress.nextActionFinishTime(builder);
			
			// take the 
			buildingAvailableTime = (A < B) ? A : B;
		}
		// otherwise if the constructed version is all we have
		else if (buildingConstructed)
		{
			if (GSN_DEBUG) printf("\t\t\tBuilding Constructed Only\n");
		
			// set the time accordingly
			buildingAvailableTime = whenConstructedBuildingReady(builder);
		}
		// otherwise the progress version is all we have
		else
		{
			if (GSN_DEBUG) printf("\t\t\tBuilding In Progress Only\n");
		
			// set the time accordingly
			buildingAvailableTime = (FrameCountType)progress.nextActionFinishTime(builder);
		}

		// get all prerequisites currently in progress but do not have any completed
		ActionSet prereqInProgress = (DATA[action].getPrerequisites() & progressUnitSet) - completedUnitSet;
		
		// remove the specific builder from this list since we calculated that earlier
		prereqInProgress.subtract(builder);
		
		// if we actually have some prerequisites in progress other than the building
		if (!prereqInProgress.isEmpty())
		{
			// get the max time the earliest of each type will be finished in
			FrameCountType C = progress.whenActionsFinished(prereqInProgress);
			
			// take the maximum of this value and when the building was available
			buildingAvailableTime = (C > buildingAvailableTime) ? C : buildingAvailableTime;
		}
	
		if (GSN_DEBUG) printf("\t\tWhen Building Prereq Ready End (return %d)\n", buildingAvailableTime);
		return buildingAvailableTime;
	}
	
	FrameCountType whenConstructedBuildingReady(const Action builder) const
	{
		if (GSN_DEBUG) printf("\t\t\tWhen Constructed Building Ready\n");
		
		// if what builds a is a building and we have at least one of them completed so far
		if (DATA[builder].isBuilding() && numUnits[builder] > 0 )
		{
			FrameCountType returnTime = currentFrame + buildings.timeUntilFree(builder);
		
			if (GSN_DEBUG) printf("\t\t\tWhen Constructed Building Ready End (return %d)\n", returnTime);
		
			// get when the next building is available
			return returnTime;
		}
		
		return currentFrame;
	}

	// when will minerals be ready
	FrameCountType whenMineralsReady(const Action action) const 
	{
		int difference = DATA[action].mineralPrice() - minerals;

		double m = currentFrame;
		double addMinerals = 0, addTime = 0;

		if (difference > 0) 
		{	
			FrameCountType lastAction = currentFrame;
			int tmw = mineralWorkers, tgw = gasWorkers;

			for (int i(0); i<progress.size(); ++i) 
			{
				// the vector is sorted in descending order
				int ri = progress.size() - i - 1;

				// the time elapsed and the current minerals per frame
				int elapsed = progress.getTime(ri) - lastAction;
				double mpf = (tmw * DATA.mpwpf);
	
				// the amount of minerals that would be added this time step
				double tempAdd = elapsed * mpf, tempTime = elapsed;

				// if this amount is enough to push us over what we need
				if (addMinerals + tempAdd >= difference) 
				{				
					// figure out when we go over
					tempTime = (difference - addMinerals) / mpf;

					// add the minerals and time
					addMinerals += tempTime * mpf;
					addTime += tempTime;

					//if (GSN_DEBUG) printf("Necessary Minerals Acquired Mid-Iteration: %lf\n", addMinerals); 

					// break out of the loop
					break;

				// otherwise, add the whole interval
				} 
				else 
				{
					addMinerals += tempAdd;
					addTime += elapsed;
					
					//if (GSN_DEBUG) printf("Another Mineral Iteration Necessary: %lf\n", addMinerals);
				}

				// if it was a drone or extractor update the temp variables
				if (DATA[progress.getAction(ri)].isWorker()) 
				{
					tmw++;
				} 
				else if (DATA[progress.getAction(ri)].isRefinery()) 
				{
					tmw -= 3; tgw += 3;
				}

				// update the last action
				lastAction = progress.getTime(ri);
			}

			// if we still haven't added enough minerals, add more time
			if (addMinerals < difference) 
			{
				addTime += (difference - addMinerals) / (tmw * DATA.mpwpf);
				
				//if (GSN_DEBUG) printf("\t\tNot Enough Minerals, Adding: minerals(%lf) time(%lf)\n", (difference - addMinerals), addTime); 
			}

			m += addTime;
		}

		//if (GSN_DEBUG) printf("\tMinerals Needs Adding: Minerals(%d, %lf) Frames(%lf, %d > %d)\n", difference, addMinerals, addTime, currentFrame, (int)ceil(m));

		// for some reason if i don't return +1, i mine 1 less mineral in the interval
		return (FrameCountType)(ceil(m) + 1);
	}

	FrameCountType whenGasReady(const Action action) const
	{
		double g = currentFrame;
		int difference = DATA[action].gasPrice() - gas;
		double addGas = 0, addTime = 0;
	
		if (difference > 0) 
		{
			int lastAction = currentFrame;
			int tmw = mineralWorkers, tgw = gasWorkers;
			

			for (int i(0); i<progress.size(); ++i) 
			{
				// the vector is sorted in descending order
				int ri = progress.size() - i - 1;

				// the time elapsed and the current minerals per frame
				int elapsed = progress.getTime(ri) - lastAction;
				double gpf = (tgw * DATA.gpwpf);
	
				// the amount of minerals that would be added this time step
				double tempAdd = elapsed * gpf, tempTime = elapsed;

				// if this amount is enough to push us over what we need
				if (addGas + tempAdd >= difference) 
				{
					// figure out when we go over
					tempTime = (difference - addGas) / gpf;

					// add the minerals and time
					addGas += tempTime * gpf;
					addTime += tempTime;

					// break out of the loop
					break;

				// otherwise, add the whole interval
				} else {

					addGas += tempAdd;
					addTime += elapsed;
				}

				// if it was a drone or extractor update temp variables
				if (DATA[progress.getAction(ri)].isWorker()) 
				{
					tmw++;
				} 
				else if (DATA[progress.getAction(ri)].isRefinery()) 
				{
					tmw -= 3; tgw += 3;
				}

				// update the last action
				lastAction = progress.getTime(ri);
			}

			// if we still haven't added enough minerals, add more time
			if (addGas < difference) 
			{
				addTime += (difference - addGas) / (tgw * DATA.gpwpf);
			}

			g += addTime;
		}

		//if (GSN_DEBUG) printf("\tGas Needs Adding: Gas(%d, %lf) Frames(%lf, %d > %d)\n", difference, addGas, addTime, currentFrame, (int)ceil(g));

		return (FrameCountType)(ceil(g) + 1);
	}
	
	/**********************************************************************************
	*
	*                 Heuristic Search + Pruning + Evaluation Functions
	*
	**********************************************************************************/
	
	// Gets an upper bound on the time it will take to complete goal:
	// Sum the following:
	// calculateGoalResourceLowerBound()
	// calculateDependencyTimeRemaining()
	// sum the build time of everything in the goal
	FrameCountType calculateUpperBoundHeuristic(const StarcraftSearchGoal & goal) const
	{
		// the upper bound
		FrameCountType upperBound(0);
	
		// dependency chain build time heuristic
		upperBound += calculateDependencyHeuristic(goal);
		
		// resources ready heuristic
		upperBound += calculateResourcesReadyHeuristic(goal);
		
		// add build time for everything in the goal
		for (Action a = 0; a<DATA.size(); ++a)
		{	
			// how many of this action we still need to build
			int need = goal[a] - numUnits[a];
		
			// if this action is in the goal
			if (need > 0)
			{
				upperBound += need * DATA[a].buildTime();
			}
		}
		
		return currentFrame + upperBound;
	}
	
	FrameCountType workerUpperBound(const StarcraftSearchGoal & goal) const
	{
		std::pair<ResourceCountType,ResourceCountType> grlb = calculateGoalResourceLowerBound(goal);
		std::pair<FrameCountType, FrameCountType> resourceReadyPair = resourceReadyLowerBound(grlb.first, grlb.second);
		
		return resourceReadyPair.second;
	}

	// heuristic evaluation function for this state
	FrameCountType eval(const StarcraftSearchGoal & goal, const bool useLandmark = true) const
	{
		// dependency chain build time heuristic
		FrameCountType dependencyHeuristic = calculateDependencyHeuristic(goal);
		
		if (useLandmark)
		{
			return dependencyHeuristic;
		}
		else
		{
			return 1;
		}
	}	
	
	FrameCountType calculateResourcesReadyHeuristic(const StarcraftSearchGoal & goal) const
	{
		// resources ready heuristic
		std::pair<ResourceCountType, ResourceCountType> grlb = calculateGoalResourceLowerBound(goal);
		std::pair<FrameCountType, FrameCountType> resourceReadyPair = resourceReadyLowerBound(grlb.first, grlb.second);
		FrameCountType resourceReadyLowerBoundVal = resourceReadyPair.first - currentFrame;
		
		return resourceReadyLowerBoundVal;
	}
	
	FrameCountType calculateDependencyHeuristic(const StarcraftSearchGoal & goal) const
	{
		// the maximum dependency time for all actions in the goal
		FrameCountType max = 0;
	
		// for each action which exists
		for (Action a = 0; a<DATA.size(); ++a)
		{	
			// if this action is in the goal
			if (goal[a] > numUnits[a])
			{
				// calculate the time remaining for this action's dependencies
				int actionDTR = calculateDependencyTimeRemaining(a);
				
				// set the maximum value
				max = (actionDTR > max) ? actionDTR : max;
			}
		}
	
		// return the maximum value
		return max;
	}
	
	// calculates a lower bound on the amount of time it would take us to
	// reach the resources necessary to reach our goal
	std::pair<ResourceCountType, ResourceCountType> calculateGoalResourceLowerBound(const StarcraftSearchGoal & goal) const
	{
		std::pair<ResourceCountType, ResourceCountType> totalResources(0,0);
		
		// first calculate the resources needed directly by goal actions
		for (Action a(0); a < MAX_ACTIONS; ++a)
		{
			// if this action is in the goal
			if (goal.get(a))
			{
				// calculate how many we still need of this action
				int stillNeed(goal[a] - (numUnits[a] + progress[a]));
				
				// if it is greater than zero, add it to the total
				if (stillNeed > 0)
				{
					//printf("Adding price of %d %s (%d,%d)\n", stillNeed, DATA[a].getName().c_str(), DATA[a].mineralPrice(), DATA[a].gasPrice());
				
					totalResources.first  += (ResourceCountType)(stillNeed * DATA[a].mineralPrice());
					totalResources.second += (ResourceCountType)(stillNeed * DATA[a].gasPrice());
				}
			}
		}
		
		// calculate how many resources are still needed by dependencies remaining
		// for all actions in the goal
		
		// an actionset of what we've added so far
		// this will prevent shared dependencies from adding resources twice
		ActionSet addedSoFar(0);
		
		// for each action in the goal
		for (Action a(0); a < MAX_ACTIONS; ++a)
		{
			// if we need some of this action 
			// and we don't have any already (since this would always return time zero
			if (goal[a])
			{
				//printf("Calculating Dependency Resources For %s\n", DATA[a].getName().c_str());
			
				// calculate how many resources we would need for these dependencies
				calculateDependencyResourcesRemaining(a, totalResources, addedSoFar);
			}
		}
		
		// if the total resources requires gas and we do not have a refinery, add the price of one
		if (totalResources.second > 0 && !numUnits[DATA.getRefinery()] && !progress[DATA.getRefinery()])
		{
			//printf("Goal requires gas, adding refinery price\n");
			totalResources.first  += DATA[DATA.getRefinery()].mineralPrice();
			totalResources.second += DATA[DATA.getRefinery()].gasPrice();
		}
		
		// return the total
		return totalResources;
	}
	
	void calculateDependencyResourcesRemaining(const int action, std::pair<ResourceCountType, ResourceCountType> & totalResources, ActionSet & addedSoFar) const
	{
		// get the strict dependencies of this action
		ActionSet strictDependencies = DATA.getStrictDependency(action);
		
		// for each of the top level dependencies this action has
		while (!strictDependencies.isEmpty())
		{
			// get the next action from this dependency
			Action nextStrictDependency = strictDependencies.popAction();
	
			// if we have already started, completed, or added this action already, continue
			if (completedUnitSet.contains(nextStrictDependency) || progressUnitSet.contains(nextStrictDependency))
			{
				continue;
			}
			// otherwise we haven't seen it yet
			else

			{
				// if we haven't added it yet, add it
				if (!addedSoFar.contains(nextStrictDependency))
				{
					// add the resources
					totalResources.first  += (short)DATA[nextStrictDependency].mineralPrice();
					totalResources.second += (short)DATA[nextStrictDependency].gasPrice();
				
					// mark the action as added
					addedSoFar.add(nextStrictDependency);
				}
				else
				{
					//printf("\tPreviously added price of %s, skipping\n",DATA[nextStrictDependency].getName().c_str()); 
				}
				
				// recurse down the line
				calculateDependencyResourcesRemaining(nextStrictDependency, totalResources, addedSoFar);
			}
		}
	}
	
	// calculates the time remaining in the chain of dependencies of action given 'have' already completed
	FrameCountType calculateDependencyTimeRemaining(const Action action) const
	{
		// get the strict dependencies of this action
		ActionSet strictDependencies = DATA.getStrictDependency(action);
		
		//bool actionCompleted = completedUnitSet.contains(action);
		bool actionInProgress = progressUnitSet.contains(action);
		
		// this will hold the max return time of all dependencies
		FrameCountType max = 0;
		
		// if one of the unit is in progress, get the remaining time on it
		if (actionInProgress)
		{
			max += progress.nextActionFinishTime(action) - currentFrame;
		}
		// otherwise get the build time of the unit
		else
		{
			max += DATA[action].buildTime();
		}
		
		// for each of the top level dependencies this action has
		while (!strictDependencies.isEmpty())
		{
			// get the next action from this dependency
			Action nextStrictDependency = strictDependencies.popAction();
	
			// if we have this dependency completed, break out of the loop
			if (completedUnitSet.contains(nextStrictDependency))
			{
				//if (1) printf("DEPCALC:  Have  %s, stop\n", DATA[nextStrictDependency].getName().c_str());
				continue;
			}
			// if we have this dependency in progress
			else if (progressUnitSet.contains(nextStrictDependency))
			{
				// return the time left on this action
				FrameCountType timeRemaining = progress.nextActionFinishTime(nextStrictDependency) - currentFrame;
				//if (1) printf("DEPCALC:  Prog  %s, %d\n", DATA[nextStrictDependency].getName().c_str(), timeRemaining);
				
				max = (timeRemaining > max) ? timeRemaining : max;
			}
			// we do not have this dependency at all
			else
			{
				//if (1) printf("DEPCALC:  Need  %s\n", DATA[nextStrictDependency].getName().c_str());
			
				// sum the current build time as well as that of the next strict dependency
				FrameCountType sum = DATA[nextStrictDependency].buildTime() + calculateDependencyTimeRemaining(nextStrictDependency);
		
				// set the maximum
				max = (sum > max) ? sum : max;
			}
		}
		
		// return the maxium over all strict dependencies recursively
		// if this action had no dependencies max will be zero
		
		if (max == 0)
		{
			printf("WTF\n");
		}
		
		return max;
	}
	
	std::pair<FrameCountType, FrameCountType> resourceReadyLowerBound(const ResourceCountType goalMinerals, const ResourceCountType goalGas) const
	{
	
		double currentMinerals = minerals;
		double currentGas = gas;
		double currentWorkers = mineralWorkers;
		double mineralTimeElapsed = 0;
		double remainingMinerals, mineralTimeRemaining;
	
		bool alwaysTrue(true);
		while (alwaysTrue)
		{
			remainingMinerals = goalMinerals - currentMinerals;
			mineralTimeRemaining = remainingMinerals / (currentWorkers * DATA.mpwpf);
		
			//printf("%lf remaining, %lf frames, %lf\n", remainingMinerals, mineralTimeRemaining, (mineralTimeRemaining - DATA[DATA.getWorker()].buildTime()) * DATA.mpwpf);
		
			// if we were to build another worker, would it make its resources back?
			if ( (mineralTimeRemaining - DATA[DATA.getWorker()].buildTime()) * DATA.mpwpf >= DATA[DATA.getWorker()].mineralPrice() )
			{		
				currentMinerals += currentWorkers * DATA.mpwpf * DATA[DATA.getWorker()].buildTime() - DATA[DATA.getWorker()].mineralPrice();
				currentWorkers++;
				mineralTimeElapsed += DATA[DATA.getWorker()].buildTime();
			}
			else
			{
				break;
			}
		}
	
		remainingMinerals = goalMinerals - currentMinerals;
		if (remainingMinerals >= 0)
		{
			mineralTimeRemaining = remainingMinerals / (currentWorkers * DATA.mpwpf);
			mineralTimeElapsed += mineralTimeRemaining;
		}
		
		//if (goalMinerals) printf("%d minerals gathered in %lf frames with %d workers.\n", goalMinerals, mineralTimeElapsed, (int)currentWorkers);
		
		double gasRemaining = goalGas - currentGas;
		double gasTimeRemaining = gasRemaining / (3 * DATA.gpwpf);
		if (goalGas && !numUnits[DATA.getRefinery()] && !progress[DATA.getRefinery()])
		{
			gasTimeRemaining += DATA[DATA.getRefinery()].buildTime();
		}
		
		//if (goalGas) printf("%d gas gathered in %lf frames with %d workers.\n", goalGas, gasTimeRemaining, 3);
		
		int workers = (int)(currentWorkers + (goalGas ? 3 : 0));
		
		return std::pair<FrameCountType, FrameCountType>((int)ceil(currentFrame + (gasTimeRemaining > mineralTimeElapsed ? gasTimeRemaining : mineralTimeElapsed)), workers);
	}
	
	unsigned int hashAllUnits(const int h) const
	{
		int hash = 0;
		
		for (Action a(0); a<DATA.size(); ++a)
		{
		    hash ^= DATA.getHashValue(a, getNumUnits(a), h);
		}
		
		return hash;
	}
	
	unsigned int hashCompletedUnits(const int h) const
	{
		int hash = 0;
		
		for (Action a(0); a<DATA.size(); ++a)
		{
		    hash ^= DATA.getHashValue(a, numUnits[a], h);
		}
		
		return hash;
	}
	
	/**********************************************************************************
	*
	*                            Action Legality Functions
	*
	**********************************************************************************/

	ActionSet getLegalActions(const StarcraftSearchGoal & goal) const
	{
		// initially empty bitmask
		ActionSet legal;

		// for each possible action, check for legality
		for (Action i = 0; i < DATA.size(); ++i) 
		{
			// if we have the prerequisite units
			if (isLegal(i, goal)) 
			{
				// set the bitmask bit to a 1
				legal.add(i);
			}
		}

		// return the bitmask
		return legal;
	}
	
	ActionSet getAllLegalActions() const
	{
		ActionSet legal;

		for (Action i = 0; i < DATA.size(); ++i) 
		{
			// if we have the prerequisite units
			if (isLegal(i)) 
			{
				// set the bitmask bit to a 1
				legal.add(i);
			}
		}

		return legal;
	}

	ActionSet getLegalActionsMonteCarlo(const StarcraftSearchGoal & goal) const
	{
		// initially empty bitmask
		ActionSet legal;

		// for each possible action, check for legality
		for (Action i = 0; i < DATA.size(); ++i) 
		{
			// if we have the prerequisite units
			if (isLegalMonteCarlo(i, goal)) 
			{
				// set the bitmask bit to a 1
				legal.add(i);
			}
		}

		// return the bitmask
		return legal;
	}

    bool isLegalMonteCarlo(const Action a, const StarcraftSearchGoal & goal) const
	{
		if (currentSupply > maxSupply)
		{
			printData();
		}
	
		// check if the tech requirements are met
		if (!hasRequirements(DATA[a].getPrerequisites())) 
		{
			#ifdef DEBUG_LEGAL
				printf("PREREQ %d\n", a);
			#endif
			return false;
		}
		
		// if we have more than the max we want, return false
		if (goal.getMax(a) && (getNumUnits(a) >= goal.getMax(a)))
		{
			#ifdef DEBUG_LEGAL
				printf("HAVE MAX ALREADY %d\n", a);
			#endif
			return false;
		}

		// if it's a unit and we are out of supply and aren't making supply
		int supplyInProgress = progress[DATA.getSupplyProvider()]*DATA[DATA.getSupplyProvider()].supplyProvided() +
							   progress[DATA.getResourceDepot()]*DATA[DATA.getResourceDepot()].supplyProvided();
		if ( (currentSupply + DATA[a].supplyRequired()) > maxSupply + supplyInProgress) 
		{
			#ifdef DEBUG_LEGAL
				printf("NO SUPPLY %d\n", a);
			#endif
			
			return false;
		}

		// specific rule for never leaving 0 workers on minerals
		if (DATA[a].isRefinery() && (getNumUnits(DATA.getWorker()) <= 1 + 3*(getNumUnits(DATA.getRefinery()) + 1))) 
		{		
			#ifdef DEBUG_LEGAL
				printf("REFINERY 1 %d\n", a);
			#endif
			return false;
		}

		// if it's a new building and no drones are available, it's not legal
		if (DATA[a].isBuilding() && (mineralWorkers <= 1)) 
		{
			#ifdef DEBUG_LEGAL
				printf("NO MINERAL WORKERS %d\n", a);
			#endif
			return false;
		}
		
		// we can't build a building with our last worker
		if (DATA[a].isBuilding() && (mineralWorkers <= 1))
		{
			#ifdef DEBUG_LEGAL
				printf("NO MINERAL WORKERS - (%d, %d) %d\n", mineralWorkers, getNumUnits(DATA.getRefinery()), a);
			#endif
			return false;
		}

		// if we have no gas income we can't make a gas unit
		bool noGas = (gasWorkers == 0) && (getNumUnits(DATA.getRefinery()) == 0);
		if (((DATA[a].gasPrice() - gas) > 0) && noGas) 
		{ 
			#ifdef DEBUG_LEGAL
				printf("NO GAS %d\n", a);
			#endif
			return false; 
		}

		// if we have no mineral income we'll never have a minerla unit
		bool noMoney = (mineralWorkers == 0) && (progress.numInProgress(DATA.getWorker()) == 0);
		if (((DATA[a].mineralPrice() - minerals) > 0) && noMoney) 
		{ 
			#ifdef DEBUG_LEGAL
				printf("NO MONEY %d\n", a);
			#endif
			return false; 
		}

		// don't build more refineries than resource depots
		if (DATA[a].isRefinery() && (getNumUnits(DATA.getRefinery()) >= getNumUnits(DATA.getResourceDepot())))
		{
			#ifdef DEBUG_LEGAL
				printf("NOT ENOUGH DEPOTS FOR REFINERY %d", a);
			#endif
			return false;
		}

		// we don't need to go over the maximum supply limit with supply providers
		if (DATA[a].isSupplyProvider() && (maxSupply + getSupplyInProgress() >= 400))
		{
			#ifdef DEBUG_LEGAL
				printf("TOO MUCH SUPPLY FOR PROVIDER %d\n", a);
			#endif
			return false;
		}

		#ifdef DEBUG_LEGAL
			printf("Action Legal %d\n", a);
		#endif
		return true;
	}

	bool isLegal(const Action a) const
	{
		// check if the tech requirements are met
		if (!hasRequirements(DATA[a].getPrerequisites())) 
		{
			return false;
		}

		// if it's a unit and we are out of supply and aren't making an overlord, it's not legal
		int supplyInProgress = progress[DATA.getSupplyProvider()]*DATA[DATA.getSupplyProvider()].supplyProvided() +
							   progress[DATA.getResourceDepot()]*DATA[DATA.getResourceDepot()].supplyProvided();
		if ( (currentSupply + DATA[a].supplyRequired()) > maxSupply + supplyInProgress) 
		{
			return false;
		}

		// specific rule for never leaving 0 workers on minerals
		if (DATA[a].isRefinery() && (mineralWorkers <= 4 + 3*getNumUnits(DATA.getRefinery()))) 
		{		
			return false;
		}

		// if it's a new building and no drones are available, it's not legal
		if (DATA[a].isBuilding() && (mineralWorkers <= 1)) 
		{
			return false;
		}
		
		// we can't build a building with our last worker
		if (DATA[a].isBuilding() && (mineralWorkers <= 1 + 3*getNumUnits(DATA.getRefinery())))
		{
			return false;
		}

		// if we have no gas income we can't make a gas unit
		bool noGas = (gasWorkers == 0) && (getNumUnits(DATA.getRefinery()) == 0);
		if (((DATA[a].gasPrice() - gas) > 0) && noGas) 
		{ 
			return false; 
		}

		// if we have no mineral income we'll never have a minerla unit
		bool noMoney = (mineralWorkers == 0) && (progress.numInProgress(DATA.getWorker()) == 0);
		if (((DATA[a].mineralPrice() - minerals) > 0) && noMoney) 
		{ 
			return false; 
		}

		// don't build more refineries than resource depots
		if (DATA[a].isRefinery() && (getNumUnits(DATA.getRefinery()) >= getNumUnits(DATA.getResourceDepot())))
		{
			return false;
		}

		// we don't need to go over the maximum supply limit with supply providers
		if (DATA[a].isSupplyProvider() && (maxSupply + getSupplyInProgress() >= 400))
		{
			return false;
		}

		if (DATA[a].isResourceDepot())
		{
			return false;
		}

		if (DATA[a].isRefinery())
		{
			return false;
		}

		return true;
	}

	// given the goal, is this action legal
	bool isLegal(const Action a, const StarcraftSearchGoal & goal) const
	{
		if (currentSupply > maxSupply)
		{
			printData();
		}
	
		if (!goal.get(a) && !goal.getMax(a))
		{
			#ifdef DEBUG_LEGAL
				printf("NONE IN GOAL %d\n", a);
			#endif
			return false;
		}
	
		// if we have enough of this thing don't make any more
		if (goal.get(a) && (getNumUnits(a) >= goal.get(a)))
		{
			#ifdef DEBUG_LEGAL
				printf("HAVE ALREADY %d\n", a);
			#endif
			return false;
		}
		
		// if we have more than the max we want, return false
		if (goal.getMax(a) && (getNumUnits(a) >= goal.getMax(a)))
		{
			#ifdef DEBUG_LEGAL
				printf("HAVE MAX ALREADY %d\n", a);
			#endif
			return false;
		}
	
		// check if the tech requirements are met
		if (!hasRequirements(DATA[a].getPrerequisites())) 
		{
			#ifdef DEBUG_LEGAL
				printf("PREREQ %d\n", a);
			#endif
			return false;
		}

		// if it's a unit and we are out of supply and aren't making an overlord, it's not legal
		int supplyInProgress = progress[DATA.getSupplyProvider()]*DATA[DATA.getSupplyProvider()].supplyProvided() +
							   progress[DATA.getResourceDepot()]*DATA[DATA.getResourceDepot()].supplyProvided();
		if ( (currentSupply + DATA[a].supplyRequired()) > maxSupply + supplyInProgress) 
		{
			#ifdef DEBUG_LEGAL
				printf("NO SUPPLY %d\n", a);
			#endif
			//printData();
			return false;
		}

		// specific rule for never leaving 0 workers on minerals
		if (DATA[a].isRefinery() && (mineralWorkers <= 4 + 3*getNumUnits(DATA.getRefinery()))) 
		{		
			#ifdef DEBUG_LEGAL
				printf("REFINERY 1 %d\n", a);
			#endif
			return false;
		}

		// if it's a new building and no drones are available, it's not legal
		if (DATA[a].isBuilding() && (mineralWorkers <= 1)) 
		{
			#ifdef DEBUG_LEGAL
				printf("NO MINERAL WORKERS %d\n", a);
			#endif
			return false;
		}
		
		// we can't build a building with our last worker
		if (DATA[a].isBuilding() && (mineralWorkers <= 1 + 3*getNumUnits(DATA.getRefinery())))
		{
			#ifdef DEBUG_LEGAL
				printf("NO MINERAL WORKERS - (%d, %d) %d\n", mineralWorkers, getNumUnits(DATA.getRefinery()), a);
			#endif
			return false;
		}

		// if we have no gas income we can't make a gas unit
		bool noGas = (gasWorkers == 0) && (getNumUnits(DATA.getRefinery()) == 0);
		if (((DATA[a].gasPrice() - gas) > 0) && noGas) 
		{ 
			#ifdef DEBUG_LEGAL
				printf("NO GAS %d\n", a);
			#endif
			return false; 
		}

		// if we have no mineral income we'll never have a minerla unit
		bool noMoney = (mineralWorkers == 0) && (progress.numInProgress(DATA.getWorker()) == 0);
		if (((DATA[a].mineralPrice() - minerals) > 0) && noMoney) 
		{ 
			#ifdef DEBUG_LEGAL
				printf("NO MONEY %d\n", a);
			#endif
			return false; 
		}

		// don't build more refineries than resource depots
		if (DATA[a].isRefinery() && (getNumUnits(DATA.getRefinery()) >= getNumUnits(DATA.getResourceDepot())))
		{
			#ifdef DEBUG_LEGAL
				printf("NOT ENOUGH DEPOTS FOR REFINERY %d", a);
			#endif
			return false;
		}

		// we don't need to go over the maximum supply limit with supply providers
		if (DATA[a].isSupplyProvider() && (maxSupply + getSupplyInProgress() >= 400))
		{
			#ifdef DEBUG_LEGAL
				printf("TOO MUCH SUPPLY FOR PROVIDER %d\n", a);
			#endif
			return false;
		}

		#ifdef DEBUG_LEGAL
			printf("Action Legal %d\n", a);
		#endif
		return true;
	}
	

	// does the current state meet the goal requirements
	bool meetsGoal(const StarcraftSearchGoal & goal) const
	{ 	
		// for each unit in the array
		for (Action i=0; i<DATA.size(); ++i) 
		{
			// if we don't have enough of them, no good
			if (getNumUnits(i) < goal.get(i)) 
			{
				return false;
			}
		}

		// otherwise it's fine
		return true;
	}

	bool meetsGoalCompleted(const StarcraftSearchGoal & goal) const
	{ 
		if (minerals < goal.getMineralGoal() || gas < goal.getGasGoal())
		{
			return false;
		}
	
		// for each unit in the array
		for (Action i=0; i<MAX_ACTIONS; ++i) 
		{
			// if we don't have enough of them, no good
			if (numUnits[i] < goal.get(i))
			{
				return false;
			}
		}

		// otherwise it's fine
		return true;
	}
	
	bool meetsConstraints(const StarcraftSearchConstraints & ssc) const
	{
		for (int i=0; i<ssc.size(); ++i)
		{
			StarcraftSearchConstraint & c = ssc.getConstraint(i);
		
			if (currentFrame > c.frame && numUnits[c.action] < c.actionCount)
			{
				return false;
			}
		}
		
		return true;
	}
	
	/**********************************************************************************
	*
	*                                Getter + Setter Functions
	*
	**********************************************************************************/

	int getCurrentFrame() const
	{
		return currentFrame;
	}
	
	ActionSet getCompletedUnitSet() const
	{
		return completedUnitSet;
	}
	
	ActionSet getProgressUnitSet() const
	{
		return progressUnitSet;
	}
	
	Action getActionPerformed() const
	{
		return actionPerformed;
	}
	
	void setParent(StarcraftState * p)
	{
		parent = p;
	}
	
	StarcraftState * getParent() const
	{
		return parent;
	}
	
	ActionSet getUnitsCompleteAndProgress() const
	{
		return completedUnitSet + progressUnitSet;
	}
	
	int getLastFinishTime() const
	{
		return progress.getLastFinishTime();
	}
	
	int getWorkerCount() const
	{
		return getNumUnits(DATA.getWorker());
	}
	
	int getNumResourceDepots() const
	{
		return getNumUnits(DATA.getResourceDepot());
	}

	void addHatchery(UnitCountType numLarva, FrameCountType timeLastSpawn)
	{
		hatcheries[numHatcheries++] = Hatchery(numLarva, timeLastSpawn);
	}

	void setNumLarva(UnitCountType n)
	{
		numLarva = n;
	}

	// sets a bitmask of units we have based on the unit count array
	void setUnitMask() 
	{
		// reset the bits
		completedUnitSet = 0;

		// for each possible action
		for (Action a(0); a<DATA.size(); ++a) 
		{
			// if we have more than 0 of the unit, set the bit to 1
			if (numUnits[a] > 0)
			{
				completedUnitSet.add(a);

				// special cases for zerg trickle down prerequisites
				if (DATA.getRace() == BWAPI::Races::Zerg)
				{
					// if it's a Lair, remove the hatchery
					if (DATA[a].isBuilding() && (DATA[a].getUnitType() == BWAPI::UnitTypes::Zerg_Lair))
					{
						completedUnitSet.add(DATA.getAction(BWAPI::UnitTypes::Zerg_Hatchery));
					}

					// if it's a Greater Spire, remove the spire
					//if (DATA[a].isBuilding() && (DATA[a].getUnitType() == BWAPI::UnitTypes::Zerg_Greater_Spire))
					//{
					//	completedUnitSet.add(DATA.getAction(BWAPI::UnitTypes::Zerg_Spire));
					//}
				}
			}
		}
	}

	// sets the progress mask based on the progress vector
	inline void setProgressMask() 
	{
		// reset the bits
		progressUnitSet = ActionSet(__ZERO);

		// for each unit in the inProgress vector
		for (int i(0); i<progress.size(); ++i) 
		{
			// set the bit of the progress mask appropriately
			progressUnitSet.add(progress.getAction(i));
		}
	}

	void addBuilding(const Action action, const FrameCountType timeUntilFree)
	{
		buildings.addBuilding(action, timeUntilFree);
	}

	void addActionInProgress(const Action action, const int completionFrame)
	{
		// add it to the actions in progress
		progress.addAction(action, completionFrame);

		// update the progress bitmask
		progressUnitSet.add(action);
	}

	// add a number of a unit type
	void addNumUnits(const Action a, const char d) 	
	{
		// set the unit amount appropriately
		numUnits[a] += d;

		// if the new number is more than zero, set the bit in the unitMask
		if (numUnits[a] > 0) 	
		{ 
			completedUnitSet.add(a);
		}
		// otherwise, clear the bit in the unitMask
		else					
		{ 
			completedUnitSet.subtract(a); 
		}
	}

	// set the number of a particular unit type
	void setNumUnits(const int t, const unsigned char n) 
	{ 
		// set the unit amount appropriately
		numUnits[t] = n; 

		// if the new number is more than zero, set the bit in the unitMask
		if (numUnits[t] > 0) 	
		{ 
			completedUnitSet.add(t); 
		}
		// otherwise, clear the bit in the unitMask
		else
		{ 
			completedUnitSet.subtract(t); 
		}
	}

	void setFrame(const short frame)
	{
		currentFrame = frame;
	}

	void setSupply(const int cSupply, const int mSupply)
	{
		currentSupply = cSupply;
		maxSupply = mSupply;
	}

	void setResources(const int m, const int g)
	{
		minerals = m;
		gas = g;
	}
	
	UnitCountType getNumUnits(const int t) const
	{
		return numUnits[t] + (progress[t] * DATA[t].numProduced());
	}
	
	void setActionPerformedK(const unsigned char k)
	{
		actionPerformedK = k;
	}
	
	int getActionPerformedK() const
	{
		return actionPerformedK;
	}

	const bool dominates(const StarcraftState & s) const
	{
		if (getMinerals() < s.getMinerals())
		{
			return false;
		}

		if (getGas() < s.getGas())
		{
			return false;
		}
		
		for (Action a(0); a<DATA.size(); ++a)
		{
			if (getNumUnits(a) < s.getNumUnits(a))
			{
				return false;
			}
		}
	}

	SupplyCountType getSupplyInProgress() const
	{
		int supplyInProgress =	progress[DATA.getSupplyProvider()]*DATA[DATA.getSupplyProvider()].supplyProvided() +
								progress[DATA.getResourceDepot()]*DATA[DATA.getResourceDepot()].supplyProvided();

		return supplyInProgress;
	}
	
	bool hasEnoughSupplyForGoal(const StarcraftSearchGoal & goal) const
	{
		int goalSupplyNeeded = 0;
		
		for (int a=0; a<DATA.size(); ++a)
		{
			int unitsNeeded = (getNumUnits(a)) - goal[a];
			if (unitsNeeded > 0)
			{
				goalSupplyNeeded += DATA[a].supplyRequired();
			}
		}
		
		return (maxSupply - currentSupply) >= goalSupplyNeeded;
	}
	
	bool meetsActionGoal(const StarcraftSearchGoal & goal, const Action a) const
	{
		return getNumUnits(a) >= goal[a];
	}

	// determines if an action has prerequisites met via bitmask operations
	bool hasRequirements(const ActionSet required) const 
	{
		return (completedUnitSet + progressUnitSet).contains(required); 
	}
	
#ifdef EXTRA_STARCRAFTSTATE_STATISTICS
	
	std::vector< std::pair< FrameCountType, ResourceCountType > > getArmyIntegral()
	{
		return armyIntegral;
	}
	
#endif

	// setters
	void setMineralWorkers	(const unsigned char mw)	{ mineralWorkers	= mw; }
	void setGasWorkers		(const unsigned char gw)	{ gasWorkers		= gw; }
	void addMineralWorkers	(const int d)				{ mineralWorkers	= mineralWorkers+(char)d; }
	void addGasWorkers		(const int d)				{ gasWorkers		= gasWorkers+(char)d;}

	// getter methods for the internal variables
	double 					getMineralsPerFrame()		const	{ return DATA.mpwpf * mineralWorkers; }
	double 					getGasPerFrame()	        const   { return DATA.gpwpf * gasWorkers; } 
	SupplyCountType 		getMaxSupply()				const	{ return maxSupply; }						
	SupplyCountType 		getCurrentSupply()			const	{ return currentSupply; }						
	UnitCountType 			getMineralWorkers()	        const   { return mineralWorkers; }					
	UnitCountType 			getGasWorkers()		        const   { return gasWorkers; }						
	ResourceCountType 		getMinerals() 				const	{ return minerals; }	
	ResourceCountType		getGas()					const	{ return gas; }	
	
	ResourceCountType getMinerals(const int frame) const
	{
		assert(frame >= currentFrame);
	
		return minerals + (int)(getMineralsPerFrame() * (frame-currentFrame));
	}
	
	ResourceCountType getGas(const int frame) const
	{
		assert(frame >= currentFrame);
	
		return gas + (int)(getGasPerFrame() * (frame-currentFrame));
	}
	
	ResourceCountType getFinishTimeMinerals() const
	{
		return getMinerals(getLastFinishTime());
	}
	
	ResourceCountType getFinishTimeGas() const
	{
		return getGas(getLastFinishTime());
	}
	
	ResourceCountType getArmyValue() const
	{
	    int value(0);
	    
	    for (Action a(0); a<DATA.size(); ++a)
	    {
	        if (!DATA[a].isBuilding() && !DATA[a].isWorker() && !DATA[a].isSupplyProvider())
	        {
	            value += getNumUnits(a)*DATA[a].mineralPrice();
	            value += getNumUnits(a)*2*DATA[a].gasPrice();
	        }
	    }
	    
	    return value;
	}
	
	ResourceCountType getCurrentArmyValue() const
	{
		return armyValue;
	}
	
	ResourceCountType getFinishArmyValue() const
	{
		FrameCountType elapsed(getLastFinishTime() - lastActionFrame);
		
		return armyValue + (elapsed * getArmyValue());
	}

	std::vector<Action> getBuildOrder()
	{
		std::vector<Action> buildOrder;
		
		StarcraftState * s = this;
		while (s->getParent() != NULL)
		{
			for (int i=0; i<s->getActionPerformedK(); ++i)
			{
				buildOrder.push_back(s->getActionPerformed());
			}
			s = s->getParent();
		}
		
		return buildOrder;
	}

	void printData() const
	{
		printf("\n-----------------------------------------------------------\n");
		printf("TEST %d\n", minerals);

		printf("Current Frame: %d (%dm %ds)\n\n", currentFrame, (currentFrame / 24) / 60, (currentFrame / 24) % 60);

		printf("Completed:\n\n");
		for (int i=0; i<MAX_ACTIONS; i++) if (numUnits[i] > 0) {
			printf("\t%d\t%s\n", numUnits[i], DATA[i].getName().c_str());
		}

		printf("\nIn Progress:\n\n");
		for (int i(0); i<progress.size(); i++) {
			printf("\t%d\t%s\n", progress.getTime(i), DATA[progress.getAction(i)].getName().c_str());
		}

		//printf("\nLegal Actions:\n\n");
		//LOOP_BITS (getLegalActions(), action) {
		//
		//	printf("\t%s\n", data->action_strings[action].c_str());
		//}

		printf("\n%6d Minerals\n%6d Gas\n%6d Max Supply\n%6d Current Supply\n", minerals, gas, maxSupply, currentSupply);
		printf("\n%6d Mineral Drones\n%6d Gas Drones\n", mineralWorkers, gasWorkers);
		printf("\n-----------------------------------------------------------\n");
		//printPath();
	}
};
}


#endif
