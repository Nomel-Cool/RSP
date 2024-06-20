#pragma once
#ifndef DECISION_H
#define DECISION_H

#include <tuple>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <cmath>
#include <random>
#include <numeric>
using AccuracyData = std::map<std::pair<size_t, size_t>, std::vector<double>>;
using CovarianceData = std::map<std::pair<size_t, size_t>, double>;
using SizeT5 = std::tuple<size_t, size_t, size_t, size_t, size_t>;

/*概要设计
成员变量：

成员函数：
gainFeedback() 公有 用于读取反馈层写入的交互反馈参数文件
processingarguments() 保护 读取参数文件后调用算法调参生成下一次的交互序列
makeOrders() 公有 将算法生成的交互序列写入文件，供反馈层读取
*/
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

			std::vector<double> rates;
			while (std::getline(iss, token, ','))
			{
				rates.push_back(std::stod(token));
			}

			data[{i, j}] = rates;
		}
		inFile.close();

		// 暂时先用0.5，后面需要读配置文件改变这个参数
		return processingarguments(data, 0.5);
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

	// 计算样本方差
	double variance(const std::vector<double>& X, double expectation)
	{
		double sum = 0;
		for (size_t i = 0; i < X.size(); i++)
			sum += (X[i] - expectation) * (X[i] - expectation);
		return sum / (X.size() - 1);
	}

	size_t closestToExpectation(const std::vector<double>& vec, double expectation) 
	{
		return std::distance(vec.begin(), std::min_element(vec.begin(), vec.end(),
			[expectation](const auto& a, const auto& b){ return std::abs(a - expectation) < std::abs(b - expectation); }));
	}

	// 生成交互序列
	virtual SizeT5 processingarguments(AccuracyData accuracy_datas, double expectation = 0.5)
	{
		CovarianceData covarianceData;
		for (const auto& _pair : accuracy_datas)
		{
			size_t i = _pair.first.first;
			size_t j = _pair.first.second;

			// 如果存在[j,i]的数据，计算协方差并存储
			if (accuracy_datas.find({ j, i }) != accuracy_datas.end())
			{
				double cov = covariance(accuracy_datas[{i, j}], accuracy_datas[{j, i}], expectation);
				double avgVariance = (variance(accuracy_datas[{i, j}], expectation) + variance(accuracy_datas[{j, i}], expectation)) / 2;
				covarianceData[{i, j}] = avgVariance * std::abs(cov);
			}
		}

		// 找出新计算方式下的最大值
		auto maxCovPair = *std::max_element(covarianceData.begin(), covarianceData.end(),
			[](const auto& p1, const auto& p2){ return p1.second < p2.second; });

		size_t _i = maxCovPair.first.first;
		size_t _j = maxCovPair.first.second;

		// 找到离expectation最近的下标，这是作为询问元的下标
		size_t index4j = closestToExpectation(accuracy_datas[{_i, _j}], expectation);
		size_t index4i = closestToExpectation(accuracy_datas[{_j, _i}], expectation);

		// 读取文件当前的最后一行获取上一行的序号
		std::ifstream inFile("interaction_orders.csv");
		std::string line;
		size_t t = 0;
		if (inFile.peek() != std::ifstream::traits_type::eof()) {
			while (std::getline(inFile, line))
			{
				std::istringstream iss(line);
				std::string token;
				std::getline(iss, token, ',');
				t = std::stoi(token);
			}
		}
		inFile.close();

		return std::make_tuple(t + 1, _i, _j, index4i, index4j);
	}
private:
	
};

#endif // !DECISION_H
