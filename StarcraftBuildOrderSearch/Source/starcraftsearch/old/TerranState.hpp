#ifndef TERRAN_STATE_H
#define TERRAN_STATE_H

#include <string.h>
#include <queue>
#include <algorithm>

#include "BWAPI.h"
#include "ActionSet.hpp"
#include "StarcraftAction.hpp"
#include "StarcraftData.hpp"
#include "StarcraftState.hpp"


class TerranState : public StarcraftState
{

public:

	TerranState(bool init)
	{
		printf("Terran State Created\n");

		// only call this for 'new' StarcraftStates, others will have bitmasks set on add
		if (init) 
		{
			actionPerformed = 255; 
			parent = NULL; 
			currentFrame = 0;
			mineralWorkers = 0; 
			gasWorkers = 0; 
			
			for (int i=0; i<MAX_ACTIONS; ++i)
			{
				numUnits[i] = 0;
			}
			
			setGameStartData();
			setUnitMask(); 
			setProgressMask();
		}
	}

	TerranState()
	{
		actionPerformed = 255; 
		parent = NULL;

		for (int i=0; i<MAX_ACTIONS; ++i)
		{
			numUnits[i] = 0;
		}
	}

	~TerranState() {}

	virtual bool specificIsLegal(Action a)
	{
		if (TERRAN_DEBUG) printf("\tSpecific Is Legal %uc\n", a);

		return true;
	}

	virtual void setGameStartData()
	{
		// starting resources and supply
		minerals = 50; 
		gas = 0; 
		maxSupply = 18; 
		currentSupply = 8;

		buildings.addBuilding(DATA.getAction(BWAPI::UnitTypes::Terran_Command_Center));

		// starting units
		setNumUnits(DATA.getAction(BWAPI::UnitTypes::Terran_SCV), 4);
		setNumUnits(DATA.getAction(BWAPI::UnitTypes::Terran_Command_Center), 1);

		// set the workers to minerals
		mineralWorkers = 4;
	}

	virtual int specificWhenReady(Action action)
	{
		if (TERRAN_DEBUG) printf("\tSpecific When Ready %uc\n", action);
		
		if (TERRAN_DEBUG) printf("\tSpecific When Ready End (return %d)\n", 0);
		
		return 0;
	}

	virtual void specificDoAction(const Action action, const int ffTime)
	{
		if (TERRAN_DEBUG) printf("\tSpecific Do Action %d\n", ffTime);

		// if it's a building, take a worker off minerals to build it
		if (DATA[action].isBuilding())
		{
			mineralWorkers--;
		}

		if (TERRAN_DEBUG) printf("\tSpecific Do Action End\n");
	}

	virtual void specificFastForward(const int toFrame)
	{
		if (TERRAN_DEBUG) printf("\t\tSpecific Fast Forward %d\n", toFrame);

		if (TERRAN_DEBUG) printf("\t\tSpecific Fast Forward End\n");
	}

	virtual void specificFinishAction(const Action action) 
	{
		if (TERRAN_DEBUG) printf("\tSpecific Finish Action\n");
		
		

		if (TERRAN_DEBUG) printf("\tSpecific Finish Action End\n");
	}
	
	void save(std::string filename) const
	{
		std::ofstream outfile(filename.c_str(), std::ofstream::binary);
		outfile.write((char *)this, sizeof(*this));
		outfile.close();
	}
	
	void load(std::string filename)
	{
		std::ifstream infile(filename.c_str(), std::ifstream::binary);
		infile.read((char *)this, sizeof(*this));
		infile.close();
	}
};

#endif
