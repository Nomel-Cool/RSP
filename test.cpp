#include "dispatcher.h"
#include "decision.h"
#include <iostream>
#include <Windows.h>

using namespace std;

int main()
{
	std::string normal_node = "normal",
		exam_node = "examing",
		convergency_file = "interaction_accuracy.csv",
		answer_file = "interaction_answer.csv",
		order_file = "interaction_orders.csv",
		ratio_file = "interaction_ratio.csv",
		output_file = "interaction_output.csv",
		regression_file = "regression_params.csv",
		exam_convergency_file = "interaction_accuracy_exam.csv",
		exam_answer_file = "interaction_answer_exam.csv",
		exam_order_file = "interaction_orders_exam.csv",
		exam_ratio_file = "interaction_ratio_exam.csv",
		exam_output_file = "interaction_output_exam.csv",
		exam_regression_file = "regression_params_exam.csv";
	decision<5> decise;
	dispatcher<5, 1, 5, 12> dispatch;
	size_t n = 20; // 假设20次迭代后进行追加测试
	while(true)
	{
		//统计交互参数，返回上层
		dispatch.statisticConvergence(normal_node, convergency_file);
		dispatch.statisticAnswering(normal_node, answer_file);

		// 考察整体平衡度
		auto feed_back = decise.gainFeedBack(convergency_file, answer_file, order_file, ratio_file, regression_file);

		// 接着制作交互控制
		decise.makeOrders(feed_back, normal_node);

		// 读取交互指令，实现交互
		dispatch.interaction();

		// 生成位置占比数据
		dispatch.statisticRatio(normal_node, ratio_file);

		// 展示交互结果
		dispatch.show(normal_node);

		 //手动迭代
		//system("pause");

		// 自动迭代
		Sleep(1000);
		// 满足某个条件时进入测试模式
		if (!(n--))
		{
			// 初始化实验环境
			dispatch.preserver(exam_node);
			dispatch.examination(12, 5, 1); // max_size, max_value, extra_size

			std::tuple<size_t, size_t> first_interactor = std::make_tuple(0, 0);
			int counter = 0;
			while (true)
			{
				dispatch.statisticConvergence(exam_node, 1, exam_convergency_file);
				dispatch.statisticAnswering(exam_node, 1, exam_answer_file);
				auto feed_back = decise.gainFeedBack(exam_convergency_file, exam_answer_file, exam_order_file, exam_ratio_file, exam_regression_file);
				if (std::get<0>(first_interactor) == std::get<1>(first_interactor))
					first_interactor = std::make_tuple(std::get<1>(feed_back), std::get<2>(feed_back));
				if ((std::get<0>(first_interactor) == std::get<1>(feed_back) && std::get<1>(first_interactor) == std::get<2>(feed_back)) ||
					(std::get<0>(first_interactor) == std::get<2>(feed_back) && std::get<1>(first_interactor) == std::get<1>(feed_back)))
					counter++;
				decise.makeOrders(feed_back, exam_node, exam_order_file);
				dispatch.interaction(true, exam_order_file); // 启动record模式
				dispatch.statisticRatio(exam_node, exam_ratio_file);
				dispatch.show(exam_node, exam_output_file);
				// system("pause"); // 手动迭代

				Sleep(1000); // 自动迭代

				if (counter == 2)
				{
					counter = 0;
					break; // 当出现第2次实验头时，认为达到了平衡标准
				}
			}
			// 到达认知收敛平衡标准后，获取virtuality记录的抽象形成序
			auto sequences = dispatch.getInteractions(exam_node);

			// 利用模型的收敛倾向，执行抽象形成序
			for (size_t m = 1; m < sequences.size(); ++m)
			{
				dispatch.preserver(exam_node);
				for (size_t i = 0; i < m; ++i)
				{
					size_t exam_i = sequences[i].first;
					size_t exam_j = sequences[i].second;
					auto feed_back = decise.gainFeedBack(exam_convergency_file, exam_answer_file, exam_order_file, exam_ratio_file, exam_regression_file, exam_i, exam_j);
					decise.makeOrders(feed_back, exam_node, exam_order_file);
					dispatch.interaction(); // 取消启动record模式
					dispatch.show(exam_node, exam_output_file);
				}
				// 执行了前m-1个交互后，检查是否与第m个一致
				auto check_point = sequences[m];
				auto feed_back = decise.gainFeedBack(exam_convergency_file, exam_answer_file, exam_order_file, exam_ratio_file, exam_regression_file);
				if ((check_point.first == std::get<1>(feed_back) && check_point.second == std::get<2>(feed_back)) ||
					(check_point.first == std::get<2>(feed_back) && check_point.second == std::get<1>(feed_back)))
					std::cout << "yes" << std::endl;
				dispatch.recover(exam_node);
			}
			dispatch.recover(exam_node);
		}
 	}
	//system("clean_csv_contents.bat");
	return 0;
}