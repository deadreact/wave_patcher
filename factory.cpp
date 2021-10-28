#include "factory.h"
#include "wavdata.h"

ChunkData* Factory::createChunkData(const ChunkHeader &header)
{
    if (strncmp(header.id, "cue ", 4) == 0) {
        return new CueChunkData();
    }
    if (strncmp(header.id, "labl", 4) == 0) {
        return new SubListChunkData();
    }
    if (strncmp(header.id, "LIST", 4) == 0) {
        return new ListChunkData();
    }
    if (strncmp(header.id, "fmt ", 4) == 0) {
        return new FormatChunkData();
    }
    /*.....*/

    return new GeneralChunkData(header);
}
