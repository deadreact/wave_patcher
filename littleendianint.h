#pragma once

#include <inttypes.h>
#include <type_traits>
#include <fstream>
#include <vector>

enum class EHostEndianness {
    Undefined = 0,
    LittleEndian,
    BigEndian
};

bool isHostLittleEndian();

// Some Structs that we use to represent and manipulate Chunks in the Wave files

template <typename T, bool = std::is_integral<T>::value>
struct LittleEndianInt;

template <typename T>
struct LittleEndianInt<T, true>
{
    static const int bytes_count = sizeof(T);
    uint8_t data[bytes_count] = {0};

    LittleEndianInt() = default;
    LittleEndianInt(const LittleEndianInt<T, true>&) = default;

    LittleEndianInt(T value)
    {
        setInt(value);
    }

    LittleEndianInt<T, true>& operator=(T value )
    {
        setInt(value);
        return *this;
    }

    void setInt(T value)
    {
        uint8_t* uintValueBytes = (uint8_t *)&value;

        if (isHostLittleEndian())
        {
            for (int i = 0; i < bytes_count; ++i)
            {
                data[i] = uintValueBytes[i];
            }
        }
        else
        {
            for (int i = 0; i < bytes_count; ++i)
            {
                data[i] = uintValueBytes[bytes_count - 1 - i];
            }
        }
    }

    T getInt() const
    {
        T value;
        uint8_t *uintValueBytes = (uint8_t *)&value;

        if (isHostLittleEndian())
        {
            for (int i = 0; i < bytes_count; ++i)
            {
                uintValueBytes[i] = data[i];
            }
        }
        else
        {
            for (int i = 0; i < bytes_count; ++i)
            {
                uintValueBytes[i] = data[bytes_count - 1 - i];
            }
        }

        return value;
    }

    LittleEndianInt<T, true> operator+(LittleEndianInt<T, true> other) const
    {
        return LittleEndianInt<T, true>(getInt() + other.getInt());
    }

    LittleEndianInt<T, true> operator-(LittleEndianInt<T, true> other) const
    {
        return LittleEndianInt<T, true>(getInt() - other.getInt());
    }

    LittleEndianInt<T, true>& operator+=(LittleEndianInt<T, true> other)
    {
        *this = *this + other;
        return *this;
    }

    LittleEndianInt<T, true>& operator-=(LittleEndianInt<T, true> other)
    {
        *this = *this - other;
        return *this;
    }
};

template <typename T>
std::ofstream& operator<<(std::ofstream& os, const LittleEndianInt<T>& data) {
    os.write((const char*)data.data, sizeof(T));
    return os;
}

template <typename T>
std::ifstream& operator>>(std::ifstream& is, LittleEndianInt<T>& data) {
    is.read((char*)data.data, sizeof(T));
    return is;
}

using LittleEndianInt16 = LittleEndianInt<uint16_t>;
using LittleEndianInt32 = LittleEndianInt<uint32_t>;

// The header of a wave file
struct WaveHeader {
    char chunkID[4] = {'R','I','F','F'};		// Must be "RIFF" (0x52494646)
    LittleEndianInt32 dataSize;		// Byte count for the rest of the file (i.e. file length - 8 bytes)
    char riffType[4] = {'W','A','V','E'};	// Must be "WAVE" (0x57415645)
};


//// The format chunk of a wave file
//struct FormatChunk {
//    char chunkID[4] = {0,0,0,0};				// String: must be "fmt " (0x666D7420).
//    LittleEndianInt32 chunkDataSize;			// Unsigned 4-byte little endian int: Byte count for the remainder of the chunk: 16 + extraFormatbytes.
//    LittleEndianInt16 compressionCode;		// Unsigned 2-byte little endian int
//    LittleEndianInt16 numberOfChannels;		// Unsigned 2-byte little endian int
//    LittleEndianInt32 sampleRate;				// Unsigned 4-byte little endian int
//    LittleEndianInt32 averageBytesPerSecond;	// Unsigned 4-byte little endian int: This value indicates how many bytes of wave data must be streamed to a D/A converter per second in order to play the wave file. This information is useful when determining if data can be streamed from the source fast enough to keep up with playback. = SampleRate * BlockAlign.
//    LittleEndianInt16 blockAlign;				// Unsigned 2-byte little endian int: The number of bytes per sample slice. This value is not affected by the number of channels and can be calculated with the formula: blockAlign = significantBitsPerSample / 8 * numberOfChannels
//    LittleEndianInt16 significantBitsPerSample;// Unsigned 2-byte little endian int
//};


