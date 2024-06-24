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

			/* u -> v */
			for (size_t u = 0; u < N; ++u)
			{
				// 求取第u个交互元的层集
				auto e = r.getDataPairs(u);
				auto layerSets4e = getLayerSets(e);

				// 考量第v个交互元与第u个交互元交互时
				for (size_t v = 0; v < N; ++v)
				{
					if (u == v) continue;

					// 求取第v个交互元的层集
					auto w = r.getDataPairs(v);
					auto layerSets4w = getLayerSets(w);

					// 计算平衡度，首先放入压缩信息
					double balance_degree = calculateBalanceDegree(w);
					condense_rates[{u, v}].push_back(balance_degree);

					// 计算e的缺损集
					auto LackSets4e = calculateLackSets(layerSets4e, layerSets4w);

					// 计算e的缺损差值集
					auto DiffLackSets4e = calculateLackDiffSets(LackSets4e , e);

					// 计算认知收敛率
					auto Rates = calculateRates(DiffLackSets4e, layerSets4w);
					condense_rates[{u, v}].insert(condense_rates[{u, v}].end(), Rates.begin(), Rates.end());
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

	std::vector<double> calculateRates(const std::vector<std::set<size_t>>& diffLackSets, const std::vector<std::set<size_t>>& layerSets4w)
	{
		std::vector<double> resultSets;

		// 对diffLackSets[0]特殊处理
		double rate4first = 0.0;
		std::set<size_t> intersection;
		std::set_intersection(diffLackSets[0].begin(), diffLackSets[0].end(), layerSets4w[0].begin(), layerSets4w[0].end(), std::inserter(intersection, intersection.begin()));
		if (!layerSets4w[0].empty())
			rate4first = static_cast<double>(intersection.size()) / layerSets4w[0].size();
		resultSets.push_back(rate4first);

		for (size_t i = 1; i < std::min(diffLackSets.size(), layerSets4w.size()); ++i)
		{
			double rate = 0.0;
			size_t times = 0;
			for (size_t j = 0; j < i; ++j)
			{
				// Calculate the intersection of diffLackSets[i] and layerSets4w[j]
				std::set<size_t> intersection;
				std::set_intersection(diffLackSets[i].begin(), diffLackSets[i].end(), layerSets4w[j].begin(), layerSets4w[j].end(), std::inserter(intersection, intersection.begin()));

				// Calculate the rate as | diffLackSets[i] ∩ layerSets4w[j]| / |layerSets4w[j] |
				if (!layerSets4w[j].empty())
					rate += static_cast<double>(intersection.size()) / layerSets4w[j].size();
			}
			resultSets.push_back(static_cast<double>(rate) / i);
		}

		for (size_t j = std::min(diffLackSets.size(), layerSets4w.size()); j < diffLackSets.size(); ++j)
		{
			double rate = 0.0;
			for (size_t k = 0; k < std::min(diffLackSets.size(), layerSets4w.size()); ++k)
			{
				std::set<size_t> intersection;
				std::set_intersection(diffLackSets[j].begin(), diffLackSets[j].end(), layerSets4w[k].begin(), layerSets4w[k].end(), std::inserter(intersection, intersection.begin()));

				// Calculate the rate as | diffLackSets[i] ∩ layerSets4w[j]| / |layerSets4w[j] |
				if (!layerSets4w[k].empty())
					rate += static_cast<double>(intersection.size()) / layerSets4w[k].size();
			}
			resultSets.push_back(static_cast<double>(rate) / j);
		}
		return resultSets;
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

	std::vector<std::set<size_t>> getLayerSets(const std::vector<std::pair<size_t, std::string>>& e)
	{
		std::vector<std::set<size_t>> result_sets;
		for (size_t j = 0; j < e.size(); ++j)
		{
			std::set<size_t> layers;
			auto it_Ak_begin = std::find_if(e.begin(), e.end(),
				[j](const std::pair<size_t, std::string>& item)
				{
					return std::count(item.second.begin(), item.second.end(), '+') == j;
				});

			auto it_Ak_end = std::find_if(e.begin(), e.end(),
				[j](const std::pair<size_t, std::string>& item)
				{
					return std::count(item.second.begin(), item.second.end(), '+') == j + 1;
				});

			// 否则把[it_Ak_begin,it_Ak_end-1]之间的元素全部放入一个集合，并存储到result_sets中
			for (auto it = it_Ak_begin; it != it_Ak_end; ++it)
			{
				layers.insert(it->first);
			}
			result_sets.push_back(layers);

			if (it_Ak_end == e.end()) // 如果这是最后一个Ak，则停止计算
				break;
		}
		return result_sets;
	}

	std::vector<std::set<size_t>> calculateLackSets(const std::vector<std::set<size_t>>& e, const std::vector<std::set<size_t>>& w)
	{
		std::vector<std::set<size_t>> resultSets;
		for (size_t i = 0; i < std::min(e.size(), w.size()); ++i)
		{
			std::set<size_t> lacks;
			/* lacks = {e[i] - e[i] ∩ w[i]} */
			std::set<size_t> intersection;
			std::set_intersection(e[i].begin(), e[i].end(), w[i].begin(), w[i].end(), std::inserter(intersection, intersection.begin()));
			std::set_difference(e[i].begin(), e[i].end(), intersection.begin(), intersection.end(), std::inserter(lacks, lacks.begin()));
			resultSets.push_back(lacks);
		}
		for (size_t j = std::min(e.size(), w.size()); j < e.size(); ++j)
			resultSets.push_back(e[j]);
		return resultSets;
	}

	std::vector<std::set<size_t>> calculateLackDiffSets(const std::vector<std::set<size_t>>& lack_e, const std::vector<std::pair<size_t, std::string>>& e)
	{
		std::vector<std::set<size_t>> resultSets;
		for (size_t i = 0; i < lack_e.size(); ++i)
		{
			std::set<size_t> diffLack;
			for (auto& lack : lack_e[i])
			{
				for (auto& pair : e)
				{
					auto rank = std::count(pair.second.begin(), pair.second.end(), '+');
					if(lack >= pair.first && rank <= i)
						diffLack.insert(lack - pair.first);
				}
			}
			resultSets.push_back(diffLack);
		}
		return resultSets;
	}
private:
	std::vector<Node<N, max_value, max_size> > env;
};

#endif // !DISPATCHER_H
