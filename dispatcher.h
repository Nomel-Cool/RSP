#pragma once
#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "reality.h"
#include "virtuality.h"

#include <tuple>
#include <fstream>
#include <sstream>
#include <string>

/*��Ҫ���
��Ա������
reality,virtuality��һ������
binded ��ʾ�Ƿ�������
groupId ��ʾ��ǰ���id
��Ա������
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

/*��Ҫ���
��Ա������
һ������ ��Ŷ��Node

��Ա������
void bind(int ...) ���� ���ڽ����Node���һ��group������ͬ�����ս���ָ��
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
		while (std::getline(file, line))
		{
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
				// ȷ��ѯ��Ԫ����ȡ����QBag��Ϊ����
				auto vectors4Q = r.getVectors(j);
				auto vec4Q = vectors4Q.first;
				std::set<std::string> set4Q(vec4Q.begin(), vec4Q.end());

				for (size_t k = 0; k < N; ++k)
				{
					if (k == j)
						continue;

					std::vector<double> rates;

					// ȷ��Ӧ��Ԫ����ȡ����ABag����ֵ��
					auto vectors4A = r.getVectors(k);
					auto differenceSets = calculateDifferenceSet(vectors4A.second);

					// ����QBag��ABag��ֵ���Ľ���
					for (size_t u = 0; u < differenceSets.size(); ++u)
					{
						std::set<std::string> intersection;
						std::set_intersection(set4Q.begin(), set4Q.end(), differenceSets[u].begin(), differenceSets[u].end(),
							std::inserter(intersection, intersection.begin()));

						// ���㽻�����ϵ��ƺ�QBag���Ƶ���
						double rate = static_cast<double>(intersection.size()) / static_cast<double>(vec4Q.size());
						
						// ��ӵ��������
						rates.emplace_back(rate);
					}

					// ���½������и�������
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

		// ����һ��set����¼�Ѿ�������ļ�ֵ��
		std::set<std::pair<size_t, size_t>> processed;

		for (const auto& entry : data)
		{
			const auto& indices = entry.first;
			const auto& rates = entry.second;

			// �����ֵ��[i,j]�Ѿ����������������
			if (processed.count(indices) > 0) {
				continue;
			}

			// д���ֵ��[i,j]
			outFile << indices.first << "," << indices.second;
			for (const auto& rate : rates) {
				outFile << "," << rate;
			}
			outFile << '\n';

			// ����ֵ��[i,j]���뵽set��
			processed.insert(indices);

			// ���Ҳ�д���ֵ��[j,i]
			auto it_ji = data.find({ indices.second, indices.first });
			if (it_ji != data.end()) {
				const auto& rates_ji = it_ji->second;
				outFile << indices.second << "," << indices.first;
				for (const auto& rate : rates_ji) {
					outFile << "," << rate;
				}
				outFile << '\n';

				// ����ֵ��[j,i]���뵽set��
				processed.insert({ indices.second, indices.first });
			}
		}
		outFile.close();
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
			for (size_t k = i; k < vec.size(); ++k)
			{
				int ak = std::stoi(vec[k]);
				// ��Ϊvec�Ѿ���������ak - aiһ���ǷǸ���
				differenceSets[i].insert(std::to_string(ak - ai));
			}
		}
		return differenceSets;
	}
private:
	std::vector<Node<N, max_value, max_size> > env;
};

#endif // !DISPATCHER_H