//// CuePoint: each individual 'marker' in a wave file is represented by a cue point.
//struct CuePoint {
//    LittleEndianInt32 cuePointID;			// a unique ID for the Cue Point.
//    LittleEndianInt32 playOrderPosition;	// Unsigned 4-byte little endian int: If a Playlist chunk is present in the Wave file, this the sample number at which this cue point will occur during playback of the entire play list as defined by the play list's order.  **Otherwise set to same as sample offset??***  Set to 0 when there is no playlist.
//    LittleEndianInt32 dataChunkID;		// Unsigned 4-byte little endian int: The ID of the chunk containing the sample data that corresponds to this cue point.  If there is no playlist, this should be 'data'.
//    LittleEndianInt32 chunkStart;			// Unsigned 4-byte little endian int: The byte offset into the Wave List Chunk of the chunk containing the sample that corresponds to this cue point. This is the same chunk described by the Data Chunk ID value. If no Wave List Chunk exists in the Wave file, this value is 0.
//    LittleEndianInt32 blockStart;			// Unsigned 4-byte little endian int: The byte offset into the "data" or "slnt" Chunk to the start of the block containing the sample. The start of a block is defined as the first byte in uncompressed PCM wave data or the last byte in compressed wave data where decompression can begin to find the value of the corresponding sample value.
//    LittleEndianInt32 frameOffset;		// Unsigned 4-byte little endian int: The offset into the block (specified by Block Start) for the sample that corresponds to the cue point.

//    CuePoint() = default;
//    CuePoint(uint32_t id, uint32_t offset)
//    {
//        cuePointID = id;
//        playOrderPosition = 0;
//        dataChunkID.data[0] = 'd';
//        dataChunkID.data[1] = 'a';
//        dataChunkID.data[2] = 't';
//        dataChunkID.data[3] = 'a';
//        chunkStart = 0;
//        blockStart = 0;
//        frameOffset = offset;
//    }
//};


//// CuePoints are stored in a CueChunk
//struct CueChunk {
//    char chunkID[4] = {'c','u','e',' '};		// String: Must be "cue " (0x63756520).
//    LittleEndianInt32 chunkDataSize;	// Unsigned 4-byte little endian int: Byte count for the remainder of the chunk: 4 (size of cuePointsCount) + (24 (size of CuePoint struct) * number of CuePoints).
//    LittleEndianInt32 cuePointsCount;	// Unsigned 4-byte little endian int: Length of cuePoints[].
//    std::vector<CuePoint> cuePoints;
//};


//// Some chunks we don't care about the contents and will just copy them from the input file to the output,
//// so this struct just stores positions of such chunks in the input file
//struct ChunkLocation {
//    long startOffset {0}; // in bytes
//    long size {0};		  // in bytes
//};

//// For such chunks that we will copy over from input to output, this function does that in 1MB pieces
//int writeChunkLocationFromInputFileToOutputFile(ChunkLocation chunk, FILE *inputFile, FILE *outputFile);



//// The main function

//enum CuePointMergingOption {
//    MergeWithAnyExistingCuePoints = 0,
//    ReplaceAnyExistingCuePoints
//};

//class CueTool
//{
//public:
//    int addMarkersToWaveFile(const char* inFilePath, const char* markerFilePath, const char* outFilePath, enum CuePointMergingOption mergeOption = ReplaceAnyExistingCuePoints)
//    {
//        if (int res = readWaveFile(inFilePath)) {
//            return res;
//        }

//        if (int res = readMarkersFile(markerFilePath, mergeOption)) {
//            return res;
//        }

//        if (int res = writeWaveFile(outFilePath)) {
//            return res;
//        }

//        printf("Finished.\n");

//        return 0;
//    }

//private:
//    int readWaveFile(const char* inFilePath)
//    {
//        // Open the Input File
//        m_inputFile = fopen(inFilePath, "rb");
//        if (m_inputFile == nullptr)
//        {
//            fprintf(stderr, "Could not open input file %s\n", inFilePath);
//            cleanUp();
//            return -1;
//        }

