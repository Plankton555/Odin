# ------------------ Compilation options ------------------------

# bwdata folder location
BWDATA = ../../../../../cdavid/research/bwdata

# loads math library.  
LIBS = -lm 

# flags for the C compiler:
CXXFLAGS = -Wall -O3 -I${BWDATA} -std=c++0x

# which compiler to use
CC = g++

# --------------------- Code modules ----------------------------

# object files needed by final program
OBJ = test.o StarcraftData.o ${BWDATA}/*.o

# ------------------------ Rules --------------------------------

# rules for final program
test: ${OBJ}
	${CC} -o $@ ${CXXFLAGS} ${OBJ} ${LIBS}

# rules for any other object files
test.o: StarcraftData.hpp StarcraftData.cpp StarcraftAction.hpp ActionSet.hpp StarcraftState.hpp ZergState.hpp ProtossState.hpp TerranState.hpp ActionInProgress.hpp BuildingData.hpp StarcraftSearch.hpp DFBBStarcraftSearch.hpp StarcraftSearchGoal.hpp DependencyGraph.hpp TranspositionTable.hpp SmartStarcraftSearch.hpp StarcraftSearchConstraint.hpp SearchResults.hpp SearchParameters.hpp BuildOrderComparator.hpp SearchSaveState.hpp MacroAction.hpp MacroActionBuilder.hpp TwinSearch.hpp ArmySearch.hpp SmartArmySearch.hpp

${BWDATA}/*.o: ${BWDATA}/*.cpp ${BWDATA}/*.h ${BWDATA}/BWAPI/*.h ${BWDATA}/BWAPI/Client/*.h
