#ifndef STARCRAFT_COMMON_H
#define STARCRAFT_COMMON_H

#include "BWAPI.h"
#include <boost/foreach.hpp>
#include "assert.h"
#include <stdio.h>
#include <math.h>
#include <fstream>

// macro to access StarcraftData singleton
#define DATA 				(BuildOrderSearch::getStarcraftDataInstance())

// maximum number of actions allowed in StarcraftData
#define MAX_ACTIONS 		30

// maximum number of actions in progress allowed
#define MAX_PROGRESS 		30

// maximum number of buildings allowed
#define MAX_BUILDINGS 		70

// building error return code
#define BUILDING_ERROR 		-2

// maximum number of hatcheries allowed
#define MAX_HATCHERIES 		5

// number of frames between zerg larva spawn
#define ZERG_LARVA_TIMER 	336

// number of frames to use for building placement
#define BUILDING_PLACEMENT 	24 * 5

#define MAX_OF_ACTION 		200
#define NUM_HASHES 			2

// define extra StarcraftState variables for statistics
#define EXTRA_STARCRAFTSTATE_STATISTICS

// debug output
static bool GSN_DEBUG 		= false;
static bool PROTOSS_DEBUG 	= false;
static bool TERRAN_DEBUG 	= false;

// type definitions for storing data
typedef		int				ResourceCountType;
typedef 	int				SupplyCountType;
typedef 	int				FrameCountType;
typedef 	unsigned char	WorkerCountType;
typedef 	unsigned char	UnitCountType;

#endif
