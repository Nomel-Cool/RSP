#pragma once
#ifndef DECISION_H
#define DECISION_H

#include <tuple>
#include <map>
#include <fstream>
#include <random>

/*概要设计
成员变量：

成员函数：
gainFeedback() 公有 用于读取反馈层写入的交互反馈参数文件
processingarguments() 保护 读取参数文件后调用算法调参生成下一次的交互序列
makeOrders() 公有 将算法生成的交互序列写入文件，供反馈层读取
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

		// 写入CSV文件的标题行
		file << "Order,NodeId,Item_i,Item_j\n";

		// 创建随机数生成器
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(1, 5); // 假设item_i和item_j的范围是0~4

		// 生成随机的交互序列
		for (int order = 1; order <= 40; ++order) // 假设我们要生成100个交互
		{
			int item_i = dis(gen);
			int item_j = dis(gen);
			file << order << ",0," << item_i << "," << item_j << "\n"; // 暂时规定NodeId为0
		}
	}

protected:
	virtual void processingargumentts()
	{

	}
private:
	
};

#endif // !DECISION_H
