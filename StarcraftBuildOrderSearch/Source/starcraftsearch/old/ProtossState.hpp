#ifndef PROTOSS_STATE_H
#define PROTOSS_STATE_H

#include "StarcraftState.hpp"
#include "SearchParameters.hpp"

class ProtossState : public StarcraftState
{

public:

	ProtossState(bool init) 
	{		
		if (PROTOSS_DEBUG) printf("Protoss State Created\n");

		// only call this for 'new' StarcraftStates, others will have bitmasks set on add
		if (init) 
		{	
			actionPerformed = 255; 
			parent = NULL; 
			currentFrame = 0; 
			currentSupply = 0;
			mineralWorkers = 0; 
			gasWorkers = 0; 
			armyValue = 0;
			lastActionFrame = 0;
			
			for (int i=0; i<MAX_ACTIONS; ++i)
			{
				numUnits[i] = 0;
			}
			
			setGameStartData();
			setUnitMask(); 
			setProgressMask();
		}
	}

	ProtossState()
	{
		actionPerformed = 255; 
		parent = NULL;

		for (int i=0; i<MAX_ACTIONS; ++i)
		{
			numUnits[i] = 0;
		}
	}
	
	ProtossState(const char * filename)
	{
		load(std::string(filename));
	}

	virtual bool specificIsLegal(Action a)
	{	
		if (PROTOSS_DEBUG) printf("\tSpecific Is Legal %uc\n", a);

		return true;
	}
	
	virtual void setGameStartData()
	{
		// starting resources and supply
		minerals = 50; 
		gas = 0; 
		maxSupply = 18; 
		currentSupply = 8;
		
		buildings.addBuilding(DATA.getAction(BWAPI::UnitTypes::Protoss_Nexus));

		// starting units
		setNumUnits(DATA.getAction(BWAPI::UnitTypes::Protoss_Probe), 4);
		setNumUnits(DATA.getAction(BWAPI::UnitTypes::Protoss_Nexus), 1);

		// set the workers to minerals
		mineralWorkers = 4;
	}

	virtual int specificWhenReady(Action action)
	{
		if (PROTOSS_DEBUG) printf("\tSpecific Action ready %uc\n", action);

		return currentFrame;
	}

	virtual void specificDoAction(const Action action, const int ffTime)
	{
		if (PROTOSS_DEBUG) printf("\tSpecific (Protoss) Do Action %uc %d\n", action, ffTime);
		
		if (PROTOSS_DEBUG) printf("\tSpecific Do Action End\n");
	}

	virtual void specificFastForward(const int toFrame)
	{
		if (PROTOSS_DEBUG) printf("\t\tSpecific (Protoss) Fast Forward %d\n", toFrame);
		
		if (PROTOSS_DEBUG) printf("\t\tSpecific Fast Forward End\n");
	}

	virtual void specificFinishAction(const Action action) 
	{
		if (PROTOSS_DEBUG) printf("\t\t\tSpecific (Protoss) Finish Action - %s\n", DATA[action].getName().c_str());
	
		if (PROTOSS_DEBUG) printf("\t\t\tSpecific Finish Action End\n");
	}
	
	void printBuildingInformation()
	{
		buildings.printBuildingInformation();
	}
	
	void save(std::string filename) const
	{
		std::ofstream outfile (filename.c_str(), std::ofstream::binary);
		outfile.write((char *)this, sizeof(*this));
		outfile.close();
	}
	
	void load(std::string filename)
	{
		std::ifstream infile (filename.c_str(), std::ifstream::binary);
		infile.read((char *)this, sizeof(*this));
		infile.close();
	}
};



#endif
