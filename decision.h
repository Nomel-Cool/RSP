#pragma once
#ifndef DECISION_H
#define DECISION_H

#include <tuple>
#include <map>
#include <fstream>
#include <random>

/*��Ҫ���
��Ա������

��Ա������
gainFeedback() ���� ���ڶ�ȡ������д��Ľ������������ļ�
processingarguments() ���� ��ȡ�����ļ�������㷨����������һ�εĽ�������
makeOrders() ���� ���㷨���ɵĽ�������д���ļ������������ȡ
*/
class decision
{
public:
	decision()
	{

	}
	~decision()
	{

	}
	virtual void gainFeedBack()
	{
		
	}
	virtual void makeOrders()
	{
		std::ofstream file("interaction_orders.csv");

		// д��CSV�ļ��ı�����
		file << "Order,NodeId,Item_i,Item_j\n";

		// ���������������
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(1, 5); // ����item_i��item_j�ķ�Χ��0~4

		// ��������Ľ�������
		for (int order = 1; order <= 40; ++order) // ��������Ҫ����100������
		{
			int item_i = dis(gen);
			int item_j = dis(gen);
			file << order << ",0," << item_i << "," << item_j << "\n"; // ��ʱ�涨NodeIdΪ0
		}
	}

protected:
	virtual void processingargumentts()
	{

	}
private:
	
};

#endif // !DECISION_H
