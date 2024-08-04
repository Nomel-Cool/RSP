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

constexpr double SHRINK_RATIO = 1e20;

template<size_t N>
class virtuality
{
public:
	virtuality()
	{
		m_interactive_elements.resize(N);
		for (size_t i = 0; i < N; ++i)
			m_interactive_elements.at(i) = std::to_string(i + 1);
		m_up_pos = std::make_tuple(0, 1);
	}
	~virtuality()
	{

	}

	/// <summary>
	/// 执行总体回归检测
	/// </summary>
	/// <param name="i"></param>
	/// <param name="j"></param>
	virtual void _interaction(size_t i, size_t j)
	{
		if (i + 1 > N || i + 1 <= 0 || j + 1 <= 0 || j + 1 > N)
			throw;
		double determined_position = _getPosition(i + 1, j + 1) / SHRINK_RATIO;  // 缩小10^10倍
		double grass = std::pow((N - 1) * N / 2, std::get<0>(m_up_pos)) / SHRINK_RATIO;  // 缩小10^10倍
		double positon_ratio = static_cast<double>(determined_position) / grass;
		m_current_ratio = positon_ratio;
	}

	/// <summary>
	/// 执行局部回归检测
	/// </summary>
	/// <param name="i"></param>
	/// <param name="j"></param>
	virtual void interaction(size_t i, size_t j, bool isStored = false, size_t extra_size = 0)
	{
		if (i + 1 > N + extra_size || i + 1 <= 0 || j + 1 <= 0 || j + 1 > N + extra_size)
			throw;
		if (isStored)
			m_stored_makeup_sequence.emplace_back(std::make_pair(i, j));
		double relative_position = getPosition(i + 1, j + 1);
		double grass = (N - 1) * N / 2;
		double positon_ratio = static_cast<double>(relative_position) / grass;
		m_current_ratio = positon_ratio;
	}

	virtual void clearMemory()
	{
		m_stored_makeup_sequence.clear();
	}

	virtual std::vector<std::pair<size_t, size_t>> getInteractSequence()
	{
		return m_stored_makeup_sequence;
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
	virtual size_t _getPosition(size_t i, size_t j)
	{
		if (j < i)
			std::swap(i, j);
		size_t determined_anchor = std::get<1>(m_up_pos);
		size_t relative_position = (((2 * N - i) * (i - 1)) >> 1) + j - i;
		size_t determined_position = (determined_anchor - 1) * ((N * (N - 1)) >> 1) + relative_position;
		m_up_pos = std::make_tuple(std::get<0>(m_up_pos) + 1, determined_position);
		return determined_position;
	}

	virtual size_t getPosition(size_t i, size_t j)
	{
		if (j < i)
			std::swap(i, j);
		size_t relative_position = (((2 * N - i) * (i - 1)) >> 1) + j - i;
		m_up_pos = std::make_tuple(std::get<0>(m_up_pos) + 1, relative_position);
		return relative_position;
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
	std::vector<std::pair<size_t, size_t>> m_stored_makeup_sequence;
	std::stack<std::pair<size_t, size_t> > m_highlighting_elements;
	std::stack<std::bitset<N> > m_coding_status;
	std::tuple<size_t, uint64_t> m_up_pos; // 存储当前形成序的上分位置 <t,pos>
	double m_current_ratio = 0.0;
};

#endif