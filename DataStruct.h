#pragma once
#include <shared_mutex>
#include <string>

class DataStruct
{
public:
    static const int FIELD_COUNT = 2;

    DataStruct() = default;

    DataStruct(const DataStruct&) = delete;
    DataStruct& operator=(const DataStruct&) = delete;

    int get(int field) const;
    void set(int field, int value);

    explicit operator std::string() const;

private:
    int values[FIELD_COUNT] = { 0, 0 };

    mutable std::shared_mutex mutexes[FIELD_COUNT];
};