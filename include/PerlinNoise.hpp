#pragma once

#include "SFML/Graphics.hpp"
#include <cmath>

class PerlinNoise1D {
  private:
    sf::Image randNoise;

  public:
    PerlinNoise1D() {
        if (!randNoise.loadFromFile("noise.png"))
            throw std::logic_error("failed to load noise.png texture");
    }

    static float fade(float t) { return t * t * t * (t * (t * 6.0F - 15.0F) + 10.0F); }

    // Wow! Who the fuck knows if this works?
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