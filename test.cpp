#include "dispatcher.h"
#include "decision.h"
#include <iostream>

using namespace std;

int main()
{
	decision de;
	de.makeOrders();
	dispatcher<5, 1, 5, 12> d;
	d.interaction();
	d.statisticConvergence();
	d.show(0);
	return 0;
}