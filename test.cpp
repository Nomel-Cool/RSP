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
		 //ͳ�ƽ��������������ϲ�
		dispatch.statisticConvergence();

		// ��������ƽ���
		auto feed_back = decise.gainFeedBack();

		// ����������������
		decise.makeOrders(feed_back);

		// ��ȡ����ָ�ʵ�ֽ���
		dispatch.interaction();

		// չʾ�������
		dispatch.show(0);

		 //�ֶ�����
		//system("pause");

		Sleep(1000);
 	}

	//system("clean_csv_contents.bat");
	return 0;
}