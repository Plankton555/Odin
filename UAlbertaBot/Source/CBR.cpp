#include "CBR.h"

void debug(std::string str)
{
	ofstream file ("debug.txt", ios::app);
	if (file.is_open())
	{
		file << str.c_str() << endl;
		file.close();
	}
}

void debug(std::string str, int i)
{
	std::ostringstream stringStream;
	stringStream << str;
	stringStream << ": ";
	stringStream << i;
	std::string newStr = stringStream.str();
	debug(newStr);
}

void CBR::getDecision(std::map<std::string, std::vector<double> > nodes)
{
	//Do some fancy calculations here
	/*
	if (nodes.find("Probe").at(0) > 0.9)
	{
		//Hehe, he has no probes! Easy win!
		return Amazing PhotonCannonRush
	}
	*/

	std::map<std::string, std::vector<double> >::iterator it;
	for (it = nodes.begin(); it != nodes.end(); it++)
	{
		std::string output(it->first);
		for (int i = 0; i < it->second.size(); i++)
		{
			std::stringstream ss;
			ss << it->second.at(i);
			output.append(ss.str());
		}
		debug(output);
	}
}