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
		std::vector<std::string> lines;
		while (std::getline(file, line))
		{
			lines.push_back(line);
		}
		// ÿ�δ������£����һ�У���һ��
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
				// ȷ��ѯ��Ԫ����ȡ����QBag-ABag��Ϊ����
				auto pairs4Q = r.getDataPairs(j);
				std::vector<std::pair<size_t, std::string>> qa_difference;

				// �ҳ����г�ԽԪ������qa_difference
				for (const auto& pair : pairs4Q)
					if (pair.second.find("+") != std::string::npos) // ����ҵ�"+"����Ϊ��ԽԪ
						qa_difference.push_back(pair);

				for (size_t k = 0; k < N; ++k)
				{
					if (k == j)
						continue;

					std::vector<double> rates;

					// ȷ��Ӧ��Ԫ����ȡ����ABag����ֵ��
					auto pairs4A = r.getDataPairs(k);
					auto differenceSets = calculateHitDifferenceSet(pairs4A);

					// ����{pairs4Q.first}��differenceSets�Ľ���
					std::vector<std::set<size_t>> intersections;
					std::set<size_t> set4first;
					for (const auto& pair : pairs4Q)
						set4first.insert(pair.first);

					// ���� set4first �� differenceSets ��ÿһ�����ϵĽ���
					for (const auto& set : differenceSets)
					{
						std::set<size_t> intersection;
						std::set_intersection(set4first.begin(), set4first.end(), set.begin(), set.end(),
							std::inserter(intersection, intersection.begin()));
						intersections.push_back(intersection);
					}

					//����qa_differenceÿ��Ԫ�ؼ�ȥpairs4A.firstÿ��Ԫ�صļ��Ϧ�
					auto phi = calculateSolveDifferenceSet(pairs4A, qa_difference);

					// �������intersections�Ľ�����
					if (intersections.size() != phi.size()) throw;
					std::vector<std::set<size_t>> eta;
					for (size_t u = 0; u < intersections.size(); ++u)
					{
						std::set<size_t> temp;
						std::set_intersection(phi[u].begin(), phi[u].end(), intersections[u].begin(), intersections[u].end(),
							std::inserter(temp, temp.begin()));
						eta.emplace_back(temp);
					}

					// ����ǵ��ƺ�set4QBag���Ƶ���,��ӵ��������
					for (size_t u = 0; u < eta.size(); ++u)
						rates.emplace_back(static_cast<double>(eta[u].size()) / static_cast<double>(pairs4Q.size()));

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
				// ��Ϊvec�Ѿ���������ak - aiһ���ǷǸ���
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
				// ��Ϊvec�Ѿ���������ak - aiһ���ǷǸ���
				differenceSets[i].insert(sk - ai);
			}
		}
		return differenceSets;
	}

private:
	std::vector<Node<N, max_value, max_size> > env;
};

#endif // !DISPATCHER_H
