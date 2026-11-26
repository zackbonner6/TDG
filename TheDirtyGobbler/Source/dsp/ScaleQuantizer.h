#pragma once
// ============================================================================
//  ScaleQuantizer  -  key/scale logic: snap to nearest allowed note and build
//  diatonic harmony notes (3rds, 5ths...) that stay in key.
// ============================================================================
#include <cmath>
#include <cstdint>

class ScaleQuantizer
{
public:
    enum Scale { Major = 0, Minor, HarmonicMinor, MajorPentatonic, MinorPentatonic, Chromatic, NumScales };

    static int pc (int midi) { return ((midi % 12) + 12) % 12; }

    /** key: 0=C..11=B, enabledMask: bit per pitch class (C=bit0) that the user left switched on */
    void set (int key, int scale, uint16_t enabledMask)
    {
        static const uint16_t rel[NumScales] = {
            0b101010110101, // major       0 2 4 5 7 9 11
            0b010110101101, // minor       0 2 3 5 7 8 10
            0b100110101101, // harm minor  0 2 3 5 7 8 11
            0b001010010101, // maj pent    0 2 4 7 9
            0b010010101001, // min pent    0 3 5 7 10
            0b111111111111  // chromatic
        };
        const int s = (scale >= 0 && scale < NumScales) ? scale : Chromatic;
        chromatic = (s == Chromatic);
        scaleMask = 0;
        for (int i = 0; i < 12; ++i)
            if ((rel[s] >> i) & 1)
                scaleMask |= (uint16_t) (1u << ((i + key) % 12));
        allowedMask = (uint16_t) (scaleMask & enabledMask);
        if (allowedMask == 0)
            allowedMask = scaleMask; // never leave the user with zero notes
    }

    bool inScale (int midi) const { return (scaleMask >> pc (midi)) & 1; }
    bool allowed (int midi) const { return (allowedMask >> pc (midi)) & 1; }

    int nearest (float midi) const
    {
        const int c = (int) std::lround (midi);
        int best = c;
        float bestDist = 1.0e9f;
        for (int o = -7; o <= 7; ++o)
        {
            const int note = c + o;
            if (! allowed (note))
                continue;
            const float d = std::abs ((float) note - midi);
            if (d < bestDist) { bestDist = d; best = note; }
        }
        return best;
    }

    /** choice: 0 Off, 1 3rd Up, 2 5th Up, 3 6th Up, 4 Octave Up, 5 3rd Down, 6 4th Down, 7 Octave Down */
    int harmonyNote (int target, int choice) const
    {
        static const int steps[8] = { 0, 2, 4, 5, 0, -2, -3, 0 };
        if (choice == 4) return target + 12;
        if (choice == 7) return target - 12;
        const int st = (choice >= 0 && choice < 8) ? steps[choice] : 0;
        if (st == 0) return target;

        if (chromatic)
        {
            switch (st)
            {
                case 2:  return target + 4;
                case 4:  return target + 7;
                case 5:  return target + 9;
                case -2: return target - 3;
                case -3: return target - 5;
                default: return target;
            }
        }

        int note = target, count = 0, safety = 0;
        const int dir = st > 0 ? 1 : -1;
        const int need = st > 0 ? st : -st;
        while (count < need && safety++ < 24)
        {
            note += dir;
            if (inScale (note))
                ++count;
        }
        return note;
    }

private:
    uint16_t scaleMask = 0xFFF, allowedMask = 0xFFF;
    bool chromatic = true;
};
