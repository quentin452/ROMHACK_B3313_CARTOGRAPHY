#include <romhack_b3313_cartography/utils/rom_utils.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <psapi.h>

#include <tlhelp32.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

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

int getNumStarsFromMask(int mask, const std::vector<uint8_t> &saveData, int offset) {
    int numStars = 0;
    std::cout << "Starting getNumStarsFromMask with mask: " << std::hex << mask << std::dec << std::endl;

    std::ofstream logFile("mask_check_log.txt", std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file." << std::endl;
        return numStars;
    }

    if (offset >= saveData.size()) {
        std::cerr << "Offset out of bounds." << std::endl;
        return numStars;
    }

    // Vérifier un seul byte
    int byteIndex = offset; // Pas de décalage basé sur les bits, vérifier tout le byte
    if (byteIndex < saveData.size()) {
        uint8_t byteValue = saveData[byteIndex];
        for (int bit = 0; bit < 32; ++bit) {
            if (mask & (1 << bit)) {
                int bitIndex = bit % 8;
                bool isCollected = (byteValue & (1 << bitIndex)) != 0;
                if (isCollected) {
                    numStars++;
                }
            }
        }
    } else {
        logFile << "Byte index out of range: " << byteIndex << std::endl;
    }

    logFile << "Mask: " << std::hex << mask << ", Offset: " << offset << ", NumStars: " << numStars << std::endl;
    logFile.close();
    std::cout << "Mask: " << mask << " Offset: " << offset << " NumStars: " << numStars << std::endl;
    return numStars;
}
