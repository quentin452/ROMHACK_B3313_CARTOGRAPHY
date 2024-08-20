/*
VERSION SFML
#include <windows.h>

#include <psapi.h>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <nlohmann/json.hpp>
<<<<<<< Updated upstream
#include <romhack_b3313_cartography/Textures.h>
=======
>>>>>>> Stashed changes

#include <romhack_b3313_cartography/Button.h>
#include <romhack_b3313_cartography/DropdownMenu.h>
#include <romhack_b3313_cartography/Node.h>
#include <romhack_b3313_cartography/StarDisplay.h>
#include <string>
#include <tlhelp32.h>
#include <vector>
#include <windows.h>
<<<<<<< Updated upstream
const DWORD STAR_OFFSET = 0x33B8; // Offset pour les étoiles (à adapter selon la version du jeu)

using json = nlohmann::json;
=======

#include <QApplication>
#include <QComboBox>
#include <QFile>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

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
>>>>>>> Stashed changes

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

<<<<<<< Updated upstream
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

=======
>>>>>>> Stashed changes
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
}*/
#include <windows.h>

#include <psapi.h>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <nlohmann/json.hpp>
#include <romhack_b3313_cartography/utils/rom_utils.h>

#include <romhack_b3313_cartography/uis/Textures.h>

#include "uis/TabManager.hpp"
#include <romhack_b3313_cartography/uis/Node.h>
#include <romhack_b3313_cartography/uis/StarDisplay.h>

#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QFile>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPair>
#include <QPushButton>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QVector>

// Define constants
const int WIDTH = 1280;
const int HEIGHT = 720;
std::wstring global_detected_emulator;
class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow() {
        setWindowTitle("Mind Map Example");
        setFixedSize(WIDTH, HEIGHT);
        emulatorText = new QGraphicsTextItem("Emulator Status");
        b3313Text = new QGraphicsTextItem("B3313 V1.0.2 Status");
        tabManager = new TabManager(tabNames, this);
        // Setup graphics view and scene
        graphicsView = new QGraphicsView(this);
        graphicsScene = new QGraphicsScene(this);
        graphicsView->setScene(graphicsScene);
        setCentralWidget(graphicsView);

        // Setup UI elements
        QVBoxLayout *layout = new QVBoxLayout;
        QWidget *container = new QWidget;
        container->setLayout(layout);
        layout->addWidget(graphicsView);

        // Load data
        loadJsonData("resources/stars_layout/b3313-V1.0.2/layout.json");

        // Setup buttons and dropdown
        saveButton = new QPushButton("Save", this);
        switchViewButton = new QPushButton("Switch View", this);
        layout->addWidget(saveButton);
        layout->addWidget(switchViewButton);
#ifdef DEBUG
        connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveNodes);
#endif
        connect(switchViewButton, &QPushButton::clicked, this, &MainWindow::toggleStarDisplay);

        // Setup dropdown menu
        dropdownMenu = new QComboBox(this);
        dropdownMenu->addItems({"b3313-v1.0.2.json", "another_file.json"});
        layout->addWidget(dropdownMenu);

        connect(dropdownMenu, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
            QString selectedFile = dropdownMenu->itemText(index);
            if (!selectedFile.isEmpty()) {
                QFont font = this->font();
                mind_map_nodes = loadNodes("b3313-v1.0.2.json", font);
            }
        });

        // Setup timer
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &MainWindow::updateStatus);
        timer->start(1000); // Update every second

        // Add mouse events
        setMouseTracking(true);
        graphicsView->setMouseTracking(true);
    }

  protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton && dragging) {
            QPointF mousePos = graphicsView->mapToScene(event->pos());

            // Handle tab click
            if (tabManager->contains(mousePos)) {
                tabManager->handleMouseClick(mousePos);
            }

            // Handle node dragging
            // if (isMouseOverNode(nodes, event->pos(), startNodeIndex)) {
            //    startPos = mousePos;
            //    dragging = true;
            //}
        }

#ifdef DEBUG
        if (event->button() == Qt::RightButton) {
            QPointF worldPos = graphicsView->mapToScene(event->pos());

            // Create a new Node instance
            Node *newNode = new Node(worldPos.x(), worldPos.y(), "New Node", font());

            // Add the new Node to the QList
            nodes.append(newNode);
        }
