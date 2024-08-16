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
#include <romhack_b3313_cartography/Textures.h>

#include <romhack_b3313_cartography/Button.h>
#include <romhack_b3313_cartography/DropdownMenu.h>
#include <romhack_b3313_cartography/Node.h>
#include <romhack_b3313_cartography/StarDisplay.h>
#include <string>
#include <tlhelp32.h>
#include <vector>
#include <windows.h>
const DWORD STAR_OFFSET = 0x33B8; // Offset pour les étoiles (à adapter selon la version du jeu)

using json = nlohmann::json;

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

bool isEmulatorDetected(std::wstring &detectedEmulator) {
    std::vector<std::wstring> emulators = {L"project64.exe", L"wine-preloader.exe", L"mupen64.exe", L"retroarch.exe"};
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
        std::string processNameStr = pe32.szExeFile;
        std::wstring processName = stringToWstring(processNameStr);
        for (const auto &emulator : emulators) {
            if (processName == emulator) {
                detectedEmulator = processName;
                CloseHandle(hProcessSnap);
                return true;
            }
        }
    } while (Process32Next(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    return false;
}

bool readMemory(HANDLE hProcess, DWORD_PTR address, void *buffer, SIZE_T size) { // UNUSED
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQueryEx(hProcess, reinterpret_cast<LPCVOID>(address), &mbi, sizeof(mbi))) {
        if (address >= reinterpret_cast<DWORD_PTR>(mbi.BaseAddress) && address < reinterpret_cast<DWORD_PTR>(mbi.BaseAddress) + mbi.RegionSize) {
            SIZE_T bytesRead;
            if (ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(address), buffer, size, &bytesRead)) {
                if (bytesRead == size) {
                    return true;
                } else {
                    std::cerr << "[ERREUR] Nombre d'octets lus incorrect à l'adresse 0x"
                              << std::hex << address << ". Attendu: "
                              << size << ", Lu: " << bytesRead << std::endl;
                }
            } else {
                std::cerr << "[ERREUR] Lecture mémoire échouée à l'adresse 0x"
                          << std::hex << address << ". Code d'erreur: "
                          << GetLastError() << std::endl;
            }
        } else {
            std::cerr << "[ERREUR] L'adresse est hors de la région mémoire valide : 0x"
                      << std::hex << address << ". BaseAddress: 0x" << std::hex << mbi.BaseAddress << ", RegionSize: " << mbi.RegionSize << std::endl;
        }
    } else {
        std::cerr << "[ERREUR] VirtualQueryEx a échoué à l'adresse 0x"
                  << std::hex << address << ". Code d'erreur: "
                  << GetLastError() << std::endl;
    }
    return false;
}

bool isRomHackLoaded(const std::wstring &targetProcessName) {
    std::wstring detectedEmulator;
    if (!isEmulatorDetected(detectedEmulator))
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

DWORD getProcessID(const std::wstring &targetProcessName) {
    HANDLE hProcessSnap;
    PROCESSENTRY32W pe32; // Utiliser PROCESSENTRY32W pour les noms Unicode
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        std::wcerr << L"[ERREUR] Échec de la capture des processus. Code d'erreur: " << GetLastError() << std::endl;
        return 0;
    }
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    if (!Process32FirstW(hProcessSnap, &pe32)) { // Utiliser Process32FirstW
        std::wcerr << L"[ERREUR] Échec de Process32First. Code d'erreur: " << GetLastError() << std::endl;
        CloseHandle(hProcessSnap);
        return 0;
    }
    do {
        std::wstring processName = pe32.szExeFile;
        if (processName == targetProcessName) {
            DWORD processID = pe32.th32ProcessID;
            CloseHandle(hProcessSnap);
            return processID;
        }
    } while (Process32NextW(hProcessSnap, &pe32)); // Utiliser Process32NextW
    std::wcerr << L"[ERREUR] Processus non trouvé" << std::endl;
    CloseHandle(hProcessSnap);
    return 0;
}

