#pragma once
#ifndef DB_LINK_H
#define DB_LINK_H

#include <map>
#include <vector>
#include <mysql.h>

using StableSequencesData = std::map<std::pair<size_t, size_t>, std::vector<std::vector<std::pair<size_t, size_t>>>>;

struct VectorData {
    int vector_list_id;
    std::vector<std::pair<size_t, size_t>> pairs;
};

class dbManager
{
public:
    // 构造函数，创建对象即链接数据库
    dbManager()
    {
        conn = mysql_init(nullptr);
        if (conn == nullptr) {
            throw std::runtime_error("mysql_init() failed");
        }

        if (mysql_real_connect(conn, "localhost", "root", "123456", "stable_sequences", 0, nullptr, 0) == nullptr) {
            mysql_close(conn);
            throw std::runtime_error("mysql_real_connect() failed");
        }
    }

    // 析构函数，销毁内存时即断开链接
    ~dbManager()
    {
        if (conn != nullptr) {
            mysql_close(conn);
        }
    }

    // 增
    void Add(StableSequencesData datas)
    {
        for (const auto& [key, vectors] : datas) {
            // 检查是否已经存在相同的键对
            std::string checkMainMap = "SELECT id FROM MainMap WHERE (key1 = " + std::to_string(key.first) + " AND key2 = " + std::to_string(key.second) + ") OR (key1 = " + std::to_string(key.second) + " AND key2 = " + std::to_string(key.first) + ")";
            if (mysql_query(conn, checkMainMap.c_str())) {
                throw std::runtime_error("SELECT FROM MainMap failed");
            }

            MYSQL_RES* res = mysql_store_result(conn);
            if (res == nullptr) {
                throw std::runtime_error("mysql_store_result() failed");
            }

            MYSQL_ROW row = mysql_fetch_row(res);
            int mainMapId;
            if (row != nullptr) {
                // 键对已经存在，获取对应的 main_map_id
                mainMapId = std::stoi(row[0]);
                mysql_free_result(res);
            }
            else {
                // 插入新的键对
                mysql_free_result(res);
                std::string insertMainMap = "INSERT INTO MainMap (key1, key2) VALUES (" + std::to_string(key.first) + ", " + std::to_string(key.second) + ")";
                if (mysql_query(conn, insertMainMap.c_str())) {
                    throw std::runtime_error("INSERT INTO MainMap failed");
                }
                mainMapId = mysql_insert_id(conn);
            }

            for (size_t i = 0; i < vectors.size(); ++i) {
                std::string insertVectorList = "INSERT INTO VectorList (main_map_id, vector_index) VALUES (" + std::to_string(mainMapId) + ", " + std::to_string(i) + ")";
                if (mysql_query(conn, insertVectorList.c_str())) {
                    throw std::runtime_error("INSERT INTO VectorList failed");
                }

                int vectorListId = mysql_insert_id(conn);

                for (const auto& pair : vectors[i]) {
                    std::string insertPairList = "INSERT INTO PairList (vector_list_id, pair1, pair2) VALUES (" + std::to_string(vectorListId) + ", " + std::to_string(pair.first) + ", " + std::to_string(pair.second) + ")";
                    if (mysql_query(conn, insertPairList.c_str())) {
                        throw std::runtime_error("INSERT INTO PairList failed");
                    }
                }
            }
        }
    }

    // 查
    std::vector<VectorData> Query(std::pair<size_t, size_t> key)
    {
        std::vector<VectorData> result;

        std::string queryMainMap = "SELECT id FROM MainMap WHERE (key1 = " + std::to_string(key.first) + " AND key2 = " + std::to_string(key.second) + ") OR (key1 = " + std::to_string(key.second) + " AND key2 = " + std::to_string(key.first) + ")";
        if (mysql_query(conn, queryMainMap.c_str())) {
            throw std::runtime_error("SELECT FROM MainMap failed");
        }

        MYSQL_RES* res = mysql_store_result(conn);
        if (res == nullptr) {
            throw std::runtime_error("mysql_store_result() failed");
        }

        MYSQL_ROW row = mysql_fetch_row(res);
        if (row == nullptr) {
            mysql_free_result(res);
            return result; // No data found
        }

        int mainMapId = std::stoi(row[0]);
        mysql_free_result(res);

        std::string queryVectorList = "SELECT id, vector_index FROM VectorList WHERE main_map_id = " + std::to_string(mainMapId);
        if (mysql_query(conn, queryVectorList.c_str())) {
            throw std::runtime_error("SELECT FROM VectorList failed");
        }

        res = mysql_store_result(conn);
        if (res == nullptr) {
            throw std::runtime_error("mysql_store_result() failed");
        }

        while ((row = mysql_fetch_row(res)) != nullptr) {
            int vectorListId = std::stoi(row[0]);
            int vectorIndex = std::stoi(row[1]);

            VectorData vectorData;
            vectorData.vector_list_id = vectorListId;

            std::string queryPairList = "SELECT pair1, pair2 FROM PairList WHERE vector_list_id = " + std::to_string(vectorListId);
            if (mysql_query(conn, queryPairList.c_str())) {
                throw std::runtime_error("SELECT FROM PairList failed");
            }

            MYSQL_RES* pairRes = mysql_store_result(conn);
            if (pairRes == nullptr) {
                throw std::runtime_error("mysql_store_result() failed");
            }

            MYSQL_ROW pairRow;
            while ((pairRow = mysql_fetch_row(pairRes)) != nullptr) {
                vectorData.pairs.emplace_back(std::stoi(pairRow[0]), std::to_string(pairRow[1]));
            }

            mysql_free_result(pairRes);
            result.push_back(vectorData);
        }

        mysql_free_result(res);
        return result;
    }

    // 改
    void Update(double satisfiction_rate, int vector_list_id)
    {
        std::string updateVectorList = "UPDATE VectorList SET satisfiction_rate = " + std::to_string(satisfiction_rate) + " WHERE id = " + std::to_string(vector_list_id);
        if (mysql_query(conn, updateVectorList.c_str())) {
            throw std::runtime_error("UPDATE VectorList failed");
        }
    }

protected:

private:
    MYSQL* conn;
};

#endif // !DB_LINK_H
