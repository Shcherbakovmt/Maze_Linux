#include "elements.h"

elements::elements()
{
	complete_river = new point[64]; // ��������� ����
}

elements::~elements()
{
	delete[] complete_river;
}
