#pragma once
#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "reality.h"
#include "virtuality.h"

#include <tuple>
#include <fstream>
#include <sstream>
#include <string>

using AccuracyData = std::map<std::pair<size_t, size_t>, std::vector<double >>;

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
	void set(size_t i, size_t j, size_t index_i, size_t index_j)
	{
		//v.interaction(i, j);
		r.interaction(i, j, index_i, index_j);
	}
	void show()
	{
		r.show();
		for (auto e : v.getStatus())
			printf(e.c_str());
	}
	reality<N> getR()
	{
		return r;
	}
	virtuality<N> getV()
	{
		return v;
	}
private:
	int groupId;
	bool binded;
	reality<N> r;
	virtuality<N> v;
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
		std::vector<std::string> lines;
		while (std::getline(file, line))
		{
			lines.push_back(line);
		}
		// 每次处理最新（最后一行）的一行
		if (!lines.empty())
		{
			line = lines.back();
			std::istringstream iss(line);
			std::string order, node_id, item_i, item_j, e_i, e_j;
			std::getline(iss, order, ',');
			std::getline(iss, node_id, ',');
			std::getline(iss, item_i, ',');
			std::getline(iss, item_j, ',');
			std::getline(iss, e_i, ',');
			std::getline(iss, e_j, ',');
			int nodeId = std::stoi(node_id);
			size_t i = std::stoul(item_i);
			size_t j = std::stoul(item_j);
			size_t index_i = std::stoul(e_i);
			size_t index_j = std::stoul(e_j);
			if (nodeId >= 0 && nodeId < env.size())
				env[nodeId].set(i, j, index_i, index_j);
		}
	}
	void statisticConvergence()
	{
		AccuracyData condense_rates;

		for (size_t i = 0; i < env.size(); ++i)
		{
			reality<N> r = env[i].getR();

			for (size_t u = 0; u < N; ++u)
			{
				// 求取第u个交互元的前缀差值集
				auto e = r.getDataPairs(u);
				auto diffSets = calculateDifference(e);

				// 考量第v个交互元与第u个交互元交互时
				for (size_t v = 0; v < N; ++v)
				{
					if (u == v)
						continue;

					auto w = r.getDataPairs(v);

					// 计算平衡度，首先放入压缩信息
					double balance_degree = calculateBalanceDegree(w);
					condense_rates[{v, u}].push_back(balance_degree);

					// 求取Ak与diffSets(k-1)的交集
					for (size_t j = 1; j <= diffSets.size(); ++j)
					{
						auto it_Ak_begin = std::find_if(w.begin(), w.end(),
							[j](const std::pair<size_t, std::string>& item)
							{
								return std::count(item.second.begin(), item.second.end(), '+') == j - 1;
							});

						auto it_Ak_end = std::find_if(w.begin(), w.end(),
							[j](const std::pair<size_t, std::string>& item)
							{
								return std::count(item.second.begin(), item.second.end(), '+') == j;
							});

						std::set<size_t> Ak;
						for (auto it = it_Ak_begin; it != it_Ak_end; ++it)
							Ak.insert(it->first);

						// 计算超验收缩率
						double rate = calculateRates(Ak, diffSets[j - 1]);

						// 记录到超验收缩率数组
						condense_rates[{v,u}].push_back(rate);

						// 如果这是最后一个Ak，则停止计算
						if (it_Ak_end == w.end())
							break;
					}
				}
			}
		}

		writeToFile(condense_rates,"interaction_accuracy.csv");
	}
protected:
	void writeToFile(AccuracyData& data, const std::string& filename)
	{
		std::ofstream outFile(filename);

		// 创建一个set来记录已经处理过的键值对
		std::set<std::pair<size_t, size_t>> processed;

		for (const auto& entry : data)
		{
			const auto& indices = entry.first;
			const auto& rates = entry.second;

			// 如果键值对[i,j]已经被处理过，就跳过
			if (processed.count(indices) > 0) {
				continue;
			}

			// 写入键值对[i,j]
			outFile << indices.first << "," << indices.second;
			for (const auto& rate : rates) {
				outFile << "," << rate;
			}
			outFile << '\n';

			// 将键值对[i,j]加入到set中
			processed.insert(indices);

			// 查找并写入键值对[j,i]
			auto it_ji = data.find({ indices.second, indices.first });
			if (it_ji != data.end()) {
				const auto& rates_ji = it_ji->second;
				outFile << indices.second << "," << indices.first;
				for (const auto& rate : rates_ji) {
					outFile << "," << rate;
				}
				outFile << '\n';

				// 将键值对[j,i]加入到set中
				processed.insert({ indices.second, indices.first });
			}
		}
		outFile.close();
	}

	std::vector<std::set<size_t> > calculateDifference(const std::vector<std::pair<size_t, std::string>>& e)
	{
		std::vector<std::set<size_t>> result;
		// 求取Ak所咋数组的结束时的下标，最多k不超过e.size()，因此作好提前阶段这个for循环的准备
		for (size_t k = 0; k < e.size(); ++k)
		{
			auto it_k = std::find_if(e.begin(), e.end(),
				[k](const std::pair<size_t, std::string > &item)
				{
					return std::count(item.second.begin(), item.second.end(), '+') == k;
				});

			//开始求取所有到达it_k以前的差值
			std::set<size_t> diffSet;
			for (auto it_i = e.begin(); it_i != it_k; ++it_i)
			{
				for (auto it_j = it_i; it_j != it_k; ++it_j) // 允许零元存在，这意味着直接解决问题
				{
					size_t diff = (it_j->first >= it_i->first) ? (it_j->first - it_i->first) : (it_i->first - it_j->first);
					diffSet.insert(diff);
				}
			}
			result.push_back(diffSet);

			// 如果这是最后一个Ak，则停止计算
			if (it_k == e.end())
				break;
		}
		return result;
	}

	double calculateRates(std::set<size_t> Ak, std::set<size_t> differSet)
	{
		// 计算Ak与differSet的交集intersection,并计算intersection的势与Ak的势的商并返回
		std::vector<size_t> intersection;
		std::set_intersection(Ak.begin(), Ak.end(), differSet.begin(), differSet.end(), std::back_inserter(intersection));
		double rate = static_cast<double>(intersection.size()) / Ak.size();
		return rate;
	}

	double calculateBalanceDegree(const std::vector<std::pair<size_t, std::string>>& data_pair)
	{
		double expectation = 0.0;
		for (size_t j = 1; j < data_pair.size(); ++j)
		{
			auto it_Ak_begin = std::find_if(data_pair.begin(), data_pair.end(),
				[j](const std::pair<size_t, std::string>& item)
				{
					return std::count(item.second.begin(), item.second.end(), '+') == j - 1;
				});

			auto it_Ak_end = std::find_if(data_pair.begin(), data_pair.end(),
				[j](const std::pair<size_t, std::string>& item)
				{
					return std::count(item.second.begin(), item.second.end(), '+') == j;
				});
			size_t amount_ak = std::distance(it_Ak_begin, it_Ak_end);
			expectation += (static_cast<double>(j) * static_cast<double>(amount_ak) / static_cast<double>(data_pair.size()));

			// 如果这是最后一个Ak，则停止计算
			if (it_Ak_end == data_pair.end())
				break;
		}
		return expectation;
	}
private:
	std::vector<Node<N, max_value, max_size> > env;
};

#endif // !DISPATCHER_H
