#pragma once
#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "reality.h"
#include "virtuality.h"

#include <fstream>
#include <sstream>
#include <string>

/*概要设计
成员变量：
reality,virtuality各一个对象
binded 表示是否处于组内
groupId 表示当前组的id
成员函数：
set get
*/
template<size_t N, uint32_t max_value, size_t max_size>
class Node
{
public:
	Node() :r(max_size, max_value)
	{
		binded = false;
		groupId = -1;
	}
	void enlist(int groupId, bool binded)
	{
		groupId = groupId;
		binded = binded;
	}
	void set(size_t i, size_t j)
	{
		//v.interaction(i, j);
		r.interaction(i, j);
	}
	void show()
	{
		r.show();
		for (auto e : v.getStatus())
		{
			printf(e.c_str());
		}
	}
	st::reality<N>& getR()
	{
		return r;
	}
	st::virtuality<N> getV()
	{
		return v;
	}
private:
	int groupId;
	bool binded;
	st::reality<N> r;
	st::virtuality<N> v;
};

/*概要设计
成员变量：
一个数组 存放多个Node

成员函数：
void bind(int ...) 公有 用于将多个Node组成一个group，它们同步接收交互指令
*/
template<size_t N, size_t scale, uint32_t max_value, size_t max_size>
class dispatcher
{
public:
	dispatcher()
	{
		env.resize(scale);
	}
	void bind(int groupId, int arg, ...)
	{
		
	}
	void show(size_t i)
	{
		env[i].show();
	}
	void interaction()
	{
		std::ifstream file("interaction_orders.csv");
		std::string line;
		// 跳过标题行
		std::getline(file, line);
		while (std::getline(file, line))
		{
			std::istringstream iss(line);
			std::string order, node_id, item_i, item_j;
			std::getline(iss, order, ',');
			std::getline(iss, node_id, ',');
			std::getline(iss, item_i, ',');
			std::getline(iss, item_j, ',');
			int nodeId = std::stoi(node_id);
			size_t i = std::stoul(item_i);
			size_t j = std::stoul(item_j);
			if (nodeId >= 0 && nodeId < env.size())
			{
				env[nodeId].set(i, j);
				int ddd = 0;
			}
		}
	}
private:
	std::vector<Node<N, max_value, max_size> > env;
};

#endif // !DISPATCHER_H
