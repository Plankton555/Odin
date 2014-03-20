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

	std::map<const char*,int*>* units = DataModule::getFuzzyValues();
	std::map<const char*,int*>::iterator it;
	int nrFuzzyValues = DataModule::getNrFuzzyValues();
	for(it=units->begin(); it!=units->end(); it++)
	{
		if (strcmp(it->first, name.c_str()) == 0)
		{
			for (int i = 0; i<nrFuzzyValues; i++)
			{
				if (nr <= *(it->second+i))
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
