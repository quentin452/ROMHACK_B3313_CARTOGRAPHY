#ifndef ROM_UTILS_H
#define ROM_UTILS_H

#include <QString>
#include <codecvt>
#include <cstdint>
#include <romhack_b3313_cartography/utils/byteswap.hpp>
#include <romhack_b3313_cartography/utils/types.hpp>
#include <string>
#include <vector>

enum class SaveFormat : ubyte {
    EEPROM = 0,
    SRAM = 1,
    FlashRAM = 2,
    MemPak = 3,
    RawSRM = 4
};
struct SaveParams {
    SaveFormat saveFormat;
    uint slotsStart;
    uint numSlots;
    uint slotSize;
    uint activeBit;
    uint checksumOffset;
};

bool isStarCollected(const std::vector<uint8_t> &saveData, int offset, int index, int slotIndex = 0, int slotSize = 0);
void fixEndianness(uint *data, size_t words);
std::vector<uint8_t> ReadSrmFile(const std::string &filePath, const SaveParams &params);
SaveFormat parseSaveFormat(const std::string &saveType);
std::string GetParallelLauncherSaveLocation();
std::string QStringToStdString(const QString &qstr);
QString StdStringToQString(const std::string &str);
bool isEmulatorDetected(const std::vector<std::wstring> &emulators, std::wstring &detectedEmulator);
bool isRomHackLoaded(const std::wstring &targetProcessName);
void fixEndianness(uint *data, size_t words);

extern std::vector<std::wstring> parallelLauncher;
extern std::vector<std::wstring> retroarch;
#endif // ROM_UTILS_H
