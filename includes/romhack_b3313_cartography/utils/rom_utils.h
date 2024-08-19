#ifndef ROM_UTILS_H
#define ROM_UTILS_H

#include <string>
#include <cstdint> 
#include <vector>
std::string GetProcessPath(const std::string &processName);
int getNumStarsFromMask(int mask, const std::vector<uint8_t> &saveData, int offset);

#endif // ROM_UTILS_H
