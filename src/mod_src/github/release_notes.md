Realistic Economy 1.1.2 for This Grand Life 2, game version v1.03.22.

**New in 1.1.2**: the hover text of a business's box "buy worn-out assets again" says what the box does with the mod, in lines that begin with "Mod:" under the game's own sentence: an asset a job is short of is bought too, what the cash does not cover becomes a debt due at the third month end, and a worn-out asset bought again is paid for. Until now only the description said so.

**Fixed in 1.1.1**: a business handed over whole (Ctrl+Shift-click on one of its advert icons) could lose the two switches of the game that the hand-over had switched on. Loading a save that has them off left the business locked and orange while its staff was outside the automatic management and "buy worn-out assets again" was off: the mod hired nobody there, bought no asset and signed no contract, and the lock kept you from the two switches. An older save does that, and so does a save from before the mod, which the mod switches off at every load. Now a load switches both on again in a business handed over whole, and the rule for a save from before the mod leaves such a business out.

**Also in 1.1.1**: the monthly summary right after a load names what is handed to the mod, for example "Handed to the mod: S-Law (contracts), S-Bro (adverts), S-Con (all of it)". What you hand over is kept with the playthrough and not in a save, so an older save shows adverts or a whole business as the mod's that you handed over later; the line says so at once. With nothing handed over there is no line.

**Shorter lines at the top of the screen (1.1.1)**: several of the mod's lines there were wider than the band they are written in and were cut off at both ends, in every language. They are shorter now. The long explanations stay in the monthly summary; for a save from before the mod and for a business that is short of floor space the top line is a short one, "A save from before the mod: the mod changed things. See the Monthly Summary" for example.

**New in 1.1.0**: a month end that a load does not change. The same month of a playthrough ends with the same economic growth and share prices whatever you do first, a listing gets the price its window showed, and the properties and offers a new month brings are fixed the same way. Switch: `monthEndFixed` under `[guard]` in `RealisticEconomy.ini`.

**Install**: close the game, extract `RealisticEconomy-1.1.2.zip` into the game folder (the one that holds `TGL2.exe`) and double-click `RealisticEconomy_install.bat`. It copies your saves to `saves_before_RealisticEconomy_1` first. To remove the mod, run `RealisticEconomy_uninstall.bat`.

**Coming from an earlier version**: extract and run the same file. It replaces the mod's files and leaves your settings file as it was (the saves are not copied a second time).

What the mod adds and how to use it is in the README of this repository, in the game's eight languages: English, 한국어, Deutsch, Español, Français, Português (Brasil), 日本語, 简体中文.

The full description is inside the zip (`RealisticEconomy_README_en.txt`, `RealisticEconomy_README_ko.txt`). The SHA256 of the zip is in `RealisticEconomy-1.1.2.zip.sha256.txt`.

An antivirus may question `version.dll`: it is the public Ultimate ASI Loader (MIT), the file that loads the mod when the game starts.
