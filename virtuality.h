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

class virtuality
{
public:
	virtuality(size_t N)
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
	/// 执行局部回归检测
	/// </summary>
	/// <param name="i"></param>
	/// <param name="j"></param>
	virtual void interaction(size_t i, size_t j, bool isStored = false, size_t extra_size = 0)
	{
		size_t n = m_interactive_elements.size();
		if (i + 1 > n + extra_size || i + 1 <= 0 || j + 1 <= 0 || j + 1 > n + extra_size)
			throw;
		if (isStored)
			m_stored_makeup_sequence.emplace_back(std::make_pair(i, j));
		double relative_position = static_cast<double>(getPosition(i + 1, j + 1));
		double grass = (n - 1) * n / 2;
		double positon_ratio = static_cast<double>(relative_position) / grass;
		m_current_ratio = positon_ratio;
	}

	virtual size_t getInstanceSize()
	{
		return m_interactive_elements.size();
	}

	virtual void tag()
	{
		m_interactive_backup.push(m_interactive_elements);
	}

	virtual void rollBack()
	{
		if (m_interactive_backup.empty())
			return;
		m_interactive_elements = m_interactive_backup.top();
		m_interactive_backup.pop();
	}

	virtual void pushBackward(size_t i)
	{
		m_interactive_elements.emplace_back(std::to_string(i));
	}

	virtual void clearMemory()
	{
		m_stored_makeup_sequence.clear();
	}

	virtual std::vector<std::pair<size_t, size_t>> getInteractSequence()
	{
		return m_stored_makeup_sequence;
	}

	virtual double getRatio()
	{
		return m_current_ratio;
	}

	virtual std::vector<std::string> getStatus()
	{
		return m_interactive_elements;
	}
protected:

	virtual size_t getPosition(size_t i, size_t j)
	{
		if (j < i)
			std::swap(i, j);
		size_t n = m_interactive_elements.size();
		size_t relative_position = (((2 * n - i) * (i - 1)) >> 1) + j - i;
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
	std::stack<std::vector<std::string>> m_interactive_backup;
	std::vector<std::pair<size_t, size_t>> m_stored_makeup_sequence;
	std::stack<std::pair<size_t, size_t> > m_highlighting_elements;
	std::tuple<size_t, uint64_t> m_up_pos; // 存储当前形成序的上分位置 <t,pos>
	double m_current_ratio = 0.0;
};

#endif