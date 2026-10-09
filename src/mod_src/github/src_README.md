# Source of the Realistic Economy mod

The mod is one plugin, `RealisticEconomy.asi`, loaded into the game by the public Ultimate ASI Loader (shipped as `version.dll`). It is written in C for 32-bit Windows and changes the running game in memory; it changes no file of the game and no save.

## What is here

| Path | What it is |
|---|---|
| `mod_src/plugin/` | the plugin. `re_main.c` installs the hooks and holds the rules; `re_sites.c` lists every address of the game the plugin touches, with the bytes it expects there; `re_lang.c` and the `re_lang_*.h` files hold the texts in the game's eight languages; `re_wording.c` holds the corrections of the game's own texts |
| `mod_src/tests/` | `engine_test.c` and `credit_test.c` (run by the build), `install_test.ps1` (runs the two batch files in a made-up game folder) |
| `mod_src/dist/` | what goes into the package as it is: the settings file, the two descriptions, the install and uninstall files, the licences |
| `mod_src/build.ps1`, `package.ps1`, `github.ps1` | build and test, make the zip, collect this repository |
| `mod_src/github/` | the READMEs of this repository |
| `analysis/scripts/lang_placeholders.py` | compares the texts of every language file of the game with the English ones; it found the texts `re_wording.c` corrects |

## Building

The scripts expect this folder to be the game folder's copy they work in, with these next to `mod_src`:

- `analysis/_tools/w64devkit/` : w64devkit (a 32-bit `gcc`), the compiler
- `analysis/_tools/asi_loader_v9.7.4/dinput8.dll` : Ultimate ASI Loader v9.7.4, which the package renames to `version.dll`
- `TGL2.exe` and `modsLanguages/` of This Grand Life 2 v1.03.22 : the engine test reads the addresses of the game's program file and the texts of its language files. Neither is in this repository.

```
pwsh -NoProfile -File mod_src/build.ps1
pwsh -NoProfile -File mod_src/package.ps1
pwsh -NoProfile -File mod_src/tests/install_test.ps1
```

Each prints a last line that ends with `verdict=PASS` or `verdict=FAIL`. Every test has a control: a run in which a known fault is put in and the test has to fail.

## Not here

The mod was also checked in the running game, with scripts that click through its windows and read the plugin's log. Those scripts need save files and captures of the game's screens, which are not published.

## Another version of the game

The plugin compares the game's program file with the one it was written for and does nothing on any other. Every address is in `re_sites.c` with the bytes expected there, so a new game version shows as a list of places that no longer match.
