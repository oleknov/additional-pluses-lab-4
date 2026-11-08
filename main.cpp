#include "Demo.h"
#include <exception>
#include <iostream>
#include <string>
#include <vector>

int main()
{
    const int TOTAL_OPERATIONS = 3000000;
    const int REPEATS = 5;

    const std::string scenarioNames[SCENARIO_COUNT] = { "a", "b", "c" };

    const Frequencies scenarios[SCENARIO_COUNT] =
    {
        { 10, 5, 50, 10, 25 },
        { 20, 20, 20, 20, 20 },
        { 5, 40, 5, 40, 10 }
    };

    try
    {
        std::cout << "Generating files...\n";
        for (int s = 0; s < SCENARIO_COUNT; ++s)
        {
            generateFile(makeFileName(scenarioNames[s]), scenarios[s], TOTAL_OPERATIONS);
        }

        long long checksum = 0;
        double results[SCENARIO_COUNT][MAX_THREADS] = {};
        for (int s = 0; s < SCENARIO_COUNT; ++s)
        {
            std::vector<Operation> operations = readOperations(makeFileName(scenarioNames[s]));

            for (int t = 0; t < MAX_THREADS; ++t)
            {
                results[s][t] = measureScenario(operations, t + 1, REPEATS, checksum);
                std::cout << "Scenario " << scenarioNames[s] << ", " << t + 1
                    << " thread(s): " << results[s][t] << " ms\n";
            }
        }

        printResultsTable(std::cout, scenarioNames, results);

        if (checksum < 0)
        {
            std::cout << "Unexpected checksum\n";
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}