#pragma once
#ifndef PARSER_H
#define PARSER_H

#include <thread>
#include <chrono>
#include <vector>
#include <map>
#include <string>
#include <iostream>

#include "dispatcher.h"
#include "decision.h"

template<size_t N, size_t max_size, size_t max_value>
class abstractParser
{
public:
	abstractParser()
	{

	}
	~abstractParser()
	{

	}
	void Interaction(
		const std::string& manipulate_node,
		const std::string& accuracy_file,
		const std::string& answer_file,
		const std::string& order_file,
		const std::string& ratio_file,
		const std::string& output_file,
		const std::string& regression_file
	)
	{
		while (m_append_test_count--)
		{
			//ͳ�ƽ��������������ϲ�
			m_dispatch.statisticConvergence(manipulate_node, accuracy_file);
			m_dispatch.statisticAnswering(manipulate_node, answer_file);

			// ��������ƽ���
			auto feed_back = m_decise.gainFeedBack(accuracy_file, answer_file, order_file, ratio_file, regression_file);

			// ����������������
			m_decise.makeOrders(feed_back, manipulate_node);

			// ��ȡ����ָ�ʵ�ֽ���
			m_dispatch.interaction();

			// ����λ��ռ������
			m_dispatch.statisticRatio(manipulate_node, ratio_file);

			// չʾ�������
			m_dispatch.show(manipulate_node, output_file);

			//�ֶ�����
			//system("pause");

			// �Զ�����
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		m_append_test_count = 20;
	}
	std::tuple<std::vector<std::pair<size_t, size_t>>, std::vector<size_t>> ExamModel(
		const std::string& manipulate_node,
		const std::string& exam_accuracy_file,
		const std::string& exam_answer_file,
		const std::string& exam_order_file,
		const std::string& exam_ratio_file,
		const std::string& exam_output_file,
		const std::string& exam_regression_file,
		const size_t& extra_size,
		const size_t& convergence_limit = 5
	)
	{
		// ��ʼ��ʵ�黷��
		m_dispatch.preserver(manipulate_node);
		m_dispatch.examination(max_size, max_value, extra_size); // max_size, max_value, extra_size

		std::tuple<size_t, size_t> first_interactor = std::make_tuple(0, 0);
		std::vector<size_t> recur_w;
		int counter = 0;
		while (true)
		{
			m_dispatch.statisticConvergence(manipulate_node, extra_size, exam_accuracy_file);
			m_dispatch.statisticAnswering(manipulate_node, extra_size, exam_answer_file);
			auto feed_back = m_decise.gainFeedBack(exam_accuracy_file, exam_answer_file, exam_order_file, exam_ratio_file, exam_regression_file);
			if (std::get<0>(first_interactor) == std::get<1>(first_interactor))
				first_interactor = std::make_tuple(std::get<1>(feed_back), std::get<2>(feed_back));
			if ((std::get<0>(first_interactor) == std::get<1>(feed_back) && std::get<1>(first_interactor) == std::get<2>(feed_back)) ||
				(std::get<0>(first_interactor) == std::get<2>(feed_back) && std::get<1>(first_interactor) == std::get<1>(feed_back)))
				counter++;
			m_decise.makeOrders(feed_back, manipulate_node, exam_order_file);
			m_dispatch.interaction(true, exam_order_file, extra_size); // ����recordģʽ
			m_dispatch.statisticRatio(manipulate_node, exam_ratio_file);
			m_dispatch.show(manipulate_node, exam_output_file);

			std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // �Զ�����

			if (counter == convergence_limit)
			{
				counter = 0;
				break; // �����ֵ�2��ʵ��ͷʱ����Ϊ�ﵽ��ƽ���׼
			}
		}
		// ������֪����ƽ���׼�󣬻�ȡvirtuality��¼�ĳ����γ���
		auto sequences = m_dispatch.getInteractions(manipulate_node);

		// ����ģ�͵���������ִ�г����γ���
		for (size_t m = 2; m < sequences.size(); ++m)
		{
			m_dispatch.preserver(manipulate_node);
			for (size_t i = 1; i < m; ++i)
			{
				size_t exam_i = sequences[i].first;
				size_t exam_j = sequences[i].second;
				auto feed_back = m_decise.gainFeedBack(exam_accuracy_file, exam_answer_file, exam_order_file, exam_ratio_file, exam_regression_file, 1, exam_i, exam_j);
				m_decise.makeOrders(feed_back, manipulate_node, exam_order_file);
				m_dispatch.interaction(false, exam_order_file, 1); // ȡ������recordģʽ
				m_dispatch.show(manipulate_node, exam_output_file);
				std::cout << "<" << exam_i << "," << exam_j << ">" << "  |  ";
			}
			std::cout << '\n';
			// ִ����ǰm-1�������󣬼���Ƿ����m��һ��
			auto check_point = sequences[m];
			auto feed_back = m_decise.gainFeedBack(exam_accuracy_file, exam_answer_file, exam_order_file, exam_ratio_file, exam_regression_file);
			if ((check_point.first == std::get<1>(feed_back) && check_point.second == std::get<2>(feed_back)) ||
				(check_point.first == std::get<2>(feed_back) && check_point.second == std::get<1>(feed_back)))
			{
				recur_w.emplace_back(m);
				std::cout << "yes" << '\n';
			}
			m_dispatch.recover(manipulate_node);
		}
		m_dispatch.recover(manipulate_node);
		return std::make_tuple(sequences, recur_w);
	}
	std::map<std::pair<size_t, size_t>, std::vector<std::pair<size_t, size_t>>> classify(std::tuple<std::vector<std::pair<size_t, size_t>>, std::vector<size_t>> raw_datas)
	{
		std::map<std::pair<size_t, size_t>, std::vector<std::pair<size_t, size_t>>> classified_syntax_datas;
		auto sequences = std::get<0>(raw_datas);
		auto stable_indices = std::get<1>(raw_datas);
		for (const auto& i : stable_indices)
		{
			// ʹ��std::vector��insert��������һ����Χ
			classified_syntax_datas[sequences[i]].insert(
				classified_syntax_datas[sequences[i]].end(), // ����λ��
				sequences.begin(), sequences.begin() + i - 1 // ���뷶Χ
			);
		}
		return classified_syntax_datas;
	}

protected:
	size_t m_append_test_count = 20;
private:
	decision<N> m_decise;
	dispatcher<N, max_size, max_value> m_dispatch;
};
#endif // !PARSER_H