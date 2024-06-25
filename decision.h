#pragma once
#ifndef DECISION_H
#define DECISION_H

#include <tuple>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>
#include <utility>
#include <cmath>
#include <random>
#include <numeric>
#include <algorithm>

using AccuracyData = std::map<std::pair<size_t, size_t>, std::vector<double>>;
using AnswerData = std::map<std::pair<size_t, size_t>, std::map<std::pair<size_t, size_t>, std::vector<double>>>;
using BalanceData = std::map<std::pair<size_t, size_t>, double>;
using SizeT2 = std::tuple<size_t, size_t>;
using SizeT3 = std::tuple<size_t, size_t, size_t>;
using SizeT4 = std::tuple<size_t, size_t, size_t, size_t>;
using SizeT7 = std::tuple<size_t, size_t, size_t, size_t, size_t, size_t, size_t>;

/*概要设计
成员变量：

成员函数：
gainFeedback() 公有 用于读取反馈层写入的交互反馈参数文件
processingarguments() 保护 读取参数文件后调用算法调参生成下一次的交互序列
makeOrders() 公有 将算法生成的交互序列写入文件，供反馈层读取
*/
template<size_t max_size>
class decision
{
public:
	decision()
	{

	}
	~decision()
	{

	}

	virtual SizeT7 gainFeedBack()
	{
		AccuracyData accuracy_data = readFile4AccuracyData("interaction_accuracy.csv");

		AnswerData answer_data = readFile4AnswerData("interaction_answer.csv");

		return processingarguments(accuracy_data, answer_data);
	}

	virtual void makeOrders(SizeT7 arguments)
	{
		// 追加到文件interaction_orders.csv的尾部
		std::ofstream outFile("interaction_orders.csv", std::ios_base::app);
		outFile << std::get<0>(arguments) << ","
			<< 0 // 暂时只考虑一个Node的情况
			<< "," << std::get<1>(arguments) << "," << std::get<2>(arguments)
			<< "," << std::get<3>(arguments) << "," << std::get<4>(arguments)
			<< "," << std::get<5>(arguments) << "," << std::get<6>(arguments) << "\n";
		outFile.close();
	}

protected:

	// 计算协方差，使用迫半期望expectation来替代原本的期望
	double covariance(const std::vector<double>& X, const std::vector<double>& Y, double expectation = 0.5)
	{
		double sum = 0;
		std::vector<double> statistic4XY;
		for (size_t i = 0; i < X.size(); ++i)
			for (size_t j = 0; j < Y.size(); ++j)
				statistic4XY.emplace_back(X[i] * Y[j]);

		// 使用无偏估计
		double meanXY = std::accumulate(statistic4XY.begin(), statistic4XY.end(), 0.00) / statistic4XY.size();
		return meanXY - expectation * expectation;
	}

	// 计算样本方差，这使得初始状态即使全是0，也会产生方差均值上的差异
	double variance(const std::vector<double>& X, double expectation)
	{
		double sum = 0;
		for (size_t i = 0; i < X.size(); i++)
			sum += (X[i] - expectation) * (X[i] - expectation);
		return sum / (X.size() - 1);
	}

	AccuracyData readFile4AccuracyData(const std::string& filename)
	{
		AccuracyData data;
		std::ifstream inFile(filename);
		std::string line;

		while (std::getline(inFile, line))
		{
			std::istringstream iss(line);
			std::string token;
			std::getline(iss, token, ',');
			size_t i = std::stoi(token);

			std::getline(iss, token, ',');
			size_t j = std::stoi(token);

			std::getline(iss, token, ',');
			double balance_degree = std::stod(token);

			std::vector<double> rates;
			rates.push_back(balance_degree);
			while (std::getline(iss, token, ','))
			{
				rates.push_back(std::stod(token));
			}

			data[{i, j}] = rates;
		}
		inFile.close();

		return data;
	}

	AnswerData readFile4AnswerData(const std::string& filename)
	{
		std::ifstream inFile(filename);
		AnswerData data;

		std::pair<size_t, size_t> outerIndices;
		std::map<std::pair<size_t, size_t>, std::vector<double>> innerMap;

		std::string line;
		while (std::getline(inFile, line))
		{
			std::istringstream iss(line);
			std::vector<std::string> tokens;
			std::string token;

			while (std::getline(iss, token, ','))
			{
				tokens.push_back(token);
			}

			if (tokens.size() == 2) // Outer indices line
			{
				// If innerMap is not empty, save it to the data map
				if (!innerMap.empty())
				{
					data[outerIndices] = innerMap;
					innerMap.clear();
				}

				outerIndices.first = std::stoul(tokens[0]);
				outerIndices.second = std::stoul(tokens[1]);
			}
			else if (tokens.size() > 2) // Inner indices line
			{
				std::pair<size_t, size_t> innerIndices;
				innerIndices.first = std::stoul(tokens[0]);
				innerIndices.second = std::stoul(tokens[1]);

				std::vector<double> rates;
				for (size_t i = 2; i < tokens.size(); ++i)
				{
					rates.push_back(std::stod(tokens[i]));
				}

				innerMap[innerIndices] = rates;
			}
		}

		// Save the last innerMap to the data map
		if (!innerMap.empty())
		{
			data[outerIndices] = innerMap;
		}

		inFile.close();

		return data;
	}

