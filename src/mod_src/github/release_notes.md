Realistic Economy 1.2.0 for This Grand Life 2, game version v1.03.22.

**New in 1.2.0: balance.** A few numbers of the game's economy that lean to one side are changed. Each has its own setting under `[balance]` in `RealisticEconomy.ini`, and `balance=0` under `[features]` switches all of them off.

- The fee of buying and selling shares is 2% (the game: 1%). The stocks window shows what a share is worth and the price moves a fifth of the way there every month, so buying what is cheap earned far more than anything else in the game. For somebody who buys and holds little changes.
- The fee of a futures contract is 10% of its margin (the game: 1%), which is 0.5% of the contract. The label under the buttons says "fee 10% of margin" now.
- Cash earns interest: at a month end, a twelfth of half the central-bank rate. A savings account pays 0.8 times that rate (the game: 0.5). The three make a ladder: cash (no visit to the bank), a savings account (two visits, the money can be taken out at any time), a certificate of deposit (0.8 plus one point, held for its term).
- A lease that is renewed takes the market's rent, lower as well as higher. In the game a rent only ever went up.
- A business you start has an awareness of 25% on its first day (the game: 0%, which is a twentieth of its customers until adverts have worked).
- Hydroponics Farm earns what the other business types do, and the degree Communications costs half.

**Fixed in 1.2.0, four faults of the game itself:**

- A mortgage was not held to the amount the bank approved: with a small deposit a property was bought on a loan of nearly its whole price, and the cash stayed. The loan is the approved amount now and the rest is paid at the purchase.
- A rule of the Monthly Auto Transfer that buys shares bought them without cash, on a debt without interest. It buys only when the cash covers it now; the monthly summary says when it did not.
- Two nominees with the same score in a board election were both seated: six directors, and the board tab of that company ended the game. Equal scores are parted now.
- Building Construction's contract "Supply Construction Site" never asked for its civil engineer (a wrong number in the game's data). It does now, and its payout has the engineer's part.

**A matter of taste, and how to switch it off**: the README now has one table of every feature that changes a rule or a number of the game (the balance, the trade lock after a load, loan rates by credit grade, the listing that pays cash, board seats, the casino, education that does not decay, business automation) with the line of `RealisticEconomy.ini` that switches it off. Business automation does nothing in a business until you hand that business over inside the game, and the same click takes it back. The description inside the zip begins with the same list in more detail.

**Shorter hover texts (1.2.0)**: what the mod adds under a hover text of the game is two lines at most in Korean and three in the other languages, looked at in all eight languages. The longer explanations are in the description inside the zip.

**Coming from an earlier version**: extract and run the same file. It replaces the mod's files and leaves your settings file as it was (the saves are not copied a second time). The balance changes are on without a line in an older settings file; to keep the game's numbers add `balance=0` under `[features]`.

**Earlier versions**: 1.1.2 put the mod's line under the hover text of the box "buy worn-out assets again". 1.1.1 repaired a business handed over whole that a load had left locked without its two switches, added the summary line "Handed to the mod: …" after a load, and shortened the lines at the top of the screen. 1.1.0 made a month end that a load does not change (`monthEndFixed` under `[guard]`).

**Install**: close the game, extract `RealisticEconomy-1.2.0.zip` into the game folder (the one that holds `TGL2.exe`) and double-click `RealisticEconomy_install.bat`. It copies your saves to `saves_before_RealisticEconomy_1` first. To remove the mod, run `RealisticEconomy_uninstall.bat`.

What the mod adds and how to use it is in the README of this repository, in the game's eight languages: English, 한국어, Deutsch, Español, Français, Português (Brasil), 日本語, 简体中文.

The full description is inside the zip (`RealisticEconomy_README_en.txt`, `RealisticEconomy_README_ko.txt`). The SHA256 of the zip is in `RealisticEconomy-1.2.0.zip.sha256.txt`.

An antivirus may question `version.dll`: it is the public Ultimate ASI Loader (MIT), the file that loads the mod when the game starts.
