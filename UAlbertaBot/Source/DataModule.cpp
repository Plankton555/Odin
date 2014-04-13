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

std::map<const char*,std::vector<int>*> *DataModule::units = NULL;
std::map<const char*,std::vector<BWAPI::UnitType>* > *DataModule::counters = NULL;
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
			units = new std::map<const char*, std::vector<int>*>;
			counters = new std::map<const char*, std::vector<BWAPI::UnitType>* >;

			while (getline(unitfile,line)) {
				if (*line.c_str() != COMMENT_CHAR) //Ignore comments
				{
					//split the line
					std::vector<std::string>* sub = splitDelim(line, SPLIT_SYMBOL);

					//Save fuzzy values
					std::vector<std::string>* readValues = splitDelim(sub->at(FUZZY_VALUES_POSITION), SUB_SPLIT_SYMBOL);
					std::vector<int>* fuzzyValues = new std::vector<int>(readValues->size());
					for (int i = 0; i < readValues->size(); i++)
					{
						fuzzyValues->at(i) = atoi(readValues->at(i).c_str());
					}
					(*units)[sub->at(0).c_str()] = fuzzyValues;

					//Save the counters
					std::vector<std::string>* readCounters = splitDelim(sub->at(COUNTER_NAMES_POSITION), SUB_SPLIT_SYMBOL);
					if (readCounters->size() == 1 && strcmp(readCounters->at(0).c_str(),"None") == 0) //There are no counters
					{
						(*counters)[sub->at(0).c_str()] = NULL;
					} else
					{
						std::vector<BWAPI::UnitType>* counterNames = new std::vector<BWAPI::UnitType>(readCounters->size());

						for (int i = 0; i < readCounters->size(); i++)
						{
							counterNames->at(i) = BWAPI::UnitTypes::getUnitType(readCounters->at(i).c_str());
						}
						(*counters)[sub->at(0).c_str()] = counterNames;
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
		std::map<const char*,std::vector<int>*>::iterator it;
		for(it=units->begin(); it!=units->end(); it++)
		{
			delete (it->first); //Delete each string
			delete (it->second); //delete each int array
		}
		delete units; //delete the map itself
	}
	loaded = 0;
}

std::map<const char*,std::vector<int>*>* DataModule::getFuzzyValues()
{
	return units;
}

std::vector<BWAPI::UnitType> * DataModule::getCounter(std::string unit)
{
	std::map<const char*,std::vector<BWAPI::UnitType>* >::iterator it;
	for (it = counters->begin(); it != counters->end(); it++)
	{
		if (strcmp(it->first, unit.c_str()) == 0)
		{
			return it->second;
		}
	}
	return NULL;
}

std::vector<std::string>* DataModule::splitDelim(const std::string& str, const std::string& delim)
{
	std::string s = str;
	std::vector<std::string> *output = new std::vector<std::string>;
	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delim)) != std::string::npos) {
		token = s.substr(0, pos);
		output->push_back(token);
		s.erase(0, pos + delim.length());
	}
	output->push_back(s);
	return output;
}