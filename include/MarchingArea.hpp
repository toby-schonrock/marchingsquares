#pragma once

#include "PerlinNoise.hpp"
#include "SFML/Graphics.hpp"
#include "Vector2.hpp"
#include "imgui.h"

sf::Vector2f visualize(const Vec2& v);

class MarchingArea {
  private:
    std::vector<sf::Vertex> pointVerts;
    std::vector<float>      points;
    sf::Texture             pointTexture;
    PerlinNoise1D           noise{};
    Vector2<std::size_t>    resolution;
    Vec2F                   gap;
    Vec2F                   area;
    Vec2F                   pos; // top left position of bounding box
    float                   radius;

  public:
    MarchingArea(Vec2F pos_, Vec2F area_, const Vector2<std::size_t>& resolution_, float radius_)
        : resolution(resolution_), gap(area_.x / static_cast<float>(resolution.x - 1),
                                       area_.y / static_cast<float>(resolution.y - 1)),
          area(area_), pos(pos_), radius(radius_) {
        if (area.mag() == 0.0F) throw std::logic_error("marching area cannot have 0 magnitude");
        if (resolution.x < 2 || resolution.y < 2)
            throw std::logic_error("resolution must be greater than 1 in both axis");
        if (radius <= 0) throw std::logic_error("radius must be > 0");
        if (!pointTexture.loadFromFile("point.png"))
            throw std::logic_error("failed to load point.png texture");
        pointTexture.setSmooth(true);
        pointVerts.reserve(resolution.x * resolution.y * 4);
        points.reserve(resolution.x * resolution.y);
        for (std::size_t y = 0; y != resolution.y; ++y) {
            for (std::size_t x = 0; x != resolution.x; ++x) {
                points.emplace_back(0.0F);
                sf::Vector2f pointPos(pos.x + static_cast<float>(x) * gap.x,
                                      pos.y + static_cast<float>(y) * gap.y);
                // sf::Uint8    red   = static_cast<std::uint8_t>((x * 255) / resolution.x);
                // sf::Uint8    green = static_cast<std::uint8_t>((y * 255) / resolution.y);
                // pointVerts.emplace_back(pointPos, sf::Color{red, green, 255}, sf::Vector2f{0,
                // 0}); pointVerts.emplace_back(pointPos + sf::Vector2f{radius, 0.0F},
                //                         sf::Color{red, green, 255}, sf::Vector2f{300, 0});
                // pointVerts.emplace_back(pointPos + sf::Vector2f{radius, radius},
                //                         sf::Color{red, green, 255}, sf::Vector2f{300, 300});
                // pointVerts.emplace_back(pointPos + sf::Vector2f{0.0F, radius},
                //                         sf::Color{red, green, 255}, sf::Vector2f{0, 300});
                pointVerts.emplace_back(pointPos + sf::Vector2f{-radius, -radius},
                                        sf::Vector2f{0, 0});
                pointVerts.emplace_back(pointPos + sf::Vector2f{radius, -radius},
                                        sf::Vector2f{300, 0});
                pointVerts.emplace_back(pointPos + sf::Vector2f{radius, radius},
                                        sf::Vector2f{300, 300});
                pointVerts.emplace_back(pointPos + sf::Vector2f{-radius, radius},
                                        sf::Vector2f{0, 300});
            }
        }
    }

    void updateColor(std::size_t i, const sf::Color& color) {
        i *= 4;
        pointVerts[i].color     = color;
        pointVerts[i + 1].color = color;
        pointVerts[i + 2].color = color;
        pointVerts[i + 3].color = color;
    }

    void updatePos(std::size_t i, sf::Vector2f p) {
        sf::Vector2f pointPos = p + sf::Vector2f(pos.x + static_cast<float>(i) * gap.x, pos.y);
        i *= 4;
        pointVerts[i].position     = pointPos + sf::Vector2f{-radius, -radius};
        pointVerts[i + 1].position = pointPos + sf::Vector2f{radius, -radius};
        pointVerts[i + 2].position = pointPos + sf::Vector2f{radius, radius};
        pointVerts[i + 3].position = pointPos + sf::Vector2f{-radius, radius};
    }

    void updateNoise() {
        static float freq = 5.0F;
        static float amp  = 5.0F;
        ImGui::Begin("Noise");
        ImGui::DragFloat("Frequencey", &freq, 0.01F, 0.0F, 10000.0F);
        ImGui::DragFloat("Amplitude", &amp, 0.01F, -10000.0F, 10000.0F);
        ImGui::End();

        for (std::size_t x = 0; x != resolution.x; ++x) {
            float n = (noise.noiseFunc(static_cast<float>(x) / static_cast<float>(resolution.x),
                                       freq, amp) +
                       1.0F) /
                      2.0F;
            updatePos(x, {0.0F, n});
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(pointVerts.data(), pointVerts.size(), sf::Quads, &pointTexture);
    }
};