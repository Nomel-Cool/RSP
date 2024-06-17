#include "virtuality.h"
#include "reality.h"
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
	
	reality<5> r(5,12);
	int n = 10;
	while (n--)
		r.interaction(1, 2);
	r.show();
	return 0;
}