	size_t closestToExpectation(const std::vector<double>& vec1, const std::vector<double>& vec2)
	{
		// 扩展较短的向量以匹配较长的向量的长度
		std::vector<double> v1 = vec1;
		std::vector<double> v2 = vec2;
		if (v1.size() < v2.size())
			v1.resize(v2.size(), 0.0);
		else if (v2.size() < v1.size())
			v2.resize(v1.size(), 0.0);

		// 计算两个向量之间的对应和
		std::vector<double> diff(v1.size());
		for (size_t i = 1; i < v1.size(); ++i)
			diff[i] = v1[i] + v2[i];

		// 找到最大的元素的下标
		size_t maxIndex = std::distance(diff.begin(), std::max_element(diff.begin(), diff.end()));

		// 返回下标
		return maxIndex;
	}

	// 平衡度计算公式
	double balanceDegree(std::vector<double> list_a, std::vector<double> list_b)
	{
		double result = 0.0;
		result = (list_a[0] + list_b[0]) * 0.5;
		return result;
	}

	// 惩罚函数
	bool penalty(size_t i, size_t j)
	{
		// 如果_i, _j已经被选择了3次或更多次，返回true
		if (m_penalty_data[{i, j}] >= 1)
			return true;
		// 否则，增加_i, _j的选择次数并返回false
		else
		{
			++m_penalty_data[{i, j}];
			return false;
		}
	}

	void executePenalty(size_t& _i, size_t& _j)
	{
		m_penalty_data[{_i, _j}] = 0;
		m_penalty_data[{_j, _i}] = 0;
		_i = rand() % max_size;
		_j = rand() % max_size;
		if (_i == _j)
			_j = (_i + 1) % max_size;
	}

	SizeT3 processAccuracy(const AccuracyData& accuracy_datas)
	{
		BalanceData balanceData;
		for (const auto& _pair : accuracy_datas)
		{
			size_t i = _pair.first.first;
			size_t j = _pair.first.second;

			// 如果存在[j,i]的数据，计算平均平衡度并存储
			if (accuracy_datas.find({ j, i }) != accuracy_datas.end())
				balanceData[{i, j}] = balanceDegree(accuracy_datas.at({ i, j }), accuracy_datas.at({ j, i }));
		}

		// 找出新计算方式下的最大值
		auto maxCovPair = *std::max_element(balanceData.begin(), balanceData.end(),
			[](const auto& p1, const auto& p2) { return p1.second < p2.second; });

		size_t _i = maxCovPair.first.first;
		size_t _j = maxCovPair.first.second;

		// 重复选择多次，添加惩罚，转嫁到随机上
		bool isPenal = penalty(_i, _j);
		if (isPenal)
			executePenalty(_i, _j);

		// 挑选最大超验收敛率的组
		auto index_query = closestToExpectation(accuracy_datas.at({ _i, _j }), accuracy_datas.at({ _j, _i }));

		return std::make_tuple(_i, _j, index_query);
	}

	SizeT2 processAnswer(SizeT3 query_info, const AnswerData& answer_datas)
	{
		size_t _i = std::get<0>(query_info), _j = std::get<1>(query_info), query_indice = std::get<2>(query_info);
		auto answer_data = answer_datas.at({ _i, _j });
		size_t max_value = 1;
		double max_rate = -1.0;

		for (const auto& item : answer_data)
		{
			if (item.first.first == query_indice)
			{
				for (const auto& rate : item.second)
				{
					if (rate > max_rate)
					{
						max_rate = rate;
						max_value = item.first.second;
					}
				}
			}
		}

		if (max_value > 10)
		{
			int i = 9;
		}

		auto _answer_data = answer_datas.at({ _j, _i });
		size_t _max_value = 1;
		double _max_rate = -1.0;

		for (const auto& item : _answer_data)
		{
			if (item.first.first == query_indice)
			{
				for (const auto& rate : item.second)
				{
					if (rate > _max_rate)
					{
						_max_rate = rate;
						_max_value = item.first.second;
					}
				}
			}
		}
		if (_max_value > 10)
		{
			int i = 9;
		}
		return std::make_tuple(max_value, _max_value);
	}

	// 生成交互序列
	virtual SizeT7 processingarguments(AccuracyData accuracy_datas, AnswerData answer_datas)
	{
		SizeT3 query_info = processAccuracy(accuracy_datas);

		SizeT2 answer_info = processAnswer(query_info, answer_datas);

		// 读取文件当前的最后一行获取上一行的序号
		std::ifstream inFile("interaction_orders.csv");
		std::string line;
		size_t t = 0;
		if (inFile.peek() != std::ifstream::traits_type::eof())
			while (std::getline(inFile, line))
			{
				std::istringstream iss(line);
				std::string token;
				std::getline(iss, token, ',');
				t = std::stoi(token);
			}
		inFile.close();

		auto result = std::make_tuple(t + 1, std::get<0>(query_info), std::get<1>(query_info), 
			std::get<2>(query_info), std::get<0>(answer_info), 
			std::get<2>(query_info), std::get<1>(answer_info));
 		return result;
	}
private:
	std::map<std::pair<size_t, size_t>, size_t> m_penalty_data;
};

#endif // !DECISION_H
