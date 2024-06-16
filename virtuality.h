#pragma once
#ifndef VIRTUALITY_H
#define VIRTUALITY_H

#include <vector>
#include <stack>
#include <string>
#include <bitset>

/* ��Ҫ���
��Ա������
һ������ ��ŵ�ǰ����״̬ (a1,a2,...,an)
һ��ջ ���ڱ�ʾ��ǰ״̬�Ľ���Ԫ {(i,j)}
һ��ջ ���ڴ洢ÿһ��״̬��ͨ���������Ķ����Ʊ���
��Ա������
interaction(i,j) ���� ��������Ȳ�ʹ�ã�ʹ��ai��aj���н���
getStatus() ���� ����������û�ȡ�����Ʊ���ջ
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
	}
	~virtuality()
	{

	}
	virtual bool interaction(size_t i, size_t j)
	{
		if (i > N || i <= 0 || j <= 0 || j > N)
			return false;

		std::string si = m_interactive_elements.at(i - 1);
		std::string sj = m_interactive_elements.at(j - 1);
		m_interactive_elements.at(i - 1) = (sj + "(" + si + ")" + sj);
		m_interactive_elements.at(j - 1) = (si + "(" + sj + ")" + si);

		std::pair<size_t, size_t> highlighted_p = std::make_pair(i - 1, j - 1);
		m_highlighting_elements.push(highlighted_p);

		std::bitset<N> code;
		code.set(N - i), code.set(N - j); // ���ô��
		code.flip();
		
		m_coding_status.push(code);

		return true;
	}

	virtual std::vector<std::string> getStatus()
	{
		return m_interactive_elements;
	}

	virtual std::stack<std::bitset<N> > getCode()
	{
		return m_coding_status;
	}
private:
	std::vector<std::string> m_interactive_elements;
	std::stack<std::pair<size_t,size_t> > m_highlighting_elements;
	std::stack<std::bitset<N> > m_coding_status;
};

#endif