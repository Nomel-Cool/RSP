#include "dispatcher.h"
#include "decision.h"
//#include "reality.h"
#include <iostream>
#include <Windows.h>

using namespace std;

int main()
{
	decision<5> decise;
	dispatcher<5, 1, 5, 12> dispatch;
	while(true)
	{
		 //ͳ�ƽ��������������ϲ�
		dispatch.statisticConvergence();
		dispatch.statisticAnswering();

		// ��������ƽ���
		auto feed_back = decise.gainFeedBack();

		// ����������������
		decise.makeOrders(feed_back);

		// ��ȡ����ָ�ʵ�ֽ���
		dispatch.interaction();

		// ����λ��ռ������
		dispatch.statisticRatio();

		// չʾ�������
		dispatch.show(0);

		 //�ֶ�����
		//system("pause");

		// �Զ�����
		Sleep(1000);
 	}

	//system("clean_csv_contents.bat");
	return 0;
}