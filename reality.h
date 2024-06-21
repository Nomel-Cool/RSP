#pragma once
#ifndef REALITY_H
#define REALITY_H

#include <cstdlib>
#include <ctime>

#include <algorithm> // for std::shuffle
#include <random> // for std::default_random_engine
#include <chrono> // for std::chrono::system_clock

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <set>

#include <fstream> // for file operations
#include <iterator> // for std::ostream_iterator

namespace st
{

    bool cmpFunc(const size_t& a, const size_t& b)
    {
        return a < b;
    }
    /* 概要设计：
    成员变量:
    一个散列表 用于存储交互元，每个交互元都有两个数组ABag和QBag，用于存储实际的交互内容物

    成员函数：
    interaction(i,j) 公有 供网络调度层使用，使得ai与aj进行交互
    show() 公有 展示当前样本空间到Excel表

    */
    template<size_t N>
    class reality
    {
    public:
        reality(size_t max_size, size_t max_value)
        {
            if (max_value == 0 || max_size <= 0)
                return;

            unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

            for (size_t i = 0; i < N; ++i)
            {
                auto rand_size = (rand() % max_size) + 1; // Not Empty Set
                std::vector<size_t> temp_v(rand_size);
                for (size_t j = 0; j < rand_size; ++j)
                    temp_v[j] = j;

                // Shuffle the vector using the default random engine
                std::shuffle(temp_v.begin(), temp_v.end(), std::default_random_engine(seed));
                m_interactive_instances[i] = std::make_pair(temp_v, temp_v);
            }
        }

        ~reality()
        {

        }

        /// <summary>
        /// 实际的交互设计会影响认知收敛率
        /// </summary>
        /// <param name="i"></param>
        /// <param name="j"></param>
        virtual void interaction(size_t i, size_t j, size_t index_i, size_t index_j)
        {
            auto& vi = m_interactive_instances[i];
            auto& vj = m_interactive_instances[j];

            size_t qi = vi.first[rand() % vi.first.size()];
            size_t ai = vi.second[index_i];
            size_t qj = vj.first[rand() % vj.first.size()];
            size_t aj = vj.second[index_j];

            bool viq = false, via = false, vjq = false, vja = false; // 滞后信号量，因为交互是同时发生的，所以QBag和ABag的修改都必须是同时的

            /* vi -> vj */
            size_t result4j = qi + aj;
            auto it_aj = std::find(vj.second.begin(), vj.second.end(), result4j);
            auto it_qj = std::find(vj.first.begin(), vj.first.end(), result4j);
            if (it_aj != vj.second.end()) // 如果解决了问题，应当去查询该结果是否解决了vi的QBag中的问题
            {
                auto it_qi = std::find(vi.first.begin(), vi.first.end(), result4j);
                auto it_ai = std::find(vi.second.begin(), vi.second.end(), result4j);
                if (it_qi != vi.first.end() && it_ai == vi.second.end()) // 如果解决了vi的QBag的问题并且vi的ABag没有这个答案，则更新vi的ABag
                    via = true;
            }
            else // 如果没有解决问题，并且如果QBag中没有相同的问题，则新增一则问题
                if (it_qj == vj.first.end())
                    vjq = true;

            /* vj -> vi  */
            size_t result4i = qj + ai;
            auto it_ai = std::find(vi.second.begin(), vi.second.end(), result4i);
            auto it_qi = std::find(vi.first.begin(), vi.first.end(), result4i);
            if (it_ai != vi.second.end()) // 如果解决了问题，应当去查询该结果是否解决了vj的QBag中的问题
            {
                auto it_qj = std::find(vj.first.begin(), vj.first.end(), result4i);
                auto it_aj = std::find(vj.second.begin(), vj.second.end(), result4i);
                if (it_qj != vj.first.end() && it_aj == vj.second.end()) // 如果解决了vj的QBag的问题并且vj的ABag没有对应的答案，则更新vj的ABag
                    vja = true;
            }
            else // 如果没有解决问题，并且如果vi的QBag中没有相同的问题，则新增一则问题
                if (it_qi == vi.first.end())
                    viq = true;
            
            //滞后更新
            if(viq)    
                vi.first.emplace_back(result4i);
            if (vja)
                vj.second.emplace_back(result4i);
            if(via)
                vi.second.emplace_back(result4j);
            if(vjq)
                vj.first.emplace_back(result4j);
        }

        virtual std::pair<std::vector<size_t>, std::vector<size_t> >getVectors(size_t index)
        {
            if(m_interactive_instances.find(index) != m_interactive_instances.end())
                return m_interactive_instances[index];
        }

        virtual void show()
        {
            std::ofstream file("interaction_output.csv");

            // Write data
            for (auto& pair : m_interactive_instances)
            {
                file << pair.first; // Write the key
                auto& vectors = pair.second;

                align(vectors.first, vectors.second);

                // Write the first vector values
                for (const auto& val : vectors.first)
                {
                    file << "," << val;
                }

                // Write the second vector values
                for (const auto& val : vectors.second)
                {
                    file << "," << val;
                }

                file << "\n"; // Next line for next key-value pair
            }

            file.close(); // Close the file when done
        }
    protected:
        virtual void align(std::vector<size_t>& A, std::vector<size_t>& B) {
            // 将A和B转换为集合，以便进行集合操作
            std::set<size_t> setA(A.begin(), A.end());
            std::set<size_t> setB(B.begin(), B.end());

            // 计算交集
            std::vector<size_t> intersection;
            std::set_intersection(setA.begin(), setA.end(), setB.begin(), setB.end(), std::back_inserter(intersection));

            // 计算差集
            std::vector<size_t> differenceA;
            std::set_difference(setA.begin(), setA.end(), setB.begin(), setB.end(), std::back_inserter(differenceA));

            std::vector<size_t> differenceB;
            std::set_difference(setB.begin(), setB.end(), setA.begin(), setA.end(), std::back_inserter(differenceB));

            // 对交集和差集进行排序
            std::sort(intersection.begin(), intersection.end(), &st::cmpFunc);

            std::sort(differenceA.begin(), differenceA.end(), &st::cmpFunc);

            std::sort(differenceB.begin(), differenceB.end(), &st::cmpFunc);

            // 将结果复制回A和B
            A.clear();
            B.clear();
            A.insert(A.end(), intersection.begin(), intersection.end());
            B.insert(B.end(), intersection.begin(), intersection.end());
            A.insert(A.end(), differenceA.begin(), differenceA.end());
            B.insert(B.end(), differenceB.begin(), differenceB.end());
        }
    private:
        std::map<size_t, std::pair<std::vector<size_t>, std::vector<size_t> > > m_interactive_instances;
    };
}

#endif // !REALITY_H
