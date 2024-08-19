#ifndef ROM_UTILS_H
#define ROM_UTILS_H

#include <cstdint>
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

std::string GetProcessPath(const std::string &processName);
int getNumStarsFromMask(int mask, const std::vector<uint8_t> &saveData, int offset);
bool isStarCollected(int mask, const std::vector<uint8_t> &saveData, int offset);
#endif // ROM_UTILS_H
