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

	std::pair<size_t, size_t> closestToExpectation(const std::vector<double>& vec1, const std::vector<double>& vec2)
	{
		// ��չ�϶̵�������ƥ��ϳ��������ĳ���
		std::vector<double> v1 = vec1;
		std::vector<double> v2 = vec2;
		if (v1.size() < v2.size())
			v1.resize(v2.size(), 0.0);
		else if (v2.size() < v1.size())
			v2.resize(v1.size(), 0.0);

		// ������������֮��Ķ�Ӧ��
		std::vector<double> diff(v1.size());
		for (size_t i = 1; i < v1.size(); ++i)
			diff[i] = v1[i] + v2[i];

		// �ҵ�����Ԫ�ص��±�
		size_t maxIndex = std::distance(diff.begin(), std::max_element(diff.begin(), diff.end()));

		// �����±� + 1
		return std::make_pair(
			maxIndex < vec1.size() ? maxIndex : 1,
			maxIndex < vec2.size() ? maxIndex : 1
		);
	}

	// ƽ��ȼ��㹫ʽ
	double balanceDegree(std::vector<double> list_a, std::vector<double> list_b)
	{
		double result = 0.0;
		result = (list_a[0] + list_b[0]) * 0.5;
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

	void executePenalty(size_t& _i, size_t& _j)
	{
		m_penalty_data[{_i, _j}] = 0;
		m_penalty_data[{_j, _i}] = 0;
		_i = rand() % max_size;
		_j = rand() % max_size;
		if (_i == _j)
			_j = (_i + 1) % max_size;
	}

	// ���ɽ�������
	virtual SizeT5 processingarguments(AccuracyData accuracy_datas)
	{
		BalanceData balanceData;
		for (const auto& _pair : accuracy_datas)
		{
			size_t i = _pair.first.first;
			size_t j = _pair.first.second;

			// �������[j,i]�����ݣ�����ƽ��ƽ��Ȳ��洢
			if (accuracy_datas.find({ j, i }) != accuracy_datas.end())
				balanceData[{i, j}] = balanceDegree(accuracy_datas[{i, j}], accuracy_datas[{j, i}]);
		}

		// �ҳ��¼��㷽ʽ�µ����ֵ
		auto maxCovPair = *std::max_element(balanceData.begin(), balanceData.end(),
			[](const auto& p1, const auto& p2){ return p1.second < p2.second; });

		size_t _i = maxCovPair.first.first;
		size_t _j = maxCovPair.first.second;

		// �ظ�ѡ���Σ���ӳͷ���ת�޵������
		bool isPenal = penalty(_i, _j, accuracy_datas[{_i, _j}], accuracy_datas[{_j, _i}]);
		if (isPenal)
			executePenalty(_i, _j);

		// ��ѡ����������ʵ���
		auto index_both = closestToExpectation(accuracy_datas[{_i, _j}], accuracy_datas[{_j, _i}]);

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

		return std::make_tuple(t + 1, _i, _j, index_both.first, index_both.second);
	}
private:
	std::map<std::pair<size_t, size_t>, size_t> m_penalty_data;
};

#endif // !DECISION_H
