#pragma once
#ifndef REALITY_H
#define REALITY_H

#include <cstdlib>
#include <ctime>

#include <algorithm> // for std::shuffle
#include <random> // for std::default_random_engine
#include <chrono> // for std::chrono::system_clock
#include <numeric>

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <set>
#include <stack>

#include <fstream> // for file operations
#include <iterator> // for std::ostream_iterator
#include <sstream>

bool cmpFunc(std::string a, std::string b)
{
    return std::stoul(a) < std::stoul(b);
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
    /// <summary>
    /// 构造函数
    /// 可能必须保证有0与1，才能保证收敛？
    /// </summary>
    /// <param name="max_size">Bag的最大容量</param>
    /// <param name="max_value">Bag中的最大数值</param>
    reality(size_t max_size, size_t max_value)
    {
        if (max_value == 0 || max_size <= 0)
            return;

        std::srand(std::time(nullptr));

        std::default_random_engine generator;
        std::uniform_int_distribution<size_t> distribution(0, max_value);

        for (size_t i = 0; i < N; ++i)
        {
            auto rand_size = (rand() % max_size); // Not Empty Set
            m_interactive_instances[i].resize(rand_size);

            m_interactive_instances[i].clear();
            m_interactive_instances[i].reserve(rand_size + 2); // +2 for the two elements you're adding

            for (size_t j = 0; j < rand_size; ++j)
            {
                size_t rand_value = distribution(generator);
                m_interactive_instances[i].emplace_back(std::make_pair(rand_value, std::to_string(rand_value)));
            }

            // 使满足认知收敛的必要条件
            m_interactive_instances[i].emplace_back(std::make_pair(0, "0"));
            m_interactive_instances[i].emplace_back(std::make_pair(1, "1"));

            // 一定要去重，不然后续很难利用集合计算Pfi
            // 对 m_interactive_instances[i] 进行排序
            std::sort(m_interactive_instances[i].begin(), m_interactive_instances[i].end());

            // 删除 m_interactive_instances[i] 中的重复元素
            auto it = std::unique(m_interactive_instances[i].begin(), m_interactive_instances[i].end());
            m_interactive_instances[i].erase(it, m_interactive_instances[i].end());

            sortByExprLen();
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
    virtual void interaction(size_t i, size_t j, size_t _query_i, size_t _answer_i, size_t _query_j, size_t _answer_j)
    {
        auto& vi = m_interactive_instances[i];
        auto& vj = m_interactive_instances[j];

        size_t converted_index_qi = convertIndex4Q(vi, _query_i);
        size_t converted_index_qj = convertIndex4Q(vj, _query_j);
        size_t converted_index_ai = convertIndex4A(vi, _answer_i);
        size_t converted_index_aj = convertIndex4A(vj, _answer_j);

        size_t query_i = vi[converted_index_qi].first;
        std::string answer_i = vi[converted_index_ai].second;
        size_t query_j = vj[converted_index_qj].first;
        std::string answer_j = vj[converted_index_aj].second;

        std::vector<std::pair<size_t,std::string> >::iterator storage_it_qi = vi.end(), storage_it_qj = vj.end();
        bool update_it_aj = false, update_it_ai = false;

        /* vi -> vj */
        size_t result4j = query_i + parseAdditionExpr(answer_j);
        auto it_qj = std::find_if(vj.begin(), vj.end(), [result4j](const auto& p) { return p.first == result4j; });
        if (it_qj != vj.end()) // 命中，应该查询qi，是否也有同样的问题
        {
            auto temp_it_qi = std::find_if(vi.begin(), vi.end(), [result4j](const auto& p) { return p.first == result4j; });
            if (temp_it_qi != vi.end()) // 若解决了qi的同样的问题，把对应位置的迭代器存起来，用于更新ai中对应位置的表达式
                storage_it_qi = temp_it_qi;
        }
        else // 没命中，应该向qj放入result4j并向aj中放入表达式"query_i+answer_j"
            update_it_aj = true;

        /* vj -> vi */
        size_t result4i = query_j + parseAdditionExpr(answer_i);
        auto it_qi = std::find_if(vi.begin(), vi.end(), [result4i](const auto& p) { return p.first == result4i; });
        if (it_qi != vi.end()) // 命中，应该查询qj，是否也有同样的问题
        {
            auto temp_it_qj = std::find_if(vj.begin(), vj.end(), [result4i](const auto& p) { return p.first == result4i; });
            if (temp_it_qj != vj.end()) // 若解决了qj的同样的问题，把对应位置的迭代器存起来，用于更新aj中对应位置的表达式
                storage_it_qj = temp_it_qj;
        }
        else // 没命中，应该向qi放入result4i并向aj中放入表达式"query_j+answer_i"
            update_it_ai = true;

        // 延迟更新
        if (storage_it_qi != vi.end())
        {
            auto dis4i = std::distance(vi.begin(), storage_it_qi);
            auto dis4j = std::distance(vj.begin(), it_qj);
            auto it_ai = vi.begin() + dis4i;
            auto it_aj = vj.begin() + dis4j;
            auto rank_i = std::count(it_ai->second.begin(), it_ai->second.end(), '+');
            auto rank_j = std::count(it_aj->second.begin(), it_aj->second.end(), '+');
            it_ai->second = rank_i < rank_j ? it_ai->second : it_aj->second;
        }
        if (storage_it_qj != vj.end())
        {
            auto dis4j = std::distance(vj.begin(), storage_it_qj);
            auto dis4i = std::distance(vi.begin(), it_qi);
            auto it_aj = vj.begin() + dis4j;
            auto it_ai = vi.begin() + dis4i;
            auto rank_i = std::count(it_ai->second.begin(), it_ai->second.end(), '+');
            auto rank_j = std::count(it_aj->second.begin(), it_aj->second.end(), '+');
            it_aj->second = rank_j < rank_i ? it_aj->second : it_ai->second;
        }
        if (update_it_aj)
        {
            vj.emplace_back(std::make_pair(result4j, std::to_string(query_i) + "+" + answer_j));
            sortByExprLen();
        }
        if (update_it_ai)
        {
            vi.emplace_back(std::make_pair(result4i, std::to_string(query_j) + "+" + answer_i));
            sortByExprLen();
        }
    }

    virtual std::vector<std::pair<size_t, std::string>>getDataPairs(size_t index)
    {
        if(m_interactive_instances.find(index) != m_interactive_instances.end())
            return m_interactive_instances[index];
        return std::vector<std::pair<size_t, std::string>>();
    }

    virtual void show(const std::string& ouput_file = "interaction_output.csv")
    {
        std::ofstream file(ouput_file);

        // Write data
        for (auto& pair : m_interactive_instances)
        {
            file << pair.first; // Write the key
            auto& data_pair = pair.second;

            std::vector<size_t> QBag;
            std::vector<std::string> ABag;

            for (const auto& val : data_pair)
            {
                QBag.emplace_back(val.first);
                ABag.emplace_back(val.second);
            }

            for (const auto& q : QBag)
            {
                file << "," + std::to_string(q);
            }

            for (const auto& a : ABag)
            {
                file << "," + a;
            }

            file << "\n"; // Next line for next key-value pair
        }

        file.close(); // Close the file when done
    }

    virtual void tag()
    {
        m_interactive_backup.push(m_interactive_instances);
    }

    virtual void rollBack()
    {
        if (m_interactive_backup.empty())
            return;
        m_interactive_instances = m_interactive_backup.top();
        m_interactive_backup.pop();
    }

    virtual size_t getInstanceSize()
    {
        return m_interactive_instances.size();
    }

    virtual void pushBackward(size_t key, std::pair<size_t, std::string> value)
    {
        m_interactive_instances[key].emplace_back(value);
    }
protected: 
    size_t parseAdditionExpr(const std::string& str)
    {
        size_t sum = 0;
        std::istringstream iss(str);
        std::string part;
        while (std::getline(iss, part, '+'))
            sum += std::stoul(part);
        return sum;
    }

    virtual void sortByExprLen()
    {
        for (auto& pair : m_interactive_instances)
        {
            std::sort(pair.second.begin(), pair.second.end(),
                [](const auto& a, const auto& b)
                {
                    // 计算 "+" 的数量
                    auto countPlus =
                        [](const auto& str)
                        {
                            return std::count(str.begin(), str.end(), '+');
                        };

                    int countA = countPlus(a.second);
                    int countB = countPlus(b.second);

                    if (countA != countB)
                        return countA < countB; // "+" 的数量不同，按数量排序
                    else
                        return a.first < b.first; // "+" 的数量相同，按第一分量排序
                });
        }
    }

    virtual size_t convertIndex4A(const std::vector<std::pair<size_t, std::string>>& data_pair, size_t index)
    {
        auto it_Ak_begin = std::find_if(data_pair.begin(), data_pair.end(),
            [index](const std::pair<size_t, std::string>& item)
            {
                return std::count(item.second.begin(), item.second.end(), '+') == 0;
            });
        auto it_Ak_end = std::find_if(data_pair.begin(), data_pair.end(),
            [index](const std::pair<size_t, std::string>& item)
            {
                return std::count(item.second.begin(), item.second.end(), '+') == index;
            });
        std::random_device rd;
        std::mt19937 gen(rd());
        if (it_Ak_end != data_pair.begin())
        {
            // 生成一个在 [it_Ak_begin, it_Ak_end - 1] 范围内的随机下标
            std::uniform_int_distribution<> distrib(it_Ak_begin - data_pair.begin(), it_Ak_end - data_pair.begin() - 1);
            return distrib(gen);
        }
    }

    virtual size_t convertIndex4Q(const std::vector<std::pair<size_t, std::string>>& data_pair, size_t index)
    {
        auto it_Ak_begin = std::find_if(data_pair.begin(), data_pair.end(),
            [index](const std::pair<size_t, std::string>& item)
            {
                return std::count(item.second.begin(), item.second.end(), '+') == index - 1;
            });
        auto it_Ak_end = std::find_if(data_pair.begin(), data_pair.end(),
            [index](const std::pair<size_t, std::string>& item)
            {
                return std::count(item.second.begin(), item.second.end(), '+') == index;
            });
        std::random_device rd;
        std::mt19937 gen(rd());
        if (it_Ak_end != data_pair.begin())
        {
            // 生成一个在 [it_Ak_begin, it_Ak_end - 1] 范围内的随机下标
            std::uniform_int_distribution<> distrib(it_Ak_begin - data_pair.begin(), it_Ak_end - data_pair.begin() - 1);
            return distrib(gen);
        }
    }
private:
    // <交互元ID,{<QBag_i,ABag_i>}>
    std::map<size_t, std::vector<std::pair<size_t, std::string>>> m_interactive_instances;

    std::stack<std::map<size_t, std::vector<std::pair<size_t, std::string>>>> m_interactive_backup;
};
#endif // !REALITY_H