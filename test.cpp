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
	abstractParser<5, 5, 12> parser_factory;
	parser_factory.Interaction(
		normal_node,
		accuracy_file,
		answer_file,
		order_file,
		ratio_file,
		output_file,
		regression_file
	);

    // 满足某个条件时进入测试模式
	auto test_result = parser_factory.ExamModel(
		exam_node,
		exam_accuracy_file,
		exam_answer_file,
		exam_order_file,
		exam_ratio_file,
		exam_output_file,
		exam_regression_file,
		1
	);

	return 0;
}