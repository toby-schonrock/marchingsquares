#pragma once

#include "SFML/Graphics.hpp"
#include "Vector2.hpp"
#include <cmath>

template <typename To, typename From>
Vector2<To> unvisualize(const sf::Vector2<From>& v);

class PerlinNoise1D {
  private:
    sf::Image randNoise;

  public:
    PerlinNoise1D() {
        if (!randNoise.loadFromFile("noise.png"))
            throw std::logic_error("failed to load noise.png texture");
    }

    static float fade(float t) { return t * t * t * (t * (t * 6.0F - 15.0F) + 10.0F); }

    // Wow! Who the fuck knows if this works? (it doesn't TODO FIX)
    // Jesus Fucking Christ.
    // ig it doesn't matter if it looks random.
    // Making it sure as hell was!
    float grad(float p) const {
        float pfloor = floorf(p);
        int   i      = static_cast<int>(pfloor);
        // -1 to keep it in range idk (loses 1 match i think)
        i        = i % (static_cast<int>(randNoise.getSize().x) - 1);
        float r  = static_cast<float>(randNoise.getPixel(static_cast<unsigned>(i), 0).r);
        float r2 = static_cast<float>(randNoise.getPixel(static_cast<unsigned>(i + 1), 0).r);
        float rf = r + (p - pfloor) * (r2 - r);
        return (rf > 70) ? -1 : 1;
    }

    // idek what this does... im so confused
    // https://gpfault.net/posts/perlin-noise.txt.html
    float noise(float p) const {
        float p0 = floorf(p);
        float p1 = p0 + 1.0F;

        float t      = p - p0;
        float fade_t = fade(t);

        float g0 = grad(p0);
        float g1 = grad(p1);

        return (1.0F - fade_t) * g0 * (p - p0) + fade_t * g1 * (p - p1);
    }

    float noiseFunc(float p, float freq, float amp) const { return noise(p * freq) * amp; }

    float fractcalNoise(float p, float freq, float amp, std::size_t count = 10,
                        float freqMult = 2.0F, float ampMult = 0.5F) const {
        float n = 0.0F;
        for (std::size_t i = 0; i != count; ++i) {
            n += noise(p * freq) * amp;
            amp *= ampMult;
            freq *= freqMult;
        }
        return n;
    }
};

class PerlinNoise2D {
  public:
    sf::Image         randNoise; // make private again
    Vector2<unsigned> noiseSize;
    Vec2F             avgColor; // (rg)

    PerlinNoise2D() {
        if (!randNoise.loadFromFile("noise.png"))
            throw std::logic_error("failed to load noise.png texture");
        noiseSize = unvisualize<unsigned>(randNoise.getSize());
        std::cout << "succesfully loaded noise at resolution : " << noiseSize << "\n";
        sf::Color tempColor;
        for (unsigned y = 0; y != noiseSize.y; ++y) {
            for (unsigned x = 0; x != noiseSize.x; ++x) {
                tempColor = randNoise.getPixel(x, y);
                avgColor.x += tempColor.r;
                avgColor.y += tempColor.g;
            }
        }
        avgColor /= static_cast<float>(noiseSize.y * noiseSize.x);
        std::cout << "average colors (rg) : " << avgColor << "\n";
    }

    static float fade(float t) { return t * t * t * (t * (t * 6.0F - 15.0F) + 10.0F); }

    Vec2F grad(Vec2F p) const {
        Vec2F value = NearestNeighbourAccess(p) - avgColor;
        return value.norm();
    }

    // i think this works (see interp test file)
    // implements nearest neighbour filtering and wrapping
    Vec2F NearestNeighbourAccess(Vec2F p) const {
        Vec2F pfloor(floorf(p.x), floorf(p.y));
        Vec2I i(static_cast<int>(pfloor.x), static_cast<int>(pfloor.y));
        // -1 to keep it in range idk (loses 1 match i think)
        i = Vec2I(i.x % (static_cast<int>(noiseSize.x) - 1),
                  i.y % (static_cast<int>(noiseSize.y) - 1));
        i = Vec2I((std::signbit(i.x) ? static_cast<int>(noiseSize.x) + i.x - 1 : i.x),
                  (std::signbit(i.y) ? static_cast<int>(noiseSize.y) + i.y - 1 : i.y)); // wrapping
        Vector2<unsigned> ui(static_cast<unsigned>(i.x),
                             static_cast<unsigned>(i.y)); // unsinged for pixel access
        sf::Color         tempColor = randNoise.getPixel(ui.x, ui.y);
        Vec2F             r1        = Vec2F(static_cast<float>(tempColor.r),
                                            static_cast<float>(tempColor.g)); // pixel access top left
        tempColor                   = randNoise.getPixel(ui.x + 1, ui.y);
        Vec2F r2                    = Vec2F(static_cast<float>(tempColor.r),
                                            static_cast<float>(tempColor.g)); // pixel access top right
        tempColor                   = randNoise.getPixel(ui.x + 1, ui.y + 1);
        Vec2F r3                    = Vec2F(static_cast<float>(tempColor.r),
                                            static_cast<float>(tempColor.g)); // pixel access bottom right
        tempColor                   = randNoise.getPixel(ui.x, ui.y + 1);
        Vec2F r4                    = Vec2F(static_cast<float>(tempColor.r),
                                            static_cast<float>(tempColor.g)); // pixel access bottom left
        Vec2F interPixelPos(p - pfloor);
        Vec2F topInterp = r1 + interPixelPos.x * (r2 - r1); // top right -> top left interp (rg)
        Vec2F botInterp =
            r4 + interPixelPos.x * (r3 - r4); // bottom right -> bottom left interp (rg)
        Vec2F midPointInterp = topInterp + interPixelPos.y * (botInterp - topInterp);
        return midPointInterp;
    }

    // idk what this does
    // https://gpfault.net/posts/perlin-noise.txt.html
    float noise(Vec2F p) const {
        Vec2F p0(floorf(p.x), floorf(p.y));
        Vec2F p1 = p0 + Vec2F(1.0, 0.0);
        Vec2F p2 = p0 + Vec2F(0.0, 1.0);
        Vec2F p3 = p0 + Vec2F(1.0, 1.0);

        /* Look up gradients at lattice points. */
        Vec2F g0 = grad(p0);
        Vec2F g1 = grad(p1);
        Vec2F g2 = grad(p2);
        Vec2F g3 = grad(p3);

        float t0      = p.x - p0.x;
        float fade_t0 = fade(t0); /* Used for interpolation in horizontal direction */

        float t1      = p.y - p0.y;
        float fade_t1 = fade(t1); /* Used for interpolation in vertical direction. */

        /* Calculate dot products and interpolate.*/
        float p0p1 = (1.0F - fade_t0) * g0.dot(p - p0) +
                     fade_t0 * g1.dot(p - p1); /* between upper two lattice points */
        float p2p3 = (1.0F - fade_t0) * g2.dot(p - p2) +
                     fade_t0 * g3.dot(p - p3); /* between lower two lattice points */

        /* Calculate final result */
        return (1.0F - fade_t1) * p0p1 + fade_t1 * p2p3;
    }

    float noiseFunc(Vec2F p, float freq, float amp) const { return noise(p * freq) * amp; }

    float fractcalNoise(Vec2F p, float freq, float amp, std::size_t count = 10,
                        float freqMult = 2.0F, float ampMult = 0.5F) const {
        float n = 0.0F;
        for (std::size_t i = 0; i != count; ++i) {
            n += noise(p * freq) * amp;
            amp *= ampMult;
            freq *= freqMult;
        }
        return n;
    }
};