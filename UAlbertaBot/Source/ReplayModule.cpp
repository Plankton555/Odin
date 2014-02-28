#include "ReplayModule.h"
#include <iostream>
#include <fstream>

using namespace BWAPI;
using namespace std;

ReplayModule::ReplayModule()  {}
ReplayModule::~ReplayModule() {}

void ReplayModule::onStart()
{
	//Print Hello
	string filename = Broodwar->mapFileName();
	string pathname = Broodwar->mapPathName();
	Broodwar->printf("Hello, my name is Odin! This is a replay.");
	Broodwar->printf("Replay: %s \n", filename.c_str());
	Broodwar->printf("Location: %s \n", pathname.c_str());

	//Check if this replay was checked already
	string folder;
	folder = pathname.substr(0, pathname.size()-filename.size());
	string line;
	ifstream myfile ((folder + "seen.txt").c_str());
	if (myfile.is_open()) {
		while (getline(myfile,line)) {
			if (line.compare(Broodwar->mapFileName()) == 0) {
				Broodwar->printf("This replay has already been seen.");
				Broodwar->leaveGame();
			}
		}
		myfile.close();
	} else {
		Broodwar->printf("Unable to open file.");
	}

	//Set this replay as checked if it wasn't already
	ofstream myfile2 ((folder + "seen.txt").c_str(), ios::app);
	if (myfile2.is_open())
	{
		myfile2 << Broodwar->mapFileName().c_str() << endl;
		myfile2.close();
	} else {
		Broodwar->printf("Unable to save file.");
	}
}

void ReplayModule::onFrame()
{
}