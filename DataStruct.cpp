#include "DataStruct.h"
#include <mutex>

int DataStruct::get(int field) const
{
    std::shared_lock<std::shared_mutex> lock(mutexes[field]);
    return values[field];
}

void DataStruct::set(int field, int value)
{
    std::lock_guard<std::shared_mutex> lock(mutexes[field]);
    values[field] = value;
}

DataStruct::operator std::string() const
{
    int value0;
    int value1;

    {
        std::shared_lock<std::shared_mutex> lock0(mutexes[0], std::defer_lock);
        std::shared_lock<std::shared_mutex> lock1(mutexes[1], std::defer_lock);

        std::lock(lock0, lock1);

        value0 = values[0];
        value1 = values[1];
    }

    return "[" + std::to_string(value0) + ", " + std::to_string(value1) + "]";
}