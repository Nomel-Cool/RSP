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
		// ����������
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

					// ��vectors.firstת��Ϊ����
					std::set<std::string> set4Q(vec4Q.begin(), vec4Q.end());

					// ���㽻��
					std::set<std::string> intersection;
					std::set_intersection(set4Q.begin(), set4Q.end(), differenceSets[k].begin(), differenceSets[k].end(),
						std::inserter(intersection, intersection.begin()));

					// ���㽻�����ϵ��ƺ�vectors.first����
					double rate = static_cast<double>(intersection.size()) / static_cast<double>(vec4Q.size());

					// ��ӵ��������
					accuracy_rates.emplace_back(std::make_tuple(k, j, std::vector<double>{rate}));
				}
			}
		}

		writeAccuracyToFile(accuracy_rates,"interaction_accuracy.csv");
	}
protected:
	using AccuracyData = std::vector<std::tuple<size_t, size_t, std::vector<double>>>;
	void writeAccuracyToFile(AccuracyData& data, const std::string& filename) {
		// �����ݽ�������
		std::sort(data.begin(), data.end(),[](const auto& a, const auto& b)
			{
				// ���ȱȽ�Index_i
				if (std::get<0>(a) != std::get<0>(b)) {
					return std::get<0>(a) < std::get<0>(b);
				}
		// ���Index_i��ȣ���ô�Ƚ�Index_j
		return std::get<1>(a) < std::get<1>(b);
			});

		std::ofstream file(filename);

		if (!file.is_open())
			return;

		// д���ļ�ͷ��
		file << "Index_i, Index_j, AccuracyRate" << std::endl;

		// д������
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
