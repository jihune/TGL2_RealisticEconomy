Realistic Economy 1.2.1 for This Grand Life 2, game version v1.03.22.

**New in 1.2.1**

- Four degrees cost a year of the pay of the jobs they open, before the city's and the price index's part: Computer Science $53,800 (the game: $113,000), Art and Design $51,200 ($107,300), Science and Engineering $63,000 ($115,000), Business and Finance $56,500 ($125,000). In the game they cost 22 to 27 months of that pay, where 25 of the 30 courses that open jobs cost 13 months or less. The other courses keep their price. `degreePrices=0` under `[balance]` keeps the game's prices.
- Fifteen more texts of the game put right. Korean: three lines of a business's month that were in English (a pay demand, a member of staff who leaves for personal reasons, the heading of an automatic purchase). French, German, Chinese: that heading, which read "car purchase". Japanese: the line of a bankrupt company's shares, which left out the number of shares. German, Japanese, Chinese: the news of a new currency (the Japanese and the Chinese file had each other's sentence). Korean, German, French, Japanese, Chinese: the title of the month's summary (German "Beenden von Oktober", which means "quit", is now "Ende Oktober 2035"; Japanese and Chinese had the 月 of the month twice).
- The mod's line of the research fee in the month's summary: in Japanese and Chinese it broke inside the amount ("$8,0" and "41.20" on two lines), and the sentence now reaches the second line before the amount starts. In English, Spanish, French and Portuguese it said "1 companies" for one subscribed company; it is in the singular now.

**From 1.2.0: balance.** A few numbers of the game's economy that lean to one side are changed. Each has its own setting under `[balance]` in `RealisticEconomy.ini`, and `balance=0` under `[features]` switches all of them off.

- The fee of buying and selling shares is 2% (the game: 1%). The stocks window shows what a share is worth and the price moves a fifth of the way there every month, so buying what is cheap earned far more than anything else in the game. For somebody who buys and holds little changes.
- The fee of a futures contract is 10% of its margin (the game: 1%), which is 0.5% of the contract. The label under the buttons says "fee 10% of margin".
- Cash earns interest: at a month end, a twelfth of half the central-bank rate. A savings account pays 0.8 times that rate (the game: 0.5). The three make a ladder: cash (no visit to the bank), a savings account (two visits, the money can be taken out at any time), a certificate of deposit (0.8 plus one point, held for its term).
- A lease that is renewed takes the market's rent, lower as well as higher. In the game a rent only ever went up.
- A business you start has an awareness of 25% on its first day (the game: 0%, which is a twentieth of its customers until adverts have worked).
- Hydroponics Farm earns what the other business types do, and the degree Communications costs half.

**Fixed in 1.2.0, four faults of the game itself:** a mortgage beyond the amount the bank approved; a rule of the Monthly Auto Transfer that bought shares without cash; two nominees with the same score in a board election both seated, after which the board tab ended the game; Building Construction's contract "Supply Construction Site" that never asked for its civil engineer.

**A matter of taste, and how to switch it off**: the README has one table of every feature that changes a rule or a number of the game, with the line of `RealisticEconomy.ini` that switches it off. Business automation does nothing in a business until you hand that business over inside the game, and the same click takes it back. The description inside the zip begins with the same list in more detail.

**Coming from an earlier version**: extract and run the same file. It replaces the mod's files and leaves your settings file as it was (the saves are not copied a second time). The new settings work without a line in an older settings file; to keep the game's numbers add `balance=0` under `[features]`.

**Earlier versions**: 1.2.0 brought the balance, the four repairs above, shorter hover texts and the table of what is a matter of taste. 1.1.2 put the mod's line under the hover text of the box "buy worn-out assets again". 1.1.1 repaired a business handed over whole that a load had left locked without its two switches. 1.1.0 made a month end that a load does not change (`monthEndFixed` under `[guard]`).

**Install**: close the game, extract `RealisticEconomy-1.2.1.zip` into the game folder (the one that holds `TGL2.exe`) and double-click `RealisticEconomy_install.bat`. It copies your saves to `saves_before_RealisticEconomy_1` first. To remove the mod, run `RealisticEconomy_uninstall.bat`.

What the mod adds and how to use it is in the README of this repository, in the game's eight languages: English, 한국어, Deutsch, Español, Français, Português (Brasil), 日本語, 简体中文.

The full description is inside the zip (`RealisticEconomy_README_en.txt`, `RealisticEconomy_README_ko.txt`). The SHA256 of the zip is in `RealisticEconomy-1.2.1.zip.sha256.txt`.

An antivirus may question `version.dll`: it is the public Ultimate ASI Loader (MIT), the file that loads the mod when the game starts.
