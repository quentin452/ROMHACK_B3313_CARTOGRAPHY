#include "Tab.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
class TabManager {
  public:
    TabManager(const std::vector<std::string> &tabNames, const sf::Font &font)
        : currentTabIndex(0), font(font) {
        initializeTabs(tabNames);
    }

    void initializeTabs(const std::vector<std::string> &tabNames) {
        tabs.clear();
        float xOffset = 100;
        float yOffset = 50;
        for (const auto &name : tabNames) {
            tabs.emplace_back(name, xOffset, yOffset);
            xOffset += 110;
        }
    }

    void draw(sf::RenderWindow &window, const sf::View &tabsView) const {
        sf::View originalView = window.getView(); // Sauvegarder la vue actuelle
        window.setView(tabsView);                 // Définir la vue fixe pour les onglets

        for (size_t i = 0; i < tabs.size(); ++i) {
            tabs[i].draw(window, font, i == currentTabIndex);
        }

        window.setView(originalView); // Restaurer la vue originale
    }

    void handleMouseClick(const sf::Vector2i &mousePosition, const sf::View &tabsView, sf::RenderWindow &window) {
        // Convertir les coordonnées du clic en coordonnées de la vue des onglets
        sf::Vector2f viewMousePosition = window.mapPixelToCoords(mousePosition, tabsView);

        // Vérifier si un onglet a été cliqué en utilisant les coordonnées de la vue des onglets
        for (size_t i = 0; i < tabs.size(); ++i) {
            if (tabs[i].contains(viewMousePosition)) {
                currentTabIndex = i;
                break;
            }
        }
    }

    std::string getCurrentTabName() const {
        return tabs[currentTabIndex].getName();
    }

  private:
    std::vector<Tab> tabs;
    size_t currentTabIndex;
    const sf::Font &font;
};