//        // Get & check the input file header
//        fprintf(stdout, "Reading input wave file.\n");

//        fread(&m_waveHeader, sizeof(WaveHeader), 1, m_inputFile);
//        if (ferror(m_inputFile) != 0)
//        {
//            fprintf(stderr, "Error reading input file %s\n", inFilePath);
//            cleanUp();
//            return -1;
//        }

//        if (strncmp(&(m_waveHeader.chunkID[0]), "RIFF", 4) != 0)
//        {
//            fprintf(stderr, "Input file is not a RIFF file\n");
//            cleanUp();
//            return -1;
//        }

//        if (strncmp(&(m_waveHeader.riffType[0]), "WAVE", 4) != 0)
//        {
//            fprintf(stderr, "Input file is not a WAVE file\n");
//            cleanUp();
//            return -1;
//        }

//        uint32_t remainingFileSize = m_waveHeader.dataSize.getInt() - sizeof(m_waveHeader.riffType); // dataSize does not counf the chunkID or the dataSize, so remove the riffType size to get the length of the rest of the file.

//        fprintf(stdout, "remainingFileSize: %u\n", remainingFileSize);

//        if (remainingFileSize <= 0)
//        {
//            fprintf(stderr, "Input file is an empty WAVE file\n");
//            cleanUp();
//            return -1;
//        }



//        // Start reading in the rest of the wave file
//        while (1)
//        {
//            char nextChunkID[4];

//            // Read the ID of the next chunk in the file, and bail if we hit End Of File
//            fread(&nextChunkID[0], sizeof(nextChunkID), 1, m_inputFile);
//            if (feof(m_inputFile))
//            {
//                break;
//            }

//            if (ferror(m_inputFile) != 0)
//            {
//                fprintf(stderr, "Error reading input file %s\n", inFilePath);
//                cleanUp();
//                return -1;
//            }


//            // See which kind of chunk we have

//            if (strncmp(&nextChunkID[0], "fmt ", 4) == 0)
//            {
//                // We found the format chunk

//                fseek(m_inputFile, -4, SEEK_CUR);
//                fread(&m_formatChunk, sizeof(FormatChunk), 1, m_inputFile);
//                if (ferror(m_inputFile) != 0)
//                {
//                    fprintf(stderr, "Error reading input file %s\n", inFilePath);
//                    cleanUp();
//                    return -1;
//                }

//                if (m_formatChunk.compressionCode.getInt() != (uint16_t)1)
//                {
//                    fprintf(stderr, "Compressed audio formats are not supported\n");
//                    cleanUp();
//                    return -1;
//                }

//                // Note: For compressed audio data there may be extra bytes appended to the format chunk,
//                // but as we are only handling uncompressed data we shouldn't encounter them

//                // There may or may not be extra data at the end of the fomat chunk.  For uncompressed audio there should be no need, but some files may still have it.
//                // if formatChunk.chunkDataSize > 16 (16 = the number of bytes for the format chunk, not counting the 4 byte ID and the chunkDataSize itself) there is extra data
//                uint32_t extraFormatBytesCount = m_formatChunk.chunkDataSize.getInt() - 16;
//                if (extraFormatBytesCount > 0)
//                {
//                    m_formatChunkExtraBytes.startOffset = ftell(m_inputFile);
//                    m_formatChunkExtraBytes.size = extraFormatBytesCount;
//                    fseek(m_inputFile, extraFormatBytesCount, SEEK_CUR);
//                    if (extraFormatBytesCount % 2 != 0)
//                    {
//                        fseek(m_inputFile, 1, SEEK_CUR);
//                    }
//                }


//                printf("Got Format Chunk\n");
//            }

//            else if (strncmp(&nextChunkID[0], "data", 4) == 0)
//            {
//                // We found the data chunk
//                m_dataChunkLocation.startOffset = ftell(m_inputFile) - sizeof(nextChunkID);

//                // The next 4 bytes are the chunk data size - the size of the sample data
//                LittleEndianInt32 sampleDataSizeBytes;
//                fread(sampleDataSizeBytes.data, sizeof(uint8_t), 4, m_inputFile);
//                if (ferror(m_inputFile) != 0)
//                {
//                    fprintf(stderr, "Error reading input file %s\n", inFilePath);
//                    cleanUp();
//                    return -1;
//                }
//                uint32_t sampleDataSize = sampleDataSizeBytes.getInt();

