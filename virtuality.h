#pragma once
#ifndef VIRTUALITY_H
#define VIRTUALITY_H

#include <vector>
#include <stack>
#include <string>
#include <bitset>
#include <tuple>

/* ��Ҫ���
��Ա������
һ������ ��ŵ�ǰ����״̬ (a1,a2,...,an)
һ��ջ ���ڱ�ʾ��ǰ״̬�Ľ���Ԫ {(i,j)}
һ��ջ ���ڴ洢ÿһ��״̬��ͨ���������Ķ����Ʊ���
��Ա������
interaction(i,j) ���� ��������Ȳ�ʹ�ã�ʹ��ai��aj���н���
getCode() ���� ����������û�ȡ�����Ʊ���ջ
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
		m_up_pos = std::make_tuple(0, 1);
	}
	~virtuality()
	{

	}
	virtual void interaction(size_t i, size_t j)
	{
		if (i + 1 > N || i + 1 <= 0 || j + 1 <= 0 || j + 1 > N)
			throw;
		uint64_t determined_position = getPosition(i + 1, j + 1);
		uint64_t grass = (uint64_t)std::pow((N - 1) * N / 2, std::get<0>(m_up_pos));
		float positon_ratio = static_cast<float>(determined_position) / grass;
		m_current_ratio = positon_ratio;
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
		size_t determined_anchor = std::get<1>(m_up_pos);
		size_t relative_position = (((2 * N - i) * (i - 1)) >> 1) + j - i;
		size_t determined_position = (determined_anchor - 1) * ((N * (N - 1)) >> 1) + relative_position;
		m_up_pos = std::make_tuple(std::get<0>(m_up_pos) + 1, determined_position);
		return determined_position;
	}
	size_t C(size_t n, size_t m)
	{
		size_t ans = 1;
		for (size_t i = 1; i <= m; i++)
			ans = ans * (n - m + i) / i; // ע��һ��Ҫ�ȳ��ٳ�
		return ans;
	}

private:
	std::vector<std::string> m_interactive_elements;
	std::stack<std::pair<size_t, size_t> > m_highlighting_elements;
	std::stack<std::bitset<N> > m_coding_status;
	std::tuple<size_t, uint64_t> m_up_pos; // �洢��ǰ�γ�����Ϸ�λ�� <t,pos>
	double m_current_ratio = 0.0;
};

#endif