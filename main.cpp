
#include <iostream>
#include <cstring>
#include "wavdata.h"
#include "iowave.h"


std::string fileNameFromPath(const std::string& path)
{
    size_t slashIndex = path.find_last_of("/\\");
    size_t dotIndex = path.find_last_of('.');
    size_t index1 = (slashIndex == std::string::npos) ? 0 : slashIndex + 1;
    size_t index2 = (dotIndex == std::string::npos) ? path.size() : dotIndex;

    return path.substr(index1, index2 - index1);
}

void patchFile(const char* sourcePath, const char* targetPath, bool traceInfo)
{
    IOWave ioObj(traceInfo);

    if (ioObj.load(sourcePath))
    {
        ioObj.clearPointsAndLabels();
        auto fileName = fileNameFromPath(sourcePath);
        ioObj.addLabel(fileName.c_str(), 0);
        ioObj.save(targetPath);
    }
}

void printHelp(const char* execPath)
{
    std::cout << "Help:\n" << fileNameFromPath(execPath) << " <sourcePath> <targetPath> [-t]\n    -t: trace debug" << std::endl;
}

int main(int argc, char *argv[])
{

    if (argc > 1)
    {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
        {
            printHelp(argv[0]);
            return 0;
        }
        if (argc > 4)
        {
            std::cout << "Wrong argument count" << std::endl;
            printHelp(argv[0]);
            return 0;
        }

        bool traceInfo = false;
        if (argc == 4)
        {
            if (strcmp(argv[3], "-t") != 0)
            {
                std::cout << "Unknown parameter passed \"" << argv[3] << "\"" << std::endl;
                return 0;
            }
            traceInfo = true;
        }

        patchFile(argv[1], argv[2], traceInfo);

        return 0;
    }

    std::cout << "No enough arguments" << std::endl;
    printHelp(argv[0]);

    return 0;
}
