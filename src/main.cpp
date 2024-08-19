#include <SFML/Graphics.hpp>

#include <windows.h>

#include <psapi.h>

#include <algorithm>
#include <chrono>
#include <codecvt>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <nlohmann/json.hpp>
#include <romhack_b3313_cartography/utils/byteswap.hpp>

#include <romhack_b3313_cartography/utils/rom_utils.h>

#include <romhack_b3313_cartography/uis/Textures.h>

#include <romhack_b3313_cartography/uis/Button.h>
#include <romhack_b3313_cartography/uis/DropdownMenu.h>
#include <romhack_b3313_cartography/uis/Node.h>
#include <romhack_b3313_cartography/uis/StarDisplay.h>
#include <romhack_b3313_cartography/uis/TabManager.hpp>
#include <string>
#include <tlhelp32.h>
#include <vector>
#include <windows.h>
std::vector<std::wstring> parallelLauncher = {L"parallel-launcher.exe"};
std::vector<std::wstring> retroarch = {L"retroarch.exe"};
using json = nlohmann::json;
std::vector<std::string> generateTabNames(int numSlots) {
    std::vector<std::string> tabNames;
    for (int i = 0; i < numSlots; ++i) {
        if (i < 26) {
            tabNames.push_back("MARIO " + std::string(1, 'A' + i));
        } else {
            tabNames.push_back("MARIO " + std::to_string(i - 25));
        }
    }
    return tabNames;
}
std::string GetParallelLauncherSaveLocation() {
    char *userProfile = getenv("USERPROFILE");
    if (userProfile == nullptr) {
        std::cerr << "Erreur: Impossible de récupérer le chemin du répertoire utilisateur." << std::endl;
        return "";
    }
    std::string saveLocation = std::string(userProfile) + "\\AppData\\Local\\parallel-launcher\\data\\retro-data\\saves\\sync";
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

void saveNodes(const std::vector<Node> &nodes, const std::string &filename) {
    json j;
    for (const auto &node : nodes) {
        j.push_back(node.toJson());
    }
    std::ofstream file(filename);
    file << j.dump(4); // Pretty print JSON with an indent of 4 spaces
}

std::vector<Node> loadNodes(const std::string &filename, sf::Font &font) {
    std::ifstream file(filename);
    if (!file.is_open())
        return {};
    json j;
    file >> j;
    std::vector<Node> nodes;
    for (const auto &item : j) {
        nodes.push_back(Node::fromJson(item, font));
    }
    return nodes;
}
bool isMouseOverNode(const std::vector<Node> &nodes, sf::Vector2i mousePos, int &nodeIndex) {
    for (int i = 0; i < nodes.size(); ++i) {
        sf::Vector2f nodePos = nodes[i].shape.getPosition();
        sf::FloatRect nodeBounds(nodePos.x, nodePos.y, nodes[i].shape.getRadius() * 2, nodes[i].shape.getRadius() * 2);
        if (nodeBounds.contains(static_cast<sf::Vector2f>(mousePos))) {
            nodeIndex = i;
            return true;
        }
    }
    return false;
}
std::wstring stringToWstring(const std::string &str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

bool isEmulatorDetected(const std::vector<std::wstring>& emulators, std::wstring& detectedEmulator) {
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
        std::wstring processName = stringToWstring(pe32.szExeFile);
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
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return false;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return false;
    }
    do {
        std::string processName = pe32.szExeFile;
        if (processName.find("-") != std::string::npos) {
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
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Erreur lors de l'ouverture du fichier: " << filePath << std::endl;
        return {};
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    uint saveFileSize = params.slotSize * params.numSlots;
    std::vector<uint8_t> buffer(saveFileSize);

    switch (params.saveFormat) {
    case SaveFormat::EEPROM: {
        if (size < 0x800) {
            std::cerr << "Erreur: La taille du fichier est inférieure à la taille attendue pour EEPROM." << std::endl;
            return {};
        }
        file.seekg(0);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        break;
    }
    case SaveFormat::SRAM: {
        if (size < 0x8000) {
            std::cerr << "Erreur: La taille du fichier est inférieure à la taille attendue pour SRAM." << std::endl;
            return {};
        }
        file.seekg(0x20800u);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        fixEndianness(reinterpret_cast<uint *>(buffer.data()), buffer.size() / sizeof(uint));
        break;
    }
    case SaveFormat::FlashRAM: {
        if (size < 0x20000) {
            std::cerr << "Erreur: La taille du fichier est inférieure à la taille attendue pour FlashRAM." << std::endl;
            return {};
        }
        file.seekg(0x28800);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        fixEndianness(reinterpret_cast<uint *>(buffer.data()), buffer.size() / sizeof(uint));
        break;
    }
    case SaveFormat::MemPak: {
        if (size < 0x20000) {
            std::cerr << "Erreur: La taille du fichier est inférieure à la taille attendue pour MemPak." << std::endl;
            return {};
        }
        file.seekg(0x800);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        break;
    }
    default: {
        std::cerr << "Format de sauvegarde non reconnu." << std::endl;
        return {};
    }
    }

    if (file.bad()) {
        std::cerr << "Erreur lors de la lecture du fichier." << std::endl;
        return {};
    }

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
int main(int argc, char *argv[]) {
    std::string retroarch_path = GetProcessPath("retroarch.exe");
    std::cout << "RetroArch path: " << retroarch_path << std::endl;
    std::ifstream star_layout("resources/stars_layout/b3313-V1.0.2/layout.json");
    json jsonData;
    if (!star_layout.is_open() || !(star_layout >> jsonData)) {
        std::cerr << "Erreur lors du chargement du fichier JSON." << std::endl;
        return 1;
    }
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Mind Map Example");
    // Charger textures
    if (!textures.loadTextures()) {
        std::cerr << "[ERREUR] Échec du chargement des textures." << std::endl;
        return 1;
    }
    // Créer les objets
    Button saveButton(1150, 10, textures.saveTexture);               // Positionnez le bouton comme vous le souhaitez
    Button switchViewButton(550, 90, textures.starCollectedTexture); // Positionnez le bouton comme vous le souhaitez
    std::vector<std::string> menuItems = {"b3313-v1.0.2.json", "another_file.json"};
    sf::Font font;
    if (!font.loadFromFile("resources/assets/Arial.ttf"))
        return 1;
    DropdownMenu dropdownMenu(1150, 50, textures.dropdownTexture, menuItems, font); // Positionnez le menu comme vous le souhaitez
    std::vector<Node> nodes = loadNodes("b3313-v1.0.2.json", font);
    sf::Vector2i startPos;
    bool dragging = false;
    int startNodeIndex = -1;
    std::vector<std::pair<int, int>> connections;
    sf::Text emulatorText;
    emulatorText.setFont(font);
    emulatorText.setString("Is parallel launcher running?");
    emulatorText.setCharacterSize(24);
    emulatorText.setPosition(10, 10);
    sf::Text b3313Text;
    b3313Text.setFont(font);
    b3313Text.setString("Is b3313 V1.0.2 ROM IS LOADED?");
    b3313Text.setCharacterSize(24);
    b3313Text.setPosition(10, 40);
    bool showStarDisplay = false; // Contrôle pour afficher l'affichage des étoiles
    StarDisplay starDisplay;      // Instance de la classe StarDisplay
    sf::View scrollView(sf::FloatRect(0, 0, 1280, 720));
    window.setView(scrollView);
    std::vector<std::string> tabNames;
    TabManager tabManager(tabNames, font);
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (showStarDisplay && event.type == sf::Event::MouseWheelScrolled) {
                // Défilement avec la molette de la souris
                float offset = event.mouseWheelScroll.delta * -10.0f; // Ajustez la vitesse de défilement
                sf::Vector2f newCenter = scrollView.getCenter() + sf::Vector2f(0, offset);
                scrollView.setCenter(newCenter);
                window.setView(scrollView);
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    tabManager.handleMouseClick(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                    if (saveButton.isClicked(sf::Vector2i(event.mouseButton.x, event.mouseButton.y))) {
                        saveNodes(nodes, "b3313-v1.0.2.json");
                    } else if (dropdownMenu.isClicked(sf::Vector2i(event.mouseButton.x, event.mouseButton.y))) {
                        std::string selectedFile = dropdownMenu.getSelectedItem(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                        if (!selectedFile.empty())
                            nodes = loadNodes(selectedFile, font);
                    } else if (switchViewButton.isClicked(sf::Vector2i(event.mouseButton.x, event.mouseButton.y))) {
                        showStarDisplay = !showStarDisplay; // Basculer l'état d'affichage des étoiles
                    } else if (isMouseOverNode(nodes, sf::Vector2i(event.mouseButton.x, event.mouseButton.y), startNodeIndex)) {
                        startPos = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
                        dragging = true;
                    }
                }
#ifdef DEBUG
                if (event.mouseButton.button == sf::Mouse::Right) {
                    sf::Vector2f worldPos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                    nodes.emplace_back(worldPos.x, worldPos.y, "New Node", font);
                }
#endif
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left && dragging) {
                    int endNodeIndex = -1;
                    if (isMouseOverNode(nodes, sf::Vector2i(event.mouseButton.x, event.mouseButton.y), endNodeIndex) && endNodeIndex != startNodeIndex) {
                        connections.push_back({startNodeIndex, endNodeIndex});
                        nodes[startNodeIndex].connections.push_back(endNodeIndex);
                        nodes[endNodeIndex].connections.push_back(startNodeIndex);
                    }
                    dragging = false;
                }
            }
            if (event.type == sf::Event::MouseMoved && dragging) {
                // Handle dragging logic if needed
            }
        }
        window.clear(sf::Color::White);
        std::wstring detectedEmulator;
        if (isEmulatorDetected(parallelLauncher, detectedEmulator))
            emulatorText.setFillColor(sf::Color::Green);
        else
            emulatorText.setFillColor(sf::Color::Black);
        if (isRomHackLoaded(detectedEmulator))
            b3313Text.setFillColor(sf::Color::Green);
        else
            b3313Text.setFillColor(sf::Color::Black);
        if (showStarDisplay) {
            if (isRomHackLoaded(detectedEmulator)) {
                std::string saveLocation = GetParallelLauncherSaveLocation();
                std::cerr << "1" << std::endl;

                // Récupérer les paramètres de sauvegarde depuis jsonData
                const json &format = jsonData["format"];
                SaveParams params;
                params.saveFormat = parseSaveFormat(format["save_type"].get<std::string>());
                params.slotsStart = format["slots_start"].get<uint>();
                params.slotSize = format["slot_size"].get<uint>();
                params.activeBit = format["active_bit"].get<uint>();
                params.checksumOffset = format["checksum_offset"].get<uint>();

                auto saveData = ReadSrmFile(saveLocation, params);
                std::cerr << "2" << std::endl;
                if (!saveData.empty()) {
                    int yOffset = 0;
                    std::cerr << "2_3" << std::endl;
                    int numSlots = params.numSlots;
                    std::cerr << "2_4" << std::endl;
                    std::vector<std::string> tabNames;
                    std::cerr << "2_5" << std::endl;
                    for (int i = 1; i <= numSlots; ++i) {
                        tabNames.push_back("Mario " + std::to_string(i));
                    }
                    std::cerr << "2_6" << std::endl;
                    tabManager.initializeTabs(tabNames);
                    std::cerr << "3" << std::endl;

                    std::string currentTabName = tabManager.getCurrentTabName();
                    std::cerr << "4" << std::endl;

                    for (int i = 0; i < numSlots; ++i) {
                        std::string tabName = tabNames[i];
                        if (tabName == currentTabName) {
                            sf::Text tabText;
                            tabText.setFont(font);
                            tabText.setString(tabName);
                            tabText.setCharacterSize(24);
                            tabText.setFillColor(sf::Color::Black);
                            tabText.setPosition(100, 100 + yOffset);
                            window.draw(tabText);

                            yOffset += 30;
                            std::cerr << "5" << std::endl;

                            for (const auto &group : jsonData["groups"]) {
                                std::string groupName = group["name"];
                                std::vector<StarData> groupStarData;

                                sf::Text groupText;
                                groupText.setFont(font);
                                groupText.setString(groupName);
                                groupText.setCharacterSize(24);
                                groupText.setFillColor(sf::Color::Black);
                                groupText.setPosition(100, 130 + yOffset);
                                window.draw(groupText);

                                yOffset += 30;

                                for (const auto &course : group["courses"]) {
                                    std::string courseName = course["name"];
                                    std::vector<StarData> courseStarList;

                                    for (const auto &data : course["data"]) {
                                        int offset = data["offset"];
                                        int mask = data["mask"];
                                        int numStars = getNumStarsFromMask(mask, saveData, offset);
                                        bool star_collected = false;

                                        courseStarList.push_back({courseName, numStars, star_collected, offset, mask});
                                    }

                                    groupStarData.insert(groupStarData.end(), courseStarList.begin(), courseStarList.end());
                                }

                                starDisplay.afficherEtoilesGroupe(groupName, groupStarData, window, font, yOffset);
                            }

                            tabManager.draw(window);
                            break;
                        }
                    }
                }
            } else {
                std::cerr << "Aucun émulateur détecté !" << std::endl;
            }
        } else {
            // Dessiner les connexions et les nœuds
            for (const auto &conn : connections) {
                sf::Vertex line[] = {
                    sf::Vertex(nodes[conn.first].shape.getPosition() + sf::Vector2f(30, 30), sf::Color::Black),
                    sf::Vertex(nodes[conn.second].shape.getPosition() + sf::Vector2f(30, 30), sf::Color::Black)};
                window.draw(line, 2, sf::Lines);
                // Draw arrow
                sf::Vector2f dir = nodes[conn.second].shape.getPosition() - nodes[conn.first].shape.getPosition();
                float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                dir /= length;
                float arrowSize = 10.f;
                sf::Vector2f arrowPoint1 = nodes[conn.second].shape.getPosition() - dir * (arrowSize + 10.f) + sf::Vector2f(-dir.y * arrowSize, dir.x * arrowSize);
                sf::Vector2f arrowPoint2 = nodes[conn.second].shape.getPosition() - dir * (arrowSize + 10.f) + sf::Vector2f(dir.y * arrowSize, -dir.x * arrowSize);
                sf::Vertex arrow[] = {
                    sf::Vertex(nodes[conn.second].shape.getPosition(), sf::Color::Black),
                    sf::Vertex(arrowPoint1, sf::Color::Black),
                    sf::Vertex(arrowPoint2, sf::Color::Black)};
                window.draw(arrow, 3, sf::Triangles);
            }
            // Draw nodes
            for (const auto &node : nodes) {
                node.draw(window);
            }
            // Draw button and menu
            saveButton.draw(window);
            dropdownMenu.draw(window);
            switchViewButton.draw(window); // Dessiner le bouton pour changer de vue
            // Draw emulator text
            window.draw(emulatorText);
            window.draw(b3313Text);
        }
        window.display();
    }
    return 0;
}