#pragma once

#include "PerlinNoise.hpp"
#include "SFML/Graphics.hpp"
#include "Vector2.hpp"
#include "imgui.h"

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
    sf::Image            image{};
    sf::Texture          texture{};
    sf::Sprite           sprite{};
    PerlinNoise2D        noise{};
    std::vector<float>   points;
    Vector2<std::size_t> resolution;
    Vec2F                gap;
    Vec2F                area;
    Vec2F                pos; // top left position of bounding box
    float                maxAmp;

  public:
    MarchingArea(Vec2F pos_, Vec2F area_, const Vector2<std::size_t>& resolution_)
        : resolution(resolution_), gap(area_.x / static_cast<float>(resolution.x - 1),
                                       area_.y / static_cast<float>(resolution.y - 1)),
          area(area_), pos(pos_) {
        if (area.mag() == 0.0F) throw std::logic_error("marching area cannot have 0 magnitude");
        if (resolution.x < 2 || resolution.y < 2)
            throw std::logic_error("resolution must be greater than 1 in both axis");
        points = std::vector<float>(resolution.x * resolution.y, 0.0F);
        image.create(static_cast<unsigned>(resolution.x), static_cast<unsigned>(resolution.y));
        sprite.setScale(
            {area.x / static_cast<float>(resolution.x), area.y / static_cast<float>(resolution.y)});
        sprite.setPosition(visualize<float>(pos));
        // sprite.setTexture(texture);
    }

    void updateImage() {
        for (unsigned y = 0; y != resolution.y; ++y) {
            for (unsigned x = 0; x != resolution.x; ++x) {
                sf::Uint8 v =
                    static_cast<std::uint8_t>(((points[x + y * resolution.x] + maxAmp) * 255.0F / (2.0F * maxAmp)));
                image.setPixel(x, y, sf::Color{v, v, v});
            }
        }
    }

    void updateNoise() {
        static float       freq     = 20.0F;
        static float       amp      = 2.0F;
        static float       freqMult = 2.5F;
        static float       ampMult  = 0.3F;
        static std::size_t layers   = 1;
        ImGui::Begin("Noise");
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

        for (std::size_t y = 0; y != resolution.y; ++y) {
            for (std::size_t x = 0; x != resolution.x; ++x) {
                Vec2F input(static_cast<float>(x) / static_cast<float>(resolution.x),
                            static_cast<float>(y) / static_cast<float>(resolution.y));
                points[x + y * resolution.x] =
                    noise.fractcalNoise(input, freq, amp, layers, freqMult, ampMult);
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        texture.loadFromImage(image);
        sprite.setTexture(texture); // TODO maybe dont need
        window.draw(sprite);
    }
};