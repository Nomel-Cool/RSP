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

template<size_t max_size, size_t max_value>
class abstractParser
{
public:
	abstractParser(size_t N): m_dispatch(N), m_decise(N)
	{

	}
	~abstractParser()
	{

	}
	SizeT7 Interaction(
		const std::string& manipulate_node,
		const std::string& accuracy_file,
		const std::string& answer_file,
		const std::string& order_file,
		const std::string& ratio_file,
		const std::string& output_file,
		const std::string& regression_file,
		const bool isStored = false,
		size_t interaction_depth = 1 // 交互深度
	)
	{
		SizeT7 feed_back;
		while (interaction_depth--)
		{
			//统计交互参数，返回上层
			m_dispatch.statisticConvergence(manipulate_node, accuracy_file);
			m_dispatch.statisticAnswering(manipulate_node, answer_file);

			// 考察整体平衡度
			feed_back = m_decise.gainFeedBack(accuracy_file, answer_file, order_file, ratio_file, regression_file);

			// 接着制作交互控制
			m_decise.makeOrders(feed_back, manipulate_node, order_file);

			// 读取交互指令，实现交互
			m_dispatch.interaction(isStored, order_file);

			// 生成位置占比数据
			m_dispatch.statisticRatio(manipulate_node, ratio_file);

			// 展示交互结果
			m_dispatch.show(manipulate_node, output_file);

			//手动迭代
			//system("pause");

			// 自动迭代
			//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		return feed_back; // 返回最后一次交互后的反馈
	}
	SizeT7 Interaction(
		const std::string& manipulate_node,
		const std::string& accuracy_file,
		const std::string& answer_file,
		const std::string& order_file,
		const std::string& ratio_file,
		const std::string& output_file,
		const std::string& regression_file,
		const size_t specify_i,
		const size_t specify_j,
		const bool isStored = false,
		size_t interaction_depth = 1 // 交互深度
	)
	{
		SizeT7 feed_back;
		while (interaction_depth--)
		{
			// 考察整体平衡度
			feed_back = m_decise.gainFeedBack(accuracy_file, answer_file, order_file, ratio_file, regression_file, specify_i, specify_j);

			// 接着制作交互控制
			m_decise.makeOrders(feed_back, manipulate_node, order_file);

			// 读取交互指令，实现交互
			m_dispatch.interaction(isStored, order_file);

			// 生成位置占比数据
			m_dispatch.statisticRatio(manipulate_node, ratio_file);

			// 展示交互结果
			m_dispatch.show(manipulate_node, output_file);

			//手动迭代
			//system("pause");

			// 自动迭代
			//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		return feed_back; // 返回最后一次交互后的反馈
	}
	std::vector<std::pair<size_t, size_t>> expandExamModel(
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
		// 挂起现场
		m_dispatch.preserver(manipulate_node);
		// 初始化实验环境
		m_dispatch.examination(max_size, max_value, extra_size); // max_size, max_value, extra_size
		// 初始化抽象交互环境
		m_decise.updateCurrentScale(extra_size);
		std::tuple<size_t, size_t> first_interactor = std::make_tuple(0, 0);
		int counter = 0;
		while (true)
		{
			auto feed_back = Interaction(
				manipulate_node,
				exam_accuracy_file,
				exam_answer_file,
				exam_order_file,
				exam_ratio_file,
				exam_output_file,
				exam_regression_file,
				true
			);

			if (std::get<0>(first_interactor) == std::get<1>(first_interactor))
				first_interactor = std::make_tuple(std::get<1>(feed_back), std::get<2>(feed_back));

			if ((std::get<0>(first_interactor) == std::get<1>(feed_back) && std::get<1>(first_interactor) == std::get<2>(feed_back)) ||
				(std::get<0>(first_interactor) == std::get<2>(feed_back) && std::get<1>(first_interactor) == std::get<1>(feed_back)))
				counter++;

			if (counter == convergence_limit)
			{
				counter = 0;
				break; // 当出现第2次实验头时，认为达到了平衡标准
			}
		}
		// 到达认知收敛平衡标准后，获取virtuality记录的抽象形成序
		auto sequences = m_dispatch.getInteractions(manipulate_node);

		// 清楚抽象形成序记录缓存
		m_dispatch.cleanCache(manipulate_node);

		// 还原现场
		m_dispatch.recover(manipulate_node);
		return sequences;
	}
	std::map<std::pair<size_t, size_t>, std::vector<std::pair<size_t, size_t>>> classify(
		const std::vector<std::pair<size_t, size_t>>& sequences,
		const std::string& manipulate_node,
		const std::string& exam_accuracy_file,
		const std::string& exam_answer_file,
		const std::string& exam_order_file,
		const std::string& exam_ratio_file,
		const std::string& exam_output_file,
		const std::string& exam_regression_file)
	{
		// 定义稳定形成序
		std::vector<size_t> recur_w;
		// 利用模型的收敛倾向，执行抽象形成序
		for (size_t m = 2; m < sequences.size(); ++m)
		{
			m_dispatch.preserver(manipulate_node);
			for (size_t i = 1; i < m; ++i)
			{
				size_t exam_i = sequences[i].first;
				size_t exam_j = sequences[i].second;
				auto feed_back = Interaction(
					manipulate_node,
					exam_accuracy_file,
					exam_answer_file,
					exam_order_file,
					exam_ratio_file,
					exam_output_file,
					exam_regression_file,
					exam_i,
					exam_j
				);
				std::cout << "<" << exam_i << "," << exam_j << ">" << "  |  ";
			}
			std::cout << '\n';
			// 执行了前m-1个交互后，检查是否与第m个一致
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
		std::map<std::pair<size_t, size_t>, std::vector<std::pair<size_t, size_t>>> classified_syntax_datas;
		for (const auto& i : recur_w)
		{
			// 使用std::vector的insert函数插入一个范围
			classified_syntax_datas[sequences[i]].insert(
				classified_syntax_datas[sequences[i]].end(), // 插入位置
				sequences.begin(), sequences.begin() + i - 1 // 插入范围
			);
		}
		return classified_syntax_datas;
	}

protected:
private:
	decision m_decise;
	dispatcher<max_size, max_value> m_dispatch;
};
#endif // !PARSER_H