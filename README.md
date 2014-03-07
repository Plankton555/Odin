Odin
====

A BWAPI bot based on [UAlbertaBot](http://code.google.com/p/ualbertabot/), which is licensed under [GPL v3](http://www.gnu.org/licenses/gpl.html). 
Devoted to the wisest of the supreme Norse gods, Odin.

> [Odin](http://www.comicvine.com/odin/4005-3507/) is the All-Father of the Norse gods and ruler of Asgard. [...] [He] is the wisest and by far the most powerful of the Asgardian Gods.



How to compile
----

To successfully compile, this project needs two specific system environment variables. `%BWAPI_DIR%` shall point to the BWAPI-installation and `%BOOST_DIR%` shall point to the boost directory. Also, one needs to install the [Boost-binaries](http://sourceforge.net/projects/boost/files/boost-binaries/1.55.0/boost_1_55_0-msvc-9.0-32.exe/download) in the same directory.


How to run
----

Copy the generated .dll-file to the proper location (usually `Starcraft/bwapi-data/AI/`, but look in your `BWAPI.ini`-file to be sure). Inject the dll to the Starcraft-process, eg with ChaosLauncher.
