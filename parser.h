#pragma once
#ifndef PARSER_H
#define PARSER_H

#include <thread>
#include <chrono>
#include <vector>
#include <map>
#include <string>
#include <unordered_map>

#include <iostream>
#include <utility>
#include <algorithm>

#include "dispatcher.h"
#include "decision.h"
#include "dblink.h"

using StableSequencesData = std::map<std::pair<size_t, size_t>, std::vector<std::vector<std::pair<size_t, size_t>>>>;

struct interaction_param
{
	std::string& manipulate_node;
	std::string& accuracy_file;
	std::string& answer_file;
	std::string& order_file;
	std::string& ratio_file;
	std::string& output_file;
	std::string& regression_file;
	bool isStored;
	size_t interaction_depth; // �������
	size_t interaction_scale; // ������ģ
	size_t specify_i;
	size_t specify_j;
	std::vector<std::vector<size_t>> expand_nodes;
};

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
	void SelfIteration(dbManager& db, interaction_param &noraml_mode, interaction_param &exam_mode, size_t iterate_times = 1)
	{
		// ->  ԭ��ģ�ͽ����������  --->  һ��׷�Ӳ���  --->  �ȶ��γ������洢 --
		while (iterate_times--)
		{
			Interaction(
				noraml_mode.manipulate_node,
				noraml_mode.accuracy_file,
				noraml_mode.answer_file,
				noraml_mode.order_file,
				noraml_mode.ratio_file,
				noraml_mode.output_file,
				noraml_mode.regression_file,
				noraml_mode.isStored, // ����¼�����γ���
				noraml_mode.interaction_depth // �������Ϊ20
			);

			// ����ĳ������ʱ�������Ų���ģʽ
			auto resultant_sequences = expandExamModel(
				exam_mode.manipulate_node,
				exam_mode.accuracy_file,
				exam_mode.answer_file,
				exam_mode.order_file,
				exam_mode.ratio_file,
				exam_mode.output_file,
				exam_mode.regression_file,
				exam_mode.interaction_scale, // ���ų̶�Ϊ1
				exam_mode.expand_nodes
			);

			auto stable_sequences = lexicalization(
				resultant_sequences,
				exam_mode.manipulate_node,
				exam_mode.accuracy_file,
				exam_mode.answer_file,
				exam_mode.order_file,
				exam_mode.ratio_file,
				exam_mode.output_file,
				exam_mode.regression_file
			);
			db.Add(stable_sequences);
		}
	}
	// ͨ���Բ��Ի�����һ����������ȡ�������̵��γ��򣬲����Ⱥ�׺�Ա����ݿ����Ѵ洢���γ����������ĳ����ֵ����Ϊĳ��s�γ���
	void Syntacticalization(interaction_param exam_mode)
	{
		dbManager db;
		auto sequences = expandExamModel(
			exam_mode.manipulate_node,
			exam_mode.accuracy_file,
			exam_mode.answer_file,
			exam_mode.order_file,
			exam_mode.ratio_file,
			exam_mode.output_file,
			exam_mode.regression_file,
			exam_mode.interaction_scale, // ���ų̶�Ϊ1
			exam_mode.expand_nodes
		);
		for (size_t i = 1; i < sequences.size(); ++i)
		{
			auto stable_sequences_set = db.Query(sequences[i]);
			for (const auto& s : stable_sequences_set)
			{
				double satisfiction_rate = suffixComparison(sequences, s.pairs, i - 1);
				db.Update(satisfiction_rate, s.main_map_id, s.vector_index);
			}
		}
	}
	std::vector<std::vector<std::pair<size_t, size_t>>> CollectRawSequences(interaction_param exam_mode, size_t exam_times)
	{
		std::vector<std::vector<std::pair<size_t, size_t>>> explicit_sequences;
		while (exam_times--)
		{
			auto resultant_sequences = expandExamModel(
				exam_mode.manipulate_node,
				exam_mode.accuracy_file,
				exam_mode.answer_file,
				exam_mode.order_file,
				exam_mode.ratio_file,
				exam_mode.output_file,
				exam_mode.regression_file,
				exam_mode.interaction_scale, // ���ų̶�Ϊ1
				exam_mode.expand_nodes,
				exam_mode.interaction_depth, // �������
				0 // ������������Ϊ0����֤������ȼ��γ��򳤶�
			);
			// ˳�ִ洢����ȥ�غ������
			auto explicit_sequence = adjoinUnify(resultant_sequences);
			explicit_sequences.emplace_back(std::move(explicit_sequence));
		}
		return explicit_sequences;
	}
	std::vector<std::vector<std::vector<bool>>> FindPattern(std::vector<std::vector<std::pair<size_t, size_t>>> simplified_sequences) {
		size_t num_samples = simplified_sequences.size();
		std::vector<std::vector<std::vector<bool>>> dp(num_samples);

		for (size_t sample_id = 0; sample_id < num_samples; ++sample_id) {
			size_t sequence_length = simplified_sequences[sample_id].size();
			dp[sample_id] = std::vector<std::vector<bool>>(sequence_length, std::vector<bool>(sequence_length + 1, false));

			for (size_t start = 0; start < sequence_length; ++start) {
				for (size_t length = 1; start + length <= sequence_length; ++length) {
					bool is_common = true;
					for (size_t other_sample_id = 0; other_sample_id < num_samples; ++other_sample_id) {
						if (other_sample_id == sample_id) continue;
						bool found = false;
						for (size_t other_start = 0; other_start + length <= simplified_sequences[other_sample_id].size(); ++other_start) {
							bool match = true;
							for (size_t k = 0; k < length; ++k) {
								if (!arePairsEqual(simplified_sequences[sample_id][start + k], simplified_sequences[other_sample_id][other_start + k])) {
									match = false;
									break;
								}
							}
							if (match) {
								found = true;
								break;
							}
						}
						if (!found) {
							is_common = false;
							break;
						}
					}
					if (is_common) {
						for (size_t l = 1; l <= length; ++l) {
							dp[sample_id][start][l] = false;
						}
						dp[sample_id][start][length] = true;
					}
				}
			}
		}

		return dp;
	}	std::map<size_t, std::vector<std::vector<std::pair<size_t, size_t>>>> AnalysePattern(const std::vector<std::vector<std::vector<bool>>>& dp, const std::vector<std::vector<std::pair<size_t, size_t>>>& simplified_sequences)
	{
		std::map<size_t, std::vector<std::vector<std::pair<size_t, size_t>>>> return_map;

		for (size_t sample_id = 0; sample_id < simplified_sequences.size(); ++sample_id) {
			size_t pos = 0;
			size_t d = simplified_sequences[sample_id].size();

			while (pos < d) {
				bool found = false;
				for (size_t len = d - pos; len > 0; --len) {
					if (pos + len <= dp[sample_id].size() && dp[sample_id][pos][len]) {
						std::vector<std::pair<size_t, size_t>> substring;
						for (size_t k = 0; k < len; ++k) {
							substring.push_back(simplified_sequences[sample_id][pos + k]);
						}
						return_map[sample_id].push_back(substring);
						pos += len;
						found = true;
						break;
					}
				}
				if (!found) {
					++pos; // Move to the next position if no valid length is found
				}
			}
		}

		return return_map;
	}
