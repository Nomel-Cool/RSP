#pragma once
#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "reality.h"
#include "virtuality.h"

#include <tuple>
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
	st::reality<N> getR()
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
				env[nodeId].set(i, j);
		}
	}
	void statisticConvergence()
	{
		std::vector<std::tuple<size_t, size_t, std::vector<double>>> accuracy_rates;

		for (size_t i = 0; i < env.size(); ++i)
		{
			auto r = env[i].getR();

			for (size_t j = 0; j < N; ++j)
			{
				auto vectors4A = r.getVectors(j);
				auto differenceSets = calculateDifferenceSet(vectors4A.second);

				for (size_t k = 0; k < N && k != j; ++k)
				{
					auto vectors4Q = r.getVectors(k);
					auto vec4Q = vectors4Q.first;

					// 将vectors.first转换为集合
					std::set<std::string> set4Q(vec4Q.begin(), vec4Q.end());

					// 计算交集
					std::set<std::string> intersection;
					std::set_intersection(set4Q.begin(), set4Q.end(), differenceSets[k].begin(), differenceSets[k].end(),
						std::inserter(intersection, intersection.begin()));

					// 计算交集集合的势和vectors.first的势
					double rate = static_cast<double>(intersection.size()) / static_cast<double>(vec4Q.size());

					// 添加到结果数组
					accuracy_rates.emplace_back(std::make_tuple(k, j, std::vector<double>{rate}));
				}
			}
		}

		writeAccuracyToFile(accuracy_rates,"interaction_accuracy.csv");
	}
protected:
	using AccuracyData = std::vector<std::tuple<size_t, size_t, std::vector<double>>>;
	void writeAccuracyToFile(AccuracyData& data, const std::string& filename) {
		// 对数据进行排序
		std::sort(data.begin(), data.end(),[](const auto& a, const auto& b)
			{
				// 首先比较Index_i
				if (std::get<0>(a) != std::get<0>(b)) {
					return std::get<0>(a) < std::get<0>(b);
				}
		// 如果Index_i相等，那么比较Index_j
		return std::get<1>(a) < std::get<1>(b);
			});

		std::ofstream file(filename);

		if (!file.is_open())
			return;

		// 写入文件头部
		file << "Index_i, Index_j, AccuracyRate" << std::endl;

		// 写入数据
		for (const auto& tuple : data) {
			size_t i, j;
			std::vector<double> rates;
			std::tie(i, j, rates) = tuple;

			file << i << ", " << j << ", ";
			for (const auto& rate : rates) {
				file << rate << " ";
			}
			file << std::endl;
		}

		file.close();
	}

	std::vector<std::set<std::string>> calculateDifferenceSet(std::vector<std::string> vec)
	{
		std::sort(vec.begin(), vec.end(), [](const std::string& a, const std::string& b) {
			return std::stoi(a) < std::stoi(b);
			});
		std::vector<std::set<std::string>> differenceSets(vec.size());
		for (size_t i = 0; i < vec.size(); ++i)
		{
			int ai = std::stoi(vec[i]);
			for (size_t k = i + 1; k < vec.size(); ++k)
			{
				int ak = std::stoi(vec[k]);
				// 因为vec已经排序，所以ak - ai一定是非负的
				differenceSets[i].insert(std::to_string(ak - ai));
			}
		}
		return differenceSets;
	}
private:
	std::vector<Node<N, max_value, max_size> > env;
};

#endif // !DISPATCHER_H
