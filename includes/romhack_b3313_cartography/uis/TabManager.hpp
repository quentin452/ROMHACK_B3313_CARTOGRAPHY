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

    void draw(sf::RenderWindow &window) const {
        for (size_t i = 0; i < tabs.size(); ++i) {
            tabs[i].draw(window, font, i == currentTabIndex);
        }
    }

    void handleMouseClick(const sf::Vector2i &mousePosition, const sf::View &view,sf::RenderWindow &window) {
        sf::Vector2f viewMousePosition = window.mapPixelToCoords(mousePosition, view);

        for (size_t i = 0; i < tabs.size(); ++i) {
            if (tabs[i].contains(viewMousePosition)) {
                currentTabIndex = i;
                break;
            }
        }
    }

    std::string getCurrentTabName() const {
        if (currentTabIndex < 0 || currentTabIndex >= tabs.size()) {
            throw std::out_of_range("currentTabIndex is out of range");
        }
        return tabs[currentTabIndex].getName();
    }

  private:
    std::vector<Tab> tabs;
    size_t currentTabIndex;
    const sf::Font &font;
};