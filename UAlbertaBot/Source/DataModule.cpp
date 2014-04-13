#include "DataModule.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "Common.h"

#define FUZZY_VALUES_POSITION (1)
#define COUNTER_NAMES_POSITION (2)
#define NR_FUZZY_VALUES  (FUZZY_VALUES_END - FUZZY_VALUES_START)
const std::string FUZZY_VALUES_FILEPATH = ODIN_DATA_FILEPATH + "fuzzy_units.txt";
const std::string CASES_FILEPATH = ODIN_DATA_FILEPATH + "cases.txt";
const std::string SPLIT_SYMBOL = ",";
const std::string SUB_SPLIT_SYMBOL = ".";
const char COMMENT_CHAR = ';';

using namespace std;

std::map<std::string,std::vector<int>*>* DataModule::units = NULL;
std::map<std::string,std::vector<BWAPI::UnitType>* >* DataModule::counters = NULL;
std::map< std::vector<std::string>,std::vector<std::string> >* DataModule::cases = NULL;
int DataModule::loaded = 0;

void DataModule::init()
{
	if (loaded == 0)
	{
		loadFuzzy();
		loadCase();
		loaded = 1;
	}
}

void DataModule::loadFuzzy()
{
	std::string line;
	std::ifstream unitfile (FUZZY_VALUES_FILEPATH.c_str());
	if (unitfile.is_open())
	{
		units = new std::map<std::string, std::vector<int>*>;
		counters = new std::map<std::string, std::vector<BWAPI::UnitType>* >;

		while (getline(unitfile,line)) {
			if (*line.c_str() != COMMENT_CHAR) //Ignore comments
			{
				//split the line
				std::vector<std::string> sub = splitDelim(line, SPLIT_SYMBOL);

				//Save fuzzy values
				std::vector<std::string> readValues = splitDelim(sub.at(FUZZY_VALUES_POSITION), SUB_SPLIT_SYMBOL);
				std::vector<int>* fuzzyValues = new std::vector<int>(readValues.size());
				for (unsigned int i = 0; i < readValues.size(); i++)
				{
					fuzzyValues->at(i) = atoi(readValues.at(i).c_str());
				}
				(*units)[sub.at(0).c_str()] = fuzzyValues;

				//Save the counters
				std::vector<std::string> readCounters = splitDelim(sub.at(COUNTER_NAMES_POSITION), SUB_SPLIT_SYMBOL);
				if (readCounters.size() == 1 && strcmp(readCounters.at(0).c_str(),"None") == 0) //There are no counters
				{
					(*counters)[sub.at(0).c_str()] = NULL;
				} else
				{
					std::vector<BWAPI::UnitType>* counterNames = new std::vector<BWAPI::UnitType>(readCounters.size());

					for (unsigned int i = 0; i < readCounters.size(); i++)
					{
						counterNames->at(i) = BWAPI::UnitTypes::getUnitType(readCounters.at(i).c_str());
					}
					(*counters)[sub.at(0).c_str()] = counterNames;
				}
			}
		}
		unitfile.close();
		loaded = 1;
	} else
	{
		BWAPI::Broodwar->printf("Unable to open fuzzy file.");
	}
}

void DataModule::loadCase()
{
	std::string line;
	std::ifstream file (CASES_FILEPATH.c_str());
	if (file.is_open())
	{
		cases = new std::map< std::vector<std::string>,std::vector<std::string> >;

		while (getline(file,line)) {
			if (*line.c_str() != COMMENT_CHAR) //Ignore comments
			{
				//split the line
				std::vector<std::string> sub = splitDelim(line, SPLIT_SYMBOL);

				std::vector<std::string> readCases = splitDelim(sub.at(0), SUB_SPLIT_SYMBOL);
				std::vector<std::string> readSolutions = splitDelim(sub.at(1), SUB_SPLIT_SYMBOL);
				(*cases)[readCases] = readSolutions;
			}
		}
		file.close();
	} else
	{
		BWAPI::Broodwar->printf("Unable to open cases file.");
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
	}
	loaded = 0;
}

std::map< std::vector<std::string>,std::vector<std::string> >* DataModule::getCases()
{
	return cases;
}

bool DataModule::saveCases()
{
	if (loaded)
	{
		std::ofstream file (CASES_FILEPATH.c_str());
		if (file.is_open())
		{
			std::map< std::vector<std::string>,std::vector<std::string> >::iterator it;
			for (it = cases->begin(); it != cases->end(); it++)
			{
				for (unsigned int i = 0; i < it->first.size(); i++)
				{
					file << it->first.at(i);
					if (i != it->first.size() -1) file << ".";
				}
				file << ",";
				for (unsigned int i = 0; i < it->second.size(); i++)
				{
					file << it->second.at(i);
					if (i != it->second.size() -1) file << ".";
				}
				file << endl;
			}
			file.close();
			return true;
		}
	}
	return false;
}

std::map<std::string,std::vector<int>*>* DataModule::getFuzzyValues()
{
	return units;
}

std::vector<BWAPI::UnitType> * DataModule::getCounter(std::string unit)
{
	std::map<std::string,std::vector<BWAPI::UnitType>* >::iterator it;
	it = counters->find(unit);
	if (it == counters->end()) return NULL;
	return it->second;
}

std::vector<std::string> DataModule::splitDelim(const std::string& str, const std::string& delim)
{
	std::string s = str;
	std::vector<std::string> output;
	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delim)) != std::string::npos) {
		token = s.substr(0, pos);
		output.push_back(token);
		s.erase(0, pos + delim.length());
	}
	output.push_back(s);
	return output;
}