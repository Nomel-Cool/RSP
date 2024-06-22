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
		std::map<std::pair<size_t, size_t>, std::vector<double>> accuracy_rates;

		for (size_t i = 0; i < env.size(); ++i)
		{
			auto r = env[i].getR();

			for (size_t j = 0; j < N; ++j)
			{
				// 确定询问元，提取它的QBag-ABag作为集合
				auto pairs4Q = r.getDataPairs(j);
				std::vector<std::pair<size_t, std::string>> qa_difference;

				// 找出所有超越元，放入qa_difference
				for (const auto& pair : pairs4Q)
					if (pair.second.find("+") != std::string::npos) // 如果找到"+"，则为超越元
						qa_difference.push_back(pair);

				for (size_t k = 0; k < N; ++k)
				{
					if (k == j)
						continue;

					std::vector<double> rates;

					// 确定应答元，提取它的ABag作差值集
					auto pairs4A = r.getDataPairs(k);
					auto differenceSets = calculateHitDifferenceSet(pairs4A);

					// 计算{pairs4Q.first}与differenceSets的交集
					std::vector<std::set<size_t>> intersections;
					std::set<size_t> set4first;
					for (const auto& pair : pairs4Q)
						set4first.insert(pair.first);

					// 计算 set4first 与 differenceSets 的每一个集合的交集
					for (const auto& set : differenceSets)
					{
						std::set<size_t> intersection;
						std::set_intersection(set4first.begin(), set4first.end(), set.begin(), set.end(),
							std::inserter(intersection, intersection.begin()));
						intersections.push_back(intersection);
					}

					//计算qa_difference每个元素减去pairs4A.first每个元素的集合φ
					auto phi = calculateSolveDifferenceSet(pairs4A, qa_difference);

					// 计算φ与intersections的交集η
					if (intersections.size() != phi.size()) throw;
					std::vector<std::set<size_t>> eta;
					for (size_t u = 0; u < intersections.size(); ++u)
					{
						std::set<size_t> temp;
						std::set_intersection(phi[u].begin(), phi[u].end(), intersections[u].begin(), intersections[u].end(),
							std::inserter(temp, temp.begin()));
						eta.emplace_back(temp);
					}

					// 计算η的势和set4QBag的势的商,添加到结果数组
					for (size_t u = 0; u < eta.size(); ++u)
						rates.emplace_back(static_cast<double>(eta[u].size()) / static_cast<double>(pairs4Q.size()));

					// 更新交互命中概率数组
					accuracy_rates[std::make_pair(j, k)] = rates;
				}
			}
		}

		writeAccuracyToFile(accuracy_rates,"interaction_accuracy.csv");
	}
protected:
	using AccuracyData = std::map<std::pair<size_t, size_t>, std::vector<double>>;
	void writeAccuracyToFile(AccuracyData& data, const std::string& filename)
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

	std::vector<std::set<size_t>> calculateHitDifferenceSet(std::vector<std::pair<size_t, std::string> > vec)
	{
		std::sort(vec.begin(), vec.end(), [](const auto& a, const auto& b)
			{
				return a.first < b.first;
			});
		std::vector<std::set<size_t>> differenceSets(vec.size());
		for (size_t i = 0; i < vec.size(); ++i)
		{
			size_t ai = vec[i].first;
			for (size_t k = i; k < vec.size(); ++k)
			{
				size_t ak = vec[k].first;
				// 因为vec已经排序，所以ak - ai一定是非负的
				differenceSets[i].insert(ak - ai);
			}
		}
		return differenceSets;
	}

	std::vector<std::set<size_t>> calculateSolveDifferenceSet(std::vector<std::pair<size_t, std::string> > vec, std::vector<std::pair<size_t, std::string> > vec_solve)
	{
		std::vector<std::set<size_t>> differenceSets(vec.size());
		for (size_t i = 0; i < vec.size(); ++i)
		{
			size_t ai = vec[i].first;
			for (size_t k = 0; k < vec_solve.size(); ++k)
			{
				size_t sk = vec_solve[k].first;
				// 因为vec已经排序，所以ak - ai一定是非负的
				differenceSets[i].insert(sk - ai);
			}
		}
		return differenceSets;
	}

private:
	std::vector<Node<N, max_value, max_size> > env;
};

#endif // !DISPATCHER_H
