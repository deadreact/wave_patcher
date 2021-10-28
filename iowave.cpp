#include "iowave.h"
#include <iostream>
#include <algorithm>

bool IOWave::load(const char *fileName)
{
    m_chunks.resize(0);

    std::ifstream file(fileName, std::ios_base::in | std::ios_base::binary);

    if (file.is_open())
    {
        file.read(&m_header.chunkID[0], sizeof(m_header));

        if (strncmp(m_header.chunkID, "RIFF", 4) != 0)
        {
            std::cerr << "Input file is not a RIFF file" << std::endl;
            return false;
        }

        if (strncmp(&(m_header.riffType[0]), "WAVE", 4) != 0)
        {
            std::cerr << "Input file is not a WAVE file" << std::endl;
            return false;
        }

        uint32_t remainingFileSize = m_header.dataSize.getInt() - sizeof(m_header.riffType);

        if (remainingFileSize <= 0)
        {
            std::cerr << "Input file is an empty WAVE file" << std::endl;
            return false;
        }

        if (m_traceInfo)
        {
            std::cout << "Loading file \"" << fileName << "\"..." << std::endl;
        }

        while (remainingFileSize > 0 && !file.eof())
        {
            m_chunks.push_back(ChunkObject());
            file >> m_chunks.back();
            remainingFileSize -= m_chunks.back().getDataSize();

            if (m_traceInfo)
            {
                std::cout << "Found chunk \"" << m_chunks.back().data->getId() << "\", size " << m_chunks.back().getDataSize() << " bytes" << std::endl;
            }
        }

        file.close();
        return true;
    }

    std::cerr << "Can't open the specified file \"" << fileName << "\"" << std::endl;

    return false;
}

void IOWave::save(const char *fileName) const
{
    std::ofstream file(fileName, std::ios_base::out | std::ios_base::binary);

    if (file.is_open())
    {
        file.write(&m_header.chunkID[0], sizeof(m_header));

        for (const ChunkObject& obj: m_chunks)
        {
            file << obj;
        }

        file.close();
    }
}

void IOWave::clearPointsAndLabels()
{
    auto it = std::find_if(m_chunks.begin(), m_chunks.end(), [](const ChunkObject& obj) { return strncmp(obj.data->getId(), "cue ", 4) == 0; });

    if (it != m_chunks.end())
    {
        m_header.dataSize -= it->getDataSize();
        m_chunks.erase(it);
    }

    it = std::find_if(m_chunks.begin(), m_chunks.end(), [](const ChunkObject& obj) { return strncmp(obj.data->getId(), "LIST", 4) == 0; });

    if (it != m_chunks.end())
    {
        m_header.dataSize -= it->getDataSize();
        m_chunks.erase(it);
    }
}

void IOWave::addLabel(const std::string &label, uint32_t cuePointOffset)
{
    int oldSize = 0;

    auto cueIt = std::find_if(m_chunks.begin(), m_chunks.end(), [](const ChunkObject& obj) { return strncmp(obj.data->getId(), "cue ", 4) == 0; });

    if (cueIt == m_chunks.end()) {
        m_chunks.emplace_back(new CueChunkData);
        cueIt--;
    } else {
        oldSize += cueIt->getDataSize();
    }
    CueChunkData* cueData = static_cast<CueChunkData*>(cueIt->data.get());
    uint32_t pointId = cueData->addPointIfAbsent(cuePointOffset);

    auto lstIt = std::find_if(m_chunks.begin(), m_chunks.end(), [](const ChunkObject& obj) { return strncmp(obj.data->getId(), "LIST", 4) == 0; });

    if (lstIt == m_chunks.end()) {
        m_chunks.emplace_back(new ListChunkData);
        lstIt--;
    } else {
        oldSize += lstIt->getDataSize();
    }
    ListChunkData* listData = static_cast<ListChunkData*>(lstIt->data.get());
    listData->addData(new SubListChunkData(pointId, label));

    m_header.dataSize += (cueIt->getDataSize() + lstIt->getDataSize() - oldSize);
}

void IOWave::debugPrint() const
{
    std::cout << "data size:" << m_header.dataSize.getInt() << ", chunks:\n";
    for (const ChunkObject& obj: m_chunks)
    {
        std::cout << "id: " << obj.data->getId() << ", size: " << obj.getDataSize() << std::endl;
    }
}
