#pragma once

#include <boost/timer.hpp>

class Timer 
{
	boost::timer t;

public : 

	Timer() {}
	~Timer() {};

	void restart()		{ t.restart(); }
	double elapsed()	{ return t.elapsed() * 1000.0; }
};
