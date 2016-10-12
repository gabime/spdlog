#include <iostream>

#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

void CrusherLoop()
{
    size_t counter = 0;
    while (true)
    {
        LOGF(INFO, "Some text to crush you machine. thread:");
        if(++counter % 1000000 == 0)
        {
            std::cout << "Wrote " << counter << " entries" << std::endl;
        }
    }
}


int main(int argc, char** argv)
{
    std::cout << "WARNING: This test will exaust all your machine memory and will crush it!" << std::endl;
    std::cout << "Are you sure you want to continue ? " << std::endl;
    char c;
    std::cin >> c;
    if (toupper( c ) != 'Y')
        return 0;

    auto worker = g3::LogWorker::createLogWorker();
    auto handle= worker->addDefaultLogger(argv[0], "g3log.txt");
    g3::initializeLogging(worker.get());
    CrusherLoop();

    return 0;
}