#endif
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton && dragging) {
            QPointF mousePos = graphicsView->mapToScene(event->pos());
            int endNodeIndex = -1;
            if (isMouseOverNode(mousePos, endNodeIndex) && endNodeIndex != startNodeIndex) {
                connections.push_back(QPair<int, int>(startNodeIndex, endNodeIndex));
                nodes[startNodeIndex]->connections.push_back(endNodeIndex);
                nodes[endNodeIndex]->connections.push_back(startNodeIndex);
            }
            dragging = false;
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        QPointF mousePos = graphicsView->mapToScene(event->pos());

        // Réinitialiser la couleur de tous les nœuds
        for (Node *node : nodes) {
            node->setColor(Qt::white); // Assurez-vous que `Node` a une méthode `setColor`
        }

        // Trouver le nœud sous la souris et changer sa couleur
        int nodeIndex = -1;
        if (isMouseOverNode(mousePos, nodeIndex)) {
            Node *hoveredNode = nodes[nodeIndex];
            hoveredNode->setColor(Qt::yellow); // Changer la couleur du nœud survolé
        }
        // if (event.type == sf::Event::MouseMoved && dragging) {
        //  Handle dragging logic if needed
        //}
        // Mettre à jour l'affichage des coordonnées de la souris ou d'autres informations
        // Par exemple, vous pouvez afficher les coordonnées dans un label ou un autre widget
        // Exemple :
        // statusLabel->setText(QString("Mouse Position: (%1, %2)").arg(mousePos.x()).arg(mousePos.y()));
    }

  private slots:
    void saveNodes() {
        QJsonArray jsonArray;
        for (const auto &nodePtr : mind_map_nodes) {
            jsonArray.append(nodePtr->toJson());
        }

        QJsonDocument jsonDoc(jsonArray);
        QFile file("b3313-v1.0.2.json");
        if (file.open(QIODevice::WriteOnly)) {
            file.write(jsonDoc.toJson(QJsonDocument::Indented)); // Pretty print JSON
            file.close();
        }
    }

    void toggleStarDisplay() {
        showStarDisplay = !showStarDisplay;
        updateDisplay(lastJsonData);
    }
    void updateStatus() {
        static QElapsedTimer elapsedTimer;
        static bool timerStarted = false;

        if (!timerStarted) {
            elapsedTimer.start();
            timerStarted = true;
        }

        qint64 elapsedMilliseconds = elapsedTimer.elapsed();
        if (elapsedMilliseconds < 1000) {
            return; // Exit early if not enough time has passed
        }

        // Update timer
        elapsedTimer.restart();

        // Check emulator status and update text color
        bool emulatorRunning = isEmulatorDetected(parallelLauncher, global_detected_emulator);
        bool romLoaded = isRomHackLoaded(global_detected_emulator);

        emulatorText->setPlainText(emulatorRunning ? "Emulator Running" : "Emulator Not Running");
        emulatorText->setDefaultTextColor(emulatorRunning ? Qt::green : Qt::black);

        b3313Text->setPlainText(romLoaded ? "B3313 V1.0.2 ROM Loaded" : "B3313 V1.0.2 ROM Not Loaded");
        b3313Text->setDefaultTextColor(romLoaded ? Qt::green : Qt::black);
    }

  private:
    void loadJsonData(const QString &filename) {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Failed to open file:" << filename;
            return;
        }
        QByteArray data = file.readAll();
        QJsonDocument doc(QJsonDocument::fromJson(data));
        QJsonObject jsonData = doc.object();
        lastJsonData = jsonData; // Store the loaded JSON data
        parseJsonData(jsonData);
    }

    void parseJsonData(const QJsonObject &jsonData) {
        // Clear existing nodes and connections
        graphicsScene->clear();
        nodes.clear();
        connections.clear();

        // Suppose you have a QFont object available for node creation
        QFont defaultFont("Arial", 12, QFont::Bold);
        // Parse nodes
        QJsonArray nodeArray = jsonData["nodes"].toArray();
        for (const QJsonValue &value : nodeArray) {
            QJsonObject nodeObj = value.toObject();
            qreal x = nodeObj["x"].toDouble();
            qreal y = nodeObj["y"].toDouble();
            QString label = nodeObj["label"].toString();
            Node *node = new Node(x, y, label, defaultFont); // Ensure you provide the QFont
            graphicsScene->addItem(node);
            nodes.append(node);
        }

        // Parse connections
        QJsonArray connectionArray = jsonData["connections"].toArray();
        for (const QJsonValue &value : connectionArray) {
            QJsonObject connObj = value.toObject();
            int startIndex = connObj["start"].toInt();
            int endIndex = connObj["end"].toInt();
            if (startIndex >= 0 && startIndex < nodes.size() && endIndex >= 0 && endIndex < nodes.size()) {
                connections.push_back(QPair<int, int>(startIndex, endIndex));
            }
        }

        updateDisplay(jsonData);
    }
    void updateDisplay(const QJsonObject &jsonData) {
        graphicsScene->clear();

        // Draw connections
        for (const QPair<int, int> &conn : connections) {
            Node *startNode = nodes[conn.first];
            Node *endNode = nodes[conn.second];
            QGraphicsLineItem *lineItem = new QGraphicsLineItem(startNode->x(), startNode->y(), endNode->x(), endNode->y());
            lineItem->setPen(QPen(Qt::black));
            graphicsScene->addItem(lineItem);
        }

        // Draw nodes
        for (Node *node : nodes) {
            graphicsScene->addItem(node);
        }

        if (showStarDisplay) {
            displayStars(jsonData);
        }
    }

    void displayStars(const QJsonObject &jsonData) {
        if (isRomHackLoaded(global_detected_emulator)) {
            std::string saveLocation = GetParallelLauncherSaveLocation();

            if (!jsonData.contains("format") || !jsonData["format"].toObject().contains("save_type") ||
                !jsonData["format"].toObject().contains("slots_start") || !jsonData["format"].toObject().contains("slot_size") ||
                !jsonData["format"].toObject().contains("active_bit") || !jsonData["format"].toObject().contains("checksum_offset")) {
                std::cerr << "Erreur: Les paramètres de sauvegarde sont manquants dans le JSON." << std::endl;
                return;
            }

            QJsonObject format = jsonData["format"].toObject();
            SaveParams params;
            params.saveFormat = parseSaveFormat(format["save_type"].toString().toStdString());
            params.slotsStart = format["slots_start"].toInt();
            params.slotSize = format["slot_size"].toInt();
            params.activeBit = format["active_bit"].toInt();
            params.numSlots = format["num_slots"].toInt();
            params.checksumOffset = format["checksum_offset"].toInt();

            auto saveData = ReadSrmFile(saveLocation, params);

            if (saveData.empty()) {
                std::cerr << "Erreur: Les données de sauvegarde sont vides." << std::endl;
                return;
            }

            int yOffset = 0;
            int numSlots = params.numSlots;
            if (numSlots <= 0) {
                std::cerr << "Erreur: Nombre de slots invalide." << std::endl;
                return;
            }

            tabNames.clear();
            for (int i = 1; i <= numSlots; ++i) {
                tabNames.append("Mario " + QString::number(i));
            }

            tabManager->initializeTabs(tabNames);
            QString currentTabName = tabManager->getCurrentTabName();
            if (currentTabName.isEmpty()) {
                std::cerr << "Erreur: Nom de l'onglet actuel est vide." << std::endl;
                return;
            }

            // Clear the scene
            graphicsScene->clear();
            float reservedHeight = tabManager->getTabsHeight();
            QRectF windowRect = graphicsView->rect();

            // Create a QPainter object
            QPainter painter;

            for (int i = 0; i < numSlots; ++i) {
                if (i >= tabNames.size()) {
                    std::cerr << "Erreur: Index de tabName hors limites." << std::endl;
                    continue;
                }

                QString tabName = tabNames[i];
                if (tabName == currentTabName) {
                    QGraphicsTextItem *tabText = new QGraphicsTextItem(tabName);
                    QFont font = this->font();
                    tabText->setFont(font);
                    tabText->setDefaultTextColor(Qt::black);
                    tabText->setPos(100, 100 + yOffset);
                    graphicsScene->addItem(tabText);

                    yOffset += 30;

                    for (const auto &groupValue : jsonData["groups"].toArray()) {
                        QJsonObject group = groupValue.toObject();
                        if (!group.contains("name") || !group.contains("courses")) {
                            std::cerr << "Erreur: Le groupe dans JSON est mal formé." << std::endl;
                            continue;
                        }

                        QString groupName = group["name"].toString();
                        QMap<QString, QVector<StarData>> courseStarsMap;
                        yOffset += 30;

                        for (const auto &courseValue : group["courses"].toArray()) {
                            QJsonObject course = courseValue.toObject();
                            if (!course.contains("name") || !course.contains("data")) {
                                std::cerr << "Erreur: Le cours dans JSON est mal formé." << std::endl;
                                continue;
                            }

                            QString courseName = course["name"].toString();
                            QVector<StarData> &courseStarList = courseStarsMap[courseName];

                            for (const auto &dataValue : course["data"].toArray()) {
                                QJsonObject data = dataValue.toObject();
                                int offset = data["offset"].toInt();
                                int mask = data["mask"].toInt();
                                int numStars = 1;
                                for (int bit = 0; bit < 32; ++bit) {
                                    if (mask & (1 << bit)) {
                                        bool star_collected = isStarCollected(saveData, offset, bit, i, params.slotSize);
                                        courseStarList.append({courseName, numStars, star_collected, offset, mask});
                                    }
                                }
                            }
                        }

                        // Create a QPixmap for rendering
                        QPixmap pixmap(graphicsView->size());
                        pixmap.fill(Qt::white); // Set a background color if needed
                        painter.begin(&pixmap);

                        // Call the updated afficherEtoilesGroupeFusionne function with QPainter
                        starDisplay.afficherEtoilesGroupeFusionne(groupName, courseStarsMap, painter, font, yOffset, reservedHeight, windowRect);

                        painter.end();

                        // Add the pixmap to the scene
                        graphicsScene->addPixmap(pixmap);
                    }

                    graphicsView->setScene(graphicsScene);
                    break;
                }
            }
        } else {
            graphicsScene->addItem(emulatorText);
            graphicsScene->addItem(b3313Text);
        }
    }

    bool isMouseOverNode(const QPointF &mousePos, int &nodeIndex) {
        for (int i = 0; i < nodes.size(); ++i) {
            Node *node = nodes[i];
            if (node->contains(mousePos)) {
                nodeIndex = i;
                return true;
            }
        }
        return false;
    }
    QVector<Node *> loadNodes(const QString &filename, QFont &font) {
        QVector<Node *> nodes;
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)) {
            return nodes; // Return empty vector if file can't be opened
        }

        QByteArray fileData = file.readAll();
        QJsonDocument jsonDoc(QJsonDocument::fromJson(fileData));
        QJsonArray jsonArray = jsonDoc.array();

        for (const QJsonValue &value : jsonArray) {
            if (value.isObject()) {
                QJsonObject jsonObject = value.toObject();
                Node *node = new Node(Node::fromJson(jsonObject, font)); // Create a new Node object
                nodes.push_back(node);                                   // Add pointer to vector
            }
        }

        return nodes;
    }
    QGraphicsView *graphicsView;
    QGraphicsScene *graphicsScene;
    QVector<Node *> nodes;
    QVector<QPair<int, int>> connections;
    QPointF startPos;
    bool dragging = false;
    int startNodeIndex = -1;
    TabManager *tabManager;
    QStringList tabNames;
    QJsonObject lastJsonData;
    bool showStarDisplay = false; // Contrôle pour afficher l'affichage des étoiles
    StarDisplay starDisplay;      // Instance de la classe StarDisplay
    QList<Node *> mind_map_nodes;
    QGraphicsTextItem *emulatorText = nullptr;
    QGraphicsTextItem *b3313Text = nullptr;
    QComboBox *dropdownMenu = nullptr;
    QPushButton *saveButton = nullptr;
    QPushButton *switchViewButton = nullptr;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Textures textures;

    MainWindow window;
    window.show();

    return app.exec();
}

#include "main.moc"
