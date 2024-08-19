#include <romhack_b3313_cartography/utils/rom_utils.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <psapi.h>

#include <tlhelp32.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
SaveFormat saveFormat;

std::string GetProcessPath(const std::string &processName) {
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return "Error: Unable to create snapshot";
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return "Error: Unable to get first process";
    }

    do {
        if (processName == pe32.szExeFile) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
            if (hProcess) {
                char path[MAX_PATH];
                if (GetModuleFileNameEx(hProcess, NULL, path, MAX_PATH)) {
                    CloseHandle(hProcess);
                    CloseHandle(hProcessSnap);
                    return std::string(path);
                }
                CloseHandle(hProcess);
            }
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return "Process not found";
}

bool isStarCollected(const std::vector<uint8_t> &saveData, int offset, int index, int slotIndex, int slotSize) {
    if (index < 0 || index >= 32) {
        std::cerr << "Index out of bounds." << std::endl;
        return false;
    }

    int byteOffset = offset + (index / 8) + slotIndex * slotSize;
    int bitIndex = index % 8;

    if (byteOffset < 0 || byteOffset >= static_cast<int>(saveData.size())) {
        std::cerr << "Byte offset out of bounds: " << byteOffset << std::endl;
        return false;
    }

    uint8_t byteData = saveData[byteOffset];
    bool collected = (byteData & (1 << bitIndex)) != 0;

    return collected;
}
