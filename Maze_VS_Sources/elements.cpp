#include "elements.h"

elements::elements()
{
	complete_river = new point[64]; // финальная река
}

elements::~elements()
{
	delete[] complete_river;
}
