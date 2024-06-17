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

#include <fstream> // for file operations
#include <iterator> // for std::ostream_iterator


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
    reality(size_t max_size, uint32_t max_value)
    {
        if (max_value == 0 || max_size <= 0)
            return;

        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

        for (size_t i = 0; i < N; ++i)
        {
            auto rand_size = rand() % max_size + 1; // Not Empty Set
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

	virtual void interaction(size_t i, size_t j)
	{
        auto& vi = m_interactive_instances[i];
        auto& vj = m_interactive_instances[j];

        std::string qi = vi.first[rand() % vi.first.size()];
        std::string ai = vi.second[rand() % vi.second.size()];
        std::string qj = vj.first[rand() % vj.first.size()];
        std::string aj = vj.second[rand() % vj.second.size()];

        uint32_t result4j = std::atoi(qi.c_str()) + std::atoi(aj.c_str());
        auto it_aj = std::find(vj.second.begin(), vj.second.end(), std::to_string(result4j));
        auto it_qj = std::find(vj.first.begin(), vj.first.end(), std::to_string(result4j));
        if (it_aj == vj.second.end())
            if (it_qj != vj.first.end())
            {
                auto offset = std::distance(vj.first.begin(), it_qj);
                if (offset <= vj.second.size())
                    vj.second.insert(vj.second.begin() + offset, std::to_string(result4j));
                else
                {
                    // Handle the case where offset is greater than the size of vi.second
                }
            }
            else
                vj.first.emplace_back(std::to_string(result4j));
        
        uint32_t result4i = std::atoi(qj.c_str()) + std::atoi(ai.c_str());
        auto it_ai = std::find(vi.second.begin(), vi.second.end(), std::to_string(result4i));
        auto it_qi = std::find(vi.first.begin(), vi.first.end(), std::to_string(result4i));
        if (it_ai == vi.second.end())
            if (it_qi != vi.first.end())
            {
                auto offset = std::distance(vi.first.begin(), it_qi);
                if (offset <= vi.second.size())
                    vi.second.insert(vi.second.begin() + offset, std::to_string(result4i));
                else
                {
                    // Handle the case where offset is greater than the size of vi.second
                }
            }
            else
                vi.first.emplace_back(std::to_string(result4i));

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
        for (const auto& pair : m_interactive_instances)
        {
            file << pair.first; // Write the key
            const auto& vectors = pair.second;

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

private:
	std::map<size_t, std::pair<std::vector<std::string>, std::vector<std::string> > > m_interactive_instances;
};

#endif // !REALITY_H
