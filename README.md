Odin
====

A BWAPI bot based on [UAlbertaBot](http://code.google.com/p/ualbertabot/), which is licensed under [GPL v3](http://www.gnu.org/licenses/gpl.html). 
Devoted to the wisest of the supreme Norse gods, Odin.

> [Odin](http://www.comicvine.com/odin/4005-3507/) is the All-Father of the Norse gods and ruler of Asgard. [...] [He] is the wisest and by far the most powerful of the Asgardian Gods.


This project is part of a Bachelor's thesis at Chalmers University of Technology, Sweden. 
The version of the thesis can be found at: TBA

Setup
----

Install the following programs:

* [Starcraft: Broodwar (1.16.1)](http://us.blizzard.com/en-us/games/sc/)
* [Visual C++ 2010 Express Edition (or Professional)](http://www.visualstudio.com/downloads/download-visual-studio-vs#d-2010-express)
* [Boost binaries (1.47.0 or later)](http://sourceforge.net/projects/boost/files/boost-binaries/1.55.0/boost_1_55_0-msvc-9.0-32.exe/download)
* [BWAPI version 3.7.4](http://code.google.com/p/bwapi/downloads/list)

BWAPI will come packed with a hack loader (Chaoslauncher) which you will use later.

The next step is to specify two [system environment variables](http://code.google.com/p/ualbertabot/wiki/Instructions#Prerequisites), `%BWAPI_DIR%` and `%BOOST_DIR%`:
* `%BWAPI_DIR%` shall point to the BWAPI-installation 
* `%BOOST_DIR%` shall point to the boost directory.

_IMPORTANT!_ Neither of them can contain spaces in their search path.

Navigate to the `bwapi-data`-folder and fetch the project with `git clone https://github.com/Plankton555/Odin.git`.

Open the project in VS (`Odin/VisualStudio/Odin.sln`) and compile the project [RELEASE]. Then copy the `bwapi.ini`-file from the base-folder in Odin to `bwapi-data` and modify it to [fit your needs](http://code.google.com/p/bwapi/wiki/MenuAutomation). Make extra sure that all file paths are correct.

You should end up with the file structure below (some files/folders omitted).

    |-- Starcraft/
    |   |-- bwapi-data/
    |       |-- BWTA/
    |       |-- Odin/
    |       |-- BWAPI.dll
    |       |-- bwapi.ini
    |       |-- BWAPId.dll
    |   |-- maps/
    |       |-- replays/

You might need to create `BWTA/` and `replays/` yourself.
More about the different parts of Odin, see [`File structure`](README.md#file-structure) further down.

How to run
----

Start the Chaoslauncher program that comes packaged with BWAPI. Check off the appropriate version of BWAPI to be loaded (DEBUG|RELEASE) and click Start.

Start a game. Odin should now be playing. If it is the first time Odin plays a map, it will start with analyzing it. This will take some time, so be patient.


File structure
----

A brief explanation of all the folders and files, and what they contain.

    |-- Starcraft/
    |   |-- bwapi-data/
    |       |-- BWTA/
    |       |-- Odin/
    |           |-- Odin/
    |               |-- Source/
    |               |-- VisualStudio/
    |           |-- odin_data/
    |               |-- accumulated_data/
    |               |-- bayesian_networks/
    |                   |-- protoss.xdsl
    |                   |-- terran.xdsl
    |                   |-- zerg.xdsl
    |               |-- openings/
    |               |-- replaydatastuff/
    |       |-- BWAPI.dll
    |       |-- bwapi.ini
    |       |-- BWAPId.dll
    |   |-- maps/
    |       |-- replays/
    
    
* `BWTA/` - can be omitted, but if present Odin will save all map analyzes here. That means that Odin doesn't have to analyze the same map twice and you can save yourself some time.
* `Odin/Odin/Source/` - all source code
* `Odin/Odin/VisualStudio/` - all project files
* `accumulated_data/` - arbitrary folder. Here we can save the files/data that are used to train our bayesian networks.
* `bayesian_networks/` - the bayesian networks that are used for predictions
    * `/protoss.xdsl` - the network of the protoss-race
    * `/terran.xdsl` - the network of the terran-race
    * `/zerg.xdsl` - the network of the zerg-race 
* `openings/` 
    * `*_strats.txt` - the openings Odin can choose between
    * `*.txt` - Denotes wins and losses of every opening for the particular race. 
    
    `*` denotes the race of the enemy | `Random`, `Protoss`, `Terran` or `Zerg`
* `replaydatastuff/` - a place to store information about replays, such as which replays already have been seen so they don't get parsed twice.
* `maps/replays/` - the place to save replays to (check your `bwapi.ini`)

