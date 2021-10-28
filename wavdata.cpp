#include "wavdata.h"
#include "factory.h"

#include <iostream>
#include <algorithm>

ChunkHeader::ChunkHeader(const char *_id, uint32_t _dataSize)
    : dataSize(_dataSize)
{
    for (int i = 0; i < 4; i++) {
        id[i] = _id[i];
    }
}

std::ofstream &operator<<(std::ofstream &os, const ChunkHeader &data)
{
    os.write(data.id, sizeof(data.id));
    os << data.dataSize;
    return os;
}

std::ifstream &operator>>(std::ifstream &is, ChunkHeader &data)
{
    is.read(data.id, sizeof(data.id));
    is >> data.dataSize;

    return is;
}


std::ofstream& operator<<(std::ofstream &os, const ChunkObject &obj)
{
    if (obj.data)
    {
        os << obj.data->getHeader();
        obj.data->writeDataToBuffer(os);

        if (obj.data->getDataSize() % 2 != 0)
        {
            os << '\0';
        }
    }
    return os;
}

std::ifstream& operator>>(std::ifstream &is, ChunkObject &obj)
{
    ChunkHeader header;
    is >> header;

    ChunkData* data = Factory::createChunkData(header);
    data->readDataFromBuffer(is, header.dataSize.getInt());

    if (header.dataSize.getInt() % 2 != 0)
    {
        char tmp;
        is >> tmp;
    }

    obj.data.reset(data);

    return is;
}




const char *GeneralChunkData::getId() const { return m_id.c_str(); }
uint32_t GeneralChunkData::getDataSize() const { return m_rawData.size(); }

void GeneralChunkData::readDataFromBuffer(std::ifstream &is, int size)
{
    m_rawData.resize(size);
    is.read((char*)m_rawData.data(), size);
}

void GeneralChunkData::writeDataToBuffer(std::ofstream &os) const
{
    os.write((const char*)m_rawData.data(), m_rawData.size());
}

std::ofstream &operator<<(std::ofstream &os, const CuePointData &data)
{
    os << LittleEndianInt32(data.cuePointID)
       << LittleEndianInt32(data.playOrderPosition);
    os.write(data.dataChunkID, 4);
    os << LittleEndianInt32(data.chunkStart)
       << LittleEndianInt32(data.blockStart)
       << LittleEndianInt32(data.frameOffset);

    return os;
}

std::ifstream &operator>>(std::ifstream &is, CuePointData &data)
{
    LittleEndianInt32 buff;
    is >> buff; data.cuePointID = buff.getInt();
    is >> buff; data.playOrderPosition = buff.getInt();
    is.read(data.dataChunkID, 4);
    is >> buff; data.chunkStart = buff.getInt();
    is >> buff; data.blockStart = buff.getInt();
    is >> buff; data.frameOffset = buff.getInt();

    return is;
}

void CueChunkData::readDataFromBuffer(std::ifstream &is, int /*size*/)
{
    LittleEndianInt32 pointCount;
    is >> pointCount;
    m_points.resize(pointCount.getInt());

    for (CuePointData& p: m_points)
    {
        is >> p;
    }
}

void CueChunkData::writeDataToBuffer(std::ofstream &os) const
{
    os << LittleEndianInt32(m_points.size());

    for (const CuePointData& p: m_points) {
        os << p;
    }
}

uint32_t CueChunkData::addPointIfAbsent(uint32_t frameOffset)
{
    auto it = std::find_if(m_points.begin(), m_points.end(), [frameOffset](const CuePointData& p) { return p.frameOffset == frameOffset; });
    if (it != m_points.end()) {
        return it->cuePointID;
    }

    CuePointData p;
    p.cuePointID = m_points.size() + 1;
    p.frameOffset = frameOffset;
    m_points.push_back(p);

    return p.cuePointID;
}


void SubListChunkData::readDataFromBuffer(std::ifstream &is, int size)
{
    LittleEndianInt32 cuePointId;
    is >> cuePointId;
    m_cuePointId = cuePointId.getInt();
    m_label.resize(0);

    char buff;
    is >> buff;
    while (buff)
    {
        m_label += buff;
        is >> buff;
    }
}

void SubListChunkData::writeDataToBuffer(std::ofstream &os) const
{
    os << LittleEndianInt32(m_cuePointId) << m_label.c_str() << '\0';
}

const char *ListChunkData::getId() const { return "LIST"; }

uint32_t ListChunkData::getDataSize() const
{
    uint32_t size = sizeof(m_typeId);
    for (const ChunkObject& obj: m_lst)
    {
        size += obj.getDataSize();
    }
    return size;
}

void ListChunkData::readDataFromBuffer(std::ifstream &is, int size)
{
    is.read(m_typeId, 4);
    size -= 4;
    m_lst.resize(0);

    while (size > 0)
    {
        m_lst.push_back(ChunkObject());
        is >> m_lst.back();
        size -= m_lst.back().getDataSize();
    }

    if (size != 0)
    {
        std::cerr << "Wrong size" << std::endl;
    }
}

void ListChunkData::writeDataToBuffer(std::ofstream &os) const
{
    os.write(m_typeId, 4);
    for (const ChunkObject& obj: m_lst)
    {
        os << obj;
    }
}

void FormatChunkData::readDataFromBuffer(std::ifstream &is, int size)
{
    LittleEndianInt16 buff16;
    LittleEndianInt32 buff32;

    is >> buff16; m_compressionCode = buff16.getInt();
    is >> buff16; m_numberOfChannels = buff16.getInt();
    is >> buff32; m_sampleRate = buff32.getInt();
    is >> buff32; m_averageBytesPerSecond = buff32.getInt();
    is >> buff16; m_blockAlign = buff16.getInt();
    is >> buff16; m_significantBitsPerSample = buff16.getInt();

    size -= 16;
    if (size > 0)
    {
        m_extraFormatData.resize(size);
        is.read((char*)m_extraFormatData.data(), size);
    }
}

void FormatChunkData::writeDataToBuffer(std::ofstream &os) const
{
    os << LittleEndianInt16(m_compressionCode)
       << LittleEndianInt16(m_numberOfChannels)
       << LittleEndianInt32(m_sampleRate)
       << LittleEndianInt32(m_averageBytesPerSecond)
       << LittleEndianInt16(m_blockAlign)
       << LittleEndianInt16(m_significantBitsPerSample);

    if (!m_extraFormatData.empty())
    {
        os.write((const char*)m_extraFormatData.data(), m_extraFormatData.size());
    }
}
