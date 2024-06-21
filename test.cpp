#include "dispatcher.h"
#include "decision.h"
#include <iostream>
#include <Windows.h>

using namespace std;

int main()
{
	decision<5> decise;
	dispatcher<5, 1, 5, 12> dispatch;
	while(true)
	{		
		 //统计交互参数，返回上层
		dispatch.statisticConvergence();

		// 考察整体平衡度
		auto feed_back = decise.gainFeedBack();

		// 接着制作交互控制
		decise.makeOrders(feed_back);

		// 读取交互指令，实现交互
		dispatch.interaction();

		// 展示交互结果
		dispatch.show(0);

		 //手动迭代
		//system("pause");

		Sleep(1000);
 	}

	//system("clean_csv_contents.bat");
	return 0;
}