//                m_dataChunkLocation.size = sizeof(nextChunkID) + sizeof(sampleDataSizeBytes) + sampleDataSize;


//                // Skip to the end of the chunk.  Chunks must be aligned to 2 byte boundaries, but any padding at the end of a chunk is not included in the chunkDataSize
//                fseek(m_inputFile, sampleDataSize, SEEK_CUR);
//                if (sampleDataSize % 2 != 0)
//                {
//                    fseek(m_inputFile, 1, SEEK_CUR);
//                }

//                printf("Got Data Chunk\n");
//            }

//            else if (strncmp(&nextChunkID[0], "cue ", 4) == 0)
//            {
//                // We found an existing Cue Chunk

//                LittleEndianInt32 cueChunkDataSizeBytes;
//                fread(cueChunkDataSizeBytes.data, sizeof(uint8_t), 4, m_inputFile);
//                if (ferror(m_inputFile) != 0)
//                {
//                    fprintf(stderr, "Error reading input file %s\n", inFilePath);
//                    cleanUp();
//                    return -1;
//                }
//                uint32_t cueChunkDataSize = cueChunkDataSizeBytes.getInt();

//                LittleEndianInt32 cuePointsCountBytes;
//                fread(cuePointsCountBytes.data, sizeof(uint8_t), 4, m_inputFile);
//                if (ferror(m_inputFile) != 0)
//                {
//                    fprintf(stderr, "Error reading input file %s\n", inFilePath);
//                    cleanUp();
//                    return -1;
//                }
//                uint32_t cuePointsCount = cuePointsCountBytes.getInt();

//                // Read in the existing cue points into CuePoint Structs
//                std::vector<CuePoint> existingCuePoints;
//                existingCuePoints.resize(cuePointsCount);

//                for (uint32_t cuePointIndex = 0; cuePointIndex < cuePointsCount; cuePointIndex++)
//                {
//                    fread(&existingCuePoints[cuePointIndex], sizeof(CuePoint), 1, m_inputFile);
//                    if (ferror(m_inputFile) != 0)
//                    {
//                        fprintf(stderr, "Error reading input file %s\n", inFilePath);
//                        cleanUp();
//                        return -1;
//                    }
//                }

//                // Populate the existingCueChunk struct
//                m_existingCueChunk.chunkDataSize = cueChunkDataSize;
//                m_existingCueChunk.cuePointsCount = cuePointsCount;
//                m_existingCueChunk.cuePoints = std::move(existingCuePoints);

//                printf("Found Existing Cue Chunk\n");
//            }

//            else
//            {
//                // We have found a chunk type that we are not going to work with.  Just note the location so we can copy it to the output file later

//                if (m_otherChunksCount >= maxOtherChunks)
//                {
//                    fprintf(stderr, "Input file has more chunks than the maximum supported by this program (%d)\n", maxOtherChunks);
//                    cleanUp();
//                    return -1;
//                }

//                m_otherChunkLocations[m_otherChunksCount].startOffset = ftell(m_inputFile) - sizeof(nextChunkID);

//                LittleEndianInt32 chunkDataSizeBytes;
//                fread(chunkDataSizeBytes.data, sizeof(uint8_t), 4, m_inputFile);
//                if (ferror(m_inputFile) != 0)
//                {
//                    fprintf(stderr, "Error reading input file %s\n", inFilePath);
//                    cleanUp();
//                    return -1;
//                }
//                uint32_t chunkDataSize = chunkDataSizeBytes.getInt();

//                m_otherChunkLocations[m_otherChunksCount].size = sizeof(nextChunkID) + sizeof(chunkDataSizeBytes) + chunkDataSize;


//                // Skip over the chunk's data, and any padding byte
//                fseek(m_inputFile, chunkDataSize, SEEK_CUR);
//                if (chunkDataSize % 2 != 0)
//                {
//                    fseek(m_inputFile, 1, SEEK_CUR);
//                }

//                m_otherChunksCount++;

//                fprintf(stdout, "Found chunk type \'%c%c%c%c\', size: %d bytes\n", nextChunkID[0], nextChunkID[1], nextChunkID[2], nextChunkID[3], chunkDataSize);
//            }
//        }

//        // Did we get enough data from the input file to proceed?

