#include <chrono>
#include <iostream>

#include "GUI.hpp"
#include "MarchingArea.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"
#include "Vector2.hpp"
#include "imgui-SFML.h"
#include "imgui.h"
#include "implot.h"

template <typename To, typename From>
sf::Vector2<To> visualize(const Vector2<From>& v) {
    return sf::Vector2<To>(static_cast<To>(v.x), static_cast<To>(v.y));
}

template <typename To, typename From>
Vector2<To> unvisualize(const sf::Vector2<From>& v) {
    return Vector2<To>(static_cast<To>(v.x), static_cast<To>(v.y));
}

int main() {
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Window", sf::Style::Fullscreen,
                            settings); //, sf::Style::Default);

    GUI gui(desktop, window);

    ImGui::SFML::Init(window);
    ImPlot::CreateContext();
    ImGuiIO& imguIO = ImGui::GetIO();
    imguIO.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange; // disable cursor overide

    MarchingArea area({}, {20.0F, 20.0F}, {1000, 2}, 0.01F);

    sf::Clock
        deltaClock; // for imgui - read https://eliasdaler.github.io/using-imgui-with-sfml-pt1/
    while (window.isOpen()) {
        sf::Vector2i mousePixPos = sf::Mouse::getPosition(
            window); // mouse position is only accurate to end of simulation frames (it does change)

        // poll events for sfml and imgui
        sf::Event event; // NOLINT
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (!(imguIO.WantCaptureMouse && event.type == sf::Event::MouseButtonPressed)) {
                gui.event(event, mousePixPos);
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart()); // required for imgui-sfml

        area.updateNoise();

        // draw
        window.clear();

        area.draw(window);
        gui.frame(mousePixPos);

        ImGui::SFML::Render(window);
        window.display();
    }

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();

    return 0;
}