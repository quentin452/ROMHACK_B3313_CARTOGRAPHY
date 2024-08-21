#include <romhack_b3313_cartography/utils/rom_utils.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <psapi.h>

#include <tlhelp32.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>


std::vector<std::wstring> parallelLauncher = {L"parallel-launcher.exe"};
std::vector<std::wstring> retroarch = {L"retroarch.exe"};
SaveFormat saveFormat;
std::string QStringToStdString(const QString &qstr) {
    return qstr.toStdString();
}

QString StdStringToQString(const std::string &str) {
    return QString::fromStdString(str);
}
std::wstring stringToWstring(const std::string &str) {
    // Obtenir la longueur nécessaire pour la conversion
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), NULL, 0);
    std::wstring wstr(size_needed, 0);

    // Effectuer la conversion
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), &wstr[0], size_needed);

    return wstr;
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

bool isEmulatorDetected(const std::vector<std::wstring> &emulators, std::wstring &detectedEmulator) {
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return false;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return false;
    }

    do {
        std::wstring processName(pe32.szExeFile);
        if (std::find(emulators.begin(), emulators.end(), processName) != emulators.end()) {
            detectedEmulator = processName;
            CloseHandle(hProcessSnap);
            return true;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return false;
}

bool isRomHackLoaded(const std::wstring &targetProcessName) {
    std::wstring detectedEmulator;
    if (!isEmulatorDetected(retroarch, detectedEmulator))
        return false;

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return false;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return false;
    }

    do {
        // Convert WCHAR[] to std::wstring
        std::wstring processName(pe32.szExeFile);

        // Convert std::wstring to std::string if necessary
        std::string processNameStr(processName.begin(), processName.end());

        // Check if processNameStr contains "-"
        if (processNameStr.find("-") != std::string::npos) {
            CloseHandle(hProcessSnap);
            return true;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return false;
}
/* Mupen and ParallelN64 current have a bug where the SRAM and FlashRAM
 * is incorrectly stored according to the endianness of the system.
 */
void fixEndianness(uint *data, size_t words) {
    for (size_t i = 0; i < words; i++) {
        data[i] = htonl(data[i]);
    }
}

std::vector<uint8_t> ReadSrmFile(const std::string &filePath, const SaveParams &params) {
    //  std::cerr << "Ouverture du fichier: " << filePath << std::endl;
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Erreur lors de l'ouverture du fichier: " << filePath << std::endl;
        return {};
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // std::cerr << "Taille du fichier: " << size << " octets" << std::endl;

    uint saveFileSize = params.slotSize * params.numSlots;
    // std::cerr << "Paramètres de sauvegarde - slotSize: " << params.slotSize
    //          << ", numSlots: " << params.numSlots
    //          << ", taille attendue pour le buffer: " << saveFileSize << " octets" << std::endl;

    // Vérification de la taille attendue du buffer
    if (saveFileSize == 0) {
        std::cerr << "Erreur: Taille du buffer calculée est 0. Vérifiez les paramètres de sauvegarde." << std::endl;
        return {};
    }

    std::vector<uint8_t> buffer(saveFileSize);

    // std::cerr << "Taille du buffer alloué: " << buffer.size() << " octets" << std::endl;

    switch (params.saveFormat) {
    case SaveFormat::EEPROM: {
        // std::cerr << "Traitement du format EEPROM" << std::endl;
        if (size < 0x800) {
            std::cerr << "Erreur: La taille du fichier est inférieure à la taille attendue pour EEPROM." << std::endl;
            return {};
        }
        file.seekg(0);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        // std::cerr << "Lecture EEPROM effectuée" << std::endl;
        break;
    }
    case SaveFormat::SRAM: {
        std::cerr << "Traitement du format SRAM" << std::endl;
        if (size < 0x8000) {
            std::cerr << "Erreur: La taille du fichier est inférieure à la taille attendue pour SRAM." << std::endl;
            return {};
        }
        file.seekg(0x20800u);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        fixEndianness(reinterpret_cast<uint *>(buffer.data()), buffer.size() / sizeof(uint));
        std::cerr << "Lecture SRAM effectuée" << std::endl;
        break;
    }
    case SaveFormat::FlashRAM: {
        std::cerr << "Traitement du format FlashRAM" << std::endl;
        if (size < 0x20000) {
            std::cerr << "Erreur: La taille du fichier est inférieure à la taille attendue pour FlashRAM." << std::endl;
            return {};
        }
        file.seekg(0x28800);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        fixEndianness(reinterpret_cast<uint *>(buffer.data()), buffer.size() / sizeof(uint));
        std::cerr << "Lecture FlashRAM effectuée" << std::endl;
        break;
    }
    case SaveFormat::MemPak: {
        std::cerr << "Traitement du format MemPak" << std::endl;
        if (size < 0x20000) {
            std::cerr << "Erreur: La taille du fichier est inférieure à la taille attendue pour MemPak." << std::endl;
            return {};
        }
        file.seekg(0x800);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        std::cerr << "Lecture MemPak effectuée" << std::endl;
        break;
    }
    default: {
        std::cerr << "Format de sauvegarde non reconnu: " << static_cast<int>(params.saveFormat) << std::endl;
        return {};
    }
    }

    if (file.bad()) {
        std::cerr << "Erreur lors de la lecture du fichier." << std::endl;
        return {};
    }

    // std::cerr << "Lecture du fichier réussie." << std::endl;

    return buffer;
}

SaveFormat parseSaveFormat(const std::string &saveType) {
    if (saveType == "MemPak")
        return SaveFormat::MemPak;
    if (saveType == "SRAM")
        return SaveFormat::SRAM;
    if (saveType == "FlashRAM")
        return SaveFormat::FlashRAM;
    if (saveType == "EEPROM")
        return SaveFormat::EEPROM;
    if (saveType == "Multi")
        return SaveFormat::RawSRM;
    throw std::invalid_argument("Invalid save_type");
}

std::string GetParallelLauncherSaveLocation() {
    char *userProfile = getenv("USERPROFILE");
    if (userProfile == nullptr) {
        std::cerr << "Erreur: Impossible de récupérer le chemin du répertoire utilisateur." << std::endl;
        return "";
    }

    std::string saveLocation = std::string(userProfile) + "\\AppData\\Local\\parallel-launcher\\data\\retro-data\\saves\\sync";

    // Utilisation de std::filesystem pour vérifier l'existence du répertoire
    if (!std::filesystem::exists(saveLocation)) {
        std::cerr << "Erreur: Le répertoire spécifié n'existe pas." << std::endl;
        return "";
    }

    try {
        for (const auto &entry : std::filesystem::directory_iterator(saveLocation)) {
            if (entry.path().extension() == ".srm") {
                return entry.path().string();
            }
        }
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "Erreur d'accès au répertoire: " << e.what() << std::endl;
    }

    std::cerr << "Aucun fichier .srm trouvé dans le répertoire spécifié." << std::endl;
    return "";
}