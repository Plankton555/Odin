#include "FuzzyModule.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>

#define NR_FUZZY_VALUES (4)
#define FUZZY_VALUES_FILEPATH ("odin/fuzzy_units.txt")
#define SPLIT_SYMBOL ("\t")

std::map<const char*,int*> *FuzzyModule::units = NULL;
int FuzzyModule::loaded = 0;

void FuzzyModule::init()
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
				
				if (*line.c_str() != ';') //Ignore comments
				{
					int i = 0;
					//std::string unitName; //TODO: Does this even work? Need to use new? Is when is this destroyed?
					//int data[NR_FUZZY_VALUES]; //TODO: Does this even work? Need to use new? Is when is this destroyed?
					std::string* unitName;
					int* data = new int[NR_FUZZY_VALUES];

					int n;
					//split the line
					while ((n = line.find(SPLIT_SYMBOL)) != std::string::npos)
					//for int i = 0; i < NR_FUZZY_VALUES; i++)
					{
						std::string value = line.substr(0, n);
						line = line.substr(n+1, line.length());

						if (!i) {
							unitName = new std::string(value);
						} else
						{
							data[i-1] = atoi(value.c_str());
						}
						i++;
					}
					data[i-1] = atoi(line.c_str());

					//Save the unit
					(*units)[unitName->c_str()] = data;
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

void FuzzyModule::destroy()
{
	if (loaded == 1)
	{
		std::map<const char*,int*>::iterator it;
		for(it=units->begin(); it!=units->end(); it++)
		{
			delete (it->first); //Delete each string

			//delete each int array
			for (int i = 0; i < NR_FUZZY_VALUES-1; i++) //TODO: Shouldn't be -1? But crashes, try to debug (attach to process)
			{
				delete ((it->second)+i);
			}
		}
		delete units; //delete the map itself
	}
	loaded = 0;
}

int FuzzyModule::getFuzzyNr(int nr, std::string name)
{
	if (!loaded)
	{
		return -2;
	}

	std::map<const char*,int*>::iterator it;
	for(it=units->begin(); it!=units->end(); it++)
	{
		if (strcmp(it->first, name.c_str()) == 0)
		{
			for (int i = 0; i<NR_FUZZY_VALUES; i++)
			{
				if (nr < *(it->second+i))
				{
					return i;
				}
			}
		}
	}
	return -1;
}