DWORD_PTR getBaseAddress(HANDLE hProcess) {
    MODULEINFO modInfo;
    HMODULE hMods[1024];
    DWORD cbNeeded;
    // Obtenir la liste des modules du processus
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (size_t i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            // Récupérer les informations sur le module
            if (GetModuleInformation(hProcess, hMods[i], &modInfo, sizeof(modInfo))) {
                // Retourner l'adresse de base en utilisant DWORD_PTR
                return reinterpret_cast<DWORD_PTR>(modInfo.lpBaseOfDll);
            }
        }
    } else {
        std::cerr << "[ERREUR] EnumProcessModules a échoué. Code d'erreur: " << GetLastError() << std::endl;
    }
    return 0;
}

bool RunCurrentAsAdministrator() {
    wchar_t path[MAX_PATH];
    if (!GetModuleFileNameW(NULL, path, MAX_PATH)) {
        std::wcerr << L"Erreur de récupération du chemin du module : " << GetLastError() << std::endl;
        return false;
    }
    std::wstring arguments = L"--admin";
    SHELLEXECUTEINFOW sei = {0};
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_FLAG_NO_UI;
    sei.hwnd = NULL;
    sei.lpVerb = L"runas";                // Lancer avec les droits administratifs
    sei.lpFile = path;                    // Chemin du programme en cours
    sei.lpParameters = arguments.c_str(); // Passer l'argument
    sei.lpDirectory = NULL;
    sei.nShow = SW_SHOWNORMAL;
    if (ShellExecuteExW(&sei)) {
        return true; // Succès
    } else {
        std::wcerr << L"Erreur lors du lancement en mode administrateur : " << GetLastError() << std::endl;
        return false; // Échec
    }
}

int main(int argc, char *argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--admin") {
        std::wcout << L"Programme exécuté en mode administrateur." << std::endl;
    } else {
        if (RunCurrentAsAdministrator()) {
            std::wcout << L"Programme relancé en mode administrateur avec succès." << std::endl;
            return 0; // Quitter le programme actuel pour éviter la boucle infinie
        } else {
            std::wcout << L"Échec du relancement en mode administrateur." << std::endl;
            return 1; // Échec
        }
    }
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
    emulatorText.setString("Is emulator running?");
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
        if (isEmulatorDetected(detectedEmulator))
            emulatorText.setFillColor(sf::Color::Green);
        else
            emulatorText.setFillColor(sf::Color::Black);
        if (isRomHackLoaded(detectedEmulator))
            b3313Text.setFillColor(sf::Color::Green);
        else
            b3313Text.setFillColor(sf::Color::Black);
        if (showStarDisplay) {
            if (isRomHackLoaded(detectedEmulator)) {
                DWORD processID = getProcessID(detectedEmulator);
                if (processID != 0) {
                    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
                    if (hProcess != NULL) {
                        DWORD baseAddress = getBaseAddress(hProcess);
                        if (baseAddress != 0) {
                            int yOffset = 0;
                            for (const auto &group : jsonData["groups"]) {
                                std::string groupName = group["name"];
                                std::vector<StarData> starList;
                                // Parcourir les cours (mondes) du groupe
                                for (const auto &course : group["courses"]) {
                                    std::string courseName = course["name"];
                                    int numStars = 5;           // Supposons qu'il y ait 5 étoiles par monde (peut être modifié)
                                    std::string starIcon = "☆"; // Utiliser des étoiles génériques pour le moment
                                    // Ajouter à la liste des étoiles pour ce monde
                                    starList.push_back({courseName, numStars, starIcon});
                                }
                                // Afficher les étoiles pour le groupe actuel
                                starDisplay.afficherEtoilesGroupe(groupName, starList, window, font, yOffset);
                            }
                        } else {
                            CloseHandle(hProcess);
                            std::cerr << "[ERREUR] Adresse de base non trouvée." << std::endl;
                        }
                    } else {
                        std::cerr << "[ERREUR] Impossible d'ouvrir le processus. Code d'erreur: " << GetLastError() << std::endl;
                    }
                } else {
                    std::cerr << "Processus non trouvé !" << std::endl;
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