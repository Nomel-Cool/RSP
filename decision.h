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

/*��Ҫ���
��Ա������

��Ա������
gainFeedback() ���� ���ڶ�ȡ������д��Ľ������������ļ�
processingarguments() ���� ��ȡ�����ļ�������㷨����������һ�εĽ�������
makeOrders() ���� ���㷨���ɵĽ�������д���ļ������������ȡ
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

		// ��ʱ����0.5��������Ҫ�������ļ��ı��������
		return processingarguments(data, 0.5);
	}

	virtual void makeOrders(SizeT5 arguments)
	{
		// ׷�ӵ��ļ�interaction_orders.csv��β��
		std::ofstream outFile("interaction_orders.csv", std::ios_base::app);
		outFile << std::get<0>(arguments) << ","
			<< 0 // ��ʱֻ����һ��Node�����
			<< "," << std::get<1>(arguments) << "," << std::get<2>(arguments)
			<< "," << std::get<3>(arguments) << "," << std::get<4>(arguments) << "\n";
		outFile.close();
	}

protected:

	// ����Э���ʹ���Ȱ�����expectation�����ԭ��������
	double covariance(const std::vector<double>& X, const std::vector<double>& Y, double expectation = 0.5)
	{
		double sum = 0;
		std::vector<double> statistic4XY;
		for (size_t i = 0; i < X.size(); ++i)
			for (size_t j = 0; j < Y.size(); ++j)
				statistic4XY.emplace_back(X[i] * Y[j]);

		// ʹ����ƫ����
		double meanXY = std::accumulate(statistic4XY.begin(), statistic4XY.end(), 0.00) / statistic4XY.size();
		return meanXY - expectation * expectation;
	}

	// ������������
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

	// ���ɽ�������
	virtual SizeT5 processingarguments(AccuracyData accuracy_datas, double expectation = 0.5)
	{
		CovarianceData covarianceData;
		for (const auto& _pair : accuracy_datas)
		{
			size_t i = _pair.first.first;
			size_t j = _pair.first.second;

			// �������[j,i]�����ݣ�����Э����洢
			if (accuracy_datas.find({ j, i }) != accuracy_datas.end())
			{
				double cov = covariance(accuracy_datas[{i, j}], accuracy_datas[{j, i}], expectation);
				double avgVariance = (variance(accuracy_datas[{i, j}], expectation) + variance(accuracy_datas[{j, i}], expectation)) / 2;
				covarianceData[{i, j}] = avgVariance * std::abs(cov);
			}
		}

		// �ҳ��¼��㷽ʽ�µ����ֵ
		auto maxCovPair = *std::max_element(covarianceData.begin(), covarianceData.end(),
			[](const auto& p1, const auto& p2){ return p1.second < p2.second; });

		size_t _i = maxCovPair.first.first;
		size_t _j = maxCovPair.first.second;

		// �ҵ���expectation������±꣬������Ϊѯ��Ԫ���±�
		size_t index4j = closestToExpectation(accuracy_datas[{_i, _j}], expectation);
		size_t index4i = closestToExpectation(accuracy_datas[{_j, _i}], expectation);

		// ��ȡ�ļ���ǰ�����һ�л�ȡ��һ�е����
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
