# TheDirtyGobbler

A rap/trap vocal plugin (VST3, 64-bit Windows) for FL Studio: real-time auto-tune with formant-safe pitch shifting, plus a full vocal chain (doubler, harmonizer, saturation, de-esser, compressor, synced delay, reverb, EQ) and 18 style presets.

---

## 1. Get the .vst3 file

You need to compile it once. Pick **A** (easiest, nothing to install) or **B**.

### A) Build it free on GitHub (recommended)

1. Make a free account at github.com if you don't have one.
2. Click **+ → New repository**, name it `TheDirtyGobbler`, set it to **Private**, and click **Create repository**.
3. On the new repo page click **uploading an existing file**.
4. Unzip this project, open the `TheDirtyGobbler` folder, select **everything inside it** (including the `.github` folder), and drag it all into the browser. Click **Commit changes**.
   - If the `.github` folder doesn't upload, click **Add file → Create new file**, type the name `.github/workflows/build-windows.yml`, paste in the contents of that file from the zip, and commit.
5. Go to the **Actions** tab. A "Build Windows VST3" run starts on its own (if it doesn't, click it, then **Run workflow**). It takes about 5–10 minutes.
6. When it shows a green check, click the run. Under **Artifacts**, download **TheDirtyGobbler-VST3-Windows** (it's a zip).

### B) Build it on your own PC

1. Install **Visual Studio 2022 Community** and tick **"Desktop development with C++"** during setup (that includes CMake).
2. Install **Git for Windows** (git-scm.com). CMake uses it to download JUCE.
3. Open **"x64 Native Tools Command Prompt for VS 2022"** from the Start menu, then:
   ```
   cd path\to\TheDirtyGobbler
   cmake -B build -A x64
   cmake --build build --config Release --parallel
   ```
4. The plugin ends up in `build\TheDirtyGobbler_artefacts\Release\VST3\TheDirtyGobbler.vst3`.

---

## 2. Install it

1. Unzip the download. You'll get a folder called **`TheDirtyGobbler.vst3`**. On Windows a .vst3 is a whole folder, so copy the entire folder.
2. Paste it into:
   ```
   C:\Program Files\Common Files\VST3
   ```
   (Windows will ask for admin permission. Click Continue.)

## 3. Load it in FL Studio

1. FL Studio → **Options → Manage plugins**.
2. Make sure `C:\Program Files\Common Files\VST3` is in the plugin search paths (it is by default).
3. Click **Find installed plugins** (older versions: **Find plugins**) and wait for the scan to finish.
4. Find **TheDirtyGobbler** in the list and click the **star** next to it so it shows up in your effects menu.
5. In the **Mixer**, select your vocal track, click an empty **insert slot**, and choose TheDirtyGobbler.

The plugin reports its latency (about 23 ms) to FL Studio, so plugin delay compensation lines your vocal up with the beat. If you **monitor live while recording**, you'll hear that small delay. Most people are fine with it. If it bugs you, record with the plugin bypassed and turn it on afterward.

---

## 4. Using it

**Set the key first.** Pick the beat's **Key** and **Scale** at the top. Presets never change the key, scale, input type, or note toggles.

| Control | What it does |
|---|---|
| **Input Type** | Low Male / High Male / Female. Sets the pitch range the tracker listens to. Pick the closest one. |
| **TUNE** | Turns pitch correction on or off. Transpose, formant, and FX still work when it's off. |
| **Notes** | Bright = in the scale. Click a note to switch it off and the tuner will never snap to it. |
| **Retune Speed** | 0 ms = hard robotic snap. 20–60 ms = tight but natural. 100+ ms = light touch. |
| **Humanize** | Slows correction on held notes so long notes keep some life. |
| **Formant** | Moves the voice's tone/size without changing the note. − = bigger/darker, + = smaller/brighter. |
| **Mix** | Dry/wet for the whole plugin. |
| **Flex Tune** | Notes already close to pitch get corrected less, so your natural slides come through. |
| **Vibrato / Vib Rate** | Adds vibrato to held notes. |
| **Throat** | − = chestier, deeper throat. + = thinner. |
| **Transpose** | Shifts everything ±12 semitones (chipmunk / villain voice). |
| **Output** | Final volume. |

**Effects** (each box has its own on/off switch):
- **Doubler**: fake double-track, spread left/right
- **Harmony**: up to 2 extra voices (3rd, 5th, 6th, octave…) that stay in key. Voice A goes left, voice B goes right.
- **Saturation**: warmth up to full distortion
- **De-Esser**: tames harsh S sounds
- **Compressor**: Thresh / Ratio / Makeup
- **Delay**: synced to FL's tempo, ping-pong, darker repeats
- **Reverb**: Size / Damp / Mix
- **EQ**: Low Cut and High Cut. Set both close together for a phone effect.

**Presets**: use the dropdown or the `<` `>` arrows. **Save** stores your own presets in `Documents\TheDirtyGobbler\Presets`, and they show up under "User" in the list.

### Tips for the best tuning
- Use it on a **dry, single vocal** (one voice). It can't follow chords or two people at once.
- Put it **first** on the vocal chain, before other reverbs or delays.
- If it warbles between two notes, switch off the note you don't want, or raise Retune Speed a bit.
- Deep voices sound best with **Input Type = Low Male**.

---

## Project layout
```
CMakeLists.txt               build script (downloads JUCE 8.0.4 automatically)
.github/workflows/           GitHub Actions Windows build
Source/
  PluginProcessor.*          audio engine, parameters, presets, state
  PluginEditor.*             UI layout
  Params.h                   parameter IDs + dropdown lists
  Presets.h                  the 18 factory presets (easy to tweak)
  dsp/PitchDetector.h        YIN pitch detection
  dsp/PsolaShifter.h         TD-PSOLA pitch shifter with formant control
  dsp/ScaleQuantizer.h       key/scale snapping + harmony notes
  dsp/Effects.h              doubler, delay, de-esser, saturation, tilt
  ui/                        look & feel, knobs, pitch display
```

Built with JUCE. If you share or sell the plugin, check JUCE's license (juce.com).
