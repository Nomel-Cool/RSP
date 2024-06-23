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
using BalanceData = std::map<std::pair<size_t, size_t>, double>;
using SizeT5 = std::tuple<size_t, size_t, size_t, size_t, size_t>;

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

	virtual SizeT5 gainFeedBack()
	{
		AccuracyData data;
		std::ifstream inFile("interaction_accuracy.csv");
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

		return processingarguments(data);
	}

	virtual void makeOrders(SizeT5 arguments)
	{
		// 追加到文件interaction_orders.csv的尾部
		std::ofstream outFile("interaction_orders.csv", std::ios_base::app);
		outFile << std::get<0>(arguments) << ","
			<< 0 // 暂时只考虑一个Node的情况
			<< "," << std::get<1>(arguments) << "," << std::get<2>(arguments)
			<< "," << std::get<3>(arguments) << "," << std::get<4>(arguments) << "\n";
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

	size_t closestToExpectation(const std::vector<double>& vec)
	{
		if (vec.size() < 2)
		{
			throw std::invalid_argument("The input vector must have at least two elements.");
		}

		// 找出除第一个元素外的最大值
		double max_value = *std::max_element(vec.begin() + 1, vec.end());

		// 创建一个包含所有最大值索引的向量
		std::vector<size_t> candidates;
		for (size_t i = 1; i < vec.size(); ++i)
		{
			if (vec[i] == max_value)
			{
				candidates.push_back(i);
			}
		}

		// 从候选索引中随机选择一个
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> distrib(0, candidates.size() - 1);

		return candidates[distrib(gen)];
	}

	// 平衡度计算公式
	double balanceDegree(std::vector<double> list_a, std::vector<double> list_b)
	{
		double result = 0.0;
		result = (list_a[0] + list_b[0]) * 0.5;
		return result;
	}

	// 惩罚函数
	bool penalty(size_t i, size_t j, std::vector<double> list_a, std::vector<double> list_b)
	{
		// 处理异常样本，如果样本点的所有元素都是0，返回负无穷大
		if (std::all_of(list_a.begin(), list_a.end(), [](double i) { return i == 0.0; }) &&
			std::all_of(list_b.begin(), list_b.end(), [](double i) { return i == 0.0; }))
		{
			if (m_penalty_data.find({ i,j }) != m_penalty_data.end())
				if (++m_penalty_data[{i, j}] > 5 || ++m_penalty_data[{j, i}])
					return true;
				else
					return false;
			else
			{
				m_penalty_data[{i, j}] = 0;
				m_penalty_data[{j, i}] = 0;
				return false;
			}
		}
		return false;
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

	// 生成交互序列
	virtual SizeT5 processingarguments(AccuracyData accuracy_datas)
	{
		BalanceData balanceData;
		for (const auto& _pair : accuracy_datas)
		{
			size_t i = _pair.first.first;
			size_t j = _pair.first.second;

			// 如果存在[j,i]的数据，计算平均平衡度并存储
			if (accuracy_datas.find({ j, i }) != accuracy_datas.end())
				balanceData[{i, j}] = balanceDegree(accuracy_datas[{i, j}], accuracy_datas[{j, i}]);
		}

		// 找出新计算方式下的最大值
		auto maxCovPair = *std::max_element(balanceData.begin(), balanceData.end(),
			[](const auto& p1, const auto& p2){ return p1.second < p2.second; });

		size_t _i = maxCovPair.first.first;
		size_t _j = maxCovPair.first.second;

		// 重复选择多次，添加惩罚，转嫁到随机上
		bool isPenal = penalty(_i, _j, accuracy_datas[{_i, _j}], accuracy_datas[{_j, _i}]);
		if (isPenal)
			executePenalty(_i, _j);

		// 挑选最大超验收敛率的组
		size_t index4i = closestToExpectation(accuracy_datas[{_i, _j}]);
		size_t index4j = closestToExpectation(accuracy_datas[{_j, _i}]);

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

		return std::make_tuple(t + 1, _i, _j, index4i, index4j);
	}
private:
	std::map<std::pair<size_t, size_t>, size_t> m_penalty_data;
};

#endif // !DECISION_H
