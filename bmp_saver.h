#pragma once

#include <string>
#include <vector>

class BMPSaver {
public:
    static bool saveFrameBuffer(const std::string& filename,
        int width, int height);

    static bool saveDepthMapAsBMP(const std::vector<std::vector<double>>& depthData,
        const std::string& filename);

private:
#pragma pack(push, 1)
    struct BMPHeader {
        uint16_t signature = 0x4D42; // "BM"
        uint32_t fileSize = 0;
        uint16_t reserved1 = 0;
        uint16_t reserved2 = 0;
        uint32_t dataOffset = 54;

        uint32_t headerSize = 40;
        int32_t width = 0;
        int32_t height = 0;
        uint16_t planes = 1;
        uint16_t bitsPerPixel = 24;
        uint32_t compression = 0;
        uint32_t imageSize = 0;
        int32_t xPixelsPerMeter = 2835;
        int32_t yPixelsPerMeter = 2835;
        uint32_t colorsUsed = 0;
        uint32_t colorsImportant = 0;
    };
#pragma pack(pop)

    static void normalizeDepthData(const std::vector<std::vector<double>>& depthData,
        std::vector<uint8_t>& pixels);
}; 
