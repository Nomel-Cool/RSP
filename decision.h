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
using PositionRatioData = std::vector<double>;
using BalanceData = std::map<std::pair<size_t, size_t>, double>;
using SizeT2 = std::tuple<size_t, size_t>;
using SizeT3 = std::tuple<size_t, size_t, size_t>;
using SizeT4 = std::tuple<size_t, size_t, size_t, size_t>;
using SizeT7 = std::tuple<size_t, size_t, size_t, size_t, size_t, size_t, size_t>;

constexpr size_t CONVERGENCY_LIMIT = 10; // Ŀǰ��Ϊ10�ξ�����

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

	virtual SizeT7 gainFeedBack(const std::string& accuracy_file,
		const std::string& answer_file, 
		const std::string& order_file, 
		const std::string& ratio_file, 
		const std::string& regression_file)
	{
		/* ��Begin�� ******** ����reality��ط��� *********/
		AccuracyData accuracy_data = readFile4AccuracyData(accuracy_file);
		AnswerData answer_data = readFile4AnswerData(answer_file);
		auto result = processingarguments(accuracy_data, answer_data, order_file);
		/* ��Finish�� ******** ����reality��ط��� *********/

		/* ��Begin�� ******** ����virtuality��ط��� *********/
		PositionRatioData position_ratio_data = readFile4PositionRatioData(ratio_file);
		bool is_boundary_converged = false;
		if(!position_ratio_data.empty())
			is_boundary_converged = analyseInteractionBoundary(*position_ratio_data.rbegin());
		if (position_ratio_data.size() >= 2) // ��֤�����ܹ�ִ�����Իع����
		{
			auto analyse_result = analyseRegression(position_ratio_data);
			processRegression(analyse_result, regression_file);
		}
		if (is_boundary_converged)
		{

		}
		/* ��Finish�� ******** ����virtuality��ط��� *********/
		return result;
	}

	virtual SizeT7 gainFeedBack(const std::string& accuracy_file,
		const std::string& answer_file,
		const std::string& order_file,
		const std::string& ratio_file,
		const std::string& regression_file,
		size_t exam_i,
		size_t exam_j)
	{
		/* ��Begin�� ******** ����reality��ط��� *********/
		AccuracyData accuracy_data = readFile4AccuracyData(accuracy_file);
		AnswerData answer_data = readFile4AnswerData(answer_file);
		auto result = processingarguments(accuracy_data, answer_data, order_file, exam_i, exam_j);
		/* ��Finish�� ******** ����reality��ط��� *********/

		/* ��Begin�� ******** ����virtuality��ط��� *********/
		PositionRatioData position_ratio_data = readFile4PositionRatioData(ratio_file);
		bool is_boundary_converged = false;
		if (!position_ratio_data.empty())
			is_boundary_converged = analyseInteractionBoundary(*position_ratio_data.rbegin());
		if (position_ratio_data.size() >= 2) // ��֤�����ܹ�ִ�����Իع����
		{
			auto analyse_result = analyseRegression(position_ratio_data);
			diffprocessRegression(analyse_result, regression_file);
		}
		if (is_boundary_converged)
		{

		}
		/* ��Finish�� ******** ����virtuality��ط��� *********/
		return result;
	}

	virtual void makeOrders(SizeT7 arguments, const std::string& manipulate_item, const std::string& order_file = "interaction_orders.csv")
	{
		// ׷�ӵ��ļ�interaction_orders.csv��β��
		std::ofstream outFile(order_file, std::ios_base::app);
		outFile << std::get<0>(arguments) << ","
			<< manipulate_item // ��ʱֻ����һ��Node�����
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

	PositionRatioData readFile4PositionRatioData(const std::string& filename)
	{
		std::ifstream inFile(filename);
		PositionRatioData data;
		std::string line;
		while (std::getline(inFile, line))
		{
			std::stringstream ss(line);
			float value;
			ss >> value;
			data.emplace_back(value);
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

	// Ϊ׷�������㷨���ӵ�����
	SizeT4 processAccuracy(const AccuracyData& accuracy_datas, size_t exam_i, size_t exam_j)
	{
		size_t _i = exam_i;
		size_t _j = exam_j;

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

	// ����python�ű���ͼ
	void processRegression(std::tuple<float,float> params, const std::string& regression_param_file = "regression_params.csv")
	{
		float a = std::get<0>(params), b = std::get<1>(params);
		// ������д��CSV�ļ�
		std::ofstream outFile(regression_param_file);
		outFile << a << "," << b << std::endl;
		outFile.close();

		// ����Python�ű�
		std::string data_file = "interaction_ratio.csv";
		std::string command = "python plotRegression.py " + regression_param_file + " " + data_file;
		std::system(command.c_str());
	}

	// ���ɽ�������
	virtual SizeT7 processingarguments(AccuracyData accuracy_datas, AnswerData answer_datas, const std::string& order_file = "interaction_orders.csv")
	{
		SizeT4 query_info = processAccuracy(accuracy_datas);

		SizeT2 answer_info = processAnswer(query_info, answer_datas);

		// ��ȡ�ļ���ǰ�����һ�л�ȡ��һ�е����
		std::ifstream inFile(order_file);
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

	// ���صĽ����������ɺ���
	virtual SizeT7 processingarguments(AccuracyData accuracy_datas, AnswerData answer_datas, const std::string& order_file, size_t exam_i, size_t exam_j)
	{
		SizeT4 query_info = processAccuracy(accuracy_datas, exam_i, exam_j);

		SizeT2 answer_info = processAnswer(query_info, answer_datas);

		// ��ȡ�ļ���ǰ�����һ�л�ȡ��һ�е����
		std::ifstream inFile(order_file);
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

	// �����γ�������Իع���
	virtual std::tuple<double, double> analyseRegression(PositionRatioData ratio_data)
	{
		size_t n = ratio_data.size();
		std::vector<double> t;
		t.clear();
		for (size_t i = 0; i < n; ++i)
			t.emplace_back(i);
		auto params = calMin2(t, ratio_data);
		return params;
	}

	// ��������Ԫ�����ޱ仯
	virtual bool analyseInteractionBoundary(double ratio)
	{
		// �ж�m_ratio_boundary�Ƿ���ratio�����
		if (m_ratio_boundary.find(ratio) != m_ratio_boundary.end())
			m_ratio_boundary[ratio]++;
		else
			m_ratio_boundary[ratio] = 1;

		// ��ȡm_ratio_boundary����С������ֵ
		double min_ratio = m_ratio_boundary.begin()->first;
		double max_ratio = m_ratio_boundary.rbegin()->first;

		// �ж���ε�ratio�Ƿ��е��ڵ�ǰm_ratio_boundary���½�����Ͻ�
		if (ratio < min_ratio || ratio > max_ratio)
			m_meaning_field_converge++;
		else
			m_meaning_field_converge = 0;

		// �жϵ�ǰģ���Ƿ��Ѿ�����
		if (m_meaning_field_converge == CONVERGENCY_LIMIT)
			return true;
		else
			return false;
	}

	std::tuple<double, double> calMin2(std::vector<double>& x, std::vector<double>& y)
	{
		size_t n = x.size();
		if (n != y.size())
			throw;
		double xbar = matrix_like_summarize(x);
		double x2bar = matrix_like_summarize(x, x);
		double ybar = matrix_like_summarize(y);
		double xybar = matrix_like_summarize(x, y);
		double denominator = n * x2bar - xbar * xbar;
		double a = (n * xybar - xbar * ybar) / denominator;
		double b = (x2bar * ybar - xbar * xybar) / denominator;
		return std::make_tuple(a, b);
	}

	Eigen::Vector3d calMin3(size_t _t, std::vector<double>& x, std::vector<double>& y)
	{
		if (_t != x.size() || _t != y.size()) throw;

		// ����array_t
		std::vector<double> t;
		t.clear();
		t.resize(_t);
		for (size_t j = 1; j <= _t; ++j)
			t.emplace_back(j);

		/* ��ȡ���ֱ�ߵĵ�λ�������� */
		Eigen::Matrix3d S = Eigen::Matrix3d::Zero(); // ��ʼ��SΪ�����

		for (int i = 0; i < _t; i++)
		{
			Eigen::Vector3d Yi(x[i], y[i], t[i]); // ��������Yi
			S += Yi * Yi.transpose() - Yi * Yi.transpose(); // ����ÿһ��ۼӵ�S
		}

		Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(S); // ����һ�������������ֵ�����������������
		Eigen::Vector3d eigenvalues = solver.eigenvalues().real(); // ��������ֵ
		Eigen::Matrix3d eigenvectors = solver.eigenvectors(); // ������������

		int minIndex;
		float minValue = eigenvalues.minCoeff(&minIndex); // �ҵ���С����ֵ��������

		return eigenvectors.col(minIndex); // ������С����ֵ��Ӧ��������������Ŀ�귽������
	}

	virtual double matrix_like_summarize(const std::vector<double>& x, std::vector<double> y = std::vector<double>())
	{
		/* just a projection: x * y' */
		size_t n = x.size();
		if (y.size() == 0 && n != 0)
			y = std::vector<double>(n, 1);
		if (n != y.size())
			throw;
		float sum = 0.0;
		for (size_t i = 0; i < n; ++i)
			sum += x[i] * y[i];
		return sum;
	}
private:
	std::map<std::pair<size_t, size_t>, size_t> m_penalty_data;
	std::map<double, size_t> m_ratio_boundary;
	size_t m_meaning_field_converge = 0;
};

#endif // !DECISION_H
