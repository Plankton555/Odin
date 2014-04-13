#include "FuzzyModule.h"
#include "DataModule.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>


int FuzzyModule::getFuzzyNr(int nr, std::string name)
{
	if (!DataModule::loaded)
	{
		return -2;
	}

	std::map<std::string,std::vector<int>*>* units = DataModule::getFuzzyValues();
	std::map<std::string,std::vector<int>*>::iterator it;
	for(it=units->begin(); it!=units->end(); it++)
	{
		if (strcmp(it->first.c_str(), name.c_str()) == 0)
		{
			int nrFuzzyValues = it->second->size();
			for (int i = 0; i<nrFuzzyValues; i++)
			{
				if (nr <= it->second->at(i))
				{
					return i;
				}
			}
			//More than the highest value
			return nrFuzzyValues;
		}
	}

	return -1;
}
