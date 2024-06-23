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
		AccuracyData condense_rates;

		for (size_t i = 0; i < env.size(); ++i)
		{
			reality<N> r = env[i].getR();

			for (size_t u = 0; u < N; ++u)
			{
				// ��ȡ��u������Ԫ��ǰ׺��ֵ��
				auto e = r.getDataPairs(u);
				auto diffSets = calculateDifference(e);

				// ������v������Ԫ���u������Ԫ����ʱ
				for (size_t v = 0; v < N; ++v)
				{
					if (u == v)
						continue;

					auto w = r.getDataPairs(v);

					// ����ƽ��ȣ����ȷ���ѹ����Ϣ
					double balance_degree = calculateBalanceDegree(w);
					condense_rates[{v, u}].push_back(balance_degree);

					// ��ȡAk��diffSets(k-1)�Ľ���
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

						// ���㳬��������
						double rate = calculateRates(Ak, diffSets[j - 1]);

						// ��¼����������������
						condense_rates[{v,u}].push_back(rate);

						// ����������һ��Ak����ֹͣ����
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

	std::vector<std::set<size_t> > calculateDifference(const std::vector<std::pair<size_t, std::string>>& e)
	{
		std::vector<std::set<size_t>> result;
		// ��ȡAk��զ����Ľ���ʱ���±꣬���k������e.size()�����������ǰ�׶����forѭ����׼��
		for (size_t k = 0; k < e.size(); ++k)
		{
			auto it_k = std::find_if(e.begin(), e.end(),
				[k](const std::pair<size_t, std::string > &item)
				{
					return std::count(item.second.begin(), item.second.end(), '+') == k;
				});

			//��ʼ��ȡ���е���it_k��ǰ�Ĳ�ֵ
			std::set<size_t> diffSet;
			for (auto it_i = e.begin(); it_i != it_k; ++it_i)
			{
				for (auto it_j = it_i; it_j != it_k; ++it_j) // ������Ԫ���ڣ�����ζ��ֱ�ӽ������
				{
					size_t diff = (it_j->first >= it_i->first) ? (it_j->first - it_i->first) : (it_i->first - it_j->first);
					diffSet.insert(diff);
				}
			}
			result.push_back(diffSet);

			// ����������һ��Ak����ֹͣ����
			if (it_k == e.end())
				break;
		}
		return result;
	}

	double calculateRates(std::set<size_t> Ak, std::set<size_t> differSet)
	{
		// ����Ak��differSet�Ľ���intersection,������intersection������Ak���Ƶ��̲�����
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

			// ����������һ��Ak����ֹͣ����
			if (it_Ak_end == data_pair.end())
				break;
		}
		return expectation;
	}
private:
	std::vector<Node<N, max_value, max_size> > env;
};

#endif // !DISPATCHER_H
