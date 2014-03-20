#include "DataModule.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>

#define FUZZY_VALUES_START (1)
#define FUZZY_VALUES_END (5)
#define NR_FUZZY_VALUES  (FUZZY_VALUES_END - FUZZY_VALUES_START)
#define FUZZY_VALUES_FILEPATH ("odin/fuzzy_units.txt")
#define SPLIT_SYMBOL (",")
#define COMMENT_CHAR (';')

using namespace std;

std::map<const char*,int*> *DataModule::units = NULL;
int DataModule::loaded = 0;

void DataModule::init()
{
	if (loaded == 0)
	{
		std::string line;
		int nrFiles = 0;
		std::ifstream unitfile (FUZZY_VALUES_FILEPATH);
		if (unitfile.is_open())
		{
			units = new std::map<const char*, int*>;

			while (getline(unitfile,line)) {
				if (*line.c_str() != COMMENT_CHAR) //Ignore comments
				{
					//split the line
					std::vector<std::string>* sub = splitDelim(line, SPLIT_SYMBOL);

					//Save fuzzy values
					int* fuzzyValues = new int[NR_FUZZY_VALUES];
					for (int i = 0; i < NR_FUZZY_VALUES; i++)
					{
						fuzzyValues[i] = atoi(sub->at(FUZZY_VALUES_START + i).c_str());
					}
					(*units)[sub->at(0).c_str()] = fuzzyValues;
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
		std::map<const char*,int*>::iterator it;
		for(it=units->begin(); it!=units->end(); it++)
		{
			delete (it->first); //Delete each string
			delete[] (it->second); //delete each int array
		}
		delete units; //delete the map itself
	}
	loaded = 0;
}

int DataModule::getNrFuzzyValues()
{
	if (!loaded)
	{
		return -2;
	}

	return NR_FUZZY_VALUES;
}

std::map<const char*,int*>* DataModule::getFuzzyValues()
{
	return units;
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