protected:
	SizeT7 Interaction(
		const std::string& manipulate_node,
		const std::string& accuracy_file,
		const std::string& answer_file,
		const std::string& order_file,
		const std::string& ratio_file,
		const std::string& output_file,
		const std::string& regression_file,
		const bool isStored = false,
		size_t interaction_depth = 1 // �������
	)
	{
		SizeT7 feed_back;
		while (interaction_depth--)
		{
			//ͳ�ƽ��������������ϲ�
			m_dispatch.statisticConvergence(manipulate_node, accuracy_file);
			m_dispatch.statisticAnswering(manipulate_node, answer_file);

			// ��������ƽ���
			feed_back = m_decise.gainFeedBack(accuracy_file, answer_file, order_file, ratio_file, regression_file);

			// ����������������
			m_decise.makeOrders(feed_back, manipulate_node, order_file);

			// ��ȡ����ָ�ʵ�ֽ���
			m_dispatch.interaction(isStored, order_file);

			// ����λ��ռ������
			//m_dispatch.statisticRatio(manipulate_node, ratio_file);

			// չʾ�������
			//m_dispatch.show(manipulate_node, output_file);

			//�ֶ�����
			//system("pause");

			// �Զ�����
			//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		return feed_back; // �������һ�ν�����ķ���
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
		size_t interaction_depth = 1 // �������
	)
	{
		SizeT7 feed_back;
		while (interaction_depth--)
		{
			// ��������ƽ���
			feed_back = m_decise.gainFeedBack(accuracy_file, answer_file, order_file, ratio_file, regression_file, specify_i, specify_j);

			// ����������������
			m_decise.makeOrders(feed_back, manipulate_node, order_file);

			// ��ȡ����ָ�ʵ�ֽ���
			m_dispatch.interaction(isStored, order_file);

			// ����λ��ռ������
			//m_dispatch.statisticRatio(manipulate_node, ratio_file);

			// չʾ�������
			//m_dispatch.show(manipulate_node, output_file);

			//�ֶ�����
			//system("pause");

			// �Զ�����
			//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		return feed_back; // �������һ�ν�����ķ���
	}

	StableSequencesData lexicalization(
		const std::vector<std::pair<size_t, size_t>>& sequences,
		const std::string& manipulate_node,
		const std::string& exam_accuracy_file,
		const std::string& exam_answer_file,
		const std::string& exam_order_file,
		const std::string& exam_ratio_file,
		const std::string& exam_output_file,
		const std::string& exam_regression_file)
	{
		// �����ȶ��γ���
		std::vector<size_t> recur_w;
		// ����ģ�͵���������ִ�г����γ���
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
		std::map<std::pair<size_t, size_t>, std::vector<std::vector<std::pair<size_t, size_t>>>> classified_syntax_datas;
		for (const auto& i : recur_w)
		{
			std::vector<std::pair<size_t, size_t>> section(sequences.begin(), sequences.begin() + i);
			classified_syntax_datas[sequences[i]].push_back(section);
		}
		return classified_syntax_datas;
	}
	double suffixComparison(std::vector<std::pair<size_t, size_t>> raw_sequence, std::vector<std::pair<size_t, size_t>> stable_sequence, size_t anchor_indice)
	{
		size_t match_length = 0;
		size_t stable_length = stable_sequence.size();

		// �� anchor_indice ��ʼ��ǰ���� raw_sequence
		for (size_t i = anchor_indice, j = stable_length - 1; i != SIZE_MAX && j != SIZE_MAX; --i, --j)
			if ((raw_sequence[i].first == stable_sequence[j].first && raw_sequence[i].second == stable_sequence[j].second)
				|| (raw_sequence[i].first == stable_sequence[j].second && raw_sequence[i].second == stable_sequence[j].first))
				++match_length;
			else
				break;
		return static_cast<double>(match_length) / stable_length;
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
		std::vector<std::vector<size_t>>& expand_nodes,
		const size_t& interaction_depth = 1,
		const size_t& convergence_limit = 5
	)
	{
		// �����ֳ�
		m_dispatch.preserver(manipulate_node);
		// ��ʼ��ʵ�黷��
		m_dispatch.examination(max_size, max_value, extra_size, expand_nodes); // max_size, max_value, extra_size
		// ��ʼ�����󽻻�����
		m_decise.updateCurrentScale(expand_nodes.empty() ? extra_size : expand_nodes.size());
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
				true,
				interaction_depth
			);

			if (std::get<0>(first_interactor) == std::get<1>(first_interactor))
				first_interactor = std::make_tuple(std::get<1>(feed_back), std::get<2>(feed_back));

			if ((std::get<0>(first_interactor) == std::get<1>(feed_back) && std::get<1>(first_interactor) == std::get<2>(feed_back)) ||
				(std::get<0>(first_interactor) == std::get<2>(feed_back) && std::get<1>(first_interactor) == std::get<1>(feed_back)))
				counter++;

			if (counter >= convergence_limit)
			{
				counter = 0;
				break; // �����ֵ�2��ʵ��ͷʱ����Ϊ�ﵽ��ƽ���׼
			}
		}
		// ������֪����ƽ���׼�󣬻�ȡvirtuality��¼�ĳ����γ���
		auto sequences = m_dispatch.getInteractions(manipulate_node);

		// ��������γ����¼����
		m_dispatch.cleanCache(manipulate_node);

		// ��ԭ�ֳ�
		m_dispatch.recover(manipulate_node);
		m_decise.updateCurrentScale(expand_nodes.empty() ? -1 * extra_size : -1 * expand_nodes.size());
		return sequences;
	}
	std::vector<std::pair<size_t, size_t>> adjoinUnify(std::vector<std::pair<size_t, size_t>> raw_sequences)
	{
		if (raw_sequences.empty()) return {};

		std::vector<std::pair<size_t, size_t>> result;
		result.push_back(raw_sequences[0]);

		for (size_t i = 1; i < raw_sequences.size(); ++i) {
			auto& last = result.back();
			auto& current = raw_sequences[i];

			if (!((last.first == current.first && last.second == current.second) ||
				(last.first == current.second && last.second == current.first)))
				result.push_back(current);
		}

		return result;
	}
	bool arePairsEqual(const std::pair<size_t, size_t>& p1, const std::pair<size_t, size_t>& p2)
	{
		return (p1.first == p2.first && p1.second == p2.second) || (p1.first == p2.second && p1.second == p2.first);
	}
private:
	decision m_decise;
	dispatcher<max_size, max_value> m_dispatch;
};
#endif //!PARSER_H