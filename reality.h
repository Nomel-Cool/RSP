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

#include <fstream> // for file operations
#include <iterator> // for std::ostream_iterator


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

        // Obtain a time-based seed:
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

        for (size_t i = 0; i < N; ++i)
        {
            auto rand_size = rand() % max_size; // ���N���Ǹ�����
            std::vector<uint32_t> temp_v(max_value); // ����һ����СΪmax_value������

            // Fill the vector with numbers from 0 to max_value - 1
            for (uint32_t j = 0; j < max_value; ++j)
            {
                temp_v[j] = j;
            }

            // Shuffle the vector using the default random engine
            std::shuffle(temp_v.begin(), temp_v.end(), std::default_random_engine(seed));

            // Resize the vector to rand_size
            temp_v.resize(rand_size);

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

        uint32_t result4j = vi.first[pick_one] + vj.second[pick_one];
        if (std::find(vj.second.begin(), vj.second.end(), result4j) == vj.second.end())
            vj.first.emplace(result4j);
        
        uint32_t result4i = vj.first[pick_one] + vi.second[pick_one];
        if (std::find(vi.second.begin(), vi.second.end(), result4i) == vi.second.end())
            vi.first.emplace(result4i);
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
	std::map<size_t, std::pair<std::vector<uint32_t>, std::vector<uint32_t> > > m_interactive_instances;
};

#endif // !REALITY_H
