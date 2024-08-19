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

int getNumStarsFromMask(int mask, const std::vector<uint8_t> &saveData, int offset) {
    int numStars = 0;
    //std::cout << "Starting getNumStarsFromMask with mask: " << std::hex << mask << std::dec << std::endl;

    //std::ofstream logFile("mask_check_log.txt", std::ios::app);
    //if (!logFile.is_open()) {
       // std::cerr << "Failed to open log file." << std::endl;
    //    return numStars;
   // }

    if (offset >= saveData.size()) {
       // std::cerr << "Offset out of bounds." << std::endl;
        return numStars;
    }

   // logFile << "Offset: " << offset << ", SaveData Size: " << saveData.size() << std::endl;

    // Itérer à travers tous les bits du masque
    for (int bit = 0; bit < 32; ++bit) {
        if (mask & (1 << bit)) {
            int byteOffset = offset + (bit / 8); // Calculer l'index du byte
            int bitIndex = bit % 8;              // Calculer l'index du bit à vérifier

            // Vérifier que byteOffset est dans les limites
            if (byteOffset < saveData.size()) {
                //logFile << "Bit: " << bit
               //         << ", ByteOffset: " << byteOffset
               //         << ", BitIndex: " << bitIndex
               //         << ", ByteValue: " << +saveData[byteOffset] << std::endl;

                // Incrémenter le compteur pour chaque bit du masque, peu importe l'état
                numStars++;
            } else {
                //logFile << "Byte index out of range: " << byteOffset << std::endl;
                break; // Exit loop if out of range
            }
        }
    }

    /*logFile << "Mask: " << std::hex << mask
            << ", Offset: " << offset
            << ", NumStars: " << numStars << std::endl;*/
    //logFile.close();
   // std::cout << "Mask: " << mask << " Offset: " << offset << " NumStars: " << numStars << std::endl;
    return numStars;
}
bool isStarCollected(int mask, const std::vector<uint8_t> &saveData, int offset) {
   // std::cout << "Starting isStarCollected with mask: " << std::hex << mask << std::dec << std::endl;

   // std::ofstream logFile("star_check_log.txt", std::ios::app);
    //if (!logFile.is_open()) {
       // std::cerr << "Failed to open log file." << std::endl;
  //      return false;
   // }

    if (offset >= saveData.size()) {
     //   std::cerr << "Offset out of bounds." << std::endl;
        return false;
    }

 //   logFile << "Offset: " << offset << ", SaveData Size: " << saveData.size() << std::endl;

    // Itérer à travers tous les bits du masque
    for (int bit = 0; bit < 32; ++bit) {
        if (mask & (1 << bit)) {
            int byteOffset = offset + (bit / 8); // Calculer l'index du byte
            int bitIndex = bit % 8;              // Calculer l'index du bit à vérifier

            // Vérifier que byteOffset est dans les limites
            if (byteOffset < saveData.size()) {
                /*logFile << "Bit: " << bit
                        << ", ByteOffset: " << byteOffset
                        << ", BitIndex: " << bitIndex
                        << ", ByteValue: " << +saveData[byteOffset] << std::endl;*/

                // Vérifier si le bit est défini (étoile collectée)
                if (saveData[byteOffset] & (1 << bitIndex)) {
                  //  logFile << "Star collected at bit: " << bit << std::endl;
                   // logFile.close();
                    return true;
                }
            } else {
              //  logFile << "Byte index out of range: " << byteOffset << std::endl;
                break; // Exit loop if out of range
            }
        }
    }

   /* logFile << "Mask: " << std::hex << mask
            << ", Offset: " << offset
            << ", Star collected: false" << std::endl;*/
   // logFile.close();
   // std::cout << "Mask: " << mask << " Offset: " << offset << " Star collected: false" << std::endl;
    return false;
}
