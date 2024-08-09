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
    dbManager db;
    interaction_param normal_mode =
    {
        normal_node,
        accuracy_file,
        answer_file,
        order_file,
        ratio_file,
        output_file,
        regression_file,
        false,
        20, // 交互深度
        0, // 交互规模
        0,
        0,
        std::vector<std::vector<size_t>>()
    };
    interaction_param exam_mode =
    {
        exam_node,
        exam_accuracy_file,
        exam_answer_file,
        exam_order_file,
        exam_ratio_file,
        exam_output_file,
        exam_regression_file,
        true,
        80, // 交互深度
        1, // 交互规模
        0,
        0,
        std::vector<std::vector<size_t>>()
    };

    /* 追加测试实验 */
    //int n = 2;
    //while (n--)
    //{
    //    parser_factory.SelfIteration(db, normal_mode, exam_mode, 1);
    //    parser_factory.Syntacticalization(exam_mode);
    //    exam_mode.expand_nodes.clear(); // 重新构造新的执行扩张交互元
    //}

    /* 形成序对比实验 */
    int m = 1;
    exam_mode.expand_nodes.push_back({0,1,5});
    while (m--)
    {
        auto explicit_sequences = parser_factory.CollectRawSequences(exam_mode, 3);
        auto dp_3d = parser_factory.FindPattern(explicit_sequences); // 收集模式串，返回三维dp数组
        auto pattern_sequences = parser_factory.AnalysePattern(dp_3d, explicit_sequences);
        for (const auto& p : pattern_sequences)
        {
            for (const auto& pairs : p.second)
            {
                if (pairs.size() == 1)continue;
                for (const auto& p : pairs)
                {
                    std::cout << "<" << p.first << "," << p.second << "> ";
                }
                std::cout << "|";
            }
            std::cout << '\n';
        }
    }

    return 0;
}
