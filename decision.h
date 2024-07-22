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

#include <Eigen/Dense>

using AccuracyData = std::map<std::pair<size_t, size_t>, std::vector<double>>;
using AnswerData = std::map<std::pair<size_t, size_t>, std::map<std::pair<size_t, size_t>, std::vector<double>>>;
using BalanceData = std::map<std::pair<size_t, size_t>, double>;
using SizeT2 = std::tuple<size_t, size_t>;
using SizeT3 = std::tuple<size_t, size_t, size_t>;
using SizeT4 = std::tuple<size_t, size_t, size_t, size_t>;
using SizeT7 = std::tuple<size_t, size_t, size_t, size_t, size_t, size_t, size_t>;

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

	virtual SizeT7 gainFeedBack()
	{
		AccuracyData accuracy_data = readFile4AccuracyData("interaction_accuracy.csv");

		AnswerData answer_data = readFile4AnswerData("interaction_answer.csv");

		auto result = processingarguments(accuracy_data, answer_data);
		
		return result;
	}

	virtual void makeOrders(SizeT7 arguments)
	{
		// ׷�ӵ��ļ�interaction_orders.csv��β��
		std::ofstream outFile("interaction_orders.csv", std::ios_base::app);
		outFile << std::get<0>(arguments) << ","
			<< 0 // ��ʱֻ����һ��Node�����
			<< "," << std::get<1>(arguments) << "," << std::get<2>(arguments)
			<< "," << std::get<3>(arguments) << "," << std::get<4>(arguments)
			<< "," << std::get<5>(arguments) << "," << std::get<6>(arguments) << "\n";
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

	std::pair<size_t, size_t> closestToExpectation(const std::vector<double>& vec1, const std::vector<double>& vec2)
	{
		if (vec1.size() < 2 || vec2.size() < 2)
			throw std::invalid_argument("Vectors must have at least two elements.");

		size_t maxIndex1 = std::distance(vec1.begin(), std::max_element(vec1.begin() + 1, vec1.end()));
		size_t maxIndex2 = std::distance(vec2.begin(), std::max_element(vec2.begin() + 1, vec2.end()));

		return { maxIndex1, maxIndex2 };
	}
	
	// ƽ��ȼ��㹫ʽ
	double balanceDegree(std::vector<double> list_a, std::vector<double> list_b)
	{
		double result = 0.0;
		result = (list_a[0] + list_b[0]) * 0.5;
		return result;
	}

	// �ͷ�����
	bool penalty(size_t i, size_t j)
	{
		// ���_i, _j�Ѿ���ѡ����3�λ����Σ�����true
		if (m_penalty_data[{i, j}] >= 1)
			return true;
		// ��������_i, _j��ѡ�����������false
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

	SizeT4 processAccuracy(const AccuracyData& accuracy_datas)
	{
		BalanceData balanceData;
		for (const auto& _pair : accuracy_datas)
		{
			size_t i = _pair.first.first;
			size_t j = _pair.first.second;

			// �������[j,i]�����ݣ�����ƽ��ƽ��Ȳ��洢
			if (accuracy_datas.find({ j, i }) != accuracy_datas.end())
				balanceData[{i, j}] = balanceDegree(accuracy_datas.at({ i, j }), accuracy_datas.at({ j, i }));
		}

		// �ҳ��¼��㷽ʽ�µ����ֵ
		auto maxUnBalancedPair = *std::max_element(balanceData.begin(), balanceData.end(),
			[](const auto& p1, const auto& p2) { return p1.second < p2.second; });

		size_t _i = maxUnBalancedPair.first.first;
		size_t _j = maxUnBalancedPair.first.second;

		// �ظ�ѡ���Σ���ӳͷ���ת�޵������
		bool isPenal = penalty(_i, _j);
		if (isPenal)
			executePenalty(_i, _j);

		// ��ѡ����������ʵ���
		auto index_query = closestToExpectation(accuracy_datas.at({ _i, _j }), accuracy_datas.at({ _j, _i }));

		return std::make_tuple(_i, _j, index_query.first, index_query.second);
	}

	SizeT2 processAnswer(SizeT4 query_info, const AnswerData& answer_datas)
	{
		size_t _i = std::get<0>(query_info), _j = std::get<1>(query_info), query_indice_i = std::get<2>(query_info), query_indice_j = std::get<3>(query_info);
		
		// Ѱ��_iѰ��ʱ����Ӧ��Ӧ��Ԫ
		auto answer_data_i_to_j = answer_datas.at({ _i, _j });
		size_t max_value_i = 1;
		double max_rate_i = -1.0;

		for (const auto& item : answer_data_i_to_j)
			if (item.first.first == query_indice_i)
				for (const auto& rate : item.second)
					if (rate > max_rate_i)
					{
						max_rate_i = rate;
						max_value_i = item.first.second;
					}

		// Ѱ��_jѰ��ʱ����Ӧ��Ӧ��Ԫ
		auto answer_dataj_to_i = answer_datas.at({ _j, _i });
		size_t max_value_j = 1;
		double max_rate_j = -1.0;

		for (const auto& item : answer_dataj_to_i)
			if (item.first.first == query_indice_j)
				for (const auto& rate : item.second)
					if (rate > max_rate_j)
					{
						max_rate_j = rate;
						max_value_j = item.first.second;
					}

		return std::make_tuple(max_value_i, max_value_j);
	}

	// ���ɽ�������
	virtual SizeT7 processingarguments(AccuracyData accuracy_datas, AnswerData answer_datas)
	{
		SizeT4 query_info = processAccuracy(accuracy_datas);

		SizeT2 answer_info = processAnswer(query_info, answer_datas);

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

		auto result = std::make_tuple(t + 1, 
			std::get<0>(query_info), std::get<1>(query_info), 
			std::get<2>(query_info), std::get<0>(answer_info), 
			std::get<3>(query_info), std::get<1>(answer_info));
 		return result;
	}

	std::tuple<float, float> calMin2(std::vector<float>& x, std::vector<float>& y)
	{
		size_t n = x.size();
		if (n != y.size())
			throw;
		float xbar = matrix_like_summarize(x);
		float x2bar = matrix_like_summarize(x, x);
		float ybar = matrix_like_summarize(y);
		float xybar = matrix_like_summarize(x, y);
		float determined = pow((n * x2bar - xbar * xbar), -1);
		float a = determined * (n * xybar - xbar * ybar);
		float b = determined * (x2bar * ybar - xbar * xybar);
		return std::make_tuple(a, b);
	}

	Eigen::Vector3f calMin3(size_t _t, std::vector<float>& x, std::vector<float>& y)
	{
		if (_t != x.size() || _t != y.size()) throw;

		// ����array_t
		std::vector<float> t;
		t.clear();
		t.resize(_t);
		for (size_t j = 1; j <= _t; ++j)
			t.emplace_back(j);

		/* ��ȡ���ֱ�ߵĵ�λ�������� */
		Eigen::Matrix3f S = Eigen::Matrix3f::Zero(); // ��ʼ��SΪ�����

		for (int i = 0; i < _t; i++)
		{
			Eigen::Vector3f Yi(x[i], y[i], t[i]); // ��������Yi
			S += Yi * Yi.transpose() - Yi * Yi.transpose(); // ����ÿһ��ۼӵ�S
		}

		Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> solver(S); // ����һ�������������ֵ�����������������
		Eigen::Vector3f eigenvalues = solver.eigenvalues().real(); // ��������ֵ
		Eigen::Matrix3f eigenvectors = solver.eigenvectors(); // ������������

		int minIndex;
		float minValue = eigenvalues.minCoeff(&minIndex); // �ҵ���С����ֵ��������

		return eigenvectors.col(minIndex); // ������С����ֵ��Ӧ��������������Ŀ�귽������
	}
private:
	std::map<std::pair<size_t, size_t>, size_t> m_penalty_data;
};

#endif // !DECISION_H
