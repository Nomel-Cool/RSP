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

/*��Ҫ���
��Ա������

��Ա������
gainFeedback() ���� ���ڶ�ȡ������д��Ľ������������ļ�
processingarguments() ���� ��ȡ�����ļ�������㷨����������һ�εĽ�������
makeOrders() ���� ���㷨���ɵĽ�������д���ļ������������ȡ
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

	// �������������ʹ�ó�ʼ״̬��ʹȫ��0��Ҳ����������ֵ�ϵĲ���
	double variance(const std::vector<double>& X, double expectation)
	{
		double sum = 0;
		for (size_t i = 0; i < X.size(); i++)
			sum += (X[i] - expectation) * (X[i] - expectation);
		return sum / (X.size() - 1);
	}

	size_t closestToExpectation(const std::vector<double>& vec, double expectation)
	{
		std::vector<size_t> candidates;
		double min_diff = std::numeric_limits<double>::max();

		for (size_t i = 0; i < vec.size(); ++i)
		{
			double diff = std::abs(vec[i] - expectation);
			if (diff < min_diff)
			{
				min_diff = diff;
				candidates.clear();
				candidates.push_back(i);
			}
			else if (diff == min_diff)
				candidates.push_back(i);
		}

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> distrib(0, candidates.size() - 1);

		return candidates[distrib(gen)];
	}

	// ƽ��ȼ��㹫ʽ
	double balanceDegree(std::vector<double> list_a, std::vector<double> list_b, double standard_expectation)
	{
		double result = 0.0;
		int n = list_a.size();
		int m = list_b.size();

		// ���� list_a �Ĳ���
		for (int i = 0; i < n; ++i)
			result += (list_a[i] - standard_expectation) * (list_a[i] - standard_expectation) * std::pow(10, -i);

		// ���� list_b �Ĳ���
		for (int j = 0; j < m; ++j)
			result += (list_b[j] - standard_expectation) * (list_b[j] - standard_expectation) * std::pow(10, -j - n);

		return result;
	}

	// �ͷ�����
	bool penalty(size_t i, size_t j, std::vector<double> list_a, std::vector<double> list_b)
	{
		// �����쳣��������������������Ԫ�ض���0�����ظ������
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

	// ���ɽ�������
	virtual SizeT5 processingarguments(AccuracyData accuracy_datas, double expectation = 0.5)
	{
		BalanceData balanceData;
		for (const auto& _pair : accuracy_datas)
		{
			size_t i = _pair.first.first;
			size_t j = _pair.first.second;

			// �������[j,i]�����ݣ�����Э����洢
			if (accuracy_datas.find({ j, i }) != accuracy_datas.end())
				balanceData[{i, j}] = balanceDegree(accuracy_datas[{i, j}], accuracy_datas[{j, i}], expectation);
		}

		// �ҳ��¼��㷽ʽ�µ����ֵ
		auto maxCovPair = *std::max_element(balanceData.begin(), balanceData.end(),
			[](const auto& p1, const auto& p2){ return p1.second < p2.second; });

		size_t _i = maxCovPair.first.first;
		size_t _j = maxCovPair.first.second;

		bool isPenal = penalty(_i, _j, accuracy_datas[{_i, _j}], accuracy_datas[{_j, _i}]);
		if (isPenal)
		{
			m_penalty_data[{_i, _j}] = 0;
			m_penalty_data[{_j, _i}] = 0;
			_i = rand() % max_size;
			_j = rand() % max_size;
			if (_i == _j)
				_j = (_i + 1) % max_size;
		}

		// �ҵ���expectation������±꣬������Ϊѯ��Ԫ���±�
		size_t index4j = closestToExpectation(accuracy_datas[{_i, _j}], expectation);
		size_t index4i = closestToExpectation(accuracy_datas[{_j, _i}], expectation);

		// ��ȡ�ļ���ǰ�����һ�л�ȡ��һ�е����
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