//        if (strncmp(&(m_formatChunk.chunkID[0]), "\0\0\0\0", 4) == 0 || (m_dataChunkLocation.size == 0))
//        {
//            fprintf(stderr, "Input file did not contain any format data or did not contain any sample data\n");
//            cleanUp();
//            return -1;
//        }

//        return 0;
//    }

//    int readMarkersFile(const char* markerFilePath, enum CuePointMergingOption mergeOption = ReplaceAnyExistingCuePoints)
//    {
//        // Open the Markers file
//        m_markersFile = fopen(markerFilePath, "rb");
//        if (m_markersFile == nullptr)
//        {
//            fprintf(stderr, "Could not open marker file %s\n", markerFilePath);
//            cleanUp();
//            return -1;
//        }

//        // Read in the Markers File
//        fprintf(stdout, "Reading markers file.\n");

//        // The markers file should contain a locations for each marker (cue point), one location per line
//        std::vector<uint32_t> cueLocations;

//        while (!feof(m_markersFile))
//        {
//            char cueLocationString[11] = {0}; // Max Value for a 32 bit int is 4,294,967,295, i.e. 10 numeric digits, so char[11] should be enough storage for all the digits in a line + a terminator (\0).
//            int charIndex = 0;

//            // Loop through each line int the markers file
//            while (1)
//            {
//                char nextChar = fgetc(m_markersFile);

//                // check for end of file
//                if (feof(m_markersFile))
//                {
//                    cueLocationString[charIndex] = '\0';
//                    break;
//                }

//                // check for end of line
//                if (nextChar == '\r')
//                {
//                    // This is a Classic Mac line ending '\r' or the start of a Windows line ending '\r\n'
//                    // If this is the start of a '\r\n', gobble up the '\n' too
//                    char peekAheadChar = fgetc(m_markersFile);
//                    if ((peekAheadChar != EOF) && (peekAheadChar != '\n'))
//                    {
//                        ungetc(peekAheadChar, m_markersFile);
//                    }

//                    cueLocationString[charIndex] = '\0';
//                    break;
//                }
//                if (nextChar == '\n')
//                {
//                    // This is a Unix/ OS X line ending '\n'
//                    cueLocationString[charIndex] = '\0';
//                    break;
//                }


//                if ( nextChar >= '0' && nextChar <= '9')
//                {
//                    // This is a regular numeric character, if there are less than 10 digits in the cueLocationString, add this character.
//                    // More than 10 digits is too much for a 32bit unsigned integer, so ignore this character and spin through the loop until we hit EOL or EOF
//                    if (charIndex < 10)
//                    {
//                        cueLocationString[charIndex] = nextChar;
//                        charIndex++;
//                    }
//                    else
//                    {
//                        fprintf(stderr, "Line %llu in marker file contains too many numeric digits (>10). Max value for a sample location is 4,294,967,295\n", cueLocations.size() + 1);
//                    }
//                }
//                else
//                {
//                    // This is an invalid character
//                    fprintf(stderr, "Invalid character in marker file: \'%c\' at line %llu column %d.  Ignoring this character\n", nextChar, cueLocations.size() + 1, charIndex + 1);
//                }
//            }


//            // Convert the digits from the line to a uint32 and add to cueLocations
//            if (strlen(cueLocationString) > 0)
//            {
//                uint32_t cueLocation_Long = strtol(cueLocationString, nullptr, 10);
//                if (cueLocation_Long < UINT32_MAX)
//                {
//                    cueLocations.push_back(cueLocation_Long);
//                }
//                else
//                {
//                    fprintf(stderr, "Line %llu in marker file contains a value larger than the max possible sample location value\n", cueLocations.size() + 1);
//                }
//            }
//        }


//        // Did we get any cueLocations?
//        if (cueLocations.size() < 1)
//        {
//            fprintf(stderr, "Did not find any cue point locations in the markers file\n");
//            cleanUp();
//            return -1;
//        }

//        fprintf(stdout, "Read %llu cue locations from markers file.\nPreparing new cue chunk.\n", cueLocations.size());


//        // Create CuePointStructs for each cue location
//        std::vector<CuePoint> cuePoints;

//        //  uint16_t bitsPerSample = littleEndianBytesToUInt16(formatChunk->significantBitsPerSample);
//        //  uint16_t bytesPerSample = bitsPerSample / 8;

