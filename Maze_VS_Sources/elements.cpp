#include "elements.h"

elements::elements()
{
	complete_river = new point[64]; // ôèíàëüíàÿ ðåêà
}

elements::~elements()
{
	delete[] complete_river;
}
