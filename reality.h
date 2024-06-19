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

    bool cmpFunc(const std::string& a, const std::string& b)
    {
        return std::stoi(a) < std::stoi(b);
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
        reality(size_t max_size, uint32_t max_value)
        {
            if (max_value == 0 || max_size <= 0)
                return;

            unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

            for (size_t i = 0; i < N; ++i)
            {
                auto rand_size = (rand() % max_size) + 1; // Not Empty Set
                std::vector<std::string> temp_v(rand_size);
                for (uint32_t j = 0; j < rand_size; ++j)
                    temp_v[j] = std::to_string(j);

                // Shuffle the vector using the default random engine
                std::shuffle(temp_v.begin(), temp_v.end(), std::default_random_engine(seed));
                m_interactive_instances[i] = std::make_pair(temp_v, temp_v);
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
        virtual void interaction(size_t i, size_t j)
        {
            auto& vi = m_interactive_instances[i - 1];
            auto& vj = m_interactive_instances[j - 1];

            std::string qi = vi.first[rand() % vi.first.size()];
            std::string ai = vi.second[rand() % vi.second.size()];
            std::string qj = vj.first[rand() % vj.first.size()];
            std::string aj = vj.second[rand() % vj.second.size()];

            bool viq = false, via = false, vjq = false, vja = false; // �ͺ��ź�������Ϊ������ͬʱ�����ģ�����QBag��ABag���޸Ķ�������ͬʱ��

            /* vi -> vj */
            uint32_t result4j = std::atoi(qi.c_str()) + std::atoi(aj.c_str());
            auto it_aj = std::find(vj.second.begin(), vj.second.end(), std::to_string(result4j));
            auto it_qj = std::find(vj.first.begin(), vj.first.end(), std::to_string(result4j));
            if (it_aj != vj.second.end()) // �����������⣬Ӧ��ȥ��ѯ�ý���Ƿ�����vi��QBag�е�����
            {
                auto it_qi = std::find(vi.first.begin(), vi.first.end(), std::to_string(result4j));
                auto it_ai = std::find(vi.second.begin(), vi.second.end(), std::to_string(result4j));
                if (it_qi != vi.first.end() && it_ai == vi.second.end()) // ��������vi��QBag�����Ⲣ��vi��ABagû������𰸣������vi��ABag
                    //vi.second.emplace_back(std::to_string(result4j));
                    via = true;
            }
            else // ���û�н�����⣬�������QBag��û����ͬ�����⣬������һ������
                if (it_qj == vj.first.end())
                    //vj.first.emplace_back(std::to_string(result4j));
                    vjq = true;

            /* vj -> vi  */
            uint32_t result4i = std::atoi(qj.c_str()) + std::atoi(ai.c_str());
            auto it_ai = std::find(vi.second.begin(), vi.second.end(), std::to_string(result4i));
            auto it_qi = std::find(vi.first.begin(), vi.first.end(), std::to_string(result4i));
            if (it_ai != vi.second.end()) // �����������⣬Ӧ��ȥ��ѯ�ý���Ƿ�����vj��QBag�е�����
            {
                auto it_qj = std::find(vj.first.begin(), vj.first.end(), std::to_string(result4i));
                auto it_aj = std::find(vj.second.begin(), vj.second.end(), std::to_string(result4i));
                if (it_qj != vj.first.end() && it_aj == vj.second.end()) // ��������vj��QBag�����Ⲣ��vj��ABagû�ж�Ӧ�Ĵ𰸣������vj��ABag
                    //vj.second.emplace_back(std::to_string(result4i));
                    vja = true;
            }
            else // ���û�н�����⣬�������vi��QBag��û����ͬ�����⣬������һ������
                if (it_qi == vi.first.end())
                    //vi.first.emplace_back(std::to_string(result4i));
                    viq = true;
            
            //�ͺ����
            if(viq)
                vj.first.emplace_back(std::to_string(result4j));
            if(via)
                vi.second.emplace_back(std::to_string(result4j));
            if(vjq)
                vi.first.emplace_back(std::to_string(result4i));
            if(vja)
                vj.second.emplace_back(std::to_string(result4i));
        }

        virtual std::pair<std::vector<std::string>, std::vector<std::string> >getVectors(size_t index)
        {
            if(m_interactive_instances.find(index) != m_interactive_instances.end())
                return m_interactive_instances[index];
        }

        virtual void show()
        {
            std::ofstream file("interaction_output.csv");

            // Write headers
            file << "Key";
            for (size_t i = 0; i < N; ++i)
            {
                file << ",QBag" << i + 1;
            }
            for (size_t i = 0; i < N; ++i)
            {
                file << ",ABag" << i + 1;
            }
            file << "\n";

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
        virtual void align(std::vector<std::string>& A, std::vector<std::string>& B) {
            // ��A��Bת��Ϊ���ϣ��Ա���м��ϲ���
            std::set<std::string> setA(A.begin(), A.end());
            std::set<std::string> setB(B.begin(), B.end());

            // ���㽻��
            std::vector<std::string> intersection;
            std::set_intersection(setA.begin(), setA.end(), setB.begin(), setB.end(), std::back_inserter(intersection));

            // ����
            std::vector<std::string> differenceA;
            std::set_difference(setA.begin(), setA.end(), setB.begin(), setB.end(), std::back_inserter(differenceA));

            std::vector<std::string> differenceB;
            std::set_difference(setB.begin(), setB.end(), setA.begin(), setA.end(), std::back_inserter(differenceB));

            // �Խ����Ͳ��������
            std::sort(intersection.begin(), intersection.end(), &st::cmpFunc);

            std::sort(differenceA.begin(), differenceA.end(), &st::cmpFunc);

            std::sort(differenceB.begin(), differenceB.end(), &st::cmpFunc);

            // ��������ƻ�A��B
            A.clear();
            B.clear();
            A.insert(A.end(), intersection.begin(), intersection.end());
            B.insert(B.end(), intersection.begin(), intersection.end());
            A.insert(A.end(), differenceA.begin(), differenceA.end());
            B.insert(B.end(), differenceB.begin(), differenceB.end());
        }
    private:
        std::map<size_t, std::pair<std::vector<std::string>, std::vector<std::string> > > m_interactive_instances;
    };
}

#endif // !REALITY_H
