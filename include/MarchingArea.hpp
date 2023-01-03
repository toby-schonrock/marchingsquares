#pragma once

#include "Matrix.hpp"
#include "PerlinNoise.hpp"
#include "SFML/Graphics.hpp"
#include "Vector2.hpp"
#include "imgui.h"

#include <numeric>

template <typename To, typename From>
Vector2<To> unvisualize(const sf::Vector2<From>& v);

template <typename To, typename From>
sf::Vector2<To> visualize(const Vector2<From>& v);

bool ImGui_DragUnsigned(const char* label, std::size_t* v, float v_speed = 1.0f,
                        std::uint32_t v_min = 0, std::uint32_t v_max = 0, const char* format = "%d",
                        ImGuiSliderFlags flags = 0) {
    return ImGui::DragScalar(label, ImGuiDataType_U32, v, v_speed, &v_min, &v_max, format, flags);
}

class MarchingArea {
  private:
    sf::Image               image{};
    sf::Texture             texture{};
    sf::Sprite              sprite{};
    std::vector<sf::Vertex> verts;
    PerlinNoise2D           noise{};
    Matrix<float>           points;
    Vec2F                   gap;
    Vec2F                   area;
    Vec2F                   pos; // top left position of bounding box
    float                   maxAmp;

  public:
    MarchingArea(Vec2F pos_, Vec2F area_, const Vector2<std::size_t> resolution)
        : points(resolution), gap(area_.x / static_cast<float>(points.size.x),
                                  area_.y / static_cast<float>(points.size.y)),
          area(area_), pos(pos_) {
        if (area.mag() == 0.0F) throw std::logic_error("marching area cannot have 0 magnitude");
        if (points.size.x < 2 || points.size.y < 2)
            throw std::logic_error("resolution must be greater than 1 in both axis");
        verts.reserve(points.size.x * points.size.y * 4);

        image.create(static_cast<unsigned>(points.size.x), static_cast<unsigned>(points.size.y));
        sprite.setScale({area.x / static_cast<float>(points.size.x),
                         area.y / static_cast<float>(points.size.y)});
        sprite.setPosition(visualize<float>(pos));
        // sprite.setTexture(texture);
    }

    void updateImage() {
        for (unsigned y = 0; y != points.size.y; ++y) {
            for (unsigned x = 0; x != points.size.x; ++x) {
                float norm  = (points(x, y) + 1.0F) / 2.0F;
                norm        = (norm < 0.5) ? 0 : 1;
                sf::Uint8 v = static_cast<std::uint8_t>(norm * 255.0F);
                image.setPixel(x, y, sf::Color{v, v, v});
            }
        }
    }

    Vec2F getPointPos(const Vector2<std::size_t>& i) const {
        return pos + Vec2F{static_cast<float>(i.x) * gap.x, static_cast<float>(i.y) * gap.y};
    }

    void march() {
        static const std::array<std::pair<Vector2<std::size_t>, Vector2<std::size_t>>, 4> sides{
            std::pair<Vector2<std::size_t>, Vector2<std::size_t>>{{0, 0}, {1, 0}},
            {{0, 0}, {0, 1}},
            {{1, 0}, {1, 1}},
            {{0, 1}, {1, 1}}};

        verts.clear();
        // march all squares, (x, y) is the top left of the square
        Vector2<std::size_t> i{};
        while (i.y != points.size.y - 1) {
            while (i.x != points.size.x - 1) {
                for (const auto& s: sides) {
                    if (std::signbit(points(i + s.first)) != std::signbit(points(i + s.second))) {
                        verts.emplace_back(visualize<float>(getPointPos(i + s.first) + gap / 2),
                                           sf::Color::Red);
                        verts.emplace_back(visualize<float>(getPointPos(i + s.second) + gap / 2),
                                           sf::Color::Red);
                    }
                }
                ++i.x;
            }
            i.x = 0;
            ++i.y;
        }
    }

    void updateNoise() {
        static Vec2F       offset;
        static float       freq     = 2.5F;
        static float       amp      = 2.0F;
        static float       freqMult = 2.5F;
        static float       ampMult  = 0.3F;
        static std::size_t layers   = 1;
        ImGui::Begin("Noise");
        ImGui::DragFloat2("Offset", &offset.x, 0.01F, -10000.0F, 10000.0F);
        ImGui::DragFloat("Freq", &freq, 0.01F, 0.0F, 10000.0F);
        ImGui::DragFloat("Amp", &amp, 0.01F, 0.0F, 10000.0F);
        ImGui_DragUnsigned("Layers", &layers, 1.0F, 1, 4);
        ImGui::DragFloat("FreqMult", &freqMult, 0.01F, 0.0F, 10000.0F);
        ImGui::DragFloat("AmpMult", &ampMult, 0.01F, 0.0F, 1.0F);
        ImGui::End();

        maxAmp = 0;
        for (std::size_t i = 0; i != layers; ++i) {
            maxAmp += powf(ampMult, static_cast<float>(i));
        }
        maxAmp *= amp;

        for (std::size_t y = 0; y != points.size.y; ++y) {
            for (std::size_t x = 0; x != points.size.x; ++x) {
                Vec2F input = Vec2F(static_cast<float>(x) / static_cast<float>(points.size.x),
                                    static_cast<float>(y) / static_cast<float>(points.size.y)) +
                              offset;
                points(x, y) =
                    noise.fractcalNoise(input, freq, amp, layers, freqMult, ampMult) / maxAmp;
            }
        }
        // std::cout << std::accumulate(points.v.begin(), points.v.end(), 0.0F) / points.v.size() <<
        // "\n";
    }

    void drawSprite(sf::RenderWindow& window) {
        texture.loadFromImage(image);
        sprite.setTexture(texture); // TODO maybe dont need
        window.draw(sprite);
        window.draw(verts.data(), verts.size(), sf::Lines);
    }
};

struct triangle {
    std::uint8_t s1 : 2, s2 : 2, s3 : 2, : 2;
};