#include <Core/Core.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_FILE|LOG_COUT);

	Vector<String> h;
	for(int i = 0; i < 10000000; i++)
		h.Add(AsString(Random()));
	
	{
		auto h1 = clone(h);
		TimeStop tm;
		Sort(h1);
		RLOG("Sort " << tm);
	}

	{
		auto h1 = clone(h);
		TimeStop tm;
		CoSort(h1);
		RLOG("CoSort " << tm);
	}
}