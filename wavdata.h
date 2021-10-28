#pragma once

#include <string>
#include <cstring>
#include <array>
#include <vector>
#include <memory>

#include "littleendianint.h"


struct ChunkHeader {
    char id[4];
    LittleEndianInt32 dataSize;

    ChunkHeader() = default;
    ChunkHeader(const char* _id, uint32_t _dataSize);
};

std::ofstream& operator<<(std::ofstream& os, const ChunkHeader& data);
std::ifstream& operator>>(std::ifstream& is, ChunkHeader& data);


class ChunkData
{
public:
    virtual ~ChunkData() {}

    virtual void readDataFromBuffer(std::ifstream& is, int size) = 0;
    virtual void writeDataToBuffer(std::ofstream& os) const = 0;

    virtual const char* getId() const = 0;
    virtual uint32_t getDataSize() const = 0;

    inline ChunkHeader getHeader() const {
        return ChunkHeader(getId(), getDataSize());
    }
};

struct ChunkObject
{
    ChunkObject() = default;
    ChunkObject(const ChunkObject&) = delete;
    ChunkObject& operator=(const ChunkObject&) = delete;

    ChunkObject(ChunkObject&& obj): data(obj.data.release()) {}
    ChunkObject(ChunkData* data): data(data) {}

    uint32_t getDataSize() const {
        if (data)  {
            uint32_t size = sizeof(ChunkHeader) + data->getDataSize();
            return size + (size % 2);
        }
        return 0;
    }

    std::unique_ptr<ChunkData> data;
};

std::ofstream& operator<<(std::ofstream& os, const ChunkObject& obj);
std::ifstream& operator>>(std::ifstream& is, ChunkObject& obj);


class GeneralChunkData : public ChunkData
{
public:
    GeneralChunkData(const ChunkHeader& header) {
        m_id.resize(4);
        for (int i = 0; i < 4; i++) {
            m_id[i] = header.id[i];
        }
    }

    virtual const char* getId() const override;
    virtual uint32_t getDataSize() const override;

    virtual void readDataFromBuffer(std::ifstream& is, int size) override;
    virtual void writeDataToBuffer(std::ofstream& os) const override;

private:
    std::string m_id;
    std::vector<uint8_t> m_rawData;
};


class FormatChunkData: public ChunkData {
public:
    virtual const char* getId() const override { return "fmt "; }
    virtual uint32_t getDataSize() const override { return m_extraFormatData.size() + 16; }

    virtual void readDataFromBuffer(std::ifstream& is, int size) override;
    virtual void writeDataToBuffer(std::ofstream& os) const override;
private:
    uint16_t m_compressionCode;
    uint16_t m_numberOfChannels;
    uint32_t m_sampleRate;
    uint32_t m_averageBytesPerSecond;
    uint16_t m_blockAlign;
    uint16_t m_significantBitsPerSample;
    std::vector<uint8_t> m_extraFormatData;
};


struct CuePointData {
    uint32_t cuePointID{0};
    uint32_t playOrderPosition{0};
    char dataChunkID[4] = {'d','a','t','a'};
    uint32_t chunkStart{0};
    uint32_t blockStart{0};
    uint32_t frameOffset{0};
};

std::ofstream& operator<<(std::ofstream& os, const CuePointData& data);
std::ifstream& operator>>(std::ifstream& is, CuePointData& data);


class CueChunkData: public ChunkData
{
public:
    virtual const char* getId() const override { return "cue "; }
    virtual uint32_t getDataSize() const override { return sizeof(CuePointData) * m_points.size() + 4; }

    virtual void readDataFromBuffer(std::ifstream& is, int size);
    virtual void writeDataToBuffer(std::ofstream& os) const;

    uint32_t addPointIfAbsent(uint32_t frameOffset);
private:
    std::vector<CuePointData> m_points;
};

class SubListChunkData: public ChunkData
{
public:
    SubListChunkData() = default;
    SubListChunkData(uint32_t cuePointId, const std::string& label): m_cuePointId(cuePointId), m_label(label) {}

    virtual const char* getId() const override { return "labl"; }
    virtual uint32_t getDataSize() const override { return m_label.size() + 5; }

    virtual void readDataFromBuffer(std::ifstream& is, int size);
    virtual void writeDataToBuffer(std::ofstream& os) const;

private:
    uint32_t m_cuePointId;
    std::string m_label;
};



class ListChunkData: public ChunkData
{
public:
    virtual const char* getId() const override;
    virtual uint32_t getDataSize() const override;

    virtual void readDataFromBuffer(std::ifstream& is, int size);
    virtual void writeDataToBuffer(std::ofstream& os) const;

    void addData(ChunkData* data)
    {
        m_lst.emplace_back(data);
    }
private:
    char m_typeId[4] = {'a','d','t','l'};
    std::vector<ChunkObject> m_lst;
};
