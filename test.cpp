#include "virtuality.h"
#include <iostream>

using namespace std;

int main()
{
	virtuality<5> v;
	v.interaction(1, 2);
	auto list = v.getStatus();
	for (auto e : list)
		cout << e << endl;
	auto code = v.getCode();
	printf("----------");
	while (!code.empty())
	{
		cout << code.top() << endl;
		code.pop();
	}
	return 0;
}