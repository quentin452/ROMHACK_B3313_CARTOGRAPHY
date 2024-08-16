#include <SFML/Graphics.hpp>

#include <windows.h>

#include <psapi.h>

#include <tlhelp32.h>

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <romhack_b3313_cartography/Button.h>
#include <romhack_b3313_cartography/DropdownMenu.h>
#include <romhack_b3313_cartography/Node.h>
#include <romhack_b3313_cartography/StarDisplay.h>
#include <string>
#include <vector>
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

    // Vérifier si le fichier a été ouvert avec succès
    if (!file.is_open()) {
        return {}; // Retourner un vecteur vide si le fichier n'existe pas
    }

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
bool isEmulatorDetected() {
    std::vector<std::string> emulators = {"project64", "wine-preloader", "mupen64", "retroarch"};
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return false;
    }
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return false;
    }
    do {
        std::string processName = pe32.szExeFile;
        for (const auto &emulator : emulators) {
            if (processName.find(emulator) != std::string::npos) {
                CloseHandle(hProcessSnap);
                return true;
            }
        }
    } while (Process32Next(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    return false;
}
bool readMemory(HANDLE hProcess, DWORD address, void *buffer, SIZE_T size) {
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQueryEx(hProcess, (LPCVOID)address, &mbi, sizeof(mbi))) {
        /* std::cout << "[DEBUG] VirtualQueryEx succeeded. Address: 0x" << std::hex << address
                   << ", State: " << mbi.State << ", Protect: " << mbi.Protect
                   << ", BaseAddress: 0x" << mbi.BaseAddress
                   << ", RegionSize: " << mbi.RegionSize << std::endl;

         // Affichage de la protection mémoire
         std::cout << "[DEBUG] Memory Protection Flags: " << std::hex << mbi.Protect << std::endl;
         std::cout << "[DEBUG] Memory State Flags: " << std::hex << mbi.State << std::endl;*/

        if (address >= (DWORD_PTR)mbi.BaseAddress && address < (DWORD_PTR)mbi.BaseAddress + mbi.RegionSize) {
            // if (mbi.State == MEM_COMMIT && (mbi.Protect & PAGE_READWRITE)) {
            SIZE_T bytesRead;
            if (ReadProcessMemory(hProcess, (LPCVOID)address, buffer, size, &bytesRead)) {
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
            //  } else {
            //      std::cerr << "[ERREUR] Adresse mémoire non valide ou non accessible à l'adresse 0x"
            //               << std::hex << address << ". Protection: " << std::hex << mbi.Protect << std::endl;
            //  }
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

bool isRomHackLoaded() {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return false;
    }
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

DWORD getProcessID() {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return 0;
    }
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return 0;
    }
    do {
        std::string processName = pe32.szExeFile;
        if (processName.find("-") != std::string::npos) {
            DWORD processID = pe32.th32ProcessID;
            CloseHandle(hProcessSnap);
            return processID;
        }
    } while (Process32Next(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    return 0;
}
DWORD getBaseAddress(HANDLE hProcess) {
    MODULEINFO modInfo;
    HMODULE hMods[1024];
    DWORD cbNeeded;

    // Obtenir la liste des modules du processus
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (size_t i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            // Récupérer les informations sur le module
            if (GetModuleInformation(hProcess, hMods[i], &modInfo, sizeof(modInfo))) {
                // std::cout << "[DEBUG] Module: " << hMods[i] << " Base Address: " << std::hex << modInfo.lpBaseOfDll << std::endl;
                //  Ici, tu devras probablement vérifier le nom du module pour valider qu'il correspond à Super Mario 64 ou l'émulateur
                return (DWORD)modInfo.lpBaseOfDll;
            }
        }
    } else {
        std::cerr << "[ERREUR] EnumProcessModules a échoué. Code d'erreur: " << GetLastError() << std::endl;
    }
    return 0;
}

bool RunCurrentAsAdministrator() {
    // Récupérer le chemin complet du programme en cours
    wchar_t path[MAX_PATH];
    if (!GetModuleFileNameW(NULL, path, MAX_PATH)) {
        std::wcerr << L"Erreur de récupération du chemin du module : " << GetLastError() << std::endl;
        return false;
    }

    // Ajouter un argument pour indiquer que le programme a été relancé en mode administrateur
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
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Mind Map Example");

    // Charger textures
    sf::Texture saveTexture;
    if (!saveTexture.loadFromFile("resources/textures/save_icon.png"))
        return 1;
    sf::Texture starTexture;
    if (!starTexture.loadFromFile("resources/textures/star.png"))
        return 1;
    sf::Texture dropdownTexture;
    if (!dropdownTexture.loadFromFile("resources/textures/dropdown_background.png"))
        return 1;

    // Créer les objets
    Button saveButton(1150, 10, saveTexture);      // Positionnez le bouton comme vous le souhaitez
    Button switchViewButton(550, 90, starTexture); // Positionnez le bouton comme vous le souhaitez
    std::vector<std::string> menuItems = {"b3313-v1.0.2.json", "another_file.json"};

    sf::Font font;
    if (!font.loadFromFile("resources/assets/Arial.ttf"))
        return 1;

    DropdownMenu dropdownMenu(1150, 50, dropdownTexture, menuItems, font); // Positionnez le menu comme vous le souhaitez

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

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (saveButton.isClicked(sf::Vector2i(event.mouseButton.x, event.mouseButton.y))) {
                        saveNodes(nodes, "b3313-v1.0.2.json");
                    } else if (dropdownMenu.isClicked(sf::Vector2i(event.mouseButton.x, event.mouseButton.y))) {
                        std::string selectedFile = dropdownMenu.getSelectedItem(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                        if (!selectedFile.empty()) {
                            nodes = loadNodes(selectedFile, font);
                        }
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

        if (showStarDisplay) {
            // Lecture des étoiles et mise à jour
            if (isEmulatorDetected() && isRomHackLoaded()) {
                DWORD processID = getProcessID();
                if (processID != 0) {
                    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
                    if (hProcess != NULL) {
                        DWORD baseAddress = getBaseAddress(hProcess);
                        if (baseAddress != 0) {
                            std::vector<bool> stars(460, false);
                            for (int i = 0; i < 460; ++i) {
                                BYTE star;
                                if (readMemory(hProcess, baseAddress + STAR_OFFSET + i, &star, sizeof(star))) {
                                    stars[i] = (star != 0);
                                }
                            }
                            starDisplay.updateStars(stars);
                            CloseHandle(hProcess);
                        } else {
                            std::cerr << "[ERREUR] Adresse de base non trouvée." << std::endl;
                        }
                    } else {
                        std::cerr << "[ERREUR] Impossible d'ouvrir le processus. Code d'erreur: " << GetLastError() << std::endl;
                    }
                } else {
                    std::cerr << "Processus non trouvé !" << std::endl;
                }
            }
            starDisplay.draw(window);
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