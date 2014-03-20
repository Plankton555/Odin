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
					int i = 0;
					std::string* unitName;
					int* fuzzyValues = new int[NR_FUZZY_VALUES];

					int n;
					//split the line
					while ((n = line.find(SPLIT_SYMBOL)) != std::string::npos)
					//for int i = 0; i < NR_FUZZY_VALUES; i++)
					{
						std::string value = line.substr(0, n);
						line = line.substr(n+1, line.length());

						//Save the data on different locations depending on what type of data the column is
						if (!i) { //First column is unit name
							unitName = new std::string(value);
						} else if (FUZZY_VALUES_START <= i && i <= FUZZY_VALUES_END)
						{
							fuzzyValues[i-1] = atoi(value.c_str());
						}
						i++;
					}
					fuzzyValues[i-1] = atoi(line.c_str()); //Last column must manually be put in

					//Save the unit
					(*units)[unitName->c_str()] = fuzzyValues;
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
