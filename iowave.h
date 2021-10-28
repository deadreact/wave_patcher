#pragma once

#include "wavdata.h"
#include <list>

class IOWave
{
public:
    IOWave(bool traceInfo = false): m_traceInfo(traceInfo) {}

    bool load(const char* fileName);
    void save(const char* fileName) const;

    void clearPointsAndLabels();
    void addLabel(const std::string& label, uint32_t cuePointOffset);

    void debugPrint() const;
private:
    WaveHeader m_header;
    std::list<ChunkObject> m_chunks;

    bool m_traceInfo;
};
