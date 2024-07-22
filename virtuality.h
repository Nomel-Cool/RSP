#pragma once
#ifndef VIRTUALITY_H
#define VIRTUALITY_H

#include <vector>
#include <stack>
#include <string>
#include <bitset>
#include <tuple>

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