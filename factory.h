#pragma once


class ChunkHeader;
class ChunkData;

class Factory
{
public:
    static ChunkData* createChunkData(const ChunkHeader& header);
};

