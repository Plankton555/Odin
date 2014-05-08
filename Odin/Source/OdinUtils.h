#pragma once

#include "Common.h"
#include "ReplayModule.h"
#include <boost/algorithm/string.hpp>

#define ODIN_DEBUG true

using namespace std;

class OdinUtils
{
public:
	bool updateID;
	int gameID;
	OdinUtils::OdinUtils();
	OdinUtils::~OdinUtils();

	static OdinUtils &Instance();

	int predictTimePeriodsAhead;
};

namespace odin_utils
{
	//this one only return output file for data mining, NOT REPLAYS.
	//call getBNOutputFile(int ID) if REPLAY
	inline std::string getOutputFile(int ID, int timePeriodsAhead)
	{
		std::ostringstream basePath;
		basePath << "bwapi-data/Odin/odin_data/BNlog/game/";
		if (timePeriodsAhead >= 0)
		{
			basePath << timePeriodsAhead << "/";
		}

		std::ostringstream stringStream;
		stringStream << basePath.str() << ID << ".txt";
		return stringStream.str();
	}

	inline std::string getBNOutputFile(int ID)
	{
		if (BWAPI::Broodwar->isReplay())
		{
			std::string path = "bwapi-data/Odin/odin_data/BNlog/replay/";
			std::ostringstream stringStream;
			stringStream << path << ID << ".txt";
			return stringStream.str();
		}
		return getOutputFile(ID, 0);
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
		replaceString(str, "Protoss", "");
		replaceString(str, "Terran", "");
		replaceString(str, "Zerg", "");
		replaceAllString(str," ",""); //Remove all spaces
		replaceAllString(str,"-",""); //remove '-' for u-238 shells
		replaceAllString(str,"SiegeMode",""); //Siege tanks have two modes
		replaceAllString(str,"TankMode","");
	}

	inline int getTimePeriod()
	{
		return std::min(MAX_TIME_PERIODS, BWAPI::Broodwar->getFrameCount()/1000);
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

	inline void logBN(std::string filename, int number, bool doubleDigit)
	{
		if (doubleDigit && number < 10)
		{
			logBN(filename, 0);
		}

		logBN(filename, number);
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
			return OdinUtils::Instance().gameID;
		}
		else
		{
			if (!OdinUtils::Instance().updateID) { return OdinUtils::Instance().gameID; }

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

			OdinUtils::Instance().gameID = atoi(id.c_str());
			OdinUtils::Instance().updateID = false;
			return OdinUtils::Instance().gameID;
		}

		return -1;
	}

	inline void increaseID()
	{
		if (BWAPI::Broodwar->isReplay())
		{
			std::string filename = BWAPI::Broodwar->mapFileName();
			std::vector<std::string> gameID;
			boost::split(gameID, filename, boost::is_any_of("\t .[_]"));
			OdinUtils::Instance().gameID = atoi(gameID[0].c_str());
			return;
		}

		int id = getID() + 1;
		std::string filename = "bwapi-data/Odin/odin_data/id.txt";
		std::ofstream write (filename.c_str(), ios::out);
		if (write.is_open())
		{
			write << id << endl;
			write.close();
		}

		OdinUtils::Instance().updateID = true;
	}
};