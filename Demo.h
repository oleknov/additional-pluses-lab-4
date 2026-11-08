#pragma once
#include <iostream>
#include <string>
#include <vector>

const int SCENARIO_COUNT = 3;
const int MAX_THREADS = 3;


struct Frequencies
{
    double read0;
    double write0;
    double read1;
    double write1;
    double str;
};

enum class OperationType
{
    Read,
    Write,
    String
};

struct Operation
{
    OperationType type;
    int field;
    int value;
};

std::string makeFileName(const std::string& scenarioName);

void generateFile(std::ostream& out, const Frequencies& frequencies, int operationCount);
void generateFile(const std::string& path, const Frequencies& frequencies, int operationCount);

std::vector<Operation> readOperations(std::istream& in);
std::vector<Operation> readOperations(const std::string& path);

double measureScenario(const std::vector<Operation>& operations, int threadCount, int repeats,
    long long& checksum);

void printResultsTable(std::ostream& out, const std::string scenarioNames[], const double results[][MAX_THREADS]);