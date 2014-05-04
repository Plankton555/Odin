#pragma once

#include "Common.h"
#include "ReplayModule.h"

#define ODIN_DEBUG true

using namespace std;

namespace odin_utils
{
	inline std::string setOutputFile(int ID)
	{
		std::ostringstream stringStream;
		stringStream << "bwapi-data/Odin/odin_data/BNlog/";
		stringStream << ID;
		stringStream << ".txt";
		return stringStream.str();
	}

	inline bool replaceString(std::string &str, const std::string &from, const std::string &to)
	{
		int fromPos = str.find(from);
		if (fromPos == std::string::npos)
		{
			return false;
		} else
		{
			str.replace(fromPos, from.length(), to);
			return true;
		}
	}

	inline void replaceAllString(std::string &str, const std::string &from, const std::string &to)
	{
		while (replaceString(str, from, to)) ;
	}

	//In order to use the short name in the BN
	inline void shortenUnitName(std::string &str)
	{
		replaceAllString(str, "Protoss", "");
		replaceAllString(str, "Terran", "");
		replaceAllString(str, "Zerg", "");
		replaceAllString(str," ",""); //Remove all spaces
		replaceAllString(str,"-",""); //remove '-' for u-238 shells
		replaceAllString(str,"SiegeMode",""); //Siege tanks have two modes
		replaceAllString(str,"TankMode","");
	}

	inline int getTimePeriod()
	{
		int timePeriod = BWAPI::Broodwar->getFrameCount()/1000;
		return std::min(25, timePeriod);
	}

	inline void logBN(std::string filename, std::string str)
	{
		if(!ODIN_DEBUG) return;
		
		std::ofstream file (filename.c_str(), ios::app);
		if (file.is_open())
		{
			file << str.c_str();
			file.close();
		}
	}

	inline void logBN(std::string filename, int number)
	{
		std::stringstream intToString;
		intToString << number;
		logBN(filename, intToString.str());
	}

	inline void logBN(std::string filename, double number)
	{
		std::stringstream doubleToString;
		doubleToString << number;
		logBN(filename, doubleToString.str());
	}

	inline void debug(std::string str)
	{
		if(!ODIN_DEBUG) return;

		std::ofstream file ("bwapi-data/Odin/odin_data/debug.txt", ios::app);
		if (file.is_open())
		{
			file << str.c_str() << endl;
			file.close();
		}
	}
	
	inline void debug(std::string str, int i)
	{
		std::ostringstream stringStream;
		stringStream << str;
		stringStream << ": ";
		stringStream << i;
		std::string newStr = stringStream.str();
		debug(newStr);
	}

	inline void debug(std::string str, double d)
	{
		std::ostringstream stringStream;
		stringStream << str;
		stringStream << ": ";
		stringStream << d;
		std::string newStr = stringStream.str();
		debug(newStr);
	}

	inline int getID()
	{
		if (BWAPI::Broodwar->isReplay()) 
		{
			char* buf = 0;
			size_t sz = 4;
			if (_dupenv_s(&buf, &sz, "GAME_ID") == 0)
			{
				int gameID = atoi(buf);
				free(buf);
				return gameID;
			}
		}
		else
		{
			std::string filename = "bwapi-data/Odin/odin_data/id.txt";
			std::ifstream read (filename.c_str(), ios::in);
			std::string id;
			if (read.is_open())
			{
				getline(read, id);
				if (id.empty())
				{
					id = "0";
				}
				read.close();
			}

			return atoi(id.c_str());
		}

		return -1;
	}

	inline void increaseID()
	{
		int id = getID() + 1;
		std::string filename = "bwapi-data/Odin/odin_data/id.txt";
		std::ofstream write (filename.c_str(), ios::out);
		if (write.is_open())
		{
			write << id << endl;
			write.close();
		}
	}
};