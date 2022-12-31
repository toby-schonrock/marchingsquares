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

    PerlinNoise2D noise;

    constexpr float radius = 0.1F;

    std::vector<sf::CircleShape> circles;
    circles.emplace_back(radius);
    circles.emplace_back(radius);
    circles.emplace_back(radius);
    circles.emplace_back(radius);

    circles[0].setPosition(0, 0); // top left
    circles[1].setPosition(1, 0); // top right
    circles[2].setPosition(1, 1); // bottom right
    circles[3].setPosition(0, 1); // bottom left

    sf::Color temp;
    for (sf::CircleShape& c: circles) {
        c.setOrigin(radius, radius);
        temp = noise.randNoise.getPixel(static_cast<unsigned>(c.getPosition().x),
                                        static_cast<unsigned>(c.getPosition().y));
        temp = sf::Color(temp.r, temp.g, 0, 255);
        c.setFillColor(temp);
    }

    // test point
    sf::CircleShape test(radius);
    test.setOrigin(radius, radius);

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

        test.setPosition(window.mapPixelToCoords(mousePixPos));
        Vec2F temp2 = noise.NearestNeighbourAccess(unvisualize<float>(test.getPosition()));
        ImGui::SetTooltip("(%f, %f)", temp2.x, temp2.y);
        test.setFillColor(
            {static_cast<std::uint8_t>(temp2.x), static_cast<std::uint8_t>(temp2.y), 0, 255});
        // draw
        window.clear();

        for (const sf::CircleShape& c: circles) {
            window.draw(c);
        }
        window.draw(test);
        gui.frame(mousePixPos);

        ImGui::SFML::Render(window);
        window.display();
    }

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();

    return 0;
}