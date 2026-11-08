#include "Demo.h"
#include "DataStruct.h"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <latch>
#include <mutex>
#include <random>
#include <stdexcept>
#include <thread>

using OperationIterator = std::vector<Operation>::const_iterator;

std::string makeFileName(const std::string& scenarioName)
{
    return "scenario_" + scenarioName + ".txt";
}

std::discrete_distribution<int> makeDistribution(const Frequencies& frequencies)
{
    return std::discrete_distribution<int>
    {
        frequencies.read0,
        frequencies.write0,
        frequencies.read1,
        frequencies.write1,
        frequencies.str
    };
}

void writeOperation(std::ostream& out, int kind)
{
    if (kind == 0)
    {
        out << "read 0\n";
    }
    else if (kind == 1)
    {
        out << "write 0 1\n";
    }
    else if (kind == 2)
    {
        out << "read 1\n";
    }
    else if (kind == 3)
    {
        out << "write 1 1\n";
    }
    else
    {
        out << "string\n";
    }
}

void generateFile(std::ostream& out, const Frequencies& frequencies, int operationCount)
{
    std::mt19937 engine(std::random_device{}());
    std::discrete_distribution<int> distribution = makeDistribution(frequencies);

    for (int i = 0; i < operationCount; ++i)
    {
        writeOperation(out, distribution(engine));
    }
}

void generateFile(const std::string& path, const Frequencies& frequencies, int operationCount)
{
    std::ofstream out(path);
    if (!out)
    {
        throw std::runtime_error("Cannot create file: " + path);
    }
    generateFile(out, frequencies, operationCount);
}

Operation parseOperation(std::istream& in, const std::string& command)
{
    Operation operation{ OperationType::String, 0, 0 };

    if (command == "read")
    {
        operation.type = OperationType::Read;
        in >> operation.field;
    }
    else if (command == "write")
    {
        operation.type = OperationType::Write;
        in >> operation.field >> operation.value;
    }
    else if (command != "string")
    {
        throw std::runtime_error("Unknown command: " + command);
    }

    return operation;
}

std::vector<Operation> readOperations(std::istream& in)
{
    std::vector<Operation> operations;
    std::string command;

    while (in >> command)
    {
        operations.push_back(parseOperation(in, command));
    }

    return operations;
}

std::vector<Operation> readOperations(const std::string& path)
{
    std::ifstream in(path);
    if (!in)
    {
        throw std::runtime_error("Cannot open file: " + path);
    }
    return readOperations(in);
}

long long executeOperation(DataStruct& data, const Operation& operation)
{
    long long result = 0;

    if (operation.type == OperationType::Read)
    {
        result = data.get(operation.field);
    }
    else if (operation.type == OperationType::Write)
    {
        data.set(operation.field, operation.value);
    }
    else
    {
        std::string text(data);
        result = text.size();
    }

    return result;
}

void runWorker(DataStruct& data, OperationIterator first, OperationIterator last,
    std::latch& readySignal, std::latch& startSignal,
    std::mutex& checksumMutex, long long& checksum)
{
    readySignal.count_down();
    startSignal.wait();

    long long sum = 0;
    for (; first != last; ++first)
    {
        sum += executeOperation(data, *first);
    }

    std::lock_guard<std::mutex> lock(checksumMutex);
    checksum += sum;
}

double runOnce(const std::vector<Operation>& operations, int threadCount, long long& checksum)
{
    DataStruct data;

    std::latch readySignal(threadCount);
    std::latch startSignal(1);
    std::mutex checksumMutex;
    std::vector<std::thread> threads;

    size_t blockSize = operations.size() / threadCount;
    OperationIterator blockStart = operations.begin();

    for (int i = 0; i < threadCount - 1; ++i)
    {
        OperationIterator blockEnd = blockStart + blockSize;
        threads.emplace_back(runWorker, std::ref(data), blockStart, blockEnd,
            std::ref(readySignal), std::ref(startSignal),
            std::ref(checksumMutex), std::ref(checksum));
        blockStart = blockEnd;
    }
    threads.emplace_back(runWorker, std::ref(data), blockStart, operations.end(),
        std::ref(readySignal), std::ref(startSignal),
        std::ref(checksumMutex), std::ref(checksum));

    readySignal.wait();
    auto start = std::chrono::steady_clock::now();
    startSignal.count_down();

    for (std::thread& thread : threads)
    {
        thread.join();
    }
    auto finish = std::chrono::steady_clock::now();

    return std::chrono::duration<double, std::milli>(finish - start).count();
}

double measureScenario(const std::vector<Operation>& operations, int threadCount, int repeats,
    long long& checksum)
{
    double total = 0.0;
    for (int i = 0; i < repeats; ++i)
    {
        total += runOnce(operations, threadCount, checksum);
    }

    return total / repeats;
}

void printResultsTable(std::ostream& out, const std::string scenarioNames[], const double results[][MAX_THREADS])
{
    out << "\nAverage execution time, ms\n";
    out << std::left << std::setw(12) << "Scenario";
    for (int threadCount = 1; threadCount <= MAX_THREADS; ++threadCount)
    {
        out << std::right << std::setw(14) << std::to_string(threadCount) + " thread(s)";
    }
    out << '\n';

    out << std::fixed << std::setprecision(2);
    for (int s = 0; s < SCENARIO_COUNT; ++s)
    {
        out << std::left << std::setw(12) << scenarioNames[s];
        for (int t = 0; t < MAX_THREADS; ++t)
        {
            out << std::right << std::setw(14) << results[s][t];
        }
        out << '\n';
    }
}