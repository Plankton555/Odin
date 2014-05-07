#include "DataModule.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "Common.h"

#define FUZZY_VALUES_POSITION (1)
#define COUNTER_NAMES_POSITION (2)
#define NR_FUZZY_VALUES  (FUZZY_VALUES_END - FUZZY_VALUES_START)
const std::string FUZZY_VALUES_FILEPATH = ODIN_DATA_FILEPATH + "fuzzy_units.txt";
const std::string SPLIT_SYMBOL = ",";
const std::string SUB_SPLIT_SYMBOL = ".";
const char COMMENT_CHAR = ';';

using namespace std;

std::map<std::string,std::vector<int>*> *DataModule::units = NULL;
std::map<std::string,std::vector<BWAPI::UnitType>* > *DataModule::counters = NULL;
int DataModule::loaded = 0;

void DataModule::init()
{
	if (loaded == 0)
	{
		std::string line;
		int nrFiles = 0;
		std::ifstream unitfile (FUZZY_VALUES_FILEPATH.c_str());
		if (unitfile.is_open())
		{
			units = new std::map<std::string, std::vector<int>*>;
			counters = new std::map<std::string, std::vector<BWAPI::UnitType>* >;

			while (getline(unitfile,line)) {
				if (*line.c_str() != COMMENT_CHAR) //Ignore comments
				{
					//split the line
					std::vector<std::string> sub = odin_utils::splitDelim(line, SPLIT_SYMBOL);

					//Save fuzzy values
					std::vector<std::string> readValues = odin_utils::splitDelim(sub.at(FUZZY_VALUES_POSITION), SUB_SPLIT_SYMBOL);
					std::vector<int>* fuzzyValues = new std::vector<int>(readValues.size());
					for (int i = 0; i < readValues.size(); i++)
					{
						fuzzyValues->at(i) = atoi(readValues.at(i).c_str());
					}
					(*units)[sub.at(0)] = fuzzyValues;

					//Save the counters
					std::vector<std::string> readCounters = odin_utils::splitDelim(sub.at(COUNTER_NAMES_POSITION), SUB_SPLIT_SYMBOL);
					if (readCounters.size() == 1 && strcmp(readCounters.at(0).c_str(),"None") == 0) //There are no counters
					{
						(*counters)[sub.at(0).c_str()] = NULL;
					} else
					{
						std::vector<BWAPI::UnitType>* counterNames = new std::vector<BWAPI::UnitType>(readCounters.size());

						for (int i = 0; i < readCounters.size(); i++)
						{
							counterNames->at(i) = BWAPI::UnitTypes::getUnitType(readCounters.at(i).c_str());
						}
						(*counters)[sub.at(0)] = counterNames;
					}
				}
			}


			unitfile.close();
			loaded = 1;
		} else
		{
			BWAPI::Broodwar->printf("Unable to open file.");
		}
	}
}

void DataModule::destroy()
{
	if (loaded == 1)
	{
		std::map<std::string,std::vector<int>*>::iterator it;
		for(it=units->begin(); it!=units->end(); it++)
		{
			delete (it->second); //delete each int array
		}
		delete units; //delete the map itself

		std::map<std::string,std::vector<BWAPI::UnitType>*>::iterator it2;
		for(it2=counters->begin(); it2!=counters->end(); it2++)
		{
			delete (it2->second); //delete each array
		}
		delete counters; //delete the map itself
	}
	loaded = 0;
}

std::map<std::string,std::vector<int>*>* DataModule::getFuzzyValues()
{
	return units;
}

std::vector<BWAPI::UnitType> * DataModule::getCounter(std::string unit)
{
	std::map<std::string,std::vector<BWAPI::UnitType>* >::iterator it = counters->find(unit);
	return it == counters->end() ? NULL : it->second;
}