//        for (uint32_t i = 0; i < cueLocations.size(); i++)
//        {
//            cuePoints.emplace_back(i + 1, cueLocations[i]);
//        }


//        // If necesary, merge the cuePoints with any existing cue points from the input file
//        if ( (mergeOption == MergeWithAnyExistingCuePoints) && (!m_existingCueChunk.cuePoints.empty()) )
//        {
//            //...
//        }

//        // Populate the CueChunk Struct
//        m_cueChunk.chunkDataSize = 4 + (sizeof(CuePoint) * cueLocations.size());// See struct definition
//        m_cueChunk.cuePointsCount = cueLocations.size();
//        m_cueChunk.cuePoints = std::move(cuePoints);

//        return 0;
//    }

//    int writeWaveFile(const char* outFilePath)
//    {
//        // Open the output file for writing
//        m_outputFile = fopen(outFilePath, "wb");
//        if (m_outputFile == nullptr)
//        {
//            fprintf(stderr, "Could not open output file %s\n", outFilePath);
//            cleanUp();
//            return -1;
//        }

//        fprintf(stdout, "Writing output file.\n");

//        // Update the file header chunk to have the new data size
//        uint32_t fileDataSize = 0;
//        fileDataSize += 4; // the 4 bytes for the Riff Type "WAVE"
//        fileDataSize += sizeof(FormatChunk);
//        fileDataSize += m_formatChunkExtraBytes.size;
//        if (m_formatChunkExtraBytes.size % 2 != 0)
//        {
//            fileDataSize++; // Padding byte for 2byte alignment
//        }

//        fileDataSize += m_dataChunkLocation.size;
//        if (m_dataChunkLocation.size % 2 != 0)
//        {
//            fileDataSize++;
//        }

//        for (int i = 0; i < m_otherChunksCount; i++)
//        {
//            fileDataSize += m_otherChunkLocations[i].size;
//            if (m_otherChunkLocations[i].size % 2 != 0)
//            {
//                fileDataSize++;
//            }
//        }
//        fileDataSize += 4; // 4 bytes for CueChunk ID "cue "
//        fileDataSize += 4; // UInt32 for CueChunk.chunkDataSize
//        fileDataSize += 4; // UInt32 for CueChunk.cuePointsCount
//        fileDataSize += (sizeof(CuePoint) * m_cueChunk.cuePointsCount.getInt());

//        m_waveHeader.dataSize = fileDataSize;

//        // Write out the header to the new file
//        if (fwrite(&m_waveHeader, sizeof(m_waveHeader), 1, m_outputFile) < 1)
//        {
//            fprintf(stderr, "Error writing header to output file.\n");
//            cleanUp();
//            return -1;
//        }


//        // Write out the format chunk
//        if (fwrite(&m_formatChunk, sizeof(FormatChunk), 1, m_outputFile) < 1)
//        {
//            fprintf(stderr, "Error writing format chunk to output file.\n");
//            cleanUp();
//            return -1;
//        }
//        else if (m_formatChunkExtraBytes.size > 0)
//        {
//            if (writeChunkLocationFromInputFileToOutputFile(m_formatChunkExtraBytes, m_inputFile, m_outputFile) < 0)
//            {
//                cleanUp();
//                return -1;
//            }
//            if (m_formatChunkExtraBytes.size % 2 != 0)
//            {
//                if (fwrite("\0", sizeof(char), 1, m_outputFile) < 1)
//                {
//                    fprintf(stderr, "Error writing padding character to output file.\n");
//                    cleanUp();
//                    return -1;

//                }
//            }
//        }


//        // Write out the start of new Cue Chunk: chunkID, dataSize and cuePointsCount
//        if (fwrite(&m_cueChunk, sizeof(m_cueChunk.chunkID) + sizeof(m_cueChunk.chunkDataSize)+ sizeof(m_cueChunk.cuePointsCount), 1, m_outputFile) < 1)
//        {
//            fprintf(stderr, "Error writing cue chunk header to output file.\n");
//            cleanUp();
//            return -1;
//        }

//        // Write out the Cue Points
//        for (const CuePoint& cuePoint: m_cueChunk.cuePoints)
//        {
//            if (fwrite(&cuePoint, sizeof(CuePoint), 1, m_outputFile) < 1)
//            {
//                fprintf(stderr, "Error writing cue point to output file.\n");
//                cleanUp();
//                return -1;
//            }
//        }


