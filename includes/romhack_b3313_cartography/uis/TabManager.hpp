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
        float xOffset = 100; // Position initiale des onglets
        float yOffset = 50;
        for (const auto &name : tabNames) {
            tabs.emplace_back(name, xOffset, yOffset);
            xOffset += 110; // Espacement entre les onglets
        }
    }
    void draw(sf::RenderWindow &window) const {
        for (size_t i = 0; i < tabs.size(); ++i) {
            tabs[i].draw(window, font, i == currentTabIndex);
        }
    }

    void handleMouseClick(const sf::Vector2i &mousePosition) {
        for (size_t i = 0; i < tabs.size(); ++i) {
            if (tabs[i].contains(mousePosition)) {
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
