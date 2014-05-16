Odin
====

A BWAPI bot based on [UAlbertaBot](http://code.google.com/p/ualbertabot/), which is licensed under [GPL v3](http://www.gnu.org/licenses/gpl.html). 
Devoted to the wisest of the supreme Norse gods, Odin.

> [Odin](http://www.comicvine.com/odin/4005-3507/) is the All-Father of the Norse gods and ruler of Asgard. [...] [He] is the wisest and by far the most powerful of the Asgardian Gods.


Setup
----

Install the following programs:

    [Starcraft: Broodwar (1.16.1)](http://us.blizzard.com/en-us/games/sc/)
    [Visual C++ 2010 Express Edition (or Professional)](http://www.visualstudio.com/downloads/download-visual-studio-vs#d-2010-express)
    [Boost (1.47.0 or later)](http://sourceforge.net/projects/boost/files/boost-binaries/1.55.0/boost_1_55_0-msvc-9.0-32.exe/download)

Once you have those, download and install version 3.7.4 of BWAPI from http://code.google.com/p/bwapi/downloads/list. It will come packed with a hack loader (Chaoslauncher) and example projects.

The next step is to specify two [system environment variables](http://code.google.com/p/ualbertabot/wiki/Instructions#Prerequisites), '%BWAPI_DIR%' and '%BOOST_DIR%'.
'%BWAPI_DIR%' shall point to the BWAPI-installation 
'%BOOST_DIR%' shall point to the boost directory.

IMPORTANT! Neither of them can contain a space in their search path.

Navigate to the 'bwapi-data'-folder and fetch the project with 'git clone https://github.com/Plankton555/Odin.git'.
Open the project in VS and compile the project [RELEASE]. Then copy the 'bwapi.ini'-file from the base-folder in Odin to 'bwapi-data' and modify it to [fit your needs](http://code.google.com/p/bwapi/wiki/MenuAutomation).
You should end up with the following file structure:


How to run
----

Start the Chaoslauncher program that comes packaged with BWAPI. Check off the appropriate version of BWAPI to be loaded (DEBUG|RELEASE) and click Start.

Start a game. Odin should now be playing.