//        // Write out the other chunks from the input file
//        for (int i = 0; i < m_otherChunksCount; i++)
//        {
//            if (writeChunkLocationFromInputFileToOutputFile(m_otherChunkLocations[i], m_inputFile, m_outputFile) < 0)
//            {
//                cleanUp();
//                return -1;
//            }
//            if (m_otherChunkLocations[i].size % 2 != 0)
//            {
//                if (fwrite("\0", sizeof(char), 1, m_outputFile) < 1)
//                {
//                    fprintf(stderr, "Error writing padding character to output file.\n");
//                    cleanUp();
//                    return -1;

//                }
//            }
//        }


//        // Write out the data chunk
//        if (writeChunkLocationFromInputFileToOutputFile(m_dataChunkLocation, m_inputFile, m_outputFile) < 0)
//        {
//            cleanUp();
//            return -1;
//        }
//        if (m_dataChunkLocation.size % 2 != 0)
//        {
//            if (fwrite("\0", sizeof(char), 1, m_outputFile) < 1)
//            {
//                fprintf(stderr, "Error writing padding character to output file.\n");
//                cleanUp();
//                return -1;

//            }
//        }

//        return 0;
//    }

//    void cleanUp()
//    {
//        if (m_inputFile != nullptr) fclose(m_inputFile);
//        if (m_markersFile != nullptr) fclose(m_markersFile);
//        if (m_outputFile != nullptr) fclose(m_outputFile);
//    }
//private:
//    static const int maxOtherChunks = 256;   // How many other chunks can we expect to find?  Who knows! So lets pull 256 out of the air.  That's a nice computery number.

//    FILE* m_inputFile{nullptr};
//    FILE* m_outputFile{nullptr};
//    FILE* m_markersFile{nullptr};

//    WaveHeader m_waveHeader;
//    FormatChunk m_formatChunk;

//    CueChunk m_cueChunk;
//    CueChunk m_existingCueChunk;

//    int m_otherChunksCount = 0;

//    ChunkLocation m_formatChunkExtraBytes;
//    ChunkLocation m_dataChunkLocation;
//    ChunkLocation m_otherChunkLocations[maxOtherChunks];
//};



//int writeChunkLocationFromInputFileToOutputFile(ChunkLocation chunk, FILE *inputFile, FILE *outputFile)
//{
//    // note the position of he input filr to restore later
//    long inputFileOrigLocation = ftell(inputFile);

//    if (fseek(inputFile, chunk.startOffset, SEEK_SET) < 0)
//    {
//        fprintf(stderr, "Error: could not seek input file to location %ld", chunk.startOffset);
//        return -1;
//    }

//    long remainingBytesToWrite = chunk.size;
//    while (remainingBytesToWrite >=1024)
//    {
//        char buffer[1024];

//        fread(buffer, sizeof(char), 1024, inputFile);
//        if (ferror(inputFile) != 0)
//        {
//            fprintf(stderr, "Copy chunk: Error reading input file");
//            fseek(inputFile, inputFileOrigLocation, SEEK_SET);
//            return -1;
//        }

//        if (fwrite(buffer, sizeof(char), 1024, outputFile) < 1)
//        {
//            fprintf(stderr, "Copy chunk: Error writing output file");
//            fseek(inputFile, inputFileOrigLocation, SEEK_SET);
//            return -1;
//        }
//        remainingBytesToWrite -= 1024;
//    }

//    if (remainingBytesToWrite > 0)
//    {
//        char buffer[remainingBytesToWrite];

//        fread(buffer, sizeof(char), remainingBytesToWrite, inputFile);
//        if (ferror(inputFile) != 0)
//        {
//            fprintf(stderr, "Copy chunk: Error reading input file");
//            fseek(inputFile, inputFileOrigLocation, SEEK_SET);
//            return -1;
//        }

//        if (fwrite(buffer, sizeof(char), remainingBytesToWrite, outputFile) < 1)
//        {
//            fprintf(stderr, "Copy chunk: Error writing output file");
//            fseek(inputFile, inputFileOrigLocation, SEEK_SET);
//            return -1;
//        }
//    }

//    return 0;
//}

