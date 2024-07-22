#pragma once
#ifndef VIRTUALITY_H
#define VIRTUALITY_H

#include <vector>
#include <stack>
#include <string>
#include <bitset>
#include <tuple>

#include <Eigen/Dense>

/* 概要设计
成员变量：
一个数组 存放当前交互状态 (a1,a2,...,an)
一个栈 用于表示当前状态的交互元 {(i,j)}
一个栈 用于存储每一个状态在通用语义树的二进制编码
成员函数：
interaction(i,j) 公有 供网络调度层使用，使得ai与aj进行交互
getCode() 公有 供反馈层调用获取二进制编码栈
*/

template<size_t N>
class virtuality
{
public:
	virtuality()
	{
		m_interactive_elements.resize(N);
		for (size_t i = 0; i < N; ++i)
			m_interactive_elements.at(i) = std::to_string(i + 1);
		m_interactor = std::make_tuple(0, 1, 2); // 这样初始化可以保证根节点也能纳入位置量的计算范畴
	}
	~virtuality()
	{

	}
	virtual void interaction(size_t i, size_t j)
	{
		if (i > N || i <= 0 || j <= 0 || j > N)
			return false;
		size_t determined_position = getPosition(i, j);
		size_t grass = (size_t)pos((N - 1) * N / 2, std::get<0>(m_interactor) - 1);
		float positon_ratio = static_cast<float>(determined_position) / grass;
		m_current_ratio = position_ratio;
	}

	virtual float getRatio()
	{
		return m_current_ratio;
	}

	virtual std::vector<std::string> getStatus()
	{
		return m_interactive_elements;
	}

	virtual std::stack<std::bitset<N> > getCode()
	{
		return m_coding_status;
	}

protected:
	virtual size_t getPosition(size_t i, size_t j)
	{
		if (j < i)
			std::swap(i, j);
		size_t a = std::get<1>(m_interactor), b = std::get<2>(m_interactor);
		size_t relative_position = (((2 * N - i) * (i - 1)) << 1) + j - i;
		size_t determined_anchor = ((((2 * N - a) * (a - 1)) << 1) + b - a - 1) * ((N - 1) * N >> 1);
		size_t determined_position = determined_anchor + relative_position;
		m_interactor = std::make_tuple(std::getc<0>(m_interactor) + 1, i, j);
		return determined_position;
	}
	virtual void encoding_bits()
	{
		std::bitset<N> code;
		code.set(N - i), code.set(N - j); // 倒置存放
		code.flip();

		m_coding_status.push(code);
	}
	virtual void encoding_string()
	{
		std::string si = m_interactive_elements.at(i - 1);
		std::string sj = m_interactive_elements.at(j - 1);
		m_interactive_elements.at(i - 1) = (sj + "(" + si + ")" + sj);
		m_interactive_elements.at(j - 1) = (si + "(" + sj + ")" + si);

		std::pair<size_t, size_t> highlighted_p = std::make_pair(i - 1, j - 1);
		m_highlighting_elements.push(highlighted_p);
	}
	virtual float regressionRadius(size_t t, std::vector<float>& i_1, std::vector<float>& i_2, size_t query_indice = 1)
	{
		std::vector<float> times_sequences;
		times_sequences.clear();
		times_sequences.resize(t);
		for (size_t i = 0; i < t; ++i)
			times_sequences.emplace_back(i + 1);
		size_t n = i_1.size();
		if (n != i_2.size())
			throw;
		std::vector<float> radius_sequences;
		radius_sequences.clear();
		radius_sequences.resize(n);
		for (size_t j = 0; j < n; ++j)
			radius_sequences.emplace_back(std::abs(0.5 * (i_1[j] - i_2[j])));
		auto radius_regression_result = calMin2(times_sequences, radius_sequences);
		return std::get<0>(radius_regression_result) * query_indice + std::get<1>(radius_regression_result);
	}
	virtual std::set<std::tuple<int, int>> regressionOrigin(size_t t, std::vector<float>& i_1, std::vector<float>& i_2)
	{
		// 由推导可知拟合直线必过所有样本点的中心
		float average_x = matrix_like_summarize(i_1);
		float average_y = matrix_like_summarize(i_2);
		float average_t = (1 + t) * 0.5;

		// 获取拟合直线的单位方向向量
		auto unit_direct_vec = calMin3(t, i_1, i_2);

		// 求取投影到I_1 O I_2平面上的阴影部分覆盖的非负整数点的并集
		std::set<std::tuple<int, int>> points;

		// 求出各个t对应的圆心
		for (int i = 1; i <= t; ++i)
		{
			auto origin_x = average_x + (i - average_t) * (unit_direct_vec[0] / unit_direct_vec[2]);
			auto origin_y = average_y + (i - average_t) * (unit_direct_vec[1] / unit_direct_vec[2]);

			// 求出对应的预测半径
			auto radius = regressionRadius(t, i_1, i_2, i);

			// 求出覆盖面上所有非负整数点
			for (int x = std::ceil(origin_x - radius); x <= std::floor(origin_x + radius); ++x)
				for (int y = std::ceil(origin_y - radius); y <= std::floor(origin_y + radius); ++y)
					if (std::pow(x - origin_x, 2) + std::pow(y - origin_y, 2) <= std::pow(radius, 2))
						points.insert({ x, y });
		}
		return points;
	}
	virtual float matrix_like_summarize(const std::vector<float>& x, std::vector<float> y = std::vector<float>())
	{
		/* just a projection: x * y' */
		size_t n = x.size();
		if (y.size() == 0 && n != 0)
			y = std::vector<float>(n, 1);
		if (n != y.size())
			throw;
		float sum = 0.0;
		for (size_t i = 0; i < n; ++i)
			sum += x[i] * y[i];
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

		// 构造array_t
		std::vector<float> t;
		t.clear();
		t.resize(_t);
		for (size_t j = 1; j <= _t; ++j)
			t.emplace_back(j);

		/* 求取拟合直线的单位方向向量 */
		Eigen::Matrix3f S = Eigen::Matrix3f::Zero(); // 初始化S为零矩阵

		for (int i = 0; i < _t; i++)
		{
			Eigen::Vector3f Yi(x[i], y[i], t[i]); // 构造向量Yi
			S += Yi * Yi.transpose() - Yi * Yi.transpose(); // 计算每一项并累加到S
		}

		Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> solver(S); // 创建一个用于求解特征值和特征向量的求解器
		Eigen::Vector3f eigenvalues = solver.eigenvalues().real(); // 计算特征值
		Eigen::Matrix3f eigenvectors = solver.eigenvectors(); // 计算特征向量

		int minIndex;
		float minValue = eigenvalues.minCoeff(&minIndex); // 找到最小特征值及其索引

		return eigenvectors.col(minIndex); // 返回最小特征值对应的特征向量就是目标方向向量
	}
	size_t C(size_t n, size_t m)
	{
		size_t ans = 1;
		for (size_t i = 1; i <= m; i++)
			ans = ans * (n - m + i) / i; // 注意一定要先乘再除
		return ans;
	}

private:
	std::vector<std::string> m_interactive_elements;
	std::stack<std::pair<size_t, size_t> > m_highlighting_elements;
	std::stack<std::bitset<N> > m_coding_status;
	std::tuple<size_t, size_t, size_t> m_interactor; // 存储当前形成序：<t,i,j> (j>i)
	float m_current_ratio = 0.0;
};

#endif