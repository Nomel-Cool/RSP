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

#include <fstream> // for file operations
#include <iterator> // for std::ostream_iterator
#include <sstream>

bool cmpFunc(std::string a, std::string b)
{
    return std::stoul(a) < std::stoul(b);
}
/* ��Ҫ��ƣ�
��Ա����:
һ��ɢ�б� ���ڴ洢����Ԫ��ÿ������Ԫ������������ABag��QBag�����ڴ洢ʵ�ʵĽ���������

��Ա������
interaction(i,j) ���� ��������Ȳ�ʹ�ã�ʹ��ai��aj���н���
show() ���� չʾ��ǰ�����ռ䵽Excel��

*/
template<size_t N>
class reality
{
public:
    /// <summary>
    /// ���캯��
    /// ���ܱ��뱣֤��0��1�����ܱ�֤������
    /// </summary>
    /// <param name="max_size">Bag���������</param>
    /// <param name="max_value">Bag�е������ֵ</param>
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

            // ʹ������֪�����ı�Ҫ����
            m_interactive_instances[i].emplace_back(std::make_pair(0, "0"));
            m_interactive_instances[i].emplace_back(std::make_pair(1, "1"));

            // һ��Ҫȥ�أ���Ȼ�����������ü��ϼ���Pfi
            // �� m_interactive_instances[i] ��������
            std::sort(m_interactive_instances[i].begin(), m_interactive_instances[i].end());

            // ɾ�� m_interactive_instances[i] �е��ظ�Ԫ��
            auto it = std::unique(m_interactive_instances[i].begin(), m_interactive_instances[i].end());
            m_interactive_instances[i].erase(it, m_interactive_instances[i].end());

            std::sort(m_interactive_instances[i].begin(), m_interactive_instances[i].end(), 
                [](const auto& p1, const auto& p2) 
                {
                    return p1.first < p2.first;
                });
        }
    }

    ~reality()
    {

    }

    /// <summary>
    /// ʵ�ʵĽ�����ƻ�Ӱ����֪������
    /// </summary>
    /// <param name="i"></param>
    /// <param name="j"></param>
    virtual void interaction(size_t i, size_t j, size_t index_i, size_t index_j)
    {
        auto& vi = m_interactive_instances[i];
        auto& vj = m_interactive_instances[j];

        size_t query_i = vi[rand() % vi.size()].first;
        std::string answer_i = vi[index_i].second;
        size_t query_j = vj[rand() % vj.size()].first;
        std::string answer_j = vj[index_j].second;

        std::vector<std::pair<size_t,std::string> >::iterator storage_it_qi = vi.end(), storage_it_qj = vj.end();
        bool update_it_aj = false, update_it_ai = false;

        /* vi -> vj */
        size_t result4j = query_i + parseAdditionExpr(answer_j);
        auto it_qj = std::find_if(vj.begin(), vj.end(), [result4j](const auto& p) { return p.first == result4j; });
        if (it_qj != vj.end()) // ���У�Ӧ�ò�ѯqi���Ƿ�Ҳ��ͬ��������
        {
            auto temp_it_qi = std::find_if(vi.begin(), vi.end(), [result4j](const auto& p) { return p.first == result4j; });
            if (temp_it_qi != vi.end()) // �������qi��ͬ�������⣬�Ѷ�Ӧλ�õĵ����������������ڸ���ai�ж�Ӧλ�õı��ʽ
                storage_it_qi = temp_it_qi;
        }
        else // û���У�Ӧ����qj����result4j����aj�з�����ʽ"query_i+answer_j"
            update_it_aj = true;

        /* vj -> vi */
        size_t result4i = query_j + parseAdditionExpr(answer_i);
        auto it_qi = std::find_if(vi.begin(), vi.end(), [result4i](const auto& p) { return p.first == result4i; });
        if (it_qi != vi.end()) // ���У�Ӧ�ò�ѯqj���Ƿ�Ҳ��ͬ��������
        {
            auto temp_it_qj = std::find_if(vj.begin(), vj.end(), [result4i](const auto& p) { return p.first == result4i; });
            if (temp_it_qj != vj.end()) // �������qj��ͬ�������⣬�Ѷ�Ӧλ�õĵ����������������ڸ���aj�ж�Ӧλ�õı��ʽ
                storage_it_qj = temp_it_qj;
        }
        else // û���У�Ӧ����qi����result4i����aj�з�����ʽ"query_j+answer_i"
            update_it_ai = true;

        // �ӳٸ���
        if (storage_it_qi != vi.end())
        {
            auto dis4i = std::distance(vi.begin(), storage_it_qi);
            auto dis4j = std::distance(vj.begin(), it_qj);
            auto it_ai = vi.begin() + dis4i;
            auto it_aj = vj.begin() + dis4j;
            it_ai->second = it_ai->second.size() < it_aj->second.size() ? it_ai->second : it_aj->second;
        }
        if (storage_it_qj != vj.end())
        {
            auto dis4j = std::distance(vj.begin(), storage_it_qj);
            auto dis4i = std::distance(vi.begin(), it_qi);
            auto it_aj = vj.begin() + dis4j;
            auto it_ai = vi.begin() + dis4i;
            it_aj->second = it_aj->second.size() < it_ai->second.size() ? it_aj->second : it_ai->second;
        }
        if (update_it_aj)
        {
            vj.emplace_back(std::make_pair(result4j, std::to_string(query_i) + "+" + answer_j));
            std::sort(vj.begin(), vj.end(),
                [](const auto& p1, const auto& p2)
                {
                    return p1.first < p2.first;
                });
        }
        if (update_it_ai)
        {
            vi.emplace_back(std::make_pair(result4i, std::to_string(query_j) + "+" + answer_i));
            std::sort(vi.begin(), vi.end(),
                [](const auto& p1, const auto& p2)
                {
                    return p1.first < p2.first;
                });
        }
    }

    virtual std::vector<std::pair<size_t, std::string>>getDataPairs(size_t index)
    {
        if(m_interactive_instances.find(index) != m_interactive_instances.end())
            return m_interactive_instances[index];
        return std::vector<std::pair<size_t, std::string>>();
    }

    virtual void show()
    {
        std::ofstream file("interaction_output.csv");

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
private:
    // <����ԪID,{<QBag_i,ABag_i>}>
    std::map<size_t, std::vector<std::pair<size_t, std::string>>> m_interactive_instances;
};

#endif // !REALITY_H
