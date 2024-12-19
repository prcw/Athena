#pragma once
#include "athena/public/process_event.hpp"

class entry_point
{
public:
	static inline bool begin()
	{
		debug::allocate_console();
		debug::log("entry_point::begin() called.");



		debug::log("entry_point::begin() executed.");
		return true;
	}
};