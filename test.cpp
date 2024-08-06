#include "parser.h"

using namespace std;

int main()
{
	std::string normal_node = "normal",
		exam_node = "examing",
		accuracy_file = "interaction_accuracy.csv",
		answer_file = "interaction_answer.csv",
		order_file = "interaction_orders.csv",
		ratio_file = "interaction_ratio.csv",
		output_file = "interaction_output.csv",
		regression_file = "regression_params.csv",
		exam_accuracy_file = "interaction_accuracy_exam.csv",
		exam_answer_file = "interaction_answer_exam.csv",
		exam_order_file = "interaction_orders_exam.csv",
		exam_ratio_file = "interaction_ratio_exam.csv",
		exam_output_file = "interaction_output_exam.csv",
		exam_regression_file = "regression_params_exam.csv";
	abstractParser<5, 12> parser_factory(5);
	parser_factory.Interaction(
		normal_node,
		accuracy_file,
		answer_file,
		order_file,
		ratio_file,
		output_file,
		regression_file,
		false, // 不记录抽象形成序
		20 // 交互深度为20
	);

    // 满足某个条件时进入扩张测试模式
	auto test_result = parser_factory.expandExamModel(
		exam_node,
		exam_accuracy_file,
		exam_answer_file,
		exam_order_file,
		exam_ratio_file,
		exam_output_file,
		exam_regression_file,
		1 // 扩张程度为1
	);

	parser_factory.classify(
		test_result,
		exam_node,
		exam_accuracy_file,
		exam_answer_file,
		exam_order_file,
		exam_ratio_file,
		exam_output_file,
		exam_regression_file
		);
	return 0;
}