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

constexpr size_t CONVERGENCY_LIMIT = 10; // 目前认为10次就收敛

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

	virtual SizeT7 gainFeedBack(const std::string& accuracy_file,
		const std::string& answer_file, 
		const std::string& order_file, 
		const std::string& ratio_file, 
		const std::string& regression_file)
	{
		/* 【Begin】 ******** 处理reality相关反馈 *********/
		AccuracyData accuracy_data = readFile4AccuracyData(accuracy_file);
		AnswerData answer_data = readFile4AnswerData(answer_file);
		auto result = processingarguments(accuracy_data, answer_data, order_file);
		/* 【Finish】 ******** 处理reality相关反馈 *********/

		/* 【Begin】 ******** 处理virtuality相关反馈 *********/
		PositionRatioData position_ratio_data = readFile4PositionRatioData(ratio_file);
		bool is_boundary_converged = false;
		if(!position_ratio_data.empty())
			is_boundary_converged = analyseInteractionBoundary(*position_ratio_data.rbegin());
		if (position_ratio_data.size() >= 2) // 保证起码能够执行线性回归分析
		{
			auto analyse_result = analyseRegression(position_ratio_data);
			processRegression(analyse_result, regression_file);
		}
		if (is_boundary_converged)
		{

		}
		/* 【Finish】 ******** 处理virtuality相关反馈 *********/
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
		/* 【Begin】 ******** 处理reality相关反馈 *********/
		AccuracyData accuracy_data = readFile4AccuracyData(accuracy_file);
		AnswerData answer_data = readFile4AnswerData(answer_file);
		auto result = processingarguments(accuracy_data, answer_data, order_file, exam_i, exam_j);
		/* 【Finish】 ******** 处理reality相关反馈 *********/

		/* 【Begin】 ******** 处理virtuality相关反馈 *********/
		PositionRatioData position_ratio_data = readFile4PositionRatioData(ratio_file);
		bool is_boundary_converged = false;
		if (!position_ratio_data.empty())
			is_boundary_converged = analyseInteractionBoundary(*position_ratio_data.rbegin());
		if (position_ratio_data.size() >= 2) // 保证起码能够执行线性回归分析
		{
			auto analyse_result = analyseRegression(position_ratio_data);
			diffprocessRegression(analyse_result, regression_file);
		}
		if (is_boundary_converged)
		{

		}
		/* 【Finish】 ******** 处理virtuality相关反馈 *********/
		return result;
	}

	virtual void makeOrders(SizeT7 arguments, const std::string& manipulate_item, const std::string& order_file = "interaction_orders.csv")
	{
		// 追加到文件interaction_orders.csv的尾部
		std::ofstream outFile(order_file, std::ios_base::app);
		outFile << std::get<0>(arguments) << ","
			<< manipulate_item // 暂时只考虑一个Node的情况
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

	SizeT4 processAccuracy(const AccuracyData& accuracy_datas)
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
		auto maxUnBalancedPair = *std::max_element(balanceData.begin(), balanceData.end(),
			[](const auto& p1, const auto& p2) { return p1.second < p2.second; });

		size_t _i = maxUnBalancedPair.first.first;
		size_t _j = maxUnBalancedPair.first.second;

		// 重复选择多次，添加惩罚，转嫁到随机上
		bool isPenal = penalty(_i, _j);
		if (isPenal)
			executePenalty(_i, _j);

		// 挑选最大超验收敛率的组
		auto index_query = closestToExpectation(accuracy_datas.at({ _i, _j }), accuracy_datas.at({ _j, _i }));

		return std::make_tuple(_i, _j, index_query.first, index_query.second);
	}

	// 为追加试验算法增加的重载
	SizeT4 processAccuracy(const AccuracyData& accuracy_datas, size_t exam_i, size_t exam_j)
	{
		size_t _i = exam_i;
		size_t _j = exam_j;

		// 重复选择多次，添加惩罚，转嫁到随机上
		bool isPenal = penalty(_i, _j);
		if (isPenal)
			executePenalty(_i, _j);

		// 挑选最大超验收敛率的组
		auto index_query = closestToExpectation(accuracy_datas.at({ _i, _j }), accuracy_datas.at({ _j, _i }));

		return std::make_tuple(_i, _j, index_query.first, index_query.second);
	}

	SizeT2 processAnswer(SizeT4 query_info, const AnswerData& answer_datas)
	{
		size_t _i = std::get<0>(query_info), _j = std::get<1>(query_info), query_indice_i = std::get<2>(query_info), query_indice_j = std::get<3>(query_info);
		
		// 寻找_i寻问时，对应的应答元
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

		// 寻找_j寻问时，对应的应答元
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

	// 调用python脚本画图
	void processRegression(std::tuple<float,float> params, const std::string& regression_param_file = "regression_params.csv")
	{
		float a = std::get<0>(params), b = std::get<1>(params);
		// 将参数写入CSV文件
		std::ofstream outFile(regression_param_file);
		outFile << a << "," << b << std::endl;
		outFile.close();

		// 调用Python脚本
		std::string data_file = "interaction_ratio.csv";
		std::string command = "python plotRegression.py " + regression_param_file + " " + data_file;
		std::system(command.c_str());
	}

	// 生成交互序列
	virtual SizeT7 processingarguments(AccuracyData accuracy_datas, AnswerData answer_datas, const std::string& order_file = "interaction_orders.csv")
	{
		SizeT4 query_info = processAccuracy(accuracy_datas);

		SizeT2 answer_info = processAnswer(query_info, answer_datas);

		// 读取文件当前的最后一行获取上一行的序号
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

	// 重载的交互序列生成函数
	virtual SizeT7 processingarguments(AccuracyData accuracy_datas, AnswerData answer_datas, const std::string& order_file, size_t exam_i, size_t exam_j)
	{
		SizeT4 query_info = processAccuracy(accuracy_datas, exam_i, exam_j);

		SizeT2 answer_info = processAnswer(query_info, answer_datas);

		// 读取文件当前的最后一行获取上一行的序号
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

	// 分析形成序的线性回归性
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

	// 分析交互元上下限变化
	virtual bool analyseInteractionBoundary(double ratio)
	{
		// 判断m_ratio_boundary是否有ratio这个键
		if (m_ratio_boundary.find(ratio) != m_ratio_boundary.end())
			m_ratio_boundary[ratio]++;
		else
			m_ratio_boundary[ratio] = 1;

		// 获取m_ratio_boundary的最小和最大键值
		double min_ratio = m_ratio_boundary.begin()->first;
		double max_ratio = m_ratio_boundary.rbegin()->first;

		// 判断入参的ratio是否有低于当前m_ratio_boundary的下界或者上界
		if (ratio < min_ratio || ratio > max_ratio)
			m_meaning_field_converge++;
		else
			m_meaning_field_converge = 0;

		// 判断当前模型是否已经收敛
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

		// 构造array_t
		std::vector<double> t;
		t.clear();
		t.resize(_t);
		for (size_t j = 1; j <= _t; ++j)
			t.emplace_back(j);

		/* 求取拟合直线的单位方向向量 */
		Eigen::Matrix3d S = Eigen::Matrix3d::Zero(); // 初始化S为零矩阵

		for (int i = 0; i < _t; i++)
		{
			Eigen::Vector3d Yi(x[i], y[i], t[i]); // 构造向量Yi
			S += Yi * Yi.transpose() - Yi * Yi.transpose(); // 计算每一项并累加到S
		}

		Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(S); // 创建一个用于求解特征值和特征向量的求解器
		Eigen::Vector3d eigenvalues = solver.eigenvalues().real(); // 计算特征值
		Eigen::Matrix3d eigenvectors = solver.eigenvectors(); // 计算特征向量

		int minIndex;
		float minValue = eigenvalues.minCoeff(&minIndex); // 找到最小特征值及其索引

		return eigenvectors.col(minIndex); // 返回最小特征值对应的特征向量就是目标方向向量
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
